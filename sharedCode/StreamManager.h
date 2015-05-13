//
//  StreamManager.h
//  example-video_audio_LAN
//
//  Created by dantheman on 4/28/15.
//
//

#pragma once
#include "ofMain.h"
#include "ofxOSCSync.h"
#include "ofxGstRTPClient.h"
#include "ofxGstRTPServer.h"
#include "ofxGui.h"
#include "ofxJSON.h"

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
        string clientID;
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
    
    // add a new client
    void newClient(clientParameters params);
    
    // add a new server
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
    ofPtr<ofxServerOscManager> oscBroadcaster;
    ofPtr<ofxClientOSCManager> oscReceiver;
    
    float width;
    float height;
    
    
    map<string, ofPtr<ofxGstRTPClient> > clients;
    map<string, ofPtr<ofxGstRTPServer> > servers;
    
    //    ofVideoPlayer player;
    map<string, ofPtr<ofFbo> > remoteVideos;
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