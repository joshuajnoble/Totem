#pragma once

#include "ofMain.h"

#include "ofxNetwork.h"

enum CONTENT_TYPE
{
    REMOTE, PPI, PANO, SMALL_CONTENT_1, SMALL_CONTENT_2
};

enum OVERLAY
{
    NO_OVERLAY, MARUSKA, NOBLE, MEGAN, JOHN
};

class ofApp : public ofBaseApp{

	public:

	void setup();
	void update();
	void draw();
		
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	ofVideoPlayer 		v360;
    ofVideoPlayer       remoteViewer;
    bool drawOverlay;
    
    ofImage ppiImage, ppiSmall, ppiSmall2;
    
    ofRectangle panoRect, ppiRect, remoteRect, smallRect, smallRect2;
    ofRectangle selectedCanvas;
    ofFbo panoFbo;
    
    CONTENT_TYPE canvasContentObject;
    
    OVERLAY overlayType;
    
    ofTrueTypeFont din;
    ofSerial serialConnection;
    
};

