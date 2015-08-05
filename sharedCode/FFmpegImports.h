#pragma once

#include <Windows.h>
#include <cassert>
#include <string>
#include <functional>
#include <stdint.h>

extern "C"
{
	//#include "ffmpeg.h"
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/time.h"
#include "libavutil/mathematics.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libswscale/swscale.h"
}

//typedef std::function<void(const uint8_t*, int)> FrameCallback;
typedef std::function<void(AVPacket&)> FrameCallback;
typedef std::function<void(const AVFrame&)> DecodedFrameCallback;

#define FFMPEG_IMPORT(X) do { this->##X = (decltype(##X))GetProcAddress(this->hmodule, #X); assert(this->##X != NULL); } while(0)

namespace FFmpegWrapper
{

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

			this->avdevice_register_all();
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

			this->avcodec_register_all();
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

	class utils: public FFmpegDllImport
	{
	private:
		void init()
		{
			// imgutils.h
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

			// frame.h
			FFMPEG_IMPORT(av_frame_alloc);
			FFMPEG_IMPORT(av_frame_free);
			
			// mem.h
			FFMPEG_IMPORT(av_malloc);
			FFMPEG_IMPORT(av_free);
			FFMPEG_IMPORT(av_freep);

			// dict.h
			FFMPEG_IMPORT(av_dict_set);

			// opt.h
			FFMPEG_IMPORT(av_opt_show2);
			FFMPEG_IMPORT(av_opt_set_defaults);
			FFMPEG_IMPORT(av_set_options_string);
			FFMPEG_IMPORT(av_opt_set_from_string);
			FFMPEG_IMPORT(av_opt_free);
			FFMPEG_IMPORT(av_opt_flag_is_set);
			FFMPEG_IMPORT(av_opt_set_dict);
			FFMPEG_IMPORT(av_opt_set_dict2);
			FFMPEG_IMPORT(av_opt_get_key_value);
			FFMPEG_IMPORT(av_opt_eval_flags);
			FFMPEG_IMPORT(av_opt_eval_int);
			FFMPEG_IMPORT(av_opt_eval_int64);
			FFMPEG_IMPORT(av_opt_eval_float);
			FFMPEG_IMPORT(av_opt_eval_double);
			FFMPEG_IMPORT(av_opt_eval_q);
			FFMPEG_IMPORT(av_opt_find);
			FFMPEG_IMPORT(av_opt_find2);
			FFMPEG_IMPORT(av_opt_next);
			FFMPEG_IMPORT(av_opt_child_next);
			FFMPEG_IMPORT(av_opt_child_class_next);
			FFMPEG_IMPORT(av_opt_set);
			FFMPEG_IMPORT(av_opt_set_int);
			FFMPEG_IMPORT(av_opt_set_double);
			FFMPEG_IMPORT(av_opt_set_q);
			FFMPEG_IMPORT(av_opt_set_bin);
			FFMPEG_IMPORT(av_opt_set_image_size);
			FFMPEG_IMPORT(av_opt_set_pixel_fmt);
			FFMPEG_IMPORT(av_opt_set_sample_fmt);
			FFMPEG_IMPORT(av_opt_set_video_rate);
			FFMPEG_IMPORT(av_opt_set_channel_layout);
			FFMPEG_IMPORT(av_opt_set_dict_val);
			FFMPEG_IMPORT(av_opt_get);
			FFMPEG_IMPORT(av_opt_get_int);
			FFMPEG_IMPORT(av_opt_get_double);
			FFMPEG_IMPORT(av_opt_get_q);
			FFMPEG_IMPORT(av_opt_get_image_size);
			FFMPEG_IMPORT(av_opt_get_pixel_fmt);
			FFMPEG_IMPORT(av_opt_get_sample_fmt);
			FFMPEG_IMPORT(av_opt_get_video_rate);
			FFMPEG_IMPORT(av_opt_get_channel_layout);
			FFMPEG_IMPORT(av_opt_get_dict_val);
			FFMPEG_IMPORT(av_opt_ptr);
			FFMPEG_IMPORT(av_opt_freep_ranges);
			FFMPEG_IMPORT(av_opt_query_ranges);
			FFMPEG_IMPORT(av_opt_copy);
			FFMPEG_IMPORT(av_opt_query_ranges_default);
			FFMPEG_IMPORT(av_opt_is_set_to_default);
			FFMPEG_IMPORT(av_opt_is_set_to_default_by_name);
			FFMPEG_IMPORT(av_opt_serialize);

			// pixdesc.h
			FFMPEG_IMPORT(av_read_image_line);
			FFMPEG_IMPORT(av_write_image_line);
			FFMPEG_IMPORT(av_get_pix_fmt);
			FFMPEG_IMPORT(av_get_pix_fmt_name);
			FFMPEG_IMPORT(av_get_pix_fmt_string);
			FFMPEG_IMPORT(av_get_bits_per_pixel);
			FFMPEG_IMPORT(av_get_padded_bits_per_pixel);
			FFMPEG_IMPORT(av_pix_fmt_desc_get);
			FFMPEG_IMPORT(av_pix_fmt_desc_next);
			FFMPEG_IMPORT(av_pix_fmt_desc_get_id);
			FFMPEG_IMPORT(av_pix_fmt_get_chroma_sub_sample);
			FFMPEG_IMPORT(av_pix_fmt_count_planes);
			FFMPEG_IMPORT(av_pix_fmt_swap_endianness);
			FFMPEG_IMPORT(av_get_pix_fmt_loss);
			FFMPEG_IMPORT(av_find_best_pix_fmt_of_2);
			FFMPEG_IMPORT(av_color_range_name);
			FFMPEG_IMPORT(av_color_primaries_name);
			FFMPEG_IMPORT(av_color_transfer_name);
			FFMPEG_IMPORT(av_color_space_name);
			FFMPEG_IMPORT(av_chroma_location_name);

			// time.h
			FFMPEG_IMPORT(av_gettime);
			FFMPEG_IMPORT(av_gettime_relative);
			FFMPEG_IMPORT(av_gettime_relative_is_monotonic);
			FFMPEG_IMPORT(av_usleep);

			// mathematics.h
			FFMPEG_IMPORT(av_gcd);
			FFMPEG_IMPORT(av_rescale);
			FFMPEG_IMPORT(av_rescale_rnd);
			FFMPEG_IMPORT(av_rescale_q);
			FFMPEG_IMPORT(av_rescale_q_rnd);
			FFMPEG_IMPORT(av_compare_ts);
			FFMPEG_IMPORT(av_compare_mod);
			FFMPEG_IMPORT(av_rescale_delta);
			FFMPEG_IMPORT(av_add_stable);
		}

		public:
			utils(const std::string dllName) : FFmpegDllImport(dllName) { init(); }
			utils(HMODULE hmodulle) : FFmpegDllImport(hmodule) { init(); }

			// imgutils.h
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

			// frame.h
			decltype(av_frame_alloc)* av_frame_alloc;
			decltype(av_frame_free)* av_frame_free;

			// mem.h
			decltype(av_malloc)* av_malloc;
			decltype(av_free)* av_free;
			decltype(av_freep)* av_freep;

			// dict.h
			decltype(av_dict_set)* av_dict_set;

			// opt.h
			decltype(av_opt_show2)* av_opt_show2;
			decltype(av_opt_set_defaults)* av_opt_set_defaults;
			decltype(av_set_options_string)* av_set_options_string;
			decltype(av_opt_set_from_string)* av_opt_set_from_string;
			decltype(av_opt_free)* av_opt_free;
			decltype(av_opt_flag_is_set)* av_opt_flag_is_set;
			decltype(av_opt_set_dict)* av_opt_set_dict;
			decltype(av_opt_set_dict2)* av_opt_set_dict2;
			decltype(av_opt_get_key_value)* av_opt_get_key_value;
			decltype(av_opt_eval_flags)* av_opt_eval_flags;
			decltype(av_opt_eval_int)* av_opt_eval_int;
			decltype(av_opt_eval_int64)* av_opt_eval_int64;
			decltype(av_opt_eval_float)* av_opt_eval_float;
			decltype(av_opt_eval_double)* av_opt_eval_double;
			decltype(av_opt_eval_q)* av_opt_eval_q;
			decltype(av_opt_find)* av_opt_find;
			decltype(av_opt_find2)* av_opt_find2;
			decltype(av_opt_next)* av_opt_next;
			decltype(av_opt_child_next)* av_opt_child_next;
			decltype(av_opt_child_class_next)* av_opt_child_class_next;
			decltype(av_opt_set)* av_opt_set;
			decltype(av_opt_set_int)* av_opt_set_int;
			decltype(av_opt_set_double)* av_opt_set_double;
			decltype(av_opt_set_q)* av_opt_set_q;
			decltype(av_opt_set_bin)* av_opt_set_bin;
			decltype(av_opt_set_image_size)* av_opt_set_image_size;
			decltype(av_opt_set_pixel_fmt)* av_opt_set_pixel_fmt;
			decltype(av_opt_set_sample_fmt)* av_opt_set_sample_fmt;
			decltype(av_opt_set_video_rate)* av_opt_set_video_rate;
			decltype(av_opt_set_channel_layout)* av_opt_set_channel_layout;
			decltype(av_opt_set_dict_val)* av_opt_set_dict_val;
			decltype(av_opt_get)* av_opt_get;
			decltype(av_opt_get_int)* av_opt_get_int;
			decltype(av_opt_get_double)* av_opt_get_double;
			decltype(av_opt_get_q)* av_opt_get_q;
			decltype(av_opt_get_image_size)* av_opt_get_image_size;
			decltype(av_opt_get_pixel_fmt)* av_opt_get_pixel_fmt;
			decltype(av_opt_get_sample_fmt)* av_opt_get_sample_fmt;
			decltype(av_opt_get_video_rate)* av_opt_get_video_rate;
			decltype(av_opt_get_channel_layout)* av_opt_get_channel_layout;
			decltype(av_opt_get_dict_val)* av_opt_get_dict_val;
			decltype(av_opt_ptr)* av_opt_ptr;
			decltype(av_opt_freep_ranges)* av_opt_freep_ranges;
			decltype(av_opt_query_ranges)* av_opt_query_ranges;
			decltype(av_opt_copy)* av_opt_copy;
			decltype(av_opt_query_ranges_default)* av_opt_query_ranges_default;
			decltype(av_opt_is_set_to_default)* av_opt_is_set_to_default;
			decltype(av_opt_is_set_to_default_by_name)* av_opt_is_set_to_default_by_name;
			decltype(av_opt_serialize)* av_opt_serialize;

			// pixdesc.h
			decltype(av_read_image_line)* av_read_image_line;
			decltype(av_write_image_line)* av_write_image_line;
			decltype(av_get_pix_fmt)* av_get_pix_fmt;
			decltype(av_get_pix_fmt_name)* av_get_pix_fmt_name;
			decltype(av_get_pix_fmt_string)* av_get_pix_fmt_string;
			decltype(av_get_bits_per_pixel)* av_get_bits_per_pixel;
			decltype(av_get_padded_bits_per_pixel)* av_get_padded_bits_per_pixel;
			decltype(av_pix_fmt_desc_get)* av_pix_fmt_desc_get;
			decltype(av_pix_fmt_desc_next)* av_pix_fmt_desc_next;
			decltype(av_pix_fmt_desc_get_id)* av_pix_fmt_desc_get_id;
			decltype(av_pix_fmt_get_chroma_sub_sample)* av_pix_fmt_get_chroma_sub_sample;
			decltype(av_pix_fmt_count_planes)* av_pix_fmt_count_planes;
			decltype(av_pix_fmt_swap_endianness)* av_pix_fmt_swap_endianness;
			decltype(av_get_pix_fmt_loss)* av_get_pix_fmt_loss;
			decltype(av_find_best_pix_fmt_of_2)* av_find_best_pix_fmt_of_2;
			decltype(av_color_range_name)* av_color_range_name;
			decltype(av_color_primaries_name)* av_color_primaries_name;
			decltype(av_color_transfer_name)* av_color_transfer_name;
			decltype(av_color_space_name)* av_color_space_name;
			decltype(av_chroma_location_name)* av_chroma_location_name;

			// time.h
			decltype(av_gettime)* av_gettime;
			decltype(av_gettime_relative)* av_gettime_relative;
			decltype(av_gettime_relative_is_monotonic)* av_gettime_relative_is_monotonic;
			decltype(av_usleep)* av_usleep;

			// mathematics.h
			decltype(av_gcd)* av_gcd;
			decltype(av_rescale)* av_rescale;
			decltype(av_rescale_rnd)* av_rescale_rnd;
			decltype(av_rescale_q)* av_rescale_q;
			decltype(av_rescale_q_rnd)* av_rescale_q_rnd;
			decltype(av_compare_ts)* av_compare_ts;
			decltype(av_compare_mod)* av_compare_mod;
			decltype(av_rescale_delta)* av_rescale_delta;
			decltype(av_add_stable)* av_add_stable;
	};

	class format : public FFmpegDllImport
	{
	private:
		void init()
		{
			// AVFORMAT.H
			FFMPEG_IMPORT(av_register_all);
			FFMPEG_IMPORT(avformat_alloc_context);
			FFMPEG_IMPORT(avformat_alloc_output_context2);
			FFMPEG_IMPORT(avformat_free_context);
			FFMPEG_IMPORT(av_guess_format);
			FFMPEG_IMPORT(av_guess_codec);
			FFMPEG_IMPORT(av_get_output_timestamp);
			FFMPEG_IMPORT(avformat_write_header);
			FFMPEG_IMPORT(av_write_trailer);
			FFMPEG_IMPORT(av_write_frame);
			FFMPEG_IMPORT(av_write_frame);
			FFMPEG_IMPORT(av_dump_format);
			FFMPEG_IMPORT(av_find_input_format);
			FFMPEG_IMPORT(avformat_open_input);
			FFMPEG_IMPORT(avformat_close_input);
			FFMPEG_IMPORT(avformat_find_stream_info);		
			FFMPEG_IMPORT(av_read_frame);
			FFMPEG_IMPORT(avformat_network_init);
			FFMPEG_IMPORT(av_interleaved_write_frame);

			// AVIO.H
			FFMPEG_IMPORT(avio_open);
			FFMPEG_IMPORT(avio_open2);
			FFMPEG_IMPORT(avio_close);
			FFMPEG_IMPORT(avformat_new_stream);

			this->av_register_all();
			this->avformat_network_init();
		}

	public:
		format(const std::string dllName) : FFmpegDllImport(dllName) { init(); }
		format(HMODULE hmodulle) : FFmpegDllImport(hmodule) { init(); }

		// AVFORMAT.H
		decltype(av_register_all)* av_register_all;
		decltype(avformat_alloc_context)* avformat_alloc_context;
		decltype(avformat_alloc_output_context2)* avformat_alloc_output_context2;
		decltype(avformat_free_context)* avformat_free_context;
		decltype(av_guess_format)* av_guess_format;
		decltype(av_guess_codec)* av_guess_codec;
		decltype(av_get_output_timestamp)* av_get_output_timestamp;
		decltype(avformat_write_header)* avformat_write_header;
		decltype(av_write_trailer)* av_write_trailer;
		decltype(av_write_frame)* av_write_frame;
		decltype(av_dump_format)* av_dump_format;
		decltype(av_find_input_format)* av_find_input_format;
		decltype(avformat_open_input)* avformat_open_input;
		decltype(avformat_close_input)* avformat_close_input;
		decltype(avformat_find_stream_info)* avformat_find_stream_info;
		decltype(av_read_frame)* av_read_frame;
		decltype(avformat_network_init)* avformat_network_init;
		decltype(av_interleaved_write_frame)* av_interleaved_write_frame;

		// AVIO.H
		decltype(avio_open)* avio_open;
		decltype(avio_open2)* avio_open2;
		decltype(avio_close)* avio_close;
		decltype(avformat_new_stream)* avformat_new_stream;
	};
	
	class scale : public FFmpegDllImport
	{
	private:
		void init()
		{
			FFMPEG_IMPORT(swscale_version);
			FFMPEG_IMPORT(swscale_configuration);
			FFMPEG_IMPORT(swscale_license);
			FFMPEG_IMPORT(sws_getCoefficients);
			FFMPEG_IMPORT(sws_isSupportedInput);
			FFMPEG_IMPORT(sws_isSupportedOutput);
			FFMPEG_IMPORT(sws_isSupportedEndiannessConversion);
			FFMPEG_IMPORT(sws_alloc_context);
			FFMPEG_IMPORT(sws_init_context);
			FFMPEG_IMPORT(sws_freeContext);
			FFMPEG_IMPORT(sws_getContext);
			FFMPEG_IMPORT(sws_scale);
			FFMPEG_IMPORT(sws_setColorspaceDetails);
			FFMPEG_IMPORT(sws_getColorspaceDetails);
			FFMPEG_IMPORT(sws_allocVec);
			FFMPEG_IMPORT(sws_getGaussianVec);
			FFMPEG_IMPORT(sws_getConstVec);
			FFMPEG_IMPORT(sws_getIdentityVec);
			FFMPEG_IMPORT(sws_scaleVec);
			FFMPEG_IMPORT(sws_normalizeVec);
			FFMPEG_IMPORT(sws_convVec);
			FFMPEG_IMPORT(sws_addVec);
			FFMPEG_IMPORT(sws_subVec);
			FFMPEG_IMPORT(sws_shiftVec);
			FFMPEG_IMPORT(sws_cloneVec);
			FFMPEG_IMPORT(sws_printVec2);
			FFMPEG_IMPORT(sws_freeVec);
			FFMPEG_IMPORT(sws_getDefaultFilter);
			FFMPEG_IMPORT(sws_freeFilter);
			FFMPEG_IMPORT(sws_getCachedContext);
			FFMPEG_IMPORT(sws_convertPalette8ToPacked32);
			FFMPEG_IMPORT(sws_convertPalette8ToPacked24);
			FFMPEG_IMPORT(sws_get_class);
		}

	public:
		scale(const std::string dllName) : FFmpegDllImport(dllName) { init(); }
		scale(HMODULE hmodulle) : FFmpegDllImport(hmodule) { init(); }

		decltype(swscale_version)* swscale_version;
		decltype(swscale_configuration)* swscale_configuration;
		decltype(swscale_license)* swscale_license;
		decltype(sws_getCoefficients)* sws_getCoefficients;
		decltype(sws_isSupportedInput)* sws_isSupportedInput;
		decltype(sws_isSupportedOutput)* sws_isSupportedOutput;
		decltype(sws_isSupportedEndiannessConversion)* sws_isSupportedEndiannessConversion;
		decltype(sws_alloc_context)* sws_alloc_context;
		decltype(sws_init_context)* sws_init_context;
		decltype(sws_freeContext)* sws_freeContext;
		decltype(sws_getContext)* sws_getContext;
		decltype(sws_scale)* sws_scale;
		decltype(sws_setColorspaceDetails)* sws_setColorspaceDetails;
		decltype(sws_getColorspaceDetails)* sws_getColorspaceDetails;
		decltype(sws_allocVec)* sws_allocVec;
		decltype(sws_getGaussianVec)* sws_getGaussianVec;
		decltype(sws_getConstVec)* sws_getConstVec;
		decltype(sws_getIdentityVec)* sws_getIdentityVec;
		decltype(sws_scaleVec)* sws_scaleVec;
		decltype(sws_normalizeVec)* sws_normalizeVec;
		decltype(sws_convVec)* sws_convVec;
		decltype(sws_addVec)* sws_addVec;
		decltype(sws_subVec)* sws_subVec;
		decltype(sws_shiftVec)* sws_shiftVec;
		decltype(sws_cloneVec)* sws_cloneVec;
		decltype(sws_printVec2)* sws_printVec2;
		decltype(sws_freeVec)* sws_freeVec;
		decltype(sws_getDefaultFilter)* sws_getDefaultFilter;
		decltype(sws_freeFilter)* sws_freeFilter;
		decltype(sws_getCachedContext)* sws_getCachedContext;
		decltype(sws_convertPalette8ToPacked32)* sws_convertPalette8ToPacked32;
		decltype(sws_convertPalette8ToPacked24)* sws_convertPalette8ToPacked24;
		decltype(sws_get_class)* sws_get_class;
	};
}

class FFmpegFactory
{
public:
	static FFmpegWrapper::avdevice device;
	static FFmpegWrapper::avcodec codec;
	static FFmpegWrapper::utils utils;
	static FFmpegWrapper::format format;
	static FFmpegWrapper::scale scale;
};