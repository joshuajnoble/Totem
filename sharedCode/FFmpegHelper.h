#pragma once

#include "FFmpegImports.h"
#include <vector>

class FFmpegHelper
{
public:
	FFmpegFactory* m_ffmpeg;

	FFmpegHelper();
	~FFmpegHelper();

	std::vector<std::auto_ptr<AVInputFormat>> GetDeviceList();
	//std::vector<std::auto_ptr<AVDeviceInfoList>> GetInputSources(std::auto_ptr<AVInputFormat> AVInputFormat);
	std::vector<std::auto_ptr<AVDeviceInfoList>> GetInputSource(const std::string sourceName);
};