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
    
#ifdef USE_DISCOVERY
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
    
#endif
    
#ifdef READ_NETWORK_SETTINGS_FROM_FILE
    auto path = ofToDataPath("connections");
    ofDirectory dir(path);
    if (dir.exists())
    {
        dir.listDir();
        for (int i = 0; i < dir.size(); i++){
            ofXml xml;
            xml.load(dir.getPath(i));
            clientParameters newConnection;
            newConnection.clientID = ofToString(i);
            newConnection.ipAddress = xml.getValue<string>("//ipAddress");
            newConnection.videoPort = xml.getValue<int>("//videoPort");
            newConnection.audioPort = xml.getValue<int>("//audioPort");
            newConnection.remoteVideoPort = xml.getValue<int>("//remoteVideoPort");
            newConnection.remoteAudioPort = xml.getValue<int>("//remoteAudioPort");
            newConnection.videoWidth = xml.getValue<int>("//videoWidth", 1280);
            newConnection.videoHeight = xml.getValue<int>("//videoHeight", 720);
            CreateNewConnection(newConnection);
        }
    }
#endif
}

void StreamManager::CreateNewConnection(const clientParameters& newConnection)
{
    newClient(newConnection);
    newServer(newConnection);
    connections[newConnection.clientID] = newConnection;
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
                CreateNewConnection(newConnection);
            }
        }
        
        if(json.isMember("disconnect")){
            string name = json["disconnect"].asString();
			ClientDisconnected(name);
        }
        
        if(json.isMember("rotation")){
            rotation =json["rotation"].asFloat();
        }
    }
   
}

void StreamManager::sendRotation(float rotation){
#ifdef USE_DISCOVERY
    ofxJSONElement sendJSON;
    sendJSON["rotation"] = rotation;
    sendJSONData(sendJSON);
#endif
}

void StreamManager::exit(){
#ifdef USE_DISCOVERY
    ofxJSONElement sendJSON;
    sendJSON["disconnect"] = thisClient.clientID;
    sendJSONData(sendJSON);
#endif
}

void StreamManager::sendJSONData(ofxJSONElement sendJSON){
#ifdef USE_DISCOVERY
    DataPacket p;
    string sendString;
    Poco::URI::encode(sendJSON.getRawString(false), "/", sendString);
    p.valuesString.push_back(sendString);
    if(isServer){
        oscBroadcaster->sendData(p);
    }else{
        oscReceiver->sendData(p);
    }
#endif
}

void StreamManager::newFrame(ofPixelsRef ref){
    for(map<string, ofPtr<ofxGstRTPServer> >::iterator iter = servers.begin(); iter != servers.end(); ++iter){
        iter->second->newFrame(ref);
        iter->second->videoBitrate = this->broadcastVideoBitrate;
    }
}

//void StreamManager::setImageSource(ofPtr<ofImage> cam_img){
//    mImg = cam_img;
//}

void StreamManager::update(){
    //        if(ofGetElapsedTimef() - lastSend > 1.5){
    //            ofxJSONElement sendJSON;
    //            ofxJSONElement connection;
    //
    //
    //            connection["clientID"] = thisClient.clientID;
    //            connection["ipAddress"] = thisClient.ipAddress;
    //            connection["audioPort"] = thisClient.audioPort;
    //            connection["videoPort"] = thisClient.videoPort;
    //            connection["remoteAudioPort"] = thisClient.remoteAudioPort;
    //            connection["remoteAudioPort"] = thisClient.remoteVideoPort;
    //            connection["videoWidth"] = width;
    //            connection["videoHeight"] = height;
    //
    //            sendJSON["connection"] = connection;
    //            sendJSONData(sendJSON);
    //            lastSend = ofGetElapsedTimef();
    //        }
    
    
    
    for(map<string, ofPtr<ofxGstRTPClient> >::iterator iter = clients.begin(); iter != clients.end(); ++iter){
        iter->second->update();
		if(iter->second->isFrameNewVideo())
		{
			remoteVideos[iter->first]->getTextureReference().loadData(iter->second->getPixelsVideo());
			if(!bConnected[iter->first]){
				bConnected[iter->first] = true;
				ofNotifyEvent(clientStreamAvailableEvent, const_cast<string&>(iter->first), this);
			}
		}
		else
		{
            // draw a spinner for a loading screen if we're not connected yet
            if(!bConnected[iter->first])
			{
                ofEnableAlphaBlending();
                auto video = remoteVideos[iter->first];
                video->begin();
                ofClear(0, 0, 0);
				ofSetColor(255);
				auto centerPoint = ofPoint(video->getWidth() / 2, video->getHeight() / 2);
				auto outerRadius = 15;// video->getHeight() * 0.04;
				auto radius = 5;// video->getHeight() * 0.02;
				for (int i = 0; i < 6; i++){
                    ofPushMatrix();
					ofTranslate(centerPoint);
					auto circleCenter = ofPoint(outerRadius * cos(ofGetElapsedTimef()*2.5 + i*PI / 3), outerRadius * sin(ofGetElapsedTimef()*2.5 + i*PI / 3));
					ofCircle(circleCenter, radius);
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
    int xOffset = 0;
    for(map<string,  ofPtr<ofFbo> >::iterator iter = remoteVideos.begin(); iter != remoteVideos.end(); ++iter){
        auto connection = this->connections[iter->first];
        iter->second->draw(xOffset, 0, connection.videoWidth, connection.videoHeight);
        xOffset += connection.videoWidth + 10;
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
	clients[params.clientID]->drop = true;    
    
    remoteVideos[params.clientID] = ofPtr<ofFbo>(new ofFbo());
    remoteVideos[params.clientID]->allocate(params.videoWidth,params.videoHeight, GL_RGB);
    
    bConnected[params.clientID] = (false);
    clients[params.clientID]->play();
    
    ofNotifyEvent(newClientEvent, params.clientID, this);
}

void StreamManager::ClientDisconnected(string clientId)
{
	connections.erase(clientId);
	//clients[clientId]->close(); // Destructor will close automatically
	//servers[clientId]->close(); // Destructor will close automatically
	clients.erase(clientId);
	servers.erase(clientId);
	remoteVideos.erase(clientId);
	bConnected.erase(clientId);

	ofNotifyEvent(clientDisconnectedEvent, clientId, this);
}