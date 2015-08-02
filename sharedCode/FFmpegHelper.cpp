#include "FFmpegHelper.h"

#define WRITE_VIDEO_FILE

class encoderHelper
{
public:
	AVCodec* m_encoder;
	AVCodecContext *m_context;
	FFmpegHelper* const m_ffmpeg;
	AVFrame* m_frame;
	AVPacket m_pkt;
	int64_t m_frameCountIn;
	int64_t m_frameCountOut;
#ifdef WRITE_VIDEO_FILE
	FILE *f;
#endif

	encoderHelper(const char *codecName, FFmpegHelper* ffmpeg) : m_ffmpeg(ffmpeg)
	{
		m_frameCountIn = m_frameCountOut = 0;

		m_encoder = m_ffmpeg->codecDll.avcodec_find_encoder_by_name(codecName);
		assert(m_encoder);

		m_context = m_ffmpeg->codecDll.avcodec_alloc_context3(m_encoder);
		assert(m_context);

		m_context->bit_rate = 800000;
		m_context->width = 640;
		m_context->height = 480;
		m_context->time_base.num = 1;
		m_context->time_base.den = 15;
		m_context->gop_size = 10; /* emit one intra frame every ten frames */
		m_context->max_b_frames = 1;
		m_context->pix_fmt = AV_PIX_FMT_YUV420P;

		int errCode = 0;
		if (m_encoder->name == "h264_qsv") {
			errCode = m_ffmpeg->opt.av_opt_set(m_context->priv_data, "preset", "fast", 0);
		}
		else if (m_encoder->name == "libx264") {
			errCode = m_ffmpeg->opt.av_opt_set(m_context->priv_data, "preset", "slow", 0);
		}
		assert(errCode >= 0);

		errCode = m_ffmpeg->codecDll.avcodec_open2(m_context, m_encoder, NULL);
		assert(errCode >= 0);

		auto drameSize = m_context->frame_size;
		m_frame = m_ffmpeg->codecDll.avcodec_alloc_frame();
		assert(m_frame);
		m_frame->format = m_context->pix_fmt;
		m_frame->width = m_context->width;
		m_frame->height = m_context->height;

		errCode = m_ffmpeg->imgUtilsDll.av_image_alloc(m_frame->data, m_frame->linesize, m_context->width, m_context->height, m_context->pix_fmt, 32);
		assert(errCode >= 0);

#ifdef WRITE_VIDEO_FILE
		f = fopen("output.mp4", "wb");
		assert(f);
#endif
	}

	void encodeFrame(uint8_t* imageBytes)
	{
		int got_output;

		m_ffmpeg->codecDll.av_init_packet(&m_pkt);
		m_pkt.data = NULL; // Let the encoder allocare the packet data
		m_pkt.size = 0;

		int maxIndex1 = 0;
		int maxIndex2 = 0;

		if (imageBytes) {
			auto Cb = m_frame->height * m_frame->linesize[0];
			auto Cr = Cb + m_frame->height / 2 * m_frame->linesize[1];
			memcpy(m_frame->data[0], imageBytes, m_frame->height * m_frame->linesize[0]);
			memcpy(m_frame->data[1], imageBytes + Cb, m_frame->height / 2 * m_frame->linesize[1]);
			memcpy(m_frame->data[2], imageBytes + Cr, m_frame->height / 2 * m_frame->linesize[2]);
		}
		else
		{
			auto i = m_frameCountIn;
			/* prepare a dummy image */
			/* Y */
			for (auto y = 0; y < m_context->height; y++)
			{
				for (auto x = 0; x < m_context->width; x++)
				{
					auto index = y * m_frame->linesize[0] + x;
					maxIndex1 = max(maxIndex1, index);
					m_frame->data[0][index] = x + y + i * 3;
				}
			}
			/* Cb and Cr */
			for (auto y = 0; y < m_context->height / 2; y++)
			{
				for (auto x = 0; x < m_context->width / 2; x++)
				{
					auto index = y * m_frame->linesize[2] + x;
					maxIndex2 = max(maxIndex2, index);
					m_frame->data[1][y * m_frame->linesize[1] + x] = 128 + y + i * 2;
					m_frame->data[2][y * m_frame->linesize[2] + x] = 64 + x + i * 5;
				}
			}
		}

		m_frame->pts = m_frameCountIn++;

		/* encode the image */
		auto errCode = m_ffmpeg->codecDll.avcodec_encode_video2(m_context, &m_pkt, m_frame, &got_output);
		if (errCode < 0)
		{
			fprintf(stderr, "Error encoding frame\n");
			exit(1);
		}

		if (got_output)
		{
			printf("Write frame %3d (size=%5d)\n", m_frameCountOut++, m_pkt.size);
#ifdef WRITE_VIDEO_FILE
			fwrite(m_pkt.data, 1, m_pkt.size, f);
#endif
			m_ffmpeg->codecDll.av_free_packet(&m_pkt);
		}
	}

	void stop()
	{
		/* get the delayed frames */
		//int64_t i = m_frame->pts;
		int got_output;
		do
		{
			//fflush(stdout);
			auto errCode = m_ffmpeg->codecDll.avcodec_encode_video2(m_context, &m_pkt, NULL, &got_output);
			if (errCode < 0)
			{
				fprintf(stderr, "Error encoding frame\n");
				exit(1);
			}

			if (got_output)
			{
				printf("Write frame %3d (size=%5d)\n", m_frameCountOut++, m_pkt.size);
#ifdef WRITE_VIDEO_FILE
				fwrite(m_pkt.data, 1, m_pkt.size, f);
#endif
				m_ffmpeg->codecDll.av_free_packet(&m_pkt);
			}
		} while (got_output);

#ifdef WRITE_VIDEO_FILE
		/* add sequence end code to have a real mpeg file */
		uint8_t endcode[] = { 0, 0, 1, 0xb7 };
		fwrite(endcode, 1, sizeof(endcode), f);
		fclose(f);
#endif

		m_ffmpeg->codecDll.avcodec_free_frame(&m_frame);
		m_ffmpeg->codecDll.avcodec_free_context(&m_context);
	}
};

FFmpegHelper::FFmpegHelper() :
	deviceDll("avdevice-56.dll"),
	codecDll("avcodec-56.dll"),
	imgUtilsDll("avutil-54.dll"),
	opt("avutil-54.dll")
{
	this->deviceDll.avdevice_register_all();
	this->codecDll.avcodec_register_all();

	//const char *codecName = false ? "h264_qsv" : "libx264";
	//auto encoder = encoderHelper(codecName, this);

	////
	//byte* frameBuffer = reinterpret_cast<byte *>(malloc(640 * 480 + (320 * 240) * 2));
	//for (auto i = 0; i < 15 * 10; ++i) {
	//	memset(frameBuffer, 128, 640 * 480); // Y
	//	memset(frameBuffer + 640 * 480, 48, 320 * 240); // Cb
	//	memset(frameBuffer + 640 * 480 + 320 * 240, 96, 320 * 240); // Cr
	//	encoder.encodeFrame(frameBuffer);
	//}
	//free(frameBuffer);
	//encoder.stop();
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
		fmt = deviceDll.av_input_video_device_next(fmt);
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
	auto count = deviceDll.avdevice_list_input_sources(NULL, sourceName.c_str(), NULL, &device_list);

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