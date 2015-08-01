#include "FFmpegHelper.h"

FFmpegHelper::FFmpegHelper() :
	deviceDll("avdevice-56.dll"),
	codecDll("avcodec-56.dll"),
	imgUtilsDll("avutil-54.dll")
{
	this->deviceDll.avdevice_register_all();

	this->codecDll.avcodec_register_all();
	//auto encoder = this->codecDll.avcodec_find_encoder_by_name("h264_qsv");
	//assert(encoder); // Must have the intel encoder present!

	//AVCodecContext *c = this->codecDll.avcodec_alloc_context3(encoder);
	//c->width = 640;
	//c->height = 480;
	//c->time_base.num = 1;
	//c->time_base.den = 15;
	//c->gop_size = 10; /* emit one intra frame every ten frames */
	//c->max_b_frames = 1;
	//c->pix_fmt = AV_PIX_FMT_YUV420P;

	//this->codecDll.avcodec_open2(c, encoder, NULL);

	//auto frame = this->codecDll.avcodec_alloc_frame();
	//frame->format = c->pix_fmt;
	//frame->width = c->width;
	//frame->height = c->height;

	//auto frameBuffer = this->imgUtilsDll.av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);

	//// Encode some video
	//AVPacket pkt;
	//int got_output;
	//int i;
	//for (i = 0; i < 15; ++i)
	//{
	//	this->codecDll.av_init_packet(&pkt);
	//	pkt.data = NULL; // Let the encoder allocare the packet data
	//	pkt.size = 0;

	//	/* prepare a dummy image */
	//	/* Y */
	//	for (auto y = 0; y < c->height; y++) {
	//		for (auto x = 0; x < c->width; x++) {
	//			frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
	//		}
	//	}
	//	/* Cb and Cr */
	//	for (auto y = 0; y < c->height / 2; y++) {
	//		for (auto x = 0; x < c->width / 2; x++) {
	//			frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
	//			frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
	//		}
	//	}
	//	frame->pts = i;
	//	/* encode the image */
	//	auto ret = this->codecDll.avcodec_encode_video2(c, &pkt, frame, &got_output);
	//	if (ret < 0) {
	//		fprintf(stderr, "Error encoding frame\n");
	//		exit(1);
	//	}
	//	if (got_output) {
	//		printf("Write frame %3d (size=%5d)\n", i, pkt.size);
	//		//fwrite(pkt.data, 1, pkt.size, f);
	//		this->codecDll.av_free_packet(&pkt);
	//	}
	//}

	///* get the delayed frames */
	//for (got_output = 1; got_output; i++) {
	//	fflush(stdout);
	//	auto ret = this->codecDll.avcodec_encode_video2(c, &pkt, NULL, &got_output);
	//	if (ret < 0) {
	//		fprintf(stderr, "Error encoding frame\n");
	//		exit(1);
	//	}
	//	if (got_output) {
	//		printf("Write frame %3d (size=%5d)\n", i, pkt.size);
	//		//fwrite(pkt.data, 1, pkt.size, f);
	//		this->codecDll.av_free_packet(&pkt);
	//	}
	//}

	///* add sequence end code to have a real mpeg file */
	////fwrite(endcode, 1, sizeof(endcode), f);
	////fclose(f);

	//this->codecDll.avcodec_free_frame(&frame);
	//this->codecDll.avcodec_free_context(&c);
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