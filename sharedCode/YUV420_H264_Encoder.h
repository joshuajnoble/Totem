#pragma once

#include "FFmpegImports.h"

class YUV420_H264_Encoder
{
private:
	FFmpegFactory& m_ffmpeg;

	int cInputFrames = 0;
	AVPacket pkt;
	int yBlocksize, uBlockSize, vBlockSize;

	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	FILE *fp_out = NULL;
	AVFrame *pFrame = NULL;

public:
	int getRawFrameSize();
	YUV420_H264_Encoder(FFmpegFactory& ffmpeg, char* filename_out, int width, int height, int fps);
	~YUV420_H264_Encoder();
	void EncodeFrame(const uint8_t* framebuffer);
	void Close();
};