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

#include "../../../../Engine/Graphics/Primitive/BoxPrimitive.h"
#include "../../../../Engine/Graphics/Primitive/PrimitiveFactory.h"

#include <filesystem>
#include <memory>

using namespace OpenXcom;

class GraphicsBoxTest : public TestEngineSuite
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

TEST_F(GraphicsBoxTest, TestFilledBox)
{
	std::unique_ptr<BoxFilledPrimitive> boxOutline = _gameSurface->getRenderTarget().getPrimitiveFactory().createFilledBoxPrimitive({140, 20}, {40, 40}, 1, _paletteHandle.getHandle());
	ASSERT_TRUE(boxOutline);

	// draw the line
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&boxOutline](GraphicsCommand& command) {
		boxOutline->draw(command);
	});

	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "BoxPrimitive" / "005_game_surface_filled_box_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}

TEST_F(GraphicsBoxTest, TestOutlineBox)
{
	std::unique_ptr<BoxOutlinePrimitive> boxOutline = _gameSurface->getRenderTarget().getPrimitiveFactory().createOutlineBoxPrimitive({140, 70}, {40, 40}, 1, _paletteHandle.getHandle());
	ASSERT_TRUE(boxOutline);

	// draw the line
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&boxOutline](GraphicsCommand& command) {
		boxOutline->draw(command);
	});

	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "BoxPrimitive" / "006_game_surface_outline_box_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}

TEST_F(GraphicsBoxTest, TestOutlineBoxesCorners)
{
	std::unique_ptr<BoxOutlinePrimitive> boxOutline[4];
	boxOutline[0] = _gameSurface->getRenderTarget().getPrimitiveFactory().createOutlineBoxPrimitive({0, 0}, {40, 40}, 1, _paletteHandle.getHandle());
	boxOutline[1] = _gameSurface->getRenderTarget().getPrimitiveFactory().createOutlineBoxPrimitive({279, 0}, {40, 40}, 1, _paletteHandle.getHandle());
	boxOutline[2] = _gameSurface->getRenderTarget().getPrimitiveFactory().createOutlineBoxPrimitive({0, 159}, {40, 40}, 1, _paletteHandle.getHandle());
	boxOutline[3] = _gameSurface->getRenderTarget().getPrimitiveFactory().createOutlineBoxPrimitive({279, 159}, {40, 40}, 1, _paletteHandle.getHandle());
	ASSERT_TRUE(boxOutline[0]);
	ASSERT_TRUE(boxOutline[1]);
	ASSERT_TRUE(boxOutline[2]);
	ASSERT_TRUE(boxOutline[3]);

	// draw the line
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&boxOutline](GraphicsCommand& command) {
		boxOutline[0]->draw(command);
		boxOutline[1]->draw(command);
		boxOutline[2]->draw(command);
		boxOutline[3]->draw(command);
	});

	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "BoxPrimitive" / "007_game_surface_outline_box_corners_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}
