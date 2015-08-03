#include "FFmpegHelper.h"

//
// ConvertToNV12
//
ConvertToNV12::ConvertToNV12(FFmpegFactory& ffmpeg, int width, int height) :
	width(width),
	height(height),
	m_ffmpeg(ffmpeg)
{
	// Asume we are converting from RGB24
	sws_context = m_ffmpeg.scale.sws_getContext(
		width, height, AV_PIX_FMT_RGB24,
		width, height, AV_PIX_FMT_NV12,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (!sws_context) throw std::exception("Could not allocate convetsion context.");

	srcLineSize[0] = 3 * width;
	dstLineSize[0] = width;
	dstLineSize[1] = width;
}

ConvertToNV12::~ConvertToNV12()
{
	if (sws_context)
	{
		m_ffmpeg.scale.sws_freeContext(sws_context);
		sws_context = NULL;
	}
}

void ConvertToNV12::ConvertFrame(const uint8_t *rgbSource, uint8_t* yuvDestination)
{
	srcData[0] = const_cast<uint8_t*>(rgbSource);

	dstData[0] = yuvDestination;
	dstData[1] = yuvDestination + height * dstLineSize[0];
	m_ffmpeg.scale.sws_scale(sws_context, srcData, srcLineSize, 0, height, dstData, dstLineSize);
}

//
// EncodeRGBToH264
//
EncodeRGBToH264::EncodeRGBToH264(FFmpegFactory &ffmpeg) : m_ffmpeg(ffmpeg), closed(false)
{
}

EncodeRGBToH264::~EncodeRGBToH264()
{
	this->Stop();
}

void EncodeRGBToH264::Start(int width, int height, int fps)
{
	this->converter.reset(new ConvertToNV12(m_ffmpeg, width, height));
	this->encoder.reset(new YUV420_H264_Encoder(m_ffmpeg, "test.h264", width, height, fps));
	this->outputBuffer.resize(this->encoder->getRawFrameSize());
}

void EncodeRGBToH264::EncodeFrame(const uint8_t *srcBytes)
{
	this->converter->ConvertFrame(srcBytes, this->outputBuffer.data());
	this->encoder->EncodeFrame(this->outputBuffer.data());
}

void EncodeRGBToH264::Stop()
{
	if (!this->closed)
	{
		this->encoder->Close();
		this->closed = true;
	}
}

//
// FFmpegHelper
//
FFmpegHelper::FFmpegHelper()
{
	//JustForTesting test(this->m_ffmpeg);
}

FFmpegHelper::~FFmpegHelper()
{
}

std::vector<std::auto_ptr<AVInputFormat>> FFmpegHelper::GetDeviceList()
{
	auto rval = std::vector<std::auto_ptr<AVInputFormat>>();
	AVInputFormat *fmt = NULL;
	do
	{
		fmt = m_ffmpeg->device.av_input_video_device_next(fmt);
		rval.push_back(std::auto_ptr<AVInputFormat>(fmt));
	} while (fmt);

	return rval;
}


std::vector<std::auto_ptr<AVDeviceInfoList>> FFmpegHelper::GetInputSource(const std::string sourceName)
{
	// Examples:
	// "Logitech HD Pro Webcam C920"
	// "Point Grey Camera"
	// "HP Truevision HD"
	auto rval = std::vector<std::auto_ptr<AVDeviceInfoList>>();
	AVDeviceInfoList *device_list = NULL;
	auto count = m_ffmpeg->device.avdevice_list_input_sources(NULL, sourceName.c_str(), NULL, &device_list);

	if (count > 0)
	{
		char output[1024];
		for (auto i = 0; i < device_list->nb_devices; i++) {
			sprintf_s(output, sizeof(output), "  %s %s [%s]\n", device_list->default_device == i ? "*" : " ", device_list->devices[i]->device_name, device_list->devices[i]->device_description);
			OutputDebugStringA(output);
		}
	}

	return rval;
}