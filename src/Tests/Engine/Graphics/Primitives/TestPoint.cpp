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
#include "../../_TestEngine.h"

#include "../../../../Engine/Graphics/Primitive/PointPrimitive.h"
#include "../../../../Engine/Graphics/Primitive/PrimitiveFactory.h"

#include <filesystem>
#include <memory>

using namespace OpenXcom;

class GraphicsPointTest : public TestEngineSuite
{
protected:
	OwningHandle<Palette> _paletteHandle;

	void SetUp() override
	{
		_paletteHandle = create16ColorPalette();
	}

	void TearDown() override
	{
		_paletteHandle.reset();
	}
};

TEST_F(GraphicsPointTest, TestPointListPrimitive16Points)
{
	// Create 16 points arranged in 2 rows (8 columns per row).
	std::vector<PointVertex> points;
	for (int row = 0; row < 2; ++row)
	{
		for (int col = 0; col < 8; ++col)
		{
			// Starting at (10,10) with 20 pixels spacing horizontally and vertically.
			PointVertex vertex = {glm::ivec2(10 + col * 20, 10 + row * 20)};
			points.push_back(vertex);
		}
	}

	// Create the PointListPrimitive using the primitive factory.
	std::unique_ptr<PointListPrimitive> pointListPrimitive =
		_gameSurface->getRenderTarget().getPrimitiveFactory().createPointListPrimitive(points.data(), points.size(), 1, _paletteHandle.getHandle());
	ASSERT_TRUE(pointListPrimitive);

	// Render the point list primitive.
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&pointListPrimitive](GraphicsCommand& command) {
		pointListPrimitive->draw(command);
	});

	// Capture the game surface after drawing.
	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	// Define the baseline image path for comparison.
	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "PointPrimitive" / "008_game_surface_point_list_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}

TEST_F(GraphicsPointTest, TestPointColorListPrimitive16Points)
{
	// Create 16 vertices arranged in 2 rows (8 columns per row),
	// cycling through the 16 palette colors.
	std::vector<PointColorVertex> vertices;
	for (int row = 0; row < 2; ++row)
	{
		for (int col = 0; col < 8; ++col)
		{
			// Calculate the overall index and cycle through 16 colors.
			int index = row * 8 + col;
			uint32_t color = index % 16; // Cycle through colors 0-15

			// Create a vertex starting at (10,10) with 20 pixels spacing.
			PointColorVertex vertex = {glm::ivec2(10 + col * 20, 10 + row * 20), color};
			vertices.push_back(vertex);
		}
	}

	// Create the PointColorListPrimitive using the primitive factory.
	std::unique_ptr<PointColorListPrimitive> pointColorListPrimitive =
		_gameSurface->getRenderTarget().getPrimitiveFactory().createPointColorListPrimitive(vertices.data(), vertices.size(), _paletteHandle.getHandle());
	ASSERT_TRUE(pointColorListPrimitive);

	// Render the point color list primitive.
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&pointColorListPrimitive](GraphicsCommand& command) {
		pointColorListPrimitive->draw(command);
	});

	// Capture the game surface after drawing.
	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	// Define the baseline image path for comparison.
	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "PointPrimitive" / "009_game_surface_point_color_list_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}
