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

#include "../../../../Engine/Graphics/Primitive/ImagePrimitive.h"
#include "../../../../Engine/Graphics/Primitive/PrimitiveFactory.h"

#include "../../../../Engine/Graphics/Buffer/BufferManager.h"

#include <filesystem>
#include <memory>

using namespace OpenXcom;

class GraphicsImageTest : public TestEngineSuite
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

TEST_F(GraphicsImageTest, TestBasicImage)
{
	ImagePaletteFile imageFile = createImage();

	ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();
	ImageManager& imageManager = resourceSystem.getImageManager();

	//transfer the image to device
	OwningHandle<DeviceImage> deviceImage = imageManager.createDeviceImage(*imageFile.image);

	std::unique_ptr<ImagePrimitive> image = _gameSurface->getRenderTarget().getPrimitiveFactory().createImagePrimitive({0, 0}, {0, 0}, {32, 32}, deviceImage.getHandle(), imageFile.palette.getHandle());
	ASSERT_TRUE(image);

	// draw the line
	MulticastDelegate<void(GraphicsCommand&)>::Handle onRender = _gameSurface->onRender().add([&image](GraphicsCommand& command) {
		image->draw(command);
	});

	OwningHandle<HostImage> hostImageHandle = captureGameSurface();
	ASSERT_TRUE(hostImageHandle.isValid());

	std::filesystem::path baselinePath = _dataPath / "generated" / "Graphics" / "ImagePrimitive" / "010_game_surface_image_1.png";
	compareWithBaseline(hostImageHandle, baselinePath);
}
