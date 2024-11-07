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
#include "../../Platform/Window.h"
#include "../../Engine.h"
#include "../../Resource/ResourceSystem.h"
#include "../../Resource/Shader/ShaderManager.h"
#include "../../Logger.h"

// BEGIN TEMP
#include <shaderc/shaderc.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <limits>

namespace OpenXcom
{

const char* vertexShaderSource = R"(
    #version 450
    layout(location = 0) in vec2 inPosition;
    layout(location = 1) in vec3 inColor;
    layout(location = 0) out vec3 fragColor;
    void main() {
        fragColor = inColor;
        gl_Position = vec4(inPosition, 0.0, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 450
    layout(location = 0) in vec3 fragColor;
    layout(location = 0) out vec4 outColor;
    void main() {
        outColor = vec4(fragColor, 1.0);
    }
)";

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;
};

Vertex vertices[] = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // Bottom-left, red
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  // Bottom-right, green
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},   // Top-right, blue
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}   // Top-left, white
};

// Indices for two triangles forming a rectangle
uint16_t indices[] = {0, 1, 2, 2, 3, 0};


std::vector<uint32_t> compileGLSL(shaderc::Compiler& compiler, const std::string& source, shaderc_shader_kind kind)
{
	shaderc::CompileOptions options;
	options.SetOptimizationLevel(shaderc_optimization_level_size);

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, "shader", options);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		Log(LOG_ERROR) << "Failed to compile GLSL shader: " << result.GetErrorMessage();
		return {};
	}
	return {result.cbegin(), result.cend()};
}

vk::ShaderModule createShaderModule(shaderc::Compiler& compiler, const std::string& source, shaderc_shader_kind kind, vk::Device device)
{
	std::vector<uint32_t> spirvCode = compileGLSL(compiler, source, kind);

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
	createInfo.pCode = spirvCode.data();

	return device.createShaderModule(createInfo);
}

} // namespace OpenXcom
// END TEMP

namespace OpenXcom
{

VulkanSurface::VulkanSurface(vk::Instance instance, const PlatformWindowHandle& window)
	: _instance(instance), _surface(nullptr), _device(nullptr), _commandPool(nullptr),
	  _swapChain(nullptr), _swapChainImageFormat(vk::Format::eUndefined), _swapChainExtent{}, _frames(),
	  _renderPass(nullptr), _pipelineLayout(nullptr), _pipeline(nullptr), _currentFrame(0), _windowHandle(window),
	  _vertexShaderModule(nullptr), _fragmentShaderModule(nullptr)
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

void VulkanSurface::initializeDevice(vk::Device& device, const vk::PhysicalDevice& physicalDevice, uint32_t graphicsQueueFamilyIndex, vk::Queue& graphicsQueue, vk::Queue& presentQueue)
{
	_device = device;
	_graphicsQueue = graphicsQueue;
	_presentQueue = presentQueue;
	_physicalDevice = physicalDevice;

	// Create the command queue
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	_commandPool = _device.createCommandPool(poolInfo);

	initializeGameSurface();
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

void VulkanSurface::initializeShaders(shaderc::Compiler& compiler)
{
	_vertexShaderModule = createShaderModule(compiler, vertexShaderSource, shaderc_glsl_vertex_shader, _device);
	_fragmentShaderModule = createShaderModule(compiler, fragmentShaderSource, shaderc_glsl_fragment_shader, _device);

	Engine& engine = getEngine();
	ShaderManager& shaderManager = engine.getResourceSystem().getShaderManager();
	_vertexShader = shaderManager.loadShader("WindowSurfaceVertex", vertexShaderSource, ShaderType::Vertex);
	_fragmentShader = shaderManager.loadShader("WindowSurfaceFragment", fragmentShaderSource, ShaderType::Fragment);
}

void VulkanSurface::initializePipeline()
{
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertShaderStageInfo.module = _vertexShaderModule;
	vertShaderStageInfo.pName = "main"; // Entry point in the shader

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragShaderStageInfo.module = _fragmentShaderModule;
	fragShaderStageInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = vk::VertexInputRate::eVertex;

	std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32Sfloat; // vec2 for position
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat; // vec3 for color
	attributeDescriptions[1].offset = offsetof(Vertex, color);

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

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
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

void VulkanSurface::initializeGameSurface()
{
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

	vk::DeviceMemory gameImageMemory = _device.allocateMemory(allocInfo);

	_device.bindImageMemory(_gameImage, gameImageMemory, 0);

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

	if (_gameRenderPass)
	{
		_device.destroyRenderPass(_gameRenderPass);
		_gameRenderPass = nullptr;
	}

	if(_commandPool)
	{
		_device.destroyCommandPool(_commandPool);
		_commandPool = nullptr;
	}
	
	if (_vertexShaderModule)
	{
		_device.destroyShaderModule(_vertexShaderModule);
		_vertexShaderModule = nullptr;
	}

	if (_fragmentShaderModule)
	{
		_device.destroyShaderModule(_fragmentShaderModule);
		_fragmentShaderModule = nullptr;
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

	vk::CommandBufferBeginInfo beginInfo{};
	frame.commandBuffer.begin(beginInfo);

	// render the game surface
	{
		vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.5f, 0.5f, 0.5f, 1.0f}); // Gray

		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = _gameRenderPass;
		renderPassInfo.framebuffer = _gameFramebuffer;
		renderPassInfo.renderArea.extent = vk::Extent2D{320, 200};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		frame.commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		// ... any other draw commands for the game content

		frame.commandBuffer.endRenderPass();
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

		frame.commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);


		// !!!! TODO !!!!


		frame.commandBuffer.endRenderPass();
	}

	frame.commandBuffer.end();

}


} // namespace OpenXcom
