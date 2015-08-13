#include "ofSurfaceHubApp.h"
#include "..\..\SharedCode\VideoConverters.h"

namespace
{
	const int PROFILE_PADDING = 10;
	const int PROFILE_SIZE = 128;
}

void ofSurfaceHubApp::earlyinit(int netid)
{
	this->networkInterfaceId = netid;
}

int ofSurfaceHubApp::displayWidth() const
{
	int maxWidth = 2160;
	auto width = GetSystemMetrics(SM_CXSCREEN);
	return width > maxWidth ? maxWidth : width;
}

int ofSurfaceHubApp::displayHeight() const
{
	int maxHeight = 1440;
	auto height = GetSystemMetrics(SM_CYSCREEN);
	return height > maxHeight ? maxHeight : height;
}

void ofSurfaceHubApp::setup()
{
	ofSetFrameRate(30);
	ofSetVerticalSync(true);
	
	SetupDiscovery();

	profilePictures.push_back(ofImage("profiles/profile_01.png"));
	profilePictures.push_back(ofImage("profiles/profile_02.png"));
	profilePictures.push_back(ofImage("profiles/profile_03.png"));
	presentation.loadImage("presentation_slide.png");
	connectButton.loadImage("hub_connect.png");
}

void ofSurfaceHubApp::update()
{
	udpDiscovery.update();
}

void ofSurfaceHubApp::draw()
{
	ofBackground(0);

	// Draw the profile images
	int y = PROFILE_PADDING;
	for (int i = 0; i < peers.size(); ++i)
	{
		peers[i].profilePicture.draw(10, y, PROFILE_SIZE, PROFILE_SIZE);
		y += PROFILE_SIZE + PROFILE_PADDING;
	}

	hasTotemConnected = true; // TODO: Remove this when we are ready to really test with a totem source
	if (peers.size() && hasTotemConnected)
	{
		buttonPosition.set(PROFILE_PADDING, y, PROFILE_SIZE, PROFILE_SIZE);
		connectButton.draw(buttonPosition);
	}

	// Draw the slide
	auto leftMargin = PROFILE_SIZE + PROFILE_PADDING * 2;
	ofRectangle fill(0, 0, presentation.width, presentation.height);
	fill.scaleTo(ofRectangle(leftMargin, 0, this->displayWidth() - leftMargin, this->displayHeight()), OF_SCALEMODE_FILL);
	presentation.draw(fill);
}

void ofSurfaceHubApp::exit()
{
}

void ofSurfaceHubApp::PeerArrived(UdpDiscovery::RemotePeerStatus& peer)
{
	auto found = std::find_if(peers.begin(), peers.end(), [peer](PeerInfo x)->bool { return x.id == peer.id; });
	if (found == peers.end())
	{
		auto entry = PeerInfo();
		entry.id = peer.id;
		entry.isTotem = peer.isTotem;
		entry.profilePicture = profilePictures.front();
		profilePictures.erase(profilePictures.begin());
		peers.push_back(entry);

		if (peer.isTotem)
		{
			hasTotemConnected = true;
		}
	}
}

void ofSurfaceHubApp::PeerLeft(UdpDiscovery::RemotePeerStatus& peer)
{
	auto found = std::find_if(peers.begin(), peers.end(), [peer](PeerInfo x)->bool { return x.id == peer.id; });
	if (found != peers.end())
	{
		profilePictures.push_back(found->profilePicture);
		peers.erase(found);

		if (peer.isTotem)
		{
			hasTotemConnected = false;
		}
	}
}

void ofSurfaceHubApp::SetupDiscovery()
{
	this->udpDiscovery.setupSurfaceHub();
	ofAddListener(udpDiscovery.peerArrivedEvent, this, &ofSurfaceHubApp::PeerArrived);
	ofAddListener(udpDiscovery.peerLeftEvent, this, &ofSurfaceHubApp::PeerLeft);
}

void ofSurfaceHubApp::mousePressed(int x, int y, int button)
{
	if (button == 0 && buttonPosition.inside(x, y))
	{
		// The connect button has been pressed
	}
}