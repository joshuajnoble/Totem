#pragma once

#include "FFmpegImports.h"
#include <functional>

class MP3AudioDecoder
{
private:
	FFmpegFactory m_ffmpeg;
	DecodedFrameCallback callback;

	int cInputFrames = 0;
	AVPacket pkt;
	int yBlocksize, uBlockSize, vBlockSize;

	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodecParserContext *pCodecParserCtx = NULL;

	AVFrame	*pFrame;

	uint8_t *cur_ptr;
	int cur_size;
	AVPacket packet;
	int ret, got_picture;
	int y_size;
	bool gotStartingFrame;
	bool closed;

public:
	MP3AudioDecoder(int channels, int sampleRate, DecodedFrameCallback callback);
	~MP3AudioDecoder();
	void DecodeFrame(AVPacket&);

	void Close();
};