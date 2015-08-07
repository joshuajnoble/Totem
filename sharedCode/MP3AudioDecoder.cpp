#include "MP3AudioDecoder.h"

namespace
{
	const int AVCODEC_MAX_AUDIO_FRAME_SIZE = 192000;
	const int AUDIO_INBUF_SIZE = 1024 * 64;
}

MP3AudioDecoder::MP3AudioDecoder(int channels, int sampeRate, DecodedFrameCallback c) : callback(c), closed(false)
{
	// Initialize audio
	pCodec = m_ffmpeg.codec.avcodec_find_decoder_by_name("libmp3lame");
	if (!pCodec)
	{
		pCodec = m_ffmpeg.codec.avcodec_find_decoder(CODEC_ID_MP3);
	}

	pCodecCtx = m_ffmpeg.codec.avcodec_alloc_context3(pCodec);
	pCodecCtx->codec_id = pCodec->id;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = pCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_FLT;
	pCodecCtx->channel_layout = pCodecCtx->request_channel_layout = AV_CH_LAYOUT_MONO;
	pCodecCtx->sample_rate = sampeRate;
	pCodecCtx->channels = pCodecCtx->request_channels = channels;
	pCodecCtx->bit_rate = 128000;

	auto error = m_ffmpeg.codec.avcodec_open2(pCodecCtx, pCodec, nullptr);

	if (error < 0) throw std::runtime_error("Can't open audio codec.");

	pFrame = m_ffmpeg.utils.av_frame_alloc();
	pFrame->format = pCodecCtx->sample_fmt;
}

MP3AudioDecoder::~MP3AudioDecoder()
{
	this->Close();
}

void MP3AudioDecoder::DecodeFrame(AVPacket& pkt)
{
	uint8_t inbuf[AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
	
	while (pkt.size)
	{
		// Use the "inbuf" because it has the extra room at the end that the encoder secretly needs.
		//for (int i = 0; i < pkt.size / sizeof(int16_t); ++i)
		//{
		//	auto dest = reinterpret_cast<int16_t*>(inbuf);
		//	auto source = reinterpret_cast<float*>(pkt.data);
		//	dest[i] = source[i];
		//}
		memcpy(inbuf, pkt.data, pkt.size);

		pkt.data = inbuf;
		int out_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;

		int got_frame = 0;
		auto length = m_ffmpeg.codec.avcodec_decode_audio4(pCodecCtx, pFrame, &got_frame, &pkt);
		pkt.data += length;
		pkt.size -= length;
		if (got_frame)
		{
			if (this->callback)
			{
				this->callback(*pFrame);
			}
		}
	}
}

void MP3AudioDecoder::Close()
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