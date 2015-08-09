#include "ofxFFmpegVideoReceiver.h"

ofxFFmpegVideoReceiver::ofxFFmpegVideoReceiver(const std::string &id) :
	clientId(id),
	m_isFrameNewVideo(false),
	hasEverReceivedAFrame(false),
	m_hasFrameChangedVideo(false),
	m_isFrameNewAudio(false),
	m_hasFrameChangedAudio(false),
	audioBuffer(512 * sizeof(float) * 15)
{
}

ofxFFmpegVideoReceiver::~ofxFFmpegVideoReceiver()
{
}

void ofxFFmpegVideoReceiver::start(const std::string& networkAddress, uint16_t port)
{
	this->receiver.reset(new DecodeH264LiveToRGB());
	auto videoCallback = std::bind(&ofxFFmpegVideoReceiver::ProcessRgbFrame, this, std::placeholders::_1, std::placeholders::_2);
	auto audioCallback = std::bind(&ofxFFmpegVideoReceiver::ProcessPCMFrame, this, std::placeholders::_1, std::placeholders::_2);
	this->receiver->Start(networkAddress, port, videoCallback, audioCallback);
}

bool ofxFFmpegVideoReceiver::isVideoFrameNew()
{
	return this->m_isFrameNewVideo;
}

bool ofxFFmpegVideoReceiver::isAudioFrameNew()
{
	return this->m_isFrameNewAudio;
}

void ofxFFmpegVideoReceiver::ProcessRgbFrame(const uint8_t*buffer, int bufferSize)
{
	ofScopedLock lock(this->mutex);
	this->pixels.setFromPixels(buffer, this->receiver->width(), this->receiver->height(), 3);
	InterlockedExchange(&this->m_hasFrameChangedVideo, 1);
	hasEverReceivedAFrame = true;
}

void ofxFFmpegVideoReceiver::ProcessPCMFrame(const uint8_t*buffer, int bufferSize)
{
	this->audioBuffer.Write(buffer, bufferSize);
	InterlockedExchange(&this->m_hasFrameChangedAudio, 1);
	hasEverReceivedAFrame = true;
}

void ofxFFmpegVideoReceiver::update()
{
	this->m_isFrameNewVideo = false;
	if (InterlockedCompareExchange(&this->m_hasFrameChangedVideo, 0, 1))
	{
		ofScopedLock lock(this->mutex);
		this->mainThreadImage.setFromPixels(this->pixels);
		this->m_isFrameNewVideo = true;
	}

	this->m_isFrameNewAudio = false;
	if (InterlockedCompareExchange(&this->m_hasFrameChangedAudio, 0, 1))
	{
		this->m_isFrameNewAudio = true;
	}
}

ofImage& ofxFFmpegVideoReceiver::getVideoImage()
{
	return this->mainThreadImage;
}

void ofxFFmpegVideoReceiver::Close()
{
	this->receiver->Close();
}

bool ofxFFmpegVideoReceiver::isConnected()
{
	return this->hasEverReceivedAFrame;
	//return this->receiver->isConnected();
}