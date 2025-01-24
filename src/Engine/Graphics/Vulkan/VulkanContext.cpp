/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "VulkanContext.h"
#include "VulkanSurface.h"

#include "../../Logger.h"
#include "../../Options.h"

#include "VulkanDescriptorSet.h"
#include "VulkanSampler.h"
#include "Buffer/VulkanBuffer.h"
#include "Pipeline/VulkanPipelineFactory.h"

#include "../../Engine.h"
#include "../../Platform/Window.h"
#include "../../Platform/WindowSystem.h"
#include "../../../version.h"

#include <glslang/Public/ShaderLang.h>

namespace OpenXcom
{

VkBool32 debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	(void)messageSeverity;
	(void)messageTypes;
	(void)pUserData;
	Log(LOG_DEBUG) << "Validation layer: " << pCallbackData->pMessage;
	return VK_FALSE;
}

VulkanQueue::VulkanQueue()
	: _device(nullptr), _familyIndex(std::numeric_limits<uint32_t>::max()), _queue(nullptr), _commandPool(nullptr)
{
}

VulkanQueue::~VulkanQueue()
{
	reset();
}

void VulkanQueue::reset()
{
	if (_device)
	{
		if (_commandBuffers.size() > 0)
		{
			_device.freeCommandBuffers(_commandPool, _commandBuffers);
			_commandBuffers.clear();
		}

		if (_commandPool)
		{
			_device.destroyCommandPool(_commandPool);
			_commandPool = nullptr;
		}
		_device = nullptr;
	}

	_familyIndex = std::numeric_limits<uint32_t>::max();
	_queue = nullptr;
}

void VulkanQueue::create(vk::Device device, uint32_t familyIndex, bool shouldCreateCommandBuffer)
{
	_device = device;
	_familyIndex = familyIndex;

	_queue = _device.getQueue(_familyIndex, 0);

	if(shouldCreateCommandBuffer)
	{
		// create the command pool
		vk::CommandPoolCreateInfo createInfo;
		createInfo.queueFamilyIndex = _familyIndex;
		createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;	//vk::CommandPoolCreateFlagBits::eTransient?
		_commandPool = _device.createCommandPool(createInfo);

		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = _commandPool;

		//If/when we ever have multithreaded rendering, it would be good to have command buffers for each thread.
		allocInfo.commandBufferCount = 1;

		_commandBuffers = _device.allocateCommandBuffers(allocInfo);
	}
}


VulkanContext::VulkanContext(const Options& options)
	: _instance(nullptr), _device(nullptr), _physicalDevice(nullptr),
	_swapChainImageFormat(vk::Format::eUndefined), _allocator()
{
	initializeInstance();

	// Create the Vulkan Memory Allocator
	VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;
	vulkanFunctions.vkGetPhysicalDeviceProperties = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceProperties;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceMemoryProperties;
	vulkanFunctions.vkAllocateMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkAllocateMemory;
	vulkanFunctions.vkFreeMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkFreeMemory;
	vulkanFunctions.vkMapMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkMapMemory;
	vulkanFunctions.vkUnmapMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkUnmapMemory;
	vulkanFunctions.vkFlushMappedMemoryRanges = VULKAN_HPP_DEFAULT_DISPATCHER.vkFlushMappedMemoryRanges;
	vulkanFunctions.vkInvalidateMappedMemoryRanges = VULKAN_HPP_DEFAULT_DISPATCHER.vkInvalidateMappedMemoryRanges;
	vulkanFunctions.vkBindBufferMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindBufferMemory;
	vulkanFunctions.vkBindImageMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindImageMemory;
	vulkanFunctions.vkGetBufferMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetBufferMemoryRequirements;
	vulkanFunctions.vkGetImageMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetImageMemoryRequirements;
	vulkanFunctions.vkCreateBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateBuffer;
	vulkanFunctions.vkDestroyBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkDestroyBuffer;
	vulkanFunctions.vkCreateImage = VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateImage;
	vulkanFunctions.vkDestroyImage = VULKAN_HPP_DEFAULT_DISPATCHER.vkDestroyImage;
	vulkanFunctions.vkCmdCopyBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdCopyBuffer;

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = _physicalDevice;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	allocatorInfo.pVulkanFunctions = &vulkanFunctions;

	VkResult result = vmaCreateAllocator(&allocatorInfo, &_allocator);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan Memory Allocator.");
	}


	_descriptorSetFactory = std::make_unique<VulkanDescriptorSetFactory>(*this);
	_samplerFactory = std::make_unique<VulkanSamplerFactory>(*this);
	_pipelineFactory = std::make_unique<VulkanPipelineFactory>(*this);
}

VulkanContext::~VulkanContext()
{
	// destroy the pipeline factory
	_pipelineFactory.reset();

	// destroy the sampler factory
	_samplerFactory.reset();

	// destroy the descriptor set factory
	_descriptorSetFactory.reset();

	// destroy the queues
	_graphicsQueue.reset();
	_transferQueue.reset();
	_presentQueue.reset();

	// Destroy the Vulkan Memory Allocator
	if (_allocator)
	{
		vmaDestroyAllocator(_allocator);
	}

	if (_device)
	{
		_device.destroy();
	}

	if (_debugMessenger)
	{
		_instance.destroyDebugUtilsMessengerEXT(_debugMessenger);
	}
	if (_instance)
	{
		_instance.destroy();
	}

	glslang::FinalizeProcess();
}

void VulkanContext::initializeInstance()
{
	try
	{
		glslang::InitializeProcess();

		// Load the Vulkan function loader
		PFN_vkGetInstanceProcAddr getInstanceProcAddr = _loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		if (!getInstanceProcAddr)
		{
			throw std::runtime_error("Failed to load vkGetInstanceProcAddr.");
		}

		VULKAN_HPP_DEFAULT_DISPATCHER.init(getInstanceProcAddr);

		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
										  vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
									  vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
									  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		debugCreateInfo.pfnUserCallback = debugCallback; // Pointer to your callback function

		vk::ApplicationInfo appInfo("OpenXcom", VK_MAKE_VERSION(OPENXCOM_VERSION_MAJOR, OPENXCOM_VERSION_MINOR, OPENXCOM_VERSION_PATCH), "OpenXcom", VK_MAKE_VERSION(OPENXCOM_VERSION_MAJOR, OPENXCOM_VERSION_MINOR, OPENXCOM_VERSION_PATCH), VK_API_VERSION_1_0);
		vk::InstanceCreateInfo createInfo{};
		createInfo.pApplicationInfo = &appInfo;
		createInfo.pNext = &debugCreateInfo;

		std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();

		Log(LOG_DEBUG) << "Available extensions:";
		for (const vk::ExtensionProperties& ext : availableExtensions)
		{
			Log(LOG_DEBUG) << "  " << ext.extensionName;
		}

		// Specify validation layers (only in debug mode)
#ifdef _DEBUG
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"};
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif

		// Add instance extensions (e.g., for window surface support)
		const std::vector<const char*> extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#if defined(_WIN32)
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME // Platform-specific; adjust for other platforms
#elif defined(__linux__)
			VK_KHR_XLIB_SURFACE_EXTENSION_NAME // Platform-specific; adjust for other platforms
#elif defined(__APPLE__)
			VK_EXT_METAL_SURFACE_EXTENSION_NAME // Platform-specific; adjust for other platforms
#endif
		};

		// TODO: check that we have all the extensions available
		Log(LOG_DEBUG) << "Required extensions:";
		for (const char* const& ext : extensions)
		{
			Log(LOG_DEBUG) << "  " << ext;
		}

		for (const char* const& ext : extensions)
		{
			if (std::find_if(availableExtensions.begin(), availableExtensions.end(), [ext](const vk::ExtensionProperties& prop) { return strcmp(prop.extensionName, ext) == 0; }) == availableExtensions.end())
			{
				throw std::runtime_error("Required extension not available: " + std::string(ext));
			}
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Create the Vulkan instance
		_instance = vk::createInstance(createInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance);

		// Create the debug messenger
		_debugMessenger = _instance.createDebugUtilsMessengerEXT(debugCreateInfo);

		Log(LOG_DEBUG) << "Vulkan instance created";

		// Create a temporary platform window
		PlatformWindowSystem& windowSystem = getEngine().getPlatformWindowSystem();
		std::shared_ptr<PlatformWindow> window = windowSystem.createWindow("VulkanSurfaceCapWindow", 0, 0).lock();

		vk::SurfaceKHR surface = VulkanSurface::createSurface(_instance, window->getHandle());

		// Select a physical device and create the logical device
		selectPhysicalDevice(surface);
		initializeDevice(surface);

		VulkanSurface::destroySurface(_instance, surface);
		windowSystem.destroyWindow(window);
		window.reset();

	}
	catch (const vk::SystemError& err)
	{
		(void)err;
		throw std::runtime_error("Failed to create Vulkan instance: ");
	}
	catch (const std::exception& err)
	{
		(void)err;
		throw std::runtime_error("Error creating Vulkan instance: ");
	}
	catch (...)
	{
		throw std::runtime_error("Unknown error occurred creating Vulkan instance");
	}
}

void VulkanContext::selectPhysicalDevice(const vk::SurfaceKHR& surface)
{
	std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
	if (physicalDevices.empty())
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support.");
	}

	Log(LOG_DEBUG) << "Available physical devices:";
	for (const vk::PhysicalDevice& device : physicalDevices)
	{
		vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

		Log(LOG_DEBUG) << "  Device Name: " << deviceProperties.deviceName;
		Log(LOG_DEBUG) << "  Device Type: " << vk::to_string(deviceProperties.deviceType);

		bool supportsSwapchain = false;
		bool hasGraphicsQueue = false;
		bool hasPresentationQueue = false;

		// Check for swap chain support
		std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();
		for (const vk::ExtensionProperties& ext : availableExtensions)
		{
			if (std::string((const char*)ext.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			{
				supportsSwapchain = true;
				break;
			}
		}

		// Check for suitable queue families
		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
		for (uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				hasGraphicsQueue = true;
			}

			// You need a surface created to check for presentation support
			if (device.getSurfaceSupportKHR(i, surface)) // _surface should be defined earlier
			{
				hasPresentationQueue = true;
			}

			if (hasGraphicsQueue && hasPresentationQueue)
			{
				break;
			}
		}

		// Check if the device meets all requirements
		if (supportsSwapchain && hasGraphicsQueue && hasPresentationQueue &&
			deviceFeatures.samplerAnisotropy) // Add more features as needed
		{
			_physicalDevice = device;
			Log(LOG_DEBUG) << "Selected Device: " << deviceProperties.deviceName;
			break;
		}
	}

	if (!_physicalDevice)
	{
		throw std::runtime_error("Failed to find a suitable GPU.");
	}
}

void VulkanContext::initializeDevice(const vk::SurfaceKHR& surface)
{
	uint32_t graphicsQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
	uint32_t transferQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
	uint32_t presentQueueFamilyIndex = std::numeric_limits<uint32_t>::max();

	// Find queue families
	std::vector<vk::QueueFamilyProperties> queueFamilies = _physicalDevice.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			graphicsQueueFamilyIndex = i;
		}

		if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer)
		{
			transferQueueFamilyIndex = i;
		}

		if (_physicalDevice.getSurfaceSupportKHR(i, surface))
		{
			presentQueueFamilyIndex = i;
		}

		if (graphicsQueueFamilyIndex != std::numeric_limits<uint32_t>::max() &&
			transferQueueFamilyIndex != std::numeric_limits<uint32_t>::max() &&
			presentQueueFamilyIndex != std::numeric_limits<uint32_t>::max())
		{
			break; // Found suitable queue families
		}
	}

	if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max() ||
		transferQueueFamilyIndex == std::numeric_limits<uint32_t>::max() ||
		presentQueueFamilyIndex == std::numeric_limits<uint32_t>::max())
	{
		throw std::runtime_error("Failed to find suitable queue families.");
	}

	// Specify Device Queues
	float queuePriority = 1.0f;

	vk::DeviceQueueCreateInfo graphicsQueueCreateInfo{};
	graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

	vk::DeviceQueueCreateInfo presentQueueCreateInfo{};
	if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
	{
		presentQueueCreateInfo.queueFamilyIndex = presentQueueFamilyIndex;
		presentQueueCreateInfo.queueCount = 1;
		presentQueueCreateInfo.pQueuePriorities = &queuePriority;
	}

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {graphicsQueueCreateInfo};
	if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
	{
		queueCreateInfos.push_back(presentQueueCreateInfo);
	}

	// Specify Device Features
	vk::PhysicalDeviceFeatures deviceFeatures{};

	// Finally, create the device
	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Specify device extensions (e.g., VK_KHR_swapchain)
	const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Create the logical device
	_device = _physicalDevice.createDevice(deviceCreateInfo);

	_graphicsQueue.create(_device, graphicsQueueFamilyIndex, false);
	_transferQueue.create(_device, transferQueueFamilyIndex, true);
	_presentQueue.create(_device, presentQueueFamilyIndex, false);
}



} // namespace OpenXcom
