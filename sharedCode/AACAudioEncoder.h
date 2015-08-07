#pragma once

#include "FFmpegImports.h"
#include <functional>
#include <vector>

class AACAudioEncoder
{
private:
	FFmpegFactory m_ffmpeg;
	FrameCallback callback;

	AVPacket pkt;
	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVFrame *pFrame = NULL;
	int sampleCount;
	int frameSizeBytes, frameSizeSamples;
	std::vector<uint8_t> alignedBuffer;
	bool closed;

public:
	AACAudioEncoder(int channels, int sampeRate, FrameCallback c);
	~AACAudioEncoder();
	int WriteFrame(const uint8_t* audioSource, int cbAudioSource);
	void Close();
};