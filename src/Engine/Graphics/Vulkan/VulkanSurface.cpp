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

#include "VulkanSurface.h"
#include "Shader/VulkanShader.h"

#include "../../Platform/Window.h"
#include "../../Engine.h"
#include "../../Resource/ResourceSystem.h"
#include "../../Resource/Shader/ShaderManager.h"
#include "../../Logger.h"

#include <glm/gtc/matrix_transform.hpp>

// BEGIN TEMP
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <limits>

#undef None
#include "../../../Entity/Common/RTTR.h"


namespace OpenXcom
{

const char* vertexShaderSource = R"(
	#version 450

	layout(location = 0) in vec2 inPosition;
	layout(location = 1) in vec2 inTexCoord;
	layout(location = 0) out vec2 fragTexCoord;

	layout(push_constant) uniform PushConstants {
		mat4 transform;
	} pushConstants;

	void main() {
		gl_Position = pushConstants.transform * vec4(inPosition, 0.0, 1.0);
		fragTexCoord = inTexCoord;
	}
)";

const char* fragmentShaderSource = R"(
	#version 450
	layout(location = 0) in vec2 fragTexCoord;     // Input texture coordinates from vertex shader

	layout(binding = 0) uniform sampler2D uTexture; // Texture sampler, bound to descriptor set

	layout(location = 0) out vec4 outColor;        // Output color

	void main() {
		vec4 texColor = texture(uTexture, fragTexCoord);
		outColor = texColor;
	}
)";

struct Vertex
{
	glm::vec2 pos;
	glm::vec2 texCoord;
};

//Vertex vertices[] = {
//	{{-0.5f, -0.5f}, {0.0f, 0.0f}}, // Bottom-left, red
//	{{0.5f, -0.5f}, {1.0f, 0.0f}},  // Bottom-right, green
//	{{0.5f, 0.5f}, {1.0f, 1.0f}},   // Top-right, blue
//	{{-0.5f, 0.5f}, {0.0f, 1.0f}}   // Top-left, white
//};

//Vertex vertices[] = {
//	{{0.0f, 0.0f}, {0.0f, 0.0f}},     // Bottom-left corner
//	{{320.0f, 0.0f}, {1.0f, 0.0f}},   // Bottom-right corner
//	{{320.0f, 200.0f}, {1.0f, 1.0f}}, // Top-right corner
//	{{0.0f, 200.0f}, {0.0f, 1.0f}}    // Top-left corner
//};

Vertex vertices[] = {
	{{-1.0f, -1.0f}, {0.0f, 0.0f}}, // Bottom-left
	{{1.0f, -1.0f}, {1.0f, 0.0f}},  // Bottom-right
	{{1.0f, 1.0f}, {1.0f, 1.0f}},   // Top-right
	{{-1.0f, 1.0f}, {0.0f, 1.0f}}   // Top-left
};

// Indices for two triangles forming a rectangle
uint16_t indices[] = {0, 1, 2, 2, 3, 0};

} // namespace OpenXcom

// Run time type information
SIMPLERTTR
{
	SimpleRTTR::registration().type<vk::Format>()
		.value(vk::Format::eR8G8B8A8Unorm, "R8G8B8A8Unorm")
		.value(vk::Format::eR32G32Sfloat, "R32G32Sfloat")
		.value(vk::Format::eR32G32B32Sfloat, "R32G32B32Sfloat");

	SimpleRTTR::registration().type<glm::vec2>()
		.meta("format", vk::Format::eR32G32Sfloat)
		.property(&glm::vec2::x, "x")
		.property(&glm::vec2::y, "y");

	SimpleRTTR::registration().type<glm::vec3>()
		.meta("format", vk::Format::eR32G32B32Sfloat)
		.property(&glm::vec3::x, "x")
		.property(&glm::vec3::y, "y")
		.property(&glm::vec3::z, "z");

	SimpleRTTR::registration().type<OpenXcom::Vertex>()
		.property(&OpenXcom::Vertex::pos, "pos")
		.property(&OpenXcom::Vertex::texCoord, "texCoord");
}

// END TEMP

namespace OpenXcom
{

VulkanSurface::VulkanSurface(vk::Instance instance, const PlatformWindowHandle& window)
	: _instance(instance), _surface(nullptr), _device(nullptr), _commandPool(nullptr),
	  _swapChain(nullptr), _swapChainImageFormat(vk::Format::eUndefined), _swapChainExtent{}, _frames(),
	  _renderPass(nullptr), _pipelineLayout(nullptr), _pipeline(nullptr), _currentFrame(0), _windowHandle(window),
	  _vertexShader(ShaderManager::INVALID_HANDLE), _fragmentShader(ShaderManager::INVALID_HANDLE)
{
#if defined(_WIN32)
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.hwnd = window.hWnd;
	surfaceCreateInfo.hinstance = window.hInstance;

	_surface = _instance.createWin32SurfaceKHR(surfaceCreateInfo);
	if (!_surface)
	{
		throw std::runtime_error("Failed to create Vulkan surface.");
	}
#elif defined(__linux__)
	vk::XlibSurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.dpy = (Display*)window.display;
	surfaceCreateInfo.window = window.window;
	_surface = _instance.createXlibSurfaceKHR(surfaceCreateInfo);
	if (!_surface)
	{
		throw std::runtime_error("Failed to create Vulkan surface.");
	}
#else
	#error "Unsupported platform"
#endif
}

void VulkanSurface::initializeDevice(vk::Device& device, const vk::PhysicalDevice& physicalDevice, VmaAllocator allocator, uint32_t graphicsQueueFamilyIndex, vk::Queue& graphicsQueue, vk::Queue& presentQueue)
{
	_device = device;
	_graphicsQueue = graphicsQueue;
	_presentQueue = presentQueue;
	_physicalDevice = physicalDevice;

	_allocator = allocator;

	// Create the command queue
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	_commandPool = _device.createCommandPool(poolInfo);
}

// Function to get the client area dimensions
vk::Extent2D getClientAreaSize(const PlatformWindowHandle& handle)
{
#if defined(_WIN32)
	RECT clientRect;
	GetClientRect(handle.hWnd, &clientRect); // Retrieves the client area of the window
	return vk::Extent2D{
		static_cast<uint32_t>(clientRect.right - clientRect.left),
		static_cast<uint32_t>(clientRect.bottom - clientRect.top)};
#elif defined(__linux__)
	Window root;
	int x, y;
	unsigned int width, height, borderWidth, depth;

	// Get the geometry of the client area
	XGetGeometry((Display*)handle.display, handle.window, &root, &x, &y, &width, &height, &borderWidth, &depth);

	return vk::Extent2D{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)};
#else
	#error "Unsupported platform"
#endif
}

void VulkanSurface::initializeSwapChain()
{
	// Get the surface capabilities, formats, and present modes
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);
	std::vector<vk::SurfaceFormatKHR> surfaceFormats = _physicalDevice.getSurfaceFormatsKHR(_surface);
	std::vector<vk::PresentModeKHR> presentModes = _physicalDevice.getSurfacePresentModesKHR(_surface);

	// Choose the best format
	vk::SurfaceFormatKHR chosenFormat = surfaceFormats[0];
	for (const auto& availableFormat : surfaceFormats)
	{
		if (availableFormat.format == vk::Format::eR8G8B8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			chosenFormat = availableFormat;
			break;
		}
	}

	// Choose the best presentation mode
	vk::PresentModeKHR chosenPresentMode = vk::PresentModeKHR::eFifo; // Always supported
	for (const auto& availablePresentMode : presentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
		{
			chosenPresentMode = availablePresentMode;
			break;
		}
	}

	// Choose the best swap extent
	vk::Extent2D swapChainExtent = surfaceCapabilities.currentExtent;
	if (surfaceCapabilities.currentExtent.width == UINT32_MAX)
	{
		swapChainExtent = getClientAreaSize(_windowHandle); // Replace with actual window size
		swapChainExtent.width = std::clamp(swapChainExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		swapChainExtent.height = std::clamp(swapChainExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
	}

	// now create the swap chain
	vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.surface = _surface;
	swapChainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1; // Add an extra image for triple buffering
	swapChainCreateInfo.imageFormat = chosenFormat.format;
	swapChainCreateInfo.imageColorSpace = chosenFormat.colorSpace;
	swapChainCreateInfo.imageExtent = swapChainExtent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive; // Adjust for multiple queues if necessary
	swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapChainCreateInfo.presentMode = chosenPresentMode;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // Replace if resizing

	_swapChain = _device.createSwapchainKHR(swapChainCreateInfo);
	_swapChainImageFormat = chosenFormat.format;
	_swapChainExtent = swapChainExtent;

	// Define game and window dimensions
	float gameWidth = 320.0f;
	float gameHeight = 200.0f;
	float windowWidth = static_cast<float>(_swapChainExtent.width);
	float windowHeight = static_cast<float>(_swapChainExtent.height);

	// Calculate aspect ratios
	float gameAspectRatio = gameWidth / gameHeight;
	float windowAspectRatio = windowWidth / windowHeight;

	float scaleX = 1.0f;
	float scaleY = 1.0f;

	if (windowAspectRatio > gameAspectRatio)
	{
		// Window is wider than game surface
		scaleX = gameAspectRatio / windowAspectRatio;
		// Centered horizontally in NDC, so no offset needed
	}
	else
	{
		// Window is taller than game surface
		scaleY = windowAspectRatio / gameAspectRatio;
		// Centered vertically in NDC, so no offset needed
	}


	//_transform = glm::mat4(1.0f);
	_transform = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 1.0f));

	//// Create the orthographic projection matrix
	//glm::mat4 projection = glm::ortho(0.0f, windowWidth, windowHeight, 0.0f);

	//// Apply scaling and centering to the transform matrix
	//_transform = projection * glm::translate(glm::mat4(1.0f), glm::vec3(offsetX, offsetY, 0.0f));
	//_transform = glm::scale(_transform, glm::vec3(scale, scale, 1.0f));
}

void VulkanSurface::initializeFrames(const vk::RenderPass& renderPass)
{
	_renderPass = renderPass;

	// Retrieve swap chain images
	std::vector<vk::Image> swapChainImages = _device.getSwapchainImagesKHR(_swapChain);
	std::size_t swapChainImageCount = swapChainImages.size();

	_frames.resize(swapChainImageCount);

	for (size_t i = 0; i < swapChainImageCount; ++i)
	{
		FrameData frameData{};

		// Create semaphores
		vk::SemaphoreCreateInfo semaphoreInfo{};
		frameData.imageAvailableSemaphore = _device.createSemaphore(semaphoreInfo);
		frameData.renderFinishedSemaphore = _device.createSemaphore(semaphoreInfo);

		// Create fence
		vk::FenceCreateInfo fenceInfo{};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
		frameData.inFlightFence = _device.createFence(fenceInfo);

		// Create image view for swap chain image
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = swapChainImages[i]; // Assumes you've retrieved swap chain images
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = _swapChainImageFormat;
		viewInfo.components = {vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
							   vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		frameData.imageView = _device.createImageView(viewInfo);

		// Create framebuffer for each swap chain image view
		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.renderPass = _renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &frameData.imageView;
		framebufferInfo.width = _swapChainExtent.width;
		framebufferInfo.height = _swapChainExtent.height;
		framebufferInfo.layers = 1;
		frameData.framebuffer = _device.createFramebuffer(framebufferInfo);

		// Allocate command buffer for this frame
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = _commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		frameData.commandBuffer = _device.allocateCommandBuffers(allocInfo)[0];

		_frames[i] = std::move(frameData);
	}
}

void VulkanSurface::initializeShaders(ShaderManager& shaderManager)
{
	_vertexShader = shaderManager.loadShaderFromMemory("WindowSurfaceVertex", vertexShaderSource, ShaderType::Vertex);
	assert(_vertexShader != ShaderManager::INVALID_HANDLE);

	_fragmentShader = shaderManager.loadShaderFromMemory("WindowSurfaceFragment", fragmentShaderSource, ShaderType::Fragment);
	assert(_fragmentShader != ShaderManager::INVALID_HANDLE);
}

void VulkanSurface::initializeDescriptorSet()
{
	// Create descriptor set layout
	vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerLayoutBinding;

	_descriptorSetLayout = _device.createDescriptorSetLayout(layoutInfo);

	// Create descriptor pool
	vk::DescriptorPoolSize poolSize{};

	poolSize.type = vk::DescriptorType::eCombinedImageSampler;
	poolSize.descriptorCount = 1;

	vk::DescriptorPoolCreateInfo poolInfo{};

	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	_descriptorPool = _device.createDescriptorPool(poolInfo);

	// Create descriptor set
	vk::DescriptorSetAllocateInfo allocInfo{};

	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_descriptorSetLayout;

	_descriptorSet = _device.allocateDescriptorSets(allocInfo)[0];
}

void VulkanSurface::initializePipeline()
{
	Engine& engine = getEngine();
	ShaderManager& shaderManager = engine.getResourceSystem().getShaderManager();

	const vk::ShaderModule& vertexShaderModule = shaderManager.get<VulkanShader>(_vertexShader).module();
	const vk::ShaderModule& fragmentShaderModule = shaderManager.get<VulkanShader>(_fragmentShader).module();

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertShaderStageInfo.module = vertexShaderModule;
	vertShaderStageInfo.pName = "main"; // Entry point in the shader

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragShaderStageInfo.module = fragmentShaderModule;
	fragShaderStageInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	// Vertex definitions
	SimpleRTTR::Type vertexType = SimpleRTTR::types().get_type<Vertex>().value();

	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = vk::VertexInputRate::eVertex;

	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
	attributeDescriptions.resize(vertexType.properties().size());

	int location = 0;
	for (const SimpleRTTR::Property& property : vertexType.properties())
	{
		const SimpleRTTR::Type& propertyType = property.type();
		const vk::Format& format = propertyType.meta().get("format").value().get_as<vk::Format>();

		vk::VertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;
		attributeDescription.location = location;
		attributeDescription.format = format;
		attributeDescription.offset = (uint32_t)property.offset();
		attributeDescriptions[location] = attributeDescription;
		location++;
	}

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	vk::Viewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_swapChainExtent.width);
	viewport.height = static_cast<float>(_swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor{};
	scissor.offset = vk::Offset2D{0, 0};
	scissor.extent = _swapChainExtent;

	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = vk::PolygonMode::eFill; // Fill polygons
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = VK_FALSE;

	vk::PipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
										  vk::ColorComponentFlagBits::eG |
										  vk::ColorComponentFlagBits::eB |
										  vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(glm::mat4);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	_pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;

	_pipeline = _device.createGraphicsPipeline(nullptr, pipelineInfo).value;
}

void VulkanSurface::initializeGameSurface(VulkanBufferFactory& bufferFactory)
{
	// Create the game image
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D{320, 200, 1}; // 320x200 resolution
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = vk::Format::eR8G8B8A8Unorm; // 8-bit color with alpha
	imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

	_gameImage = _device.createImage(imageInfo);

	vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(_gameImage);


	uint32_t memoryType = std::numeric_limits<uint32_t>::max();
	vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)
		{
			memoryType = i;
			break;
		}
	}

	if (memoryType == std::numeric_limits<uint32_t>::max())
	{
		throw std::runtime_error("Failed to find suitable memory type for game image!");
	}

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryType;

	_gameImageMemory = _device.allocateMemory(allocInfo);

	_device.bindImageMemory(_gameImage, _gameImageMemory, 0);

	vk::ImageViewCreateInfo imageViewInfo{};
	imageViewInfo.image = _gameImage;
	imageViewInfo.viewType = vk::ImageViewType::e2D;
	imageViewInfo.format = vk::Format::eR8G8B8A8Unorm;
	imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.layerCount = 1;

	_gameImageView = _device.createImageView(imageViewInfo);

	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // Index of the attachment in the render pass (color attachment)
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass{};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	_gameRenderPass = _device.createRenderPass(renderPassInfo);

	vk::FramebufferCreateInfo framebufferInfo{};
	framebufferInfo.renderPass = _gameRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &_gameImageView;
	framebufferInfo.width = 320;
	framebufferInfo.height = 200;
	framebufferInfo.layers = 1;

	_gameFramebuffer = _device.createFramebuffer(framebufferInfo);

	// create the texture sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.magFilter = vk::Filter::eNearest;                    // Nearest-neighbor filtering for magnification (no blending)
	samplerInfo.minFilter = vk::Filter::eNearest;                    // Nearest-neighbor filtering for minification (no blending)
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // Clamp edges, no tiling
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // Clamp edges, no tiling
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // Only relevant for 3D textures
	samplerInfo.anisotropyEnable = VK_FALSE;                         // Disable anisotropic filtering
	samplerInfo.maxAnisotropy = 1.0f;                                // No anisotropy
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;      // Border color if clamping beyond edge
	samplerInfo.unnormalizedCoordinates = VK_FALSE;                  // Use normalized coordinates (0 to 1)
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;        // No mipmap blending (uses base level only)
	samplerInfo.compareEnable = VK_FALSE;                            // No depth comparison
	samplerInfo.compareOp = vk::CompareOp::eAlways;                  // Not relevant since compare is disabled

	_textureSampler = _device.createSampler(samplerInfo);

	// Update descriptor set
	vk::DescriptorImageInfo descImageInfo{};

	descImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	descImageInfo.imageView = _gameImageView;
	descImageInfo.sampler = _textureSampler;

	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &descImageInfo;

	_device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

	//create the vertex and index buffer to present the game surface to the window surface
	//_vertexBuffer = std::make_unique<VulkanBuffer>(_allocator, _device, (sizeof(vertices) * sizeof(Vertex)), vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
	//_indexBuffer = std::make_unique<VulkanBuffer>(_allocator, _device, (sizeof(indices) * sizeof(uint16_t)), vk::BufferUsageFlagBits::eIndexBuffer, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	_vertexBuffer = bufferFactory.createDeviceBuffer<Vertex>(vertices, 4, vk::BufferUsageFlagBits::eVertexBuffer);
	_indexBuffer = bufferFactory.createDeviceBuffer<uint16_t>(indices, 6, vk::BufferUsageFlagBits::eIndexBuffer);
}

void VulkanSurface::destroySwapChain()
{
	_device.waitIdle();

	if (_pipelineLayout)
	{
		_device.destroyPipelineLayout(_pipelineLayout);
		_pipelineLayout = nullptr;
	}

	if(_pipeline)
	{
		_device.destroyPipeline(_pipeline);
		_pipeline = nullptr;
	}

	for (const FrameData& frame : _frames)
	{
		frame.commandBuffer.reset();
		_device.destroyImageView(frame.imageView);
		_device.destroyFramebuffer(frame.framebuffer);
		_device.destroySemaphore(frame.imageAvailableSemaphore);
		_device.destroySemaphore(frame.renderFinishedSemaphore);
		_device.destroyFence(frame.inFlightFence);
	}
	_frames.clear();

	//reset the command pool
	_device.resetCommandPool(_commandPool, vk::CommandPoolResetFlags());

	if (_swapChain)
	{
		_device.destroySwapchainKHR(_swapChain);
		_swapChain = nullptr;
	}
}

void VulkanSurface::handleResize()
{
	destroySwapChain();
	initializeSwapChain();
	initializeFrames(_renderPass);
	initializePipeline();

	_currentFrame = 0;
}

VulkanSurface::~VulkanSurface()
{
	destroySwapChain();

	if (_gameFramebuffer)
	{
		_device.destroyFramebuffer(_gameFramebuffer);
		_gameFramebuffer = nullptr;
	}

	if (_gameImageView)
	{
		_device.destroyImageView(_gameImageView);
		_gameImageView = nullptr;
	}

	if (_gameImage)
	{
		_device.destroyImage(_gameImage);
		_gameImage = nullptr;
	}

	if(_gameImageMemory)
	{
		_device.freeMemory(_gameImageMemory);
		_gameImageMemory = nullptr;
	}

	if (_gameRenderPass)
	{
		_device.destroyRenderPass(_gameRenderPass);
		_gameRenderPass = nullptr;
	}

	if (_descriptorPool)
	{
		_device.destroyDescriptorPool(_descriptorPool);
		_descriptorPool = nullptr;
	}

	if(_descriptorSetLayout)
	{
		_device.destroyDescriptorSetLayout(_descriptorSetLayout);
		_descriptorSetLayout = nullptr;
	}

	if (_textureSampler)
	{
		_device.destroySampler(_textureSampler);
		_textureSampler = nullptr;
	}

	if(_commandPool)
	{
		_device.destroyCommandPool(_commandPool);
		_commandPool = nullptr;
	}
	
	if (_surface)
	{
		_instance.destroySurfaceKHR(_surface);
		_surface = nullptr;
	}

	_device = nullptr;
}

void VulkanSurface::update()
{
	uint32_t imageIndex = 0;

	// Acquire an image from the swap chain
	vk::Result result = _device.acquireNextImageKHR(_swapChain, UINT64_MAX, _frames[_currentFrame].imageAvailableSemaphore, nullptr, &imageIndex);

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		// Handle window resize (recreate swap chain)
		handleResize();
		return;
	}
	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	result = _device.waitForFences(1, &_frames[_currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);
	result = _device.resetFences(1, &_frames[_currentFrame].inFlightFence);

	recordCommandBuffer(_frames[_currentFrame], imageIndex);

	vk::Semaphore waitSemaphores[] = {_frames[_currentFrame].imageAvailableSemaphore};
	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	vk::Semaphore signalSemaphores[] = {_frames[_currentFrame].renderFinishedSemaphore};

	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_frames[_currentFrame].commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	result = _graphicsQueue.submit(1, &submitInfo, _frames[_currentFrame].inFlightFence);

	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapChain;
	presentInfo.pImageIndices = &imageIndex;

	result = _presentQueue.presentKHR(&presentInfo);
	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
	{
		// Handle window resize (recreate swap chain)
		handleResize();
		return;
	}
	else if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present swap chain image!");
	}

	_currentFrame = (_currentFrame + 1) % _frames.size();
}

void VulkanSurface::recordCommandBuffer(FrameData& frame, uint32_t imageIndex)
{
	vk::CommandBuffer& commandBuffer = frame.commandBuffer;

	vk::CommandBufferBeginInfo beginInfo{};
	commandBuffer.begin(beginInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, _descriptorSet, {});

	// render the game surface
	{
		vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.5f, 0.5f, 0.5f, 1.0f}); // Gray

		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = _gameRenderPass;
		renderPassInfo.framebuffer = _gameFramebuffer;
		renderPassInfo.renderArea.extent = vk::Extent2D{320, 200};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		// ... any other draw commands for the game content

		commandBuffer.endRenderPass();
	}

	// render the window surface
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = _renderPass;
		renderPassInfo.framebuffer = frame.framebuffer;
		renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
		renderPassInfo.renderArea.extent = _swapChainExtent;

		vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	    // Bind vertex and index buffers
		vk::DeviceSize offsets[] = {0};
		commandBuffer.bindVertexBuffers(0, _vertexBuffer->getBuffer(), offsets);
		commandBuffer.bindIndexBuffer(_indexBuffer->getBuffer(), 0, vk::IndexType::eUint16);

		// Push up the window transformation matrix
		commandBuffer.pushConstants(_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &_transform);

		// Issue the draw call
		commandBuffer.drawIndexed(static_cast<uint32_t>(std::size(indices)), 1, 0, 0, 0);


		commandBuffer.endRenderPass();
	}

	commandBuffer.end();

}


} // namespace OpenXcom
