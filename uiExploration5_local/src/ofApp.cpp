#include "ofApp.h"

const int remoteVideoWidth = 480;
int rotation = -90;

int selectedScreen = -1;

//--------------------------------------------------------------
void ofApp::setup(){
    
	rec.setup(8888);

    warpedW = 1920;
    warpedH = 1080;

    /// set up the cylinder
    cylinder.set(unwarpedW, unwarpedH * 8, 120, 60, 0, false);
    cylinder.mapTexCoords(0, 0, unwarpedW, unwarpedH);

    //grabber.listDevices();
	vector<ofVideoDevice> devices = grabber.listDevices();
	for (int i = 0; i < devices.size(); i++){
		if (devices.at(i).deviceName.find("Logitech") != string::npos){
			grabber.setDeviceID(i);
			grabber.initGrabber(640, 480, true);
		}
	}
    
	mainImg.loadImage("call1.jpg");
	small1.loadImage("call2.jpg");
	small2.loadImage("call3.jpg");

	if (!grabber.isInitialized()) {
		ofExit();
	}

	fbo.allocate(800, 480, GL_RGB);
	
}

//--------------------------------------------------------------
void ofApp::exit(){
	grabber.close();
}


//--------------------------------------------------------------
void ofApp::update(){
    
    //ofSetVerticalSync(false);  
    grabber.update();
    
    
    if (grabber.isFrameNew()){
        //
		fbo.begin();
		ofBackground(0, 0, 0);
		ofPushMatrix();
		ofTranslate(0, 480);
		ofRotate(rotation);
		grabber.getTextureReference().drawSubsection(0, 0, 480, 480, 0, 0);

		// draw some images for other remote users
		//mainImg.draw(0, 480);
		small1.draw(40, 480);
		small2.draw(280, 480);

		ofPopMatrix();
		// draw other UI here
		fbo.end();
    }
    

	// check for waiting messages
	while (rec.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		rec.getNextMessage(&m);

		// check for mouse moved message
		if (m.getAddress() == "/position"){
			// both the arguments are int32's
			selectedScreen = (m.getArgAsInt32(0) + 180) / 90;
		}
	}

}



//--------------------------------------------------------------
void ofApp::draw(){
	// draw everything.
	ofBackground(64,64,64);	

	//grabber.draw(0, 0);

	//fbo.draw(0, 0);

	ofPushMatrix();
	//ofRotate(90);
	fbo.draw(0, 0);
	if (selectedScreen != 0 && selectedScreen != -1) {
		ofSetColor(0, 0, 0, 20);
		ofRect(0, 0, 480, 480);
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 1 && selectedScreen != -1) {
		ofSetColor(0, 0, 0, 20);
		ofRect(0, 0, 480, 480);
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 2 && selectedScreen != -1) {
		ofSetColor(0, 0, 0, 20);
		ofRect(0, 0, 480, 480);
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 3 && selectedScreen != -1) {
		ofSetColor(0, 0, 0, 20);
		ofRect(0, 0, 480, 480);
		ofSetColor(255, 255, 255, 255);
	}
	ofPopMatrix(); 
}

//--------------------------------------------------------------
void ofApp::drawUnwarpedVideo(){
	// draw the unwarped (corrected) video in a strip at the bottom.
	ofSetColor(255, 255, 255);
    unwarpedImage.draw(0, ofGetHeight() - unwarpedH);
}

//--------------------------------------------------------------
void ofApp::drawPlayer(){
	
	// do this in an FBO
	//ofSetColor(255, 255, 255);
	//playerScaleFactor = (float)(ofGetHeight() - unwarpedH)/(float)warpedH;

}

////--------------------------------------------------------------
//void ofApp::drawTexturedCylinder(){
//	// draw the texture-mapped cylinder.
//
//    float A = 0.90;
//    float B = 1.0-A;
//    blurredMouseX = A*blurredMouseX + B*mouseX;
//    
//    ofEnableDepthTest();
//    ofPushMatrix();
//        ofTranslate(ofGetWidth()/2, (ofGetHeight()/2), 100);
//        ofRotateY(RAD_TO_DEG * ofMap(blurredMouseX, 0, ofGetWidth(),  TWO_PI, -TWO_PI));
//        grabber.getTextureReference().bind();
//        cylinder.draw();
//		grabber.getTextureReference().unbind();
//    ofPopMatrix();
//    ofDisableDepthTest();
//    
////    ofEnableAlphaBlending();
////    ofSetColor(255, 255, 255, 50);
////    ofRect(100, 0, ofGetWidth() - 200, ofGetHeight());
////    ofDisableAlphaBlending();
//    
//    ofSetColor(0, 0, 0);
//    din.drawString("Call with Teague", 150, 100);
//}


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



//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	bMousePressed = true;
	if (bMousePressedInPlayer){
		testMouseInPlayer();
	}
	if (bMousepressedInUnwarped && !bSavingOutVideo){
		angularOffset = ofMap(mouseX, 0, ofGetWidth(), 0-180, 180, false);
		bAngularOffsetChanged = true;
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	bMousePressed         = true;
	bMousePressedInPlayer = testMouseInPlayer();
	
	bMousepressedInUnwarped = false;
	if (mouseY > (ofGetHeight() - unwarpedH)){
		bMousepressedInUnwarped = true;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	if (bMousePressedInPlayer){
		testMouseInPlayer();
	}
	bMousepressedInUnwarped = false;
	bMousePressedInPlayer = false;
	bMousePressed = false;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	bMousepressedInUnwarped = false;
	bMousePressedInPlayer = false;
	bMousePressed = false;
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
bool ofApp::testMouseInPlayer(){
	bool out = false;
	
	if ((mouseX < playerScaleFactor*warpedW) && 
		(mouseY < playerScaleFactor*warpedH)){
		
		if (bSavingOutVideo == false){
			float newCx = (float)mouseX * ((float)warpedH/(float)(ofGetHeight() - unwarpedH));
			float newCy = (float)mouseY * ((float)warpedH/(float)(ofGetHeight() - unwarpedH));	
			if ((newCx != warpedCx) || (newCy != warpedCy)){
				warpedCx = newCx;
				warpedCy = newCy;
			}
			bCenterChanged = true;
			out = true;
		}
	}
	return out;
}
