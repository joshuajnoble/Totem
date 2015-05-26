#include "ofMain.h"
#include "ofxArgParser\src\ofxArgParser.h"
#include "ofApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main(int argc, const char** argv)
{
	ofxArgParser::init(argc, argv);

	if (ofxArgParser::hasKey("help"))
	{
		std::cout << "Usage:" << std::endl <<
			" -listDevices" << endl <<
			" -showInput                  (Show the input video stream instead of the normal UI)" << endl <<
			" -noUnwrap                   (Send the raw video stream without unwarpping it)" << endl <<
			" -showUnwrapped              (Show the undistorted video stream instead of the normal UI)" << endl <<
			" -sourceFile=<path>          (Uses a test file instead of the camera for inpput)" << endl <<
			" -device=<device number>     (Only needed when there are multiple input devices)" << endl <<
			" -xMargin=<border size>      (Shifts the window left by this amount)" << endl <<
			" -yMargin=<border size>      (Shifts the window up by this amount)" << endl <<
			" -device=<device number>     (Only needed when there are multiple input devices)" << endl <<
			" -capWidth=<capture width>   (default=2048 pixels)" << endl <<
			" -capHeight=<capture height> (default=2048 pixels)" << endl;
		return 0;
	}
	else if (ofxArgParser::hasKey("listDevices"))
	{
		ofVideoGrabber grabber = ofVideoGrabber();
		grabber.listDevices();
		return 0;
	}
	else
	{
		if (ofxArgParser::hasKey("device") && ofxArgParser::getValue("device") == "")
		{
			std::cout << "The \"device\" param requires a device number.  See -help for details.";
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
		if (ofxArgParser::hasKey("sourceFile") && ofxArgParser::getValue("sourceFile") == "")
		{
			std::cout << "The \"sourceFile\" param requires a video file path.  See -help for details.";
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

	auto webCamDeviceId = 0;
	if (ofxArgParser::hasKey("device"))
	{
		webCamDeviceId = ofToInt(ofxArgParser::getValue("device"));
	}

	auto captureWidth = 2048;
	if (ofxArgParser::hasKey("capWidth"))
	{
		captureWidth = ofToInt(ofxArgParser::getValue("capWidth"));
	}

	auto captureHeight = 2048;
	if (ofxArgParser::hasKey("capHeight"))
	{
		captureHeight = ofToInt(ofxArgParser::getValue("capHeight"));
	}

	auto xMargin = 8;
	auto yMargin = 31;
	if (ofxArgParser::hasKey("xMargin"))
	{
		xMargin = ofToInt(ofxArgParser::getValue("xMargin"));
	}

	if (ofxArgParser::hasKey("yMargin"))
	{
		yMargin = ofToInt(ofxArgParser::getValue("yMargin"));
	}

	ofPtr<ofApp> app = ofPtr<ofApp>(new ofApp());

	ofAppGlutWindow window;
	if (ofxArgParser::hasKey("showInput"))
	{
		app->showInput = true;
		ofSetupOpenGL(&window, captureWidth, captureHeight, OF_WINDOW);
	}
	else if (ofxArgParser::hasKey("showUnwrapped"))
	{
		app->showUnwrapped = true;
		if (ofxArgParser::hasKey("noUnwrap"))
		{
			ofSetupOpenGL(&window, captureWidth, captureHeight, OF_WINDOW);
		}
		else
		{
			ofSetupOpenGL(&window, captureWidth * app->unwrapMultiplier, captureWidth * app->unwrapMultiplier * app->unwrapAspectRatio, OF_WINDOW);
		}
	}
	else
	{
		ofSetupOpenGL(&window, 800 * 4, 1280, OF_WINDOW);
	}

	// The capture device must be initialized after ofSetupOpenGL has been called, so it can allocate the proper internal buffers.
	ofPtr<ofBaseVideoDraws> videoSource;
	if (ofxArgParser::hasKey("sourceFile"))
	{
		auto filename = ofxArgParser::getValue("sourceFile");
		auto fullPath = ofToDataPath(filename);
		if (!ofFile::doesFileExist(fullPath))
		{
			cout << "The specified file, \"" << fullPath << "\" does not exist.";
			ofExit();
			return -1;
		}

		videoSource = ofApp::InitializeVideoPresenterFromFile(fullPath);
	}
	else
	{
		videoSource = ofApp::InitializePlayerFromCamera(webCamDeviceId, captureWidth, captureHeight);
	}

	app->passthroughVideo = ofxArgParser::hasKey("noUnwrap");
	app->videoSource = videoSource;

	//ofSetWindowPosition(-xMargin, -yMargin); // Position the window to not show the left and top window chrome.
	ofSetWindowPosition(0, 0);

	ofRunApp(app);
}
