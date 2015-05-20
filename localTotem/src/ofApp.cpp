#include "ofApp.h"

#define JOSHNOBLE_UNWRAP

using namespace ofxCv;
using namespace cv;

namespace
{
	const int remoteVideoWidth = 480;
	int rotation = -90;
	int selectedScreen = -1;
}

//--------------------------------------------------------------
void ofApp::setup()
{
	rec.setup(8888);	

	// initialize Spout as a receiver
	//small1.loadImage("meg.png");
	//small2.loadImage("matt.png");

	fbo.allocate(800, 480, GL_RGB);
	drawSecondRemote = false;
	remotePosition.set(100, 670);
	remoteScale.set(150, 120);
	mainPosition.set(-100, 10);
	mainScale.set(660, 660);

	if (this->passthroughVideo)
	{
		this->processedVideo = this->videoSource;
	}
	else
	{
		double factor = 1.25;
		//this->unwrapper.initUnwrapper(this->videoSource, this->videoSource->getWidth() * factor, this->videoSource->getWidth() * factor / 5);
		auto unwrapper = new ThreeSixtyUnwrap();
		this->processedVideo = ofPtr<ofBaseVideoDraws>(unwrapper);
		unwrapper->initUnwrapper(this->videoSource, this->videoSource->getWidth() * factor, this->videoSource->getWidth() * factor / 5);
	}

	streamManager.setup(640, 480);
	remoteImage = ofPtr<ofImage>(new ofImage());
	streamManager.setImageSource(remoteImage);
	ofAddListener(streamManager.newClientEvent, this, &ofApp::newClient);

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

	mainPlaylist.update();

	if (drawSecondRemote)
	{
		player->update();
	}

	//ofxSpout::initReceiver();
	//ofxSpout::receiveTexture();
	
	fbo.begin();
	ofBackground(0, 0, 0);
	ofPushMatrix();
	ofTranslate(0, 480);
	ofRotate(rotation);

	if (drawSecondRemote)
	{

		//ofxSpout::drawSubsection(0, 0, 400 * 1.33, 400, 100, 0, 960, 720);
		//player.getTextureReference().drawSubsection(0, 400, 500, 400, 0, 400, player.getWidth(), 800);

		//ofxSpout::drawSubsection(mainPosition.x, mainPosition.y, mainScale.x, mainScale.y, 0, 0, 960, 720);
		//player->getTextureReference().drawSubsection(remotePosition.x, remotePosition.y, remoteScale.x, remoteScale.y, 0, 400, player->getWidth(), 800);

		ofSetColor(0, 0, 0);
		ofRect(0, remotePosition.y, 480, 10);
		ofSetColor(255, 255, 255);
	}
	else
	{
		//ofxSpout::drawSubsection(-150, 10, 960 * 0.90, 660, 0, 0, 960, 720);
		//ofxSpout::draw(0, 0, 500, 500);
		//small1.draw(100, 670, 150, 120);
		//small2.draw(260, 670, 150, 120);
		//small3.draw(325, 540, 150, 120);
	}

	ofPopMatrix();

	// draw other UI here

	fbo.end();

	// check for waiting messages
	while (rec.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		rec.getNextMessage(&m);

		// check for mouse moved message
		if (m.getAddress() == "position"){
			// both the arguments are int32's
			selectedScreen = (m.getArgAsInt32(0) + 180) / 90;
		}

		if (m.getAddress() == "second_remote_on")
		{
			drawSecondRemote = true;

			mainPlaylist.addKeyFrame(Playlist::Action::tween(300.f, &remotePosition.x, 0));
			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remotePosition.y, 400));

			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remoteScale.x, 500));
			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remoteScale.y, 400));

			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainPosition.x, -24));
			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainPosition.y, 0));

			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainScale.x, 528));
			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainScale.y, 400));

			//player.play();
		}

		if (m.getAddress() == "second_remote_off")
		{
			drawSecondRemote = false;

			remotePosition.set(100, 670);
			remoteScale.set(150, 120);
			mainPosition.set(-150, 10);
			mainScale.set(960 * 0.90, 660);

			//player.stop();
			//player.setPosition(0);
		}
	}
}


//--------------------------------------------------------------
void ofApp::draw()
{
	if (!this->isInitialized)
	{
		return;
	}

	// draw everything.
	ofBackground(64, 64, 64);

	//grabber.draw(0, 0);

	//fbo.draw(0, 0);

	ofPushMatrix();
	//ofRotate(90);
	fbo.draw(0, 0);
	if (selectedScreen != 0 && selectedScreen != -1) {
		//ofRect(0, 0, 480, 480);
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 1 && selectedScreen != -1) {
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 2 && selectedScreen != -1) {
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 3 && selectedScreen != -1) {
		ofSetColor(255, 255, 255, 255);
	}
	ofPopMatrix();

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
		this->streamManager.drawDebug();
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

void ofApp::newClient(string& args)
{
	ofLog() << "new client" << endl;
}