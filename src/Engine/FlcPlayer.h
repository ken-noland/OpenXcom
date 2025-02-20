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

#pragma once
/*
 * Based on http://www.libsdl.org/projects/flxplay/
 */


namespace OpenXcom
{

class Screen;
class Game;

class FlcPlayer
{
private:

	uint8_t *_fileBuf;
	uint32_t _fileSize;
	uint8_t *_videoFrameData;
	uint8_t *_chunkData;
	uint8_t *_audioFrameData;
	uint16_t _frameCount;    /* Frame Counter */
	uint32_t _headerSize;    /* Fli file size */
	uint16_t _headerType;    /* Fli header check */
	uint16_t _headerFrames;  /* Number of frames in flic */
	uint16_t _headerWidth;   /* Fli width */
	uint16_t _headerHeight;  /* Fli height */
	uint16_t _headerDepth;   /* Color depth */
	uint16_t _headerSpeed;   /* Number of video ticks between frame */
	uint32_t _videoFrameSize;     /* Frame size in bytes */
	uint16_t _videoFrameType;
	uint16_t _frameChunks;   /* Number of chunks in frame */
	uint32_t _chunkSize;     /* Size of chunk */
	uint16_t _chunkType;     /* Type of chunk */
	uint16_t _delayOverride; /* FRAME_TYPE extension */
	uint32_t _audioFrameSize;
	uint16_t _audioFrameType;

	void (*_frameCallBack)();

	SDL_Surface *_mainScreen;
	Screen *_realScreen;
	SDL_Color _colors[256];
	int _screenWidth;
	int _screenHeight;
	int _screenDepth;
	int _dx, _dy;
	int _offset;
	int _playingState;
	bool _hasAudio, _useInternalAudio;
	int _videoDelay;
	double _volume;

	typedef struct AudioBuffer
	{
		int16_t *samples;
		int sampleCount;
		int sampleBufSize;
		int currSamplePos;
	}AudioBuffer;


	typedef struct AudioData
	{
		int sampleRate;
		AudioBuffer *loadingBuffer;
		AudioBuffer *playingBuffer;
		SDL_sem *sharedLock;

	}AudioData;

	AudioData _audioData;

	Game *_game;

	void readU16(uint16_t &dst, const uint8_t *const src);
	void readU32(uint32_t &dst, const uint8_t *const src);
	void readS16(int16_t &dst, const int8_t *const src);
	void readS32(int32_t &dst, const int8_t *const src);
	void readFileHeader();

	bool isValidFrame(uint8_t *frameHeader, uint32_t &frameSize, uint16_t &frameType);
	void decodeVideo(bool skipLastFrame);
	void decodeAudio(int frames);
	void waitForNextFrame(uint32_t delay);
	void SDLPolling();
	bool shouldQuit();

	void playVideoFrame();
	void color256();
	void fliBRun();
	void fliCopy();
	void fliSS2();
	void fliLC();
	void color64();
	void black();

	void playAudioFrame(uint16_t sampleRate);
	void initAudio(uint16_t format, uint8_t channels);
	void deInitAudio();

	bool isEndOfFile(uint8_t *pos);

	static void audioCallback(void *userData, uint8_t *stream, int len);

public:

	FlcPlayer();
	~FlcPlayer();

	/// Open FLC or FLI file, read header, prepare to play it
	bool init(const char *filename, void(*frameCallBack)(), Game *game, bool useAudio, int dx, int dy);
	/// Play the loaded file; set flc.mainScreen first!
	void play(bool skipLastFrame);
	/// Free memory, free love, etc.
	void deInit();
	// Stop FLC Player
	void stop();
	/// Delay player at the end
	void delay(uint32_t milliseconds);
	void setHeaderSpeed(int speed);
	int getFrameCount();
	bool wasSkipped();
};

}
