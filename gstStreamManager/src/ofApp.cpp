#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(255);
	//InitWebcam();
    ofAddListener(streaming.newClientEvent, this, &ofApp::newClient);
}


void ofApp::exit(){
	streaming.exit();   
}

void ofApp::newClient(string &args){
    ofLog()<<"new client"<<endl;
}


//--------------------------------------------------------------
void ofApp::update(){
	if (grabber.isInitialized())
	{
		grabber.update();
		if (grabber.isFrameNew()){
			streaming.newFrame(grabber.getPixelsRef());
		}

		streaming.update();
	}
	else if (unwrapper)
	{
		unwrapper->update();
		if (unwrapper->isFrameNew()){
			streaming.newFrame(unwrapper->getPixelsRef());
		}
		streaming.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (grabber.isInitialized())
	{
		auto scale = 0.25f;
		ofPushMatrix();
		ofScale(scale, scale);
		grabber.draw(0, 0);

		ofTranslate(0, grabber.getHeight() + 10);
		streaming.drawDebug();
		ofPopMatrix();
	}
	else if (unwrapper)
	{
		auto scale = 0.25f;
		ofPushMatrix();
		ofScale(scale, scale);
		unwrapper->draw(0, 0);
		ofTranslate(0, unwrapper->getHeight() + 10);
		streaming.drawDebug();
		ofPopMatrix();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == '1' && !grabber.isInitialized())
	{
		if (player)
		{
			player->close();
			unwrapper->close();
		}

		InitWebcam();
	}
	else if (key == '2')
	{
		if (grabber.isInitialized())
		{
			grabber.close();
		}

		player = ofPtr<ofVideoPlayer>(new ofVideoPlayer());
		videoDraws = ofPtr<ofBaseVideoDraws>(player);
		auto fullPath = ofToDataPath("totem.mp4");
		if (player->loadMovie(fullPath))
		{
			player->setLoopState(OF_LOOP_NORMAL);
			player->play();
		}

		unwrapper = ofPtr<ThreeSixtyUnwrap>(new ThreeSixtyUnwrap());
		const ofVec2f UNWRAPPED_DISPLAYRATIO(4.85, 1.0);
		auto outputSize = ThreeSixtyUnwrap::CalculateUnwrappedSize(ofVec2f(videoDraws->getWidth(), videoDraws->getHeight()), UNWRAPPED_DISPLAYRATIO);
		unwrapper->initUnwrapper(videoDraws, outputSize);

		sharedImg = ofPtr<ofImage>(new ofImage());

		streaming.setup(outputSize.x, outputSize.y);
		streaming.broadcastVideoBitrate = 8000;
	}
	else if (key == 'x')
	{
		auto path = ofToDataPath("client_settings.xml");
		ShellExecuteA(0, "open", "C:\\WINDOWS\\System32\\notepad.exe", const_cast<char*>(path.c_str()), 0, SW_SHOW);
	}
}

void ofApp::InitWebcam()
{
	grabber.setDeviceID(0);
	grabber.initGrabber(1280, 720, true);
	sharedImg = ofPtr<ofImage>(new ofImage());
	streaming.setup(1280, 720);
	streaming.broadcastVideoBitrate = 4000;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
