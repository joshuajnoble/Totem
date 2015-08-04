#pragma once

#include "FFmpegImports.h"
#include "YUV420_H264_Encoder.h"

#include <string>
#include <vector>

class ConvertToNV12
{
private:
	uint8_t* srcData[1];
	uint8_t* dstData[2];
	int srcLineSize[1];
	int dstLineSize[2];

	int width, height;
	FFmpegFactory& m_ffmpeg;
	SwsContext* sws_context;

public:
	ConvertToNV12(FFmpegFactory& ffmpeg, int width, int height);
	~ConvertToNV12();
	void ConvertFrame(const uint8_t *rgbSource, uint8_t* yuvDestination);
};

class EncodeRGBToH264
{
private:
	FFmpegFactory &m_ffmpeg;
	std::auto_ptr<ConvertToNV12> converter;
	std::auto_ptr<YUV420_H264_Encoder> encoder;
	std::vector<uint8_t> outputBuffer;
	bool closed;

public:
	EncodeRGBToH264(FFmpegFactory &ffmpeg);
	~EncodeRGBToH264();
	void Start(int width, int height, int fps);
	void EncodeFrame(const uint8_t *srcBytes);
	void Stop();
};


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