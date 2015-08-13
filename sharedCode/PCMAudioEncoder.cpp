#include "PCMAudioEncoder.h"
#include <stdexcept>

PCMAudioEncoder::PCMAudioEncoder(int channels, int sampeRate, FrameCallback c) : callback(c), closed(false)
{
	sampleCount = 0;

	// Initialize audio
	pCodec = avcodec_find_encoder_by_name("pcm_f32le");
	pCodecCtx = avcodec_alloc_context3(pCodec);
	pCodecCtx->codec_id = pCodec->id;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLT;
	pCodecCtx->channel_layout = AV_CH_LAYOUT_MONO;
	pCodecCtx->sample_rate = sampeRate;
	pCodecCtx->channels = channels;
	auto error = avcodec_open2(pCodecCtx, pCodec, nullptr);
	if (error < 0) throw std::runtime_error("Can't open audio codec.");
	pFrame = av_frame_alloc();
	pFrame->format = pCodecCtx->sample_fmt;
}

PCMAudioEncoder::~PCMAudioEncoder()
{
	this->Close();
}

void PCMAudioEncoder::WriteFrame(const uint8_t* audioSource, int cbAudioSource)
{
	pFrame->nb_samples = cbAudioSource / sizeof(float);
	pFrame->data[0] = const_cast<uint8_t*>(audioSource);
	pFrame->pts = sampleCount * 100;
	sampleCount += pFrame->nb_samples;

	int gotPacket = 0;
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	auto error = avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &gotPacket);
	if (gotPacket)
	{
		if (this->callback)
		{
			this->callback(pkt);
		}
		av_free_packet(&pkt);
	}
}

void PCMAudioEncoder::Close()
{
	if (!this->closed)
	{
		this->closed = true;

		avcodec_close(pCodecCtx);
		av_free(pCodecCtx);
		pCodecCtx = NULL;
		this->pCodec = NULL;

		if (pFrame)
		{
			av_frame_free(&pFrame);
			pFrame = NULL;
		}
	}
}