#pragma once

#include "ofMain.h"
#include "RemoteVideoInfo.h"
#include "UdpDiscovery.h"
#include "..\..\SharedCode\StreamManager.h"
#include "..\..\sharedCode\VideoConverters.h"
#include "..\..\sharedCode\FFmpegNetworkServer.h"


class VideoCaptureAppBase : public ofBaseApp
{
public:
	virtual ~VideoCaptureAppBase() {};

	virtual int displayWidth() const = 0;
	virtual int displayHeight() const = 0;
	ofPtr<ofBaseVideoDraws> videoSource;

	virtual void setup(int networkInterfaceId, bool isTotem = false);
	virtual void update();
	virtual void exit();

private:
	void newClient(string& args);
	void clientDisconnected(string& clientId);
	void clientStreamAvailable(string& clientId);

	void PeerArrived(UdpDiscovery::RemotePeerStatus& peer);
	void PeerLeft(UdpDiscovery::RemotePeerStatus& peer);
	void peerReady(UdpDiscovery::RemotePeerStatus& peer);
	
protected:
	std::vector<RemoteVideoInfo> remoteVideoSources;
	StreamManager streamManager;
	UdpDiscovery udpDiscovery;

	FFmpegFactory m_ffmpeg;
	std::auto_ptr<EncodeRGBToH264File> ffmpegEncoder;
	std::auto_ptr<FFmpegNetworkServer> ffmpegNetworkServer;

	std::vector<RemoteVideoInfo>::iterator GetRemoteFromClientId(const string& clientId);

	virtual void Handle_ClientConnected(RemoteVideoInfo& remote) = 0;
	virtual void Handle_ClientDisconnected(RemoteVideoInfo& remote) = 0;
	virtual void Handle_ClientStreamAvailable(RemoteVideoInfo& remote) {};

	void setupStreamManager();

	virtual void audioOut(float * output, int bufferSize, int nChannels);
	virtual void audioIn(float * input, int bufferSize, int nChannels);
};