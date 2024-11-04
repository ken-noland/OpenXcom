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

namespace OpenXcom
{

VulkanSurface::VulkanSurface(vk::Instance instance, const PlatformWindowHandle& window)
{
	_instance = instance;

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
	XGetGeometry(handle.display, handle.window, &root, &x, &y, &width, &height, &borderWidth, &depth);

	return vk::Extent2D{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)};
#else
	#error "Unsupported platform"
#endif
}

void VulkanSurface::initializeSwapChain(const PlatformWindowHandle& handle, const vk::PhysicalDevice& physicalDevice, vk::Device* device)
{
	_device = device;

	vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(_surface);
	std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(_surface);
	std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(_surface);

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
		swapChainExtent = getClientAreaSize(handle); // Replace with actual window size
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

	_swapChain = _device->createSwapchainKHR(swapChainCreateInfo);
	_swapChainImageFormat = chosenFormat.format;
	_swapChainExtent = swapChainExtent;
}

VulkanSurface::~VulkanSurface()
{
	if (_swapChain)
	{
		_device->destroySwapchainKHR(_swapChain);
		_swapChain = nullptr;
	}

	if (_surface)
	{
		_instance.destroySurfaceKHR(_surface);
		_surface = nullptr;
	}
}



} // namespace OpenXcom
