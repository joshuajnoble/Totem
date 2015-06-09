#include "ofMain.h"
#include "ofxArgParser\src\ofxArgParser.h"
#include "ofTotemApp.h"
#include "ofRemoteApp.h"
#include "ofAppGlutWindow.h"

namespace
{
	float unwrapMultiplier = 1.5f;
	float unwrapAspectRatio = 0.25f;

	ofPtr<ofBaseVideoDraws> InitializeVideoPresenterFromFile(std::string path)
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

	ofPtr<ofBaseVideoDraws> InitializePlayerFromCamera(int deviceId, int width, int height)
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

	ofPtr<ofBaseVideoDraws> CreateVideoSource(int webCamDeviceId, int captureWidth, int captureHeight)
	{
		if (ofxArgParser::hasKey("capSource"))
		{
			auto filename = ofxArgParser::getValue("capSource");
			auto fullPath = ofToDataPath(filename);
			if (!ofFile::doesFileExist(fullPath))
			{
				cout << "The specified file, \"" << fullPath << "\" does not exist.";
				ofExit();
			}

			return InitializeVideoPresenterFromFile(fullPath);
		}
		else
		{
			return InitializePlayerFromCamera(webCamDeviceId, captureWidth, captureHeight);
		}
	}

	ofPtr<VideoCaptureAppBase> CreateTotemAppInstance(ofAppGlutWindow& window, int captureWidth, int captureHeight)
	{
		auto totemApp = new ofTotemApp();
		totemApp->earlyinit();

		if (ofxArgParser::hasKey("showInput"))
		{
			ofSetupOpenGL(&window, captureWidth, captureHeight, OF_WINDOW);
		}
		else if (ofxArgParser::hasKey("showUnwrapped"))
		{
			totemApp->showUnwrapped = true;
			if (ofxArgParser::hasKey("dontUnwrap"))
			{
				ofSetupOpenGL(&window, captureWidth, captureHeight, OF_WINDOW);
			}
			else
			{
				ofSetupOpenGL(&window, captureWidth * unwrapMultiplier, captureWidth * unwrapMultiplier * unwrapAspectRatio, OF_WINDOW);
			}
		}
		else
		{
			ofSetupOpenGL(&window, totemApp->displayWidth(), totemApp->displayHeight(), OF_WINDOW);
		}

		return ofPtr<VideoCaptureAppBase>(totemApp);
	}

	ofPtr<VideoCaptureAppBase> CreateRemoteAppInstance(ofAppGlutWindow& window)
	{
		auto remoteApp = new ofRemoteApp();
		remoteApp->earlyinit();
		ofSetupOpenGL(&window, remoteApp->displayWidth(), remoteApp->displayHeight(), OF_WINDOW);

		if (ofxArgParser::hasKey("netSource"))
		{
			auto filename = ofxArgParser::getValue("netSource");
			auto fullPath = ofToDataPath(filename);
			if (!ofFile::doesFileExist(fullPath))
			{
				cout << "The specified file, \"" << fullPath << "\" does not exist.";
				ofExit();
			}

			auto videoSource = InitializeVideoPresenterFromFile(fullPath);

			auto unwrapper = new ThreeSixtyUnwrap();
			unwrapper->initUnwrapper(videoSource, videoSource->getWidth() * unwrapMultiplier, videoSource->getWidth() * unwrapMultiplier * unwrapAspectRatio);
			remoteApp->RegisterTotemVideoSource(ofPtr<ofBaseVideoDraws>(unwrapper));
		}

		return ofPtr<VideoCaptureAppBase>(remoteApp);
	}
}

//========================================================================
int main(int argc, const char** argv)
{
	ofxArgParser::init(argc, argv);

	if (ofxArgParser::hasKey("help"))
	{
		std::cout << "Usage:" << std::endl <<
			" -totem          (This is the default and will use the totem display)" << endl <<
			"  -dontUnwrap    (Send the raw video stream without unwrapping it)" << endl <<
			"  -showUnwrapped (Show the undistorted video stream instead of the normal UI)" << endl <<
			"  -showInput     (Show the raw input video stream instead of the normal UI)" << endl <<

			endl << " -remote             (Don't use the totem display)" << endl <<
			"  -netSource=<path>  (Uses a test file instead of a remote network connection)" << endl <<

			endl << " WINDOW SETTINGS" << endl <<
			" -xMargin=<border size> (Shifts the window left by this amount)" << endl <<
			" -yMargin=<border size> (Shifts the window up by this amount)" << endl <<

			endl << " CAPTURE SETTINGS" << endl <<
			"  -capSource=<path>           (Uses a test file instead of the camera for input)" << endl <<
			"  -capDevice=<device number>  (Only needed when there are multiple capture devices)" << endl <<
			"  -capDeviceList              (Shows all available capture devices)" << endl <<
			"  -capWidth=<capture width>   (default=2048(totem)/1920(remote) pixels)" << endl <<
			"  -capHeight=<capture height> (default=2048(totem)/1080(remote) pixels)" << endl;
		return 0;
	}
	else if (ofxArgParser::hasKey("capDeviceList"))
	{
		ofVideoGrabber grabber = ofVideoGrabber();
		grabber.listDevices();
		return 0;
	}
	else
	{
		if (ofxArgParser::hasKey("totem") && ofxArgParser::hasKey("remote"))
		{
			std::cout << "You must specify *either* -totem or -remote; not both.  See -help for details.";
			return -1;
		}
		if (ofxArgParser::hasKey("netSource") && ofxArgParser::getValue("netSource") == "")
		{
			std::cout << "The \"netSource\" param requires a video file path.  See -help for details.";
			return -1;
		}
		if (ofxArgParser::hasKey("capDevice") && ofxArgParser::getValue("capDevice") == "")
		{
			std::cout << "The \"capDevice\" param requires a device number.  See -help for details.";
			return -1;
		}
		if (ofxArgParser::hasKey("capWidth") && ofxArgParser::getValue("capWidth") == "")
		{
			std::cout << "The \"capWidth\" param requires a width.  See -help for details.";
			return -1;
		}
		if (ofxArgParser::hasKey("capHeight") && ofxArgParser::getValue("capHeight") == "")
		{
			std::cout << "The \"capHeight\" param requires a height.  See -help for details.";
			return -1;
		}
		if (ofxArgParser::hasKey("capSource") && ofxArgParser::getValue("capSource") == "")
		{
			std::cout << "The \"capSource\" param requires a video file path.  See -help for details.";
			return -1;
		}
		if (ofxArgParser::hasKey("xMargin") && ofxArgParser::getValue("xMargin") == "")
		{
			std::cout << "The \"xMargin\" param requires a value.  See -help for details.";
			return -1;
		}
		if (ofxArgParser::hasKey("yMargin") && ofxArgParser::getValue("yMargin") == "")
		{
			std::cout << "The \"yMargin\" param requires a value.  See -help for details.";
			return -1;
		}
	}

	bool totemMode = !ofxArgParser::hasKey("remote");
	auto xMargin = totemMode ? 8 : 0;
	auto yMargin = totemMode ? 31 : 0;
	if (ofxArgParser::hasKey("xMargin"))
	{
		xMargin = ofToInt(ofxArgParser::getValue("xMargin"));
	}

	if (ofxArgParser::hasKey("yMargin"))
	{
		yMargin = ofToInt(ofxArgParser::getValue("yMargin"));
	}

	auto webCamDeviceId = 0;
	if (ofxArgParser::hasKey("capDevice"))
	{
		webCamDeviceId = ofToInt(ofxArgParser::getValue("capDevice"));
	}

	auto captureWidth = totemMode ? 2048 : 1920;
	if (ofxArgParser::hasKey("capWidth"))
	{
		captureWidth = ofToInt(ofxArgParser::getValue("capWidth"));
	}

	auto captureHeight = totemMode ? 2048 : 1080;
	if (ofxArgParser::hasKey("capHeight"))
	{
		captureHeight = ofToInt(ofxArgParser::getValue("capHeight"));
	}

	ofAppGlutWindow window;
	auto app = totemMode ? CreateTotemAppInstance(window, captureWidth, captureHeight) : CreateRemoteAppInstance(window);
	auto videoSource = CreateVideoSource(webCamDeviceId, captureWidth, captureHeight);
	if (totemMode && ofxArgParser::hasKey("showInput"))
	{
		auto totemApp = (ofTotemApp*)app.get();
		totemApp->rawSource = videoSource;
	}

	if (!totemMode || ofxArgParser::hasKey("dontUnwrap"))
	{
		app->videoSource = videoSource;
	}
	else
	{
		auto unwrapper = new ThreeSixtyUnwrap();
		unwrapper->initUnwrapper(videoSource, videoSource->getWidth() * unwrapMultiplier, videoSource->getWidth() * unwrapMultiplier * unwrapAspectRatio);
		app->videoSource = ofPtr<ofBaseVideoDraws>(unwrapper);
	}

	ofSetWindowPosition(-xMargin, -yMargin);
	ofRunApp(app);
}
