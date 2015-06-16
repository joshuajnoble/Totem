//
//  StreamManager.h
//  example-video_audio_LAN
//
//  Created by dantheman on 4/28/15.
//
//

#pragma once
#include "ofMain.h"
#ifdef TARGET_OSX
#include "ofxOSCSync.h"
#include "ofxGstRTPClient.h"
#include "ofxGstRTPServer.h"
#include "ofxGui.h"
#include "ofxJSON.h"
#else
#include "ofxOSCSync/src/ofxOSCSync.h"
#include "ofxGstRTP/src/ofxGstRTPClient.h"
#include "ofxGstRTP/src/ofxGstRTPServer.h"
#include "ofxGui/src/ofxGui.h"
#include "ofxJSON/src/ofxJSON.h"
#endif
//#define SERVER

class StreamManager{
public:
    StreamManager();
    ~StreamManager();
    
    // this is the config for the manager
    struct clientParameters{
        string ipAddress;
        string audioPort;
        string videoPort;
        string audioPortTwo;
        string videoPortTwo;
		string audioPortThree;
        string videoPortThree;
        string clientID;
        int videoWidth;
        int videoHeight;
    };
    
    // initialize everything
    void setup(int _width = 640, int _height = 480);
    
    // call this every frame
    void update();
    // function to URI escape JSON and Send via ofxOSCSync
    void sendJSONData(ofxJSONElement json);
    // close everything down
    void exit();
    
    void drawDebug();
    void setImageSource(ofPtr<ofImage> cam_img);
    int hash(const char * str);
    
    void newClient(clientParameters params);
    void newServer(clientParameters params);
    void newFrame();
    
    
    void sendRotation(float rotation);
    
    // is there a new frame
    bool isFrameNew();
    void newData(DataPacket& _packet);
    bool bNewFrame;
    
    ofEvent<string> newClientEvent;
    ofEvent<float> newRotationEvent;
    
    ofxJSONElement json;
    
    ofxCommonTimeOSC* commonTimeOsc;
#ifdef SERVER
    ofPtr<ofxServerOscManager> oscBroadcaster;
#endif
    ofPtr<ofxClientOSCManager> oscReceiver;

    
    float width;
    float height;
    
    map<string, ofxGstRTPClient* > clients;
    map<string, ofxGstRTPServer* > servers;
    
    map<string, ofPtr<ofFbo> > remoteVideos;
    map<string, ofPtr<ofImage> > remotePixels;
    ofTexture mLoadingScreen;
    map<string,bool> bConnected;
    
    ofxPanel gui;
    ofxPanel clientGui;
    ofPtr<ofImage> mImg;
    
    float lastSend;
    
    clientParameters thisClient;
    
    string localIPAddrss;
    float rotation;
    int videoPort;
    int audioPort;
    
    map<string, clientParameters> connections;
};