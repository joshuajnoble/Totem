#include "ofTotemApp.h"
#include "Utils.h"

using namespace ofxCv;
using namespace cv;

void ofTotemApp::earlyinit()
{
	this->totemDisplay.initTotemDisplay(4, 800, 1280);
}

int ofTotemApp::displayWidth() const
{
	return this->totemDisplay.windowWidth();
}

int ofTotemApp::displayHeight() const
{
	return this->totemDisplay.windowHeight();
}

//--------------------------------------------------------------
void ofTotemApp::setup()
{
	VideoCaptureAppBase::setup();

	//small1.loadImage("meg.png");
	//small2.loadImage("matt.png");

	//this->setupSteamManager();

	this->totemDisplay.allocateBuffers();
	//this->totemDisplay.setVideoSource(2, this->videoSource);
	this->isRemoteSource1Initialized = false;
	this->isInitialized = true;
}

//--------------------------------------------------------------
void ofTotemApp::exit()
{
	streamManager.exit();
}

//--------------------------------------------------------------
void ofTotemApp::update()
{
	if (!this->isInitialized)
	{
		return;
	}

	// TODO: This is a hack for the current state of the networking system, since we don't get any connect/disconnect events
	if (!this->isRemoteSource1Initialized && !this->netImpersonate.get())
	{
		for (auto iter = this->streamManager.clients.begin(); iter != this->streamManager.clients.end(); ++iter)
		{
			auto clientId = iter->first;
			auto client = iter->second;
			auto video = this->streamManager.remoteVideos[clientId];

			this->isRemoteSource1Initialized = true;
			this->remoteVideoSources.push_back(video);
		}
	}

	VideoCaptureAppBase::update();

	//mainPlaylist.update();

	this->totemDisplay.update();

	for (int i = 0; i < 4; ++i)
	{
		auto output = this->totemDisplay.getDisplay(i);
		output.begin();

		if (this->netImpersonate.get())
		{	// DEBUG
			this->netImpersonate->update();
			if (this->netImpersonate->isFrameNew())
			{
				Utils::DrawVideoCroppedToFit(*this->netImpersonate.get(), (int)output.getWidth(), (int)output.getHeight());
			}
		}
		else if (this->remoteVideoSources.size())
		{
			auto videoSource = *this->remoteVideoSources[0].get();
			Utils::DrawImageCroppedToFit(videoSource, (int)output.getWidth(), (int)output.getHeight());
		}

		output.end();
	}

	// check for waiting messages
	//while (rec.hasWaitingMessages()){
	//	// get the next message
	//	ofxOscMessage m;
	//	rec.getNextMessage(&m);

	//	// check for mouse moved message
	//	if (m.getAddress() == "position"){
	//		// both the arguments are int32's
	//		selectedScreen = (m.getArgAsInt32(0) + 180) / 90;
	//	}

	//	if (m.getAddress() == "second_remote_on")
	//	{
	//		drawSecondRemote = true;

	//		mainPlaylist.addKeyFrame(Playlist::Action::tween(300.f, &remotePosition.x, 0));
	//		mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remotePosition.y, 400));

	//		mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remoteScale.x, 500));
	//		mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remoteScale.y, 400));

	//		mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainPosition.x, -24));
	//		mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainPosition.y, 0));

	//		mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainScale.x, 528));
	//		mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainScale.y, 400));

	//		//player.play();
	//	}

	//	if (m.getAddress() == "second_remote_off")
	//	{
	//		drawSecondRemote = false;

	//		remotePosition.set(100, 670);
	//		remoteScale.set(150, 120);
	//		mainPosition.set(-150, 10);
	//		mainScale.set(960 * 0.90, 660);

	//		//player.stop();
	//		//player.setPosition(0);
	//	}
	//}
}


//--------------------------------------------------------------
void ofTotemApp::draw()
{
	if (!this->isInitialized)
	{
		return;
	}

	if (this->rawSource)
	{
		this->rawSource->draw(0, 0);
	}
	else if (this->showOutput)
	{
		this->videoSource->draw(0, 0);
	}
	else
	{
		this->totemDisplay.draw();
	}
}


//--------------------------------------------------------------
void ofTotemApp::keyPressed(int key)
{
	if (!this->isInitialized)
	{
		return;
	}
}

void ofTotemApp::onKeyframe(ofxPlaylistEventArgs& args)
{
	if (!this->isInitialized)
	{
		return;
	}
}

class ofFboAsVideo : public ofFbo, public ofBaseUpdates
{
public:
	ofFboAsVideo(ofFbo input)
	{
		this->fbo = input;
	}

private:
	ofFbo fbo;
};

void ofTotemApp::Handle_ClientConnected(RemoteVideoInfo& remote)
{
	this->totemDisplay.drawTestPattern = false;

	ofLog() << "Network client connected " << remote.clientId << endl;

	// Show the client video
	this->remoteVideoSources.push_back(remote.source);
}

void ofTotemApp::Handle_ClientDisconnected(RemoteVideoInfo& remote)
{
	ofLog() << "Network client disconnected " << remote.clientId << endl;

	this->remoteVideoSources.clear(); // Limit it to only one source for now.
	this->totemDisplay.drawTestPattern = true;
}

void ofTotemApp::ImporsonateRemoteClient(ofPtr<ofBaseVideoDraws> source)
{
	this->totemDisplay.drawTestPattern = false;
	this->netImpersonate = source;
}