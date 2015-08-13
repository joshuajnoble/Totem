#include "..\..\totem.app\src\Utils.h"
#include "YUV420_H264_Encoder.h"
#include "LegacyGuards.h"
#include <windows.h>

//#define QSV_CHEAT

#ifdef QSV_CHEAT
#include "mfx/mfxvideo.h"
#include "libavcodec\qsv.h"
#include "libavcodec\qsvenc.h"

typedef struct myQSVH264EncContext {
	AVClass *avclass;
	QSVEncContext qsv;
} QSVH264EncContext;
#endif

int YUV420_H264_Encoder::getRawFrameSize()
{
	return yBlocksize + uBlockSize + vBlockSize;
}

YUV420_H264_Encoder::YUV420_H264_Encoder(int width, int height, int fps, FrameCallback callback) :
	callback(callback)
{
	auto codecName = "h264_qsv";
	//auto codecName = "libx264";
	pCodec = avcodec_find_encoder_by_name(codecName);
	if (!pCodec) {
		throw std::runtime_error((std::string("Codec not found: ") + codecName).c_str());
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		throw std::runtime_error("Could not allocate video codec context.");
	}

#ifdef QSV_CHEAT
	auto structSize = sizeof(myQSVH264EncContext);
	auto qsvContext = (myQSVH264EncContext *)pCodecCtx->priv_data;
#endif

	GuardAVCondexContext codecGuard(pCodecCtx);
	
	pCodecCtx->bit_rate = 400000;
	pCodecCtx->width = width;
	pCodecCtx->height = height;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = fps;
	pCodecCtx->gop_size = 5;
	pCodecCtx->pix_fmt = AV_PIX_FMT_NV12;// AV_PIX_FMT_YUV420P;

	// Low-latency mode settings for h264_qsv
	pCodecCtx->max_b_frames = 0;
	pCodecCtx->refs = 1;
	pCodecCtx->level = 10;
	av_opt_set(pCodecCtx->priv_data, "async_depth", "1", 0);
	av_opt_set(pCodecCtx->priv_data, "preset", "4", 0);
	av_opt_set(pCodecCtx->priv_data, "idr_interval", "1", 0);
	av_opt_set(pCodecCtx->priv_data, "profile", "66", 0);

	auto error = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (error < 0)
	{
		printf("Could not open codec.");
	}

	pFrame = av_frame_alloc();
	if (!pFrame) {
		throw std::runtime_error("Could not allocate video frame.");
	}

	pFrame->format = pCodecCtx->pix_fmt;
	pFrame->width = pCodecCtx->width;
	pFrame->height = pCodecCtx->height;

	auto ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
	if (ret < 0) {
		throw std::runtime_error("Could not allocate raw picture buffer.");
	}

	yBlocksize = this->pFrame->height * this->pFrame->linesize[0];
	uBlockSize = this->pFrame->height / 2 * this->pFrame->linesize[1];
	vBlockSize = this->pFrame->height / 2 * this->pFrame->linesize[2];

	codecGuard.Release();
}

YUV420_H264_Encoder::~YUV420_H264_Encoder()
{
	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVFrame *pFrame = NULL;

	if (this->pCodecCtx)
	{
		avcodec_close(this->pCodecCtx);
		this->pCodecCtx = NULL;
	}

	if (this->pCodecCtx)
	{
		av_free(pCodecCtx);
		this->pCodecCtx = NULL;
	}

	if (this->pCodec)
	{
		this->pCodec = NULL;
	}

	if (this->pFrame)
	{
		if (pFrame->data)
		{
			av_freep(&this->pFrame->data[0]);
		}

		av_frame_free(&this->pFrame);
		this->pFrame = NULL;
	}
}

void YUV420_H264_Encoder::WriteFrame(const uint8_t* framebuffer)
{
	if (framebuffer == NULL)
	{
		throw std::invalid_argument("framebuffer");
	}

	av_init_packet(&pkt);
	pkt.data = NULL; // packet data will be allocated by the encoder
	pkt.size = 0;

	pFrame->data[0] = const_cast<uint8_t*>(framebuffer);
	pFrame->data[1] = const_cast<uint8_t*>(framebuffer + yBlocksize);
	//pFrame->data[2] = const_cast<uint8_t*>(framebuffer + yBlocksize + uBlockSize); // Only for YUV420P; not AV_PIX_FMT_NV12

	//pFrame->pts = this->cInputFrames++; //milliseconds_now();// timeGetTime();
	int got_output;

	/* encode the image */
	auto ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
	if (ret < 0)
	{
		throw std::runtime_error("Error encoding frame.");
	}

	if (got_output)
	{
		if (this->callback)
		{
			this->callback(pkt);
		}
		av_free_packet(&pkt);
	}
}

void YUV420_H264_Encoder::Close()
{
	//Flush Encoder even at least one frame was given to us
	int got_output = this->cInputFrames ? 1 : 0;
	while (got_output)
	{
		auto ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
		if (ret < 0) {
			printf("Error encoding frame\n");
			return;
		}
		if (got_output) {
			if (this->callback)
			{
				this->callback(pkt);
			}
			av_free_packet(&pkt);
		}
	}

	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	pCodecCtx = NULL;
	this->pCodec = NULL;

	if (pFrame)
	{
		if (pFrame->data)
		{
			//av_freep(&pFrame->data[0]);
		}

		av_frame_free(&pFrame);
		pFrame = NULL;
	}
}