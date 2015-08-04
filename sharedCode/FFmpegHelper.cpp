#include "FFmpegHelper.h"

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