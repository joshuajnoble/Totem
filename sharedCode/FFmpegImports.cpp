#include "FFmpegImports.h"

void InitFFmpeg()
{
	av_register_all();
	avcodec_register_all();
	avdevice_register_all();
}