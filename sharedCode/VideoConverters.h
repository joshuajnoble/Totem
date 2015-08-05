#pragma once

#include "FFmpegImports.h"
//#include "YUV420_H264_Encoder.h"

#include <string>
#include <vector>
#include <fstream>

class YUV420_H264_Encoder;
class H264NetworkSender;

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

	int GetInputFrameSize();
	int GetOutputFrameSize();

	void WriteFrame(const uint8_t *rgbSource, uint8_t* yuvDestination);
};

class EncodeRGBToH264
{
protected:
	FFmpegFactory &m_ffmpeg;
	std::auto_ptr<ConvertToNV12> converter;
	std::auto_ptr<YUV420_H264_Encoder> encoder;
	std::vector<uint8_t> yuvBuffer;
	FrameCallback callback;
	bool closed;

public:
	EncodeRGBToH264(FFmpegFactory &ffmpeg, FrameCallback callback);
	~EncodeRGBToH264();
	void Start(int width, int height, int fps);
	void WriteFrame(const uint8_t *srcBytes);
	void Close();
};

class EncodeRGBToH264File
{
private:
	EncodeRGBToH264 encoder;
	std::ofstream outputFile;
	bool closed;
	
	void ProcessEncodedFrame(AVPacket&);

public:
	EncodeRGBToH264File(FFmpegFactory &ffmpeg, const std::string& filename);
	~EncodeRGBToH264File();

	void Start(int width, int height, int fps);
	void WriteFrame(const uint8_t *srcBytes);
	void Close();
};

class EncodeRGBToH264Live
{
private:
	std::auto_ptr<EncodeRGBToH264> encoder;
	std::auto_ptr<H264NetworkSender> streamer;
		
	std::ofstream outptuFile;
	bool closed;

	void ProcessEncodedFrame(AVPacket&);

public:
	EncodeRGBToH264Live(FFmpegFactory &ffmpeg);
	~EncodeRGBToH264Live();

	void Start(std::string& ipAddress, std::string& port, int width, int height, int fps);
	void WriteFrame(const uint8_t *srcBytes);
	void Close();
};