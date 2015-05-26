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
    mLoadingScreen.allocate(_width, _height, GL_RGB);
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
    
    
    
    oscBroadcaster = ofPtr<ofxServerOscManager>(new ofxServerOscManager());
    oscBroadcaster->init( settings.getValue<string>("//broadcastAddress"), 1234, 2345);
    oscReceiver =  ofPtr<ofxClientOSCManager>(new ofxClientOSCManager());
    
    oscReceiver->init(0, 1234);
    
    commonTimeOsc = oscReceiver->getCommonTimeOscObj();
    commonTimeOsc->setEaseOffset( true );
    
    ofAddListener(oscReceiver->newDataEvent, this, &StreamManager::newData );
    
    clientParameters newConnection;
    newConnection.ipAddress = "127.0.0.1";
    newConnection.audioPort = "1000";
    newConnection.videoPort = "2000";
    newConnection.clientID = "123124";
    
    
    
    newClient(newConnection);
    newServer(newConnection);
    
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
            
            if(connections.find(newConnection.clientID) == connections.end() && newConnection.ipAddress!= thisClient.ipAddress){
                ofNotifyEvent(newClientEvent, newConnection.clientID, this);
                connections[newConnection.clientID] = newConnection;
                
                ofLog(OF_LOG_VERBOSE)<<"CLIENT ID "<<newConnection.clientID<<endl;
                ofLog(OF_LOG_VERBOSE)<<"AudioPort "<<newConnection.audioPort<<endl;
               ofLog(OF_LOG_VERBOSE)<<"VideoPort "<<newConnection.videoPort<<endl;
                ofLog(OF_LOG_VERBOSE)<<"IpAddress "<<newConnection.ipAddress<<endl;
                
                newClient(newConnection);
                newServer(newConnection);
            }
        }
        
        if(json.isMember("disconnection")){
            string name = json["disconnection"].asString();
            if(connections.find(name) != connections.end()){
                clients[name]->close();
                servers[name]->close();
                clients.erase(name);
                servers.erase(name);
                remoteVideos.erase(name);
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
    sendJSON["disconnection"] = thisClient.clientID;
    sendJSONData(sendJSON);
}

void StreamManager::sendJSONData(ofxJSONElement sendJSON){
    DataPacket p;
    string sendString;
    Poco::URI::encode(sendJSON.getRawString(false), "/", sendString);
    p.valuesString.push_back(sendString);
    oscBroadcaster->sendData(p, true);
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

void StreamManager::setImageSource(ofPtr<ofImage> cam_img){
    mImg = cam_img;
}

void StreamManager::update(){
    
    if(ofGetElapsedTimef() - lastSend > 1.5){
        ofxJSONElement sendJSON;
        ofxJSONElement connection;

        connection["videoWidth"] = width;
        connection["videoHeight"] = height;
        connection["clientID"] = thisClient.clientID;
        connection["ipAddress"] = thisClient.ipAddress;
        if(servers.size() == 0){
            connection["audioPort"] = thisClient.audioPort;
            connection["videoPort"] = thisClient.videoPort;
        }else{
            connection["audioPort"] = thisClient.audioPortTwo;
            connection["videoPort"] = thisClient.videoPortTwo;
        }
        sendJSON["connection"] = connection;
        sendJSONData(sendJSON);
        lastSend = ofGetElapsedTimef();
    }
    
    
    if(isFrameNew()){
        for(map<string, ofPtr<ofxGstRTPServer> >::iterator iter = servers.begin(); iter != servers.end(); iter++){
            iter->second->newFrame(mImg->getPixelsRef());
        }
    }
    int i = 0;
    for(map<string, ofPtr<ofxGstRTPClient> >::iterator iter = clients.begin(); iter != clients.end(); iter++){
        iter->second->update();
        if(iter->second->isFrameNewVideo()){
            remotePixels[iter->first]->setFromPixels(iter->second->getPixelsVideo());
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
                    ofTranslate(width/2, height/2);
                    ofCircle(15*cos(ofGetElapsedTimef()*2.5+i*PI/3), 15*sin(ofGetElapsedTimef()*2.5+i*PI/3), 5);
                    ofPopMatrix();
                }
                remoteVideos[iter->first]->end();
                ofDisableAlphaBlending();
            }
        }
        i++;
    }
}

void StreamManager::drawDebug(){
    int i = 0;
    for(map<string,  ofPtr<ofFbo> >::iterator iter = remoteVideos.begin(); iter != remoteVideos.end(); iter++){
        iter->second->draw(0+i*width/2, 0, width/2, height/2);
        i++;
    }
}


void StreamManager::newServer(clientParameters params){
    if(servers.size() == 0){
        servers[params.clientID] = ofPtr<ofxGstRTPServer>(new ofxGstRTPServer());
        servers[params.clientID]->setup(params.ipAddress);
        servers[params.clientID]->addVideoChannel(ofToInt(thisClient.videoPort),width,height,30);
        servers[params.clientID]->addAudioChannel(ofToInt(thisClient.audioPort));
        servers[params.clientID]->videoBitrate = 1600;
        servers[params.clientID]->play();
    }else{
        servers[params.clientID] = ofPtr<ofxGstRTPServer>(new ofxGstRTPServer());
        servers[params.clientID]->setup(params.ipAddress);
        servers[params.clientID]->addVideoChannel(ofToInt(thisClient.videoPortTwo),width,height,30);
        servers[params.clientID]->addAudioChannel(ofToInt(thisClient.audioPortTwo));
        servers[params.clientID]->videoBitrate = 1600;
        servers[params.clientID]->play();
    }
}

void StreamManager::newClient(clientParameters params){
    
    clients[params.clientID] = ofPtr<ofxGstRTPClient>(new ofxGstRTPClient());
    clients[params.clientID]->setup(params.ipAddress, 0);
    clients[params.clientID]->addVideoChannel(ofToInt(params.videoPort));
    clients[params.clientID]->addAudioChannel(ofToInt(params.audioPort));
    
    remoteVideos[params.clientID] = ofPtr<ofFbo>(new ofFbo());
    remoteVideos[params.clientID]->allocate(params.videoWidth,params.videoHeight, GL_RGB);
    
    bConnected[params.clientID] = (false);
    clients[params.clientID]->play();
}