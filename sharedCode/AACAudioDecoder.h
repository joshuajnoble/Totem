#pragma once

#include "FFmpegImports.h"
#include <functional>

class AACAudioDecoder
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
	AACAudioDecoder(int channels, int sampleRate, DecodedFrameCallback callback);
	~AACAudioDecoder();
	void DecodeFrame(AVPacket&);

	void Close();
};