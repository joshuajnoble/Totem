#include "VideoCaptureAppBase.h"

void VideoCaptureAppBase::setup()
{
	this->udpDiscovery.setup();
	this->setupSteamManager();
}

void VideoCaptureAppBase::update()
{
	this->udpDiscovery.update();

	this->videoSource->update();
	if (this->videoSource->isFrameNew())
	{
		this->imageToBroadcast->setFromPixels(this->videoSource->getPixelsRef());
		this->streamManager.newFrame();
	}

	this->streamManager.update();
}

void VideoCaptureAppBase::exit()
{
	this->streamManager.exit();
}

void VideoCaptureAppBase::newClient(string& args)
{
	RemoteVideoInfo remote;
	remote.clientId = args;
	remote.source = this->streamManager.remoteVideos[args];
	remote.netClient = this->streamManager.clients[args];
	remote.isTotem = remote.source->getWidth() / remote.source->getHeight() >= 3;
	this->remoteVideoSources.push_back(remote);
	Handle_ClientConnected(remote);
}

void VideoCaptureAppBase::clientDisconnected(string& clientId)
{
	auto found = GetRemoteFromClientId(clientId);
	if (found != this->remoteVideoSources.end())
	{
		auto remote = *found;
		this->remoteVideoSources.erase(found);
		Handle_ClientDisconnected(remote);
	}
}

void VideoCaptureAppBase::clientStreamAvailable(string& clientId)
{
	auto found = GetRemoteFromClientId(clientId);
	if (found != this->remoteVideoSources.end())
	{
		Handle_ClientStreamAvailable(*found);
	}
}

void VideoCaptureAppBase::setupSteamManager()
{
	streamManager.setup(this->videoSource->getWidth(), this->videoSource->getHeight());
	this->imageToBroadcast = ofPtr<ofImage>(new ofImage());
	streamManager.setImageSource(this->imageToBroadcast);
	ofAddListener(streamManager.newClientEvent, this, &VideoCaptureAppBase::newClient);
	ofAddListener(streamManager.clientDisconnectedEvent, this, &VideoCaptureAppBase::clientDisconnected);
	ofAddListener(streamManager.clientStreamAvailableEvent, this, &VideoCaptureAppBase::clientStreamAvailable);
}

std::vector<RemoteVideoInfo>::iterator VideoCaptureAppBase::GetRemoteFromClientId(const string& clientId)
{
	for (auto iter = this->remoteVideoSources.begin(); iter != this->remoteVideoSources.end(); ++iter)
	{
		auto remote = *iter;
		if (remote.clientId == clientId)
		{
			return iter;
		}
	}

	return  this->remoteVideoSources.end();
}