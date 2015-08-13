#include "YUV420_H264_Decoder.h"
#include "LegacyGuards.h"

YUV420_H264_Decoder::YUV420_H264_Decoder(DecodedFrameCallback frameCallback) :
	callback(frameCallback)
{
	gotStartingFrame = false;

	auto codecName = "h264_qsv";
	//auto codecName = "libx264";
	pCodec = m_ffmpeg.codec.avcodec_find_decoder_by_name(codecName);
	if (!pCodec) {
		throw std::runtime_error((std::string("Codec not found: ") + codecName).c_str());
	}

	pCodecCtx = m_ffmpeg.codec.avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		throw std::runtime_error("Could not allocate video codec context.");
	}
	
	pCodecParserCtx = m_ffmpeg.codec.av_parser_init(pCodec->id);
	if (!pCodecParserCtx){
		throw std::runtime_error("Could not allocate video parser context");
	}
	
	pCodecCtx->level = 10;
	m_ffmpeg.utils.av_opt_set(pCodecCtx->priv_data, "async_depth", "1", 0);

	if (m_ffmpeg.codec.avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		throw std::runtime_error("Could not open codec");
	}

#if USE_SWSCALE
	int first_time = 1;
	struct SwsContext *img_convert_ctx;
	AVFrame	*pFrameYUV;
	uint8_t *out_buffer;

#endif
	
	pFrame = m_ffmpeg.utils.av_frame_alloc();
	m_ffmpeg.codec.av_init_packet(&packet);

}

YUV420_H264_Decoder::~YUV420_H264_Decoder()
{
}

void YUV420_H264_Decoder::DecodeFrame(const AVPacket& inPacket)
{
	cur_size = inPacket.size;
	cur_ptr = inPacket.data;

	if (!gotStartingFrame)
	{
		// Really should look for 00 00 00 01 followed by (0x1F == 7)
		if (cur_size >= 11 && (cur_ptr[10] & 0x1F) != 7)
		{
			return;
		}

		gotStartingFrame = true;
	}

	while (cur_size > 0)
	{
		int len = m_ffmpeg.codec.av_parser_parse2(
			pCodecParserCtx, pCodecCtx,
			&packet.data, &packet.size,
			cur_ptr, cur_size,
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

		cur_ptr += len;
		cur_size -= len;

		if (packet.size == 0)
		{
			continue;
		}

		//Some Info from AVCodecParserContext
		//printf("[Packet]Size:%6d\t", packet.size);
		//switch (pCodecParserCtx->pict_type){
		//	case AV_PICTURE_TYPE_I: printf("Type:I\t"); break;
		//	case AV_PICTURE_TYPE_P: printf("Type:P\t"); break;
		//	case AV_PICTURE_TYPE_B: printf("Type:B\t"); break;
		//	default: printf("Type:Other\t"); break;
		//}
		//printf("Number:%4d\n", pCodecParserCtx->output_picture_number);

		ret = m_ffmpeg.codec.avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
		if (ret < 0) {
			printf("Decode Error.\n");
			return;
		}
		if (got_picture) {
#if USE_SWSCALE
			if (first_time){
				printf("\nCodec Full Name:%s\n", pCodecCtx->codec->long_name);
				printf("width:%d\nheight:%d\n\n", pCodecCtx->width, pCodecCtx->height);
				//SwsContext
				img_convert_ctx = m_ffmpeg.scale.sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
					pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

				pFrameYUV = av_frame_alloc();
				out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
				avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

				y_size = pCodecCtx->width*pCodecCtx->height;

				first_time = 0;
			}
			m_ffmpeg.scale.sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameYUV->data, pFrameYUV->linesize);

			fwrite(pFrameYUV->data[0], 1, y_size, fp_out);     //Y 
			fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_out);   //U
			fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_out);   //V
#else
			if (this->callback)
			{
				this->callback(*pFrame);
			}
#endif
		}
	}
}

void YUV420_H264_Decoder::Close()
{
	//Flush Decoder
	packet.data = NULL;
	packet.size = 0;
	while (1){
		ret = m_ffmpeg.codec.avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
		if (ret < 0) {
			printf("Decode Error.\n");
			return;
		}
		if (!got_picture)
			break;
		if (got_picture) {

#if USE_SWSCALE
			m_ffmpeg.scale.sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

			fwrite(pFrameYUV->data[0], 1, y_size, fp_out);     //Y
			fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_out);   //U
			fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_out);   //V
#else
			int i = 0;
			unsigned char* tempptr = NULL;
			tempptr = pFrame->data[0];
			for (i = 0; i<pFrame->height; i++){
				fwrite(tempptr, 1, pFrame->width, fp_out);     //Y 
				tempptr += pFrame->linesize[0];
			}
			tempptr = pFrame->data[1];
			for (i = 0; i<pFrame->height / 2; i++){
				fwrite(tempptr, 1, pFrame->width / 2, fp_out);   //U
				tempptr += pFrame->linesize[1];
			}
			tempptr = pFrame->data[2];
			for (i = 0; i<pFrame->height / 2; i++){
				fwrite(tempptr, 1, pFrame->width / 2, fp_out);   //V
				tempptr += pFrame->linesize[2];
			}
#endif
			printf("Flush Decoder: Succeed to decode 1 frame!\n");
	}
}

	fclose(fp_in);
	fclose(fp_out);

#if USE_SWSCALE
	m_ffmpeg.scale.sws_freeContext(img_convert_ctx);
	m_ffmpeg.utils.av_frame_free(&pFrameYUV);
#endif

	m_ffmpeg.codec.av_parser_close(pCodecParserCtx);

	m_ffmpeg.utils.av_frame_free(&pFrame);
	m_ffmpeg.codec.avcodec_close(pCodecCtx);
	m_ffmpeg.utils.av_free(pCodecCtx);
}