#include "threesixtyUnwarp.h"
#include "ofGstVideoPlayer.h"

//--------------------------------------------------------------
void threesixtyUnwarp::setup(){
    
    drawCylinder = false;
	
	//---------------------------
	// app properties
	ofSetVerticalSync(false);
	bMousePressed   = false;
	bCenterChanged  = false;
	bPlayerPaused   = false;
	bAngularOffsetChanged = false;
	bMousePressedInPlayer = false;
	bMousepressedInUnwarped = false;
	bSavingOutVideo = false;
	bSaveAudioToo   = false;
	nWrittenFrames  = 0;
	handyString = new char[128];
	outputFileName = "output.mov";

	//---------------------------
	// Load settings file
	if( XML.loadFile("UnwarperSettings.xml") ){
		printf("UnwarperSettings.xml loaded!\n");
	} else{
		printf("Unable to load UnwarperSettings.xml!\nPlease check 'data' folder.\n");
	}
	
	//maxR_factor   = XML.getValue("MAXR_FACTOR", 0.96);
	//minR_factor   = XML.getValue("MINR_FACTOR", 0.16);
    maxR_factor   = XML.getValue("MAXR_FACTOR", 0.8);
    minR_factor   = XML.getValue("MINR_FACTOR", 0.2);

    angularOffset = XML.getValue("ROTATION_DEGREES", 0.0);
	
//	player.loadMovie(XML.getValue("INPUT_FILENAME", "input.mov"));
    
#ifndef GRABBER
    
    player.loadMovie(XML.getValue("INPUT_FILENAME", "untitled.mov"));
	player.getDuration();
    
#endif
	
	unwarpedW = (int) XML.getValue("OUTPUT_W", 1280);
	unwarpedH = (int) XML.getValue("OUTPUT_H", 256);
	
	//if the XML element doesn't exist, create it.
	XML.setValue("OUTPUT_W", (int) unwarpedW);
	XML.setValue("OUTPUT_H", (int) unwarpedH);
	
	
	// Interpolation method: 
	// 0 = CV_INTER_NN, 1 = CV_INTER_LINEAR, 2 = CV_INTER_CUBIC.
	interpMethod = (int) XML.getValue("INTERP_METHOD", 1); 
	XML.setValue("INTERP_METHOD", (int) interpMethod);
	
	int bSaveAud = (int) XML.getValue("INCLUDE_AUDIO", 0); 
	bSaveAudioToo = (bSaveAud != 0);

	yWarpA =   0.1850;
	yWarpB =   0.8184;
	yWarpC =  -0.0028;
	yWarpA = XML.getValue("R_WARP_A",  0.1850);
	yWarpB = XML.getValue("R_WARP_B",  0.8184);
	yWarpC = XML.getValue("R_WARP_C", -0.0028);


	//======================================
	// create data structures for unwarping
	blackOpenCV = cvScalarAll(0);
	
	// The "warped" original source video produced by the Bloggie.
	
#ifdef GRABBER
    warpedW = 1920;
    warpedH = 1080;
    
#else
    warpedW = player.width;
	warpedH = player.height;
#endif
    
    
	int nWarpedBytes = warpedW * warpedH * 3;
	printf("warpedW = %d, warpedH = %d\n", warpedW, warpedH);
	
	warpedImageOpenCV.allocate(warpedW, warpedH);
	warpedPixels = new unsigned char[nWarpedBytes];	
	warpedIplImage = warpedImageOpenCV.getCvImage();
	//cvSetImageROI(warpedIplImage, cvRect(0, 0, warpedW, warpedH));
    cvSetImageROI(warpedIplImage, cvRect(0, 0, warpedW, warpedH));
	
	int nUnwarpedPixels = unwarpedW * unwarpedH;
	int nUnwarpedBytes  = unwarpedW * unwarpedH * 3;
	unwarpedImage.allocate(unwarpedW, unwarpedH, OF_IMAGE_COLOR);
	//unwarpedPixels = new unsigned char[nUnwarpedBytes];
    unwarpedPixels.allocate(unwarpedW, unwarpedH, 3);
    
    ofTextureData texData;
    texData.width = unwarpedW;
    texData.height = unwarpedH;
    texData.bFlipTexture = true;
    
	unwarpedTexture.allocate(texData);
	
	unwarpedImageOpenCV.allocate(unwarpedW, unwarpedH);
	unwarpedImageOpenCV.setROI(0, 0, unwarpedW, unwarpedH);
	unwarpedIplImage = unwarpedImageOpenCV.getCvImage();
	
	srcxArrayOpenCV.allocate(unwarpedW, unwarpedH);
	srcyArrayOpenCV.allocate(unwarpedW, unwarpedH);
	srcxArrayOpenCV.setROI(0, 0, unwarpedW, unwarpedH);
	srcyArrayOpenCV.setROI(0, 0, unwarpedW, unwarpedH);
	
	xocvdata = (float*) srcxArrayOpenCV.getCvImage()->imageData; 
	yocvdata = (float*) srcyArrayOpenCV.getCvImage()->imageData; 
	
	playerScaleFactor = (float)(ofGetHeight() - unwarpedH)/(float)warpedH;
	savedWarpedCx = warpedCx = XML.getValue("CENTERX", warpedW / 2.0);
	savedWarpedCy = warpedCy = XML.getValue("CENTERY", warpedH / 2.0);
	savedAngularOffset = angularOffset;
	
	//if the XML element doesn't exist, create it.
	XML.setValue("CENTERX", warpedCx);
	XML.setValue("CENTERY", warpedCy);
	XML.setValue("ROTATION_DEGREES", angularOffset);
	
	
	//---------------------------
	// cylinder vizualization properties
    // NOTE => switching over to ofPrimitive
//	cylinderRes = 90;
//	cylinderWedgeAngle = 360.0 / (cylinderRes-1);
//	cylinderX = new float[cylinderRes];
//	cylinderY = new float[cylinderRes];
//	for (int i = 0; i < cylinderRes; i++) {
//		cylinderX[i] = cos(ofDegToRad((float)i * cylinderWedgeAngle));
//		cylinderY[i] = sin(ofDegToRad((float)i * cylinderWedgeAngle));
//	}
//	blurredMouseY = 0;
//	blurredMouseX = 0;

    /// set up the cylinder
    cylinder.set(unwarpedW, unwarpedH * 8, 120, 60, 0, false);
    cylinder.mapTexCoords(0, 0, unwarpedW, unwarpedH);
    
	//---------------------------
	// start it up
	computePanoramaProperties();
	computeInversePolarTransform();
    
#ifdef IMAGE
    //image.allocate(1920, 1080);
    image.loadImage("images/maxresdefault_640_320.jpg");

#elif defined(GRABBER)
    
    
    grabber.listDevices();
    grabber.setDeviceID(1);
    grabber.initGrabber(1920, 1080);
    
    
#else
	player.play();
#endif
    
    din.loadFont("fonts/FF_DIN_Pro_Light_Italic.otf", 32);
    
	
}

//--------------------------------------------------------------
void threesixtyUnwarp::exit(){
#ifndef IMAGE
	player.close();
#endif
}


//--------------------------------------------------------------
void threesixtyUnwarp::update(){
    
    ofSetVerticalSync(false);
    
#ifdef IMAGE
    
    if (bCenterChanged || bAngularOffsetChanged){
        XML.setValue("CENTERX", warpedCx);
        XML.setValue("CENTERY", warpedCy);
        XML.setValue("ROTATION_DEGREES", angularOffset);
        
        computePanoramaProperties();
        computeInversePolarTransform();
        
        bAngularOffsetChanged = false;
        bCenterChanged = false;
    }
    
    memcpy(warpedPixels, image.getPixels(), warpedW*warpedH*3);
    
    warpedIplImage->imageData = (char*) warpedPixels;
    
    cvRemap(warpedIplImage,  unwarpedIplImage,
            srcxArrayOpenCV.getCvImage(),
            srcyArrayOpenCV.getCvImage(),
            interpMethod | CV_WARP_FILL_OUTLIERS, blackOpenCV );
    
    unwarpedPixels = (unsigned char*) unwarpedIplImage->imageData;
    unwarpedImage.setFromPixels(unwarpedPixels, unwarpedW, unwarpedH, OF_IMAGE_COLOR, true);
    unwarpedTexture.loadData(unwarpedPixels, unwarpedW, unwarpedH, GL_RGB);
    
#elif defined(GRABBER)
    
    grabber.update();
    
    
    if (grabber.isFrameNew()){
        
        if (bCenterChanged || bAngularOffsetChanged){
            XML.setValue("CENTERX", warpedCx);
            XML.setValue("CENTERY", warpedCy);
            XML.setValue("ROTATION_DEGREES", angularOffset);
            
            computePanoramaProperties();
            computeInversePolarTransform();
            
            bAngularOffsetChanged = false;
            bCenterChanged = false;
        }
        
        memcpy(warpedPixels, grabber.getPixels(), warpedW*warpedH*3);
        
        warpedIplImage->imageData = (char*) warpedPixels;
        
        cvSetImageROI(warpedIplImage, cvRect(0, 0, warpedIplImage->width, warpedIplImage->height));
        
        //        float m[6];
        //        CvMat M = cvMat(2, 3, CV_32F, m);
        //        int w = warpedIplImage->width;
        //        int h = warpedIplImage->height;
        //
        //        float angle = 180.f;
        //
        //        float factor = (cos(angle*CV_PI/180.) + 1.05) * 2;
        //        m[0] = (float)(factor*cos(-angle*2*CV_PI/180.));
        //        m[1] = (float)(factor*sin(-angle*2*CV_PI/180.));
        //        m[3] = -m[1];
        //        m[4] = m[0];
        //        m[2] = w*0.5f;
        //        m[5] = h*0.5f;
        //
        //        cvGetQuadrangleSubPix( warpedIplImage, warpedIplImage, &M);
        
        
        
        //        double angle = -45.0, scale = 1.0;
        //
        //        float m[2];
        //        CvPoint2D32f center;
        //
        //        //CvMat center(warpedIplImage->width * 0.5, warpedIplImage->height * 0.5);
        //        center.x = warpedIplImage->width * 0.5;
        //        center.y = warpedIplImage->height * 0.5;
        //
        //        CvMat *rotation2d = cvCreateMat(2, 3, CV_32F);
        //
        //        CvMat *affine_matrix = cv2DRotationMatrix(center, angle, scale, rotation2d);
        //
        //        //cvWarpAffine(const CvArr* src, CvArr* dst, const CvMat* map_matrix, int flags=CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, CvScalar fillval=cvScalarAll(0) )
        //        cvWarpAffine(warpedIplImage, warpedIplImage, rotation2d);
        
        cvRemap(warpedIplImage,
                unwarpedIplImage,
                srcxArrayOpenCV.getCvImage(),
                srcyArrayOpenCV.getCvImage(),
                interpMethod | CV_WARP_FILL_OUTLIERS, blackOpenCV );
        
        unwarpedPixels.setFromPixels((unsigned char*) unwarpedIplImage->imageData, unwarpedIplImage->width, unwarpedIplImage->height, 3);
        unwarpedPixels.mirror(false, false);
        
        unwarpedImage.setFromPixels(unwarpedPixels.getPixels(), unwarpedW, unwarpedH, OF_IMAGE_COLOR, true);
        unwarpedTexture.loadData(unwarpedPixels.getPixels(), unwarpedW, unwarpedH, GL_RGB);
        
    }
    
    
#else
    player.update();
    
    if (player.isFrameNew()  || (bPlayerPaused && !player.isFrameNew())){
        
        if (bCenterChanged || bAngularOffsetChanged){
            XML.setValue("CENTERX", warpedCx);
            XML.setValue("CENTERY", warpedCy);
            XML.setValue("ROTATION_DEGREES", angularOffset);
            
            computePanoramaProperties();
            computeInversePolarTransform();
            
            bAngularOffsetChanged = false;
            bCenterChanged = false;
        }
        
        memcpy(warpedPixels, player.getPixels(), warpedW*warpedH*3);
        
        warpedIplImage->imageData = (char*) warpedPixels;
        
        cvRemap(warpedIplImage,
                unwarpedIplImage,
                srcxArrayOpenCV.getCvImage(),
                srcyArrayOpenCV.getCvImage(),
                interpMethod | CV_WARP_FILL_OUTLIERS, blackOpenCV );
        
        unwarpedPixels = (unsigned char*) unwarpedIplImage->imageData;

        unwarpedImage.setFromPixels(unwarpedPixels, unwarpedW, unwarpedH, OF_IMAGE_COLOR, true);
        unwarpedTexture.loadData(unwarpedPixels, unwarpedW, unwarpedH, GL_RGB);
    }
    
#endif
    
}



//=============================================
void threesixtyUnwarp::computePanoramaProperties(){
	
	//maxR_factor = 0.9 + 0.1*(float)mouseX/(float)ofGetWidth();
	//minR_factor = 0.2 + 0.1*(float)mouseY/(float)ofGetHeight();
	
	maxR  = warpedH * maxR_factor / 2;
	minR  = warpedH * minR_factor / 2;
}


//Used for the by hand portion and OpenCV parts of the shootout. 
//For the by hand, use the normal unwarpedW width instead of the step
//For the OpenCV, get the widthStep from the CvImage and use that for quarterstep calculation
//=============================================
void threesixtyUnwarp::computeInversePolarTransform(){

	// we assert that the two arrays have equal dimensions, srcxArray = srcyArray
	float radius, angle;
	float circFactor = 0 - TWO_PI/(float)unwarpedW;
	float difR = maxR-minR;
	int   dstRow, dstIndex;
	
	xocvdata = (float*) srcxArrayOpenCV.getCvImage()->imageData; 
	yocvdata = (float*) srcyArrayOpenCV.getCvImage()->imageData; 
	
	for (int dsty=0; dsty<unwarpedH; dsty++){
		float y = ((float)dsty/(float)unwarpedH);
		float yfrac = yWarpA*y*y + yWarpB*y + yWarpC;
		yfrac = MIN(1.0, MAX(0.0, yfrac)); 

		radius = (yfrac * difR) + minR;
		dstRow = dsty * unwarpedW; 	
		
		for (int dstx=0; dstx<unwarpedW; dstx++){
			dstIndex = dstRow + dstx;
			angle    = ((float)dstx * circFactor) + (DEG_TO_RAD * angularOffset);
			
			xocvdata[dstRow + dstx] = warpedCx + radius*cosf(angle);
			yocvdata[dstRow + dstx] = warpedCy + radius*sinf(angle);
		}
	}
	
	srcxArrayOpenCV.setFromPixels(xocvdata, unwarpedW, unwarpedH);
	srcyArrayOpenCV.setFromPixels(yocvdata, unwarpedW, unwarpedH);
}



//--------------------------------------------------------------
void threesixtyUnwarp::draw(){
	// draw everything.
	ofBackground(64,64,64);	
	
	drawPlayer();
	drawUnwarpedVideo();

    if(drawCylinder) {
        drawTexturedCylinder();
    }
}

//--------------------------------------------------------------
void threesixtyUnwarp::drawUnwarpedVideo(){
	// draw the unwarped (corrected) video in a strip at the bottom.
	ofSetColor(255, 255, 255);
    unwarpedImage.draw(0, ofGetHeight() - unwarpedH);
}

//--------------------------------------------------------------
void threesixtyUnwarp::drawPlayer(){
	
	
	// draw the (warped) player
	ofSetColor(255, 255, 255);
	playerScaleFactor = (float)(ofGetHeight() - unwarpedH)/(float)warpedH;
    
#if defined(IMAGE)
    
    //image.draw(0, 0, 320, 240);
    image.draw(0, 0, 640, 480);
    
#elif defined(GRABBER)
    
    grabber.draw(0, 0, playerScaleFactor*warpedW, playerScaleFactor*warpedH);
    
#else
    
    player.draw(0, 0, playerScaleFactor*warpedW, playerScaleFactor*warpedH);
    
#endif
	
	// draw the center point, as a red cross
	ofSetColor(255,0,0);
	glEnable(GL_LINE_SMOOTH);
	ofEnableAlphaBlending();
	float f = playerScaleFactor;
	ofLine((f*(warpedCx))-5, (f*(warpedCy)),   (f*(warpedCx))+5, (f*(warpedCy)));
	ofLine((f*(warpedCx))  , (f*(warpedCy))-5, (f*(warpedCx)),   (f*(warpedCy))+5);
	ofDisableAlphaBlending();
	glDisable(GL_LINE_SMOOTH);

}

//--------------------------------------------------------------
void threesixtyUnwarp::drawTexturedCylinder(){
	// draw the texture-mapped cylinder.

    float A = 0.90;
    float B = 1.0-A;
    blurredMouseX = A*blurredMouseX + B*mouseX;
    
    ofEnableDepthTest();
    ofPushMatrix();
        ofTranslate(ofGetWidth()/2, (ofGetHeight()/2), 100);
        ofRotateY(RAD_TO_DEG * ofMap(blurredMouseX, 0, ofGetWidth(),  TWO_PI, -TWO_PI));
        unwarpedTexture.bind();
        cylinder.draw();
        unwarpedTexture.unbind();
    ofPopMatrix();
    ofDisableDepthTest();
    
//    ofEnableAlphaBlending();
//    ofSetColor(255, 255, 255, 50);
//    ofRect(100, 0, ofGetWidth() - 200, ofGetHeight());
//    ofDisableAlphaBlending();
    
    ofSetColor(0, 0, 0);
    din.drawString("Call with Teague", 150, 100);
}


//--------------------------------------------------------------
void threesixtyUnwarp::keyPressed  (int key){ 
	
	/*
	<!-- // Press Space to toggle movie play.                      --> 
	<!-- // Press 's' to save the geometry settings.               -->
	<!-- // Press 'r' to reload the previously saved settings.     -->
	<!-- // Use the +/- keys to change the export codec.           -->
	<!-- // Press 'v' to export the unwarped video.                -->
	<!-- // Use the arrow keys to nudge the center point.          -->
	<!-- // Drag the unwarped video left or right to shift it.     -->
	 */

	
//	int nCodecs = videoRecorder->getNCodecs();
	
	switch (key){
		
			
		case '0':
		case '1':
		case '2':
			interpMethod = key - '0';
			break;
			
		case 356: // arrow left
			warpedCx -= 0.25;
			bCenterChanged = true;
			break;
		case 358: // arrow right
			warpedCx += 0.25;
			bCenterChanged = true;
			break;
		case 357: // arrow up
			warpedCy -= 0.25;
			bCenterChanged = true;
			break;
		case 359: // arrow down
			warpedCy += 0.25;
			bCenterChanged = true;
			break;
            
        case 'c':
                drawCylinder = !drawCylinder;
            break;
			
		case ' ':
			bPlayerPaused = !bPlayerPaused;
			player.setPaused(bPlayerPaused);
			break;
		
		case 'r':
		case 'R':
			warpedCx = savedWarpedCx;
			warpedCy = savedWarpedCy;
			angularOffset = savedAngularOffset;
			computeInversePolarTransform();
			break;
			
		case 's':
		case 'S':
			XML.setValue("CENTERX", warpedCx);
			XML.setValue("CENTERY", warpedCy);
			XML.setValue("ROTATION_DEGREES", angularOffset); 
			XML.saveFile("UnwarperSettings.xml");
			savedWarpedCx      = warpedCx;
			savedWarpedCy      = warpedCy;
			savedAngularOffset = angularOffset;
            cout << "Saved settings to UnwarperSettings.xml." << endl;
			break;
			
        case '{':
        case '[':
            maxR_factor += 0.02;
            XML.setValue("MAXR_FACTOR", maxR_factor);
            
            bCenterChanged = true;
            
            cout << "MAXR_FACTOR" << maxR_factor << endl;
            
            break;
        case '}':
        case ']':
            maxR_factor -= 0.02;
            XML.setValue("MAXR_FACTOR", maxR_factor);
            
            bCenterChanged = true;
            
            cout << "MAXR_FACTOR" << maxR_factor << endl;
            break;
            
        case ';':
        case ':':
            minR_factor += 0.02;
            XML.setValue("MINR_FACTOR", minR_factor);
            
            bCenterChanged = true;
            
            cout << "MINR_FACTOR" << minR_factor << endl;
            
            break;
            
        case '"':
        case '\'':
            minR_factor -= 0.02;
            XML.setValue("MINR_FACTOR", minR_factor);
            bCenterChanged = true;
            
            cout << "MINR_FACTOR" << minR_factor << endl;
            
            break;

//		
//		case 'v':
//		case 'V':
//			if (bSavingOutVideo == false){
//				player.setLoopState(OF_LOOP_NONE); //OF_LOOP_NORMAL
//				player.setPosition(0.0);
//				player.setPaused(true);
//				
//				string inName = XML.getValue("INPUT_FILENAME", "input.mov"); 
//				int inNameLastDotIndex = inName.find_last_of('.');
//				if (inNameLastDotIndex > 1){
//					inName = inName.substr (0, inNameLastDotIndex);
//				}
//				//sprintf(handyString, "%2d%2d%2d.mov", ofGetHours(), ofGetMinutes(), ofGetSeconds());
//                sprintf(handyString, "%2d%2d%2d.mov", ofGetHours(), ofGetMinutes(), ofGetSeconds());
//				inName += "_out_" + ofToString(ofGetHours()) + ofToString(ofGetMinutes()) + ofToString(ofGetSeconds()) + ".mov"; 
//				outputFileName = inName;
//				
//				videoRecorder->setup(unwarpedW, unwarpedH, outputFileName);
//				if (videoRecorder->bAmSetupForRecording()){
//					bSavingOutVideo = true;
//					nWrittenFrames = 0;
//				}
//			}
//			break;
		
	}
	
}



//--------------------------------------------------------------
void threesixtyUnwarp::mouseDragged(int x, int y, int button){
	bMousePressed = true;
	if (bMousePressedInPlayer){
		testMouseInPlayer();
	}
	if (bMousepressedInUnwarped && !bSavingOutVideo){
		angularOffset = ofMap(mouseX, 0, ofGetWidth(), 0-180, 180, false);
		bAngularOffsetChanged = true;
	}
}

//--------------------------------------------------------------
void threesixtyUnwarp::mousePressed(int x, int y, int button){
	bMousePressed         = true;
	bMousePressedInPlayer = testMouseInPlayer();
	
	bMousepressedInUnwarped = false;
	if (mouseY > (ofGetHeight() - unwarpedH)){
		bMousepressedInUnwarped = true;
	}
}

//--------------------------------------------------------------
void threesixtyUnwarp::mouseReleased(int x, int y, int button){
	if (bMousePressedInPlayer){
		testMouseInPlayer();
	}
	bMousepressedInUnwarped = false;
	bMousePressedInPlayer = false;
	bMousePressed = false;
}

//--------------------------------------------------------------
void threesixtyUnwarp::mouseMoved(int x, int y ){
	bMousepressedInUnwarped = false;
	bMousePressedInPlayer = false;
	bMousePressed = false;
}
//--------------------------------------------------------------
void threesixtyUnwarp::keyReleased(int key){ 
}

//--------------------------------------------------------------
void threesixtyUnwarp::windowResized(int w, int h){
}

//--------------------------------------------------------------
bool threesixtyUnwarp::testMouseInPlayer(){
	bool out = false;
	
	if ((mouseX < playerScaleFactor*warpedW) && 
		(mouseY < playerScaleFactor*warpedH)){
		
		if (bSavingOutVideo == false){
			float newCx = (float)mouseX * ((float)warpedH/(float)(ofGetHeight() - unwarpedH));
			float newCy = (float)mouseY * ((float)warpedH/(float)(ofGetHeight() - unwarpedH));	
			if ((newCx != warpedCx) || (newCy != warpedCy)){
				warpedCx = newCx;
				warpedCy = newCy;
			}
			bCenterChanged = true;
			out = true;
		}
	}
	return out;
}
