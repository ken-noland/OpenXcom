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
#include "Surface/VulkanWindowedSurface.h"

#include "../../Logger.h"
#include "../../Options.h"
#include "../../EngineContext.h"

#include "VulkanDescriptorSet.h"
#include "VulkanSampler.h"
#include "Buffer/VulkanBuffer.h"
#include "Pipeline/VulkanPipelineFactory.h"

#include "../../Engine.h"
#include "../../Platform/PlatformWindowSystem.h"
#include "../../../version.h"

#include <glslang/Public/ShaderLang.h>

#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#else
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#endif

// Enable these to get detailed information about Vulkan extensions and layers
//#define DUMP_VULKAN_INSTANCE_EXTENSIONS
//#define DUMP_VULKAN_INSTANCE_LAYERS
//#define DUMP_VULKAN_DEVICE_EXTENSIONS

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
	//DEBUG_BREAK();
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


VulkanContext::VulkanContext(EngineContext& context)
	: _engineContext(context), _instance(nullptr), _device(nullptr), _physicalDevice(nullptr),
	  _swapChainImageFormat(vk::Format::eUndefined), _allocator()
{
	bool isHeadless = _engineContext.getOptions().get<&GraphicsOptions::_headless>();
	initializeInstance(isHeadless);

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
	vulkanFunctions.vkGetDeviceBufferMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceBufferMemoryRequirements;
	vulkanFunctions.vkGetDeviceImageMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceImageMemoryRequirements;

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
	_device.waitIdle();

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
		_device.waitIdle();
		_device.destroy();
		_device = nullptr;

		Log(LOG_INFO) << "Vulkan Device Destroyed";
	}

	if (_debugMessenger)
	{
		_instance.destroyDebugUtilsMessengerEXT(_debugMessenger);
		_debugMessenger = nullptr;
	}
	if (_instance)
	{
		_instance.destroy();
		_instance = nullptr;
	}

	glslang::FinalizeProcess();
}

void VulkanContext::initializeInstance(bool isHeadless)
{
	glslang::InitializeProcess();

	PFN_vkGetInstanceProcAddr getInstanceProcAddr = _loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	if (!getInstanceProcAddr)
	{
		Log(LOG_ERROR) << "Failed to load vkGetInstanceProcAddr.";
		throw new std::runtime_error("Failed to load vkGetInstanceProcAddr.");
	}

	VULKAN_HPP_DEFAULT_DISPATCHER.init(getInstanceProcAddr);

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
									  vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
								  vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
								  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	debugCreateInfo.pfnUserCallback = debugCallback;

	vk::ApplicationInfo appInfo("OpenXcom",
								VK_MAKE_VERSION(OPENXCOM_VERSION_MAJOR, OPENXCOM_VERSION_MINOR, OPENXCOM_VERSION_PATCH),
								"OpenXcom",
								VK_MAKE_VERSION(OPENXCOM_VERSION_MAJOR, OPENXCOM_VERSION_MINOR, OPENXCOM_VERSION_PATCH),
								VK_API_VERSION_1_0);


	std::vector<const char*> extensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
	std::vector<const char*> required_layers = {};
	std::vector<const char*> optional_layers = {"VK_LAYER_KHRONOS_validation"};

	std::vector<const char*> layers = required_layers;

	if (!isHeadless)
	{
		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
		extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
		extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif
	}

	std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();

#if defined(DUMP_VULKAN_INSTANCE_EXTENSIONS)
	Log(LOG_INFO) << "Available extensions: ";
	for (vk::ExtensionProperties extension : availableExtensions)
	{
		Log(LOG_INFO) << "  " << extension.extensionName;
	}
#endif

	for (const char* ext : extensions)
	{
		if (std::none_of(availableExtensions.begin(), availableExtensions.end(),
						 [ext](const vk::ExtensionProperties& prop) { return strcmp(prop.extensionName, ext) == 0; }))
		{
			Log(LOG_ERROR) << "Required extension not available: " << ext;
			throw new std::runtime_error("Required extension not available.");
		}
	}

	// Enumerate available instance layers
	std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

#if defined(DUMP_VULKAN_INSTANCE_LAYERS)
	Log(LOG_INFO) << "Available layers: ";
	for (vk::LayerProperties layer : availableLayers)
	{
		Log(LOG_INFO) << "  " << layer.layerName << "(" << layer.description << ")";
	}
#endif

	for (const char* layer : required_layers)
	{
		if (std::none_of(availableLayers.begin(), availableLayers.end(),
						 [layer](const vk::LayerProperties& layerProp) { return strcmp(layerProp.layerName, layer) == 0; }))
		{
			Log(LOG_ERROR) << "Required layer not available: " << layer;
			throw new std::runtime_error("Required layer not available.");
		}
	}

	// Check for optional layers
	for (const char* layer : optional_layers)
	{
		if (std::none_of(availableLayers.begin(), availableLayers.end(),
						 [layer](const vk::LayerProperties& layerProp) { return strcmp(layerProp.layerName, layer) == 0; }))
		{
			Log(LOG_WARNING) << "Optional layer not available: " << layer;
		}
		else
		{
			layers.push_back(layer);
		}
	}

	vk::InstanceCreateInfo createInfo{};
	createInfo.pApplicationInfo = &appInfo;
	createInfo.pNext = &debugCreateInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Add the validation layers to the create info
	createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	createInfo.ppEnabledLayerNames = layers.data();

	_instance = vk::createInstance(createInfo);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance);

	_debugMessenger = _instance.createDebugUtilsMessengerEXT(debugCreateInfo);

	Log(LOG_INFO) << "Vulkan Instance created";

	vk::SurfaceKHR surface;
	if (!isHeadless)
	{
		OwningHandle<PlatformWindow> window = _engineContext.getPlatformWindowSystem().create("VulkanSurfaceCapWindow", 100, 100);
		surface = VulkanWindowedSurface::createSurface(_instance, *window);

		selectPhysicalDevice(surface);
		initializeDevice(surface);

		VulkanWindowedSurface::destroySurface(_instance, surface);
		window.reset();
	}
	else
	{
		selectPhysicalDevice(std::nullopt);
		initializeDevice(std::nullopt);
	}

}

void VulkanContext::selectPhysicalDevice(std::optional<vk::SurfaceKHR> surface)
{
	std::vector<vk::PhysicalDevice> physicalDevices = _instance.enumeratePhysicalDevices();
	if (physicalDevices.empty())
	{
		Log(LOG_ERROR) << "Failed to find GPUs with Vulkan support.";
		throw new std::runtime_error("Failed to find GPUs with Vulkan support.");
	}

	Log(LOG_INFO) << "Available physical devices:";
	for (const vk::PhysicalDevice& device : physicalDevices)
	{
		vk::PhysicalDeviceProperties properties = device.getProperties();
		vk::PhysicalDeviceFeatures features = device.getFeatures();
		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

		Log(LOG_INFO) << "  Device Name: " << properties.deviceName;
		Log(LOG_INFO) << "  Device Type: " << vk::to_string(properties.deviceType);

		bool supportsGraphics = false;
		bool supportsPresentation = false;
		bool supportsSwapchain = false;

		// Check for swap chain support (only needed if windowed)
		if (surface)
		{
			std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();
			supportsSwapchain = std::any_of(availableExtensions.begin(), availableExtensions.end(),
											[](const vk::ExtensionProperties& ext) { return strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0; });
		}

		for (uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				supportsGraphics = true;
			}
			if (surface && device.getSurfaceSupportKHR(i, *surface))
			{
				supportsPresentation = true;
			}
		}

		if (supportsGraphics && (!surface || (supportsPresentation && supportsSwapchain)) && checkPhysicalDeviceHasFeatures(properties, features))
		{
			_physicalDevice = device;
			Log(LOG_INFO) << "Selected Device: " << properties.deviceName;
			return;
		}
	}

	Log(LOG_ERROR) << "Failed to find a suitable GPU.";
	throw new std::runtime_error("Failed to find a suitable GPU.");
}

bool VulkanContext::checkPhysicalDeviceHasFeatures(const vk::PhysicalDeviceProperties& properties, const vk::PhysicalDeviceFeatures& features)
{
	//check for antialiasing
	if (!features.sampleRateShading)
	{
		Log(LOG_WARNING) << "Device does not support sample rate shading. Will attempt another device.";
		return false;
	}

	return true;
}


void VulkanContext::initializeDevice(std::optional<vk::SurfaceKHR> surface)
{
	uint32_t graphicsQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
	uint32_t transferQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
	uint32_t presentQueueFamilyIndex = std::numeric_limits<uint32_t>::max();

	auto queueFamilies = _physicalDevice.getQueueFamilyProperties();

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

		// Check surface support only if we are not in headless mode
		if (surface && _physicalDevice.getSurfaceSupportKHR(i, *surface))
		{
			presentQueueFamilyIndex = i;
		}
	}

	// Ensure that at least graphics and transfer queues are available
	if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max() ||
		transferQueueFamilyIndex == std::numeric_limits<uint32_t>::max())
	{
		Log(LOG_ERROR) << "Failed to find suitable queue families.";
		std::terminate();
	}

	bool isHeadless = !surface.has_value();

	// Specify Device Queues
	float queuePriority = 1.0f;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

	vk::DeviceQueueCreateInfo graphicsQueueCreateInfo{};
	graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(graphicsQueueCreateInfo);

	vk::DeviceQueueCreateInfo transferQueueCreateInfo{};
	transferQueueCreateInfo.queueFamilyIndex = transferQueueFamilyIndex;
	transferQueueCreateInfo.queueCount = 1;
	transferQueueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(transferQueueCreateInfo);

	if (!isHeadless && graphicsQueueFamilyIndex != presentQueueFamilyIndex)
	{
		vk::DeviceQueueCreateInfo presentQueueCreateInfo{};
		presentQueueCreateInfo.queueFamilyIndex = presentQueueFamilyIndex;
		presentQueueCreateInfo.queueCount = 1;
		presentQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(presentQueueCreateInfo);
	}

	// Specify Device Features
	vk::PhysicalDeviceFeatures deviceFeatures{};

	deviceFeatures.sampleRateShading = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	vk::PhysicalDeviceLineRasterizationFeaturesKHR lineRasterizationFeatures{};
	lineRasterizationFeatures.smoothLines = VK_TRUE;
	lineRasterizationFeatures.rectangularLines = VK_TRUE;
	lineRasterizationFeatures.bresenhamLines = VK_TRUE;

	// Specify device extensions
	std::vector<const char*> deviceExtensions;
	if (!isHeadless)
	{
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}


#if defined(DUMP_VULKAN_DEVICE_EXTENSIONS)
	std::vector<vk::ExtensionProperties> availableExtensions = _physicalDevice.enumerateDeviceExtensionProperties();
	Log(LOG_INFO) << "Available device extensions: ";
	for (vk::ExtensionProperties extension : availableExtensions)
	{
		Log(LOG_INFO) << "  " << extension.extensionName;
	}
#endif

	// Finally, create the logical device
	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	deviceCreateInfo.pNext = &lineRasterizationFeatures;

	_device = _physicalDevice.createDevice(deviceCreateInfo);
	Log(LOG_INFO) << "Vulkan Device created";

	_graphicsQueue.create(_device, graphicsQueueFamilyIndex, true);
	_transferQueue.create(_device, transferQueueFamilyIndex, true);

	if (!isHeadless)
	{
		_presentQueue.create(_device, presentQueueFamilyIndex, false);
	}
}

vk::BufferUsageFlags VulkanContext::getBufferUsageFlags(BufferUsage usage)
{
	switch (usage)
	{
	case BufferUsage::Vertex:
		return vk::BufferUsageFlagBits::eVertexBuffer;
	case BufferUsage::Index:
		return vk::BufferUsageFlagBits::eIndexBuffer;
	case BufferUsage::Uniform:
		return vk::BufferUsageFlagBits::eUniformBuffer;
	case BufferUsage::Storage:
		return vk::BufferUsageFlagBits::eStorageBuffer;
	}

	return vk::BufferUsageFlags();
}




} // namespace OpenXcom
