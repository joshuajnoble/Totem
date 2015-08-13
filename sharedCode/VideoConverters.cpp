#include "VideoConverters.h"
#include "H264NetworkSender.h"
#include "H264NetworkReceiver.h"
#include "YUV420_H264_Encoder.h"
#include "YUV420_H264_Decoder.h"
#include "PCMAudioEncoder.h"
#include "AACNetworkSender.h"
#include "AACAudioEncoder.h"
#include "AACAudioDecoder.h"

//
// ConvertToNV12
//
ConvertToNV12::ConvertToNV12(int width, int height) :
	width(width),
	height(height)
{
	// Asume we are converting from RGB24
	this->sws_context = m_ffmpeg.scale.sws_getContext(
		this->width, this->height, AV_PIX_FMT_RGB24,
		this->width, this->height, AV_PIX_FMT_NV12,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (!this->sws_context) throw std::runtime_error("Could not allocate convetsion context.");

	this->srcLineSize[0] = 3 * width;
	this->dstLineSize[0] = width;
	this->dstLineSize[1] = width;
}

ConvertToNV12::~ConvertToNV12()
{
	if (this->sws_context)
	{
		m_ffmpeg.scale.sws_freeContext(this->sws_context);
		this->sws_context = NULL;
	}
}

int ConvertToNV12::GetInputFrameSize()
{
	return this->width * this->height * 3;
}

int ConvertToNV12::GetOutputFrameSize()
{
	return (this->width * this->height) + (this->width * this->height / 2);
}

void ConvertToNV12::WriteFrame(const uint8_t *rgbSource, uint8_t* yuvDestination)
{
	this->srcData[0] = const_cast<uint8_t*>(rgbSource);

	this->dstData[0] = yuvDestination;
	this->dstData[1] = yuvDestination + height * dstLineSize[0];
	m_ffmpeg.scale.sws_scale(this->sws_context, this->srcData, this->srcLineSize, 0, this->height, this->dstData, this->dstLineSize);
}

//
// ConvertToRGB
//
ConvertToRGB::ConvertToRGB(int width, int height) :
	width(width),
	height(height)
{
	// Asume we are converting from NV12
	this->sws_context = m_ffmpeg.scale.sws_getContext(
		this->width, this->height, AV_PIX_FMT_NV12,
		this->width, this->height, AV_PIX_FMT_RGB24,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (!this->sws_context) throw std::runtime_error("Could not allocate convetsion context.");

	this->dstLineSize[0] = width * 3;
}

ConvertToRGB::~ConvertToRGB()
{
	if (this->sws_context)
	{
		m_ffmpeg.scale.sws_freeContext(this->sws_context);
		this->sws_context = NULL;
	}
}

int ConvertToRGB::GetInputFrameSize()
{
	return (this->width * this->height) + (this->width * this->height / 2);
}

int ConvertToRGB::GetOutputFrameSize()
{
	return this->width * this->height * 3;
}

void ConvertToRGB::WriteFrame(const AVFrame& frameSource, uint8_t *rgbDestination)
{
	this->dstData[0] = const_cast<uint8_t*>(rgbDestination);
	m_ffmpeg.scale.sws_scale(this->sws_context, frameSource.data, frameSource.linesize, 0, this->height, this->dstData, this->dstLineSize);
}

//
// EncodeRGBToH264
//
EncodeRGBToH264::EncodeRGBToH264(FrameCallback callback) : callback(callback), closed(false)
{
}

EncodeRGBToH264::~EncodeRGBToH264()
{
	this->Close();
}

void EncodeRGBToH264::Start(int width, int height, int fps)
{
	this->converter.reset(new ConvertToNV12(width, height));
	this->yuvBuffer.resize(this->converter->GetInputFrameSize());
	this->encoder.reset(new YUV420_H264_Encoder(width, height, fps, this->callback));
}

void EncodeRGBToH264::WriteFrame(const uint8_t *srcBytes)
{
	this->converter->WriteFrame(srcBytes, this->yuvBuffer.data());
	this->encoder->WriteFrame(this->yuvBuffer.data());
}

void EncodeRGBToH264::Close()
{
	if (!this->closed)
	{
		this->encoder->Close();
		this->closed = true;
	}
}

//
// EncodeRGBToH264File
//
EncodeRGBToH264File::EncodeRGBToH264File(const std::string& filename) :
	closed(false),
	outputFile(std::ofstream(filename, std::ofstream::binary)),
	encoder(std::bind(&EncodeRGBToH264File::ProcessEncodedFrame, this, std::placeholders::_1))
{
	this->outputFile = std::ofstream(filename, std::ofstream::binary);
}

EncodeRGBToH264File::~EncodeRGBToH264File()
{
	this->Close();
}

void EncodeRGBToH264File::Start(int width, int height, int fps)
{
	this->encoder.Start(width, height, fps);
}

void EncodeRGBToH264File::WriteFrame(const uint8_t *srcBytes)
{
	this->encoder.WriteFrame(srcBytes);
}

void EncodeRGBToH264File::ProcessEncodedFrame(AVPacket &packet)
{
	this->outputFile.write(reinterpret_cast<const char *>(packet.data), packet.size);
}

void EncodeRGBToH264File::Close()
{
	if (!this->closed)
	{
		this->encoder.Close();
		this->outputFile.close();
		this->closed = true;
	}
}


//
// EncodeRGBToH264Live
//
EncodeRGBToH264Live::EncodeRGBToH264Live() :
	closed(false)	
{
	this->streamer.reset(new H264NetworkSender());
	this->audioStreamer.reset(new AACNetworkSender());

	this->encoder.reset(new EncodeRGBToH264(std::bind(&EncodeRGBToH264Live::ProcessEncodedFrame, this, std::placeholders::_1)));
	this->audioEncoder.reset(new AACAudioEncoder(1, 22050, std::bind(&EncodeRGBToH264Live::ProcessEncodedAudioFrame, this, std::placeholders::_1)));

	InitializeCriticalSection(&lock);
	this->thread = CreateThread(NULL, 0, &EncodeRGBToH264Live::VideoThreadStarter, this, 0, 0);
	this->threadExit = CreateEventA(NULL, TRUE, FALSE, NULL);
}

EncodeRGBToH264Live::~EncodeRGBToH264Live()
{
	this->Close();
}

void EncodeRGBToH264Live::Start(const std::string& ipAddress, uint16_t port, int width, int height, int fps)
{
	std::string networkAddressVideo("rtp://" + ipAddress + ":" + std::to_string(port));
	std::string networkAddressAudio("rtp://" + ipAddress + ":" + std::to_string(port + 5));

	this->streamer->Start(networkAddressVideo, width, height, fps);
	this->encoder->Start(width, height, fps);
	this->audioStreamer->Start(networkAddressAudio, 1, 22050);
}

DWORD WINAPI EncodeRGBToH264Live::VideoThreadStarter(LPVOID context)
{
	if (context)
	{
		((EncodeRGBToH264Live *)context)->VideoProcessingThread();
	}

	return 0;
}

void EncodeRGBToH264Live::VideoProcessingThread()
{
	while (WaitForSingleObject(this->threadExit, 10) == WAIT_TIMEOUT)
	{
		if (TryEnterCriticalSection(&lock))
		{
			if (!this->videoQueue.empty())
			{
				uint8_t* pixels = this->videoQueue.front();
				this->videoQueue.pop();
				LeaveCriticalSection(&lock);
				this->encoder->WriteFrame(pixels);
				delete pixels;
			}
			else
			{
				LeaveCriticalSection(&lock);
			}
		}
	}
}

void EncodeRGBToH264Live::WriteVideoFrame(const uint8_t *videoSource, int cbVideoSource)
{
	if (this->closed) return;

	EnterCriticalSection(&this->lock);
	auto newPixels = new uint8_t[cbVideoSource];
	memcpy(newPixels, videoSource, cbVideoSource);
	while (this->videoQueue.size() > 3)
	{
		auto pixels = this->videoQueue.front();
		this->videoQueue.pop();
		delete pixels;
	}
	this->videoQueue.push(newPixels);
	LeaveCriticalSection(&lock);
}

int EncodeRGBToH264Live::WriteAudioFrame(const uint8_t* audioSource, int cbAudioSource)
{
	if (this->closed) return 0;
	return this->audioEncoder->WriteFrame(audioSource, cbAudioSource);
}

void EncodeRGBToH264Live::ProcessEncodedFrame(AVPacket& packet)
{
	if (this->closed) return;
	this->streamer->WriteFrame(packet);
}

void EncodeRGBToH264Live::ProcessEncodedAudioFrame(AVPacket& packet)
{
	if (this->closed) return;
	//fwrite(packet.data, 1, packet.size, fp);
	this->audioStreamer->WriteFrame(packet);
}

void EncodeRGBToH264Live::Close()
{
	if (!this->closed)
	{
		this->closed = true;

		SetEvent(this->threadExit);
		WaitForSingleObject(this->thread, INFINITE);
		CloseHandle(this->thread);
		CloseHandle(this->threadExit);
		DeleteCriticalSection(&this->lock);

		while (this->videoQueue.size())
		{
			auto pixels = this->videoQueue.front();
			this->videoQueue.pop();
			delete pixels;
		}

		this->encoder->Close();
		this->streamer->Close();
	}
}

//
// DecodeH264LiveToRGB
//
DecodeH264LiveToRGB::DecodeH264LiveToRGB() :
	closed(false)
{
	//pcmFile = fopen("received.pcm", "wb");
	//m_ffmpeg.utils.av_log_set_level(AV_LOG_QUIET);
	m_ffmpeg.utils.av_log_set_level(AV_LOG_VERBOSE);
	//m_ffmpeg.utils.av_log_set_level(AV_LOG_DEBUG);
	this->receiver.reset(new H264NetworkReceiver());
	this->decoder.reset(new YUV420_H264_Decoder(std::bind(&DecodeH264LiveToRGB::ProcessYUVFrame, this, std::placeholders::_1)));
	auto pcmFinalCallback = std::bind(&DecodeH264LiveToRGB::ProcessDecodedAudioFrame, this, std::placeholders::_1);
	this->audioDecoder.reset(new AACAudioDecoder(1, 22050, pcmFinalCallback));
}

DecodeH264LiveToRGB::~DecodeH264LiveToRGB()
{
	this->Close();
}

void DecodeH264LiveToRGB::Start(const std::string& ipAddress, uint16_t port, RGBFrameCallback rgbFrameCallback, PCMFrameCallback pcmFrameCallback)
{
	this->callback = rgbFrameCallback;
	this->callbackAudio = pcmFrameCallback;
	this->receiver->Start(ipAddress, port,
		std::bind(&DecodeH264LiveToRGB::ProcessEncodedFrame, this, std::placeholders::_1),
		std::bind(&DecodeH264LiveToRGB::ProcessEncodedAudioFrame, this, std::placeholders::_1));
}

void DecodeH264LiveToRGB::Close()
{
	if (!this->closed)
	{
		//fclose(pcmFile);
		this->closed = true;
		this->receiver->Close();
	}
}

void DecodeH264LiveToRGB::ProcessEncodedAudioFrame(AVPacket& packet)
{
	this->audioDecoder->DecodeFrame(packet);
}

void DecodeH264LiveToRGB::ProcessDecodedAudioFrame(const AVFrame& frame)
{
	//fwrite(frame.data[0], 1, frame.nb_samples* sizeof(float), pcmFile);

	if (this->callbackAudio)
	{
		this->callbackAudio((uint8_t *)frame.data[0], frame.nb_samples* sizeof(float));
	}
}

void DecodeH264LiveToRGB::ProcessEncodedFrame(AVPacket& packet)
{
	this->decoder->DecodeFrame(packet);
}

void DecodeH264LiveToRGB::ProcessYUVFrame(const AVFrame &frame)
{
	if (!this->converter.get())
	{
		this->m_width = frame.width;
		this->m_height = frame.height;
		this->m_fps = 15; // TODO: Can we read this from somewhere?

		this->converter.reset(new ConvertToRGB(frame.width, frame.height));
		this->rgbBuffer.resize(this->converter->GetOutputFrameSize());
	}

	this->converter->WriteFrame(frame, this->rgbBuffer.data());

	if (this->callback)
	{
		this->callback(this->rgbBuffer.data(), this->rgbBuffer.size());
	}
}

bool DecodeH264LiveToRGB::isConnected()
{
	return this->receiver->isConnected();
}