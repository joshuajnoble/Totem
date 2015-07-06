#include "RemoteNetworkDisplay.h"
#include "Utils.h"

namespace
{
	const auto VIDEO1_ENTRANCE_BEGIN = "VIDEO1_ENTRANCE_BEGIN";
	const auto VIDEO1_ENTRANCE_END = "VIDEO1_ENTRANCE_END";
	const auto VIDEO2_ENTRANCE_BEGIN = "VIDEO2_ENTRANCE_BEGIN";
	const auto VIDEO2_ENTRANCE_END = "VIDEO2_ENTRANCE_END";
	const auto VIDEO1_EXIT_BEGIN = "VIDEO1_EXIT_BEGIN";
	const auto VIDEO1_EXIT_END = "VIDEO1_EXIT_END";
	const auto VIDEO2_EXIT_BEGIN = "VIDEO2_EXIT_BEGIN";
	const auto VIDEO2_EXIT_END = "VIDEO2_EXIT_END";

	const auto AnimationSpeed = 500.0f;
	const int VIDEO_DIVIDER_SIZE = 10;
}

RemoteNetworkDisplay::RemoteNetworkDisplay()
{
}


RemoteNetworkDisplay::~RemoteNetworkDisplay()
{
}


void RemoteNetworkDisplay::initializeRemoteNetworkDisplay(ofRectangle v)
{
	this->viewport = v;
	this->drawRegion.setFromCenter(
		this->viewport.x + this->viewport.width / 2,
		this->viewport.y + this->viewport.height / 2,
		roundf(this->viewport.height * 0.7 * (10.0 / 16.0)),
		roundf(this->viewport.height * 0.7));

	this->SingleVideoRegion = this->drawRegion;
	this->FirstVideoRegion = this->drawRegion;
	this->FirstVideoRegion.height = (this->FirstVideoRegion.height / 2) - (VIDEO_DIVIDER_SIZE / 2);
	this->SecondVideoRegion = this->FirstVideoRegion;
	this->SecondVideoRegion.translateY(this->SecondVideoRegion.height + VIDEO_DIVIDER_SIZE);

	ofxKeyframeAnimRegisterEvents(this);
}


void RemoteNetworkDisplay::AddVideoSource(ofPtr<CroppedDrawable> source)
{
	this->videoSources.push_back(source);

	if (this->activeVideos.size() == 0)
	{
		auto activeVideo = ofPtr<ActiveVideo>(new ActiveVideo());
		activeVideo->alpha = 0.0f;
		activeVideo->videoSource = source;
		activeVideo->windowType = WINDOW_TYPE_Single;
		activeVideo->currentRegion = this->SingleVideoRegion;
		activeVideo->removing = false;
		this->activeVideos.push_back(activeVideo);
		auto test = this->activeVideos[0];
		auto aresame = &test == &activeVideo;

		this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed, &activeVideo->currentRegion.x, activeVideo->currentRegion.x));
		this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &activeVideo->alpha, 1.0f));

		// Position the video off the right side of the screen, so it can scroll onto the screen.
		activeVideo->currentRegion.x = this->viewport.getRight();
	}
	else if (this->activeVideos.size() == 1)
	{
		auto activeVideo = ofPtr<ActiveVideo>(new ActiveVideo());
		activeVideo->alpha = 0.0f;
		activeVideo->videoSource = source;
		activeVideo->windowType = WINDOW_TYPE_Second;
		activeVideo->currentRegion = this->SecondVideoRegion;
		activeVideo->removing = false;
		this->activeVideos.push_back(activeVideo);

		if (this->activeVideos[0]->windowType == WINDOW_TYPE_Single)
		{
			this->activeVideos[0]->windowType = WINDOW_TYPE_First;

			// Shrink the first video
			this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->activeVideos[0]->currentRegion.height, this->FirstVideoRegion.height));

			// Slide in the second video
			this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &activeVideo->currentRegion.x, activeVideo->currentRegion.x));
			this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &activeVideo->alpha, 1.0f));

			activeVideo->currentRegion.x = this->viewport.getRight();
		}
	}
}

void RemoteNetworkDisplay::RemoveFirstVideoSource()
{
	auto iter = this->GetActiveWindwoByType(WINDOW_TYPE_First);
	if (iter != this->activeVideos.end())
	{
		RemoveVideoSource((*iter)->videoSource);
	}
	else
	{
		iter = this->GetActiveWindwoByType(WINDOW_TYPE_Single);
		if (iter != this->activeVideos.end())
		{
			RemoveVideoSource((*iter)->videoSource);
		}
	}
}


void RemoteNetworkDisplay::RemoveSecondVideoSource()
{
	auto iter = this->GetActiveWindwoByType(WINDOW_TYPE_Second);
	if (iter != this->activeVideos.end())
	{
		RemoveVideoSource((*iter)->videoSource);
	}
	else
	{
		iter = this->GetActiveWindwoByType(WINDOW_TYPE_Single);
		if (iter != this->activeVideos.end())
		{
			RemoveVideoSource((*iter)->videoSource);
		}
	}
}


void RemoteNetworkDisplay::RemoveVideoSource(ofPtr<CroppedDrawable> source)
{
	auto found = std::find(this->videoSources.begin(), this->videoSources.end(), source);
	if (found != this->videoSources.end())
	{
		this->videoSources.erase(found);
	}

	for (auto iter = this->activeVideos.begin(); iter != this->activeVideos.end(); ++iter)
	{
		auto activeVideo = *iter;

		if (activeVideo->videoSource == source)
		{
			// Slide this video off the screen;
			activeVideo->removing = true;
			this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed, &activeVideo->currentRegion.x, this->viewport.getRight()));
			this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &activeVideo->alpha, 0.0f));

			if (activeVideo->windowType == WINDOW_TYPE_Single)
			{
			}
			else if (activeVideo->windowType == WINDOW_TYPE_First)
			{
				auto secondWindow = GetActiveWindwoByType(WINDOW_TYPE_Second);
				if (secondWindow != this->activeVideos.end())
				{
					(*secondWindow)->windowType = WINDOW_TYPE_Single;
					this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &(*secondWindow)->currentRegion.x, this->SingleVideoRegion.x));
					this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &(*secondWindow)->currentRegion.y, this->SingleVideoRegion.y));
					this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &(*secondWindow)->currentRegion.width, this->SingleVideoRegion.width));
					this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &(*secondWindow)->currentRegion.height, this->SingleVideoRegion.height));
				}
			}
			else if (activeVideo->windowType == WINDOW_TYPE_Second)
			{
				auto firstWindow = GetActiveWindwoByType(WINDOW_TYPE_First);
				if (firstWindow != this->activeVideos.end())
				{
					(*firstWindow)->windowType = WINDOW_TYPE_Single;
					this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &(*firstWindow)->currentRegion.x, this->SingleVideoRegion.x));
					this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &(*firstWindow)->currentRegion.y, this->SingleVideoRegion.y));
					this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &(*firstWindow)->currentRegion.width, this->SingleVideoRegion.width));
					this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &(*firstWindow)->currentRegion.height, this->SingleVideoRegion.height));
				}
			}
			else
			{
				this->activeVideos.erase(iter);
			}

			return;
		}
	}
}


void RemoteNetworkDisplay::update()
{
	this->playlist.update();

	for (auto iter = this->activeVideos.begin(); iter != this->activeVideos.end(); ++iter)
	{
		auto activeVideo = *iter;
		if (activeVideo->removing)
		{
			auto current = activeVideo->currentRegion;
			auto right = this->viewport.getRight();
			if (activeVideo->currentRegion.x >= this->viewport.getRight())
			{
				this->activeVideos.erase(iter);
				break; // We modified the vector, so we can't keep iterating -- just check again on the next update loop.
			}
		}
	}
}


void RemoteNetworkDisplay::draw()
{
	if (this->activeVideos.size())
	{
		ofPushStyle();

		ofSetColor(0x02, 0x49, 0x70);
		ofRect(this->viewport);

		for (auto iter = this->activeVideos.begin(); iter != this->activeVideos.end(); ++iter)
		{
			auto activeVideo = *iter;

			ofPushMatrix();
			ofTranslate(activeVideo->currentRegion.x, activeVideo->currentRegion.y);
			ofSetColor(255, 255, 255, 255 * activeVideo->alpha);
			activeVideo->videoSource->DrawCropped((int)activeVideo->currentRegion.width, (int)activeVideo->currentRegion.height);
			ofPopMatrix();
		}

		ofPopStyle();
	}
}

void RemoteNetworkDisplay::onKeyframe(ofxPlaylistEventArgs& args)
{
	//if (args.message == VIDEO1_ENTRANCE_BEGIN)
	//{
	//	this->animatingVideo1Entrance = true;
	//}
	//else if (args.message == VIDEO1_ENTRANCE_END)
	//{
	//	this->animatingVideo1Entrance = false;
	//}
	//else if (args.message == VIDEO2_ENTRANCE_BEGIN)
	//{
	//	this->animatingVideo2Entrance = true;
	//}
	//else if (args.message == VIDEO2_ENTRANCE_END)
	//{
	//	this->animatingVideo2Entrance = false;
	//}
	//else if (args.message == VIDEO1_EXIT_BEGIN)
	//{
	//	this->animatingVideo1Exit = true;
	//}
	//else if (args.message == VIDEO1_EXIT_END)
	//{
	//	if (this->videoSources.size() == 1)
	//	{
	//		auto erase = this->videoSources.begin();
	//		this->videoSources.erase(erase);
	//	}
	//	this->animatingVideo1Exit = false;
	//}
	//else if (args.message == VIDEO2_EXIT_BEGIN)
	//{
	//	this->animatingVideo2Exit = true;
	//}
	//else if (args.message == VIDEO2_EXIT_END)
	//{
	//	if (this->videoSources.size() > 1)
	//	{
	//		auto erase = ++this->videoSources.begin();
	//		this->videoSources.erase(erase);
	//	}
	//	this->animatingVideo2Exit = false;
	//}
}


RemoteNetworkDisplay::activeVideoList::iterator RemoteNetworkDisplay::GetActiveWindwoByType(ACTIVE_WINDOW_TYPE windowType)
{
	for (auto iter = this->activeVideos.begin(); iter != this->activeVideos.end(); ++iter)
	{
		if ((*iter)->windowType == windowType)
		{
			return iter;
		}
	}

	return this->activeVideos.end();
}