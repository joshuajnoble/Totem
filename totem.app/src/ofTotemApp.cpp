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
	serial.setup(1, 9600);
	if (!serial.isInitialized())
	{
		serial.setup(0, 9600);;
	}

	this->waitingForSurfaceHub = true;
	this->foundSurfaceHub = false;
	this->surfaceHubDetectionTimeout = ofGetElapsedTimef();
	this->surfaceHubDetectionTimeout += 2.0f;

	cortanaPrompt.loadFont("surfacehub/segoeui.ttf", 32);
	cortanaPrompt.setSpaceSize(cortanaPrompt.getSpaceSize() / 2);

	cortanaTip.loadFont("surfacehub/segoeui.ttf", 24);
	cortanaTip.setSpaceSize(cortanaTip.getSpaceSize() / 2);
}

//--------------------------------------------------------------
void ofTotemApp::exit()
{
	this->serial.close();
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

	unsigned char serialBuffer[3];
	while (serial.isInitialized() && serial.available() > 0)
	{
		auto directionId = serial.readByte();
		if (directionId >= '1' && directionId <= '9')
		{
			auto angle = int(std::roundf((directionId - '1') / 8.0 * 360));// -22.5);
			if (angle < 0) angle += 360;
			this->udpDiscovery.SetSourceRotation(angle);
			cout << (char)directionId << " - " << angle << std::endl;
		}
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

	// See if we can find a surfaceHub instance
	if (this->waitingForSurfaceHub && !this->foundSurfaceHub)
	{
		this->foundSurfaceHub = std::any_of(this->peers.begin(), this->peers.end(), [](const RemoteVideoInfo& p) { return p.peerStatus.isSurfaceHub; });
		this->waitingForSurfaceHub = !this->foundSurfaceHub;
		this->ReadyUp();
	}

	// Stop searching for the surfaceHub if it has been long enough
	if (this->waitingForSurfaceHub)
	{
		if (ofGetElapsedTimef() >= this->surfaceHubDetectionTimeout)
		{
			this->waitingForSurfaceHub = false;
			this->ConnectToSession();
		}
	}

	// If we are connected to a hub-based session and all of the remotes are gone, then disconnect from the session
	if (this->foundSurfaceHub & this->udpDiscovery.isConnectedToSession)
	{
		auto hasRemotes = std::any_of(this->peers.begin(), this->peers.end(), [](const RemoteVideoInfo& p) { return !p.peerStatus.isSurfaceHub && !p.peerStatus.isTotem; });
		if (!hasRemotes)
		{
			this->DisconnectSession();
		}
	}
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

			if (this->netImpersonate.get())
			{	// DEBUG
				output.begin();
				this->netImpersonate->update();
				if (this->netImpersonate->isFrameNew())
				{
					Utils::DrawCroppedToFit(*this->netImpersonate.get(), (int)output.getWidth(), (int)output.getHeight());
				}
				output.end();
				this->totemDisplay.drawCloned();
			}
			else
			{
				std::vector<int> sources;
				for (int i = 0; i < this->peers.size(); ++i)
				{
					if (this->peers[i].peerStatus.isConnectedToSession && !(this->peers[i].peerStatus.isTotem || this->peers[i].peerStatus.isSurfaceHub))
					{
						sources.push_back(i);
					}
				}

				int remoteSourceCount = sources.size();
				if (remoteSourceCount == 0)
				{
					if (this->cortanaPlayer.isFrameNew())
					{
						output.begin();
						ofBackground(0);

						auto halfHeight = (int)output.getHeight() * 0.4;
						auto halfWidth = (int)output.getWidth() / 2;
						ofRectangle region(0, 0, this->cortanaPlayer.width, this->cortanaPlayer.height);

						auto scale = 1.0;// output.getWidth() / this->cortanaPlayer.getWidth();
						region.scaleFromCenter(scale, scale);

						region.translateY(halfHeight - this->cortanaPlayer.height / 2 - 30 * scale);
						region.translateX(halfWidth - this->cortanaPlayer.width / 2);

						ofSetColor(28, 137, 175);
						this->cortanaPlayer.draw(region);

						if (this->foundSurfaceHub)
						{
							std::string prompt("I'm waiting for guests to join.");
							auto size = cortanaPrompt.getStringBoundingBox(prompt, 0, 0);
							size.alignToHorz(region);
							size.setPosition(size.x, region.getBottom() + 50);
							cortanaPrompt.drawString(prompt, size.x, size.y);

							//size = cortanaPrompt.getStringBoundingBox("the meeting.", 0, 0);
							//size.alignToHorz(region);
							//size.setPosition(size.x, region.getBottom() + 50 + size.height * 1.3);
							//cortanaPrompt.drawString("the meeting.", size.x, size.y);
						}
						else
						{
							auto size = cortanaPrompt.getStringBoundingBox("Who would you like to meet with?", 0, 0);
							size.alignToHorz(region);
							size.setPosition(size.x, region.getBottom() + 75);
							cortanaPrompt.drawString("Who would you like to meet with?", size.x, size.y);
							auto bottom = size.getBottom();

							//size = cortanaPrompt.getStringBoundingBox("you like to", 0, 0);
							//size.alignToHorz(region);
							//size.setPosition(size.x, bottom + size.height * 0.3);
							//cortanaPrompt.drawString("you like to", size.x, size.y);
							//bottom = size.getBottom();

							//size = cortanaPrompt.getStringBoundingBox("meet with?", 0, 0);
							//size.alignToHorz(region);
							//size.setPosition(size.x, bottom + size.height * 0.3);
							//cortanaPrompt.drawString("meet with?", size.x, size.y);
							//bottom = size.getBottom();

							ofSetColor(128);
							size = cortanaTip.getStringBoundingBox("try call Mark in Studio B.", 0, 0);
							size.alignToHorz(region);
							size.setPosition(size.x, bottom + size.height * 2.3);
							cortanaTip.drawString("try call Mark in Studio B.", size.x, size.y);
						}

						ofSetColor(255);
						output.end();
					}

					this->totemDisplay.drawCloned();
				}
				else
				{
					bool anyUpdates = std::any_of(this->peers.begin(), this->peers.end(), [](RemoteVideoInfo &x)->bool
					{
						return !x.peerStatus.isTotem &&
							!x.peerStatus.isSurfaceHub &&
							x.remoteVideoSource->isVideoFrameNew();
					});
					if (anyUpdates)
					{
						auto margin = 10;
						auto halfMargin = margin / 2;
						auto halfHeight = (int)output.getHeight() / 2;
						auto halfWidth = (int)output.getWidth() / 2;

						output.begin();
						ofBackground(BACKGROUND_COLOR);

						if (remoteSourceCount == 1)
						{
							auto videoSource = this->peers[sources[0]].remoteVideoSource->getVideoImage();
							Utils::DrawCroppedToFit(videoSource, (int)output.getWidth(), (int)output.getHeight());
						}
						else if (remoteSourceCount == 2)
						{
							auto videoSource = this->peers[sources[0]].remoteVideoSource->getVideoImage();
							Utils::DrawCroppedToFit(videoSource, (int)output.getWidth(), halfHeight - halfMargin);

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

						output.end();
					}
					this->totemDisplay.drawCloned();
				}
			}
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

	if (this->foundSurfaceHub && key == '\x20')
	{
		this->ConnectToSession();
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