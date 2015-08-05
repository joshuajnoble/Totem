#pragma once

#include "FFmpegImports.h"
#include "YUV420_H264_Encoder.h"
#include "VideoConverters.h"

#include "LegacyGuards.h"

#include <string>
#include <vector>
#include <fstream>

class TestStreamer
{
public:
	TestStreamer()
	{

	}

	int Start()
	{
		FFmpegFactory m_ffmpeg;

		AVOutputFormat *avOutputFormat = NULL;
		AVFormatContext *avFormatContextSource = NULL, *avOutputFormatContext = NULL;
		AVPacket pkt;
		const char *in_filename, *out_filename;
		int ret, i;
		int videoindex = -1;
		int frame_index = 0;
		int64_t start_time = 0;
		in_filename = "test.h264";
		out_filename = "rtp://239.0.0.210:5800";

		//Input
		if ((ret = m_ffmpeg.format.avformat_open_input(&avFormatContextSource, in_filename, 0, 0)) < 0) {
			printf("Could not open input file.");
			goto end;
		}

		if ((ret = m_ffmpeg.format.avformat_find_stream_info(avFormatContextSource, 0)) < 0) {
			printf("Failed to retrieve input stream information");
			goto end;
		}

		for (i = 0; i < avFormatContextSource->nb_streams; i++)
			if (avFormatContextSource->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
				videoindex = i;
				break;
			}

		m_ffmpeg.format.av_dump_format(avFormatContextSource, 0, in_filename, 0);

		//Output
		//avformat_alloc_output_context2(&avOutputFormatContext, NULL, "h264", out_filename); //RTMP
		//avformat_alloc_output_context2(&avOutputFormatContext, NULL, "mpegts", out_filename);//UDP
		m_ffmpeg.format.avformat_alloc_output_context2(&avOutputFormatContext, NULL, "rtp", out_filename);//UDP

		if (!avOutputFormatContext) {
			printf("Could not create output context\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		avOutputFormat = avOutputFormatContext->oformat;
		for (i = 0; i < avFormatContextSource->nb_streams; i++) {
			//Create output AVStream according to input AVStream
			AVStream *in_stream = avFormatContextSource->streams[i];
			AVStream *out_stream = m_ffmpeg.format.avformat_new_stream(avOutputFormatContext, in_stream->codec->codec);
			if (!out_stream) {
				printf("Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				goto end;
			}
			//Copy the settings of AVCodecContext
			ret = m_ffmpeg.codec.avcodec_copy_context(out_stream->codec, in_stream->codec);
			if (ret < 0) {
				printf("Failed to copy context from input to output stream codec context\n");
				goto end;
			}
			out_stream->codec->codec_tag = 0;
			if (avOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
		//Dump Format------------------
		m_ffmpeg.format.av_dump_format(avOutputFormatContext, 0, out_filename, 1);
		//Open output URL
		if (!(avOutputFormat->flags & AVFMT_NOFILE)) {
			ret = m_ffmpeg.format.avio_open(&avOutputFormatContext->pb, out_filename, AVIO_FLAG_WRITE);
			if (ret < 0) {
				printf("Could not open output URL '%s'", out_filename);
				goto end;
			}
		}
		//Write file header
		ret = m_ffmpeg.format.avformat_write_header(avOutputFormatContext, NULL);
		if (ret < 0) {
			printf("Error occurred when opening output URL\n");
			goto end;
		}

		start_time = m_ffmpeg.utils.av_gettime();
		while (1) {
			AVStream *in_stream, *out_stream;
			//Get an AVPacket
			ret = m_ffmpeg.format.av_read_frame(avFormatContextSource, &pkt);
			if (ret < 0)
				break;
			//FIX：No PTS (Example: Raw H.264)
			//Simple Write PTS
			if (pkt.pts == AV_NOPTS_VALUE){
				//Write PTS
				AVRational time_base1 = avFormatContextSource->streams[videoindex]->time_base;
				//Duration between 2 frames (us)
				int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(avFormatContextSource->streams[videoindex]->r_frame_rate);
				//Parameters
				pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
				pkt.dts = pkt.pts;
				pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			}
			//Important:Delay
			if (pkt.stream_index == videoindex){
				AVRational time_base = avFormatContextSource->streams[videoindex]->time_base;
				AVRational time_base_q = { 1, AV_TIME_BASE };
				int64_t pts_time = m_ffmpeg.utils.av_rescale_q(pkt.dts, time_base, time_base_q);
				int64_t now_time = m_ffmpeg.utils.av_gettime() - start_time;
				if (pts_time > now_time)
					m_ffmpeg.utils.av_usleep(pts_time - now_time);

			}

			in_stream = avFormatContextSource->streams[pkt.stream_index];
			out_stream = avOutputFormatContext->streams[pkt.stream_index];

			/* copy packet */
			//Convert PTS/DTS
			pkt.pts = m_ffmpeg.utils.av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.dts = m_ffmpeg.utils.av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.duration = m_ffmpeg.utils.av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;

			//Print to Screen
			if (pkt.stream_index == videoindex){
				printf("Send %8d video frames to output URL\n", frame_index);
				frame_index++;
			}

			//ret = av_write_frame(avOutputFormatContext, &pkt);
			ret = m_ffmpeg.format.av_interleaved_write_frame(avOutputFormatContext, &pkt);

			if (ret < 0) {
				printf("Error muxing packet\n");
				break;
			}

			m_ffmpeg.codec.av_free_packet(&pkt);

		}
		//Write file trailer
		m_ffmpeg.format.av_write_trailer(avOutputFormatContext);
	end:
		m_ffmpeg.format.avformat_close_input(&avFormatContextSource);
		/* close output */
		if (avOutputFormatContext && !(avOutputFormat->flags & AVFMT_NOFILE))
			m_ffmpeg.format.avio_close(avOutputFormatContext->pb);
		m_ffmpeg.format.avformat_free_context(avOutputFormatContext);
		if (ret < 0 && ret != AVERROR_EOF) {
			printf("Error occurred.\n");
			return -1;
		}
		return 0;
	}
};

class TestStreamerLive
{
private:
	FFmpegFactory m_ffmpeg;
	AVOutputFormat *avOutputFormat = NULL;
	AVFormatContext *avOutputFormatContext = NULL;
	const char* in_filename = "test.h264";
	const char* out_filename = "rtp://239.0.0.210:5800";
	bool initialized, closed;

public:
	TestStreamerLive() : initialized(false), closed(false)
	{
	}

	~TestStreamerLive()
	{
		Close();
	}

	int Start()
	{
		auto inputFormatContext = m_ffmpeg.format.avformat_alloc_context();
		if (!inputFormatContext) {
			throw std::runtime_error("Could not create input context");
		}

		GuardAVFormatContext inputGuard(inputFormatContext, &m_ffmpeg);

		inputFormatContext->iformat = m_ffmpeg.format.av_find_input_format("h264");
		auto in_stream = m_ffmpeg.format.avformat_new_stream(inputFormatContext, inputFormatContext->video_codec);
		in_stream->time_base.num = 1;
		in_stream->time_base.den = 1200000;
		in_stream->avg_frame_rate.num = 15;
		in_stream->avg_frame_rate.den = 1;
		in_stream->r_frame_rate.num = 30;
		in_stream->r_frame_rate.den = 2;
		in_stream->pts_wrap_bits = 69;
		in_stream->codec->codec_type = AVMEDIA_TYPE_VIDEO;
		in_stream->codec->codec_id = AV_CODEC_ID_H264;
		in_stream->codec->time_base.num = 1;
		in_stream->codec->time_base.den = 30;
		in_stream->codec->ticks_per_frame = 2;
		in_stream->codec->width = 640;
		in_stream->codec->height = 480;
		in_stream->codec->coded_width = 640;
		in_stream->codec->coded_height = 480;
		in_stream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
		in_stream->codec->sample_aspect_ratio.num = 1;
		in_stream->codec->sample_aspect_ratio.den = 1;
		in_stream->codec->chroma_sample_location = AVCHROMA_LOC_LEFT;
		in_stream->codec->profile = 100;
		in_stream->codec->level = 22;
		in_stream->codec->framerate.num = 15;
		in_stream->codec->framerate.den = 1;
		in_stream->codec->pkt_timebase.num = 1;
		in_stream->codec->pkt_timebase.den = 1200000;

		//Output
		m_ffmpeg.format.avformat_alloc_output_context2(&avOutputFormatContext, NULL, "rtp", out_filename);//UDP
		if (!avOutputFormatContext) {
			throw std::runtime_error("Could not create output context");
		}

		avOutputFormat = avOutputFormatContext->oformat;

		//Create output AVStream according to input AVStream
		AVStream *out_stream = m_ffmpeg.format.avformat_new_stream(avOutputFormatContext, in_stream->codec->codec);
		if (!out_stream) {
			throw std::runtime_error("Failed allocating output stream");
		}

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

		m_ffmpeg.format.av_dump_format(avOutputFormatContext, 0, out_filename, 1);

		//Open output URL
		if (!(avOutputFormat->flags & AVFMT_NOFILE)) {
			ret = m_ffmpeg.format.avio_open(&avOutputFormatContext->pb, out_filename, AVIO_FLAG_WRITE);
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

	void WriteFrame(AVPacket& pkt)
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

	void Close()
	{
		if (this->initialized && !this->closed)
		{
			this->closed = true;

			if (avOutputFormatContext && !(avOutputFormat->flags & AVFMT_NOFILE))
				m_ffmpeg.format.avio_close(avOutputFormatContext->pb);
			m_ffmpeg.format.avformat_free_context(avOutputFormatContext);
		}
	}
};

class EncodeToNetworkStream
{
private:
	FFmpegFactory& m_ffmpeg;
	FrameCallback callback;

	int cInputFrames = 0;
	AVPacket pkt;
	int yBlocksize, uBlockSize, vBlockSize;

	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVFrame *pFrame = NULL;

public:
	EncodeToNetworkStream(FFmpegFactory& ffmpeg, int width, int height, int fps, const std::string& networkAddress) :
		m_ffmpeg(ffmpeg),
		callback(callback)
	{
		auto out_filename = std::string("rtp://") + networkAddress;
		auto _pVideoOutputFmt = m_ffmpeg.format.av_guess_format("rtp", NULL, NULL);
		auto _pVideoFormatCtx = m_ffmpeg.format.avformat_alloc_context();
		_pVideoOutputFmt->video_codec = CODEC_ID_H264;
		m_ffmpeg.format.avformat_alloc_output_context2(&_pVideoFormatCtx, _pVideoOutputFmt, "rtp", networkAddress.c_str());

		auto codec = m_ffmpeg.codec.avcodec_find_encoder_by_name("h264_qsv");
		auto _videoStream = m_ffmpeg.format.avformat_new_stream(_pVideoFormatCtx, codec);
		_videoStream->id = _pVideoFormatCtx->nb_streams - 1;
		auto _codecContext = _videoStream->codec;

		m_ffmpeg.codec.avcodec_get_context_defaults3(_codecContext, codec);
		_codecContext->codec_id = _pVideoOutputFmt->video_codec;
		_codecContext->bit_rate = 400000;
		_codecContext->width = width;
		_codecContext->height = height;
		_codecContext->time_base.num = 1;
		_codecContext->time_base.den = fps;
		_codecContext->pix_fmt = AV_PIX_FMT_QSV;

		if (_pVideoFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			_codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}

		{
			AVCodecContext *c = _videoStream->codec;
			AVDictionary *opt = NULL;
			auto ret = m_ffmpeg.codec.avcodec_open2(c, codec, &opt);
		}

		/* open the output file, if needed */
		if (!(_pVideoOutputFmt->flags & AVFMT_NOFILE))
		{
			m_ffmpeg.format.avio_open(&_pVideoFormatCtx->pb, networkAddress.c_str(), AVIO_FLAG_WRITE);
		}

		m_ffmpeg.format.avformat_write_header(_pVideoFormatCtx, NULL);
#if 0
		AVOutputFormat *ofmt = ofmt_ctx->oformat;
		pCodec = m_ffmpeg.codec.avcodec_find_encoder_by_name("rtp");
		if (!pCodec) {
			throw std::exception((std::string("Coded not found: ") + "rtp").c_str());
		}

		pCodecCtx = m_ffmpeg.codec.avcodec_alloc_context3(pCodec);
		if (!pCodecCtx) {
			throw std::exception("Could not allocate video codec context.");
		}

		//Create output AVStream according to input AVStream
		AVStream *out_stream = m_ffmpeg.format.avformat_new_stream(ofmt_ctx, NULL);
		if (!out_stream) {
			throw std::exception("Failed allocating output stream.");
		}

		////Copy the settings of AVCodecContext
		//auto ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
		//if (ret < 0) {
		//	throw std::exception("Failed to copy context from input to output stream codec context.");
		//}
		//out_stream->codec->codec_tag = 0;
		//if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		//	out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

		//Dump Format------------------
		m_ffmpeg.format.av_dump_format(ofmt_ctx, 0, out_filename.c_str(), 1);


		GuardCodecContext codecGuard(pCodecCtx, &m_ffmpeg);

		pCodecCtx->bit_rate = 400000;
		pCodecCtx->width = width;
		pCodecCtx->height = height;
		pCodecCtx->time_base.num = 1;
		pCodecCtx->time_base.den = fps;
		pCodecCtx->gop_size = 10;
		pCodecCtx->max_b_frames = 3;
		pCodecCtx->pix_fmt = AV_PIX_FMT_NV12;// AV_PIX_FMT_YUV420P;

		m_ffmpeg.utils.av_opt_set(pCodecCtx->priv_data, "async_depth", "1", 0);
		pCodecCtx->max_b_frames = 0;

		auto error = m_ffmpeg.codec.avcodec_open2(pCodecCtx, pCodec, NULL);
		if (error < 0)
		{
			printf("Could not open codec.");
		}

		pFrame = m_ffmpeg.utils.av_frame_alloc();
		if (!pFrame) {
			throw std::exception("Could not allocate video frame.");
		}

		pFrame->format = pCodecCtx->pix_fmt;
		pFrame->width = pCodecCtx->width;
		pFrame->height = pCodecCtx->height;

		auto ret = m_ffmpeg.utils.av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
		if (ret < 0) {
			throw std::exception("Could not allocate raw picture buffer.");
		}

		yBlocksize = this->pFrame->height * this->pFrame->linesize[0];
		uBlockSize = this->pFrame->height / 2 * this->pFrame->linesize[1];
		vBlockSize = this->pFrame->height / 2 * this->pFrame->linesize[2];

		codecGuard.Release();
#endif
	}

	~EncodeToNetworkStream()
	{
		AVCodec *pCodec = NULL;
		AVCodecContext *pCodecCtx = NULL;
		AVFrame *pFrame = NULL;

		if (this->pCodecCtx)
		{
			m_ffmpeg.codec.avcodec_close(this->pCodecCtx);
			this->pCodecCtx = NULL;
		}

		if (this->pCodecCtx)
		{
			m_ffmpeg.utils.av_free(pCodecCtx);
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
				m_ffmpeg.utils.av_freep(&this->pFrame->data[0]);
			}

			m_ffmpeg.utils.av_frame_free(&this->pFrame);
			this->pFrame = NULL;
		}
	}

	void WriteFrame(const uint8_t* framebuffer, int frameSize)
	{
		if (framebuffer == NULL)
		{
			throw std::invalid_argument("framebuffer");
		}

		m_ffmpeg.codec.av_init_packet(&pkt);
		pkt.data = NULL; // packet data will be allocated by the encoder
		pkt.size = 0;

		pFrame->data[0] = const_cast<uint8_t*>(framebuffer);
		pFrame->data[1] = const_cast<uint8_t*>(framebuffer + yBlocksize);
		pFrame->data[2] = const_cast<uint8_t*>(framebuffer + yBlocksize + uBlockSize);

		pFrame->pts = this->cInputFrames++;
		int got_output;

		/* encode the image */
		auto ret = m_ffmpeg.codec.avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
		if (ret < 0)
		{
			throw std::exception("Error encoding frame.");
		}

		if (got_output)
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
		//Flush Encoder even at least one frame was given to us
		int got_output = this->cInputFrames ? 1 : 0;
		while (got_output)
		{
			auto ret = m_ffmpeg.codec.avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
			if (ret < 0) {
				printf("Error encoding frame\n");
				return;
			}
			if (got_output) {
				if (this->callback)
				{
					this->callback(pkt);
				}
				m_ffmpeg.codec.av_free_packet(&pkt);
			}
		}

		m_ffmpeg.codec.avcodec_close(pCodecCtx);
		m_ffmpeg.utils.av_free(pCodecCtx);
		pCodecCtx = NULL;
		this->pCodec = NULL;

		if (pFrame)
		{
			if (pFrame->data)
			{
				//m_ffmpeg.utils.av_freep(&pFrame->data[0]);
			}

			m_ffmpeg.utils.av_frame_free(&pFrame);
			pFrame = NULL;
		}
	}
};

class FFmpegNetworkServer
{
private:
	FFmpegFactory& m_ffmpeg;
	std::auto_ptr<EncodeRGBToH264> rgbToh264Encoder;
	std::auto_ptr<EncodeToNetworkStream> streamEncoder;
	bool closed;

	//void ProcessEncodedFrame(const uint8_t* encodedFrame, int frameSize)
	void ProcessEncodedFrame(AVPacket& packet)
	{
		this->streamEncoder->WriteFrame(packet.data, packet.size);
	}

public:
	FFmpegNetworkServer(FFmpegFactory &ffmpeg) : m_ffmpeg(ffmpeg), closed(false)
	{
	}

	~FFmpegNetworkServer()
	{
		this->Close();
	}

	void Start(int width, int height, int fps, const std::string& networkAddress)
	{
		auto callback = std::bind(&FFmpegNetworkServer::ProcessEncodedFrame, this, std::placeholders::_1);
		this->rgbToh264Encoder.reset(new EncodeRGBToH264(m_ffmpeg, callback));
		this->streamEncoder.reset(new EncodeToNetworkStream(m_ffmpeg, width, height, fps, networkAddress));
	}

	void WriteFrame(const uint8_t *srcBytes)
	{
		this->rgbToh264Encoder->WriteFrame(srcBytes);
	}

	void Close()
	{
		if (!this->closed)
		{
			this->rgbToh264Encoder.reset();
			this->streamEncoder.reset();
			this->closed = true;
		}
	}
};