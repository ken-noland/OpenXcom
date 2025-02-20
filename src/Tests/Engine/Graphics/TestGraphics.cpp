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
#include "../../../Engine/Graphics/Image/Image.h"
#include "../../../Engine/Graphics/Image/ImageManager.h"
#include "../../../Engine/Graphics/Palette/PaletteManager.h"
#include "../../../Engine/Graphics/Types/PackedColor.h"

#include "../../../Engine/Resource/ResourceManager.h"
#include "../../../Engine/Resource/ResourceSystem.h"
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
	static std::unique_ptr<Engine> _engine;

	static std::filesystem::path _dataPath;
	static std::filesystem::path _configPath;
	static std::filesystem::path _userPath;

	std::unique_ptr<GameSurface> _gameSurface;
	std::unique_ptr<WindowSurface> _windowSurface;

	OwningHandle<Palette> _paletteHandle;

	static void SetUpTestSuite()
	{
		std::filesystem::path path = TEST_DATA_DIR;
		_dataPath = path / "Data";
		_configPath = path / "Config";
		_userPath = path / "User";

		std::vector<std::string> args = {"-data", _dataPath.string(), "-config", _configPath.string(), "-user", _userPath.string(), "-headless"};
		_engine = std::make_unique<Engine>(args);
	}

	static void TearDownTestSuite()
	{
		_engine.reset();
	}

	void SetUp() override
	{
		// set up the palette
		PackedColor paletteData[] = {
			0x000000FF, // 0 - Black
			0xFFFFFFFF, // 1 - White
			0x808080FF, // 2 - Gray
			0xFF0000FF, // 3 - Red
			0x00FF00FF, // 4 - Green
			0x0000FFFF, // 5 - Blue
			0xFFFF00FF, // 6 - Yellow
			0xFF00FFFF, // 7 - Magenta
			0x00FFFFFF, // 8 - Cyan
			0xFFA500FF, // 9 - Orange
			0x8A2BE2FF, // 10 - Blue Violet
			0x008080FF, // 11 - Teal
			0x4B0082FF, // 12 - Indigo
			0x800000FF, // 13 - Maroon
			0x808000FF, // 14 - Olive
			0x8B4513FF  // 15 - Saddle Brown
		};

		PaletteManager& paletteManager = _engine->getEngineContext().getResourceSystem().getPaletteManager();

		_paletteHandle = paletteManager.createPalette("16colors", paletteData, 16);

		_gameSurface = std::make_unique<GameSurface>(_engine->getEngineContext());
		_windowSurface = std::make_unique<WindowSurface>(_engine->getEngineContext(), *_gameSurface);
	}

	void TearDown() override
	{
		_paletteHandle.reset();
		_windowSurface.reset();
		_gameSurface.reset();
	}

	/**
	 * Captures the game surface into a HostImage.
	 */
	OwningHandle<HostImage> captureGameSurface()
	{
		_windowSurface->update();	// render the surface once

		OwningHandle<HostImage> hostImageHandle = _engine->getResourceSystem().getImageManager().createHostImage("screenCapture", _gameSurface->getScreenSize(), ImageFormat::R8G8B8A8);
		if (!hostImageHandle.isValid())
		{
			ADD_FAILURE() << "Failed to create HostImage.";
			return OwningHandle<HostImage>();
		}

		_gameSurface->captureFrame(hostImageHandle.get());
		return hostImageHandle;
	}

	/**
	 * Compares a captured image with a baseline.
	 */
	void compareWithBaseline(const OwningHandle<HostImage>& hostImageHandle, const std::filesystem::path& baselinePath)
	{
		HostImage& hostImage = hostImageHandle.get();
		const uint8_t* pixels = static_cast<const uint8_t*>(hostImage.map());

		// recursively generate the path
		if (!std::filesystem::exists(baselinePath.parent_path()))
		{
			std::filesystem::create_directories(baselinePath.parent_path());
		}

		// Generate a baseline if it doesn't exist
		if (!std::filesystem::exists(baselinePath) || FORCE_REGENERATE_BASELINE)
		{
			unsigned error = lodepng::encode(baselinePath.string().c_str(), pixels, hostImage.getWidth(), hostImage.getHeight(), LodePNGColorType::LCT_RGBA, 8);
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
			ASSERT_EQ(width, hostImage.getExtent().x) << "Image width mismatch.";
			ASSERT_EQ(height, hostImage.getExtent().y) << "Image height mismatch.";

			// Compare pixel data
			ASSERT_EQ(baseline.size(), hostImage.getExtent().x * hostImage.getExtent().y * 4) << "Image data size mismatch.";
			for (size_t i = 0; i < baseline.size(); ++i)
			{
				std::size_t x = (i / 4) % hostImage.getExtent().x;
				std::size_t y = (i/4) / hostImage.getExtent().x;
				ASSERT_EQ(baseline[i], pixels[i]) << "Pixel mismatch at index " << i << "(x=" << x << " y=" << y << ")";
			}
		}

		hostImage.unmap();
	}
};

std::unique_ptr<Engine> GraphicsTest::_engine = nullptr;

std::filesystem::path GraphicsTest::_dataPath;
std::filesystem::path GraphicsTest::_configPath;
std::filesystem::path GraphicsTest::_userPath;


TEST_F(GraphicsTest, TestGraphicsSurface)
{
	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "Surface" / "001_game_surface_blank.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}
