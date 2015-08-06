#pragma once

#include "ofMain.h"
#include "VideoConverters.h"

class ofxFFmpegVideoReceiver
{
private:
	std::auto_ptr<DecodeH264LiveToRGB> receiver;
	ofPixels pixels;
	ofImage mainThreadImage;
	ofMutex mutex;
	bool m_isFrameNew;
	bool hasEverReceivedAFrame;
	volatile LONG m_hasFrameChanged;

	void ProcessRgbFrame(const uint8_t*, int);

public:
	bool isFrameNew();
	ofxFFmpegVideoReceiver(const std::string &clientId);
	~ofxFFmpegVideoReceiver();

	void update();
	void start(const std::string& networkAddress, uint16_t port);
	void Close();
	ofImage& getVideoImage();

	std::string clientId;
	bool isConnected();
};