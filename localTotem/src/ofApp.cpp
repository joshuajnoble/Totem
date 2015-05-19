#include "ofApp.h"

#define LOGPOLAR

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
	//ofxSpout::init("", 640, 480, false);
	//small1.loadImage("meg.png");
	//small2.loadImage("matt.png");
	this->videoSourceCalibration.setFillFrame(false);
	auto ymlFullPath = ofToDataPath("undistort.yml");
	this->videoSourceCalibration.load(ymlFullPath);
	imitate(this->videSourceUnwrapped, *this->videoSource.get());
	this->videSourceUnwrapped.width *= 4;

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
		auto cvVideoSource = toCv(*this->videoSource);
		auto cvVideSourceUnwrapped = toCv(this->videSourceUnwrapped);

#ifdef LOGPOLAR
		CvMat source = cvVideoSource;
		CvMat dest = cvVideSourceUnwrapped;
		cvLinearPolar(&source, &dest, cvPoint2D32f(source.cols / 2, source.rows / 2), this->doubleM, CV_INTER_CUBIC/*| CV_WARP_INVERSE_MAP*/);
		//cvLogPolar(&source, &dest, cvPoint2D32f(source.cols / 2, source.rows / 2), this->doubleM / 10, CV_INTER_CUBIC/*| CV_WARP_INVERSE_MAP*/);
		cvVideSourceUnwrapped = cv::Mat(&dest, false);
#elif POLAR_MANUAL
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