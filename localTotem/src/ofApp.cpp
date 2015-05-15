#include "ofApp.h"

const int remoteVideoWidth = 480;
int rotation = -90;

int selectedScreen = -1;

//--------------------------------------------------------------
void ofApp::setup(){
    
	rec.setup(8888);

	player.loadMovie("IMG_1628.mov");

	// initialize Spout as a receiver
	ofxSpout::init("", 640, 480, false);
	small1.loadImage("meg.png");
	small2.loadImage("matt.png");

	fbo.allocate(800, 480, GL_RGB);

	drawSecondRemote = false;

	remotePosition.set(100, 670);
	remoteScale.set(150, 120);
	mainPosition.set(-100, 10);
	mainScale.set(660, 660);
}

//--------------------------------------------------------------
void ofApp::exit(){
	ofxSpout::exit();
}


//--------------------------------------------------------------
void ofApp::update(){

	mainPlaylist.update();

	if (drawSecondRemote)
	{
		player.update();
	}

	ofxSpout::initReceiver();
	ofxSpout::receiveTexture();

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
		player.getTextureReference().drawSubsection(remotePosition.x, remotePosition.y, remoteScale.x, remoteScale.y, 0, 400, player.getWidth(), 800);

		ofSetColor(0, 0, 0);
		ofRect(0, remotePosition.y, 480, 10);
		ofSetColor(255, 255, 255);
	}
	else
	{
		//ofxSpout::drawSubsection(-150, 10, 960 * 0.90, 660, 0, 0, 960, 720);
		//ofxSpout::draw(0, 0, 500, 500);
		small1.draw(100, 670, 150, 120);
		small2.draw(260, 670, 150, 120);
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

			player.play();
		}

		if (m.getAddress() == "second_remote_off")
		{
			drawSecondRemote = false;

			remotePosition.set(100, 670);
			remoteScale.set(150, 120);
			mainPosition.set(-150, 10);
			mainScale.set(960 * 0.90, 660);

			player.stop();
			player.setPosition(0);

		}
	}

}



//--------------------------------------------------------------
void ofApp::draw(){
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
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){ 
	
	/*
	<!-- // Press Space to toggle movie play.                      --> 
	<!-- // Press 's' to save the geometry settings.               -->
	<!-- // Press 'r' to reload the previously saved settings.     -->
	<!-- // Use the +/- keys to change the export codec.           -->
	<!-- // Press 'v' to export the unwarped video.                -->
	<!-- // Use the arrow keys to nudge the center point.          -->
	<!-- // Drag the unwarped video left or right to shift it.     -->
	 */

	
//	int nCodecs = videoRecorder->getNCodecs();
	
	switch (key){
	case '+':
		rotation += 1;
		break;
	case '-':
		rotation -= 1;
		break;
	}
	
}


void ofApp::onKeyframe(ofxPlaylistEventArgs& args)
{

}