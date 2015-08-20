#include "ofSurfaceHubApp.h"
#include "..\..\SharedCode\VideoConverters.h"

ofSurfaceHubApp::ofSurfaceHubApp() :
	PeopleRegion(61, 137, 332, 869),
	PeopleMargin(24, 26),
	slidePositionOpen(410, 92, 1448, 915),
	slidePositionClosed(62, 0, 1858, 1006),
	hasSessionStarted(false)
{

}

void ofSurfaceHubApp::earlyinit(int netid)
{
	this->networkInterfaceId = netid;
}

int ofSurfaceHubApp::displayWidth() const
{
	int maxWidth = 1920;
	auto width = GetSystemMetrics(SM_CXSCREEN);
	return width > maxWidth ? maxWidth : width;
}

int ofSurfaceHubApp::displayHeight() const
{
	int maxHeight = 1080;
	auto height = GetSystemMetrics(SM_CYSCREEN);
	return height > maxHeight ? maxHeight : height;
}

void ofSurfaceHubApp::setup()
{
	ofSetFrameRate(30);
	ofSetVerticalSync(true);

	StatusFont.loadFont("surfacehub/segoeui.ttf", 11);
	StatusFont.setSpaceSize(StatusFont.getSpaceSize() / 2);
	NameFont.loadFont("surfacehub/segoeui.ttf", 15);
	auto xx = NameFont.getSpaceSize();
	NameFont.setSpaceSize(NameFont.getSpaceSize() / 2);
	TitleFont = StatusFont;

	avatarMask.loadImage("surfacehub/avatar_mask.png");
	backgroundOpen.loadImage("surfacehub/background_open.png");
	backgroundClosed.loadImage("surfacehub/background_closed.png");
	slide.loadImage("surfacehub/slide.png");
	connectButton.loadImage("hub_connect.png");

	Person p;
	p.name = "Albert Russell";
	p.title = "???";
	p.avatar = ofImage("profiles/AlbertRussell.jpg");
	people.push_back(p);

	p.name = "Brandon Gray";
	p.title = "Project Manager";
	p.avatar = ofImage("profiles/BrandonGray.jpg");
	people.push_back(p);

	p.name = "Brenda Bryant";
	p.title = "Senior Product Designer";
	p.avatar = ofImage("profiles/BrendaBryant.jpg");
	people.push_back(p);

	p.name = "Carolyn Hill";
	p.title = "Textile Designer";
	p.avatar = ofImage("profiles/CarolynHill.jpg");
	people.push_back(p);

	p.name = "Christine Spencer";
	p.title = "Administrative Assistant";
	p.avatar = ofImage("profiles/ChristineSpencer.jpg");
	people.push_back(p);

	p.name = "Donald Lewis";
	p.title = "Retail Planning Manager";
	p.avatar = ofImage("profiles/DonaldLewis.jpg");
	people.push_back(p);

	p.name = "Edward James";
	p.title = "Product Manager";
	p.avatar = ofImage("profiles/EdwardJames.jpg");
	people.push_back(p);

	p.name = "Kathryn Hill";
	p.title = "Strategic Analyst";
	p.avatar = ofImage("profiles/KathrynHill.jpg");
	people.push_back(p);

	p.name = "Patrick Arnold";
	p.title = "Brand Manager";
	p.avatar = ofImage("profiles/PatrickArnold.jpg");
	people.push_back(p);

	p.name = "Thomas Kelly";
	p.title = "Senior CAD Designer";
	p.avatar = ofImage("profiles/ThomasKelly.jpg");
	people.push_back(p);

	std::random_shuffle(people.begin(), people.end());
	
	SetupDiscovery();

#if _DEBUGX
	UdpDiscovery::RemotePeerStatus s;
	s.isTotem = false;
	s.id = "01"; PeerArrived(s); s.id = "02"; PeerArrived(s); s.id = "03"; PeerArrived(s); s.id = "04"; PeerArrived(s); s.id = "05"; PeerArrived(s);
	s.id = "06"; PeerArrived(s); s.id = "07"; PeerArrived(s); s.id = "08"; PeerArrived(s); s.id = "09"; PeerArrived(s); s.id = "10"; PeerArrived(s);
#endif
}

void ofSurfaceHubApp::update()
{
	udpDiscovery.update();
}

void ofSurfaceHubApp::draw()
{
	if (hasSessionStarted)
	{
		// Draw the background
		ofRectangle fill(0, 0, backgroundClosed.width, backgroundClosed.height);
		fill.scaleTo(ofRectangle(0, 0, this->displayWidth(), this->displayHeight()), OF_SCALEMODE_FILL);
		backgroundClosed.draw(fill);

		// Draw the the slide
		ofRect(slidePositionClosed); // Only need this if the background isn't already white
		fill = ofRectangle(slidePositionClosed.x, slidePositionClosed.y, slide.width, slide.height);
		fill.scaleTo(slidePositionClosed);
		fill.scaleTo(slidePositionClosed, OF_SCALEMODE_CENTER);
		slide.draw(fill);
	}
	else
	{
		// Draw the background
		ofRectangle fill(0, 0, backgroundOpen.width, backgroundOpen.height);
		fill.scaleTo(ofRectangle(0, 0, this->displayWidth(), this->displayHeight()), OF_SCALEMODE_FILL);
		backgroundOpen.draw(fill);

		// Draw the the slide
		//ofRect(slidePositionOpen); // Only need this if the background isn't already white
		fill = ofRectangle(slidePositionOpen.x, slidePositionOpen.y, slide.width, slide.height);
		fill.scaleTo(slidePositionOpen);
		slide.draw(fill);

		// Draw the profile images
		ofPushMatrix();
		auto topLeft = PeopleRegion.getTopLeft() + PeopleMargin;
		ofTranslate(topLeft);

		int y = -13;

		std::vector<int> peersOnline;
		std::vector<int> peersConnected;
		for (auto i = 0; i < peers.size(); ++i)
		{
			auto p = peers[i];
			if (p.isConnectedToSession)
			{
				peersConnected.push_back(i);
			}
			else
			{
				peersOnline.push_back(i);
			}
		}

		if (peersOnline.size())
		{
			y += 28;
			StatusFont.drawString("Online", 0, y);
			y += 15;
			for (int i = 0; i < peersOnline.size(); ++i)
			{
				Person &p = peers[peersOnline[i]].person;
				DrawPerson(p, 0, y);
				y += PROFILE_SIZE + PROFILE_PADDING;
			}
		}

		if (peersConnected.size())
		{
			y += 28;
			StatusFont.drawString("Alpine Vista - 56/2300", 0, y);
			y += 15;
			for (int i = 0; i < peersConnected.size(); ++i)
			{
				Person &p = peers[peersConnected[i]].person;
				DrawPerson(p, 0, y);
				y += PROFILE_SIZE + PROFILE_PADDING;
			}
		}
	}

	//y += 28;
	//StatusFont.drawString("Not yet joined", 0, y);
	//y += 15;
	//for (int i = 5; i < peers.size(); ++i)
	//{
	//	Person &p = peers[i].person;
	//	DrawPerson(p, 0, y);
	//	y += PROFILE_SIZE + PROFILE_PADDING;
	//}

	//hasTotemConnected = true; // TODO: Remove this when we are ready to really test with a totem source
	//if (peers.size() && hasTotemConnected)
	//{
	//	buttonPosition.set(PROFILE_PADDING, y, PROFILE_SIZE, PROFILE_SIZE);
	//	connectButton.draw(buttonPosition);
	//}

	ofPopMatrix();
}

void ofSurfaceHubApp::DrawPerson(Person& p, float x, float y)
{
	p.avatar.draw(x + 1, y, PROFILE_SIZE, PROFILE_SIZE);
	avatarMask.draw(x + 1, y);

	auto nameY = y + NameFont.getLineHeight();
	NameFont.drawString(p.name, x + PROFILE_SIZE + 13, nameY);
	ofSetColor(0x62, 0x7D, 0x85);
	TitleFont.drawString(p.title, x + PROFILE_SIZE + 13, nameY + TitleFont.getLineHeight() + 4);
	ofSetColor(0xFF);
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
		entry.isConnectedToSession = peer.isConnectedToSession;
		entry.person = people.front();
		people.erase(people.begin());
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
		people.push_back(found->person);
		peers.erase(found);

		if (peer.isTotem)
		{
			hasTotemConnected = false;
		}
	}
}

void ofSurfaceHubApp::PeerJoinedSession(UdpDiscovery::RemotePeerStatus& peer)
{
	auto found = GetPeerFromClientId(peer.id);
	if (found != this->peers.end())
	{
		found->isConnectedToSession = true;
	}
}

void ofSurfaceHubApp::PeerLeftSession(UdpDiscovery::RemotePeerStatus& peer)
{
	auto found = GetPeerFromClientId(peer.id);
	if (found != this->peers.end())
	{
		found->isConnectedToSession = false;
	}
}

void ofSurfaceHubApp::SetupDiscovery()
{
	this->udpDiscovery.setupSurfaceHub();
	ofAddListener(udpDiscovery.peerArrivedEvent, this, &ofSurfaceHubApp::PeerArrived);
	ofAddListener(udpDiscovery.peerLeftEvent, this, &ofSurfaceHubApp::PeerLeft);

	ofAddListener(udpDiscovery.peerJoinedSessionEvent, this, &ofSurfaceHubApp::PeerJoinedSession);
	ofAddListener(udpDiscovery.peerLeftSessionEvent, this, &ofSurfaceHubApp::PeerLeftSession);
}

void ofSurfaceHubApp::mousePressed(int x, int y, int button)
{
	if (button == 0 && buttonPosition.inside(x, y))
	{
		// The connect button has been pressed
	}
}

std::vector<ofSurfaceHubApp::PeerInfo>::iterator ofSurfaceHubApp::GetPeerFromClientId(const string& clientId)
{
	auto found = std::find_if(this->peers.begin(), this->peers.end(), [clientId](PeerInfo &x)->bool { return x.id == clientId; });
	return found;
}