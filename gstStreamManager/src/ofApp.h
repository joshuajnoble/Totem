/*
 * ofApp.h
 *
 *  Created on: Jul 19, 2013
 *      Author: arturo castro
 */

#pragma once

#include "ofMain.h"
#include "StreamManager.h"
#include "ofxGui.h"
#include "..\totem.app\src\ThreeSixtyUnwrap.h"

class ofApp : public ofBaseApp{
	void InitWebcam();
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void newClient(string & args);
    
    ofVideoGrabber grabber;
    ofPtr<ofImage> sharedImg;
    StreamManager streaming;

	ofPtr<ThreeSixtyUnwrap> unwrapper;
	ofPtr<ofBaseVideoDraws> videoDraws;
	ofPtr<ofVideoPlayer> player;
};
