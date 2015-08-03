#pragma once

#include "FFmpegImports.h"

#include <string>
#include <vector>

class FFmpegHelper
{
public:
	FFmpegWrapper::avdevice device;
	FFmpegWrapper::avcodec codec;
	FFmpegWrapper::utils utils;
	FFmpegWrapper::format format;

	FFmpegHelper();
	~FFmpegHelper();

	std::vector<std::auto_ptr<AVInputFormat>> GetDeviceList();
	//std::vector<std::auto_ptr<AVDeviceInfoList>> GetInputSources(std::auto_ptr<AVInputFormat> AVInputFormat);
	std::vector<std::auto_ptr<AVDeviceInfoList>> GetInputSource(const std::string sourceName);
};