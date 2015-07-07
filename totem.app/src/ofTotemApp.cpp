#include "ofTotemApp.h"
#include "Utils.h"

using namespace ofxCv;
using namespace cv;

namespace
{
	const ofColor BACKGROUND_COLOR = ofColor(0x08, 0x21, 0x35);

	const std::string CORTANA_IDLE = "personaassets720x1280_circle_calm1_10.gif";
	const std::string CORTANA_ELATED = "personaassets720x1280_circle_elated1_10.gif";
	const std::string CORTANA_GREET = "personaassets720x1280_circle_greeting1_10.gif";
	const std::string CORTANA_GREET_ALTERNATE = "personaassets720x1280_circle_greeting2_10.gif";
	const std::string CORTANA_LISTENTING = "personaassets720x1280_circle_listening1_10.gif";
	const std::string CORTANA_THINKING = "personaassets720x1280_circle_thinking1_10.gif";
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

	cortanaPlayIntro();
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

	if (this->cortanaPlayer.getIsMovieDone())
	{
		cortanaPlayIdle();
	}

	if (this->cortanaPlayer.isFrameNew())
	{
		this->cortanaPlayer.update();
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

				ofBackground(BACKGROUND_COLOR);
				auto remoteSourceCount = this->remoteVideoSources.size();
				if (remoteSourceCount == 1)
				{
					auto videoSource = this->remoteVideoSources[0].source;
					videoSource->DrawCropped((int)output.getWidth(), (int)output.getHeight());
				}
				else if (remoteSourceCount == 2)
				{
					auto videoSource = this->remoteVideoSources[0].source;
					videoSource->DrawCropped((int)output.getWidth(), halfHeight - halfMargin);

					ofPushMatrix();

					videoSource = this->remoteVideoSources[1].source;
					ofTranslate(0, halfHeight + halfMargin);
					videoSource->DrawCropped((int)output.getWidth(), halfHeight - halfMargin);

					ofPopMatrix();
				}
				else if (remoteSourceCount == 3)
				{
					auto videoSource = this->remoteVideoSources[0].source;
					videoSource->DrawCropped((int)output.getWidth(), halfHeight - halfMargin);

					ofPushMatrix();

					videoSource = this->remoteVideoSources[1].source;
					ofTranslate(0, halfHeight + halfMargin);
					videoSource->DrawCropped(halfWidth - halfMargin, halfHeight - halfMargin);

					videoSource = this->remoteVideoSources[2].source;
					ofTranslate(halfWidth + halfMargin, 0);
					videoSource->DrawCropped(halfWidth - halfMargin, halfHeight - halfMargin);

					ofPopMatrix();
				}
			}
			else
			{
				ofBackground(0);

				auto scale = output.getWidth() / this->cortanaPlayer.getWidth();
				auto height = (int)roundf(this->cortanaPlayer.getHeight() * scale);
				auto yOffset = -(int)roundf(30 * scale);
				this->cortanaPlayer.draw(0, (int)(output.getHeight() - height) / 2 + yOffset, (int)output.getWidth(), height);
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
}

void ofTotemApp::Handle_ClientDisconnected(RemoteVideoInfo& remote)
{
	if (!this->remoteVideoSources.size())
	{
		cortanaPlayIntro();
	}
}

void ofTotemApp::ImporsonateRemoteClient(ofPtr<ofBaseVideoDraws> source)
{
	this->totemDisplay.drawTestPattern = false;
	this->netImpersonate = source;
}

void ofTotemApp::cortanaLoadClip(const string& clipName)
{
	auto fullPath = ofToDataPath("cortana\\" + clipName);
	if (ofFile::doesFileExist(fullPath))
	{
		this->cortanaPlayer.loadMovie(fullPath);
	}
}

void ofTotemApp::cortanaPlayIntro()
{
	cortanaLoadClip(CORTANA_GREET);
	this->cortanaPlayer.setLoopState(OF_LOOP_NONE);
	this->cortanaPlayer.play();
}

void ofTotemApp::cortanaPlayIdle()
{
	cortanaLoadClip(CORTANA_IDLE);
	this->cortanaPlayer.setLoopState(OF_LOOP_NORMAL);
	this->cortanaPlayer.play();
}