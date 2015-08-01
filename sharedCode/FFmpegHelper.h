#pragma once

#include "FFmpegImports.h"

#include <string>
#include <vector>

class FFmpegHelper
{
	FFmpegWrapper::avdevice deviceDll;
	FFmpegWrapper::avcodec codecDll;

public:
	FFmpegHelper();
	~FFmpegHelper();

	std::vector<std::auto_ptr<AVInputFormat>> GetDeviceList();
	//std::vector<std::auto_ptr<AVDeviceInfoList>> GetInputSources(std::auto_ptr<AVInputFormat> AVInputFormat);
	std::vector<std::auto_ptr<AVDeviceInfoList>> GetInputSource(const std::string sourceName);
};