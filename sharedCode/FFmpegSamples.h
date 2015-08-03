#pragma once

#include "FFmpegImports.h"
#include "LegacyGuards.h"
#include "stdio.h"
#include <fstream>
#include <vector>
#include "YUV420_H264_Encoder.h"

// http ://sourceforge.net/p/simplestffmpegvideoencoder/code/ci/master/tree/simplest_ffmpeg_video_encoder/simplest_ffmpeg_video_encoder.cpp
class encodeToVideoFile
{
private:
	FFmpegFactory *m_ffmpeg;

	int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index){
		int ret;
		int got_frame;
		AVPacket enc_pkt;
		if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
			CODEC_CAP_DELAY))
			return 0;
		while (1) {
			enc_pkt.data = NULL;
			enc_pkt.size = 0;
			m_ffmpeg->codec.av_init_packet(&enc_pkt);
			ret = m_ffmpeg->codec.avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
			m_ffmpeg->utils.av_frame_free(NULL);
			if (ret < 0)
				break;
			if (!got_frame){
				ret = 0;
				break;
			}
			printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
			/* mux encoded frame */
			ret = m_ffmpeg->format.av_write_frame(fmt_ctx, &enc_pkt);
			if (ret < 0)
				break;
		}
		return ret;
	}

public:
	encodeToVideoFile(FFmpegFactory *ffmpeg, char *inputFileName, char* outputFilename, int width, int height, int fps) : m_ffmpeg(ffmpeg)
	{
		AVFormatContext* pFormatCtx;
		AVStream* video_st;
		AVCodecContext* pCodecCtx;
		AVCodec* pCodec;
		AVPacket pkt;
		uint8_t* picture_buf;
		AVFrame* pFrame;
		int picture_size;
		int y_size;
		int framecnt = 0;
		FILE *in_file;

		auto error = fopen_s(&in_file, inputFileName, "rb");   //Input raw YUV data
		assert(!error);
		GuardFileHandle safeFile(in_file);

		int in_w = width, in_h = height;
		//const char* out_file = "src01.h264";              //Output Filepath 
		//const char* out_file = "src01.ts";
		//const char* out_file = "src01.hevc";
		const char* out_file = outputFilename;

		//Method1.
		pFormatCtx = m_ffmpeg->format.avformat_alloc_context();
		//Guess Format
		{
			auto fmt = m_ffmpeg->format.av_guess_format(NULL, out_file, NULL);
			pFormatCtx->oformat = fmt;

			//Method 2.
			//m_ffmpeg->format.avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
			//fmt = pFormatCtx->oformat;
		}

		//Open output URL
		if (m_ffmpeg->format.avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0){
			printf("Failed to open output file! \n");
			return;
		}

		pCodec = m_ffmpeg->codec.avcodec_find_encoder_by_name("libx264");
		//pCodec = m_ffmpeg->codec.avcodec_find_encoder_by_name("h264_qsv"); // TODO: Why doesn't this work?
		if (!pCodec){
			printf("Can not find encoder! \n");
			return;
		}

		video_st = m_ffmpeg->format.avformat_new_stream(pFormatCtx, pCodec);
		video_st->time_base.num = 1;
		video_st->time_base.den = fps;

		if (video_st == NULL){
			return;
		}
		//Param that must set
		pCodecCtx = video_st->codec;
		//pCodecCtx->codec_id =AV_CODEC_ID_HEVC;
		pCodecCtx->codec_id = video_st->codec->codec_id;
		pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
		pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
		pCodecCtx->width = in_w;
		pCodecCtx->height = in_h;
		pCodecCtx->time_base.num = video_st->time_base.num;
		pCodecCtx->time_base.den = video_st->time_base.den;
		pCodecCtx->bit_rate = 400000;
		pCodecCtx->gop_size = video_st->time_base.den * 10;
		//H264
		//pCodecCtx->me_range = 16;
		//pCodecCtx->max_qdiff = 4;
		//pCodecCtx->qcompress = 0.6;
		pCodecCtx->qmin = 10;
		pCodecCtx->qmax = 51;

		//Optional Param
		pCodecCtx->max_b_frames = 3;

		// Set Option
		AVDictionary *param = 0;
		//H.264
		if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
			m_ffmpeg->utils.av_dict_set(&param, "preset", "slow", 0);
			m_ffmpeg->utils.av_dict_set(&param, "tune", "zerolatency", 0);
			//av_dict_set(&param, "profile", "main", 0);
		}
		//H.265
		if (pCodecCtx->codec_id == AV_CODEC_ID_H265){
			m_ffmpeg->utils.av_dict_set(&param, "preset", "ultrafast", 0);
			m_ffmpeg->utils.av_dict_set(&param, "tune", "zero-latency", 0);
		}

		//Show some Information
		m_ffmpeg->format.av_dump_format(pFormatCtx, 0, out_file, 1);

		if (m_ffmpeg->codec.avcodec_open2(pCodecCtx, pCodec, &param) < 0){
			printf("Failed to open encoder! \n");
			return;
		}

		pFrame = m_ffmpeg->utils.av_frame_alloc();
		picture_size = m_ffmpeg->codec.avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
		picture_buf = (uint8_t *)m_ffmpeg->utils.av_malloc(picture_size);
		m_ffmpeg->codec.avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

		//Write File Header
		m_ffmpeg->format.avformat_write_header(pFormatCtx, NULL);

		m_ffmpeg->codec.av_new_packet(&pkt, picture_size);

		y_size = pCodecCtx->width * pCodecCtx->height;

		for (int i = 0; !feof(in_file); i++){
			//Read raw YUV data
			auto cbRead = fread(picture_buf, 1, y_size * 3 / 2, in_file);
			if (cbRead == y_size * 3 / 2)
			{
				pFrame->data[0] = picture_buf;              // Y
				pFrame->data[1] = picture_buf + y_size;      // U 
				pFrame->data[2] = picture_buf + y_size * 5 / 4;  // V
				//PTS
				pFrame->pts = i;
				int got_picture = 0;
				//Encode
				int ret = m_ffmpeg->codec.avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
				if (ret < 0){
					printf("Failed to encode! \n");
					return;
				}
				if (got_picture == 1){
					printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
					framecnt++;
					pkt.stream_index = video_st->index;
					ret = m_ffmpeg->format.av_write_frame(pFormatCtx, &pkt);
					m_ffmpeg->codec.av_free_packet(&pkt);
				}
			}
			else if (cbRead != 0 || (i == cbRead == 0))
			{	// Either we could not ready anything at all
				// or we read a bunch and then ran out of bytes when we did not expect to
				// in the second case, that usually means that the width or height doesn't match.
				printf("Failed reading input file\n");
				return;
			}
		}

		//Flush Encoder
		int ret = flush_encoder(pFormatCtx, 0);
		if (ret < 0) {
			printf("Flushing encoder failed\n");
			return;
		}

		//Write file trailer
		m_ffmpeg->format.av_write_trailer(pFormatCtx);

		//Clean
		if (video_st){
			m_ffmpeg->codec.avcodec_close(video_st->codec);
			m_ffmpeg->utils.av_free(pFrame);
			m_ffmpeg->utils.av_free(picture_buf);
		}
		m_ffmpeg->format.avio_close(pFormatCtx->pb);
		m_ffmpeg->format.avformat_free_context(pFormatCtx);

		fclose(in_file);
	}
};

// http://sourceforge.net/p/simplestffmpegvideoencoder/code/ci/master/tree/simplest_ffmpeg_video_encoder_pure/simplest_ffmpeg_video_encoder_pure.cpp
class encodeRawFile
{
private:
	FFmpegFactory *m_ffmpeg;

public:
	encodeRawFile(FFmpegFactory *ffmpeg, char *filename_in, char* filename_out, int in_w, int in_h, int fps) : m_ffmpeg(ffmpeg)
	{
		AVCodec *pCodec;
		AVCodecContext *pCodecCtx = NULL;
		int i, ret, got_output;
		FILE *fp_in;
		FILE *fp_out;
		AVFrame *pFrame;
		AVPacket pkt;
		int y_size;
		int framecnt = 0;

		//auto intelCodec = m_ffmpeg->codec.avcodec_find_encoder_by_name("h264_qsv"); // TODO: why doesn't this work?
		auto x264Codec = m_ffmpeg->codec.avcodec_find_encoder_by_name("libx264");

		pCodec = x264Codec;
		if (!pCodec) {
			printf("Codec not found\n");
			return;
		}

		AVCodecID codec_id = pCodec->id;

		pCodecCtx = m_ffmpeg->codec.avcodec_alloc_context3(pCodec);
		if (!pCodecCtx) {
			printf("Could not allocate video codec context\n");
			return;
		}

		pCodecCtx->bit_rate = 400000;
		pCodecCtx->width = in_w;
		pCodecCtx->height = in_h;
		pCodecCtx->time_base.num = 1;
		pCodecCtx->time_base.den = fps;
		pCodecCtx->gop_size = 10;
		pCodecCtx->max_b_frames = 1;
		pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

		if (codec_id == AV_CODEC_ID_H264)
		{
			//m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);
			//m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
			//m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
			m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "preset", "7", 0);
			m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "async_depth", "1", 0);
			m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "bf", "0", 0);
		}

		if (m_ffmpeg->codec.avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
			printf("Could not open codec\n");
			return;
		}

		pFrame = m_ffmpeg->utils.av_frame_alloc();
		if (!pFrame) {
			printf("Could not allocate video frame\n");
			return;
		}
		pFrame->format = pCodecCtx->pix_fmt;
		pFrame->width = pCodecCtx->width;
		pFrame->height = pCodecCtx->height;

		ret = m_ffmpeg->utils.av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
		if (ret < 0) {
			printf("Could not allocate raw picture buffer\n");
			return;
		}
		//Input raw data
		auto error = fopen_s(&fp_in, filename_in, "rb");
		if (error) {
			printf("Could not open %s\n", filename_in);
			return;
		}
		GuardFileHandle safeInput(fp_in);

		//Output bitstream
		error = fopen_s(&fp_out, filename_out, "wb");
		if (error) {
			printf("Could not open %s\n", filename_out);
			return;
		}
		GuardFileHandle safeOutput(fp_out);

		y_size = pCodecCtx->width * pCodecCtx->height;
		//Encode
		for (i = 0; !feof(fp_in); i++) {
			m_ffmpeg->codec.av_init_packet(&pkt);
			pkt.data = NULL;    // packet data will be allocated by the encoder
			pkt.size = 0;
			//Read raw YUV data
			auto cbRead = fread(pFrame->data[0], 1, y_size, fp_in);
			//assert(cbRead == y_size);
			cbRead = fread(pFrame->data[1], 1, y_size / 4, fp_in);
			//assert(cbRead == y_size / 4);
			cbRead = fread(pFrame->data[2], 1, y_size / 4, fp_in);
			//assert(cbRead == y_size / 4);

			if (cbRead != y_size / 4)
			{
				break;
			}

			pFrame->pts = i;
			/* encode the image */
			ret = m_ffmpeg->codec.avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
			if (ret < 0) {
				printf("Error encoding frame\n");
				return;
			}
			if (got_output) {
				printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
				framecnt++;
				fwrite(pkt.data, 1, pkt.size, fp_out);
				m_ffmpeg->codec.av_free_packet(&pkt);
			}
		}

		//Flush Encoder
		for (got_output = 1; got_output; i++) {
			ret = m_ffmpeg->codec.avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
			if (ret < 0) {
				printf("Error encoding frame\n");
				return;
			}
			if (got_output) {
				printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", pkt.size);
				fwrite(pkt.data, 1, pkt.size, fp_out);
				m_ffmpeg->codec.av_free_packet(&pkt);
			}
		}

		m_ffmpeg->codec.avcodec_close(pCodecCtx);
		m_ffmpeg->utils.av_free(pCodecCtx);
		m_ffmpeg->utils.av_freep(&pFrame->data[0]);
		m_ffmpeg->utils.av_frame_free(&pFrame);
	}
};

// https://github.com/leixiaohua1020/simplest_ffmpeg_device/blob/master/simplest_ffmpeg_readcamera/simplest_ffmpeg_readcamera.cpp
class VideoCaptureTest
{
private:
	FFmpegFactory* m_ffmpeg;
	AVFormatContext	*pFormatCtx;
	int				videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;

public:
	VideoCaptureTest(FFmpegFactory* ffmpeg, const char *filename, int width, int height, int fps, int duration) : m_ffmpeg(ffmpeg)
	{
		AVInputFormat *ifmt = m_ffmpeg->format.av_find_input_format("dshow");
		//Set own video device's name
		pFormatCtx = m_ffmpeg->format.avformat_alloc_context();

		std::string captureSourceName("Logitech HD Pro Webcam C920");
		//std::string captureSourceName("HP Truevision HD");

		AVDictionary* options = NULL;
		m_ffmpeg->utils.av_dict_set(&options, "video_size", (std::to_string(width) + "x" + std::to_string(height)).c_str(), 0);
		m_ffmpeg->utils.av_dict_set(&options, "framerate", std::to_string(fps).c_str(), 0);
		m_ffmpeg->utils.av_dict_set(&options, "pixel_format", m_ffmpeg->utils.av_get_pix_fmt_name(PIX_FMT_YUV420P), 0);
		if (m_ffmpeg->format.avformat_open_input(&pFormatCtx, (std::string("video=") + captureSourceName).c_str(), ifmt, &options) != 0)
		{
			if (!m_ffmpeg->utils.av_dict_set(&options, "pixel_format", m_ffmpeg->utils.av_get_pix_fmt_name(PIX_FMT_YUYV422), 0))
			{
				if (!m_ffmpeg->utils.av_dict_set(&options, "pixel_format", m_ffmpeg->utils.av_get_pix_fmt_name(PIX_FMT_RGB24), 0))
				{
					printf("Couldn't open input stream.\n");
					return;
				}
			}
		}

		if (m_ffmpeg->format.avformat_find_stream_info(pFormatCtx, NULL) < 0)
		{
			printf("Couldn't find stream information.\n");
			return;
		}

		auto videoindex = -1;
		for (unsigned i = 0; i < pFormatCtx->nb_streams; i++)
		{
			auto candidateCodec = pFormatCtx->streams[i]->codec;
			if (candidateCodec->codec_type == AVMEDIA_TYPE_VIDEO
				&& candidateCodec->width == width
				&& candidateCodec->height == height)
				if (candidateCodec->pix_fmt == PIX_FMT_YUV420P)
				{
					videoindex = i;
					break;
				}
		}

		if (videoindex == -1)
		{
			printf("Couldn't find a video stream.\n");
			return;
		}
		pCodecCtx = pFormatCtx->streams[videoindex]->codec;
		pCodec = m_ffmpeg->codec.avcodec_find_decoder(pCodecCtx->codec_id);
		if (pCodec == NULL)
		{
			printf("Codec not found.\n");
			return;
		}

		pCodecCtx->width = width;
		pCodecCtx->height = height;
		pCodecCtx->framerate.num = 1;
		pCodecCtx->framerate.den = fps;

		if (m_ffmpeg->codec.avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		{
			printf("Could not open codec.\n");
			return;
		}

		AVFrame* pFrameYUV = m_ffmpeg->utils.av_frame_alloc();
		//uint8_t *out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
		//avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

		AVPacket *packet = (AVPacket *)m_ffmpeg->utils.av_malloc(sizeof(AVPacket));
		FILE *fp_yuv;
		auto error = fopen_s(&fp_yuv, filename, "wb+");
		assert(!error);
		GuardFileHandle safeFile(fp_yuv);

		int ret, got_picture;
		//struct SwsContext *img_convert_ctx;
		//img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		//------------------------------

		LARGE_INTEGER time, nextTime, frequency, stopTime;
		QueryPerformanceFrequency(&frequency);
		auto frameInterval = frequency.QuadPart / fps;

		QueryPerformanceCounter(&time);
		nextTime = stopTime = time;
		stopTime.QuadPart += frequency.QuadPart * duration;
		for (; stopTime.QuadPart > time.QuadPart;)
		{
			QueryPerformanceCounter(&time);
			bool hasNewFrame = time.QuadPart >= nextTime.QuadPart;
			if (!hasNewFrame) {
				Sleep(5);
				continue;
			}

			do
			{
				nextTime.QuadPart += frameInterval;
			} while (nextTime.QuadPart < time.QuadPart);

			if (m_ffmpeg->format.av_read_frame(pFormatCtx, packet) >= 0)
			{
				if (packet->stream_index == videoindex)
				{
					ret = m_ffmpeg->codec.avcodec_decode_video2(pCodecCtx, pFrameYUV, &got_picture, packet);
					if (ret < 0)
					{
						printf("Decode Error.\n");
						return;
					}
					if (got_picture)
					{
						int y_size = pCodecCtx->width*pCodecCtx->height;
						fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y   
						fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U  
						fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V  
					}
				}
				m_ffmpeg->codec.av_free_packet(packet);
			}
		}

		//sws_freeContext(img_convert_ctx);

		safeFile.Cleanup();

		//av_free(out_buffer);
		m_ffmpeg->utils.av_free(pFrameYUV);
		m_ffmpeg->codec.avcodec_close(pCodecCtx);
		m_ffmpeg->format.avformat_close_input(&pFormatCtx);
	}
};

class JustForTesting
{
private:
	FFmpegFactory& m_ffmpeg;

public:
	JustForTesting(FFmpegFactory& ffmpeg) : m_ffmpeg(ffmpeg)
	{
		int width = 640;
		int height = 480;
		int fps = 15;
		VideoCaptureTest videoCapture(&m_ffmpeg, "test.yuv", width, height, fps, 10);
		YUV420_H264_Encoder streamingEncoder(m_ffmpeg, "test.h264", width, height, fps);

		auto frameSize = streamingEncoder.getRawFrameSize();
		std::vector<byte> buffer(frameSize);
		std::ifstream rawFileYuv("test.yuv", std::ios::binary);
		while (rawFileYuv && !rawFileYuv.eof())
		{
			rawFileYuv.read((char *)buffer.data(), frameSize);
			streamingEncoder.EncodeFrame(buffer.data());
		}

		streamingEncoder.Close();

		//auto fileTest = new encodeToVideoFile(m_ffmpeg, "test.yuv", "test.h264", width, height, fps);
		//delete fileTest;

		//auto streamTest = new encodeRawFile(m_ffmpeg, "test.yuv", "test-raw.h264", width, height, fps);
		//delete streamTest;

		//{
		//	AVFormatContext *pFormatCtx = m_ffmpeg->format.avformat_alloc_context();
		//	AVDictionary* options = NULL;
		//	m_ffmpeg->utils.av_dict_set(&options, "list_devices", "true", 0);
		//	AVInputFormat *iformat = m_ffmpeg->format.av_find_input_format("dshow");
		//	printf("========Device Info=============\n");
		//	m_ffmpeg->format.avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);
		//	m_ffmpeg->format.avformat_close_input(&pFormatCtx);
		//	printf("================================\n");
		//}

		//{
		//	AVFormatContext *pFormatCtx = m_ffmpeg->format.avformat_alloc_context();
		//	AVDictionary* options = NULL;
		//	m_ffmpeg->utils.av_dict_set(&options, "list_options", "true", 0);
		//	AVInputFormat *iformat = m_ffmpeg->format.av_find_input_format("dshow");
		//	printf("========Device Option Info======\n");
		//	std::string deviceName("Logitech HD Pro Webcam C920");
		//	//std::string deviceName("HP Truevision HD");
		//	m_ffmpeg->format.avformat_open_input(&pFormatCtx, (std::string("video=") + deviceName).c_str(), iformat, &options);
		//	m_ffmpeg->format.avformat_close_input(&pFormatCtx);
		//	printf("================================\n");
		//}
	}
};