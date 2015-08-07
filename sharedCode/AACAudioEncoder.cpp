#include "..\..\totem.app\src\Utils.h"
#include "AACAudioEncoder.h"
#include <limits.h>

AACAudioEncoder::AACAudioEncoder(int channels, int sampeRate, FrameCallback c) : callback(c)
{
	sampleCount = 0;

	// Initialize audio
	pCodec = m_ffmpeg.codec.avcodec_find_encoder_by_name("libfdk_aac");
	pCodecCtx = m_ffmpeg.codec.avcodec_alloc_context3(pCodec);
	pCodecCtx->codec_id = pCodec->id;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodecCtx->channel_layout = AV_CH_LAYOUT_MONO;
	pCodecCtx->sample_rate = sampeRate;
	pCodecCtx->channels = channels;
	pCodecCtx->bit_rate = 128000;
	auto error = m_ffmpeg.codec.avcodec_open2(pCodecCtx, pCodec, nullptr);
	if (error < 0) throw std::runtime_error("Can't open audio codec.");
	pFrame = m_ffmpeg.utils.av_frame_alloc();
	pFrame->format = pCodecCtx->sample_fmt;

	frameSizeSamples = pCodecCtx->frame_size;
	frameSizeBytes = frameSizeSamples * sizeof(float) * channels;
	alignedBuffer.resize(frameSizeBytes);
}

AACAudioEncoder::~AACAudioEncoder()
{
	this->Close();
}

int AACAudioEncoder::WriteFrame(const uint8_t* audioSource, int cbAudioSource)
{
	int error;
	int cbEncoded = 0;

	while (cbAudioSource - cbEncoded >= frameSizeBytes)
	{
		//pFrame->data[0] = const_cast<uint8_t*>(audioSource)+cbEncoded;
		memcpy(alignedBuffer.data(), audioSource + cbEncoded, frameSizeBytes);
		pFrame->data[0] = const_cast<uint8_t*>(alignedBuffer.data());
		pFrame->linesize[0] = frameSizeBytes;
		pFrame->nb_samples = frameSizeSamples;
		//pFrame->pts = sampleCount * 100;
		//pFrame->pts = milliseconds_now();
		sampleCount += pFrame->nb_samples;

		int gotPacket = 0;
		AVPacket pkt;
		m_ffmpeg.codec.av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;
		pkt.pos = 0;
		error = m_ffmpeg.codec.avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &gotPacket);
		cbEncoded += frameSizeBytes;

		if (gotPacket)
		{
			if (this->callback)
			{
				this->callback(pkt);
			}
			m_ffmpeg.codec.av_free_packet(&pkt);
		}
	}

	return cbEncoded;
}

void AACAudioEncoder::Close()
{
	if (!this->closed)
	{
		this->closed = true;

		m_ffmpeg.codec.avcodec_close(pCodecCtx);
		m_ffmpeg.utils.av_free(pCodecCtx);
		pCodecCtx = NULL;
		this->pCodec = NULL;

		if (pFrame)
		{
			m_ffmpeg.utils.av_frame_free(&pFrame);
			pFrame = NULL;
		}
	}
}