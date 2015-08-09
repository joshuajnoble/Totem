#pragma once

#include "ofMain.h"
#include "VideoConverters.h"
#include "..\totem.app\src\Utils.h"

class ofxFFmpegVideoReceiver
{
private:
	std::auto_ptr<DecodeH264LiveToRGB> receiver;
	ofPixels pixels;
	ofImage mainThreadImage;
	ofMutex mutex;
	bool m_isFrameNewVideo, m_isFrameNewAudio;
	bool hasEverReceivedAFrame;
	volatile LONG m_hasFrameChangedVideo;
	volatile LONG m_hasFrameChangedAudio;

	void ProcessRgbFrame(const uint8_t*, int);
	void ProcessPCMFrame(const uint8_t*buffer, int bufferSize);

public:
	bool isVideoFrameNew();
	bool isAudioFrameNew();
	ofxFFmpegVideoReceiver(const std::string &clientId);
	~ofxFFmpegVideoReceiver();

	void update();
	void start(const std::string& networkAddress, uint16_t port);
	void Close();
	ofImage& getVideoImage();

	RingBuffer audioBuffer;

	std::string clientId;
	bool isConnected();
};