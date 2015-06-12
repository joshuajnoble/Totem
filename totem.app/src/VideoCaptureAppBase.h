#pragma once

#include "ofMain.h"
#include "RemoteVideoInfo.h"
#include "..\..\SharedCode\StreamManager.h"

class VideoCaptureAppBase : public ofBaseApp
{
public:
	virtual ~VideoCaptureAppBase() {};

	virtual int displayWidth() const = 0;
	virtual int displayHeight() const = 0;
	ofPtr<ofBaseVideoDraws> videoSource;

	virtual void update();
	virtual void exit();

private:
	void newClient(string& args);
	void clientDisconnected(string& clientId);
	void clientStreamAvailable(string& clientId);

protected:
	StreamManager streamManager;
	ofPtr<ofImage> imageToBroadcast;
	std::vector<RemoteVideoInfo> remoteVideoSources;

	std::vector<RemoteVideoInfo>::iterator GetRemoteFromClientId(const string& clientId);

	virtual void Handle_ClientConnected(RemoteVideoInfo& remote) = 0;
	virtual void Handle_ClientDisconnected(RemoteVideoInfo& remote) = 0;
	virtual void Handle_ClientStreamAvailable(RemoteVideoInfo& remote) {};

	void setupSteamManager();
};