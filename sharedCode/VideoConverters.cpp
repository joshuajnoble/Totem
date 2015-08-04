#include "VideoConverters.h"

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
	if (!this->sws_context) throw std::exception("Could not allocate convetsion context.");

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
EncodeRGBToH264File::EncodeRGBToH264File(FFmpegFactory &ffmpeg, const std::string& filename) : closed(false),
encoder(ffmpeg, std::bind(&EncodeRGBToH264File::ProcessEncodedFrame, this, std::placeholders::_1, std::placeholders::_2))
{
	this->outptuFile = std::ofstream(filename, std::ofstream::binary);
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

void EncodeRGBToH264File::ProcessEncodedFrame(const uint8_t* buffer, int count)
{
	this->outptuFile.write(reinterpret_cast<const char *>(buffer), count);
}

void EncodeRGBToH264File::Close()
{
	if (!this->closed)
	{
		this->encoder.Close();
		this->outptuFile.close();
		this->closed = true;
	}
}