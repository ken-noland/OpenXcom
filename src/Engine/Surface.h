#pragma once
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
#include <string>
#include <vector>
#include <memory>
#include <vector>
#include <assert.h>
#include <entt/entt.hpp>

#include "GraphSubset.h"

#include "../Entity/Engine/PaletteHandle.h"

namespace OpenXcom
{

class Font;
class Language;
class ScriptWorkerBase;
class SurfaceCrop;
template<typename Pixel> class SurfaceRaw;

// SDLHACK
class SDL_Surface{};
class SDL_Rect{};
class SDL_Color{};

/**
 * Element that is blit (rendered) onto the screen.
 * Mainly an encapsulation for SDL's SDL_Surface struct, so it
 * borrows a lot of its terminology. Takes care of all the common
 * rendering tasks and color effects, while serving as the base
 * class for more specialized screen elements.
 */
class Surface
{
public:
	struct UniqueBufferDeleter
	{
		void operator()(uint8_t*);
	};
	struct UniqueSurfaceDeleter
	{
		void operator()(SDL_Surface*);
	};

	using UniqueBufferPtr = std::unique_ptr<uint8_t, UniqueBufferDeleter>;
	using UniqueSurfacePtr = std::unique_ptr<SDL_Surface, UniqueSurfaceDeleter>;

	/// Create aligned buffer for surface.
	static UniqueBufferPtr NewAlignedBuffer(int bpp, int width, int height);
	/// Smart pointer for for SDL_Surface.
	static UniqueSurfacePtr NewSdlSurface(SDL_Surface* surface);
	/// Create surface from aligned buffer.
	static UniqueSurfacePtr NewSdlSurface(const UniqueBufferPtr& buffer, int bpp, int width, int height);
	/// Create buffer and surface.
	static std::pair<UniqueBufferPtr, UniqueSurfacePtr> NewPair32Bit(int width, int height)
	{
		auto tempBuffer = Surface::NewAlignedBuffer(32, width, height);
		auto tempSurface = Surface::NewSdlSurface(tempBuffer, 32, width, height);
		return std::make_pair(std::move(tempBuffer), std::move(tempSurface));
	}

	/// Create buffer and surface.
	static std::pair<UniqueBufferPtr, UniqueSurfacePtr> NewPair8Bit(int width, int height)
	{
		auto tempBuffer = Surface::NewAlignedBuffer(8, width, height);
		auto tempSurface = Surface::NewSdlSurface(tempBuffer, 8, width, height);
		return std::make_pair(std::move(tempBuffer), std::move(tempSurface));
	}

	/// Zero whole surface.
	static void CleanSdlSurface(SDL_Surface* surface);

protected:
	UniqueBufferPtr _alignedBuffer;
	UniqueSurfacePtr _surface;
	int16_t _x, _y;
	int16_t _width, _height, _pitch;
	uint8_t _visible : 1;
	uint8_t _hidden : 1;
	uint8_t _redraw : 1;

	/// Copies raw pixels.
	template <typename T>
	void rawCopy(const std::vector<T> &bytes);
	/// Resizes the surface.
	void resize(int width, int height);
public:
	/// Default empty surface.
	Surface();
	/// Creates a new surface with the specified size and position.
	Surface(int width, int height, int x = 0, int y = 0);
	/// Creates a new surface from an existing one.
	Surface(const Surface& other);
	/// Move surface to another place.
	Surface(Surface&& other) = default;
	/// Move assignment
	Surface& operator=(Surface&& other) = default;
	/// Copy assignment
	Surface& operator=(const Surface& other) { *this = Surface(other); return *this; };
	/// Cleans up the surface.
	virtual ~Surface();

	/// Is surface empty?
	explicit operator bool() const
	{
		return _alignedBuffer.get();
	}

	/// Loads a raw pixel array.
	void loadRaw(const std::vector<unsigned char> &bytes);
	/// Loads a raw pixel array.
	void loadRaw(const std::vector<char> &bytes);
	/// Loads an X-Com SCR graphic.
	void loadScr(const std::string &filename);
	/// Loads an X-Com SPK graphic.
	void loadSpk(const std::string &filename);
	/// Loads a TFTD BDY graphic.
	void loadBdy(const std::string &filename);
	/// Loads a general image file.
	void loadImage(const std::string &filename);
	/// Clears the surface's contents with a specified colour.
	void clear();
	/// Offsets the surface's colors by a set amount.
	void offset(int off, int min = -1, int max = -1, int mul = 1);
	/// Offsets the surface's colors in a color block.
	void offsetBlock(int off, int blk = 16, int mul = 1);
	/// Inverts the surface's colors.
	void invert(uint8_t mid);
	/// Runs surface functionality every cycle
	virtual void think();
	/// Draws the surface's graphic.
	virtual void draw();
	/// Blits this surface onto another one.
	virtual void blit(SDL_Surface *surface);
	/// Initializes the surface's various text resources.
	virtual void initText(Font *, Font *, Language *) {};
	/// Copies a portion of another surface into this one.
	void copy(Surface *surface);
	/// Draws a filled rectangle on the surface.
	void drawRect(SDL_Rect *rect, uint8_t color);
	/// Draws a filled rectangle on the surface.
	void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
	/// Draws a line on the surface.
	void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color);
	/// Draws a filled circle on the surface.
	void drawCircle(int16_t x, int16_t y, int16_t r, uint8_t color);
	/// Draws a filled polygon on the surface.
	void drawPolygon(int16_t *x, int16_t *y, int n, uint8_t color);
	/// Draws a textured polygon on the surface.
	void drawTexturedPolygon(int16_t *x, int16_t *y, int n, Surface *texture, int dx, int dy);
	/// Draws a string on the surface.
	void drawString(int16_t x, int16_t y, const char *s, uint8_t color);
	/// Sets the surface's palette.
	[[deprecated("Use palette system for setting palette")]] virtual void setPalette(const SDL_Color *colors, int firstcolor = 0, int ncolors = 256);

	/**
	 * Returns the surface's 8bpp palette.
	 * @return Pointer to the palette's colors.
	 */
	SDL_Color *getPalette() const
	{
		return nullptr;//_surface->format->palette->colors;
	}
	/// Sets the X position of the surface.
	virtual void setX(int x);
	/**
	 * Returns the position of the surface in the X axis.
	 * @return X position in pixels.
	 */
	int getX() const
	{
		return _x;
	}
	/// Sets the Y position of the surface.
	virtual void setY(int y);
	/**
	 * Returns the position of the surface in the Y axis.
	 * @return Y position in pixels.
	 */
	int getY() const
	{
		return _y;
	}
	/// Sets the surface's visibility.
	virtual void setVisible(bool visible);
	/// Gets the surface's visibility.
	bool getVisible() const;
	/// Get the redraw flag which indicates if the surface should be redrawn.
	bool shouldRedraw() { return _redraw; }
	/// Set the redraw flag which indicates if the surface should be redrawn.
	void setRedraw(bool redraw) { _redraw = redraw; }
	/// Gets the redraw flag which indicates if the surface should be redrawn.
	bool getRedraw() const { return _redraw; }
	/// Gets the cropping rectangle for the surface.
	SurfaceCrop getCrop() const;
	/**
	 * Changes the color of a pixel in the surface, relative to
	 * the top-left corner of the surface. Invalid positions are ignored.
	 * @param x X position of the pixel.
	 * @param y Y position of the pixel.
	 * @param pixel New color for the pixel.
	 */
	void setPixel(int x, int y, uint8_t pixel)
	{
		if (x < 0 || x >= getWidth() || y < 0 || y >= getHeight())
		{
			return;
		}
		*getRaw(x, y) = pixel;
	}
	/**
	 * Changes the color of a pixel in the surface and returns the
	 * next pixel position. Useful when changing a lot of pixels in
	 * a row, eg. manipulating images.
	 * @param x Pointer to the X position of the pixel. Changed to the next X position in the sequence.
	 * @param y Pointer to the Y position of the pixel. Changed to the next Y position in the sequence.
	 * @param pixel New color for the pixel.
	 */
	void setPixelIterative(int *x, int *y, uint8_t pixel)
	{
		setPixel(*x, *y, pixel);
		(*x)++;
		if (*x == getWidth())
		{
			(*y)++;
			*x = 0;
		}
	}
	/**
	 * Returns the color of a specified pixel in the surface.
	 * @param x X position of the pixel.
	 * @param y Y position of the pixel.
	 * @return Color of the pixel, zero if the position is invalid.
	 */
	uint8_t getPixel(int x, int y) const
	{
		if (x < 0 || x >= getWidth() || y < 0 || y >= getHeight())
		{
			return 0;
		}
		return *getRaw(x, y);
	}
	/**
	 * Returns the pointer to a specified pixel in the surface.
	 * @param x X position of the pixel.
	 * @param y Y position of the pixel.
	 * @return Pointer to the pixel.
	 */
	const uint8_t *getRaw(int x, int y) const
	{
		return nullptr;//(uint8_t *)_surface->pixels + (y * _surface->pitch + x * _surface->format->BytesPerPixel);
	}
	/**
	 * Returns the pointer to a specified pixel in the surface.
	 * @param x X position of the pixel.
	 * @param y Y position of the pixel.
	 * @return Pointer to the pixel.
	 */
	uint8_t *getRaw(int x, int y)
	{
		return nullptr; //(uint8_t *)_surface->pixels + (y * _surface->pitch + x * _surface->format->BytesPerPixel);
	}
	/**
	 * Returns the internal SDL_Surface for SDL calls.
	 * @return Pointer to the surface.
	 */
	SDL_Surface *getSurface()
	{
		return _surface.get();
	}
	/**
	 * Returns the width of the surface.
	 * @return Width in pixels.
	 */
	int getWidth() const
	{
		return _width;
	}
	/// Sets the width of the surface.
	virtual void setWidth(int width);
	/**
	 * Returns the height of the surface.
	 * @return Height in pixels
	 */
	int getHeight() const
	{
		return _height;
	}
	/// Sets the height of the surface.
	virtual void setHeight(int height);
	/// Get surface pitch in bytes.
	int getPitch() const
	{
		return _pitch;
	}
	/// Get pointer to buffer
	uint8_t* getBuffer()
	{
		return _alignedBuffer.get();
	}
	/// Get pointer to buffer
	const uint8_t* getBuffer() const
	{
		return _alignedBuffer.get();
	}
	/// Sets the surface's special hidden flag.
	void setHidden(bool hidden);
	/// Gets the surface's special hidden flag.
	bool isHidden() const { return _hidden; };
	/// Locks the surface.
	void lock();
	/// Unlocks the surface.
	void unlock();
	/// Specific blit function to blit battlescape terrain data in different shades in a fast way.
	static void blitRaw(SurfaceRaw<uint8_t> dest, SurfaceRaw<const uint8_t> src, int x, int y, int shade, bool half = false, int newBaseColor = 0);
	/// Specific blit function to blit battlescape terrain data in different shades in a fast way.
	void blitNShade(SurfaceRaw<uint8_t> surface, int x, int y, int shade = 0, bool half = false, int newBaseColor = 0) const;
	/// Specific blit function to blit battlescape terrain data in different shades in a fast way.
	void blitNShade(SurfaceRaw<uint8_t> surface, int x, int y, int shade, GraphSubset range) const;
	/// Invalidate the surface: force it to be redrawn
	void invalidate(bool valid = true);

	/// Sets the color of the surface.
	virtual void setColor(uint8_t /*color*/) { /* empty by design */ };
	/// Sets the secondary color of the surface.
	virtual void setSecondaryColor(uint8_t /*color*/) { /* empty by design */ };
	/// Sets the border colour of the surface.
	virtual void setBorderColor(uint8_t /*color*/) { /* empty by design */ };
	/// Sets the high contrast color setting of the surface.
	virtual void setHighContrast(bool /*contrast*/) { /* empty by design */ };

	SDL_Surface* getSDLSurface() { return _surface.get(); }
};

/**
 * Raw pointer to surface buffer, can be created from different sources
 */
template<typename Pixel>
class SurfaceRaw
{
	Pixel* _buffer;
	uint16_t _width, _height, _pitch;

public:
	/// Default constructor
	SurfaceRaw() :
		_buffer{ nullptr },
		_width{ 0 },
		_height{ 0 },
		_pitch{ 0 }
	{

	}

	/// Copy constructor
	SurfaceRaw(const SurfaceRaw&) = default;

	/// Move constructor
	SurfaceRaw(SurfaceRaw&&) = default;

	/// Constructor
	SurfaceRaw(Pixel* buffer, int width, int height, int pitch) :
		_buffer{ buffer },
		_width{ static_cast<uint16_t>(width) },
		_height{ static_cast<uint16_t>(height) },
		_pitch{ static_cast<uint16_t>(pitch) }
	{

	}

	/// Constructor, SFINAE enable it only for `uint8_t`
	template<typename = std::enable_if<std::is_same<uint8_t, Pixel>::value, void>>
	SurfaceRaw(Surface* surf) : SurfaceRaw{ }
	{
		assert(!"Not implemented anymore(part of SDL to Vulkan conversion)");
		// if (surf)
		//{
		//	*this = SurfaceRaw{ surf->getBuffer(), surf->getWidth(), surf->getHeight(), surf->getPitch() };
		//}
	}

	/// Constructor, SFINAE enable it only for `uint8_t`
	template<typename = std::enable_if<std::is_same<const uint8_t, Pixel>::value, void>>
	SurfaceRaw(const Surface* surf) : SurfaceRaw{ }
	{
		assert(!"Not implemented anymore(part of SDL to Vulkan conversion)");
		// if (surf)
		//{
		//	*this = SurfaceRaw{ surf->getBuffer(), surf->getWidth(), surf->getHeight(), surf->getPitch() };
		//}
	}

	/// Constructor, SFINAE enable it only for `uint8_t`
	template<typename = std::enable_if<std::is_same<uint8_t, Pixel>::value, void>>
	SurfaceRaw(SDL_Surface* surf) : SurfaceRaw{ }
	{
		assert(!"Not implemented anymore(part of SDL to Vulkan conversion)");
		// if (surf)
		//{
		//	*this = SurfaceRaw{ (Pixel*)surf->pixels, surf->w, surf->h, surf->pitch };
		//}
	}

	/// Constructor, SFINAE enable it only for `const uint8_t`
	template<typename = std::enable_if<std::is_same<const uint8_t, Pixel>::value, void>>
	SurfaceRaw(const SDL_Surface* surf) : SurfaceRaw{ }
	{
		assert(!"Not implemented anymore(part of SDL to Vulkan conversion)");
		//if (surf)
		//{
		//	*this = SurfaceRaw{ (Pixel*)surf->pixels, surf->w, surf->h, surf->pitch };
		//}
	}

	/// Constructor, SFINAE enable it only for non const `PixelType`
	template<typename = std::enable_if<std::is_const<Pixel>::value == false, void>>
	SurfaceRaw(std::vector<Pixel>& vec, int width, int height) : SurfaceRaw{ vec.data(), width, height, static_cast<uint16_t>(width*sizeof(Pixel)) }
	{
		assert((size_t)(width*height) <= vec.size() && "Incorrect dimensions compared to vector size");
	}

	/// Constructor, SFINAE enable it only for `const PixelType`
	template<typename = std::enable_if<std::is_const<Pixel>::value, void>>
	SurfaceRaw(const std::vector<typename std::remove_const<Pixel>::type>& vec, int width, int height) : SurfaceRaw{ vec.data(), width, height, static_cast<uint16_t>(width*sizeof(Pixel)) }
	{
		assert((size_t)(width*height) <= vec.size() && "Incorrect dimensions compared to vector size");
	}

	/// Constructor
	template<int I>
	SurfaceRaw(Pixel (&buffer)[I], int width, int height) : SurfaceRaw{ buffer, width, height, static_cast<uint16_t>(width*sizeof(Pixel)) }
	{
		assert(width*height <= I && "Incorrect dimensions compared to array size");
	}

	/// Assignment from nullptr
	SurfaceRaw& operator=(std::nullptr_t)
	{
		*this = SurfaceRaw{};
		return *this;
	}

	/// Assignment
	SurfaceRaw& operator=(const SurfaceRaw&) = default;

	/// Is empty?
	explicit operator bool() const
	{
		return _buffer;
	}

	/// Returns the width of the surface.
	int getWidth() const
	{
		return _width;
	}

	/// Returns the height of the surface.
	int getHeight() const
	{
		return _height;
	}

	/// Get surface pitch in bytes.
	int getPitch() const
	{
		return _pitch;
	}

	/// Get pointer to buffer
	Pixel* getBuffer() const
	{
		return _buffer;
	}
};

/**
 * Helper class used to blit part of surface to another one.
 */
class SurfaceCrop
{
	const Surface* _surface;
	SDL_Rect _crop;
	int _x, _y;

public:
	/// Default constructor
	SurfaceCrop() : _surface{ nullptr }, _crop{ }, _x{ }, _y{ }
	{

	}

	/// Constructor
	SurfaceCrop(const Surface* surf) : _surface{ surf }, _crop{ }, _x{ surf->getX() }, _y{ surf->getY() }
	{

	}

	/// Get crop rectangle.
	SDL_Rect* getCrop()
	{
		return &_crop;
	}

	/// Get Surface.
	const Surface* getSurface()
	{
		return _surface;
	}

	/// Sets the X position of the surface.
	void setX(int x)
	{
		_x = x;
	}

	/// Returns the position of the surface in the X axis.
	int getX() const
	{
		return _x;
	}

	/// Sets the Y position of the surface.
	void setY(int y)
	{
		_y = y;
	}

	/// Returns the position of the surface in the Y axis.
	int getY() const
	{
		return _y;
	}

	/// Blit Cropped surface to another surface.
	void blit(Surface* dest);
};





}
