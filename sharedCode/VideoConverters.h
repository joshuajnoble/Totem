#pragma once

#include "FFmpegImports.h"
//#include "YUV420_H264_Encoder.h"
#include <string>
#include <vector>
#include <fstream>

typedef std::function<void(const uint8_t* buffer, int bufferSize)> RGBFrameCallback;

class PCMAudioEncoder;
class PCMNetworkSender;
class YUV420_H264_Encoder;
class H264NetworkSender;
class H264NetworkReceiver;
class YUV420_H264_Decoder;

class ConvertToNV12
{
private:
	uint8_t* srcData[1];
	uint8_t* dstData[2];
	int srcLineSize[1];
	int dstLineSize[2];

	int width, height;
	FFmpegFactory m_ffmpeg;
	SwsContext* sws_context;

public:
	ConvertToNV12(int width, int height);
	~ConvertToNV12();

	int GetInputFrameSize();
	int GetOutputFrameSize();

	void WriteFrame(const uint8_t *rgbSource, uint8_t* yuvDestination);
};

class ConvertToRGB
{
private:
	uint8_t* dstData[1];
	int dstLineSize[2];

	int width, height;
	FFmpegFactory m_ffmpeg;
	SwsContext* sws_context;

public:
	ConvertToRGB(int width, int height);
	~ConvertToRGB();

	int GetInputFrameSize();
	int GetOutputFrameSize();

	void WriteFrame(const AVFrame& frameSource, uint8_t *rgbDestination);
};

class EncodeRGBToH264
{
protected:
	FFmpegFactory m_ffmpeg;
	std::auto_ptr<ConvertToNV12> converter;
	std::auto_ptr<YUV420_H264_Encoder> encoder;
	std::vector<uint8_t> yuvBuffer;
	FrameCallback callback;
	bool closed;

public:
	EncodeRGBToH264(FrameCallback callback);
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
	EncodeRGBToH264File(const std::string& filename);
	~EncodeRGBToH264File();

	void Start(int width, int height, int fps);
	void WriteFrame(const uint8_t *srcBytes);
	void Close();
};

class EncodeRGBToH264Live
{
private:
	std::auto_ptr<EncodeRGBToH264> encoder;
	std::auto_ptr<PCMAudioEncoder> audioEncoder;
	std::auto_ptr<H264NetworkSender> streamer;
	std::auto_ptr<PCMNetworkSender> audioStreamer;
		
	bool closed;

	void ProcessEncodedFrame(AVPacket&);
	void ProcessEncodedAudioFrame(AVPacket&);

public:
	EncodeRGBToH264Live();
	~EncodeRGBToH264Live();

	void Start(const std::string& ipAddress, uint16_t port, int width, int height, int fps);
	void WriteFrame(const uint8_t *srcBytes);
	void WriteAudioFrame(const uint8_t* audioSource, int cbAudioSource);
	void Close();
};


class DecodeH264LiveToRGB
{
private:
	FFmpegFactory m_ffmpeg;

	std::auto_ptr<YUV420_H264_Decoder> decoder;
	std::auto_ptr<H264NetworkReceiver> receiver;
	std::auto_ptr<ConvertToRGB> converter;

	bool closed;

	void ProcessEncodedFrame(AVPacket&);
	void ProcessYUVFrame(const AVFrame &);
	RGBFrameCallback callback;

	int m_width, m_height, m_fps;
	std::vector<uint8_t> rgbBuffer;

public:
	DecodeH264LiveToRGB();
	~DecodeH264LiveToRGB();

	int width() { return m_width; }
	int height() { return m_height; }
	int fps() { return m_fps; }

	void Start(const std::string& ipAddress, uint16_t port, RGBFrameCallback rgbFrameCallback);
	void Close();
	bool isConnected();
};