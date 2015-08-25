#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "DataPacket.h"


struct oscClient
{
    string ipAddress;
    int clientID;
    ofxOscSender sender;
};


// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
class ofxServerOscManager
{
public:

    ofxServerOscManager();
    ~ofxServerOscManager();

    void 			init( string _xmlSettingsPath = "settings/sync-settings.xml" );
    void 			init( string _serverSendHost, int _serverSendPort, int _serverReceivePort );
    void 			draw();
    void 			sendData(DataPacket _packet,  bool broadCast = false);
    void 			sendData(DataPacket _packet, int clientID);
    void 			sendData(vector<string> _valuesStrings, vector<int> _valuesInt, vector<float> _valuesFloat );
    void 			sendData(string clientID, vector<string> _valuesStrings, vector<int> _valuesInt, vector<float> _valuesFloat );
    bool			isInitialised() { return initialised; }

    void			_update(ofEventArgs &e);

    ofEvent<DataPacket>  	newDataEvent;

private:

    int 		getServerTime();

    ofxOscSender 	multicastSender;
    ofxOscReceiver 	receiver;

    vector< string > 	receivedMessageSubjects;
    vector<int> 	clientIDs;

#ifdef HEADLESS
#else
    ofTrueTypeFont font;
#endif
    string			serverSendHost;
    int				serverSendPort;
    int				serverReceivePort;

    int 			lastSentHelloMessageMillis;
    int 			milliseBetweenHelloMessages;

    bool			initialised;


    map<string, oscClient> clients;
};
