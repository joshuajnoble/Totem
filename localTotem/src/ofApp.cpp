#include "ofApp.h"

#define JOSHNOBLE_UNWRAP

using namespace ofxCv;
using namespace cv;

namespace
{
	const int remoteVideoWidth = 480;
	int rotation = -90;
	int selectedScreen = -1;

#ifdef POLAR_MANUAL
	int rnd(double d)
	{
		return static_cast<int>(d + 0.5);
	}

	int rndf(float d)
	{
		return static_cast<int>(d + 0.5);
	}

	struct Options
	{
		int cx, cy, ri, ro;
		string input, output, filtered, test;
		bool interpolation, bilinear, bicubic;
		double sx, sy;
		int blackThr;
		int borderT, borderB, borderL, borderR;
		bool fixedCenter, estimateCenterThreshold, estimateCenterEdge;
		bool unwrap, createTest;
		int minRadius, maxRadius;
		int edgeCX, edgeCY, edgeDX, edgeDY;
	};

	void unwrap(const Options &opt, IplImage* inputImg, IplImage** outputImg)
	{
		// Create the unwrap image
		int uwWidth = static_cast<int>(ceil((opt.ro * 2.0 * PI)*opt.sx));
		int uwHeight = static_cast<int>(ceil((opt.ro - opt.ri + 1)*opt.sy));
		IplImage* unwrappedImg = cvCreateImage(cvSize(uwWidth, uwHeight), 8, 3);

		// Perform unwrapping
		for (int uwX = 0; uwX<uwWidth; ++uwX)
			for (int uwY = 0; uwY<uwHeight; ++uwY)
			{
				// Convert polar to cartesian
				double w = -static_cast<double>(uwX)*2.0*PI / static_cast<double>(uwWidth);
				double r = static_cast<double>(opt.ri) +
					static_cast<double>(uwHeight - uwY)*static_cast<double>(opt.ro - opt.ri + 1) / static_cast<double>(uwHeight);
				double iX = r*cos(w) + opt.cx;
				double iY = r*sin(w) + opt.cy;

				// Do safety check
				if ((iX<1) || (iX>inputImg->width - 2) || (iY<1) || (iY>inputImg->height - 2))
				{
					*(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 0) = 0;
					*(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 1) = 0;
					*(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 2) = 0;
				}
				else // Tansform image data
				{
					//if (opt.interpolation)
					//{ // With interpolation
					//	*reinterpret_cast<unsigned char *>(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 0) =
					//		getInterpolation(opt, inputImg, 0, iX, iY);
					//	*reinterpret_cast<unsigned char *>(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 1) =
					//		getInterpolation(opt, inputImg, 1, iX, iY);
					//	*reinterpret_cast<unsigned char *>(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 2) =
					//		getInterpolation(opt, inputImg, 2, iX, iY);
					//}
					//else
					{ // No interpolation
						int tmpX = rnd(iX);
						int tmpY = rnd(iY);
						*(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 0) =
							*(inputImg->imageData + tmpY*inputImg->widthStep + tmpX * 3 + 0);
						*(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 1) =
							*(inputImg->imageData + tmpY*inputImg->widthStep + tmpX * 3 + 1);
						*(unwrappedImg->imageData + uwY*unwrappedImg->widthStep + uwX * 3 + 2) =
							*(inputImg->imageData + tmpY*inputImg->widthStep + tmpX * 3 + 2);
					} // if
				} // if
			} // for

		// Return the unwrapped image
		(*outputImg) = unwrappedImg;
	}
#endif
}

//--------------------------------------------------------------
void ofApp::setup()
{
	rec.setup(8888);	

	// initialize Spout as a receiver
	//small1.loadImage("meg.png");
	//small2.loadImage("matt.png");

#if defined(JOSHNOBLE_UNWRAP)
	if (XML.loadFile("UnwarperSettings.xml")){
		//printf("UnwarperSettings.xml loaded!\n");
	}
	else{
		//printf("Unable to load UnwarperSettings.xml!\nPlease check 'data' folder.\n");
	}

	//maxR_factor   = XML.getValue("MAXR_FACTOR", 0.96);
	//minR_factor   = XML.getValue("MINR_FACTOR", 0.16);
	maxR_factor = XML.getValue("MAXR_FACTOR", 0.95);
	minR_factor = XML.getValue("MINR_FACTOR", 0.45);
	angularOffset = XML.getValue("ROTATION_DEGREES", 0.0);
	unwarpedW = (int)XML.getValue("OUTPUT_W", 2048);
	unwarpedH = (int)XML.getValue("OUTPUT_H", 512);
	// Interpolation method: 
	// 0 = CV_INTER_NN, 1 = CV_INTER_LINEAR, 2 = CV_INTER_CUBIC.
	interpMethod = (int)XML.getValue("INTERP_METHOD", 1);
	yWarpA = 0.1850;
	yWarpB = 0.8184;
	yWarpC = -0.0028;
	yWarpA = XML.getValue("R_WARP_A", 0.1850);
	yWarpB = XML.getValue("R_WARP_B", 0.8184);
	yWarpC = XML.getValue("R_WARP_C", -0.0028);

	//======================================
	// create data structures for unwarping
	blackOpenCV = cvScalarAll(0);
	warpedW = this->videoSource->getWidth();
	warpedH = this->videoSource->getHeight();

	int nWarpedBytes = warpedW * warpedH * 3;
	printf("warpedW = %d, warpedH = %d\n", warpedW, warpedH);

	warpedImageOpenCV.allocate(warpedW, warpedH);
	warpedPixels = new unsigned char[nWarpedBytes];
	warpedIplImage = warpedImageOpenCV.getCvImage();
	//cvSetImageROI(warpedIplImage, cvRect(0, 0, warpedW, warpedH));
	cvSetImageROI(warpedIplImage, cvRect(0, 0, warpedW, warpedH));

	int nUnwarpedPixels = unwarpedW * unwarpedH;
	int nUnwarpedBytes = unwarpedW * unwarpedH * 3;
	unwarpedImage.allocate(unwarpedW, unwarpedH, OF_IMAGE_COLOR);
	//unwarpedPixels = new unsigned char[nUnwarpedBytes];
	unwarpedPixels.allocate(unwarpedW, unwarpedH, 3);

	unwarpedImageOpenCV.allocate(unwarpedW, unwarpedH);
	unwarpedImageOpenCV.setROI(0, 0, unwarpedW, unwarpedH);
	unwarpedIplImage = unwarpedImageOpenCV.getCvImage();

	srcxArrayOpenCV.allocate(unwarpedW, unwarpedH);
	srcyArrayOpenCV.allocate(unwarpedW, unwarpedH);
	srcxArrayOpenCV.setROI(0, 0, unwarpedW, unwarpedH);
	srcyArrayOpenCV.setROI(0, 0, unwarpedW, unwarpedH);

	xocvdata = (float*)srcxArrayOpenCV.getCvImage()->imageData;
	yocvdata = (float*)srcyArrayOpenCV.getCvImage()->imageData;

	playerScaleFactor = (float)(ofGetHeight() - unwarpedH) / (float)warpedH;
	savedWarpedCx = warpedCx = XML.getValue("CENTERX", warpedW / 2.0);
	savedWarpedCy = warpedCy = XML.getValue("CENTERY", warpedH / 2.0);
	savedAngularOffset = angularOffset;

	computePanoramaProperties();
	computeInversePolarTransform();
#endif

	fbo.allocate(800, 480, GL_RGB);
	drawSecondRemote = false;
	remotePosition.set(100, 670);
	remoteScale.set(150, 120);
	mainPosition.set(-100, 10);
	mainScale.set(660, 660);

	this->isInitialized = true;
}

ofPtr<ofBaseVideoDraws> ofApp::InitializeVideoPresenterFromFile(std::string path)
{
	ofVideoPlayer* player = new ofVideoPlayer();
	ofPtr<ofBaseVideoDraws> rval = ofPtr<ofBaseVideoDraws>(player);
	if (player->loadMovie(path))
	{
		player->setLoopState(OF_LOOP_NORMAL);
		player->play();
	}

	return rval;
}

ofPtr<ofBaseVideoDraws> ofApp::InitializePlayerFromCamera(int deviceId, int width, int height)
{
	ofVideoGrabber *grabber = new ofVideoGrabber();
	ofPtr<ofVideoGrabber> rval = ofPtr<ofVideoGrabber>(grabber);
	if (deviceId != 0)
	{
		grabber->setDeviceID(deviceId);
	}

	grabber->initGrabber(width, height);
	return rval;
}

//--------------------------------------------------------------
void ofApp::exit()
{
}


//--------------------------------------------------------------
void ofApp::update()
{
	if (!this->isInitialized)
	{
		return;
	}

	this->videoSource->update();
	if (this->videoSource->isFrameNew())
	{
#if defined(JOSHNOBLE_UNWRAP)
		auto bCenterChanged = false;
		auto bAngularOffsetChanged = false;
		if (bCenterChanged || bAngularOffsetChanged)
		{
			XML.setValue("CENTERX", warpedCx);
			XML.setValue("CENTERY", warpedCy);
			XML.setValue("ROTATION_DEGREES", angularOffset);

			computePanoramaProperties();
			computeInversePolarTransform();

			bAngularOffsetChanged = false;
			bCenterChanged = false;
		}

		memcpy(warpedPixels, this->videoSource->getPixels(), warpedW*warpedH * 3);
		warpedIplImage->imageData = (char*)warpedPixels;

		cvSetImageROI(warpedIplImage, cvRect(0, 0, warpedIplImage->width, warpedIplImage->height));

		cvRemap(warpedIplImage,
			unwarpedIplImage,
			srcxArrayOpenCV.getCvImage(),
			srcyArrayOpenCV.getCvImage(),
			interpMethod | CV_WARP_FILL_OUTLIERS, blackOpenCV );

		unwarpedPixels.setFromPixels((unsigned char*) unwarpedIplImage->imageData, unwarpedIplImage->width, unwarpedIplImage->height, 3);
		unwarpedPixels.mirror(true, false);

		unwarpedImage.setFromPixels(unwarpedPixels.getPixels(), unwarpedW, unwarpedH, OF_IMAGE_COLOR, true);
		//unwarpedTexture.loadData(unwarpedPixels.getPixels(), unwarpedW, unwarpedH, GL_RGB);
#else
		auto cvVideoSource = toCv(*this->videoSource);
		auto cvVideSourceUnwrapped = toCv(this->videSourceUnwrapped);

#if defined(LOGPOLAR)
		CvMat source = cvVideoSource;
		CvMat dest = cvVideSourceUnwrapped;
		cvLinearPolar(&source, &dest, cvPoint2D32f(source.cols / 2, source.rows / 2), this->doubleM, CV_INTER_CUBIC/*| CV_WARP_INVERSE_MAP*/);
		//cvLogPolar(&source, &dest, cvPoint2D32f(source.cols / 2, source.rows / 2), this->doubleM / 10, CV_INTER_CUBIC/*| CV_WARP_INVERSE_MAP*/);
		cvVideSourceUnwrapped = cv::Mat(&dest, false);
#elif define(POLAR_MANUAL)
		//Options options;
		//options.cx = 1024;
		//options.cy = 1024;
		//options.ro = 0.5f;
		//options.ri = 0.5f;
		//options.sx = 2048;
		//options.sy = 2048;
		//unwrap(options, &cvVideoSource, &cvVideSourceUnwrapped);
#else
		copy(cvVideoSource, cvVideSourceUnwrapped);
#endif
		this->videSourceUnwrapped.update();
#endif
	}

	mainPlaylist.update();

	if (drawSecondRemote)
	{
		player->update();
	}

	//ofxSpout::initReceiver();
	//ofxSpout::receiveTexture();
	
	fbo.begin();
	ofBackground(0, 0, 0);
	ofPushMatrix();
	ofTranslate(0, 480);
	ofRotate(rotation);

	if (drawSecondRemote)
	{

		//ofxSpout::drawSubsection(0, 0, 400 * 1.33, 400, 100, 0, 960, 720);
		//player.getTextureReference().drawSubsection(0, 400, 500, 400, 0, 400, player.getWidth(), 800);

		//ofxSpout::drawSubsection(mainPosition.x, mainPosition.y, mainScale.x, mainScale.y, 0, 0, 960, 720);
		//player->getTextureReference().drawSubsection(remotePosition.x, remotePosition.y, remoteScale.x, remoteScale.y, 0, 400, player->getWidth(), 800);

		ofSetColor(0, 0, 0);
		ofRect(0, remotePosition.y, 480, 10);
		ofSetColor(255, 255, 255);
	}
	else
	{
		//ofxSpout::drawSubsection(-150, 10, 960 * 0.90, 660, 0, 0, 960, 720);
		//ofxSpout::draw(0, 0, 500, 500);
		//small1.draw(100, 670, 150, 120);
		//small2.draw(260, 670, 150, 120);
		//small3.draw(325, 540, 150, 120);
	}

	ofPopMatrix();

	// draw other UI here

	fbo.end();

	// check for waiting messages
	while (rec.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		rec.getNextMessage(&m);

		// check for mouse moved message
		if (m.getAddress() == "position"){
			// both the arguments are int32's
			selectedScreen = (m.getArgAsInt32(0) + 180) / 90;
		}

		if (m.getAddress() == "second_remote_on")
		{
			drawSecondRemote = true;

			mainPlaylist.addKeyFrame(Playlist::Action::tween(300.f, &remotePosition.x, 0));
			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remotePosition.y, 400));

			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remoteScale.x, 500));
			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &remoteScale.y, 400));

			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainPosition.x, -24));
			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainPosition.y, 0));

			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainScale.x, 528));
			mainPlaylist.addToKeyFrame(Playlist::Action::tween(300.f, &mainScale.y, 400));

			//player.play();
		}

		if (m.getAddress() == "second_remote_off")
		{
			drawSecondRemote = false;

			remotePosition.set(100, 670);
			remoteScale.set(150, 120);
			mainPosition.set(-150, 10);
			mainScale.set(960 * 0.90, 660);

			//player.stop();
			//player.setPosition(0);
		}
	}

}



//--------------------------------------------------------------
void ofApp::draw()
{
	if (!this->isInitialized)
	{
		return;
	}

	// draw everything.
	ofBackground(64, 64, 64);

	//grabber.draw(0, 0);

	//fbo.draw(0, 0);

	ofPushMatrix();
	//ofRotate(90);
	fbo.draw(0, 0);
	if (selectedScreen != 0 && selectedScreen != -1) {
		//ofRect(0, 0, 480, 480);
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 1 && selectedScreen != -1) {
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 2 && selectedScreen != -1) {
		ofSetColor(255, 255, 255, 255);
	}
	ofTranslate(800, 0);
	fbo.draw(0, 0);
	if (selectedScreen != 3 && selectedScreen != -1) {
		ofSetColor(255, 255, 255, 255);
	}
	ofPopMatrix();

	if (this->showInput)
	{
		this->videoSource->draw(0, 0);
	}
	else if (this->showUnwrapped)
	{
#if defined(JOSHNOBLE_UNWRAP)		
		ofSetColor(255, 255, 255);
		unwarpedImage.draw(0, 0);// ofGetHeight() - unwarpedH);
#elif defined(LOGPOLAR)
		ofPushMatrix();
		//this->videoSource->draw(0, 0);
		ofScale(0.25, 0.25);
		ofTranslate(1024, 1024);
		ofRotate(-90, 0, 0, 1);
		ofPushMatrix();
		ofTranslate(-1024, -1024);
		this->videSourceUnwrapped.draw(0, 0);
		ofPopMatrix();
		ofPopMatrix();
#endif
	}
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (!this->isInitialized)
	{
		return;
	}

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

	switch (key)
	{
	case '+':
		rotation += 1;
		break;
	case '-':
		rotation -= 1;
		break;
#ifdef LOGPOLAR
	case '[':
		doubleM -= 5;
		if (doubleM <= 5) doubleM = 5;
		break;
	case ']':
		doubleM += 5;
		break;
#endif
	}
}

void ofApp::onKeyframe(ofxPlaylistEventArgs& args)
{
	if (!this->isInitialized)
	{
		return;
	}
}

#if defined(JOSHNOBLE_UNWRAP)
//=============================================
void ofApp::computePanoramaProperties(){

	//maxR_factor = 0.9 + 0.1*(float)mouseX/(float)ofGetWidth();
	//minR_factor = 0.2 + 0.1*(float)mouseY/(float)ofGetHeight();

	maxR = warpedH * maxR_factor / 2;
	minR = warpedH * minR_factor / 2;
}


//Used for the by hand portion and OpenCV parts of the shootout. 
//For the by hand, use the normal unwarpedW width instead of the step
//For the OpenCV, get the widthStep from the CvImage and use that for quarterstep calculation
//=============================================
void ofApp::computeInversePolarTransform(){

	// we assert that the two arrays have equal dimensions, srcxArray = srcyArray
	float radius, angle;
	float circFactor = 0 - TWO_PI / (float)unwarpedW;
	float difR = maxR - minR;
	int   dstRow, dstIndex;

	xocvdata = (float*)srcxArrayOpenCV.getCvImage()->imageData;
	yocvdata = (float*)srcyArrayOpenCV.getCvImage()->imageData;

	for (int dsty = 0; dsty<unwarpedH; dsty++){
		float y = ((float)dsty / (float)unwarpedH);
		float yfrac = yWarpA*y*y + yWarpB*y + yWarpC;
		yfrac = std::min(1.0f, std::max(0.0f, yfrac));

		radius = (yfrac * difR) + minR;
		dstRow = dsty * unwarpedW;

		for (int dstx = 0; dstx<unwarpedW; dstx++){
			dstIndex = dstRow + dstx;
			angle = ((float)dstx * circFactor) + (DEG_TO_RAD * angularOffset);

			xocvdata[dstRow + dstx] = warpedCx + radius*cosf(angle);
			yocvdata[dstRow + dstx] = warpedCy + radius*sinf(angle);
		}
	}

	srcxArrayOpenCV.setFromPixels(xocvdata, unwarpedW, unwarpedH);
	srcyArrayOpenCV.setFromPixels(yocvdata, unwarpedW, unwarpedH);
}
#endif