#pragma once

#include "FFmpegImports.h"

#include <string>

class PCMNetworkSender
{
private:
	FFmpegFactory m_ffmpeg;
	AVOutputFormat *avOutputFormat = NULL;
	AVFormatContext *avOutputFormatContext = NULL;
	std::string out_filename;
	bool initialized, closed;

public:
	PCMNetworkSender();
	~PCMNetworkSender();
	void Start(const std::string& networkAddress, int channels, int sample_rate);
	void WriteFrame(AVPacket& pkt);
	void Close();
};