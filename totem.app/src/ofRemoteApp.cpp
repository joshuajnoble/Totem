#include "ofRemoteApp.h"


//--------------------------------------------------------------
void ofRemoteApp::earlyinit()
{

}


//--------------------------------------------------------------
void ofRemoteApp::setup()
{

}


//--------------------------------------------------------------
void ofRemoteApp::update()
{
	this->videoSource->update();
	if (this->videoSource->isFrameNew())
	{
	}
}


//--------------------------------------------------------------
void ofRemoteApp::draw()
{
	ofPushMatrix();
	ofScale(this->scale, this->scale);

	ofSetColor(255, 255, 255);

	// Draw the source video in a small window
	auto ratio = this->videoSource->getWidth() / this->videoSource->getHeight();
	int selfieWidth = 320;
	int selfieHeight = selfieWidth / ratio;
	this->videoSource->draw((this->width - selfieWidth) / 2, this->height - selfieHeight - 25, selfieWidth, selfieHeight);

	ofPopMatrix();
}


//--------------------------------------------------------------
void ofRemoteApp::exit()
{

}


//--------------------------------------------------------------
void ofRemoteApp::keyPressed(int key)
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