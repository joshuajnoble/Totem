#ifndef _BLOGGIE_APP
#define _BLOGGIE_APP
#include "ofMain.h"

//#define IMAGE
#define GRABBER

#include "ofxNetwork.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp{
	
	public:
		
		//----------------------------------------
		/* standard openFrameworks app stuff */
		void setup();
		void update();
		void draw();
		void exit();
	
		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
    
		
		//----------------------------------------
		/* Panoramic unwarp stuff */
	
        ofImage image;
		ofVideoPlayer player;
    
        ofVideoGrabber grabber;
		int	currentCodecId;
		string outputFileName;
		char *handyString;
	
		void computePanoramaProperties();
		void computeInversePolarTransform();
		 
		//void drawTexturedCylinder();
		void drawPlayer();
		void drawUnwarpedVideo();
		 
		bool testMouseInPlayer();
		bool bMousePressed;
		bool bMousePressedInPlayer;
		bool bMousepressedInUnwarped;
		bool bAngularOffsetChanged;
		bool bPlayerPaused;
		bool bCenterChanged;
		bool bSavingOutVideo;
		bool bSaveAudioToo;
		int  nWrittenFrames;
		int  codecQuality;
		
		ofImage unwarpedImage;
	
        ofPixels conversionPixels;
        ofPixels inputPixels;
    
		unsigned char *warpedPixels;
		//unsigned char *unwarpedPixels;
        ofPixels unwarpedPixels;
		
		int   warpedW;
		int   warpedH;
		float unwarpedW;
		float unwarpedH;
		float warpedCx;
		float warpedCy;
		float savedWarpedCx;
		float savedWarpedCy;
		float savedAngularOffset;
		float angularOffset;
	
		float maxR;
		float minR;
		float maxR_factor;
		float minR_factor;
		int   interpMethod; 
		float playerScaleFactor;

		unsigned char *blackColor;
		
		float *xocvdata;
		float *yocvdata;
	
		float yWarpA; // for parabolic fit for Y unwarping
		float yWarpB;
		float yWarpC;
	
		//-----------------------------------
		/* For the texture-mapped cylinder */
		ofTexture unwarpedTexture;
		int   cylinderRes;
		float *cylinderX;
		float *cylinderY;
		float cylinderWedgeAngle;
		float blurredMouseX;
		float blurredMouseY;
    
    bool drawCylinder;
    ofTrueTypeFont din;
    bool pixelsLoaded;
    
    ofCylinderPrimitive cylinder;

	ofxOscReceiver rec;
	ofxUDPManager videoStreamer;

	ofFbo fbo;
		
};

#endif	

