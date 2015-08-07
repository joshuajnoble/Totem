#pragma once

#include "FFmpegImports.h"

#include <string>

class AACNetworkSender
{
private:
	FFmpegFactory m_ffmpeg;
	AVOutputFormat *avOutputFormat = NULL;
	AVFormatContext *avOutputFormatContext = NULL;
	std::string out_filename;
	bool initialized, closed;

public:
	AACNetworkSender();
	~AACNetworkSender();
	void Start(const std::string& networkAddress, int channels, int sample_rate);
	void WriteFrame(AVPacket& pkt);
	void Close();
};