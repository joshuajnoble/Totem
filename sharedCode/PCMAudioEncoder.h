#pragma once

#include "FFmpegImports.h"
#include <functional>

class PCMAudioEncoder
{
private:
	FFmpegFactory m_ffmpeg;
	FrameCallback callback;

	AVPacket pkt;
	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVFrame *pFrame = NULL;
	int sampleCount;

public:
	PCMAudioEncoder(int channels, int sampeRate, FrameCallback c) : callback(c)
	{
		sampleCount = 0;

		// Initialize audio
		pCodec = m_ffmpeg.codec.avcodec_find_encoder_by_name("pcm_f32le");
		pCodecCtx = m_ffmpeg.codec.avcodec_alloc_context3(pCodec);
		pCodecCtx->codec_id = pCodec->id;
		pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
		pCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLT;
		pCodecCtx->channel_layout = AV_CH_LAYOUT_MONO;
		pCodecCtx->sample_rate = sampeRate;
		pCodecCtx->channels = channels;
		auto error = m_ffmpeg.codec.avcodec_open2(pCodecCtx, pCodec, nullptr);
		if (error < 0) throw std::runtime_error("Can't open audio codec.");
		pFrame = m_ffmpeg.utils.av_frame_alloc();
		pFrame->format = pCodecCtx->sample_fmt;
	}

	~PCMAudioEncoder()
	{
		this->Close();
	}

	void WriteFrame(const uint8_t* audioSource, int cbAudioSource)
	{
		pFrame->nb_samples = cbAudioSource / sizeof(float);
		pFrame->data[0] = const_cast<uint8_t*>(audioSource);
		pFrame->pts = sampleCount * 100;
		sampleCount += pFrame->nb_samples;

		int gotPacket = 0;
		AVPacket pkt;
		m_ffmpeg.codec.av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;
		auto error = m_ffmpeg.codec.avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &gotPacket);
		if (gotPacket)
		{
			if (this->callback)
			{
				this->callback(pkt);
			}
			m_ffmpeg.codec.av_free_packet(&pkt);
		}
	}

	void Close()
	{

	}
};