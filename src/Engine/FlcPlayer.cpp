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

/*
 * Based on http://www.libsdl.org/projects/flxplay/
 * See also https://github.com/aseprite/flic
 */
#ifdef _MSC_VER
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif
#endif
#include "FlcPlayer.h"
#include <algorithm>
#include <cassert>
#include <string.h>
#include <SDL_mixer.h>
#include "FileMap.h"
#include "Logger.h"
#include "Screen.h"
#include "Options.h"
#include "Game.h"

namespace OpenXcom
{


// Taken from: http://www.compuphase.com/flic.htm
enum FileTypes
{
	FLI_TYPE = 0xAF11,
	FLC_TYPE = 0xAF12,
};


enum ChunkTypes
{
	COLOR_256 = 0x04,
	FLI_SS2 = 0x07, // or DELTA_FLC
	COLOR_64 = 0x0B,
	FLI_LC = 0x0C, // or DELTA_FLI
	BLACK = 0x0D,
	FLI_BRUN = 0x0F, // or BYTE_RUN
	FLI_COPY = 0x10,

	AUDIO_CHUNK = 0xAAAA, // This is the only exception, it's from TFTD
	PREFIX_CHUNK = 0xF100,
	FRAME_TYPE = 0xF1FA,
};

enum ChunkOpcodes
{
	PACKETS_COUNT = 0x0000, // 0000000000000000
	LAST_PIXEL = 0x8000, // 1000000000000000
	SKIP_LINES = 0xc000, // 1100000000000000
	MASK = SKIP_LINES
};

enum PlayingState
{
	PLAYING,
	FINISHED,
	SKIPPED
};

FlcPlayer::FlcPlayer() : _fileBuf(0), _mainScreen(0), _realScreen(0), _game(0)
{
	_volume = Game::volumeExponent(Options::musicVolume);
}

FlcPlayer::~FlcPlayer()
{
	deInit();
}

/**
 * Initialize data structures needed buy the player and read the whole file into memory
 * @param filename Video file name
 * @param frameCallback Function to call each video frame
 * @param game Pointer to the Game instance
 * @param dx An offset on the x axis for the video to be rendered
 * @param dy An offset on the y axis for the video to be rendered
 */
bool FlcPlayer::init(const char *filename, void(*frameCallBack)(), Game *game, bool useInternalAudio, int dx, int dy)
{
	if (_fileBuf != 0)
	{
		Log(LOG_ERROR) << "Trying to init a video player that is already initialized";
		return false;
	}

	_frameCallBack = frameCallBack;
	_realScreen = game->getScreen();
	_realScreen->clear();
	_game = game;
	_useInternalAudio = useInternalAudio;
	_dx = dx;
	_dy = dy;

	_fileSize = 0;
	_frameCount = 0;
	_audioFrameData = 0;
	_hasAudio = false;
	_audioData.loadingBuffer = 0;
	_audioData.playingBuffer = 0;

	auto file = FileMap::getIStream(filename);
	file->seekg(0, std::istream::end);
	auto size = file->tellg();
	file->seekg(0, std::istream::beg);
	_fileBuf = new uint8_t[size];
	_fileSize = (int)size;
	file->read((char *)_fileBuf, size);

	_audioFrameData = _fileBuf + 128;

	// Let's read the first 128 bytes
	readFileHeader();

	// If it's a FLC or FLI file, it's ok
	if (_headerType == SDL_SwapLE16(FLI_TYPE) || (_headerType == SDL_SwapLE16(FLC_TYPE)))
	{
		_screenWidth = _headerWidth;
		_screenHeight = _headerHeight;
		_screenDepth = 8;

		Log(LOG_INFO) << "Playing flx, " << _screenWidth << "x" << _screenHeight << ", " << _headerFrames << " frames";
	}
	else
	{
		Log(LOG_ERROR) << "Flx file failed header check.";
		return false;
	}
	if (_screenWidth > _realScreen->getSurface()->w && Options::displayWidth >= _screenWidth)
	{
		// base resolution of video is higher than our surface width
		// and our display resolution allows a hi-res video
		// set base resolution to video resolution
		Options::baseXResolution = _screenWidth;
		Options::baseYResolution = _screenHeight;
		_realScreen->resetDisplay();
	}
	// If the current surface used is at 8bpp use it
	if (_realScreen->getSurface()->format->BitsPerPixel == 8)
	{
		_mainScreen = _realScreen->getSurface();
	}
	else // Otherwise create a new one
	{
		_mainScreen = SDL_AllocSurface(SDL_SWSURFACE, _realScreen->getSurface()->w, _realScreen->getSurface()->h, 8, 0, 0, 0, 0);
	}

	return true;
}

void FlcPlayer::deInit()
{
	if (_mainScreen != 0 && _realScreen != 0)
	{
		if (_mainScreen != _realScreen->getSurface())
			SDL_FreeSurface(_mainScreen);

		_mainScreen = 0;
	}

	if (_fileBuf != 0)
	{
		delete[] _fileBuf;
		_fileBuf = 0;

		deInitAudio();
	}
}

/**
 * Starts decoding and playing the FLI/FLC file
 */
void FlcPlayer::play(bool skipLastFrame)
{
	_playingState = PLAYING;

	// Vertically center the video
	_dy = (_mainScreen->h - _headerHeight) / 2;

	_offset = _dy * _mainScreen->pitch + _mainScreen->format->BytesPerPixel * _dx;

	// Skip file header
	_videoFrameData = _fileBuf + 128;
	_audioFrameData = _videoFrameData;

	while (!shouldQuit())
	{
		if (_frameCallBack)
			(*_frameCallBack)();
		else // TODO: support both, in the case the callback is not some audio?
			decodeAudio(2);

		if (!shouldQuit())
			decodeVideo(skipLastFrame);

		if(!shouldQuit())
			SDLPolling();
	}

}

void FlcPlayer::delay(uint32_t milliseconds)
{
	uint32_t pauseStart = SDL_GetTicks();
	while(_playingState != SKIPPED && SDL_GetTicks() < (pauseStart + milliseconds))
	{
		SDLPolling();
	}
}
void FlcPlayer::SDLPolling()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_MOUSEBUTTONDOWN:
		case SDL_KEYDOWN:
			_playingState = SKIPPED;
			break;
		case SDL_VIDEORESIZE:
			if (Options::allowResize)
			{
				Options::newDisplayWidth = Options::displayWidth = std::max(Screen::ORIGINAL_WIDTH, event.resize.w);
				Options::newDisplayHeight = Options::displayHeight = std::max(Screen::ORIGINAL_HEIGHT, event.resize.h);
				if (_mainScreen != _realScreen->getSurface())
				{
					_realScreen->resetDisplay();
				}
				else
				{
					_realScreen->resetDisplay();
					_mainScreen = _realScreen->getSurface();
				}
			}
			break;
		case SDL_QUIT:
			exit(0);
		default:
			break;
		}
	}
}

bool FlcPlayer::shouldQuit()
{
	return _playingState == FINISHED || _playingState == SKIPPED;
}

void FlcPlayer::readFileHeader()
{
	readU32(_headerSize, _fileBuf);
	readU16(_headerType, _fileBuf + 4);
	readU16(_headerFrames, _fileBuf + 6);
	readU16(_headerWidth, _fileBuf + 8);
	readU16(_headerHeight, _fileBuf + 10);
	readU16(_headerDepth, _fileBuf + 12);
	readU16(_headerSpeed, _fileBuf + 16);
}

bool FlcPlayer::isValidFrame(uint8_t *frameHeader, uint32_t &frameSize, uint16_t &frameType)
{
	readU32(frameSize, frameHeader);
	readU16(frameType, frameHeader + 4);

	return (frameType == FRAME_TYPE || frameType == AUDIO_CHUNK || frameType == PREFIX_CHUNK);
}

void FlcPlayer::decodeAudio(int frames)
{

	int audioFramesFound = 0;

	while (audioFramesFound < frames && !isEndOfFile(_audioFrameData))
	{
		if (!isValidFrame(_audioFrameData, _audioFrameSize, _audioFrameType))
		{
			_playingState = FINISHED;
			break;
		}

		switch (_audioFrameType)
		{
			case FRAME_TYPE:
			case PREFIX_CHUNK:
				_audioFrameData += _audioFrameSize;
				break;
			case AUDIO_CHUNK:
				uint16_t sampleRate;

				readU16(sampleRate, _audioFrameData + 8);

				_chunkData = _audioFrameData + 16;

				playAudioFrame(sampleRate);

				_audioFrameData += _audioFrameSize + 16;

				++audioFramesFound;

				break;
		}
	}
}

void FlcPlayer::decodeVideo(bool skipLastFrame)
{
	bool videoFrameFound = false;

	while (!videoFrameFound)
	{
		if (!isValidFrame(_videoFrameData, _videoFrameSize, _videoFrameType))
		{
			_playingState = FINISHED;
			break;
		}

		switch (_videoFrameType)
		{
		case FRAME_TYPE:

			uint32_t delay;

			readU16(_frameChunks, _videoFrameData + 6);
			readU16(_delayOverride, _videoFrameData + 8);

			if (_headerType == FLI_TYPE)
			{
				delay = _delayOverride > 0 ? _delayOverride : (uint32_t)(_headerSpeed * (1000.0 / 70.0));
			}
			else if (_useInternalAudio && !_frameCallBack) // this means TFTD videos are playing
			{
				delay = _videoDelay;
			}
			else
			{
				delay = _headerSpeed;
			}

			waitForNextFrame(delay);

			// Skip the frame header, we are not interested in the rest
			_chunkData = _videoFrameData + 16;

			_videoFrameData += _videoFrameSize;
			// If this frame is the last one, don't play it
			if(isEndOfFile(_videoFrameData))
				_playingState = FINISHED;

			if(!shouldQuit() || !skipLastFrame)
				playVideoFrame();

			videoFrameFound = true;

			break;
		case AUDIO_CHUNK:
			_videoFrameData += _videoFrameSize + 16;
			break;
		case PREFIX_CHUNK:
			// Just skip it
			_videoFrameData += _videoFrameSize;

			break;
		}
	}
}

void FlcPlayer::playVideoFrame()
{
	++_frameCount;
	if (SDL_LockSurface(_mainScreen) < 0)
		return;
	int chunkCount = _frameChunks;

	for (int i = 0; i < chunkCount; ++i)
	{
		readU32(_chunkSize, _chunkData);
		readU16(_chunkType, _chunkData + 4);

		switch (_chunkType)
		{
			case COLOR_256:
				color256();
				break;
			case FLI_SS2:
				fliSS2();
				break;
			case COLOR_64:
				color64();
				break;
			case FLI_LC:
				fliLC();
				break;
			case BLACK:
				black();
				break;
			case FLI_BRUN:
				fliBRun();
				break;
			case FLI_COPY:
				fliCopy();
				break;
			case 18:
				break;
			default:
				Log(LOG_WARNING) << "Ieek an non implemented chunk type:" << _chunkType;
				break;
		}

		_chunkData += _chunkSize;
	}

	SDL_UnlockSurface(_mainScreen);

	/* TODO: Track which rectangles have really changed */
	//SDL_UpdateRect(_mainScreen, 0, 0, 0, 0);
	if (_mainScreen != _realScreen->getSurface())
		SDL_BlitSurface(_mainScreen, 0, _realScreen->getSurface(), 0);

	_realScreen->flip();
}

void FlcPlayer::playAudioFrame(uint16_t sampleRate)
{
	/* TFTD audio header (10 bytes)
	* uint16_t unknown1 - always 0
	* uint16_t sampleRate
	* uint16_t unknown2 - always 1 (Channels? bytes per sample?)
	* uint16_t unknown3 - always 10 (No idea)
	* uint16_t unknown4 - always 0
	* uint8_t[] unsigned 1-byte 1-channel PCM data of length _chunkSize_ (so the total chunk is _chunkSize_ + 6-byte flc header + 10 byte audio header */

	if (_useInternalAudio)
	{
		if (!_hasAudio)
		{
			_audioData.sampleRate = sampleRate;
			_hasAudio = true;
			initAudio(AUDIO_S16SYS, 1);
		}
		else
		{
			/* Cannot change sample rate mid-video */
			assert(sampleRate == _audioData.sampleRate);
		}

		SDL_SemWait(_audioData.sharedLock);
		AudioBuffer *loadingBuff = _audioData.loadingBuffer;
		assert(loadingBuff->currSamplePos == 0);
		int newSize = (_audioFrameSize + loadingBuff->sampleCount )*2;
		if (newSize > loadingBuff->sampleBufSize)
		{
			/* If the sample count has changed, we need to reallocate (Handles initial state
			* of '0' sample count too, as realloc(NULL, size) == malloc(size) */
			loadingBuff->samples = (int16_t*)realloc(loadingBuff->samples, newSize);
			loadingBuff->sampleBufSize = newSize;
		}

		for (unsigned int i = 0; i < _audioFrameSize; i++)
		{
			loadingBuff->samples[loadingBuff->sampleCount + i] = (int16_t)((float)((_chunkData[i]) -128) * 240 * _volume);
		}
		loadingBuff->sampleCount += _audioFrameSize;

		SDL_SemPost(_audioData.sharedLock);
	}
	else
	{
		_audioData.sampleRate = sampleRate; // this is used to keep the framerate correct
	}
}

void FlcPlayer::color256()
{
	uint8_t *pSrc;
	uint16_t numColorPackets;
	uint16_t numColors = 0;
	uint8_t numColorsSkip;

	pSrc = _chunkData + 6;
	readU16(numColorPackets, pSrc);
	pSrc += 2;

	while (numColorPackets--)
	{
		numColorsSkip = *(pSrc++) + numColors;
		numColors = *(pSrc++);
		if (numColors == 0)
		{
			numColors = 256;
		}

		for (int i = 0; i < numColors; ++i)
		{
			_colors[i].r = *(pSrc++);
			_colors[i].g = *(pSrc++);
			_colors[i].b = *(pSrc++);
		}

		if (_mainScreen != _realScreen->getSurface())
			SDL_SetColors(_mainScreen, _colors, numColorsSkip, numColors);
		_realScreen->setPalette(_colors, numColorsSkip, numColors, true);

		if (numColorPackets >= 1)
		{
			++numColors;
		}
	}
}

void FlcPlayer::fliSS2()
{
	uint8_t *pSrc, *pDst, *pTmpDst;
	int8_t countData;
	uint8_t columSkip, fill1, fill2;
	uint16_t lines;
	int16_t count;
	bool setLastByte = false;
	uint8_t lastByte = 0;

	pSrc = _chunkData + 6;
	pDst = (uint8_t*)_mainScreen->pixels + _offset;
	readU16(lines, pSrc);

	pSrc += 2;

	while (lines--)
	{
		readS16(count, (int8_t *)pSrc);
		pSrc += 2;

		if ((count & MASK) == SKIP_LINES)
		{
			pDst += (-count)*_mainScreen->pitch;
			++lines;
			continue;
		}

		else if ((count & MASK) == LAST_PIXEL)
		{
			setLastByte = true;
			lastByte = (count & 0x00FF);
			readS16(count, (int8_t *)pSrc);
			pSrc += 2;
		}

		if ((count & MASK) == PACKETS_COUNT)
		{
			pTmpDst = pDst;
			while (count--)
			{
				columSkip = *(pSrc++);
				pTmpDst += columSkip;
				countData = *(pSrc++);

				if (countData > 0)
				{
					std::copy(pSrc, pSrc + (2 * countData), pTmpDst);
					pTmpDst += (2 * countData);
					pSrc += (2 * countData);

				}
				else
				{
					if (countData < 0)
					{
						countData = -countData;

						fill1 = *(pSrc++);
						fill2 = *(pSrc++);
						while (countData--)
						{
							*(pTmpDst++) = fill1;
							*(pTmpDst++) = fill2;
						}
					}
				}
			}

			if (setLastByte)
			{
				setLastByte = false;
				*(pDst + _mainScreen->pitch - 1) = lastByte;
			}
			pDst += _mainScreen->pitch;
		}
	}
}

void FlcPlayer::fliBRun()
{
	uint8_t *pSrc, *pDst, *pTmpDst, fill;
	int8_t countData;
	int heightCount;

	heightCount = _headerHeight;
	pSrc = _chunkData + 6; // Skip chunk header
	pDst = (uint8_t*)_mainScreen->pixels + _offset;

	while (heightCount--)
	{
		pTmpDst = pDst;
		++pSrc; // Read and skip the packet count value

		int pixels = 0;
		while (pixels != _headerWidth)
		{
			countData = *(pSrc++);
			if (countData > 0)
			{
				fill = *(pSrc++);

				std::fill_n(pTmpDst, countData, fill);
				pTmpDst += countData;
				pixels += countData;
			}
			else
			{
				if (countData < 0)
				{
					countData = -countData;

					std::copy(pSrc, pSrc + countData, pTmpDst);
					pTmpDst += countData;
					pSrc += countData;
					pixels += countData;
				}
			}
		}
		pDst += _mainScreen->pitch;
	}
}


void FlcPlayer::fliLC()
{
	uint8_t *pSrc, *pDst, *pTmpDst;
	int8_t countData;
	uint8_t countSkip;
	uint8_t fill;
	uint16_t lines, tmp;
	int packetsCount;

	pSrc = _chunkData + 6;
	pDst = (uint8_t*)_mainScreen->pixels + _offset;

	readU16(tmp, pSrc);
	pSrc += 2;
	pDst += tmp*_mainScreen->pitch;
	readU16(lines, pSrc);
	pSrc += 2;

	while (lines--)
	{
		pTmpDst = pDst;
		packetsCount = *(pSrc++);

		while (packetsCount--)
		{
			countSkip = *(pSrc++);
			pTmpDst += countSkip;
			countData = *(pSrc++);
			if (countData > 0)
			{
				while (countData--)
				{
					*(pTmpDst++) = *(pSrc++);
				}
			}
			else
			{
				if (countData < 0)
				{
					countData = -countData;

					fill = *(pSrc++);
					while (countData--)
					{
						*(pTmpDst++) = fill;
					}
				}
			}
		}
		pDst += _mainScreen->pitch;
	}
}

void FlcPlayer::color64()
{
	uint8_t *pSrc;
	uint16_t NumColors, NumColorPackets;
	uint8_t NumColorsSkip;

	pSrc = _chunkData + 6;
	readU16(NumColorPackets, pSrc);
	pSrc += 2;

	while (NumColorPackets--)
	{
		NumColorsSkip = *(pSrc++);
		NumColors = *(pSrc++);

		if (NumColors == 0)
		{
			NumColors = 256;
		}

		for (int i = 0; i < NumColors; ++i)
		{
			_colors[i].r = *(pSrc++) << 2;
			_colors[i].g = *(pSrc++) << 2;
			_colors[i].b = *(pSrc++) << 2;
		}

		if (_mainScreen != _realScreen->getSurface())
			SDL_SetColors(_mainScreen, _colors, NumColorsSkip, NumColors);
		_realScreen->setPalette(_colors, NumColorsSkip, NumColors, true);
	}
}

void FlcPlayer::fliCopy()
{
	uint8_t *pSrc, *pDst;
	int Lines = _screenHeight;
	pSrc = _chunkData + 6;
	pDst = (uint8_t*)_mainScreen->pixels + _offset;

	while (Lines--)
	{
		memcpy(pDst, pSrc, _screenWidth);
		pSrc += _screenWidth;
		pDst += _mainScreen->pitch;
	}
}

void FlcPlayer::black()
{
	uint8_t *pDst;
	int Lines = _screenHeight;
	pDst = (uint8_t*)_mainScreen->pixels + _offset;

	while (Lines-- > 0)
	{
		memset(pDst, 0, _screenHeight);
		pDst += _mainScreen->pitch;
	}
}

void FlcPlayer::audioCallback(void *userData, uint8_t *stream, int len)
{
	AudioData *audio = (AudioData*)userData;

	AudioBuffer *playBuff = audio->playingBuffer;

	while (len > 0)
	{
		if (playBuff->sampleCount > 0)
		{
			int bytesToCopy = std::min(len, playBuff->sampleCount * 2);
			memcpy(stream, playBuff->samples + playBuff->currSamplePos, bytesToCopy);

			playBuff->currSamplePos += bytesToCopy / 2;
			playBuff->sampleCount -= bytesToCopy / 2;
			len -= bytesToCopy;

			assert(playBuff->sampleCount >= 0);
		}

		if (len > 0)
		{
			/* Need to swap buffers */
			playBuff->currSamplePos = 0;
			SDL_SemWait(audio->sharedLock);
			AudioBuffer *tempBuff = playBuff;
			audio->playingBuffer = playBuff = audio->loadingBuffer;
			audio->loadingBuffer = tempBuff;
			SDL_SemPost(audio->sharedLock);

			if (playBuff->sampleCount == 0)
				break;
		}
	}
}

void FlcPlayer::initAudio(uint16_t format, uint8_t channels)
{
	_videoDelay = 1000 / (_audioData.sampleRate / _audioFrameSize );
	if (_useInternalAudio)
	{
		if (!Options::mute)
		{
			if (Mix_OpenAudio(_audioData.sampleRate, format, channels, _audioFrameSize * 2) != 0)
			{
				Log(LOG_ERROR) << Mix_GetError();
				Log(LOG_WARNING) << "Failed to init cutscene audio";
				Options::mute = true;
			}
		}

		/* Start runnable */
		_audioData.sharedLock = SDL_CreateSemaphore(1);

		_audioData.loadingBuffer = new AudioBuffer();
		_audioData.loadingBuffer->currSamplePos = 0;
		_audioData.loadingBuffer->sampleCount = 0;
		_audioData.loadingBuffer->samples = (int16_t *)malloc(_audioFrameSize * 2);
		_audioData.loadingBuffer->sampleBufSize = _audioFrameSize * 2;

		_audioData.playingBuffer = new AudioBuffer();
		_audioData.playingBuffer->currSamplePos = 0;
		_audioData.playingBuffer->sampleCount = 0;
		_audioData.playingBuffer->samples = (int16_t *)malloc(_audioFrameSize * 2);
		_audioData.playingBuffer->sampleBufSize = _audioFrameSize * 2;

		if (!Options::mute)
		{
			Mix_HookMusic(FlcPlayer::audioCallback, &_audioData);
		}
	}
}

void FlcPlayer::deInitAudio()
{
	if (_game)
	{
		if (!Options::mute)
		{
			Mix_HookMusic(NULL, NULL);
			Mix_CloseAudio();
			_game->initAudio();
		}
	}
	else if (_audioData.sharedLock)
		SDL_DestroySemaphore(_audioData.sharedLock);

	if (_audioData.loadingBuffer)
	{
		free(_audioData.loadingBuffer->samples);
		delete _audioData.loadingBuffer;
		_audioData.loadingBuffer = 0;
	}

	if (_audioData.playingBuffer)
	{
		free(_audioData.playingBuffer->samples);
		delete _audioData.playingBuffer;
		_audioData.playingBuffer = 0;
	}

}

void FlcPlayer::stop()
{
	_playingState = FINISHED;
}

bool FlcPlayer::isEndOfFile(uint8_t *pos)
{
	return (pos - _fileBuf) == (int)(_fileSize); // should be Sint64, but let's assume the videos won't be 2gb
}

int FlcPlayer::getFrameCount()
{
	return _frameCount;
}

void FlcPlayer::setHeaderSpeed(int speed)
{
	_headerSpeed = speed;
}

bool FlcPlayer::wasSkipped()
{
	return _playingState == SKIPPED;
}

void FlcPlayer::waitForNextFrame(uint32_t delay)
{
	static uint32_t oldTick = 0;
	int newTick;
	int currentTick;

	currentTick = SDL_GetTicks();
	if (oldTick == 0)
	{
		oldTick = currentTick;
		newTick = oldTick;
	}
	else
		newTick = oldTick + delay;

	if (_hasAudio)
	{
		while (currentTick < newTick)
		{
			while ((newTick - currentTick) > 10 && !isEndOfFile(_audioFrameData))
			{
				decodeAudio(1);
				currentTick = SDL_GetTicks();
			}
			SDL_Delay(1);
			currentTick = SDL_GetTicks();
		}
	}
	else
	{
		while (currentTick < newTick)
		{
			SDL_Delay(1);
			currentTick = SDL_GetTicks();
		}
	}
	oldTick = SDL_GetTicks();
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
inline void FlcPlayer::readU16(uint16_t &dst, const uint8_t * const src)
{
	dst = (src[0] << 8) | src[1];
}
inline void FlcPlayer::readU32(uint32_t &dst, const uint8_t * const src)
{
	dst = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
}
inline void FlcPlayer::readS16(int16_t &dst, const int8_t * const src)
{
	dst = (src[0] << 8) | src[1];
}
inline void FlcPlayer::readS32(int32_t &dst, const int8_t * const src)
{
	dst = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
}
#else
inline void FlcPlayer::readU16(uint16_t &dst, const uint8_t * const src)
{
	dst = (src[1] << 8) | src[0];
}
inline void FlcPlayer::readU32(uint32_t &dst, const uint8_t * const src)
{
	dst = (src[3] << 24) | (src[2] << 16) | (src[1] << 8) | src[0];
}
inline void FlcPlayer::readS16(int16_t &dst, const int8_t * const src)
{
	dst = (src[1] << 8) | src[0];
}
inline void FlcPlayer::readS32(int32_t &dst, const int8_t * const src)
{
	dst = (src[3] << 24) | (src[2] << 16) | (src[1] << 8) | src[0];
}
#endif

}
