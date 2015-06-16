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
    isServer = ofToBool(settings.getValue<string>("//isServer"));
    
    if(isServer){
        oscBroadcaster = ofPtr<ofxServerOscManager>(new ofxServerOscManager());
        oscBroadcaster->init( settings.getValue<string>("//broadcastAddress"), 1234, 2345);
    }
    oscReceiver =  ofPtr<ofxClientOSCManager>(new ofxClientOSCManager());
    oscReceiver->init(hash(thisClient.clientID.c_str()), 1234);
    
    
    commonTimeOsc = oscReceiver->getCommonTimeOscObj();
    commonTimeOsc->setEaseOffset( true );
    
    if(isServer){
        ofAddListener(oscBroadcaster->newDataEvent, this, &StreamManager::newData );
    }else{
        ofAddListener(oscReceiver->newDataEvent, this, &StreamManager::newData );
    }
    
    
    ofDirectory dir;
    dir.listDir(ofToDataPath("connections"));
    for(int i = 0; i < dir.size(); i++){
        ofXml xml;
        xml.load(dir.getPath(i));
        clientParameters newConnection;
        newConnection.clientID = ofToString(i);
        newConnection.ipAddress = xml.getValue<string>("//ipAddress");
        newConnection.videoPort = xml.getValue<int>("//videoPort");
        newConnection.audioPort = xml.getValue<int>("//audioPort");
		newConnection.remoteVideoPort = xml.getValue<int>("//remoteVideoPort");
		newConnection.remoteAudioPort = xml.getValue<int>("//remoteAudioPort");
        newConnection.videoWidth = xml.getValue<int>("//videoWidth", 640);
        newConnection.videoHeight = xml.getValue<int>("//videoHeight", 480);
        
		newClient(newConnection);
        newServer(newConnection);

		connections[newConnection.clientID] = newConnection;
    }
    
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
            newConnection.audioPort = json["connection"]["audioPort"].asInt();
            newConnection.videoPort = json["connection"]["videoPort"].asInt();
            newConnection.remoteAudioPort = json["connection"]["remoteAudioPort"].asInt();
            newConnection.remoteVideoPort = json["connection"]["remoteVideoPort"].asInt();
            newConnection.clientID = json["connection"]["clientID"].asString();
            newConnection.videoWidth = json["connection"]["videoWidth"].asInt();
            newConnection.videoHeight = json["connection"]["videoHeight"].asInt();
            
            
            ofLog(OF_LOG_NOTICE)<<"CLIENT ID "<<newConnection.clientID<<endl;
            ofLog(OF_LOG_NOTICE)<<"IpAddress "<<newConnection.ipAddress<<endl;
            ofLog(OF_LOG_NOTICE)<<"AudioPort "<<newConnection.audioPort<<endl;
            ofLog(OF_LOG_NOTICE)<<"VideoPort "<<newConnection.videoPort<<endl;
            
            
            
            if (connections.find(newConnection.clientID) == connections.end() && newConnection.ipAddress != thisClient.ipAddress){
            
                newClient(newConnection);
                newServer(newConnection);
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
                bConnected.erase(name);
                connections.erase(name);

				ClientDisconnected(name);
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
    if(isServer){
        oscBroadcaster->sendData(p);
    }else{
        oscReceiver->sendData(p);
    }
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
            connection["audioPort"] = thisClient.audioPort;
            connection["videoPort"] = thisClient.videoPort;
            connection["remoteAudioPort"] = thisClient.remoteAudioPort;
            connection["remoteAudioPort"] = thisClient.remoteVideoPort;
            connection["videoWidth"] = width;
            connection["videoHeight"] = height;
    
            sendJSON["connection"] = connection;
            sendJSONData(sendJSON);
            lastSend = ofGetElapsedTimef();
        }
    
    
    if(isFrameNew()){
        for(map<string, ofPtr<ofxGstRTPServer> >::iterator iter = servers.begin(); iter != servers.end(); ++iter){
            iter->second->newFrame(mImg->getPixelsRef());
            iter->second->videoBitrate = 6000;
        }
    }
    for(map<string, ofPtr<ofxGstRTPClient> >::iterator iter = clients.begin(); iter != clients.end(); ++iter){
        iter->second->update();
        if(iter->second->isFrameNewVideo()){
            remoteVideos[iter->first]->getTextureReference().loadData(iter->second->getPixelsVideo());
            if(!bConnected[iter->first]){
                bConnected[iter->first] = true;
				ofNotifyEvent(clientStreamAvailableEvent, const_cast<string&>(iter->first), this);
            }
        }else{
            // draw a spinner for a loading screen if we're not connected yet
            if(!bConnected[iter->first]){
                ofEnableAlphaBlending();
				auto video = remoteVideos[iter->first];
                video->begin();
                ofClear(0, 0, 0);
                for(int i = 0; i < 6; i++){
                    ofPushMatrix();
                    ofTranslate(remoteVideos[iter->first]->getWidth()/2, remoteVideos[iter->first]->getHeight()/2);
                    ofCircle(15*cos(ofGetElapsedTimef()*2.5+i*PI/3), 15*sin(ofGetElapsedTimef()*2.5+i*PI/3), 5);

					ofPopMatrix();
                }
                video->end();
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
    servers[params.clientID] = ofPtr<ofxGstRTPServer>(new ofxGstRTPServer());
    servers[params.clientID]->setup(params.ipAddress);
	servers[params.clientID]->addVideoChannel(params.remoteVideoPort,width,height,30);
    servers[params.clientID]->addAudioChannel(params.remoteAudioPort);
    servers[params.clientID]->play();
}

void StreamManager::newClient(clientParameters params){
    
    clients[params.clientID] = ofPtr<ofxGstRTPClient>(new ofxGstRTPClient());
    clients[params.clientID]->setup(params.ipAddress, 0);
    clients[params.clientID]->addVideoChannel(params.videoPort);
    clients[params.clientID]->addAudioChannel(params.audioPort);
    
    
    remoteVideos[params.clientID] = ofPtr<ofFbo>(new ofFbo());
    remoteVideos[params.clientID]->allocate(params.videoWidth,params.videoHeight, GL_RGB);
    
    bConnected[params.clientID] = (false);
    clients[params.clientID]->play();

	ofNotifyEvent(newClientEvent, params.clientID, this);
}

void StreamManager::ClientDisconnected(string clientId)
{
	ofNotifyEvent(clientDisconnectedEvent, clientId, this);

}