#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    grabber.setDeviceID(0);
    grabber.initGrabber(640,480, true);
    streaming.setup();
    sharedImg = ofPtr<ofImage>(new ofImage());
    streaming.setImageSource(sharedImg);
    

    ofBackground(255);
    
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
    grabber.update();
    if(grabber.isFrameNew()){
        sharedImg->setFromPixels(grabber.getPixelsRef());
        streaming.newFrame();
    }
    
    streaming.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    grabber.draw(0, 0);
    streaming.drawDebug();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	if (key == 'x')
	{
		auto path = ofToDataPath("client_settings.xml");
		ShellExecuteA(0, "open", "C:\\WINDOWS\\System32\\notepad.exe", const_cast<char*>(path.c_str()), 0, SW_SHOW);
	}
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
