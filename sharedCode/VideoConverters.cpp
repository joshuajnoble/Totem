#include "VideoConverters.h"
#include "H264NetworkSender.h"
#include "H264NetworkReceiver.h"
#include "YUV420_H264_Encoder.h"

//
// ConvertToNV12
//
ConvertToNV12::ConvertToNV12(FFmpegFactory& ffmpeg, int width, int height) :
width(width),
height(height),
m_ffmpeg(ffmpeg)
{
	// Asume we are converting from RGB24
	this->sws_context = m_ffmpeg.scale.sws_getContext(
		this->width, this->height, AV_PIX_FMT_RGB24,
		this->width, this->height, AV_PIX_FMT_NV12,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (!this->sws_context) throw std::runtime_error("Could not allocate convetsion context.");

	this->srcLineSize[0] = 3 * width;
	this->dstLineSize[0] = width;
	this->dstLineSize[1] = width;
}

ConvertToNV12::~ConvertToNV12()
{
	if (this->sws_context)
	{
		m_ffmpeg.scale.sws_freeContext(this->sws_context);
		this->sws_context = NULL;
	}
}

int ConvertToNV12::GetInputFrameSize()
{
	return this->width * this->height * 3;
}

int ConvertToNV12::GetOutputFrameSize()
{
	return (this->width * this->height) + (this->width * this->height / 2);
}

void ConvertToNV12::WriteFrame(const uint8_t *rgbSource, uint8_t* yuvDestination)
{
	this->srcData[0] = const_cast<uint8_t*>(rgbSource);

	this->dstData[0] = yuvDestination;
	this->dstData[1] = yuvDestination + height * dstLineSize[0];
	m_ffmpeg.scale.sws_scale(this->sws_context, this->srcData, this->srcLineSize, 0, this->height, this->dstData, this->dstLineSize);
}

//
// EncodeRGBToH264
//
EncodeRGBToH264::EncodeRGBToH264(FFmpegFactory &ffmpeg, FrameCallback callback) : m_ffmpeg(ffmpeg), callback(callback), closed(false)
{
}

EncodeRGBToH264::~EncodeRGBToH264()
{
	this->Close();
}

void EncodeRGBToH264::Start(int width, int height, int fps)
{
	this->converter.reset(new ConvertToNV12(m_ffmpeg, width, height));
	this->yuvBuffer.resize(this->converter->GetInputFrameSize());
	this->encoder.reset(new YUV420_H264_Encoder(m_ffmpeg, width, height, fps, this->callback));
}

void EncodeRGBToH264::WriteFrame(const uint8_t *srcBytes)
{
	this->converter->WriteFrame(srcBytes, this->yuvBuffer.data());
	this->encoder->WriteFrame(this->yuvBuffer.data());
}

void EncodeRGBToH264::Close()
{
	if (!this->closed)
	{
		this->encoder->Close();
		this->closed = true;
	}
}

//
// EncodeRGBToH264File
//
EncodeRGBToH264File::EncodeRGBToH264File(FFmpegFactory &ffmpeg, const std::string& filename) :
	closed(false),
	outputFile(std::ofstream(filename, std::ofstream::binary)),
	encoder(ffmpeg, std::bind(&EncodeRGBToH264File::ProcessEncodedFrame, this, std::placeholders::_1))
{
	this->outputFile = std::ofstream(filename, std::ofstream::binary);
}

EncodeRGBToH264File::~EncodeRGBToH264File()
{
	this->Close();
}

void EncodeRGBToH264File::Start(int width, int height, int fps)
{
	this->encoder.Start(width, height, fps);
}

void EncodeRGBToH264File::WriteFrame(const uint8_t *srcBytes)
{
	this->encoder.WriteFrame(srcBytes);
}

void EncodeRGBToH264File::ProcessEncodedFrame(AVPacket &packet)
{
	this->outputFile.write(reinterpret_cast<const char *>(packet.data), packet.size);
}

void EncodeRGBToH264File::Close()
{
	if (!this->closed)
	{
		this->encoder.Close();
		this->outputFile.close();
		this->closed = true;
	}
}


//
// EncodeRGBToH264Live
//
EncodeRGBToH264Live::EncodeRGBToH264Live(FFmpegFactory &ffmpeg) :
	closed(false)
{
	this->streamer.reset(new H264NetworkSender());
	this->encoder.reset(new EncodeRGBToH264(ffmpeg, std::bind(&EncodeRGBToH264Live::ProcessEncodedFrame, this, std::placeholders::_1)));
}

EncodeRGBToH264Live::~EncodeRGBToH264Live()
{
	this->Close();
}

void EncodeRGBToH264Live::Start(std::string& ipAddress, std::string& port, int width, int height, int fps)
{
	std::string networkAddress("rtp://" + ipAddress + ":" + port);

	this->streamer->Start(networkAddress, width, height, fps);
	this->encoder->Start(width, height, fps);
}

void EncodeRGBToH264Live::WriteFrame(const uint8_t *srcBytes)
{
	this->encoder->WriteFrame(srcBytes);
}

void EncodeRGBToH264Live::ProcessEncodedFrame(AVPacket& packet)
{
	this->streamer->WriteFrame(packet);
}

void EncodeRGBToH264Live::Close()
{
	if (!this->closed)
	{
		this->encoder->Close();
		this->streamer->Close();
		this->closed = true;
	}
}

//
// EncodeH264LiveToRGB
//
void EncodeH264LiveToRGB::ProcessEncodedFrame(AVPacket& packet)
{
	outputFile.write((char *)packet.buf->data, packet.buf->size);
}

EncodeH264LiveToRGB::EncodeH264LiveToRGB(FFmpegFactory &ffmpeg) :
	closed(false),
	outputFile(std::ofstream("received.h264", std::ofstream::binary))
{
	this->receiver.reset(new H264NetworkReceiver(ffmpeg));
}

EncodeH264LiveToRGB::~EncodeH264LiveToRGB()
{
	this->Close();
}

void EncodeH264LiveToRGB::Start(std::string& ipAddress, std::string& port, RGBFrameCallback rgbFrameCallback)
{
	this->receiver->Start(std::bind(&EncodeH264LiveToRGB::ProcessEncodedFrame, this, std::placeholders::_1));
}

void EncodeH264LiveToRGB::Close()
{
	if (!this->closed)
	{
		this->closed = true;
		this->receiver->Close();
		this->outputFile.close();
	}
}