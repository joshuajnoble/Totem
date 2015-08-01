#pragma once

#include <Windows.h>
#include <cassert>
#include <string>

extern "C"
{
//#include "ffmpeg.h"
#include "libavdevice/avdevice.h"
}

#define FFMPEG_IMPORT(X) do { this->##X = (decltype(##X))GetProcAddress(this->hmodule, #X); assert(this->##X != NULL); } while(0)

namespace FFmpegWrapper {
	class FFmpegDllImport
	{
	protected:
		bool isHmoduleOwner = false;
		HMODULE hmodule;

	public:
		~FFmpegDllImport() {
			if (this->isHmoduleOwner) {
				FreeLibrary(this->hmodule);
			}
		}

		FFmpegDllImport(HMODULE hmodule)
		{
			this->hmodule = hmodule;
		}

		FFmpegDllImport(const std::string dllName)
		{
			this->hmodule = LoadLibraryA(dllName.c_str());
			if (this->hmodule) {
				this->isHmoduleOwner = true;
			}
		}
	};

	class avdevice : public FFmpegDllImport
	{
	private:
		void init() {
			FFMPEG_IMPORT(avdevice_version);
			FFMPEG_IMPORT(avdevice_configuration);
			FFMPEG_IMPORT(avdevice_license);
			FFMPEG_IMPORT(avdevice_register_all);
			FFMPEG_IMPORT(av_input_audio_device_next);
			FFMPEG_IMPORT(av_input_video_device_next);
			FFMPEG_IMPORT(av_output_audio_device_next);
			FFMPEG_IMPORT(avdevice_app_to_dev_control_message);
			FFMPEG_IMPORT(avdevice_dev_to_app_control_message);
			FFMPEG_IMPORT(avdevice_capabilities_create);
			FFMPEG_IMPORT(avdevice_capabilities_free);
			FFMPEG_IMPORT(avdevice_list_input_sources);
			FFMPEG_IMPORT(avdevice_free_list_devices);
			FFMPEG_IMPORT(avdevice_list_devices);
			FFMPEG_IMPORT(avdevice_list_output_sinks);
		}

	public:
		avdevice(const std::string dllName) : FFmpegDllImport(dllName) { init(); }
		avdevice(HMODULE hmodulle) : FFmpegDllImport(hmodule) { init(); }

		decltype(avdevice_version)* avdevice_version;
		decltype(avdevice_configuration)* avdevice_configuration;
		decltype(avdevice_license)* avdevice_license;
		decltype(avdevice_register_all)* avdevice_register_all;
		decltype(av_input_audio_device_next)* av_input_audio_device_next;
		decltype(av_input_video_device_next)* av_input_video_device_next;
		decltype(av_output_audio_device_next)* av_output_audio_device_next;
		decltype(av_output_video_device_next)* av_output_video_device_next;
		decltype(avdevice_app_to_dev_control_message)* avdevice_app_to_dev_control_message;
		decltype(avdevice_dev_to_app_control_message)* avdevice_dev_to_app_control_message;
		decltype(avdevice_capabilities_create)* avdevice_capabilities_create;
		decltype(avdevice_capabilities_free)* avdevice_capabilities_free;
		decltype(avdevice_list_devices)* avdevice_list_devices;
		decltype(avdevice_free_list_devices)* avdevice_free_list_devices;
		decltype(avdevice_list_input_sources)* avdevice_list_input_sources;
		decltype(avdevice_list_output_sinks)* avdevice_list_output_sinks;
	};
}