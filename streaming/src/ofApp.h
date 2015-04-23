/*
 * ofApp.h
 *
 *  Created on: Jul 19, 2013
 *      Author: arturo castro
 */

#pragma once

#include "ofMain.h"
#include "ofxGstRTPClient.h"
#include "ofxGstRTPServer.h"
#include "ofxGui.h"

#define DO_ECHO_CANCEL 0

#if DO_ECHO_CANCEL
#include "ofxEchoCancel.h"
#endif

class ofApp : public ofBaseApp{
    
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
    
    vector<ofxGstRTPClient*> clients;
    vector<ofxGstRTPServer*> servers;
    
    ofVideoGrabber grabber;
//    ofVideoPlayer player;
    vector<ofFbo* > remoteVideos;
    ofTexture mLoadingScreen;
    vector<bool> bConnected;
    
    ofxPanel gui;
    ofxPanel clientGui;
    

#if DO_ECHO_CANCEL
    ofxEchoCancel echoCancel;
#endif
};
