#include "ofApp.h"


const int VIDEO_WIDTH = 3240;
const int VIDEO_HEIGHT = 2160;

const int SCREEN_WIDTH = 2160;
const int SCREEN_HEIGHT = 1440;

const int ratio1x2W = 680;
const int ratio1x2H = 300;

const int ratio1x1W = 330;
const int ratio1x1H = 300;

const int canvasW = 1440;
const int canvasH = 860;

long lastClick = 0;


//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(255,255,255);
	ofSetVerticalSync(true);

	// Uncomment this to show movies with alpha channels
	// v360.setPixelFormat(OF_PIXELS_RGBA);

    
    panoRect.set(0, (SCREEN_HEIGHT*0.6), SCREEN_WIDTH, SCREEN_HEIGHT - (SCREEN_HEIGHT * 0.6));
    ppiRect.set( 0, 300, ratio1x2W, ratio1x2H );
    remoteRect.set( 0, 0, ratio1x2W, ratio1x2H );
    
    smallRect2.set( ratio1x1H, ratio1x2H * 2, ratio1x1H, ratio1x1H );
    smallRect.set( 0, ratio1x2H * 2, ratio1x1H, ratio1x1H );
    
    selectedCanvas.set( 0, 0, canvasW, canvasH );
    
	v360.loadMovie("movies/v360_capture1.mp4");
	v360.play();
    
    remoteViewer.loadMovie("movies/TalkingtoJosh.mov");
    remoteViewer.play();
    
    panoFbo.allocate(VIDEO_WIDTH*2, VIDEO_HEIGHT/2);
    
    ppiSmall.loadImage("images/ppi_image_small.jpg");
    ppiSmall2.loadImage("images/ppi_image_small2.png");
    ppiImage.loadImage("images/ppi_image.png");
    
    lastClick = ofGetElapsedTimeMillis();
    
    din.loadFont("fonts/FF_DIN_Pro_Light_Italic.otf", 36);
    
    // this should work
    serialConnection.setup(1, 56900);
}

//--------------------------------------------------------------
void ofApp::update(){
    v360.update();
    remoteViewer.update();
    
    if(serialConnection.available() > 1)
    {
        char serialBuffer[2];
        serialConnection.readBytes(*serialBuffer[0], 2);
        
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetHexColor(0xFFFFFF);
    
    const float panoYPosition = (SCREEN_HEIGHT*0.65);
    const float screenRatioW = 0.31578947368;
    
    // draw viewer
    remoteViewer.getTextureReference().drawSubsection(10, 10, ratio1x2W, ratio1x2H, 0, 0, 1180, 680);
    
    // draw images
    ppiImage.getTextureReference().draw( 10, ratio1x2H, ratio1x2W, ratio1x2H );
    ppiSmall.draw( 10, 10 + ratio1x1H * 2, ratio1x1W, ratio1x1H );
    ppiSmall2.draw( ratio1x1W + 20, 10 + ratio1x1H * 2, ratio1x1W, ratio1x1H );
    
    
    // draw pano
    panoFbo.begin();
    
    v360.getTextureReference().drawSubsection(VIDEO_WIDTH, // x
                                              0, // y
                                              VIDEO_WIDTH, // w
                                              VIDEO_HEIGHT/2,  // h
                                              0,  // sx
                                              0,  // sy
                                              VIDEO_WIDTH, // sw
                                              VIDEO_HEIGHT * 0.5);  // sh
    
    v360.getTextureReference().drawSubsection(0, // x
                                              0, // y
                                              VIDEO_WIDTH, // w
                                              VIDEO_HEIGHT/2,  // h
                                              0,  // sx
                                              VIDEO_HEIGHT * 0.5,  // sy
                                              VIDEO_WIDTH, // sw
                                              VIDEO_HEIGHT * 0.5); // sh
    
    panoFbo.end();
    
    panoFbo.draw(0, panoYPosition, 2160, 450);
    
    // draw selected channel
    //v360.getTextureReference().drawSubsection(700, 0, selectedCanvas.getWidth(), selectedCanvas.getHeight(), selectedCanvas.x, selectedCanvas.y, selectedCanvas.getWidth(), 2160/2);
    if(canvasContentObject == PPI)
    {
        ppiImage.draw(700, 20, canvasW, canvasH);
    }
    
    if(canvasContentObject == REMOTE)
    {
        remoteViewer.draw(700, 20, canvasW, canvasH);
    }
    
    if(canvasContentObject == PANO)
    {
        panoFbo.getTextureReference().drawSubsection(700,
                                                     20,
                                                     selectedCanvas.getWidth(),
                                                     selectedCanvas.getHeight(),
                                                     selectedCanvas.x,
                                                     selectedCanvas.y,
                                                     selectedCanvas.getWidth(),
                                                     2160/2);
    }
    
    if(canvasContentObject == SMALL_CONTENT_1)
    {
        ppiSmall.draw(700, 20, canvasW, canvasH);
    }
    
    if(canvasContentObject == SMALL_CONTENT_2)
    {
        ppiSmall2.draw(700, 20, canvasW, canvasH);
    }
    
    // overlay
    if(overlayType == MARUSKA)
    {
        din.drawString("Josh Maruska", 850, 100);
    }
    
    if(overlayType == NOBLE)
    {
        din.drawString("Josh Noble", 850, 100);
    }
    
    if(overlayType == MEGAN)
    {
        din.drawString("Megan Geiman", 850, 100);
    }
    
    if(overlayType == JOHN)
    {
        din.drawString("John Mabry", 850, 100);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
    if( panoRect.inside(x, y))
    {
        selectedCanvas.x = ( x * ((VIDEO_WIDTH * 2)/SCREEN_WIDTH)  - (selectedCanvas.getWidth()/2));
        selectedCanvas.y = 0;
        
        if( abs(x - 269) < 100 )
        {
            overlayType = MARUSKA;
        }
        else if( abs(x - 937) < 100 )
        {
            overlayType = JOHN;
        }
        else if( abs(x - 1426) < 100 )
        {
            overlayType = MEGAN;
        }
        else if( abs(x - 1982) < 100 )
        {
            overlayType = NOBLE;
        }
        else
        {
            overlayType = NO_OVERLAY;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
    if(ppiRect.inside(x, y))
    {
        canvasContentObject = PPI;
    }
    
    if(remoteRect.inside(x,y))
    {
        canvasContentObject = REMOTE;
    }
    //smallRect2
    if(smallRect2.inside(x, y))
    {
        canvasContentObject = SMALL_CONTENT_2;
    }
    
    if(smallRect.inside(x, y))
    {
        canvasContentObject = SMALL_CONTENT_1;
    }
    
    if( panoRect.inside(x, y))
    {
        
        cout << (ofGetElapsedTimeMillis() - lastClick)  << endl;

        if(ofGetElapsedTimeMillis() - lastClick < 200)
        {
            drawOverlay = true;
            
            if( abs(x - 269) < 100 )
            {
                overlayType = MARUSKA;
            }
            else if( abs(x - 937) < 100 )
            {
                overlayType = JOHN;
            }
            else if( abs(x - 1426) < 100 )
            {
                overlayType = MEGAN;
            }
            else if( abs(x - 1982) < 100 )
            {
                overlayType = NOBLE;
            }
            else
            {
                overlayType = NO_OVERLAY;
            }

        }
        
        selectedCanvas.x = ( x * ((VIDEO_WIDTH * 2)/SCREEN_WIDTH)  - (selectedCanvas.getWidth()/2));
        selectedCanvas.y = 0;
        canvasContentObject = PANO;
    }
    
    lastClick = ofGetElapsedTimeMillis();
    
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
