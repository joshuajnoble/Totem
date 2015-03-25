#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetWindowTitle("ofxSpout sender example");

	_red = 0;
	_isIncreasing = true;

	// initialize Spout with a sender name, and a texture size
	ofxSpout::init("ofxSpout sender example", ofGetWidth(), ofGetHeight(), true);	

	vector<ofVideoDevice> devices = grabber.listDevices();
	for (int i = 0; i < devices.size(); i++){
		if (devices.at(i).deviceName.find("Logitech") != string::npos){
			grabber.setDeviceID(i);
			grabber.initGrabber(640, 480, true);
		}
	}

}

//--------------------------------------------------------------
void ofApp::update()
{
	grabber.update();

	if (grabber.isFrameNew()) {
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// init sender if it's not already initialized
	ofxSpout::initSender();
	grabber.draw(0, 0);

	// send screen to Spout
	ofxSpout::sendTexture();
}

//--------------------------------------------------------------
void ofApp::exit()
{
	// exit spout
	ofxSpout::exit();
}

