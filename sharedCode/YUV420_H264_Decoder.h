#pragma once

#include "FFmpegImports.h"
#include <functional>

class YUV420_H264_Decoder
{
private:
	DecodedFrameCallback callback;

	int cInputFrames = 0;
	AVPacket pkt;
	int yBlocksize, uBlockSize, vBlockSize;

	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodecParserContext *pCodecParserCtx = NULL;

	FILE *fp_in;
	FILE *fp_out;
	AVFrame	*pFrame;

	uint8_t *cur_ptr;
	int cur_size;
	AVPacket packet;
	int ret, got_picture;
	int y_size;
	bool gotStartingFrame;

public:
	YUV420_H264_Decoder(DecodedFrameCallback callback);
	~YUV420_H264_Decoder();
	void DecodeFrame(const AVPacket&);

	void Close();
};