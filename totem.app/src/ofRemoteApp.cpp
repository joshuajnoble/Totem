#include "ofRemoteApp.h"

#define CYLINDER_PIECE_WIDTH 44
#define CYLINDER_PIECE_HEIGHT 2
#define PIECE_TEXCOORD_WIDTH 720
#define NEO_PIXELS_COUNT 45

namespace
{
	float lastElapsed;
	float lastSentMouseLocation;

	//DEBUG
	ofPtr<RemoteVideoInfo> video1;
	ofPtr<RemoteVideoInfo> video2;
}

// ********************************************************************************************************************
void ofRemoteApp::earlyinit()
{
	this->cylinderDisplay.initCylinderDisplay(1920, 1080);
	int margin = 70;
	int remoteViewOffsetX = 1130 + margin;// (int)(1920 * .75);
	this->networkDisplay.initializeRemoteNetworkDisplay(ofRectangle(remoteViewOffsetX, 70, 1920 - remoteViewOffsetX - margin, 1080 - margin * 2));
}

// ********************************************************************************************************************
void ofRemoteApp::setup()
{
	this->cylinderDisplay.allocateBuffers();
}


// ********************************************************************************************************************
void ofRemoteApp::update()
{
	this->videoSource->update();
	if (this->videoSource->isFrameNew())
	{
		this->cylinderDisplay.update();
	}

	this->networkDisplay.update();

	//mainPlaylist.update();
	ofSetVerticalSync(false);
}


// ********************************************************************************************************************
void ofRemoteApp::draw()
{
	ofBackground(11,26,38);

	this->cylinderDisplay.draw();

	DrawSelfie();

	this->networkDisplay.draw();

	if (false)// showInstructions)
	{
		//ofDrawBitmapString(" ?: show instructions \n z: show remote caller \n m: turn on directional mics \n l: look for the left-most participant \n r: look for the right most participant \n space: show the raw v360 feed", 50, 50);
	}
}


// ********************************************************************************************************************
void ofRemoteApp::DrawSelfie()
{
	ofPushMatrix();

	// Scale the video, but also flip the X axis, so we see it mirrored.
	// This means all of the following x values need to be negated and offset.
	ofScale(-this->scale, this->scale);
	
	// Draw the source video in a small window
	auto ratio = this->videoSource->getWidth() / this->videoSource->getHeight();
	int selfieWidth = 320;
	int selfieHeight = selfieWidth / ratio;
	int selfieX = (this->width - selfieWidth) / 2;
	int selfieY = 40;
	int margin = 15;

	ofSetColor(32); // Selfie margin color
	ofRect(-(selfieX + margin + selfieWidth), selfieY - margin, selfieWidth + margin * 2, selfieHeight + margin * 2);

	ofSetColor(255);
	this->videoSource->draw(-(selfieX + selfieWidth), selfieY, selfieWidth, selfieHeight);

	ofPopMatrix();
}


// ********************************************************************************************************************
void ofRemoteApp::exit()
{
}


// ********************************************************************************************************************
int ofRemoteApp::displayWidth() const
{
	return this->width * this->scale;
}


// ********************************************************************************************************************
int ofRemoteApp::displayHeight() const
{
	return this->height * this->scale;
}


// ********************************************************************************************************************
void ofRemoteApp::RegisterTotemVideoSource(ofPtr<ofBaseVideoDraws> source)
{
	this->remoteTotemSource = source;
	this->cylinderDisplay.setTotemVideoSource(this->remoteTotemSource);
}


// ********************************************************************************************************************
ofPtr<RemoteVideoInfo> ofRemoteApp::RegisterRemoteVideoSource(ofPtr<ofBaseVideoDraws> source)
{
	auto remote = ofPtr<RemoteVideoInfo>(new RemoteVideoInfo());
	remote->source = source;
	this->remoteVideoSources.push_back(remote);
	this->networkDisplay.AddVideoSource(remote);
	return remote;
}

void ofRemoteApp::keyPressed(int key)
{
	if (this->networkDisplay.CanModify())
	{
		if (key == 'a')
		{
			if (!video1.get())
			{
				video1 = ofPtr<RemoteVideoInfo>(new RemoteVideoInfo());
				video1->source = this->videoSource;
				this->networkDisplay.AddVideoSource(video1);
			}
			else if (!video2.get())
			{
				video2 = ofPtr<RemoteVideoInfo>(new RemoteVideoInfo());
				video2->source = this->videoSource;
				this->networkDisplay.AddVideoSource(video2);
			}
		}
		else if (key == 'z')
		{
			if (video2.get())
			{
				if (this->networkDisplay.RemoveVideoSource(video2))
				{
					video2.reset();
				}
			}
			else if (video1.get())
			{
				if (this->networkDisplay.RemoveVideoSource(video1))
				{
					video1.reset();
				}
			}
		}
	}
}