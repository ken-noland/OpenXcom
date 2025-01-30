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
#include "../../../Engine/Options.h"
#include "../../../Engine/Resource/ResourceSystem.h"
#include "../../../Engine/Resource/ResourceManager.h"
#include "../../../Engine/Resource/Image/Image.h"
#include "../../../Engine/Resource/Image/ImageManager.h"
#include "../../../Engine/Graphics/GraphicsSystem.h"
#include "../../../Engine/Graphics/GraphicsSurface.h"
#include "../../../Engine/Graphics/Common/GameSurface.h"
#include "../../../Engine/Graphics/Common/WindowSurface.h"
#include "../../../Engine/Platform/Window.h"

#include <lodepng.h>

// Anonymouse namespace to avoid name collisions in the global
namespace
{
	bool FORCE_REGENERATE_BASELINE = false;
}

using namespace OpenXcom;

TEST(TestGraphics, TestGraphicsInitialization)
{
	Options options;

	options.set<&GraphicsOptions::_headless>(OptionLevel::CONFIG, true);
	std::unique_ptr<GraphicsSystem> graphicsSystem = createGraphicsSystem(options);
}

TEST(TestGraphics, TestGraphicsSurface)
{
	std::filesystem::path path = TEST_DATA_DIR;
	std::filesystem::path dataPath = path / "Data";
	std::filesystem::path configPath = path / "Config";
	std::filesystem::path userPath = path / "User";

	std::vector<std::string> args = {"-data", dataPath.string(), "-config", configPath.string(), "-user", userPath.string(), "-headless"};
	Engine engine(args);

	GameSurface gameSurface(engine.getEngineContext());
	WindowSurface windowSurface(engine.getEngineContext(), gameSurface);

	//render once
	windowSurface.update();

	std::unique_ptr<HostImage> hostImage = engine.getResourceSystem().getImageManager().createHostImage(gameSurface.getScreenSize(), ImageFormat::RGBA8);
	EXPECT_TRUE(hostImage);

	gameSurface.captureFrame(*hostImage);

	//with the game surface rendered, let's check against the expected results
	const uint8_t* pixels = static_cast<const uint8_t*>(hostImage->map());
		
	// If baseline file is missing, generate and save it
	std::filesystem::path filename = dataPath / "Test" / "Graphics" / "game_surface_blank.png";
	if (!std::filesystem::exists(filename) || FORCE_REGENERATE_BASELINE)
	{
		// Save the image
		unsigned error = lodepng::encode(filename.string().c_str(), pixels, hostImage->getWidth(), hostImage->getHeight(), LodePNGColorType::LCT_RGBA, 8);
		EXPECT_EQ(error, 0);
	}
	else
	{
		// Load the baseline image
		std::vector<unsigned char> baseline;
		uint32_t width = 0, height = 0;

		unsigned error = lodepng::decode(baseline, width, height, filename.string().c_str());
		EXPECT_EQ(error, 0);

		// Compare the images
		ASSERT_EQ(width, hostImage->getExtent().x);
		ASSERT_EQ(height, hostImage->getExtent().y);

		for (size_t i = 0; i < baseline.size(); ++i)
		{
			ASSERT_EQ(baseline[i], pixels[i]);
		}
	}

	hostImage->unmap();
}
