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

#include "../../../../Engine/Graphics/Primitive/LinePrimitive.h"
#include "../../../../Engine/Graphics/Primitive/PrimitiveFactory.h"

#include <filesystem>
#include <memory>

using namespace OpenXcom;

class GraphicsLineTest : public TestEngineSuite
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

TEST_F(GraphicsLineTest, TestLineList)
{
	LineVertex lines[] = {{{0, 0}}, {{320, 200}}};
	std::unique_ptr<LineListPrimitive> lineList = _gameSurface->getRenderTarget().getPrimitiveFactory().createLineListPrimitive(lines, 2, 1, _paletteHandle.getHandle());
	ASSERT_TRUE(lineList);

	// draw the line
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&lineList](GraphicsCommand& command) {
		lineList->draw(command);
	});

	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "LinePrimitive" / "002_game_surface_line_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}

TEST_F(GraphicsLineTest, TestLineListMultiple)
{
	// create multiple vertical lines
	LineVertex lines[] = {{{10, 10}}, {{10, 190}}, {{20, 10}}, {{20, 190}}, {{30, 10}}, {{30, 190}}};
	std::unique_ptr<LineListPrimitive> lineList = _gameSurface->getRenderTarget().getPrimitiveFactory().createLineListPrimitive(lines, 6, 1, _paletteHandle.getHandle());
	ASSERT_TRUE(lineList);

	// draw the line
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&lineList](GraphicsCommand& command) {
		lineList->draw(command);
	});

	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "LinePrimitive" / "003_game_surface_line_multiple_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}

TEST_F(GraphicsLineTest, TestLineListResize)
{
	// create multiple vertical lines
	LineVertex lines[] = {{{10, 10}}, {{10, 190}}, {{20, 10}}, {{20, 190}}, {{30, 10}}, {{30, 190}}, {{40, 10}}, {{40, 190}}};
	std::unique_ptr<LineListPrimitive> lineList = _gameSurface->getRenderTarget().getPrimitiveFactory().createLineListPrimitive(lines, 6, 1, _paletteHandle.getHandle());
	ASSERT_TRUE(lineList);

	// draw the line
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&lineList](GraphicsCommand& command) {
		lineList->draw(command);
	});

	OwningHandle<HostImage> hostImageHandle;
	hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	compareWithBaseline(hostImageHandle, _dataPath / "generated" / "Graphics" / "LinePrimitive" / "003_game_surface_line_multiple_1.png");

	lineList->setLines(lines, 4);

	hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	compareWithBaseline(hostImageHandle, _dataPath / "generated" / "Graphics" / "LinePrimitive" / "003_game_surface_line_multiple_2.png");

	lineList->setLines(lines, 8);

	hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	compareWithBaseline(hostImageHandle, _dataPath / "generated" / "Graphics" / "LinePrimitive" / "003_game_surface_line_multiple_3.png");
}

TEST_F(GraphicsLineTest, TestLineStrip)
{
	LineVertex lines[] = {
		{{10, 10}},
		{{10, 190}},
		{{310, 190}},
		{{310, 10}},
		{{10, 10}}};

	std::unique_ptr<LineStripPrimitive> lineStrip = _gameSurface->getRenderTarget().getPrimitiveFactory().createLineStripPrimitive(lines, 5, 1, _paletteHandle.getHandle());
	ASSERT_TRUE(lineStrip);

	// draw the line
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&lineStrip](GraphicsCommand& command) {
		lineStrip->draw(command);
	});

	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "LinePrimitive" / "004_game_surface_line_strip_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}
