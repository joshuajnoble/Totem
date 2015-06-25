#include "ofTotemApp.h"
#include "Utils.h"

using namespace ofxCv;
using namespace cv;

namespace
{
	const ofColor BACKGROUND_COLOR = ofColor(0x08, 0x21, 0x35);
}

void ofTotemApp::earlyinit(int netid)
{
	this->networkInterfaceId = netid;
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
	VideoCaptureAppBase::setup(this->networkInterfaceId);

	//small1.loadImage("meg.png");
	//small2.loadImage("matt.png");

	this->totemDisplay.allocateBuffers();
	this->isRemoteSource1Initialized = false;
	this->isInitialized = true;

	ofSetWindowShape(this->totemDisplay.windowWidth(), this->totemDisplay.windowHeight());

	this->streamManager.broadcastVideoBitrate = 8000;
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
			auto margin = 10;
			auto halfMargin = margin / 2;
			auto halfHeight = (int)output.getHeight() / 2;
			auto halfWidth = (int)output.getWidth() / 2;

			auto remoteSourceCount = this->remoteVideoSources.size();
			ofBackground(BACKGROUND_COLOR);
			if (remoteSourceCount == 1)
			{
				auto videoSource = *this->remoteVideoSources[0].get();
				Utils::DrawImageCroppedToFit(videoSource, (int)output.getWidth(), (int)output.getHeight());
			}
			else if (remoteSourceCount == 2)
			{
				auto videoSource = *this->remoteVideoSources[0].get();
				Utils::DrawImageCroppedToFit(videoSource, (int)output.getWidth(), halfHeight - halfMargin);
				videoSource = *this->remoteVideoSources[1].get();
				Utils::DrawImageCroppedToFit(videoSource, 0, halfHeight + halfMargin, (int)output.getWidth(), halfHeight - halfMargin);
			}
			else if (remoteSourceCount == 3)
			{
				auto videoSource = *this->remoteVideoSources[0].get();
				Utils::DrawImageCroppedToFit(videoSource, (int)output.getWidth(), halfHeight - halfMargin);
				videoSource = *this->remoteVideoSources[1].get();
				Utils::DrawImageCroppedToFit(videoSource, 0, halfHeight + halfMargin, halfWidth - halfMargin, halfHeight - halfMargin);
				videoSource = *this->remoteVideoSources[2].get();
				Utils::DrawImageCroppedToFit(videoSource, halfWidth + halfMargin, halfHeight + halfMargin, halfWidth - halfMargin, halfHeight - halfMargin);
			}
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
}

void ofTotemApp::ImporsonateRemoteClient(ofPtr<ofBaseVideoDraws> source)
{
	this->totemDisplay.drawTestPattern = false;
	this->netImpersonate = source;
}