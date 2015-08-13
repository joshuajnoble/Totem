#pragma once

#include "FFmpegImports.h"

#include <string>

class H264NetworkSender
{
private:
	AVOutputFormat *avOutputFormat = NULL;
	AVFormatContext *avOutputFormatContext = NULL;
	std::string out_filename;
	bool initialized, closed;

public:
	H264NetworkSender();
	~H264NetworkSender();
	void Start(const std::string& networkAddress, int width, int height, int fps);
	void WriteFrame(AVPacket& pkt);
	void Close();
};