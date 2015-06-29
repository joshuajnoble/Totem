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
		const static int PORT_SKIP = 5;
        string ipAddress;
        int audioPort;
        int videoPort;
        int remoteAudioPort;
        int remoteVideoPort;

        string clientID;

        int videoWidth;
        int videoHeight;
		int portOffset;
    };
    
	void CreateNewConnection(const clientParameters& newConnection);

    // initialize everything
    void setup(int _width = 640, int _height = 480);
    
    // call this every frame
    void update();
    // function to URI escape JSON and Send via ofxOSCSync
    void sendJSONData(ofxJSONElement json);
    // close everything down
    void exit();
    
    void drawDebug();
//    void setImageSource(ofPtr<ofImage> cam_img);
    int hash(const char * str);
    
    void newClient(clientParameters params);
    void newServer(clientParameters params);
    void newFrame(ofPixelsRef ref);

    bool isServer;

	void ClientDisconnected(string clientId);

    
    void sendRotation(float rotation);
    
    // is there a new frame
    bool isFrameNew();
    void newData(DataPacket& _packet);
    bool bNewFrame;
    
    ofEvent<string> newClientEvent;
	ofEvent<string> clientDisconnectedEvent;
	ofEvent<string> clientStreamAvailableEvent;
    ofEvent<float> newRotationEvent;
    
    ofxJSONElement json;
    
    ofxCommonTimeOSC* commonTimeOsc;

    ofPtr<ofxServerOscManager> oscBroadcaster;
    ofPtr<ofxClientOSCManager> oscReceiver;

    
	int broadcastVideoBitrate;
    float width;
    float height;
    
    map<string, ofPtr<ofxGstRTPClient> > clients;
    map<string, ofPtr<ofxGstRTPServer> > servers;
    
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