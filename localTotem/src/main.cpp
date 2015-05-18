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
			" -sourceFile=<path>          (Uses a test file instead of the camera for inpput)" << endl <<
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
			return 0;
		}
		if (ofxArgParser::hasKey("capWidth") && ofxArgParser::getValue("capWidth") == "")
		{
			std::cout << "The \"capWidth\" param requires a width.  See -help for details.";
			return 0;
		}
		if (ofxArgParser::hasKey("capHeight") && ofxArgParser::getValue("capHeight") == "")
		{
			std::cout << "The \"capHeight\" param requires a height.  See -help for details.";
			return 0;
		}
		if (ofxArgParser::hasKey("sourceFile") && ofxArgParser::getValue("sourceFile") == "")
		{
			std::cout << "The \"sourceFile\" param requires a video file path.  See -help for details.";
			return 0;
		}
	}

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofPtr<ofApp> app = ofPtr<ofApp>(new ofApp());

	if (ofxArgParser::hasKey("sourceFile"))
	{
		std::string filename = ofxArgParser::getValue("sourceFile");
		app->useWebCam = false;
		app->videoFilename = filename;
	}
	else
	{
		if (ofxArgParser::hasKey("device"))
		{
			app->webCamDeviceId = atoi(ofxArgParser::getValue("device").c_str());
		}

		if (ofxArgParser::hasKey("capWidth"))
		{
			app->captureWidth = atoi(ofxArgParser::getValue("capWidth").c_str());
		}

		if (ofxArgParser::hasKey("capHeight"))
		{
			app->captureHeight = atoi(ofxArgParser::getValue("capHeight").c_str());
		}
	}

	ofAppGlutWindow window;
	if (ofxArgParser::hasKey("showInput"))
	{
		app->showInput = true;
		ofSetupOpenGL(&window, app->captureWidth, app->captureHeight, OF_WINDOW);
	}
	else
	{
		ofSetupOpenGL(&window, 3200, 490, OF_WINDOW);
	}

	ofSetWindowPosition(0, 0);

	ofRunApp(app);
}
