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

	this->SingleVideoSize = ofRectangle(0, 0, this->drawRegion.width, this->drawRegion.height);
	this->DoubleVideoSize = ofRectangle(0, 0, this->drawRegion.width, this->drawRegion.height / 2 - VIDEO_DIVIDER_SIZE / 2);

	ofxKeyframeAnimRegisterEvents(this);
}


bool RemoteNetworkDisplay::AddVideoSource(ofPtr<CroppedDrawable> source)
{
	if (!this->CanModify())
	{
		return false;
	}

	this->videoSources.push_back(source);
	if (this->videoSources.size() == 1)
	{
		// Slide the video in from the right
		this->currentVideoPosition1 = ofRectangle(SingleVideoSize);
		this->currentVideoPosition1.translate(this->drawRegion.getPosition());
		this->video1Alpha = 0.0f;
		this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_ENTRANCE_BEGIN));
		this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &this->currentVideoPosition1.x, this->currentVideoPosition1.x));
		this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &this->video1Alpha, 1.0f));
		this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_ENTRANCE_END));

		this->currentVideoPosition1.x = this->drawRegion.getRight();
		return true;
	}
	else if (this->videoSources.size() == 2)
	{
		this->animatingVideo2Entrance = true;

		// Shrink the first video
		this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO2_ENTRANCE_BEGIN));
		this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->currentVideoPosition1.height, DoubleVideoSize.height));

		// Slide in the second video
		this->currentVideoPosition2 = ofRectangle(DoubleVideoSize);
		this->currentVideoPosition2.translate(this->drawRegion.getPosition());
		this->currentVideoPosition2.translateY(this->drawRegion.height / 2 + VIDEO_DIVIDER_SIZE / 2);
		this->video2Alpha = 0.0f;
		this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->currentVideoPosition2.x, this->currentVideoPosition2.x));
		this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video2Alpha, 1.0f));
		this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO2_ENTRANCE_END));

		this->currentVideoPosition2.x = this->drawRegion.getRight();
		return true;
	}

	return false;
}


bool RemoteNetworkDisplay::RemoveVideoSource(ofPtr<CroppedDrawable> source)
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
				this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &this->currentVideoPosition1.x, this->viewport.getRight()));
				this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed, &this->video1Alpha, 0.0f));
				this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_EXIT_END));
			}
			else
			{
				// TODO: We have to swap which video is video 1 and which is video 2 after (or maybe during) the animation.
				this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_EXIT_BEGIN));
				this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->currentVideoPosition1.x, this->viewport.getRight()));
				this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video1Alpha, 1.0f));

				this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->currentVideoPosition2.height, SingleVideoSize.height));
				this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->currentVideoPosition2.y, this->drawRegion.y));
				this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO1_EXIT_END));
			}
		}
		else
		{
			// Just undo the transition we did above if we are removing the second video source
			this->playlist.addKeyFrame(Playlist::Action::event(this, VIDEO2_EXIT_BEGIN));
			this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->currentVideoPosition2.x, this->viewport.getRight()));
			this->playlist.addToKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->video2Alpha, 1.0f));

			this->playlist.addKeyFrame(Playlist::Action::tween(AnimationSpeed / 2, &this->currentVideoPosition1.height, SingleVideoSize.height));
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

		ofSetColor(0x02, 0x49, 0x70);
		ofRect(this->viewport);

		if (this->videoSources.size() >= 1)
		{
			auto source = this->videoSources[0];
			ofPushMatrix();
			ofTranslate(this->currentVideoPosition1.x, this->currentVideoPosition1.y);
			ofSetColor(255, 255, 255, 255 * this->video1Alpha);
			source->DrawCropped((int)this->currentVideoPosition1.width, (int)this->currentVideoPosition1.height);
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
			ofTranslate(this->currentVideoPosition2.x, this->currentVideoPosition2.y);
			source->DrawCropped((int)this->currentVideoPosition2.width, (int)this->currentVideoPosition2.height);
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