#pragma once

#include "FFmpegImports.h"
#include <functional>

class PCMAudioEncoder
{
private:
	FrameCallback callback;

	AVPacket pkt;
	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVFrame *pFrame = NULL;
	int sampleCount;
	bool closed;

public:
	PCMAudioEncoder(int channels, int sampeRate, FrameCallback c);
	~PCMAudioEncoder();
	void WriteFrame(const uint8_t* audioSource, int cbAudioSource);
	void Close();
};