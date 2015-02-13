//
//  pixelClient.h
//  rebloggie
//
//  Created by Joshua Noble on 2/3/15.
//
//

#ifndef rebloggie_pixelClient_h
#define rebloggie_pixelClient_h

#include "ofxNetwork.h"
#include "ofMain.h"

const int NUM_BYTES_IN_IMG = (640 * 480 * 4);

enum clientState{
    NONE, READY, RECEIVING, COMPLETE
};

class pixelClient{
public:
    
    bool hasFrameBegin;
    
    pixelClient(){
        state = NONE;
        hasFrameBegin = false;
        totalBytes = NUM_BYTES_IN_IMG;
        bytesRecieved = 0;
        memset(pixels, 255, totalBytes);
    }
    
    int getState(){
        return state;
    }
    
    string getStateStr(){
        if(state == NONE) { return "NONE"; }
        else if(state == READY) { return "READY"; }
        else if(state == RECEIVING) { return "RECEIVING"; }
        else if(state == COMPLETE) { return "COMPLETE"; }
        else { return "ERR"; }
    }
    
    void reset(){
        state = READY;
        bytesRecieved = 0;
    }
    void setup(string ip, int port = 11999){
        TCP.setup("192.168.1.3", 11999);
        state = READY;
        bytesRecieved = 0;
    }
    
    void update(int bytesToGet = 2048){
        
        if( state == READY || state == RECEIVING ){
            
//            if(!hasFrameBegin)
//            {
//                if( TCP.receive() == "begin" )
//                {
//                    hasFrameBegin = true;
//                }
//                return;
//            }
            
            if( bytesToGet + bytesRecieved >= NUM_BYTES_IN_IMG ){
                bytesToGet -= ( ( bytesToGet + bytesRecieved ) -
                               NUM_BYTES_IN_IMG );
            }
            
            char tmpBuffer[bytesToGet];
            int numRecieved = TCP.receiveRawBytes(tmpBuffer, bytesToGet);
            
            if( numRecieved > 0 ){
                state = RECEIVING;
                memcpy(&pixels[bytesRecieved],tmpBuffer, numRecieved);
                bytesRecieved += numRecieved;
            }
            
            if( bytesRecieved >= NUM_BYTES_IN_IMG ){
                state = COMPLETE;
            }
        }
    }
    
    clientState state;
    int bytesRecieved;
    int totalBytes;
    ofxTCPClient TCP; // the TCP client that handles the actual communication
    unsigned char pixels[NUM_BYTES_IN_IMG];
};

#endif
