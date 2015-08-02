#include "FFmpegHelper.h"

// http ://sourceforge.net/p/simplestffmpegvideoencoder/code/ci/master/tree/simplest_ffmpeg_video_encoder/simplest_ffmpeg_video_encoder.cpp
class encodeToVideoFile
{
private:
	FFmpegHelper *m_ffmpeg;

	int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index){
		int ret;
		int got_frame;
		AVPacket enc_pkt;
		if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
			CODEC_CAP_DELAY))
			return 0;
		while (1) {
			enc_pkt.data = NULL;
			enc_pkt.size = 0;
			m_ffmpeg->codec.av_init_packet(&enc_pkt);
			ret = m_ffmpeg->codec.avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
			m_ffmpeg->utils.av_frame_free(NULL);
			if (ret < 0)
				break;
			if (!got_frame){
				ret = 0;
				break;
			}
			printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
			/* mux encoded frame */
			ret = m_ffmpeg->format.av_write_frame(fmt_ctx, &enc_pkt);
			if (ret < 0)
				break;
		}
		return ret;
	}

public:
	encodeToVideoFile(FFmpegHelper *ffmpeg, char *inputFileName, char* outputFilename, int width, int height, int fps) : m_ffmpeg(ffmpeg)
	{
		AVFormatContext* pFormatCtx;
		AVStream* video_st;
		AVCodecContext* pCodecCtx;
		AVCodec* pCodec;
		AVPacket pkt;
		uint8_t* picture_buf;
		AVFrame* pFrame;
		int picture_size;
		int y_size;
		int framecnt = 0;
		FILE *in_file = fopen(inputFileName, "rb");   //Input raw YUV data
		int in_w = width, in_h = height;
		//const char* out_file = "src01.h264";              //Output Filepath 
		//const char* out_file = "src01.ts";
		//const char* out_file = "src01.hevc";
		const char* out_file = outputFilename;

		//Method1.
		pFormatCtx = m_ffmpeg->format.avformat_alloc_context();
		//Guess Format
		{
			auto fmt = m_ffmpeg->format.av_guess_format(NULL, out_file, NULL);
			pFormatCtx->oformat = fmt;

			//Method 2.
			//m_ffmpeg->format.avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
			//fmt = pFormatCtx->oformat;
		}

		//Open output URL
		if (m_ffmpeg->format.avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0){
			printf("Failed to open output file! \n");
			return;
		}

		pCodec = m_ffmpeg->codec.avcodec_find_encoder_by_name("libx264");
		//pCodec = m_ffmpeg->codec.avcodec_find_encoder_by_name("h264_qsv"); // TODO: Why doesn't this work?
		if (!pCodec){
			printf("Can not find encoder! \n");
			return;
		}

		video_st = m_ffmpeg->format.avformat_new_stream(pFormatCtx, pCodec);
		video_st->time_base.num = 1;
		video_st->time_base.den = fps;

		if (video_st == NULL){
			return;
		}
		//Param that must set
		pCodecCtx = video_st->codec;
		//pCodecCtx->codec_id =AV_CODEC_ID_HEVC;
		pCodecCtx->codec_id = video_st->codec->codec_id;
		pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
		pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
		pCodecCtx->width = in_w;
		pCodecCtx->height = in_h;
		pCodecCtx->time_base.num = video_st->time_base.num;
		pCodecCtx->time_base.den = video_st->time_base.den;
		pCodecCtx->bit_rate = 400000;
		pCodecCtx->gop_size = video_st->time_base.den * 10;
		//H264
		//pCodecCtx->me_range = 16;
		//pCodecCtx->max_qdiff = 4;
		//pCodecCtx->qcompress = 0.6;
		pCodecCtx->qmin = 10;
		pCodecCtx->qmax = 51;

		//Optional Param
		pCodecCtx->max_b_frames = 3;

		// Set Option
		AVDictionary *param = 0;
		//H.264
		if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
			m_ffmpeg->utils.av_dict_set(&param, "preset", "slow", 0);
			m_ffmpeg->utils.av_dict_set(&param, "tune", "zerolatency", 0);
			//av_dict_set(&param, "profile", "main", 0);
		}
		//H.265
		if (pCodecCtx->codec_id == AV_CODEC_ID_H265){
			m_ffmpeg->utils.av_dict_set(&param, "preset", "ultrafast", 0);
			m_ffmpeg->utils.av_dict_set(&param, "tune", "zero-latency", 0);
		}

		//Show some Information
		m_ffmpeg->format.av_dump_format(pFormatCtx, 0, out_file, 1);

		if (m_ffmpeg->codec.avcodec_open2(pCodecCtx, pCodec, &param) < 0){
			printf("Failed to open encoder! \n");
			return;
		}

		pFrame = m_ffmpeg->utils.av_frame_alloc();
		picture_size = m_ffmpeg->codec.avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
		picture_buf = (uint8_t *)m_ffmpeg->utils.av_malloc(picture_size);
		m_ffmpeg->codec.avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

		//Write File Header
		m_ffmpeg->format.avformat_write_header(pFormatCtx, NULL);

		m_ffmpeg->codec.av_new_packet(&pkt, picture_size);

		y_size = pCodecCtx->width * pCodecCtx->height;

		for (int i = 0; !feof(in_file); i++){
			//Read raw YUV data
			auto cbRead = fread(picture_buf, 1, y_size * 3 / 2, in_file);
			if (cbRead == y_size * 3 / 2)
			{
				pFrame->data[0] = picture_buf;              // Y
				pFrame->data[1] = picture_buf + y_size;      // U 
				pFrame->data[2] = picture_buf + y_size * 5 / 4;  // V
				//PTS
				pFrame->pts = i;
				int got_picture = 0;
				//Encode
				int ret = m_ffmpeg->codec.avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
				if (ret < 0){
					printf("Failed to encode! \n");
					return;
				}
				if (got_picture == 1){
					printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
					framecnt++;
					pkt.stream_index = video_st->index;
					ret = m_ffmpeg->format.av_write_frame(pFormatCtx, &pkt);
					m_ffmpeg->codec.av_free_packet(&pkt);
				}
			}
			else if (cbRead != 0 || (i == cbRead == 0))
			{	// Either we could not ready anything at all
				// or we read a bunch and then ran out of bytes when we did not expect to
				// in the second case, that usually means that the width or height doesn't match.
				printf("Failed reading input file\n");
				return;
			}
		}

		//Flush Encoder
		int ret = flush_encoder(pFormatCtx, 0);
		if (ret < 0) {
			printf("Flushing encoder failed\n");
			return;
		}

		//Write file trailer
		m_ffmpeg->format.av_write_trailer(pFormatCtx);

		//Clean
		if (video_st){
			m_ffmpeg->codec.avcodec_close(video_st->codec);
			m_ffmpeg->utils.av_free(pFrame);
			m_ffmpeg->utils.av_free(picture_buf);
		}
		m_ffmpeg->format.avio_close(pFormatCtx->pb);
		m_ffmpeg->format.avformat_free_context(pFormatCtx);

		fclose(in_file);
	}
};

// http://sourceforge.net/p/simplestffmpegvideoencoder/code/ci/master/tree/simplest_ffmpeg_video_encoder_pure/simplest_ffmpeg_video_encoder_pure.cpp
class encodeRawFile
{
private:
	FFmpegHelper *m_ffmpeg;

public:
	encodeRawFile(FFmpegHelper *ffmpeg, char *filename_in, char* filename_out, int in_w, int in_h, int fps) : m_ffmpeg(ffmpeg)
	{
		AVCodec *pCodec;
		AVCodecContext *pCodecCtx = NULL;
		int i, ret, got_output;
		FILE *fp_in;
		FILE *fp_out;
		AVFrame *pFrame;
		AVPacket pkt;
		int y_size;
		int framecnt = 0;

		//auto intelCodec = m_ffmpeg->codec.avcodec_find_encoder_by_name("h264_qsv"); // TODO: why doesn't this work?
		auto x264Codec = m_ffmpeg->codec.avcodec_find_encoder_by_name("libx264");

		pCodec = x264Codec;
		if (!pCodec) {
			printf("Codec not found\n");
			return;
		}

		AVCodecID codec_id = pCodec->id;

		pCodecCtx = m_ffmpeg->codec.avcodec_alloc_context3(pCodec);
		if (!pCodecCtx) {
			printf("Could not allocate video codec context\n");
			return;
		}

		pCodecCtx->bit_rate = 400000;
		pCodecCtx->width = in_w;
		pCodecCtx->height = in_h;
		pCodecCtx->time_base.num = 1;
		pCodecCtx->time_base.den = fps;
		pCodecCtx->gop_size = 10;
		pCodecCtx->max_b_frames = 1;
		pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

		if (codec_id == AV_CODEC_ID_H264)
		{
			//m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);
			//m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
			//m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
			m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "preset", "7", 0);
			m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "async_depth", "1", 0);
			m_ffmpeg->utils.av_opt_set(pCodecCtx->priv_data, "bf", "0", 0);
		}

		if (m_ffmpeg->codec.avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
			printf("Could not open codec\n");
			return;
		}

		pFrame = m_ffmpeg->utils.av_frame_alloc();
		if (!pFrame) {
			printf("Could not allocate video frame\n");
			return;
		}
		pFrame->format = pCodecCtx->pix_fmt;
		pFrame->width = pCodecCtx->width;
		pFrame->height = pCodecCtx->height;

		ret = m_ffmpeg->utils.av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
		if (ret < 0) {
			printf("Could not allocate raw picture buffer\n");
			return;
		}
		//Input raw data
		fp_in = fopen(filename_in, "rb");
		if (!fp_in) {
			printf("Could not open %s\n", filename_in);
			return;
		}
		//Output bitstream
		fp_out = fopen(filename_out, "wb");
		if (!fp_out) {
			printf("Could not open %s\n", filename_out);
			return;
		}

		y_size = pCodecCtx->width * pCodecCtx->height;
		//Encode
		for (i = 0; !feof(fp_in); i++) {
			m_ffmpeg->codec.av_init_packet(&pkt);
			pkt.data = NULL;    // packet data will be allocated by the encoder
			pkt.size = 0;
			//Read raw YUV data
			auto cbRead = fread(pFrame->data[0], 1, y_size, fp_in);
			//assert(cbRead == y_size);
			cbRead = fread(pFrame->data[1], 1, y_size / 4, fp_in);
			//assert(cbRead == y_size / 4);
			cbRead = fread(pFrame->data[2], 1, y_size / 4, fp_in);
			//assert(cbRead == y_size / 4);

			if (cbRead != y_size / 4)
			{
				break;
			}

			pFrame->pts = i;
			/* encode the image */
			ret = m_ffmpeg->codec.avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
			if (ret < 0) {
				printf("Error encoding frame\n");
				return;
			}
			if (got_output) {
				printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
				framecnt++;
				fwrite(pkt.data, 1, pkt.size, fp_out);
				m_ffmpeg->codec.av_free_packet(&pkt);
			}
		}

		//Flush Encoder
		for (got_output = 1; got_output; i++) {
			ret = m_ffmpeg->codec.avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
			if (ret < 0) {
				printf("Error encoding frame\n");
				return;
			}
			if (got_output) {
				printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", pkt.size);
				fwrite(pkt.data, 1, pkt.size, fp_out);
				m_ffmpeg->codec.av_free_packet(&pkt);
			}
		}

		fclose(fp_out);
		m_ffmpeg->codec.avcodec_close(pCodecCtx);
		m_ffmpeg->utils.av_free(pCodecCtx);
		m_ffmpeg->utils.av_freep(&pFrame->data[0]);
		m_ffmpeg->utils.av_frame_free(&pFrame);
	}
};

FFmpegHelper::FFmpegHelper() :
	deviceDll("avdevice-56.dll"),
	codec("avcodec-56.dll"),
	utils("avutil-54.dll"),
	format("avformat-56.dll")
{
	this->deviceDll.avdevice_register_all();
	this->codec.avcodec_register_all();
	this->format.av_register_all();

	auto fileTest = new encodeToVideoFile(this, "test.yuv", "test.h264", 640, 480, 15);
	delete fileTest;

	auto streamTest = new encodeRawFile(this, "test.yuv", "test-raw.h264", 640, 480, 15);
	delete streamTest;

	//const char *codecName = false ? "h264_qsv" : "libx264";
	//auto encoder = encoderHelper(codecName, this);

	//// Read a raw YUV image source and encode it
	//auto filename = "test.yuv";
	//FILE *srcFile = fopen(filename, "rb");
	//int srcWidth = 640, srcHeight = 480;
	//auto buffer = (byte *)malloc(srcWidth * 3 / 2 * srcHeight);
	//int frameCount = 0; // The test file has 138 frames in it.
	//int frameSize = srcWidth * 3 / 2 * srcHeight;
	//do
	//{
	//	if (fread(buffer, 1, frameSize, srcFile) == frameSize)
	//	{
	//		++frameCount;
	//		encoder.encodeFrame(buffer);
	//	}
	//} while (!feof(srcFile));

	//fclose(srcFile);
	//encoder.stop();

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