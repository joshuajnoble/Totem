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
    
    //
    //  All Client Settings are in /data/client_settings.xml
    //
    //  - Unique Client ID                  <clientID>
    //  - IP Address of the Machine         <ipAddress>
    //  - Unique Video Streaming Port       <videoPort>
    //  - Unique Audio Streaming Port       <audioPort>
    //  - Broadcast Address of the Network  <broadcastAddress>
    //
    // port numbers have to be even according to the standard and have to be separated by at least
    // 4 numbers since internally rtp uses the next 2 odd port numbers for communicating stats
    // of the network state through rctp, so if we set 5000 for video internally it'll use
    // also 5001 and 5003

    
    ofXml settings;
    settings.load("client_settings.xml");

	thisClient.ipAddress = settings.getValue<string>("//ipAddress");
	thisClient.clientID = settings.getValue<string>("//clientID");
	thisClient.videoPort = settings.getValue<string>("//videoPort");
	thisClient.audioPort = settings.getValue<string>("//audioPort");
    
    oscBroadcaster = new ofxServerOscManager();
    oscBroadcaster->init(settings.getValue<string>("//broadcastAddress"), 1234, 2345);
    oscReceiver = new ofxClientOSCManager();
    oscReceiver->init(hash(thisClient.clientID.c_str()), 1234);
    
    commonTimeOsc = oscReceiver->getCommonTimeOscObj();
    commonTimeOsc->setEaseOffset( true );
    
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
    if(_packet.valuesString.size() == 4){
        clientParameters newConnection;
        newConnection.ipAddress = _packet.valuesString[0];
        newConnection.audioPort = _packet.valuesString[1];
        newConnection.videoPort = _packet.valuesString[2];
        newConnection.clientID = _packet.valuesString[3];
        
        if(connections.find(newConnection.clientID) == connections.end() && newConnection.clientID != thisClient.clientID){
            ofNotifyEvent(newClientEvent, newConnection.clientID, this);
            connections[newConnection.clientID] = newConnection;
            
            cout<<newConnection.clientID<<endl;
            
            newClient(newConnection);
            newServer(newConnection);
        }
        
    }
    
    if(_packet.valuesString.size() == 2){
        if(_packet.valuesString[1] == "CLOSE"){
            string name = _packet.valuesString[0];
            if(connections.find(name) != connections.end()){
                clients[name]->close();
                servers[name]->close();
                delete clients[name];
                clients.erase(name);
                delete servers[name];
                servers.erase(name);
                delete remoteVideos[name];
                remoteVideos.erase(name);
                bConnected.erase(name);
				connections.erase(name);
            }
        }
    }
    
    if(_packet.valuesFloat.size() > 0){
        rotation = _packet.valuesFloat[0];
    }
    
}

void StreamManager::exit(){
    
    DataPacket p;
	p.valuesString.push_back(thisClient.clientID);
    p.valuesString.push_back("CLOSE");
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

void StreamManager::setImageSource(shared_ptr<ofImage> cam_img){
    mImg = cam_img;
}

void StreamManager::update(){

        if(ofGetElapsedTimef() - lastSend > 1.5){
            DataPacket p;
            p.valuesString.push_back(thisClient.ipAddress);
            p.valuesString.push_back(thisClient.audioPort);
            p.valuesString.push_back(thisClient.videoPort);
            p.valuesString.push_back(thisClient.clientID);
            oscBroadcaster->sendData(p, true);
        }
    
    
    if(isFrameNew()){
        for(map<string, ofxGstRTPServer*>::iterator iter = servers.begin(); iter != servers.end(); iter++){
            iter->second->newFrame(mImg->getPixelsRef());
        }
    }
    int i = 0;
    for(map<string, ofxGstRTPClient*>::iterator iter = clients.begin(); iter != clients.end(); iter++){
        iter->second->update();
        if(iter->second->isFrameNewVideo()){
            remoteVideos[iter->first]->getTextureReference().loadData(iter->second->getPixelsVideo());
            if(!bConnected[iter->first]){
                bConnected[iter->first]= true;
            }
        }else{
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
    for(map<string, ofFbo*>::iterator iter = remoteVideos.begin(); iter != remoteVideos.end(); iter++){
        iter->second->draw(0+i*width/2, 0, width/2, height/2);
        i++;
    }
}


void StreamManager::newServer(clientParameters params){
    servers[params.clientID] = new ofxGstRTPServer();
    servers[params.clientID]->setup(params.ipAddress);
	servers[params.clientID]->addVideoChannel(ofToInt(thisClient.videoPort),width,height,30);
    servers[params.clientID]->addAudioChannel(ofToInt(thisClient.audioPort));
    servers[params.clientID]->videoBitrate = 1600;
    servers[params.clientID]->play();
}

void StreamManager::newClient(clientParameters params){
    
    clients[params.clientID] = new ofxGstRTPClient();
    clients[params.clientID]->setup(params.ipAddress, 0);
	clients[params.clientID]->addVideoChannel(ofToInt(params.videoPort));
    clients[params.clientID]->addAudioChannel(ofToInt(params.audioPort));
    
    remoteVideos[params.clientID] = new ofFbo();
    remoteVideos[params.clientID]->allocate(width,height, GL_RGB);
    
    bConnected[params.clientID] = (false);
    clients[params.clientID]->play();
}