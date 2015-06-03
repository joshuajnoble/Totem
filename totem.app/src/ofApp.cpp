#include "ofApp.h"
#include "Utils.h"

using namespace ofxCv;
using namespace cv;

void ofApp::earlyinit()
{
	this->totemDisplay.initTotemDisplay(4, 768, 1360);
}

int ofApp::displayWidth() const
{
	return this->totemDisplay.windowWidth();
}

int ofApp::displayHeight() const
{
	return this->totemDisplay.windowHeight();
}

//--------------------------------------------------------------
void ofApp::setup()
{
	//small1.loadImage("meg.png");
	//small2.loadImage("matt.png");

	if (this->passthroughVideo)
	{
		this->processedVideo = this->videoSource;
	}
	else
	{
		auto unwrapper = new ThreeSixtyUnwrap();
		this->processedVideo = ofPtr<ofBaseVideoDraws>(unwrapper);
		unwrapper->initUnwrapper(this->videoSource, this->videoSource->getWidth() * this->unwrapMultiplier, this->videoSource->getWidth() * this->unwrapMultiplier * this->unwrapAspectRatio);
	}

	streamManager.setup(this->processedVideo->getWidth(), this->processedVideo->getHeight());
	remoteImage = ofPtr<ofImage>(new ofImage());
	streamManager.setImageSource(remoteImage);
	ofAddListener(streamManager.newClientEvent, this, &ofApp::newClient);

	this->totemDisplay.allocateBuffers();
	//this->totemDisplay.setVideoSource(2, this->videoSource);
	this->isInitialized = true;
}

//--------------------------------------------------------------
void ofApp::exit()
{
	streamManager.exit();
}

//--------------------------------------------------------------
void ofApp::update()
{
	if (!this->isInitialized)
	{
		return;
	}

	this->processedVideo->update();

	if (this->processedVideo->isFrameNew())
	{
		remoteImage->setFromPixels(this->processedVideo->getPixelsRef());
		streamManager.newFrame();
	}

	streamManager.update();

	//mainPlaylist.update();

	this->totemDisplay.update();

	for (int i = 0; i < this->remoteVideoSources.size(); ++i)
	{
		auto output = this->totemDisplay.getDisplay(i);
		output.begin();
		//this->remoteVideoSources[i]->draw(0, 0);
		Utils::DrawImageCroppedToFit(*this->remoteVideoSources[i].get(), (int)output.getWidth(), (int)output.getHeight());
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
void ofApp::draw()
{
	if (!this->isInitialized)
	{
		return;
	}

	if (this->showInput)
	{
		this->videoSource->draw(0, 0);
	}
	else if (this->showUnwrapped)
	{
		this->processedVideo->draw(0, 0);
	}
	else
	{
		this->totemDisplay.draw();
	}
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (!this->isInitialized)
	{
		return;
	}
}

void ofApp::onKeyframe(ofxPlaylistEventArgs& args)
{
	if (!this->isInitialized)
	{
		return;
	}
}

ofPtr<ofBaseVideoDraws> ofApp::InitializeVideoPresenterFromFile(std::string path)
{
	ofVideoPlayer* player = new ofVideoPlayer();
	ofPtr<ofBaseVideoDraws> rval = ofPtr<ofBaseVideoDraws>(player);
	if (player->loadMovie(path))
	{
		player->setLoopState(OF_LOOP_NORMAL);
		player->play();
	}

	return rval;
}

ofPtr<ofBaseVideoDraws> ofApp::InitializePlayerFromCamera(int deviceId, int width, int height)
{
	ofVideoGrabber *grabber = new ofVideoGrabber();
	ofPtr<ofVideoGrabber> rval = ofPtr<ofVideoGrabber>(grabber);
	if (deviceId != 0)
	{
		grabber->setDeviceID(deviceId);
	}

	grabber->initGrabber(width, height);
	return rval;
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

void ofApp::newClient(string& args)
{
	ofLog() << "new client" << endl;

	// Show the client video
	auto source = this->streamManager.remoteVideos.begin()->second;
	this->remoteVideoSources.clear(); // Limit it to only one source for now.
	this->remoteVideoSources.push_back(source);
}