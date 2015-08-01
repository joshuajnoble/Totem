#pragma once

#include <Windows.h>
#include <cassert>
#include <string>

extern "C"
{
	//#include "ffmpeg.h"
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
}

#define FFMPEG_IMPORT(X) do { this->##X = (decltype(##X))GetProcAddress(this->hmodule, #X); assert(this->##X != NULL); } while(0)

namespace FFmpegWrapper {
	class FFmpegDllImport
	{
	protected:
		bool isHmoduleOwner = false;
		HMODULE hmodule;

	public:
		~FFmpegDllImport()
		{
			if (this->isHmoduleOwner)
			{
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
			if (this->hmodule)
			{
				this->isHmoduleOwner = true;
			}
		}
	};

	class avdevice : public FFmpegDllImport
	{
	private:
		void init()
		{
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

	class avcodec : public FFmpegDllImport
	{
	private:
		void init()
		{
			FFMPEG_IMPORT(av_codec_next);
			FFMPEG_IMPORT(avcodec_version);
			FFMPEG_IMPORT(avcodec_configuration);
			FFMPEG_IMPORT(avcodec_license);
			FFMPEG_IMPORT(avcodec_register);
			FFMPEG_IMPORT(avcodec_register_all);
			FFMPEG_IMPORT(avcodec_alloc_context3);
			FFMPEG_IMPORT(avcodec_free_context);
			FFMPEG_IMPORT(avcodec_get_context_defaults3);
			FFMPEG_IMPORT(avcodec_get_class);
			FFMPEG_IMPORT(avcodec_get_frame_class);
			FFMPEG_IMPORT(avcodec_get_subtitle_rect_class);
			FFMPEG_IMPORT(avcodec_copy_context);
			FFMPEG_IMPORT(avcodec_alloc_frame);
			FFMPEG_IMPORT(avcodec_get_frame_defaults);
			FFMPEG_IMPORT(avcodec_free_frame);
			FFMPEG_IMPORT(avcodec_open2);
			FFMPEG_IMPORT(avcodec_close);
			FFMPEG_IMPORT(avsubtitle_free);
			FFMPEG_IMPORT(av_destruct_packet);
			FFMPEG_IMPORT(av_init_packet);
			FFMPEG_IMPORT(av_new_packet);
			FFMPEG_IMPORT(av_shrink_packet);
			FFMPEG_IMPORT(av_grow_packet);
			FFMPEG_IMPORT(av_packet_from_data);
			FFMPEG_IMPORT(av_dup_packet);
			FFMPEG_IMPORT(av_copy_packet);
			FFMPEG_IMPORT(av_copy_packet_side_data);
			FFMPEG_IMPORT(av_free_packet);
			FFMPEG_IMPORT(av_packet_new_side_data);
			FFMPEG_IMPORT(av_packet_shrink_side_data);
			FFMPEG_IMPORT(av_packet_get_side_data);
			FFMPEG_IMPORT(av_packet_merge_side_data);
			FFMPEG_IMPORT(av_packet_split_side_data);
			FFMPEG_IMPORT(av_packet_side_data_name);
			FFMPEG_IMPORT(av_packet_pack_dictionary);
			FFMPEG_IMPORT(av_packet_unpack_dictionary);
			FFMPEG_IMPORT(av_packet_free_side_data);
			FFMPEG_IMPORT(av_packet_ref);
			FFMPEG_IMPORT(av_packet_unref);
			FFMPEG_IMPORT(av_packet_move_ref);
			FFMPEG_IMPORT(av_packet_copy_props);
			FFMPEG_IMPORT(av_packet_rescale_ts);
			FFMPEG_IMPORT(avcodec_find_decoder);
			FFMPEG_IMPORT(avcodec_find_decoder_by_name);
			FFMPEG_IMPORT(avcodec_default_get_buffer);
			FFMPEG_IMPORT(avcodec_default_release_buffer);
			FFMPEG_IMPORT(avcodec_default_reget_buffer);
			FFMPEG_IMPORT(avcodec_default_get_buffer2);
			FFMPEG_IMPORT(avcodec_get_edge_width);
			FFMPEG_IMPORT(avcodec_align_dimensions);
			FFMPEG_IMPORT(avcodec_align_dimensions2);
			FFMPEG_IMPORT(avcodec_enum_to_chroma_pos);
			FFMPEG_IMPORT(avcodec_chroma_pos_to_enum);
			FFMPEG_IMPORT(avcodec_decode_audio3);
			FFMPEG_IMPORT(avcodec_decode_audio4);
			FFMPEG_IMPORT(avcodec_decode_video2);
			FFMPEG_IMPORT(avcodec_decode_subtitle2);
			FFMPEG_IMPORT(av_parser_next);
			FFMPEG_IMPORT(av_register_codec_parser);
			FFMPEG_IMPORT(av_parser_init);
			FFMPEG_IMPORT(av_parser_parse2);
			FFMPEG_IMPORT(av_parser_change);
			FFMPEG_IMPORT(av_parser_close);
			FFMPEG_IMPORT(avcodec_find_encoder);
			FFMPEG_IMPORT(avcodec_find_encoder_by_name);
			FFMPEG_IMPORT(avcodec_encode_audio);
			FFMPEG_IMPORT(avcodec_encode_audio2);
			FFMPEG_IMPORT(avcodec_encode_video);
			FFMPEG_IMPORT(avcodec_encode_video2);
			FFMPEG_IMPORT(avcodec_encode_subtitle);
			FFMPEG_IMPORT(av_audio_resample_init);
			FFMPEG_IMPORT(audio_resample);
			FFMPEG_IMPORT(audio_resample_close);
			FFMPEG_IMPORT(av_resample_init);
			FFMPEG_IMPORT(av_resample);
			FFMPEG_IMPORT(av_resample_compensate);
			FFMPEG_IMPORT(av_resample_close);
			FFMPEG_IMPORT(avpicture_alloc);
			FFMPEG_IMPORT(avpicture_free);
			FFMPEG_IMPORT(avpicture_fill);
			FFMPEG_IMPORT(avpicture_layout);
			FFMPEG_IMPORT(avpicture_get_size);
			FFMPEG_IMPORT(avpicture_deinterlace);
			FFMPEG_IMPORT(av_picture_copy);
			FFMPEG_IMPORT(av_picture_crop);
			FFMPEG_IMPORT(av_picture_pad);
			FFMPEG_IMPORT(avcodec_get_chroma_sub_sample);
			FFMPEG_IMPORT(avcodec_pix_fmt_to_codec_tag);
			FFMPEG_IMPORT(avcodec_get_pix_fmt_loss);
			FFMPEG_IMPORT(avcodec_find_best_pix_fmt_of_list);
			FFMPEG_IMPORT(avcodec_find_best_pix_fmt_of_2);
			FFMPEG_IMPORT(avcodec_find_best_pix_fmt2);
			FFMPEG_IMPORT(avcodec_default_get_format);
			FFMPEG_IMPORT(avcodec_set_dimensions);
			FFMPEG_IMPORT(av_get_codec_tag_string);
			FFMPEG_IMPORT(avcodec_string);
			FFMPEG_IMPORT(av_get_profile_name);
			FFMPEG_IMPORT(avcodec_default_execute);
			FFMPEG_IMPORT(avcodec_default_execute2);
			FFMPEG_IMPORT(avcodec_fill_audio_frame);
			FFMPEG_IMPORT(avcodec_flush_buffers);
			FFMPEG_IMPORT(av_get_bits_per_sample);
			FFMPEG_IMPORT(av_get_pcm_codec);
			FFMPEG_IMPORT(av_get_exact_bits_per_sample);
			FFMPEG_IMPORT(av_get_audio_frame_duration);
			FFMPEG_IMPORT(av_register_bitstream_filter);
			FFMPEG_IMPORT(av_bitstream_filter_init);
			FFMPEG_IMPORT(av_bitstream_filter_filter);
			FFMPEG_IMPORT(av_bitstream_filter_close);
			FFMPEG_IMPORT(av_bitstream_filter_next);
			FFMPEG_IMPORT(av_fast_padded_malloc);
			FFMPEG_IMPORT(av_fast_padded_mallocz);
			FFMPEG_IMPORT(av_xiphlacing);
			FFMPEG_IMPORT(av_log_missing_feature);
			FFMPEG_IMPORT(av_log_ask_for_sample);
			FFMPEG_IMPORT(av_register_hwaccel);
			FFMPEG_IMPORT(av_hwaccel_next);
			FFMPEG_IMPORT(av_lockmgr_register);
			FFMPEG_IMPORT(avcodec_get_type);
			FFMPEG_IMPORT(avcodec_get_name);
			FFMPEG_IMPORT(avcodec_is_open);
			FFMPEG_IMPORT(av_codec_is_encoder);
			FFMPEG_IMPORT(av_codec_is_decoder);
			FFMPEG_IMPORT(avcodec_descriptor_get);
			FFMPEG_IMPORT(avcodec_descriptor_next);
			FFMPEG_IMPORT(avcodec_descriptor_get_by_name);
		}

	public:
		avcodec(const std::string dllName) : FFmpegDllImport(dllName) { init(); }
		avcodec(HMODULE hmodulle) : FFmpegDllImport(hmodule) { init(); }
		
		decltype(av_codec_next)* av_codec_next;
		decltype(avcodec_version) * avcodec_version;
		decltype(avcodec_configuration) * avcodec_configuration;
		decltype(avcodec_license) * avcodec_license;
		decltype(avcodec_register) * avcodec_register;
		decltype(avcodec_register_all)* avcodec_register_all;
		decltype(avcodec_alloc_context3)* avcodec_alloc_context3;
		decltype(avcodec_free_context)* avcodec_free_context;
		decltype(avcodec_get_context_defaults3)* avcodec_get_context_defaults3;
		decltype(avcodec_get_class)* avcodec_get_class;
		decltype(avcodec_get_frame_class)* avcodec_get_frame_class;
		decltype(avcodec_get_subtitle_rect_class)* avcodec_get_subtitle_rect_class;
		decltype(avcodec_copy_context)* avcodec_copy_context;
		decltype(avcodec_alloc_frame)* avcodec_alloc_frame;
		decltype(avcodec_get_frame_defaults)* avcodec_get_frame_defaults;
		decltype(avcodec_free_frame)* avcodec_free_frame;
		decltype(avcodec_open2)* avcodec_open2;
		decltype(avcodec_close)* avcodec_close;
		decltype(avsubtitle_free)* avsubtitle_free;
		decltype(av_destruct_packet)* av_destruct_packet;
		decltype(av_init_packet)* av_init_packet;
		decltype(av_new_packet)* av_new_packet;
		decltype(av_shrink_packet)* av_shrink_packet;
		decltype(av_grow_packet)* av_grow_packet;
		decltype(av_packet_from_data)* av_packet_from_data;
		decltype(av_dup_packet)* av_dup_packet;
		decltype(av_copy_packet)* av_copy_packet;
		decltype(av_copy_packet_side_data)* av_copy_packet_side_data;
		decltype(av_free_packet)* av_free_packet;
		decltype(av_packet_new_side_data)* av_packet_new_side_data;
		decltype(av_packet_shrink_side_data)* av_packet_shrink_side_data;
		decltype(av_packet_get_side_data)* av_packet_get_side_data;
		decltype(av_packet_merge_side_data)* av_packet_merge_side_data;
		decltype(av_packet_split_side_data)* av_packet_split_side_data;
		decltype(av_packet_side_data_name)* av_packet_side_data_name;
		decltype(av_packet_pack_dictionary)* av_packet_pack_dictionary;
		decltype(av_packet_unpack_dictionary)* av_packet_unpack_dictionary;
		decltype(av_packet_free_side_data)* av_packet_free_side_data;
		decltype(av_packet_ref)* av_packet_ref;
		decltype(av_packet_unref)* av_packet_unref;
		decltype(av_packet_move_ref)* av_packet_move_ref;
		decltype(av_packet_copy_props)* av_packet_copy_props;
		decltype(av_packet_rescale_ts)* av_packet_rescale_ts;
		decltype(avcodec_find_decoder)* avcodec_find_decoder;
		decltype(avcodec_find_decoder_by_name)* avcodec_find_decoder_by_name;
		decltype(avcodec_default_get_buffer)* avcodec_default_get_buffer;
		decltype(avcodec_default_release_buffer)* avcodec_default_release_buffer;
		decltype(avcodec_default_reget_buffer)* avcodec_default_reget_buffer;
		decltype(avcodec_default_get_buffer2)* avcodec_default_get_buffer2;
		decltype(avcodec_get_edge_width)* avcodec_get_edge_width;
		decltype(avcodec_align_dimensions)* avcodec_align_dimensions;
		decltype(avcodec_align_dimensions2)* avcodec_align_dimensions2;
		decltype(avcodec_enum_to_chroma_pos)* avcodec_enum_to_chroma_pos;
		decltype(avcodec_chroma_pos_to_enum)* avcodec_chroma_pos_to_enum;
		decltype(avcodec_decode_audio3)* avcodec_decode_audio3;
		decltype(avcodec_decode_audio4)* avcodec_decode_audio4;
		decltype(avcodec_decode_video2)* avcodec_decode_video2;
		decltype(avcodec_decode_subtitle2)* avcodec_decode_subtitle2;
		decltype(av_parser_next)* av_parser_next;
		decltype(av_register_codec_parser)* av_register_codec_parser;
		decltype(av_parser_init)* av_parser_init;
		decltype(av_parser_parse2)* av_parser_parse2;
		decltype(av_parser_change)* av_parser_change;
		decltype(av_parser_close)* av_parser_close;
		decltype(avcodec_find_encoder)* avcodec_find_encoder;
		decltype(avcodec_find_encoder_by_name)* avcodec_find_encoder_by_name;
		decltype(avcodec_encode_audio)* avcodec_encode_audio;
		decltype(avcodec_encode_audio2)* avcodec_encode_audio2;
		decltype(avcodec_encode_video)* avcodec_encode_video;
		decltype(avcodec_encode_video2)* avcodec_encode_video2;
		decltype(avcodec_encode_subtitle)* avcodec_encode_subtitle;
		decltype(av_audio_resample_init)* av_audio_resample_init;
		decltype(audio_resample)* audio_resample;
		decltype(audio_resample_close)* audio_resample_close;
		decltype(av_resample_init)* av_resample_init;
		decltype(av_resample)* av_resample;
		decltype(av_resample_compensate)* av_resample_compensate;
		decltype(av_resample_close)* av_resample_close;
		decltype(avpicture_alloc)* avpicture_alloc;
		decltype(avpicture_free)* avpicture_free;
		decltype(avpicture_fill)* avpicture_fill;
		decltype(avpicture_layout)* avpicture_layout;
		decltype(avpicture_get_size)* avpicture_get_size;
		decltype(avpicture_deinterlace)* avpicture_deinterlace;
		decltype(av_picture_copy)* av_picture_copy;
		decltype(av_picture_crop)* av_picture_crop;
		decltype(av_picture_pad)* av_picture_pad;
		decltype(avcodec_get_chroma_sub_sample)* avcodec_get_chroma_sub_sample;
		decltype(avcodec_pix_fmt_to_codec_tag)* avcodec_pix_fmt_to_codec_tag;
		decltype(avcodec_get_pix_fmt_loss)* avcodec_get_pix_fmt_loss;
		decltype(avcodec_find_best_pix_fmt_of_list)* avcodec_find_best_pix_fmt_of_list;
		decltype(avcodec_find_best_pix_fmt_of_2)* avcodec_find_best_pix_fmt_of_2;
		decltype(avcodec_find_best_pix_fmt2)* avcodec_find_best_pix_fmt2;
		decltype(avcodec_default_get_format)* avcodec_default_get_format;
		decltype(avcodec_set_dimensions)* avcodec_set_dimensions;
		decltype(av_get_codec_tag_string)* av_get_codec_tag_string;
		decltype(avcodec_string)* avcodec_string;
		decltype(av_get_profile_name)* av_get_profile_name;
		decltype(avcodec_default_execute)* avcodec_default_execute;
		decltype(avcodec_default_execute2)* avcodec_default_execute2;
		decltype(avcodec_fill_audio_frame)* avcodec_fill_audio_frame;
		decltype(avcodec_flush_buffers)* avcodec_flush_buffers;
		decltype(av_get_bits_per_sample)* av_get_bits_per_sample;
		decltype(av_get_pcm_codec)* av_get_pcm_codec;
		decltype(av_get_exact_bits_per_sample)* av_get_exact_bits_per_sample;
		decltype(av_get_audio_frame_duration)* av_get_audio_frame_duration;
		decltype(av_register_bitstream_filter)* av_register_bitstream_filter;
		decltype(av_bitstream_filter_init)* av_bitstream_filter_init;
		decltype(av_bitstream_filter_filter)* av_bitstream_filter_filter;
		decltype(av_bitstream_filter_close)* av_bitstream_filter_close;
		decltype(av_bitstream_filter_next)* av_bitstream_filter_next;
		decltype(av_fast_padded_malloc)* av_fast_padded_malloc;
		decltype(av_fast_padded_mallocz)* av_fast_padded_mallocz;
		decltype(av_xiphlacing)* av_xiphlacing;
		decltype(av_log_missing_feature)* av_log_missing_feature;
		decltype(av_log_ask_for_sample)* av_log_ask_for_sample;
		decltype(av_register_hwaccel)* av_register_hwaccel;
		decltype(av_hwaccel_next)* av_hwaccel_next;
		decltype(av_lockmgr_register)* av_lockmgr_register;
		decltype(avcodec_get_type)* avcodec_get_type;
		decltype(avcodec_get_name)* avcodec_get_name;
		decltype(avcodec_is_open)* avcodec_is_open;
		decltype(av_codec_is_encoder)* av_codec_is_encoder;
		decltype(av_codec_is_decoder)* av_codec_is_decoder;
		decltype(avcodec_descriptor_get)* avcodec_descriptor_get;
		decltype(avcodec_descriptor_next)* avcodec_descriptor_next;
		decltype(avcodec_descriptor_get_by_name)* avcodec_descriptor_get_by_name;
	};

	class imgutils: public FFmpegDllImport
	{
	private:
		void init()
		{
			FFMPEG_IMPORT(av_image_fill_max_pixsteps);
			FFMPEG_IMPORT(av_image_get_linesize);
			FFMPEG_IMPORT(av_image_fill_linesizes);
			FFMPEG_IMPORT(av_image_fill_pointers);
			FFMPEG_IMPORT(av_image_alloc);
			FFMPEG_IMPORT(av_image_copy_plane);
			FFMPEG_IMPORT(av_image_copy);
			FFMPEG_IMPORT(av_image_fill_arrays);
			FFMPEG_IMPORT(av_image_get_buffer_size);
			FFMPEG_IMPORT(av_image_copy_to_buffer);
			FFMPEG_IMPORT(av_image_check_size);
			FFMPEG_IMPORT(av_image_check_sar);
		}

		public:
			imgutils(const std::string dllName) : FFmpegDllImport(dllName) { init(); }
			imgutils(HMODULE hmodulle) : FFmpegDllImport(hmodule) { init(); }

			decltype(av_image_fill_max_pixsteps)* av_image_fill_max_pixsteps;
			decltype(av_image_get_linesize)* av_image_get_linesize;
			decltype(av_image_fill_linesizes)* av_image_fill_linesizes;
			decltype(av_image_fill_pointers)* av_image_fill_pointers;
			decltype(av_image_alloc)* av_image_alloc;
			decltype(av_image_copy_plane)* av_image_copy_plane;
			decltype(av_image_copy)* av_image_copy;
			decltype(av_image_fill_arrays)* av_image_fill_arrays;
			decltype(av_image_get_buffer_size)* av_image_get_buffer_size;
			decltype(av_image_copy_to_buffer)* av_image_copy_to_buffer;
			decltype(av_image_check_size)* av_image_check_size;
			decltype(av_image_check_sar)* av_image_check_sar;
	};
}