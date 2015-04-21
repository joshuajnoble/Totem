#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    grabber.initGrabber(640,480);
    
    
    // to run this example sending data from different applications or computers
    // set the ports to be different in the client and server, but matching the client
    // and server ports from one app to the other, for example one computer will have:
    //
    // client video: 5000
    // client audio: 6000
    // server video: 7000
    // server audio: 8000
    //
    // the other:
    //
    // client video: 7000
    // client audio: 8000
    // server video: 5000
    // server audio: 6000
    //
    // port numbers have to be even according to the standard and have to be separated by at least
    // 4 numbers since internally rtp uses the next 2 odd port numbers for communicating stats
    // of the network state through rctp, so if we set 5000 for video internally it'll use
    // also 5001 and 5003
    
    
#if DO_ECHO_CANCEL
    // if echo cancel is enabled we need to setup the echo cancel module and pass it to
    // the server and client, also the server needs a reference to the client in order to now
    // the audio latency to be able to remove the echo
    echoCancel.setup();
    client.setEchoCancel(echoCancel);
    server.setEchoCancel(echoCancel);
    server.setRTPClient(client);
#endif
    
    //    grabber.load("vid_bigbuckbunny.mp4");
    
    // this sets the remote ip and the latency, in a LAN you can usually use latency 0
    // over internet you'll probably need to make it higher, around 200 is usually a good
    // number but depends on the network conditions
    
    for(int i = 0; i < 10; i++){
        
    }
    
    
    
    ofLog()<<(clients.size()+1)*1000<<endl;
    
    gui.setup("","settings.xml",660,10);
    clientGui.setup("","clients.xml");
    for(int i = 0; i < servers.size(); i++){
        gui.add(servers[i]->parameters);
    }
#if DO_ECHO_CANCEL
    gui.add(echoCancel.parameters);
#endif
    
    for(int i = 0; i < servers.size(); i++){
        servers[i]->play();
    }
    
    mLoadingScreen.allocate(640, 480, GL_RGB);
    
    ofBackground(255);
}


void ofApp::exit(){
}


//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    
    if(grabber.isFrameNew()){
        for(int i = 0; i < clients.size(); i++){
            servers[i]->newFrame(grabber.getPixelsRef());
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
                    ofTranslate(grabber.getWidth()/2, grabber.getHeight()/2);
                    ofCircle(15*cos(ofGetElapsedTimef()*2.5+i*PI/3), 15*sin(ofGetElapsedTimef()*2.5+i*PI/3), 5);
                    ofPopMatrix();
                }
                remoteVideos[i]->end();
                ofDisableAlphaBlending();
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    float x = 0;
    float y = 0;
    float width;
    if( (ofGetWidth()/grabber.getWidth())+(ofGetScreenHeight()/grabber.getHeight())<=remoteVideos.size()){
        width = ofGetWidth()/(remoteVideos.size()/(grabber.getWidth()/grabber.getHeight()));
    }else{
        width = grabber.getWidth();
    }
    float height = width*grabber.getHeight()/grabber.getWidth();
    for(int i = 0; i < remoteVideos.size(); i++){
        remoteVideos[i]->draw(x, y, width,height);
        x+=width;
        if((x+width) > ofGetWidth()){
            x = 0;
            y+=height;
        }
    }
    grabber.draw(400,300,grabber.getWidth()/3,grabber.getHeight()/3);
    gui.draw();
    clientGui.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == '='){
        if(clients.size() < 10){
            
            servers.push_back(new ofxGstRTPServer());
            servers.back()->setup("127.0.0.1");
            servers.back()->addVideoChannel(6000+10*(servers.size()),grabber.getWidth(),grabber.getHeight(),30);
            servers.back()->addAudioChannel(5000+10*(servers.size()));
            servers.back()->videoBitrate = 6000;
            servers.back()->play();
            
            clients.push_back(new ofxGstRTPClient());
            clients.back()->setup("127.0.0.1", 0);
            clients.back()->addVideoChannel(6000+10*clients.size());
            clients.back()->addAudioChannel(5000+10*clients.size());
            clients.back()->play();
            
            //            clientGui.add(clients.back()->parameters);
            
            remoteVideos.push_back(new ofFbo());
            remoteVideos.back()->allocate(grabber.getWidth(),grabber.getHeight(), GL_RGB);
            bConnected.push_back(false);
        }
    }
    
    if(key == '-'){
        if(clients.size() > 0){
            clients.back()->close();
            servers.back()->close();
            //            clients.back()->~ofxGstRTPClient();
            delete clients.back();
            clients.erase(clients.end()-1);
            delete servers.back();
            servers.erase(servers.end()-1);
            delete remoteVideos.back();
            remoteVideos.erase(remoteVideos.end()-1);
            bConnected.erase(bConnected.end()-1);
            
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
