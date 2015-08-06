#include "ofxFFmpegVideoReceiver.h"

ofxFFmpegVideoReceiver::ofxFFmpegVideoReceiver(const std::string &id) :
	clientId(id),
	m_isFrameNew(false),
	hasEverReceivedAFrame(false),
	m_hasFrameChanged(false)
{
}

ofxFFmpegVideoReceiver::~ofxFFmpegVideoReceiver()
{
}

void ofxFFmpegVideoReceiver::start(const std::string& networkAddress, uint16_t port)
{
	this->receiver.reset(new DecodeH264LiveToRGB());
	auto callback = std::bind(&ofxFFmpegVideoReceiver::ProcessRgbFrame, this, std::placeholders::_1, std::placeholders::_2);
	this->receiver->Start(networkAddress, port, callback);
}

bool ofxFFmpegVideoReceiver::isFrameNew()
{
	return this->m_isFrameNew;
}

void ofxFFmpegVideoReceiver::ProcessRgbFrame(const uint8_t*buffer, int bufferSize)
{
	ofScopedLock lock(this->mutex);
	this->pixels.setFromPixels(buffer, this->receiver->width(), this->receiver->height(), 3);
	InterlockedExchange(&this->m_hasFrameChanged, 1);
	hasEverReceivedAFrame = true;
}

void ofxFFmpegVideoReceiver::update()
{
	if (InterlockedCompareExchange(&this->m_hasFrameChanged, 0, 1))
	{
		ofScopedLock lock(this->mutex);
		this->mainThreadImage.setFromPixels(this->pixels);
		this->m_isFrameNew = true;
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