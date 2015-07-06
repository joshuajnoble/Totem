#include "ofTotemApp.h"
#include "Utils.h"

using namespace ofxCv;
using namespace cv;

namespace
{
	const ofColor BACKGROUND_COLOR = ofColor(0x08, 0x21, 0x35);
}

void ofTotemApp::earlyinit(int netid)
{
	this->networkInterfaceId = netid;
	this->totemDisplay.initTotemDisplay(4, 800, 1280);
}

int ofTotemApp::displayWidth() const
{
	return this->totemDisplay.windowWidth();
}

int ofTotemApp::displayHeight() const
{
	return this->totemDisplay.windowHeight();
}

//--------------------------------------------------------------
void ofTotemApp::setup()
{
	VideoCaptureAppBase::setup(this->networkInterfaceId, true);

	this->totemDisplay.allocateBuffers();
	this->isRemoteSource1Initialized = false;
	this->isInitialized = true;

	this->streamManager.broadcastVideoBitrate = 8000;
}

//--------------------------------------------------------------
void ofTotemApp::exit()
{
	streamManager.exit();
}

//--------------------------------------------------------------
void ofTotemApp::update()
{
	if (!this->isInitialized)
	{
		return;
	}

	VideoCaptureAppBase::update();

	this->totemDisplay.update();
}


//--------------------------------------------------------------
void ofTotemApp::draw()
{
	if (!this->isInitialized)
	{
		return;
	}

	if (this->rawSource)
	{
		this->rawSource->draw(0, 0);
	}
	else if (this->showOutput)
	{
		this->videoSource->draw(0, 0);
	}
	else
	{
		if (this->totemDisplay.drawTestPattern)
		{
			this->totemDisplay.draw();
		}
		else
		{
			auto output = this->totemDisplay.getDisplay(0);
			output.begin();
			ofBackground(BACKGROUND_COLOR);

			if (this->netImpersonate.get())
			{	// DEBUG
				this->netImpersonate->update();
				if (this->netImpersonate->isFrameNew())
				{
					Utils::DrawCroppedToFit(*this->netImpersonate.get(), (int)output.getWidth(), (int)output.getHeight());
				}
			}
			else if (this->remoteVideoSources.size())
			{
				auto margin = 10;
				auto halfMargin = margin / 2;
				auto halfHeight = (int)output.getHeight() / 2;
				auto halfWidth = (int)output.getWidth() / 2;

				auto remoteSourceCount = this->remoteVideoSources.size();
				if (remoteSourceCount == 1)
				{
					auto videoSource = this->remoteVideoSources[0];
					videoSource->DrawCropped((int)output.getWidth(), (int)output.getHeight());
				}
				else if (remoteSourceCount == 2)
				{
					auto videoSource = this->remoteVideoSources[0];
					videoSource->DrawCropped((int)output.getWidth(), halfHeight - halfMargin);

					ofPushMatrix();

					videoSource = this->remoteVideoSources[1];
					ofTranslate(0, halfHeight + halfMargin);
					videoSource->DrawCropped((int)output.getWidth(), halfHeight - halfMargin);

					ofPopMatrix();
				}
				else if (remoteSourceCount == 3)
				{
					auto videoSource = this->remoteVideoSources[0];
					videoSource->DrawCropped((int)output.getWidth(), halfHeight - halfMargin);

					ofPushMatrix();

					videoSource = this->remoteVideoSources[1];
					ofTranslate(0, halfHeight + halfMargin);
					videoSource->DrawCropped(halfWidth - halfMargin, halfHeight - halfMargin);

					videoSource = this->remoteVideoSources[2];
					ofTranslate(halfWidth + halfMargin, 0);
					videoSource->DrawCropped(halfWidth - halfMargin, halfHeight - halfMargin);

					ofPopMatrix();
				}
			}

			output.end();
			this->totemDisplay.drawCloned();
		}
	}
}


//--------------------------------------------------------------
void ofTotemApp::keyPressed(int key)
{
	if (!this->isInitialized)
	{
		return;
	}
}

void ofTotemApp::onKeyframe(ofxPlaylistEventArgs& args)
{
	if (!this->isInitialized)
	{
		return;
	}
}

void ofTotemApp::Handle_ClientConnected(RemoteVideoInfo& remote)
{
	this->totemDisplay.drawTestPattern = false;

	ofLog() << "Network client connected " << remote.clientId << endl;

	// Show the client video
	this->remoteVideoSources.push_back(remote.source);
}

void ofTotemApp::Handle_ClientDisconnected(RemoteVideoInfo& remote)
{
	ofLog() << "Network client disconnected " << remote.clientId << endl;
	auto found = std::find(this->remoteVideoSources.begin(), this->remoteVideoSources.end(), remote.source);
	if (found != this->remoteVideoSources.end())
	{
		this->remoteVideoSources.erase(found);
	}
}

void ofTotemApp::ImporsonateRemoteClient(ofPtr<ofBaseVideoDraws> source)
{
	this->totemDisplay.drawTestPattern = false;
	this->netImpersonate = source;
}