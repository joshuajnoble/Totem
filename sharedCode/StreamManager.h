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
    
    struct clientParameters{
        string ipAddress;
        string audioPort;
        string videoPort;
        string remoteAudioPort;
        string remoteVideoPort;
        string clientID;
    };
    
    void setup(int _width = 640, int _height = 480);
    void update();
    void exit();
    void drawDebug();
    void setImageSource(shared_ptr<ofImage> cam_img);
    
    void newClient(clientParameters params);
    void newServer(clientParameters params);
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
    
    map<string, ofxGstRTPClient*> clients;
    map<string, ofxGstRTPServer*> servers;
    
    //    ofVideoPlayer player;
    map<string, ofFbo*> remoteVideos;
    ofTexture mLoadingScreen;
    map<string,bool> bConnected;
    
    ofxPanel gui;
    ofxPanel clientGui;
    shared_ptr<ofImage> mImg;
    
    float lastSend;
    

    
    clientParameters thisClient;
    
    string localIPAddrss;
    float rotation;
    int videoPort;
    int audioPort;
    
    map<string, clientParameters> connections;
    
};