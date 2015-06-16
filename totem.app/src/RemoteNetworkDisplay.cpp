#include "RemoteNetworkDisplay.h"
#include "Utils.h"

namespace
{
	const auto SingleVideoSize = ofRectangle(0, 0, 650, 750);
	const auto DoubleVideoSize = ofRectangle(0, 0, 650, 420);

	const auto VIDEO1_ENTRANCE_BEGIN = "VIDEO1_ENTRANCE_BEGIN";
	const auto VIDEO1_ENTRANCE_END = "VIDEO1_ENTRANCE_END";
	const auto VIDEO2_ENTRANCE_BEGIN = "VIDEO2_ENTRANCE_BEGIN";
	const auto VIDEO2_ENTRANCE_END = "VIDEO2_ENTRANCE_END";
	const auto VIDEO1_EXIT_BEGIN = "VIDEO1_EXIT_BEGIN";
	const auto VIDEO1_EXIT_END = "VIDEO1_EXIT_END";
	const auto VIDEO2_EXIT_BEGIN = "VIDEO2_EXIT_BEGIN";
	const auto VIDEO2_EXIT_END = "VIDEO2_EXIT_END";

	const auto AnimationSpeed = 500.0f;
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
	ofxKeyframeAnimRegisterEvents(this);
}


bool RemoteNetworkDisplay::AddVideoSource(ofPtr<ofFbo> source)
{
	if (!this->CanModify())
	{
		return false;
	}

	this->videoSources.push_back(source);
	if (this->videoSources.size() == 1)
	{
		// Slide the video in from the right
		this->video1Height = SingleVideoSize.height;
		this->video1Left = viewport.width + 100;
		this->video1Alpha = 0.0f;
		this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed, &this->video1Left, 0.0f));
		this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &this->video1Alpha, 1.0f));
		this->playlist.addToKeyFrame(Playlist::Action::event(this, VIDEO1_ENTRANCE_BEGIN));
		this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_ENTRANCE_END));
	}
	else if (this->videoSources.size() == 2)
	{
		// Slide the new video in from the bottom (and make some room)
		this->video2Top = 570;
		this->video2Alpha = 0.0f;
		this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video2Top, 340));
		this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video2Alpha, 1.0f));

		this->video1Height = SingleVideoSize.height;
		this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video1Height, DoubleVideoSize.height));
		this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video2Top, 0));
		this->playlist.addToKeyFrame(Playlist::Action::event(this, VIDEO2_ENTRANCE_BEGIN));
		this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO2_ENTRANCE_END));
		this->animatingVideo2Entrance = true;
	}
}


bool RemoteNetworkDisplay::RemoveVideoSource(ofPtr<ofFbo> source)
{
	if (!this->CanModify())
	{
		return false;
	}

	auto found = std::find(this->videoSources.begin(), this->videoSources.end(), source);
	if (found != this->videoSources.end())
	{
		// For the sake of animations, check which item is being removed
		if (found == this->videoSources.begin())
		{
			if (this->videoSources.size() == 1)
			{
				this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_EXIT_BEGIN));
				this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &this->video1Left, viewport.width + 100));
				this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &this->video1Alpha, 0.0f));
				this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_EXIT_END));
			}
			else
			{
				// TODO: We don't have an animation for removing the top of two videos, yet.
				this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_EXIT_BEGIN));
				this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_EXIT_END));
			}
		}
		else
		{
			// Just undo the transition we did above if we are removing the second video source
			this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video2Top, 340));
			this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video1Height, SingleVideoSize.height));

			this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video2Top, 570));
			this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video2Alpha, 0.0f));
			this->playlist.addToKeyFrame(Playlist::Action::event(this, VIDEO2_EXIT_BEGIN));
			this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO2_EXIT_END));
		}
	}

	return true;
}


void RemoteNetworkDisplay::update()
{
	this->playlist.update();
}


void RemoteNetworkDisplay::draw()
{
	if (this->videoSources.size())
	{
		ofPushStyle();
		
		if (this->videoSources.size() >= 1)
		{
			auto region = SingleVideoSize; 
			region.translate(this->viewport.getPosition());
			
			auto source = this->videoSources[0];
			ofPushMatrix();
			ofTranslate(region.x + this->video1Left, region.y);
			ofSetColor(255, 255, 255, 255 * this->video1Alpha);
			Utils::DrawImageCroppedToFit(*source, (int)region.width, (int)this->video1Height);
			ofPopMatrix();
		}

		if (!this->animatingVideo1Entrance && this->videoSources.size() > 1)
		{
			auto region = DoubleVideoSize;
			region.translate(this->viewport.getPosition());
			
			region.translate(0, region.getHeight() + 20);
			auto source = this->videoSources[1];
			ofPushMatrix();
			ofSetColor(255, 255, 255, 255 * this->video2Alpha);
			ofTranslate(region.x, region.y + this->video2Top);
			Utils::DrawImageCroppedToFit(*source, (int)region.width, (int)region.height);
			ofPopMatrix();
		}

		ofPopStyle();
	}
}

bool RemoteNetworkDisplay::CanModify()
{
	return !(this->animatingVideo1Entrance || this->animatingVideo1Exit || this->animatingVideo2Entrance || this->animatingVideo2Exit);
}

void RemoteNetworkDisplay::onKeyframe(ofxPlaylistEventArgs& args)
{
	if (args.message == VIDEO1_ENTRANCE_BEGIN)
	{
		this->animatingVideo1Entrance = true;
	}
	else if (args.message == VIDEO1_ENTRANCE_END)
	{
		this->animatingVideo1Entrance = false;
	}
	else if (args.message == VIDEO2_ENTRANCE_BEGIN)
	{
		this->animatingVideo2Entrance = true;
	}
	else if (args.message == VIDEO2_ENTRANCE_END)
	{
		this->animatingVideo2Entrance = false;
	}
	else if (args.message == VIDEO1_EXIT_BEGIN)
	{
		this->animatingVideo1Exit = true;
	}
	else if (args.message == VIDEO1_EXIT_END)
	{
		if (this->videoSources.size() == 1)
		{
			auto erase = this->videoSources.begin();
			this->videoSources.erase(erase);
		}
		this->animatingVideo1Exit = false;
	}
	else if (args.message == VIDEO2_EXIT_BEGIN)
	{
		this->animatingVideo2Exit = true;
	}
	else if (args.message == VIDEO2_EXIT_END)
	{
		if (this->videoSources.size() > 1)
		{
			auto erase = ++this->videoSources.begin();
			this->videoSources.erase(erase);
		}
		this->animatingVideo2Exit = false;
	}
}