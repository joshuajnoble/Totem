#include "utils.h"

template <class T>
void Utils::DrawCroppedToFit(T& source, int displayWidth, int displayHeight)
{
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	auto cropRegion = Utils::CalculateCroppedRegion(displayWidth, displayHeight, sourceWidth, sourceHeight);
	source.getTextureReference().drawSubsection(0, 0, displayWidth, displayHeight, cropRegion.x, cropRegion.y, cropRegion.width, cropRegion.height);
}

void Utils::DrawCroppedToFit(ofImage& source, int displayWidth, int displayHeight)
{
	Utils::DrawCroppedToFit(source, 0, 0, displayWidth, displayHeight);
}

void Utils::DrawCroppedToFit(ofImage& source, int x, int y, int displayWidth, int displayHeight)
{
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	auto cropRegion = Utils::CalculateCroppedRegion(displayWidth, displayHeight, sourceWidth, sourceHeight);
	source.getTextureReference().drawSubsection(x, y, displayWidth, displayHeight, cropRegion.x, cropRegion.y, cropRegion.width, cropRegion.height);
}

void Utils::DrawCroppedToFit(ofFbo& source, int displayWidth, int displayHeight)
{
	Utils::DrawCroppedToFit(source, 0, 0, displayWidth, displayHeight);
}

void Utils::DrawCroppedToFit(ofFbo& source, int x, int y, int displayWidth, int displayHeight)
{
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	auto cropRegion = Utils::CalculateCroppedRegion(displayWidth, displayHeight, sourceWidth, sourceHeight);
	source.getTextureReference().drawSubsection(x, y, displayWidth, displayHeight, cropRegion.x, cropRegion.y, cropRegion.width, cropRegion.height);
}

void Utils::DrawCroppedToFit(ofBaseVideoDraws& source, ofRectangle area)
{
	Utils::DrawCroppedToFit(source, area.x, area.y, area.width, area.height);
}

void Utils::DrawCroppedToFit(ofBaseVideoDraws& source, int displayWidth, int displayHeight)
{
	Utils::DrawCroppedToFit(source, 0, 0, displayWidth, displayHeight);
}

void Utils::DrawCroppedToFit(ofBaseVideoDraws& source, int x, int y, int displayWidth, int displayHeight)
{
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	auto cropRegion = Utils::CalculateCroppedRegion(displayWidth, displayHeight, sourceWidth, sourceHeight);
	source.getTextureReference().drawSubsection(x, y, displayWidth, displayHeight, cropRegion.x, cropRegion.y, cropRegion.width, cropRegion.height);
}

ofRectangle Utils::CalculateCroppedRegion(int displayWidth, int displayHeight, int sourceWidth, int sourceHeight)
{
	float displayRatio = displayWidth / static_cast<float>(displayHeight);
	ofRectangle cropRegion;
	if (sourceHeight * displayRatio <= sourceWidth)
	{
		auto cropWidth = sourceHeight * displayRatio;
		cropRegion = ofRectangle((sourceWidth - cropWidth) / 2, 0, cropWidth, sourceHeight);
	}
	else
	{
		auto cropHeight = sourceWidth / displayRatio;
		cropRegion = ofRectangle(0, (sourceHeight - cropHeight) / 2, sourceWidth, cropHeight);
	}

	return cropRegion;
}

ofPtr<ofBaseVideoDraws> Utils::CreateVideoSourceFromFile(std::string path)
{
	ofVideoPlayer* player = new ofVideoPlayer();
	ofPtr<ofBaseVideoDraws> rval = ofPtr<ofBaseVideoDraws>(player);
	if (player->loadMovie(path))
	{
		player->setLoopState(OF_LOOP_NORMAL);
		player->play();
	}

	return rval;
}

ofPtr<ofBaseVideoDraws> Utils::CreateVideoSourceFromCamera(int deviceId, int width, int height)
{
	ofVideoGrabber *grabber = new ofVideoGrabber();
	ofPtr<ofVideoGrabber> rval = ofPtr<ofVideoGrabber>(grabber);
	if (deviceId != 0)
	{
		grabber->setDeviceID(deviceId);
	}

	grabber->initGrabber(width, height);
	return rval;
}


shared_ptr<ofxBaseKeyframe> ofxFunctionKeyframe::create(std::function<void()> c)
{
	return shared_ptr<ofxBaseKeyframe>(new ofxFunctionKeyframe(c));
}

// ----------------------------------------------------------------------

void ofxFunctionKeyframe::start(){
	is_idle = FALSE;     // bool value to give notice that the keyframe is done with.
	(isFrameBased) ? startValue = 0 : startValue = ofGetSystemTime();
	step = 0;
	hasStarted = TRUE;
};

// ----------------------------------------------------------------------

void ofxFunctionKeyframe::execute(){
	// call the Event as soon as possible.
	// call event here
	if (is_idle == FALSE) {

		if (hasStarted == FALSE) start();

		step = isFrameBased ? labs(startValue) : labs(ofGetSystemTime() - startValue);
		if (isFrameBased) startValue++;		// increase frame count if the animation is by-frame

		delayHasEnded();	// check whether delay has ended.

		if (isDelayed) return;
		// ----------| invariant: delay has ended, we are ready to execute:

		this->_callback();
		// ofxCoreKeyframeEvents Ev;
#ifdef PLAYLIST_DEBUG_MODE
		ofLog(OF_LOG_VERBOSE) << ofToString(ofGetFrameNum()) << ": EventKeyframe calling event callback";
#endif
		is_idle = TRUE;	// get rid of it.
	}
}

// ----------------------------------------------------------------------

bool ofxFunctionKeyframe::delayHasEnded() {
	if (isDelayed && (step >= delay_steps)) {
		isDelayed = false;
		return true;
	}
	return false;
};
