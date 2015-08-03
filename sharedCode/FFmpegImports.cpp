#include "FFmpegImports.h"

FFmpegWrapper::avdevice FFmpegFactory::device("avdevice-56.dll");
FFmpegWrapper::avcodec FFmpegFactory::codec("avcodec-56.dll");
FFmpegWrapper::utils FFmpegFactory::utils("avutil-54.dll");
FFmpegWrapper::format FFmpegFactory::format("avformat-56.dll");