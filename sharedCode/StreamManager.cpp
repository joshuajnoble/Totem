//
//  StreamManager.cpp
//  example-video_audio_LAN
//
//  Created by dantheman on 4/28/15.
//
//

#include "StreamManager.h"
#include <Poco/URI.h>

StreamManager::StreamManager(){
    
}
StreamManager::~StreamManager(){
    
}

void StreamManager::setup(int _width, int _height){
    width = _width;
    height = _height;
    
    //
    //  All Client Settings are in /data/client_settings.xml
    //
    //  - Unique Client ID                  <clientID>
    //  - IP Address of the Machine         <ipAddress>
    //  - Unique Video Streaming Port       <videoPort>
    //  - Unique Audio Streaming Port       <audioPort>
    //  - Unique Video Streaming Port       <videoPortTwo>
    //  - Unique Audio Streaming Port       <audioPortTwo>
    //  - Broadcast Address of the Network  <broadcastAddress>
    //
    // port numbers have to be even according to the standard and have to be separated by at least
    // 4 numbers since internally rtp uses the next 2 odd port numbers for communicating stats
    // of the network state through rctp, so if we set 5000 for video internally it'll use
    // also 5001 and 5003
    
    
    ofXml settings;
    settings.load(ofToDataPath("client_settings.xml"));
    
    thisClient.ipAddress = settings.getValue<string>("//ipAddress");
    thisClient.clientID = settings.getValue<string>("//clientID");
    thisClient.videoPort = settings.getValue<string>("//videoPort");
    thisClient.audioPort = settings.getValue<string>("//audioPort");
    thisClient.videoPortTwo = settings.getValue<string>("//videoPortTwo");
    thisClient.audioPortTwo = settings.getValue<string>("//audioPortTwo");
    
    
#ifdef SERVER
    oscBroadcaster = ofPtr<ofxServerOscManager>(new ofxServerOscManager());
    oscBroadcaster->init( settings.getValue<string>("//broadcastAddress"), 1234, 2345);
#endif
    oscReceiver =  ofPtr<ofxClientOSCManager>(new ofxClientOSCManager());
    oscReceiver->init(hash(thisClient.clientID.c_str()), 1234);

    
    commonTimeOsc = oscReceiver->getCommonTimeOscObj();
        commonTimeOsc->setEaseOffset( true );
    
#ifdef SERVER
    //ofAddListener(oscBroadcaster->newDataEvent, this, &StreamManager::newData );
#endif
    ofAddListener(oscReceiver->newDataEvent, this, &StreamManager::newData );
}

int StreamManager::hash(const char * str)
{
    int h = 0;
    while (*str)
        h = h << 1 ^ *str++;
    return h;
}

void StreamManager::newData( DataPacket& _packet  )
{
    string decodedJson;
    Poco::URI::decode(_packet.valuesString[0], decodedJson);
    if(json.parse(decodedJson)){
        if(json.isMember("connection")){
            clientParameters newConnection;
            newConnection.ipAddress = json["connection"]["ipAddress"].asString();
            newConnection.audioPort = json["connection"]["audioPort"].asString();
            newConnection.videoPort = json["connection"]["videoPort"].asString();
            newConnection.clientID = json["connection"]["clientID"].asString();
            newConnection.videoWidth = json["connection"]["videoWidth"].asInt();
            newConnection.videoHeight = json["connection"]["videoHeight"].asInt();
            
            
            ofLog(OF_LOG_NOTICE)<<"CLIENT ID "<<newConnection.clientID<<endl;
            ofLog(OF_LOG_NOTICE)<<"AudioPort "<<newConnection.audioPort<<endl;
            ofLog(OF_LOG_NOTICE)<<"VideoPort "<<newConnection.videoPort<<endl;
            ofLog(OF_LOG_NOTICE)<<"IpAddress "<<newConnection.ipAddress<<endl;
            
            
            if (connections.find(newConnection.clientID) == connections.end() && newConnection.ipAddress != thisClient.ipAddress){
           
                newServer(newConnection);
                newClient(newConnection);
                connections[newConnection.clientID] = newConnection;
            }
        }
        
        if(json.isMember("disconnect")){
            string name = json["disconnect"].asString();
            if(connections.find(name) != connections.end()){
                clients[name]->close();
                servers[name]->close();
                clients.erase(name);
                servers.erase(name);
                remoteVideos.erase(name);
                remotePixels.erase(name);
                bConnected.erase(name);
                connections.erase(name);
            }
        }
        
        if(json.isMember("rotation")){
            rotation =json["rotation"].asFloat();
        }
    }
    
}

void StreamManager::sendRotation(float rotation){
    ofxJSONElement sendJSON;
    sendJSON["rotation"] = rotation;
    sendJSONData(sendJSON);
}

void StreamManager::exit(){
    ofxJSONElement sendJSON;
    sendJSON["disconnect"] = thisClient.clientID;
    sendJSONData(sendJSON);
}

void StreamManager::sendJSONData(ofxJSONElement sendJSON){
    DataPacket p;
    string sendString;
    Poco::URI::encode(sendJSON.getRawString(false), "/", sendString);
    p.valuesString.push_back(sendString);
#ifdef SERVER
    oscBroadcaster->sendData(p);
#else
    oscReceiver->sendData(p);
#endif
}

bool StreamManager::isFrameNew(){
    bool rval = bNewFrame;
    bNewFrame = false;
    return rval;
}

void StreamManager::newFrame(){
    bNewFrame = true;
}

void StreamManager::setImageSource(ofPtr<ofImage> cam_img){
    mImg = cam_img;
}

void StreamManager::update(){
    if(ofGetElapsedTimef() - lastSend > 1.5){
        ofxJSONElement sendJSON;
        ofxJSONElement connection;
        
        
        connection["clientID"] = thisClient.clientID;
        connection["ipAddress"] = thisClient.ipAddress;
        if(connections.size() == 0){
            connection["audioPort"] = thisClient.audioPort;
            connection["videoPort"] = thisClient.videoPort;
        }else{
            connection["audioPort"] = thisClient.audioPortTwo;
            connection["videoPort"] = thisClient.videoPortTwo;
        }
        connection["videoWidth"] = width;
        connection["videoHeight"] = height;
        
        sendJSON["connection"] = connection;
        sendJSONData(sendJSON);
        lastSend = ofGetElapsedTimef();
    }
    
    
    if(isFrameNew()){
        for(map<string, ofxGstRTPServer* >::iterator iter = servers.begin(); iter != servers.end(); ++iter){
            iter->second->newFrame(mImg->getPixelsRef());
        }
    }
    for(map<string, ofxGstRTPClient*>::iterator iter = clients.begin(); iter != clients.end(); ++iter){
        iter->second->update();
        if(iter->second->isFrameNewVideo()){
            remoteVideos[iter->first]->getTextureReference().loadData(iter->second->getPixelsVideo());
            if(!bConnected[iter->first]){
                bConnected[iter->first]= true;
            }
        }else{
            
            // draw a spinner for a loading screen if we're not connected yet
            if(!bConnected[iter->first]){
                ofEnableAlphaBlending();
                remoteVideos[iter->first]->begin();
                ofClear(0, 0, 0);
                ofSetColor(255, 255, 255, 75);
                for(int i = 0; i < 6; i++){
                    ofPushMatrix();
                    ofTranslate(remoteVideos[iter->first]->getWidth()/2, remoteVideos[iter->first]->getHeight()/2);
                    ofCircle(15*cos(ofGetElapsedTimef()*2.5+i*PI/3), 15*sin(ofGetElapsedTimef()*2.5+i*PI/3), 5);
                    ofPopMatrix();
                }
                remoteVideos[iter->first]->end();
                ofDisableAlphaBlending();
            }
        }
    }
}

void StreamManager::drawDebug(){
    int i = 0;
    for(map<string,  ofPtr<ofFbo> >::iterator iter = remoteVideos.begin(); iter != remoteVideos.end(); ++iter){
        iter->second->draw(0+i*width/2, 0, width/2, height/2);
        i++;
    }
}


void StreamManager::newServer(clientParameters params){
    if(connections.size() == 0){
        servers[params.clientID] = new ofxGstRTPServer();
        servers[params.clientID]->setup(params.ipAddress);
        servers[params.clientID]->addVideoChannel(ofToInt(thisClient.videoPort),width,height,30);
        servers[params.clientID]->addAudioChannel(ofToInt(thisClient.audioPort));
        servers[params.clientID]->videoBitrate = 6000;
        //        servers[params.clientID]->audioBitrate = 1000;
        servers[params.clientID]->play();
    }else{
        servers[params.clientID] = new ofxGstRTPServer();
        servers[params.clientID]->setup(params.ipAddress);
        servers[params.clientID]->addVideoChannel(ofToInt(thisClient.videoPortTwo),width,height,30);
        servers[params.clientID]->addAudioChannel(ofToInt(thisClient.audioPortTwo));
        servers[params.clientID]->videoBitrate = 6000;
        //        servers[params.clientID]->audioBitrate = 1000;
        servers[params.clientID]->play();
    }
}

void StreamManager::newClient(clientParameters params){
    
    clients[params.clientID] = new ofxGstRTPClient();
    clients[params.clientID]->setup(params.ipAddress, 0);
    clients[params.clientID]->addVideoChannel(ofToInt(params.videoPort));
    clients[params.clientID]->addAudioChannel(ofToInt(params.audioPort));
    
    remoteVideos[params.clientID] = ofPtr<ofFbo>(new ofFbo());
    remoteVideos[params.clientID]->allocate(params.videoWidth,params.videoHeight, GL_RGB);
    remotePixels[params.clientID] = ofPtr<ofImage>(new ofImage());
    
    bConnected[params.clientID] = (false);
    clients[params.clientID]->play();
    
    ofNotifyEvent(newClientEvent, params.clientID, this);
}