#include "ofTotemApp.h"
#include "Utils.h"
#include "..\..\SharedCode\ofxFFmpegVideoReceiver.h"

//#define SHOW_FPS

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
#ifdef SHOW_FPS
	ofSetVerticalSync(false);
#else
	ofSetFrameRate(30);
	ofSetVerticalSync(true);
#endif

	VideoCaptureAppBase::setup(this->networkInterfaceId, true);

	this->totemDisplay.allocateBuffers();
	this->isRemoteSource1Initialized = false;
	this->isInitialized = true;

	cortanaPlayIntro();
	this->ConnectToSession();
}

//--------------------------------------------------------------
void ofTotemApp::exit()
{
	this->DisconnectSession();
	VideoCaptureAppBase::exit();
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

	this->cortanaPlayer.update();
	if (this->cortanaPlayer.isFrameNew())
	{
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
			else
			{
				std::vector<int> sources;
				for (int i = 0; i < this->peers.size(); ++i)
				{
					if (this->peers[i].peerStatus.isConnectedToSession)
					{
						sources.push_back(i);
					}
				}

				int remoteSourceCount = sources.size();

				if (remoteSourceCount == 0)
				{
					ofBackground(0);

					auto halfHeight = (int)output.getHeight() / 2;
					auto halfWidth = (int)output.getWidth() / 2;
					ofRectangle region(0, 0, this->cortanaPlayer.width, this->cortanaPlayer.height);

					auto scale = 1.0;// output.getWidth() / this->cortanaPlayer.getWidth();
					region.scaleFromCenter(scale, scale);

					region.translateY(halfHeight - this->cortanaPlayer.height / 2 - 30 * scale);
					region.translateX(halfWidth - this->cortanaPlayer.width / 2);

					this->cortanaPlayer.draw(region);
				}
				else
				{
					auto margin = 10;
					auto halfMargin = margin / 2;
					auto halfHeight = (int)output.getHeight() / 2;
					auto halfWidth = (int)output.getWidth() / 2;

					ofBackground(BACKGROUND_COLOR);

					if (remoteSourceCount == 1)
					{
						auto videoSource = this->peers[sources[0]].remoteVideoSource->getVideoImage();
						Utils::DrawCroppedToFit(videoSource, (int)output.getWidth(), (int)output.getHeight());
					}
					else if (remoteSourceCount == 2)
					{
						auto videoSource = this->peers[sources[0]].remoteVideoSource->getVideoImage();
						Utils::DrawCroppedToFit(videoSource, (int)output.getWidth(), (int)output.getHeight());

						ofPushMatrix();

						videoSource = this->peers[sources[1]].remoteVideoSource->getVideoImage();
						ofTranslate(0, halfHeight + halfMargin);
						Utils::DrawCroppedToFit(videoSource, (int)output.getWidth(), halfHeight - halfMargin);

						ofPopMatrix();
					}
					else if (remoteSourceCount == 3)
					{
						auto videoSource = this->peers[sources[0]].remoteVideoSource->getVideoImage();
						Utils::DrawCroppedToFit(videoSource, (int)output.getWidth(), halfHeight - halfMargin);

						ofPushMatrix();

						videoSource = this->peers[sources[1]].remoteVideoSource->getVideoImage();
						ofTranslate(0, halfHeight + halfMargin);
						Utils::DrawCroppedToFit(videoSource, halfWidth - halfMargin, halfHeight - halfMargin);

						videoSource = this->peers[sources[2]].remoteVideoSource->getVideoImage();
						ofTranslate(halfWidth + halfMargin, 0);
						Utils::DrawCroppedToFit(videoSource, halfWidth - halfMargin, halfHeight - halfMargin);

						ofPopMatrix();
					}

				}
			}

			output.end();
			this->totemDisplay.drawCloned();
		}
	}

#ifdef SHOW_FPS
	ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, 10);
#endif
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
	auto anyConenctedPeers = std::any_of(this->peers.begin(), this->peers.end(), [](RemoteVideoInfo &x)->bool { return x.peerStatus.isConnectedToSession; });
	if (!anyConenctedPeers)
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
	else if (!this->cortanaLoadErrorDisplayed)
	{
		this->cortanaLoadErrorDisplayed = true;
		cout << "You are missing a cortana file: " + fullPath;
	}
}

void ofTotemApp::cortanaPlayIntro()
{
	cortanaLoadClip(CORTANA_GREET);
	this->cortanaPlayer.setLoopState(OF_LOOP_NONE);
	this->cortanaPlayer.setSpeed(0.5);
	this->cortanaPlayer.play();
}

void ofTotemApp::cortanaPlayIdle()
{
	cortanaLoadClip(CORTANA_IDLE);
	this->cortanaPlayer.setLoopState(OF_LOOP_NORMAL);
	this->cortanaPlayer.setSpeed(0.5);
	this->cortanaPlayer.play();
}