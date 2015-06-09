#include "ofRemoteApp.h"

#define CYLINDER_PIECE_WIDTH 44
#define CYLINDER_PIECE_HEIGHT 2
#define PIECE_TEXCOORD_WIDTH 720
#define NEO_PIXELS_COUNT 45

namespace
{
	float lastElapsed;
	float lastSentMouseLocation;
}

//--------------------------------------------------------------
void ofRemoteApp::earlyinit()
{
	this->cylinderDisplay = CylinderDisplay();
	this->cylinderDisplay.initCylinderDisplay(1920, 1080);
}


//--------------------------------------------------------------
void ofRemoteApp::setup()
{
	this->cylinderDisplay.allocateBuffers();

//	showInstructions = false;
//
//	blurredMouseX = 0;
//
//	warpedW = 1920;
//	warpedH = 1080;
//
//	/// set up the cylinder
//	cylinder.set(warpedW, warpedH * 6, 120, 60, 0, false);
//	cylinder.mapTexCoords(0, 0, warpedW, warpedH);
//	//cylinder.mapTexCoords(warpedW, 0, 0, warpedH);
//
//	//rightCylinderPiece.setMode(ofPrimitiveMode::OF_PRIMITIVE_TRIANGLE_STRIP  );
//
//	createCylinderPiece(leftCylinderPiece, warpedW, 1080 * 2, CYLINDER_PIECE_WIDTH);
//	createCylinderPiece(rightCylinderPiece, warpedW, 1080 * 2, CYLINDER_PIECE_WIDTH);
//
//	isDrawingLeftCylinder = false;
//	isDrawingRightCylinder = false;
//
//	isDoingMicDetection = false;
//	isDrawingSecondRemote = false;
//
//	sender.setup("localhost", 8888);
//
//	//ofxSpout::init("", ofGetWidth(), ofGetHeight(), false);
//
//#ifndef MIC_DEBUGGING
//	vector<ofVideoDevice> dd = grabber.listDevices();
//	auto deviceId = 0;
//	for (int i = 0; i < dd.size(); i++)
//	{
//		if (dd.at(i).deviceName.find("XI100DUSB") != string::npos)
//		{
//			deviceId = i;
//			break;
//		}
//	}
//
//	grabber.setDeviceID(deviceId);
//	grabber.initGrabber(1920, 1080, true);
//
//
//	if (!grabber.isInitialized())
//	{
//		ofExit();
//	}
//
//#else
//
//	panoView.loadImage("panoView.png");
//
//#endif
//
//	//din.loadFont("fonts/FF_DIN_Pro_Light_Italic.otf", 32);
//	gradient.loadImage("CylinderUI_Grad-01.png");
//	//overlay.loadImage("CylinderUI-01.png");
//
//	serial.setup("/COM3", 115200);
//
//	finder.setup("haarcascade_mcs_upperbody.xml");
//	finder.setPreset(ofxCv::ObjectFinder::Accurate);
//	finder.setMaxSizeScale(3.0);
//
//	lastElapsed = ofGetElapsedTimef();
//	navState = USER_CONTROL;
//	userDisplayState = UDS_SHOW_FEED;
//
//	cam.enableMouseInput();
//	cam.enableMouseMiddleButton();
//	gui.setup(); // most of the time you don't need a name
//
//	gui.add(xPosition.setup("x pos", 0, -2000, 1000));
//	gui.add(yPosition.setup("y pos", 0, -200, 1000));
//	gui.add(zPosition.setup("z pos", 0, -2000, 1000));
//	gui.add(rotation.setup("rot", 0, 0, 360));
//
//	lastSentMouseLocation = ofGetElapsedTimef();
//
//	remotePlayer.loadMovie("IMG_1628.mov");
//	remotePlayer.setLoopState(ofLoopType::OF_LOOP_NORMAL);
//
//	remoteCaller.loadImage("meg.png");
//	remoteCaller1.loadImage("matt.png");
//	remoteCaller2.loadImage("josh.png");
//
//	memset(&detectedMicrophone[0], 0, sizeof(int) * 6);
}


//--------------------------------------------------------------
void ofRemoteApp::update()
{
	this->videoSource->update();
	if (this->videoSource->isFrameNew())
	{
		this->cylinderDisplay.update();
	}

	//mainPlaylist.update();
	ofSetVerticalSync(false);

//#ifndef MIC_DEBUGGING
//	grabber.update();
//
//	if (grabber.isFrameNew()){
//	}
//
//#endif
//
//	if (isDrawingSecondRemote)
//	{
//		remotePlayer.update();
//	}
//	int nRead = 0;
//
//	unsigned char bytesReturned[2];
//	// if there's two bytes to be read, read until you get them both
//	if (serial.isInitialized() && serial.readBytes(bytesReturned, 1) > 0)
//	{
//		if (navState == SYSTEM_CONTROL)
//		{
//			if (bytesReturned[0] > '1' && bytesReturned[0] < '8')
//			{
//				//blurredMouseX = (int(bytesReturned[0] - '2') * 59);
//				//cout << bytesReturned[0] << " " << int(bytesReturned[0] - '2') << " " << blurredMouseX << endl;
//				int index = int(bytesReturned[0] - '2');
//				detectedMicrophone[index] += 60;
//
//				int highestIndex = -1, highestValue = -1;
//				for (int i = 0; i < 6; i++){
//					if (detectedMicrophone[i] > highestValue) {
//						highestValue = detectedMicrophone[i];
//						highestIndex = i;
//					}
//				}
//
//				float rotation = (highestIndex * 59);
//				if (rotation < 0) { rotation += 360; }
//
//				// trying to send position
//				//mainPlaylist.addKeyFrame(Playlist::Action::tween(300.f, &rotateToPosition, rotation));
//
//				//int led = NEO_PIXELS_COUNT - ofMap(rotation, -180, 180, 0, NEO_PIXELS_COUNT, false);
//
//				//unsigned char val[4];
//				//stringstream ss;
//				//ss << '2' << led;
//				//strncpy((char*)&val[0], ss.str().c_str(), sizeof(val));
//				//serial.writeBytes(&val[0], 4);
//				// end saving position
//
//				cout << "changing from mic " << bytesReturned[0] << " " << index << " " << rotation << " " << rotateToPosition << endl;
//
//			}
//		}
//	};
//
//	if (navState == USER_CONTROL)
//	{
//		userControlCountdown -= (ofGetElapsedTimef() - lastElapsed);
//		lastElapsed = ofGetElapsedTimef();
//
//		if (userControlCountdown < 0)
//		{
//			cout << " should be system control  " << endl;
//			navState = SYSTEM_CONTROL;
//
//		}
//	}
//
//	if (isDoingMicDetection)
//	{
//		// decay
//		for (int i = 0; i < 6; i++){
//			if (detectedMicrophone[i] > 0) {
//				detectedMicrophone[i] -= 1;
//			}
//		}
//	}
}


//--------------------------------------------------------------
void ofRemoteApp::draw()
{
	ofBackground(11,26,38);
	cylinderDisplay.draw();

//
//	if (drawCylinder) {
//
//		ofEnableDepthTest();
//		ofPushMatrix();
//
//#ifdef MIC_DEBUGGING
//
//		ofTranslate(ofGetWidth() / 2, (ofGetHeight() / 2), 100);
//		ofRotateY(rotateToPosition);
//		panoView.getTextureReference().bind();
//		cylinder.draw();
//		panoView.getTextureReference().unbind();
//
//#else
//		ofTranslate(ofGetWidth() / 2, (ofGetHeight() / 2), 100);
//		ofRotateY(rotateToPosition);
//		grabber.getTextureReference().bind();
//		cylinder.draw();
//		grabber.getTextureReference().unbind();
//#endif
//		ofPopMatrix();
//		ofDisableDepthTest();
//
//		gradient.draw(0, 0, 1920, 1280);
//		//overlay.draw(-120, -50, 1920, 1280);
//
//		if (isDrawingLeftCylinder)
//		{
//			ofPushMatrix();
//			ofTranslate(430, (ofGetHeight() / 2), 300);
//			ofRotateY(135);
//			drawLeftCylinder();
//			ofPopMatrix();
//		}
//
//		if (isDrawingRightCylinder)
//		{
//			ofPushMatrix();
//			ofTranslate(1200, (ofGetHeight() / 2), 100);
//			ofRotateY(80);
//			drawRightCylinder();
//			ofPopMatrix();
//		}
//
		const int iconYPosition = 920;

		//if (isDrawingSecondRemote)
		//{
		//	//remotePlayer.draw(1300, 100, (1080/2), (1920/2));
		//	remotePlayer.getTextureReference().drawSubsection(1160, 210, 660, 660, 0, 0, 1080, 1080);
		//	ofSetColor(255, 255, 255, 255);
		//	remoteCaller.draw((ofGetWidth() / 2), iconYPosition, 150, 120);
		//	ofSetColor(255, 255, 255, 150);
		//	remoteCaller2.draw((ofGetWidth() / 2) + 160, iconYPosition, 150, 120);
		//	remoteCaller1.draw((ofGetWidth() / 2) + 320, iconYPosition, 150, 120);
		//	ofSetColor(255, 255, 255, 255);
		//}
		//else
		//{
		//	ofSetColor(255, 255, 255, 150);
		//	remoteCaller2.draw((ofGetWidth() / 2) + 160, iconYPosition, 150, 120);
		//	remoteCaller1.draw((ofGetWidth() / 2) + 320, iconYPosition, 150, 120);
		//	remoteCaller.draw((ofGetWidth() / 2), iconYPosition, 150, 120);
		//	ofSetColor(255, 255, 255, 255);
		//}
//
//	}
//	else {
//		drawPlayer();
//	}

	// init receiver if it's not already initialized
	//ofxSpout::initReceiver();

	// receive Spout texturen
	//ofxSpout::receiveTexture();
	//ofxSpout::draw( (ofGetWidth()/2) - 150, 0, 300, 210);

	DrawSelfie();

	if (false)// showInstructions)
	{
		//ofDrawBitmapString(" ?: show instructions \n z: show remote caller \n m: turn on directional mics \n l: look for the left-most participant \n r: look for the right most participant \n space: show the raw v360 feed", 50, 50);
	}
}

void ofRemoteApp::DrawSelfie()
{
	ofPushMatrix();
	ofScale(this->scale, this->scale);

	ofSetColor(255);

	// Draw the source video in a small window
	auto ratio = this->videoSource->getWidth() / this->videoSource->getHeight();
	int selfieWidth = 320;
	int selfieHeight = selfieWidth / ratio;
	ofSetColor(32); // Selfie margin colors
	int selfieX = (this->width - selfieWidth) / 2;
	int selfieY = 40;
	int margin = 15;
	ofRect(selfieX - margin, selfieY - margin, selfieWidth + margin * 2, selfieHeight + margin * 2);
	ofSetColor(255);
	this->videoSource->draw(selfieX, selfieY, selfieWidth, selfieHeight);

	ofPopMatrix();
}

//--------------------------------------------------------------
void ofRemoteApp::exit()
{
}


//--------------------------------------------------------------
int ofRemoteApp::displayWidth() const
{
	return this->width * this->scale;
}


//--------------------------------------------------------------
int ofRemoteApp::displayHeight() const
{
	return this->height * this->scale;
}


void ofRemoteApp::RegisterTotemVideoSource(ofPtr<ofBaseVideoDraws> source)
{
	this->remoteTotemSource = source;
	this->cylinderDisplay.setTotemVideoSource(this->remoteTotemSource);
}