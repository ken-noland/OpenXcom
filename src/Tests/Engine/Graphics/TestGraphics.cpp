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
#include <gtest/gtest.h>

#include "../../../Engine/Engine.h"
#include "../../../Engine/Graphics/Common/GameSurface.h"
#include "../../../Engine/Graphics/Common/WindowSurface.h"
#include "../../../Engine/Graphics/Primitive/LinePrimitive.h"
#include "../../../Engine/Graphics/Primitive/PrimitiveFactory.h"
#include "../../../Engine/Graphics/GraphicsSurface.h"
#include "../../../Engine/Graphics/GraphicsSystem.h"
#include "../../../Engine/Graphics/BufferManager.h"
#include "../../../Engine/Options.h"
#include "../../../Engine/Platform/Window.h"

#include "../../../Engine/Resource/ResourceManager.h"
#include "../../../Engine/Resource/ResourceSystem.h"
#include "../../../Engine/Graphics/Image/Image.h"
#include "../../../Engine/Graphics/Image/ImageManager.h"
#include <filesystem>
#include <lodepng.h>
#include <memory>
#include <vector>

namespace
{
bool FORCE_REGENERATE_BASELINE = false;
}

using namespace OpenXcom;

class GraphicsTest : public ::testing::Test
{
protected:
	std::unique_ptr<Engine> _engine;
	std::unique_ptr<GameSurface> _gameSurface;
	std::unique_ptr<WindowSurface> _windowSurface;

	std::filesystem::path _dataPath;
	std::filesystem::path _configPath;
	std::filesystem::path _userPath;

	void SetUp() override
	{
		std::filesystem::path path = TEST_DATA_DIR;
		_dataPath = path / "Data";
		_configPath = path / "Config";
		_userPath = path / "User";

		std::vector<std::string> args = {"-data", _dataPath.string(), "-config", _configPath.string(), "-user", _userPath.string(), "-headless"};
		_engine = std::make_unique<Engine>(args);

		_gameSurface = std::make_unique<GameSurface>(_engine->getEngineContext());
		_windowSurface = std::make_unique<WindowSurface>(_engine->getEngineContext(), *_gameSurface);
	}

	void TearDown() override
	{
		_windowSurface.reset();
		_gameSurface.reset();
		_engine.reset();
	}

	/**
	 * Captures the game surface into a HostImage.
	 */
	std::unique_ptr<HostImage> captureGameSurface()
	{
		_windowSurface->update();	// render the surface once

		std::unique_ptr<HostImage> hostImage = _engine->getResourceSystem().getImageManager().createHostImage(_gameSurface->getScreenSize(), ImageFormat::RGBA8);
		if (!hostImage)
		{
			ADD_FAILURE() << "Failed to create HostImage.";
			return nullptr;
		}

		_gameSurface->captureFrame(*hostImage);
		return hostImage;
	}

	/**
	 * Compares a captured image with a baseline.
	 */
	void compareWithBaseline(const std::unique_ptr<HostImage>& hostImage, const std::filesystem::path& baselinePath)
	{
		const uint8_t* pixels = static_cast<const uint8_t*>(hostImage->map());

		// Generate a baseline if it doesn't exist
		if (!std::filesystem::exists(baselinePath) || FORCE_REGENERATE_BASELINE)
		{
			unsigned error = lodepng::encode(baselinePath.string().c_str(), pixels, hostImage->getWidth(), hostImage->getHeight(), LodePNGColorType::LCT_RGBA, 8);
			EXPECT_EQ(error, 0) << "Failed to save baseline image.";
		}
		else
		{
			// Load the baseline image
			std::vector<unsigned char> baseline;
			uint32_t width = 0, height = 0;

			unsigned error = lodepng::decode(baseline, width, height, baselinePath.string().c_str());
			EXPECT_EQ(error, 0) << "Failed to load baseline image.";

			// Compare dimensions
			ASSERT_EQ(width, hostImage->getExtent().x) << "Image width mismatch.";
			ASSERT_EQ(height, hostImage->getExtent().y) << "Image height mismatch.";

			// Compare pixel data
			ASSERT_EQ(baseline.size(), hostImage->getExtent().x * hostImage->getExtent().y * 4) << "Image data size mismatch.";
			for (size_t i = 0; i < baseline.size(); ++i)
			{
				ASSERT_EQ(baseline[i], pixels[i]) << "Pixel mismatch at index " << i;
			}
		}

		hostImage->unmap();
	}
};

TEST_F(GraphicsTest, TestGraphicsSurface)
{
	std::unique_ptr<HostImage> hostImage = captureGameSurface();
	ASSERT_TRUE(hostImage);

	std::filesystem::path baselinePath = _dataPath / "Test" / "Graphics" / "game_surface_blank.png";
	compareWithBaseline(hostImage, baselinePath);
}

TEST_F(GraphicsTest, TestLineList)
{
	//TODO: Temp palette buffer(we need to finish the palette system)
	const uint32_t paletteData[] = {0x00000000, 0xFFFFFFFF};
	std::unique_ptr<DeviceBuffer> palette = _engine->getResourceSystem().getBufferManager().createDeviceBuffer<uint32_t>(paletteData, 2, BufferUsage::Storage);

	LineVertex lines[] = {{{0, 0}}, {{320, 200}}};
	std::unique_ptr<LineListPrimitive> lineList = _gameSurface->getRenderTarget().getPrimitiveFactory().createLineListPrimitive(lines, 2, 1, *palette);
	ASSERT_TRUE(lineList);

	// draw the line
	_gameSurface->onRender() << [&lineList](GraphicsCommand& command) {
		lineList->draw(command);
	};

	std::unique_ptr<HostImage> hostImage = captureGameSurface();
	ASSERT_TRUE(hostImage);

	std::filesystem::path baselinePath = _dataPath / "Test" / "Graphics" / "game_surface_line_1.png";
	compareWithBaseline(hostImage, baselinePath);
}
