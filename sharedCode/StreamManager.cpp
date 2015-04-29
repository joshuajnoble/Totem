//
//  StreamManager.cpp
//  example-video_audio_LAN
//
//  Created by dantheman on 4/28/15.
//
//

#include "StreamManager.h"

StreamManager::StreamManager(){
    
}
StreamManager::~StreamManager(){
    
}

void StreamManager::setup(int _width, int _height){
    mLoadingScreen.allocate(_width, _height, GL_RGB);
    width = _width;
    height = _height;
    
    
    oscBroadcaster = new ofxServerOscManager();
    ///
    ///     Put the Broadcast IP of your Network Here
    ///
    oscBroadcaster->init("192.168.1.255", 1234, 2345);
    
    
    oscReceiver = new ofxClientOSCManager();
    oscReceiver->init(0, 1234);
    
    commonTimeOsc = oscReceiver->getCommonTimeOscObj();
    commonTimeOsc->setEaseOffset( true );
    
    ofAddListener(oscReceiver->newDataEvent, this, &StreamManager::newData );
}

void StreamManager::newData( DataPacket& _packet  )
{
    if(_packet.valuesString.size() == 4){
        clientParameters newConnection;
        newConnection.ipAddress = _packet.valuesString[0];
        newConnection.audioPort = _packet.valuesString[1];
        newConnection.videoPort = _packet.valuesString[2];
        newConnection.clientID = _packet.valuesString[3];
        if(connections.find(newConnection.clientID) == connections.end()){
            ofNotifyEvent(newClientEvent, newConnection.clientID, this);
            connections[newConnection.clientID] = newConnection;
         
            newClient(newConnection.ipAddress, ofToInt(newConnection.audioPort), ofToInt(newConnection.videoPort));
            newServer(newConnection.ipAddress, ofToInt(newConnection.audioPort), ofToInt(newConnection.videoPort));
        }
        
    }
    
    
    if(_packet.valuesFloat.size() > 0){
        rotation = _packet.valuesFloat[0];
    }
    
}

bool StreamManager::isFrameNew(){
    if(bNewFrame){
        bNewFrame = false;
        return true;
    }
}
void StreamManager::newFrame(){
    bNewFrame = true;
}

void StreamManager::setImageSource(shared_ptr<ofImage> cam_img){
    mImg = cam_img;
}

void StreamManager::update(){
    
    if(ofGetElapsedTimef() - lastSend > 1.5){
        DataPacket p;
        p.valuesString.push_back("127.0.0.1");
        p.valuesString.push_back("8000");
        p.valuesString.push_back("7000");
        p.valuesString.push_back("CLIENT-7800");
        oscBroadcaster->sendData(p);
    }
    
    if(ofGetElapsedTimef() - lastSend > 1.5){
        DataPacket p;
        p.valuesString.push_back("127.0.0.1");
        p.valuesString.push_back("6000");
        p.valuesString.push_back("5000");
        p.valuesString.push_back("CLIENT-5600");
        oscBroadcaster->sendData(p);
    }
    
    
    if(ofGetElapsedTimef() - lastSend > 1.5){
        DataPacket p;
        p.valuesString.push_back("127.0.0.1");
        p.valuesString.push_back("4000");
        p.valuesString.push_back("3000");
        p.valuesString.push_back("CLIENT-4300");
        oscBroadcaster->sendData(p);
        lastSend = ofGetElapsedTimef();
    }
    
    if(isFrameNew()){
        for(int i = 0; i < servers.size(); i++){
            servers[i]->newFrame(mImg->getPixelsRef());
        }
    }
    
    for(int i = 0; i < clients.size(); i++){
        clients[i]->update();
        if(clients[i]->isFrameNewVideo()){
            remoteVideos[i]->getTextureReference().loadData(clients[i]->getPixelsVideo());
            if(!bConnected[i]){
                bConnected[i]= true;
            }
        }else{
            if(!bConnected[i]){
                ofEnableAlphaBlending();
                remoteVideos[i]->begin();
                ofClear(0, 0, 0);
                ofSetColor(255, 255, 255, 75);
                for(int i = 0; i < 6; i++){
                    ofPushMatrix();
                    ofTranslate(width/2, height/2);
                    ofCircle(15*cos(ofGetElapsedTimef()*2.5+i*PI/3), 15*sin(ofGetElapsedTimef()*2.5+i*PI/3), 5);
                    ofPopMatrix();
                }
                remoteVideos[i]->end();
                ofDisableAlphaBlending();
            }
        }
    }
}

void StreamManager::drawDebug(){
    for(int i = 0; i < remoteVideos.size(); i++){
        remoteVideos[i]->draw(0+i*remoteVideos[i]->getWidth()/4, 0, remoteVideos[i]->getWidth()/4, remoteVideos[i]->getHeight()/4);
    }
}
// port numbers have to be even according to the standard and have to be separated by at least
// 4 numbers since internally rtp uses the next 2 odd port numbers for communicating stats
// of the network state through rctp, so if we set 5000 for video internally it'll use
// also 5001 and 5003


void StreamManager::newServer(string ip, int audioPort, int videoPort){
    servers.push_back(new ofxGstRTPServer());
    servers.back()->setup(ip);
    servers.back()->addVideoChannel(videoPort,width,height,30);
    servers.back()->addAudioChannel(audioPort);
    servers.back()->videoBitrate = 1600;
    servers.back()->play();
}

void StreamManager::newClient(string ip, int audioPort, int videoPort){

    clients.push_back(new ofxGstRTPClient());
    clients.back()->setup(ip, 0);
    clients.back()->addVideoChannel(videoPort);
    clients.back()->addAudioChannel(audioPort);
    
    remoteVideos.push_back(new ofFbo());
    remoteVideos.back()->allocate(width,height, GL_RGB);

    bConnected.push_back(false);
    clients.back()->play();
}