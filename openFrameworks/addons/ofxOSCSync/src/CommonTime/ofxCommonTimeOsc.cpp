//
//  ofxCommonTimeOsc.cpp
//
//
//  Created by Andreas Müller on 26/12/2012.
//  Modified by Dan Moore
//

#include "ofxCommonTimeOsc.h"

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofxCommonTimeOSC::ofxCommonTimeOSC()
{
    isSenderSetup = false;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
ofxCommonTimeOSC::~ofxCommonTimeOSC()
{
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxCommonTimeOSC::init( ofxOscSender* _oscSender, int _uniqueComputerID  )
{
    baseInit();
    uniqueComputerID = _uniqueComputerID;
    oscSender = _oscSender;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxCommonTimeOSC::senderIsSetup( bool _isSetup )
{
    isSenderSetup = _isSetup;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxCommonTimeOSC::sendPing()
{
    if( !isSenderSetup )
    {
        if(ofGetLogLevel() == OF_LOG_VERBOSE)
            ofLog(OF_LOG_WARNING)<<"ofxCommonTimeOSC sender is not setup"<<endl;
        return;
    }

    ofxOscMessage m;
    m.setAddress("/ping");
    m.addIntArg( uniqueComputerID );
    m.addIntArg( getInternalTimeMillis() );

    oscSender->sendMessage( m );
}
