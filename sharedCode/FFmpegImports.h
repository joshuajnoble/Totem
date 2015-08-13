#pragma once

#include <functional>

extern "C"
{
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/time.h"
#include "libavutil/log.h"
#include "libavutil/mathematics.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libswscale/swscale.h"
}

typedef std::function<void(AVPacket&)> FrameCallback;
typedef std::function<void(const AVFrame&)> DecodedFrameCallback;