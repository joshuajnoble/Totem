#include "ofApp.h"

float lastElapsed;
#define CYLINDER_PIECE_WIDTH 44
#define CYLINDER_PIECE_HEIGHT 2

#define PIECE_TEXCOORD_WIDTH 720

float lastSentMouseLocation;

void ofApp::setup(){

	blurredMouseX = 0;
    
    warpedW = 1920;
    warpedH = 1080;

    /// set up the cylinder
	cylinder.set(warpedW, warpedH * 6, 120, 60, 0, false);
	cylinder.mapTexCoords(0, 0, warpedW, warpedH);
	//cylinder.mapTexCoords(warpedW, 0, 0, warpedH);

	//rightCylinderPiece.setMode(ofPrimitiveMode::OF_PRIMITIVE_TRIANGLE_STRIP  );

	createCylinderPiece(leftCylinderPiece, warpedW, 1080 * 2, CYLINDER_PIECE_WIDTH);
	createCylinderPiece(rightCylinderPiece, warpedW, 1080 * 2, CYLINDER_PIECE_WIDTH);

	isDrawingLeftCylinder = false;
	isDrawingRightCylinder = false;

	isDoingMicDetection = true;
	isDrawingSecondRemote = false;
	isDoingMicDetection = false;

	sender.setup("localhost", 8888);
	
	ofxSpout::init("", ofGetWidth(), ofGetHeight(), false);

    vector<ofVideoDevice> dd = grabber.listDevices();

#ifndef MIC_DEBUGGING

	for (int i = 0; i < dd.size(); i++)
	{
		if (dd.at(i).deviceName.find("XI100DUSB") != string::npos)
		{
			grabber.setDeviceID(i);
			grabber.initGrabber(1920, 1080, true);
		}
	}


	if (!grabber.isInitialized())
	{
		ofExit();
	}

#else

	panoView.loadImage("panoView.png");

#endif
 
    //din.loadFont("fonts/FF_DIN_Pro_Light_Italic.otf", 32);
	gradient.loadImage("CylinderUI_Grad-01.png");
	//overlay.loadImage("CylinderUI-01.png");
	
	serial.setup("/COM6", 115200);

	finder.setup("haarcascade_mcs_upperbody.xml");
	finder.setPreset(ofxCv::ObjectFinder::Accurate);
	finder.setMaxSizeScale(3.0);

	lastElapsed = ofGetElapsedTimef();
	navState = USER_CONTROL;
	userDisplayState = UDS_SHOW_FEED;

	cam.enableMouseInput();
	cam.enableMouseMiddleButton();
	gui.setup(); // most of the time you don't need a name

	gui.add(xPosition.setup("x pos", 0, -2000, 1000));
	gui.add(yPosition.setup("y pos", 0, -200, 1000));
	gui.add(zPosition.setup("z pos", 0, -2000, 1000));
	gui.add(rotation.setup("rot", 0, 0, 360));

	lastSentMouseLocation = ofGetElapsedTimef();

	remotePlayer.loadMovie("IMG_1628.mov");
	remotePlayer.setLoopState(ofLoopType::OF_LOOP_NORMAL);
	remotePlayer.play();

	memset(&detectedMicrophone[0], 0, sizeof(int) * 6);
}

void ofApp::exit()
{
	grabber.close();
}


void ofApp::update(){
    
    ofSetVerticalSync(false);  
    
#ifndef MIC_DEBUGGING
	grabber.update();
    
    if (grabber.isFrameNew()){
	}

#endif

	if (isDrawingSecondRemote)
	{
		remotePlayer.update();
	}
	int nRead = 0;

	unsigned char bytesReturned[2];
	// if there's two bytes to be read, read until you get them both
	if (serial.isInitialized() && serial.readBytes(bytesReturned, 1) > 0)
	{
		if (navState == SYSTEM_CONTROL)
		{
			if (bytesReturned[0] > '1' && bytesReturned[0] < '8')
			{
				//blurredMouseX = (int(bytesReturned[0] - '2') * 59);
				//cout << bytesReturned[0] << " " << int(bytesReturned[0] - '2') << " " << blurredMouseX << endl;
				detectedMicrophone[int(bytesReturned[0] - '2')] += 60;

				int highestIndex = -1, highestValue = -1;
				for (int i = 0; i < 6; i++){
					if (detectedMicrophone[i] > highestValue) {
						highestValue = detectedMicrophone[i];
						highestIndex = i;
					}
				}

				blurredMouseX = (highestIndex * 59);

				cout << "changing from mic " << bytesReturned[0] << " " << int(bytesReturned[0] - '2') << " " << blurredMouseX << endl;

			}
		}
	};
    
	if (navState == USER_CONTROL)
	{
		userControlCountdown -= (ofGetElapsedTimef() - lastElapsed);
		lastElapsed = ofGetElapsedTimef();

		if (userControlCountdown < 0)
		{
			cout << " should be system control  " << endl;
			navState = SYSTEM_CONTROL;

		}
	}

	if (isDoingMicDetection)
	{
		// decay
		for (int i = 0; i < 6; i++){
			if (detectedMicrophone[i] > 0) {
				detectedMicrophone[i] -= 1;
			}
		}
	}

}


void ofApp::draw(){
	// draw everything.
	ofBackground(64,64,64);	
	
    if(drawCylinder) {
	
		ofEnableDepthTest();
		ofPushMatrix();

#ifdef MIC_DEBUGGING
		
		ofTranslate(ofGetWidth() / 2, (ofGetHeight() / 2), 100);
		ofRotateY(blurredMouseX);
		panoView.getTextureReference().bind();
		cylinder.draw();
		panoView.getTextureReference().unbind();

#else
		ofTranslate(ofGetWidth() / 2, (ofGetHeight() / 2), 100);
		ofRotateY(blurredMouseX);
		grabber.getTextureReference().bind();
		cylinder.draw();
		grabber.getTextureReference().unbind();
#endif
		ofPopMatrix();
		ofDisableDepthTest();

		gradient.draw(0, 0, 1920, 1280);
		//overlay.draw(-120, -50, 1920, 1280);

		if (isDrawingLeftCylinder)
		{
			ofPushMatrix();
			ofTranslate(430, (ofGetHeight() / 2), 300);
			ofRotateY(135);
			drawLeftCylinder();
			ofPopMatrix();

			/*cam.begin();
			drawLeftCylinder();
			cam.end();*/
		}
		if (isDrawingRightCylinder)
		{
			ofPushMatrix();
			ofTranslate(1200, (ofGetHeight() / 2), 100);
			ofRotateY(80);
			drawRightCylinder();
			ofPopMatrix();
		}

		if (isDrawingSecondRemote)
		{
			remotePlayer.draw(1400, 200);
		}

	}
	else {
		drawPlayer();
	}

	// init receiver if it's not already initialized
	ofxSpout::initReceiver();

	// receive Spout texturen
	ofxSpout::receiveTexture();
	ofxSpout::draw(600, 0, 300, 210);

	gui.draw();
}

void ofApp::drawPlayer(){
	ofSetColor(255, 255, 255);

#ifdef MIC_DEBUGGING

    panoView.draw(0, 0, 1920, 1080);

#else

	grabber.draw(0, 0, 1920, 1080);

#endif

	for (int i = 0; i < finder.size(); i++)
	{
		ofNoFill();
		ofRect(finder.getObject(i).x, 400, finder.getObject(i).width, 400);
	}
}

void ofApp::findLeftFace()
{
	finder.clear();


#ifdef MIC_DEBUGGING

	panoView.getPixelsRef().cropTo(cropped, 0, 380, 1920, 700);
	finder.update(panoView.getPixelsRef());

#else

	grabber.getPixelsRef().cropTo(cropped, 0, 380, 1920, 700);
	finder.update(cropped);
	
#endif

	ofRectangle leftMost(1920, 0, 1920, 0);


	for (int i = 0; i < finder.size(); i++) {
		ofRectangle object = finder.getObjectSmoothed(i);
		if (object.getTopLeft().x < leftMost.getTopLeft().x && object.x > 0) {
			leftMost = object;

			cout << leftMost << endl;
		}
	}

	if (leftMost.x > 1920) {
		leftMost.x = 1920;
	}
	//mapTexCoords(leftCylinderPiece, leftMost.x + 100, 380, leftMost.x + PIECE_TEXCOORD_WIDTH - 100, 700);
	mapTexCoords(leftCylinderPiece, max(400, min(1920, (int)leftMost.x + 200)), 380, 0, 700);
}

void ofApp::findRightFace()
{
	//finder.findHaarObjects(grabber.getPixels());


#ifdef MIC_DEBUGGING

	panoView.getPixelsRef().cropTo(cropped, 0, 380, 1920, 700);
	finder.update(panoView.getPixelsRef());

#else

	grabber.getPixelsRef().cropTo(cropped, 0, 380, 1920, 700);
	finder.update(cropped);

#endif

	ofRectangle rightMost; 

	for (int i = 0; i < finder.size(); i++) {
		ofRectangle object = finder.getObject(i);
		if (object.x > rightMost.x && object.x < 1920) {
			rightMost = object;
			rightMost.width = object.width;
			cout << rightMost << endl;

		}
	}
	if (rightMost.x < 700) {
		rightMost.x = 700;
	}
	mapTexCoords(rightCylinderPiece, min(1920, max(500, (int) rightMost.x + 200)), 380, 0, 700);

}

void ofApp::drawLeftCylinder()
{
	float A = 0.90;
	float B = 1.0 - A;
	currentLeftCylinder = A * currentLeftCylinder + B * targetLeftCylinder;

#ifdef MIC_DEBUGGING

	panoView.getTextureReference().bind();
	leftCylinderPiece.draw();
	panoView.getTextureReference().unbind();

#else

	grabber.getTextureReference().bind();
	leftCylinderPiece.draw();
	grabber.getTextureReference().unbind();

#endif
}

void ofApp::drawRightCylinder()
{
	float A = 0.90;
	float B = 1.0 - A;
	currentRightCylinder = A * currentRightCylinder + B * targetRightCylinder;

#ifdef MIC_DEBUGGING

	panoView.getTextureReference().bind();
	rightCylinderPiece.draw();
	panoView.getTextureReference().unbind();

#else

	grabber.getTextureReference().bind();
	rightCylinderPiece.draw();
	grabber.getTextureReference().unbind();

#endif
}

void ofApp::drawRemoteUser()
{
	//remoteUserVideo.draw(ofGetWidth() / 2, 300);
}

void ofApp::drawTexturedCylinder(){
	// draw the texture-mapped cylinder.
    

}


void ofApp::keyPressed  (int key){ 
	
	switch (key){
	case ' ':
		drawCylinder = !drawCylinder;
		break;

	case 'm':
		if (userDisplayState == UDS_SHOW_BOTH_LOCAL_USER) {
			userDisplayState = UDS_SHOW_FEED;
		}
		else {
			int i = (int)userDisplayState;
			userDisplayState = (USER_DISPLAY_STATE) ++i;
		}

		break;

	case 'h':
		{
			serial.writeByte('1');
		} 
		break;

	case 'r':
		isDrawingRightCylinder = !isDrawingRightCylinder;
		if (isDrawingRightCylinder) {
			findRightFace();
		}
		currentRightCylinder = targetRightCylinder = 1400;
		break;

	case 'z':
		isDrawingSecondRemote = !isDrawingSecondRemote;
		break;

	case 'l':
		isDrawingLeftCylinder = !isDrawingLeftCylinder;
		if (isDrawingLeftCylinder) {
			findLeftFace();
		}
		currentLeftCylinder = targetLeftCylinder = 0;
		break;
	}

	
}



void ofApp::mouseDragged(int x, int y, int button){

	userControlCountdown = 5.0;
	navState = USER_CONTROL;

	float A = 0.90;
	float B = 1.0-A;
	blurredMouseX = A*blurredMouseX + B*mouseX;


	angularOffset = ofMap(mouseX, 0, ofGetWidth(), 0-180, 180, false);

	ofxOscMessage m;
	m.setAddress("position");
	m.addIntArg(angularOffset);
	sender.sendMessage(m);

	float adaptedMouseX = blurredMouseX;
	float denominator = fmod(adaptedMouseX , 450.0);
	adaptedMouseX -= denominator * 450.0;

	if (ofGetElapsedTimef() - lastSentMouseLocation > 0.05)
	{

		//int led = ofMap(adaptedMouseX, 0, 450, 0, 48, false);
		int led = ofMap(blurredMouseX, 0, ofGetWidth(), 0, 48, false);

		unsigned char val[4];
		stringstream ss;
		ss << '2' << led;

		cout << led << " " << ss << endl;

		strncpy((char*)&val[0], ss.str().c_str(), sizeof(val));
		serial.writeBytes(&val[0], 4);

		lastSentMouseLocation = ofGetElapsedTimef();
	}


	//bAngularOffsetChanged = true;
}


void ofApp::mousePressed(int x, int y, int button){
	userControlCountdown = 5.0;
	navState = USER_CONTROL;
}

void ofApp::mouseReleased(int x, int y, int button){
	serial.writeByte('3');
}

void ofApp::mouseMoved(int x, int y ){
}

void ofApp::keyReleased(int key){ 

}

void ofApp::windowResized(int w, int h){
}

void ofApp::createCylinderPiece(ofMesh &m, float radius, float height, float degrees)
{

	m.setupIndicesAuto();

	ofVec2f lastCoord;
	int f = 0;
	for (float deg = 0; deg < degrees; deg+=1.0, f += 4)
	{
		float ca = deg * 0.01745329252;//cache current angle
		float na = (deg + 1) * 0.01745329252;//cache next angle (could do with a conditional and storing previous angle)
		float ccos = cos(ca);//current cos
		float csin = sin(ca);//current sin
		float ncos = cos(na);//next cos
		float nsin = sin(na);//next sin

		ofVec3f tl(radius * ccos, height * .5, radius * csin);//top left = current angle, positive y
		ofVec3f bl(radius * ccos, -height * .5, radius * csin);//bottom left = current angle, negative y
		ofVec3f tr(radius * ncos, height * .5, radius * nsin);//top right = next angle, positive y
		ofVec3f br(radius * ncos, -height * .5, radius * nsin);//bottom right = next angle, negative y

		/*
		tl--tr
		|  /|
		| / |
		|/  |
		bl--br
		*/

		// 6 verts
		m.addVertex(tl);
		m.addTexCoord(ofVec2f(radius * ccos, 380));
		m.addVertex(tr);
		m.addTexCoord(ofVec2f(radius * ncos, 380));
		m.addVertex(bl);
		m.addTexCoord(ofVec2f(radius * ccos, 700));

		m.addVertex(bl);
		m.addTexCoord(ofVec2f(radius * ccos, 700));
		m.addVertex(tr);
		m.addTexCoord(ofVec2f(radius * ncos, 380));
		m.addVertex(br);
		m.addTexCoord(ofVec2f(radius * ncos, 700));

		lastCoord.set(ofVec2f(radius * ncos, 700));
	}

	prevTcoordLCP.set(0, 0, lastCoord.x, lastCoord.y);
	prevTcoordRCP.set(0, 0, lastCoord.x, lastCoord.y);

}

void ofApp::mapTexCoords(ofMesh &m, float u1, float v1, float u2, float v2)
{
	//for (int j = 0; j < m.getNumTexCoords(); j++)
	//{
	//	ofVec2f tcoord = m.getTexCoord(j);
	//	tcoord.x = ofMap(tcoord.x, prevTcoordRCP.x, prevTcoordRCP.z, u1, u2, true);
	//	m.setTexCoord(j, tcoord);
	//}

	m.clearTexCoords();
	m.clearVertices();

	int f = 0;
	for (float deg = 0; deg < 44.0; deg += 1.0, f += 4)
	{
		float ca = deg * 0.01745329252;//cache current angle
		float na = (deg + 1) * 0.01745329252;//cache next angle (could do with a conditional and storing previous angle)
		float ccos = cos(ca);//current cos
		float csin = sin(ca);//current sin
		float ncos = cos(na);//next cos
		float nsin = sin(na);//next sin

		ofVec3f tl(warpedW * ccos, 1080, warpedW * csin);//top left = current angle, positive y
		ofVec3f bl(warpedW * ccos, -1080, warpedW * csin);//bottom left = current angle, negative y
		ofVec3f tr(warpedW * ncos, 1080, warpedW * nsin);//top right = next angle, positive y
		ofVec3f br(warpedW * ncos, -1080, warpedW * nsin);//bottom right = next angle, negative y

		/*
		tl--tr
		|  /|
		| / |
		|/  |
		bl--br
		*/

		// 6 verts
		m.addVertex(tl);
		m.addTexCoord(ofVec2f((warpedW * ccos) - (warpedW - u1), 700));
		m.addVertex(tr);
		m.addTexCoord(ofVec2f((warpedW * ncos) - (warpedW - u1), 700));
		m.addVertex(bl);
		m.addTexCoord(ofVec2f((warpedW * ccos) - (warpedW - u1), 380));

		m.addVertex(bl);
		m.addTexCoord(ofVec2f((warpedW * ccos) - (warpedW - u1), 380));
		m.addVertex(tr);
		m.addTexCoord(ofVec2f((warpedW * ncos) - (warpedW - u1), 700));
		m.addVertex(br);
		m.addTexCoord(ofVec2f((warpedW * ncos) - (warpedW - u1), 380));

	}
}
