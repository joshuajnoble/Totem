#include "ofMain.h"

#include "AACNetworkSender.h"
#include "LegacyGuards.h"

AACNetworkSender::AACNetworkSender() : initialized(false), closed(false)
{
}

AACNetworkSender::~AACNetworkSender()
{
	Close();
}

void AACNetworkSender::Start(const std::string& networkAddress, int channels, int sample_rate)
{
	this->out_filename = networkAddress;
	if (this->out_filename.substr(0, 6) != "rtp://")
	{
		this->out_filename = "rtp://" + this->out_filename;
	}

	auto pCodec = m_ffmpeg.codec.avcodec_find_encoder_by_name("libfdk_aac");
	auto pCodecCtx = m_ffmpeg.codec.avcodec_alloc_context3(pCodec);
	pCodecCtx->codec_id = pCodec->id;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodecCtx->channel_layout = AV_CH_LAYOUT_MONO;
	pCodecCtx->sample_rate = sample_rate;
	pCodecCtx->channels = channels;
	
	m_ffmpeg.format.avformat_alloc_output_context2(&avOutputFormatContext, NULL, "rtp", out_filename.c_str());
	m_ffmpeg.codec.avcodec_free_context(&pCodecCtx);
	if (!avOutputFormatContext) {
		throw std::runtime_error("Could not create output context");
	}

	avOutputFormat = avOutputFormatContext->oformat;

	AVStream *out_stream = m_ffmpeg.format.avformat_new_stream(avOutputFormatContext, pCodec);
	if (!out_stream) {
		throw std::runtime_error("Failed allocating output stream");
	}
	out_stream->codec->codec_id = pCodec->id;
	out_stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
	out_stream->codec->sample_fmt = AV_SAMPLE_FMT_S16;
	out_stream->codec->channel_layout = AV_CH_LAYOUT_MONO;
	out_stream->codec->sample_rate = 22050;
	out_stream->codec->channels = 1;

	m_ffmpeg.format.av_dump_format(avOutputFormatContext, 0, out_filename.c_str(), 1);

	int ret;

	//Open output URL
	if (!(avOutputFormat->flags & AVFMT_NOFILE)) {
		ret = m_ffmpeg.format.avio_open(&avOutputFormatContext->pb, out_filename.c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			throw std::runtime_error((std::string("Could not open output URL ") + out_filename).c_str());
		}
	}

	//Write file header
	ret = m_ffmpeg.format.avformat_write_header(avOutputFormatContext, NULL);
	if (ret < 0) {
		throw std::runtime_error("Error occurred when opening output URL");
	}

	//m_ffmpeg.codec.avcodec_close(in_stream->codec);
	//m_ffmpeg.codec.avcodec_free_context(&in_stream->codec);

	this->initialized = true;
}

void AACNetworkSender::WriteFrame(AVPacket& pkt)
{
	if (m_ffmpeg.format.av_interleaved_write_frame(avOutputFormatContext, &pkt) < 0)
	{
		printf("Error streaming audio packet.");
		return;
	}
}

void AACNetworkSender::Close()
{
	if (this->initialized && !this->closed)
	{
		this->closed = true;

		if (avOutputFormatContext && !(avOutputFormat->flags & AVFMT_NOFILE))
			m_ffmpeg.format.avio_close(avOutputFormatContext->pb);
		m_ffmpeg.format.avformat_free_context(avOutputFormatContext);
	}
}