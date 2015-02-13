#include "ofApp.h"


#ifndef LIVE_VIDEO
const int VIDEO_WIDTH = 3240;
const int VIDEO_HEIGHT = 2160;
#else
const int VIDEO_WIDTH = 1920;
const int VIDEO_HEIGHT = 1080;
#endif

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

    panoRect.set( 0, (SCREEN_HEIGHT*0.6), SCREEN_WIDTH, SCREEN_HEIGHT - (SCREEN_HEIGHT * 0.5));
    ppiRect.set( 0, 300, ratio1x2W, ratio1x2H );
    remoteRect.set( 0, 0, ratio1x2W, ratio1x2H );
    
    smallRect2.set( ratio1x1H, ratio1x2H * 2, ratio1x1H, ratio1x1H );
    smallRect.set( 0, ratio1x2H * 2, ratio1x1H, ratio1x1H );
    
    selectedCanvas.set( 0, 0, canvasW, canvasH );
    
    
#ifndef LIVE_VIDEO
    
	v360.loadMovie("movies/v360_capture1.mp4");
	v360.play();
#else
    v360.listDevices();
    v360.setDeviceID(2);
    v360.initGrabber(VIDEO_WIDTH, VIDEO_HEIGHT);
#endif
    
    remoteViewer.loadMovie("movies/TalkingtoJosh.mov");
    remoteViewer.play();
    
#ifndef LIVE_VIDEO
    
    panoFbo.allocate(VIDEO_WIDTH*2, VIDEO_HEIGHT/2);
#else
    panoFbo.allocate(SCREEN_WIDTH, 640);
#endif
    
    ppiSmall.loadImage("images/ppi_image_small.jpg");
    ppiSmall2.loadImage("images/ppi_image_small2.png");
    
    remote2.loadMovie("movies/ppi2.mov");
    remote2.play();
    
    for( int i = 1; i < 11; i++ )
    {
        stringstream ss;
        ss << "images/slides/" << i << ".png";
        ppiImage[i].loadImage(ss.str());
    }
    
    lastClick = ofGetElapsedTimeMillis();
    
    din.loadFont("fonts/FF_DIN_Pro_Light_Italic.otf", 36);
    currentSlide = 0;
}

//--------------------------------------------------------------
void ofApp::update(){
    v360.update();
    remoteViewer.update();
    remote2.update();
    
    double twenty = 20.0;
    if( modf(ofGetElapsedTimef(), &twenty) == 0 )
    {
        currentSlide++;
        if(currentSlide > 10) {
            currentSlide = 0;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetHexColor(0xFFFFFF);
    
#ifndef LIVE_VIDEO
    const float panoYPosition = (SCREEN_HEIGHT*0.62 );
#else
    const float panoYPosition = (SCREEN_HEIGHT*0.58 );
#endif
    const float screenRatioW = 0.31578947368;
    
    // draw viewer
    remoteViewer.getTextureReference().drawSubsection(10, 10, ratio1x2W, ratio1x2H, 0, 0, 1180, 680);
    
    // draw images
    ppiImage[currentSlide].draw( 10, 400, ratio1x2W, ratio1x2H );
    remote2.draw(10, 10 + ratio1x1H * 2, ratio1x2W, ratio1x2H);
    
    //ppiSmall.draw( 10, 10 + ratio1x1H * 2, ratio1x1W, ratio1x1H );
    //ppiSmall2.draw( ratio1x1W + 20, 10 + ratio1x1H * 2, ratio1x1W, ratio1x1H );
    
    // draw selected channel
    //v360.getTextureReference().drawSubsection(700, 0, selectedCanvas.getWidth(), selectedCanvas.getHeight(), selectedCanvas.x, selectedCanvas.y, selectedCanvas.getWidth(), 2160/2);
    if(canvasContentObject == PPI)
    {
        ppiImage[currentSlide].draw(700, 20, canvasW, canvasH);
    }
    
    if(canvasContentObject == REMOTE2)
    {
        remote2.draw(700, 20, canvasW, canvasH);
    }
    
    if(canvasContentObject == REMOTE)
    {
        remoteViewer.draw(700, 20, canvasW, canvasH);
    }
    
    if(canvasContentObject == PANO)
    {
        
#ifndef LIVE_VIDEO
        
        panoFbo.getTextureReference().drawSubsection(700,
                                                     20,
                                                     selectedCanvas.getWidth(),
                                                     selectedCanvas.getHeight(),
                                                     selectedCanvas.x,
                                                     selectedCanvas.y,
                                                     selectedCanvas.getWidth(),
                                                     2160/2);
        
#else
      
        panoFbo.getTextureReference().drawSubsection(700,
                                                     20,
                                                     selectedCanvas.getWidth(),
                                                     selectedCanvas.getHeight(),
                                                     selectedCanvas.x,
                                                     selectedCanvas.y,
                                                     selectedCanvas.getWidth(),
                                                     640);
        if( selectedCanvas.x < 0 )
        {
            panoFbo.getTextureReference().drawSubsection(700,
                                                         20,
                                                         -1 * selectedCanvas.x,
                                                         selectedCanvas.getHeight(),
                                                         SCREEN_WIDTH + selectedCanvas.x,
                                                         selectedCanvas.y,
                                                         -1 * selectedCanvas.x,
                                                         640);
        }
        
        if( selectedCanvas.x > (VIDEO_WIDTH - selectedCanvas.getWidth()) )
        {
            
            cout
            << (700 + (VIDEO_WIDTH - selectedCanvas.x)) << " " <<
            (VIDEO_WIDTH - selectedCanvas.x) << " " <<
            selectedCanvas.getWidth() - (VIDEO_WIDTH - selectedCanvas.x) << " " << endl;
            
            panoFbo.getTextureReference().drawSubsection(700 + (SCREEN_WIDTH - selectedCanvas.x),
                                                         20,
                                                         selectedCanvas.getWidth(),//(VIDEO_WIDTH - selectedCanvas.x),
                                                         selectedCanvas.getHeight(),
                                                         0,
                                                         selectedCanvas.y,
                                                         selectedCanvas.getWidth(),// - (VIDEO_WIDTH - selectedCanvas.x),
                                                         640);
        }
        
#endif
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
    
    // draw pano
    panoFbo.begin();
    
    
#ifndef LIVE_VIDEO
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
#else
    
    v360.getTextureReference().drawSubsection(0, 0, SCREEN_WIDTH, 600, 0, (VIDEO_HEIGHT/2)-160, 1920, 320);
    panoFbo.end();
    panoFbo.draw(0, panoYPosition);
#endif
    
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
        
#ifndef LIVE_VIDEO
        
        selectedCanvas.x = ( x * ((VIDEO_WIDTH * 2)/SCREEN_WIDTH)  - (selectedCanvas.getWidth()/2));
#else
        selectedCanvas.x = ( x - (selectedCanvas.getWidth()/2));
        
#endif
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
//    if(smallRect2.inside(x, y))
//    {
//        canvasContentObject = SMALL_CONTENT_2;
//    }
//    
//    if(smallRect.inside(x, y))
//    {
//        canvasContentObject = SMALL_CONTENT_1;
//    }

    if(smallRect2.inside(x, y) || smallRect.inside(x, y))
    {
        canvasContentObject = REMOTE2;
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
        
#ifndef LIVE_VIDEO
        
        selectedCanvas.x = ( x * ((VIDEO_WIDTH * 2)/SCREEN_WIDTH)  - (selectedCanvas.getWidth()/2));
#else
        selectedCanvas.x = ( x - (selectedCanvas.getWidth()/2));
        
#endif
        
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
