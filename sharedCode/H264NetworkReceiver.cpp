#include "H264NetworkReceiver.h"

namespace
{
	std::string& ReplaceAllSubstrings(std::string& source, const std::string& token, const std::string& replacement)
	{
		int index = 0;
		while ((index = source.find(token, index)) != std::string::npos)
		{
			source.erase(index, token.length());
			source.insert(index, replacement);
			index += replacement.length();
		}

		return source;
	}
}

H264NetworkReceiver::H264NetworkReceiver() :
	closeHandle(CreateEventA(NULL, TRUE, FALSE, NULL)),
	ifmt_ctx(NULL),
	videoindex(-1),
	audioindex(-1),
	initialized(false),
	closed(false),
	hasReceviedPackets(false)
{
}

H264NetworkReceiver::~H264NetworkReceiver()
{
	this->Close();
}

void H264NetworkReceiver::Start(const std::string& remote, int port, FrameCallback videoCallback, FrameCallback audioCallback)
{
	if (this->initialized)
	{
		throw std::runtime_error("Already started.");
	}

	this->remoteIp = remote;
	this->remoteVideoPort = port;
	this->callback = videoCallback;
	this->callbackAudio = audioCallback;
	this->initialized = true;
	this->thread = CreateThread(NULL, 0, &H264NetworkReceiver::ThreadMarshaller, this, 0, NULL);
}

DWORD H264NetworkReceiver::ThreadMarshaller(LPVOID context)
{
	((H264NetworkReceiver*)context)->OtherThread();
	return 0;
}

void H264NetworkReceiver::OtherThread()
{
	std::string sdpConfig(
		"v=0\n"
		"o=- 0 0 IN IP4 127.0.0.1\n"
		"s=No Name\n"
		"t=0 0\n"
		"a=tool:libavformat 56.40.101\n"
		"m=video _VIDEOPORT_ RTP/AVP 96\n"
		"c=IN IP4 _REMOTEIP_\n"
		"b=AS:1000\n"
		"a=rtpmap:96 H264/90000\n"
		"a=fmtp:96 packetization-mode=1; sprop-parameter-sets=J00AFpaIBQHtgIgAUWFQCYloBKA=,KO4BryA=; profile-level-id=4D0016\n"
		"m=audio _AUDIOPORT_ RTP/AVP 97\n"
		"c=IN IP4 _REMOTEIP_\n"
		"b=AS:128\n"
		"a=rtpmap:97 MPEG4-GENERIC/22050/1\n"
		"a = fmtp:97 profile - level - id = 1; mode = AAC - hbr; sizelength = 13; indexlength = 3; indexdeltalength = 3; config = 1388");

	ReplaceAllSubstrings(sdpConfig, "_REMOTEIP_", this->remoteIp);
	ReplaceAllSubstrings(sdpConfig, "_VIDEOPORT_", std::to_string(this->remoteVideoPort));
	ReplaceAllSubstrings(sdpConfig, "_AUDIOPORT_", "11010");

	// Write the SDP to a temp file so we can pass it in later
	char tempPath[MAX_PATH];
	char tempFile[MAX_PATH];
	GetTempPathA(sizeof(tempPath), tempPath);
	GetTempFileNameA(tempPath, NULL, timeGetTime(), tempFile);
	auto extention = strchr(tempFile, '.');
	if (!extention)
	{
		strcat_s(tempFile, sizeof(tempFile), ".dsp");
	}
	else
	{
		strcpy_s(extention, sizeof(tempFile) - (extention - tempFile), ".sdp");
	}

	FILE *fp;
	if (fopen_s(&fp, tempFile, "wt"))
	{
		throw std::runtime_error((std::string("Could not create temp file ") + tempFile).c_str());
	}

	if (sdpConfig.length() != fwrite(sdpConfig.c_str(), 1, sdpConfig.length(), fp))
	{
		fclose(fp);
		throw std::runtime_error((std::string("Could not write temp file ") + tempFile).c_str());
	}

	fclose(fp);

	//AVOutputFormat *ofmt = NULL;
	//Input AVFormatContext and Output AVFormatContext
	//AVFormatContext *ofmt_ctx = NULL;
	int ret;

	//const char *in_filename, *out_filename;
	////in_filename  = "rtp://239.0.0.210:11000";
	//out_filename = "receive.h264";

	//Input
	//if ((ret = m_ffmpeg.format.avformat_open_input(&ifmt_ctx, "test.sdp", 0, 0)) < 0) {
	if ((ret = m_ffmpeg.format.avformat_open_input(&ifmt_ctx, tempFile, 0, 0)) < 0) {
		DeleteFileA(tempFile);
		throw std::runtime_error("Could not open input file.");
	}

	DeleteFileA(tempFile);

	//int retriesLeft = 3;
	//while (retriesLeft--)
	//{
	//	if ((ret = m_ffmpeg.format.avformat_find_stream_info(ifmt_ctx, 0)) < 0)
	//	{
	//		if (!retriesLeft)
	//		{
	//			printf("Unable to decode remote video stream.");
	//			throw std::runtime_error("Failed to retrieve input stream information.");
	//		}
	//	}

	//	break;
	//}

	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex = i;
		}
		else if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			audioindex = i;
		}
	}

	// Set these so we don't have to call avformat_find_stream_info (above)
	ifmt_ctx->streams[videoindex]->codec->framerate.num = 15;
	ifmt_ctx->streams[videoindex]->codec->framerate.den = 1;
	ifmt_ctx->streams[videoindex]->codec->time_base.num = 1;
	ifmt_ctx->streams[videoindex]->codec->time_base.num = 30;
	ifmt_ctx->streams[videoindex]->codec->ticks_per_frame = 2;
	ifmt_ctx->streams[videoindex]->codec->pix_fmt = AV_PIX_FMT_YUV420P;
	ifmt_ctx->streams[videoindex]->codec->sample_aspect_ratio.num = 1;
	ifmt_ctx->streams[videoindex]->codec->sample_aspect_ratio.den = 1;

	if (videoindex == -1)
	{
		throw std::runtime_error("Could not find video stream index.");
	}
	//m_ffmpeg.format.av_dump_format(ifmt_ctx, 0, in_filename, 0);

	//Output
	//m_ffmpeg.format.avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename); //RTMP

	//if (!ofmt_ctx) {
	//	printf("Could not create output context\n");
	//	ret = AVERROR_UNKNOWN;
	//	goto end;
	//}
	//ofmt = ofmt_ctx->oformat;
	//for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
	//	//Create output AVStream according to input AVStream
	//	AVStream *in_stream = ifmt_ctx->streams[i];
	//	AVStream *out_stream = m_ffmpeg.format.avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
	//	if (!out_stream) {
	//		printf("Failed allocating output stream\n");
	//		ret = AVERROR_UNKNOWN;
	//		goto end;
	//	}
	//	//Copy the settings of AVCodecContext
	//	ret = m_ffmpeg.codec.avcodec_copy_context(out_stream->codec, in_stream->codec);
	//	if (ret < 0) {
	//		printf("Failed to copy context from input to output stream codec context\n");
	//		goto end;
	//	}
	//	out_stream->codec->codec_tag = 0;
	//	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
	//		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	//}

	//Dump Format------------------
	//m_ffmpeg.format.av_dump_format(ofmt_ctx, 0, out_filename, 1);
	////Open output URL
	//if (!(ofmt->flags & AVFMT_NOFILE)) {
	//	ret = m_ffmpeg.format.avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
	//	if (ret < 0) {
	//		printf("Could not open output URL '%s'", out_filename);
	//		goto end;
	//	}
	//}
	////Write file header
	//ret = m_ffmpeg.format.avformat_write_header(ofmt_ctx, NULL);
	//if (ret < 0) {
	//	printf("Error occurred when opening output URL\n");
	//	goto end;
	//}
	while (WaitForSingleObject(this->closeHandle, 0) == WAIT_TIMEOUT)
	{
		ifmt_ctx->interrupt_callback.callback = &H264NetworkReceiver::Interrupt;
		ifmt_ctx->interrupt_callback.opaque = (void*)this;
		auto ret = m_ffmpeg.format.av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0 && WaitForSingleObject(this->closeHandle, 0) != WAIT_TIMEOUT)
		{
			return;
		}

		//AVStream *in_stream, *out_stream;
		//in_stream = ifmt_ctx->streams[pkt.stream_index];
		//out_stream = ofmt_ctx->streams[pkt.stream_index];
		///* copy packet */
		////Convert PTS/DTS
		//pkt.pts = m_ffmpeg.utils.av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.dts = m_ffmpeg.utils.av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.duration = m_ffmpeg.utils.av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		//Print to Screen
		if (pkt.stream_index == videoindex)
		{
			this->hasReceviedPackets = true;
			if (this->callback)
			{
				this->callback(pkt);
			}
		}
		else if (pkt.stream_index == audioindex)
		{
			this->hasReceviedPackets = true;
			if (this->callbackAudio)
			{
				this->callbackAudio(pkt);
			}
		}

		m_ffmpeg.codec.av_free_packet(&pkt);
	}
}

int H264NetworkReceiver::Interrupt(void* x)
{
	auto rval = WaitForSingleObject(((H264NetworkReceiver *)x)->closeHandle, 0) != WAIT_TIMEOUT;
	return rval;
}

void H264NetworkReceiver::Close()
{
	if (this->initialized && !this->closed)
	{
		this->closed = true;

		SetEvent(this->closeHandle);
		WaitForSingleObject(this->thread, INFINITE);
		this->thread = 0;

		CloseHandle(this->closeHandle);
		this->closeHandle = 0;

		m_ffmpeg.format.avformat_close_input(&ifmt_ctx);
	}
}

bool H264NetworkReceiver::isConnected()
{
	return this->hasReceviedPackets;
}