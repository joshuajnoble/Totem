#include "ofSurfaceHubApp.h"
#include "..\..\SharedCode\VideoConverters.h"

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
}

void ofSurfaceHubApp::update()
{
}

void ofSurfaceHubApp::draw()
{
	ofBackground(0);
	ofPushStyle();
	ofSetColor(ofColor::pink);
	ofCircle(200, 200, 100);
	ofPopStyle();

	ofCircle(400, 200, 100);
}

void ofSurfaceHubApp::exit()
{
}

void ofSurfaceHubApp::PeerArrived(UdpDiscovery::RemotePeerStatus& peer)
{

}

void ofSurfaceHubApp::PeerLeft(UdpDiscovery::RemotePeerStatus& peer)
{

}

void ofSurfaceHubApp::SetupDiscovery()
{
	this->udpDiscovery.setupSurfaceHub();
	ofAddListener(udpDiscovery.peerArrivedEvent, this, &ofSurfaceHubApp::PeerArrived);
	ofAddListener(udpDiscovery.peerLeftEvent, this, &ofSurfaceHubApp::PeerLeft);
}