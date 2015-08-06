#pragma once

#include "ofMain.h"
#include "RemoteVideoInfo.h"
#include "UdpDiscovery.h"
#include "..\..\SharedCode\FFmpegImports.h"

class EncodeRGBToH264Live;
class ofxFFmpegVideoReceiver;

class VideoCaptureAppBase : public ofBaseApp
{
public:
	virtual ~VideoCaptureAppBase() {};

	virtual int displayWidth() const = 0;
	virtual int displayHeight() const = 0;
	ofPtr<ofBaseVideoDraws> videoSource;
	ofMutex videoLock;
	volatile bool hasNewVideo = false;

	virtual void setup(int networkInterfaceId, bool isTotem = false);
	virtual void update();
	virtual void exit();

private:
	//void newClient(string& args);
	//void clientDisconnected(string& clientId);
	//void clientStreamAvailable(string& clientId);

	void PeerArrived(UdpDiscovery::RemotePeerStatus& peer);
	void PeerLeft(UdpDiscovery::RemotePeerStatus& peer);
	void PeerReady(UdpDiscovery::RemotePeerStatus& peer);
	
protected:
	UdpDiscovery udpDiscovery;
	std::vector<ofxFFmpegVideoReceiver *> remoteVideoSources;
	std::auto_ptr<EncodeRGBToH264Live> ffmpegVideoBroadcast;

	std::vector<RemoteVideoInfo> peers;
	std::vector<RemoteVideoInfo>::iterator GetRemoteFromClientId(const string& clientId);

	virtual void Handle_ClientConnected(RemoteVideoInfo& remote) = 0;
	virtual void Handle_ClientDisconnected(RemoteVideoInfo& remote) = 0;
	virtual void Handle_ClientStreamAvailable(RemoteVideoInfo& remote) {};

	void setupStreamManager();

	virtual void audioOut(float * output, int bufferSize, int nChannels);
	virtual void audioIn(float * input, int bufferSize, int nChannels);
};