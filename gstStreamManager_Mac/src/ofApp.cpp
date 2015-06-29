#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_NOTICE);
    grabber.setDeviceID(0);
    grabber.initGrabber(640,480, true);
    streaming.setup(640, 480);


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
        streaming.newFrame(grabber.getPixelsRef());
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
