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

class StreamManager{
public:
    StreamManager();
    ~StreamManager();
    
    void setup(int _width = 640, int _height = 480);
    void update();
    void drawDebug();
    void setImageSource(shared_ptr<ofImage> cam_img);
    
    void newClient(string ip, int audioPort, int videoPort);
    void newServer(string ip, int audioPort, int videoPort);
    void newFrame();
    bool isFrameNew();
    void newData(DataPacket& _packet);
    bool bNewFrame;
    
    ofEvent<string> newClientEvent;
    ofEvent<float> newRotationEvent;
    
    
    ofxCommonTimeOSC* commonTimeOsc;
    ofxServerOscManager* oscBroadcaster;
    ofxClientOSCManager* oscReceiver;
    
    float width;
    float height;
    
    vector<ofxGstRTPClient*> clients;
    vector<ofxGstRTPServer*> servers;
    
    //    ofVideoPlayer player;
    vector<ofFbo* > remoteVideos;
    ofTexture mLoadingScreen;
    vector<bool> bConnected;
    
    ofxPanel gui;
    ofxPanel clientGui;
    shared_ptr<ofImage> mImg;
    
    float lastSend;
    
    struct clientParameters{
        string ipAddress;
        string audioPort;
        string videoPort;
        string clientID;
    };
    
    clientParameters thisClient;
    
    float rotation;
    int videoPort;
    int audioPort;
    
    map<string, clientParameters> connections;
    
};