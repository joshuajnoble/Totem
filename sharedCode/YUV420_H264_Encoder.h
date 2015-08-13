#pragma once

#include "FFmpegImports.h"
#include <functional>

class YUV420_H264_Encoder
{
private:
	FFmpegFactory m_ffmpeg;
	FrameCallback callback;

	int cInputFrames = 0;
	AVPacket pkt;
	int yBlocksize, uBlockSize, vBlockSize;

	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVFrame *pFrame = NULL;

public:
	int getRawFrameSize();
	YUV420_H264_Encoder(int width, int height, int fps, FrameCallback callback);
	~YUV420_H264_Encoder();
	void WriteFrame(const uint8_t* framebuffer);

	void Close();
};