#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	blurredMouseX = 0;
    
    warpedW = 1920;
    warpedH = 1080;

    /// set up the cylinder
	cylinder.set(warpedW, warpedH * 5, 120, 60, 0, false);
	cylinder.mapTexCoords(0, 0, warpedW, warpedH);

	sender.setup("localhost", 8888);

    vector<ofVideoDevice> dd = grabber.listDevices();

	for (int i = 0; i < dd.size(); i++)
	{
		if (dd.at(i).deviceName.find("XI100DUSB") != string::npos)
		{
			grabber.setDeviceID(i);
			grabber.initGrabber(1920, 1080, true);
		}
	}


	if (!grabber.isInitialized())
	{
		ofExit();
	}
 
    //din.loadFont("fonts/FF_DIN_Pro_Light_Italic.otf", 32);
    
	
}

//--------------------------------------------------------------
void ofApp::exit()
{

	grabber.close();

}


//--------------------------------------------------------------
void ofApp::update(){
    
    ofSetVerticalSync(false);  
    grabber.update();
    
    
    if (grabber.isFrameNew()){
        
            
    }
    

}



//--------------------------------------------------------------
void ofApp::draw(){
	// draw everything.
	ofBackground(64,64,64);	
	
	//drawPlayer();
	//drawUnwarpedVideo();

    if(drawCylinder) {
        drawTexturedCylinder();
	}
	else {
		drawPlayer();
	}
}

//--------------------------------------------------------------
void ofApp::drawUnwarpedVideo(){
	// draw the unwarped (corrected) video in a strip at the bottom.
	ofSetColor(255, 255, 255);
    unwarpedImage.draw(0, ofGetHeight() - unwarpedH);
}

//--------------------------------------------------------------
void ofApp::drawPlayer(){
	
	
	// draw the (warped) player
	ofSetColor(255, 255, 255);
	playerScaleFactor = (float)(ofGetHeight() - unwarpedH)/(float)warpedH;

	//1920
    grabber.draw(0, 0, 1920/2, 1080/2);

}

//--------------------------------------------------------------
void ofApp::drawTexturedCylinder(){
	// draw the texture-mapped cylinder.

    float A = 0.90;
    float B = 1.0-A;
    blurredMouseX = A*blurredMouseX + B*mouseX;
    
    ofEnableDepthTest();
    ofPushMatrix();
        ofTranslate(ofGetWidth()/2, (ofGetHeight()/2), 100);
        ofRotateY(RAD_TO_DEG * ofMap(blurredMouseX, 0, ofGetWidth(),  TWO_PI, -TWO_PI));
        grabber.getTextureReference().bind();
        cylinder.draw();
		grabber.getTextureReference().unbind();
    ofPopMatrix();
    ofDisableDepthTest();
    
//    ofEnableAlphaBlending();
//    ofSetColor(255, 255, 255, 50);
//    ofRect(100, 0, ofGetWidth() - 200, ofGetHeight());
//    ofDisableAlphaBlending();
    
    //ofSetColor(0, 0, 0);
    //din.drawString("Call with Teague", 150, 100);
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){ 
	
	switch (key){
	case ' ':
		drawCylinder = !drawCylinder;
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

		ofxOscMessage m;
		m.setAddress("position");
		m.addIntArg(angularOffset);
		sender.sendMessage(m);

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
