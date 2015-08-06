#include "H264NetworkSender.h"
#include "LegacyGuards.h"

#define SAMPLING_RATE 44100
#if AUDIO_GOOFING
float *noise_buffer;
void generate_440(float *buffer) {
	int pos;  // sample number we're on
	float volume = 0.1;  // 0 to 1.0, one being the loudest

	for (pos = 0; pos < SAMPLING_RATE; pos++) {
		float a = (2 * 3.14159) * pos / (SAMPLING_RATE / 440.0);
		float v = sin(a) * volume;
		buffer[pos] = v;
	}
}
#endif

H264NetworkSender::H264NetworkSender() : initialized(false), closed(false)
{
#if AUDIO_GOOFING
	noise_buffer = (float *)malloc(SAMPLING_RATE * sizeof(float));
	generate_440(noise_buffer);
#endif
}

H264NetworkSender::~H264NetworkSender()
{
	Close();
}

void H264NetworkSender::Start(const std::string& networkAddress, int width, int height, int fps)
{
	this->out_filename = networkAddress;
	if (this->out_filename.substr(0, 6) != "rtp://")
	{
		this->out_filename = "rtp://" + this->out_filename;
	}

	auto inputFormatContext = m_ffmpeg.format.avformat_alloc_context();
	if (!inputFormatContext) {
		throw std::runtime_error("Could not create input context");
	}

	GuardAVFormatContext inputGuard(inputFormatContext, &m_ffmpeg);

	inputFormatContext->iformat = m_ffmpeg.format.av_find_input_format("h264");
	auto in_stream = m_ffmpeg.format.avformat_new_stream(inputFormatContext, inputFormatContext->video_codec);
	in_stream->codec->codec_type = AVMEDIA_TYPE_VIDEO;
	in_stream->codec->codec_id = AV_CODEC_ID_H264;
	in_stream->codec->framerate.num = 1;
	in_stream->codec->framerate.den = fps;
	in_stream->codec->width = width;
	in_stream->codec->height = height;

	//
	// All of these settings are from having ffmpeg actually parse a sample H264 encoded file.  So these, are the
	// official settings, but it seems like it only needs those that are above here. It's possible that some future issue
	// might be resolved by figuring out which of the settings below should be uncommented and moved above.
	//
	//in_stream->time_base.num = 1;
	//in_stream->time_base.den = 1200000;
	//in_stream->avg_frame_rate.num = fps;
	//in_stream->avg_frame_rate.den = 1;
	//in_stream->r_frame_rate.num = 30;
	//in_stream->r_frame_rate.den = 2;
	//in_stream->pts_wrap_bits = 69;
	//in_stream->codec->framerate.num = fps; // Not sure why the auto-detect code uses num (not den) for this.
	//in_stream->codec->coded_width = width;
	//in_stream->codec->coded_height = height;
	//in_stream->codec->framerate.den = 1;
	//in_stream->codec->time_base.num = 1;
	//in_stream->codec->ticks_per_frame = 2;
	//in_stream->codec->time_base.den = fps * in_stream->codec->ticks_per_frame;
	//in_stream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
	//in_stream->codec->sample_aspect_ratio.num = 1;
	//in_stream->codec->sample_aspect_ratio.den = 1;
	//in_stream->codec->chroma_sample_location = AVCHROMA_LOC_LEFT;
	//in_stream->codec->profile = 100;
	//in_stream->codec->level = 22;
	//in_stream->codec->pkt_timebase.num = 1;
	//in_stream->codec->pkt_timebase.den = 1200000;

	//Output
	m_ffmpeg.format.avformat_alloc_output_context2(&avOutputFormatContext, NULL, "rtp", out_filename.c_str());
	if (!avOutputFormatContext) {
		throw std::runtime_error("Could not create output context");
	}

	avOutputFormat = avOutputFormatContext->oformat;

	//Create output AVStream according to input AVStream
	AVStream *out_stream = m_ffmpeg.format.avformat_new_stream(avOutputFormatContext, in_stream->codec->codec);
	if (!out_stream) {
		throw std::runtime_error("Failed allocating output stream");
	}

#if AUDIO_GOOFING
	auto audioCodec = m_ffmpeg.codec.avcodec_find_encoder_by_name("pcm_f32le");
	AVStream *audio_stream = m_ffmpeg.format.avformat_new_stream(avOutputFormatContext, audioCodec);
	if (!audio_stream) {
		throw std::runtime_error("Failed allocating output stream");
	}
	audio_stream->codec->sample_rate = 44100;
	audio_stream->codec->channels = 1;
	audio_stream->codec->bits_per_coded_sample = audio_stream->codec->bits_per_raw_sample = 16;
	uint8_t out_audio_buffer[SAMPLING_RATE * sizeof(float)];

	AVPacket audioPkt;
	m_ffmpeg.codec.av_init_packet(&audioPkt);
	audioPkt.size = m_ffmpeg.codec.avcodec_encode_audio(audio_stream->codec, (uint8_t*)out_audio_buffer, sizeof(out_audio_buffer), (short *)noise_buffer);
	audioPkt.stream_index = audio_stream->index;
	audioPkt.data = out_audio_buffer;
	m_ffmpeg.format.av_interleaved_write_frame(avOutputFormatContext, &audioPkt);
#endif

	//Copy the settings of AVCodecContext
	auto ret = m_ffmpeg.codec.avcodec_copy_context(out_stream->codec, in_stream->codec);
	if (ret < 0) {
		throw std::runtime_error("Failed to copy context from input to output stream codec context");
	}
	out_stream->codec->codec_tag = 0;
	if (avOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
	{
		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	m_ffmpeg.format.av_dump_format(avOutputFormatContext, 0, out_filename.c_str(), 1);

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

void H264NetworkSender::WriteFrame(AVPacket& pkt)
{
	if (!this->initialized)
	{
		throw std::logic_error("Must initialize class before calling WriteFrame().");
	}

	//ret = av_write_frame(avOutputFormatContext, &pkt);
	if (m_ffmpeg.format.av_interleaved_write_frame(avOutputFormatContext, &pkt) < 0)
	{
		throw std::runtime_error("Error muxing packet");
	}
}

void H264NetworkSender::Close()
{
	if (this->initialized && !this->closed)
	{
		this->closed = true;

		if (avOutputFormatContext && !(avOutputFormat->flags & AVFMT_NOFILE))
			m_ffmpeg.format.avio_close(avOutputFormatContext->pb);
		m_ffmpeg.format.avformat_free_context(avOutputFormatContext);
	}
}