#include "ofMain.h"
#include "ofxArgParser\src\ofxArgParser.h"
#include "ofTotemApp.h"
#include "ofRemoteApp.h"
#include "Utils.h"
#include "ofxPGR\src\PGRCamera.h"

namespace
{
	const ofVec2f UNWRAPPED_DISPLAYRATIO(4.85, 1.0);

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

			return Utils::CreateVideoSourceFromFile(fullPath);
		}
		else
		{
			return Utils::CreateVideoSourceFromCamera(webCamDeviceId, captureWidth, captureHeight);
		}
	}

	void ForceDisplayToBestFitOf(int num, int den, int &width, int&height)
	{
		//float displayRatio = width / static_cast<float>(height);
		float displayRatio = num / static_cast<float>(den);
		if (height * displayRatio <= width)
		{
			width = height * displayRatio;
		}
		else
		{
			height = width / displayRatio;
		}
	}

	ofPtr<VideoCaptureAppBase> CreateTotemAppInstance(int captureWidth, int captureHeight, int networkInterfaceId)
	{
		auto totemApp = new ofTotemApp();
		totemApp->earlyinit(networkInterfaceId);

		if (ofxArgParser::hasKey("showInput"))
		{
			ofSetupOpenGL(captureWidth, captureHeight, OF_WINDOW);
		}
		else if (ofxArgParser::hasKey("showOutput"))
		{
			totemApp->showOutput = true;
			if (ofxArgParser::hasKey("dontUnwrap"))
			{
				ofSetupOpenGL(captureWidth, captureHeight, OF_WINDOW);
			}
			else
			{
				auto outputSize = ThreeSixtyUnwrap::CalculateUnwrappedSize(ofVec2f(captureWidth, captureHeight), UNWRAPPED_DISPLAYRATIO);
				ofSetupOpenGL(outputSize.x, outputSize.y, OF_WINDOW);
			}
		}
		else
		{
			ofSetupOpenGL((int)totemApp->displayWidth(), (int)totemApp->displayHeight(), OF_WINDOW);
		}

		return ofPtr<VideoCaptureAppBase>(totemApp);
	}

	ofPtr<VideoCaptureAppBase> CreateRemoteAppInstance(int networkInterfaceId)
	{
		auto remoteApp = new ofRemoteApp();

		ofSetupOpenGL(10, 10, OF_WINDOW);
		auto screenWidth = min(2160, ofGetScreenWidth());
		auto screenHeight = min(1440, ofGetScreenHeight());

		int windowWidth = screenWidth;
		int windowHeight = screenHeight;

		if (ofxArgParser::hasKey("display16by9"))
		{
			ForceDisplayToBestFitOf(16, 9, windowWidth, windowHeight);
		}
		else if (ofxArgParser::hasKey("display16by10"))
		{
			ForceDisplayToBestFitOf(16, 10, windowWidth, windowHeight);
		}
		else if (ofxArgParser::hasKey("display3by2"))
		{
			ForceDisplayToBestFitOf(3, 2, windowWidth, windowHeight);
		}

		ofSetWindowShape(windowWidth, windowHeight);

		remoteApp->earlyinit(networkInterfaceId, windowWidth, windowHeight);
		//ofSetupOpenGL(windowWidth, windowHeight, OF_WINDOW);

		if (ofxArgParser::hasKey("totemSource"))
		{
			auto filename = ofxArgParser::getValue("totemSource");
			auto fullPath = ofToDataPath(filename);
			if (!ofFile::doesFileExist(fullPath))
			{
				cout << "The specified file, \"" << fullPath << "\" does not exist.";
				ofExit();
			}

			auto videoSource = Utils::CreateVideoSourceFromFile(fullPath);
			auto unwrapper = new ThreeSixtyUnwrap();
			auto outputSize = ThreeSixtyUnwrap::CalculateUnwrappedSize(ofVec2f(videoSource->getWidth(), videoSource->getHeight()), UNWRAPPED_DISPLAYRATIO);
			unwrapper->initUnwrapper(videoSource, outputSize);
			remoteApp->ImpersonateRemoteConnection("localTotemImpersonator", ofPtr<ofBaseVideoDraws>(unwrapper));
		}

		return ofPtr<VideoCaptureAppBase>(remoteApp);
	}
}

//========================================================================
int main(int argc, const char** argv)
{
	ofSetWorkingDirectoryToDefault();

	//ofSetLogLevel(OF_LOG_VERBOSE);
	//ofLogToFile("logfile-mempool.txt", true);

	ofxArgParser::init(argc, argv);

	if (ofxArgParser::hasKey("help"))
	{
		std::cout << "PRIMARY SETTINGS" << std::endl <<
			"-totem              (This is the default and will use the totem display)" << endl <<
			"  -dontUnwrap       (Send the raw video stream without unwrapping it)" << endl <<
			"  -showOutput       (Show the undistorted video stream instead of the normal UI)" << endl <<
			"  -showInput        (Show the raw input video stream instead of the normal UI)" << endl <<
			"  -netSource=<path> (Use a test file instead of the remote network connection)" << endl <<

			endl <<
			"-remote               (Don't use the totem display)" << endl <<
			"  -totemSource=<path> (Use a test file instead of a remote totem network connection)" << endl <<
			"  -display16by10      (Force the window to a 16:9 aspect ratio)" << endl <<
			"  -display16by9       (Force the window to a 16:9 aspect ratio)" << endl <<
			"  -display3by2        (Force the window to a 16:9 aspect ratio)" << endl <<

			endl << "WINDOW SETTINGS" << endl <<
			"-xMargin=<border size> (Shift the window left by this amount)" << endl <<
			"-yMargin=<border size> (Shift the window up by this amount)" << endl <<

			endl << "NETWORK SETTINGS" << endl <<
			"-netList                     (Show all network interfaces)" << endl <<
			"-netInterface=<interface id> (Select a specific network interface)" << endl <<

			endl << "CAPTURE SETTINGS" << endl <<
			"-capSource=<path>           (Use a test file instead of the camera for input)" << endl <<
			"-capDevice=<device number>  (Only needed when there are multiple capture devices)" << endl <<
			"-capDeviceList              (Show all available capture devices)" << endl <<
			"-capWidth=<capture width>   (default=2160(totem)/1280(remote) pixels)" << endl <<
			"-capHeight=<capture height> (default=2160(totem)/720(remote) pixels)" << endl;
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

		vector<string> requireParams = { "totemSource", "netSource", "capDevice", "capWidth", "capHeight", "capSource", "xMargin", "yMargin", "netInterface" };
		for (auto i = requireParams.begin(); i != requireParams.end(); ++i)
		{
			if (ofxArgParser::hasKey(*i) && ofxArgParser::getValue(*i) == "")
			{
				std::cout << "The \"" << *i << "\" param requires a value to be provided.  See -help for details.";
				return -1;
			}
		}
	}

	auto interfaces = UdpDiscovery::GetAllNetworkInterfaces();
	if (!interfaces.size())
	{
		cout << "There are no network interfaces available on this computer.";
		return -1;
	}

	if (ofxArgParser::hasKey("netList"))
	{
		const int displayNameLengthMax = 39;
		std::cout << left << setw(4) << "ID" << setw(displayNameLengthMax + 1) << "Interface Name" << setw(3 * 4 + 3 + 1) << "IP Address" << "MAC Address" << std::endl;
		for (auto iter = interfaces.begin(); iter != interfaces.end(); ++iter)
		{
			auto i = *iter;
			auto mac = i.macAddress();
			auto displayName = string(i.displayName());
			if (displayName.length() > displayNameLengthMax)
			{
				displayName.resize(displayNameLengthMax);
			}

			std::cout << left << setw(4) << i.index() << setw(displayNameLengthMax + 1) << displayName << setw(3 * 4 + 3 + 1) << i.address().toString() << UdpDiscovery::MACtoString(i.macAddress()) << std::endl;
		}

		return 0;
	}

	bool totemMode = !ofxArgParser::hasKey("remote");
	auto xMargin = totemMode ? 0 : 0;
	auto yMargin = totemMode ? 0 : 0;
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

	auto captureWidth = totemMode ? 2160 : 1280;
	if (ofxArgParser::hasKey("capWidth"))
	{
		captureWidth = ofToInt(ofxArgParser::getValue("capWidth"));
	}

	auto captureHeight = totemMode ? 2160 : 720;
	if (ofxArgParser::hasKey("capHeight"))
	{
		captureHeight = ofToInt(ofxArgParser::getValue("capHeight"));
	}

	auto networkId = -1;
	if (ofxArgParser::hasKey("netInterface"))
	{
		networkId = ofToInt(ofxArgParser::getValue("netInterface"));
	}

	auto app = totemMode ? CreateTotemAppInstance(captureWidth, captureHeight, networkId) : CreateRemoteAppInstance(networkId);

	ofPtr<ofBaseVideoDraws> videoSource;
	if (!totemMode || ofxArgParser::hasKey("capDevice") || ofxArgParser::hasKey("capSource") || ofxArgParser::hasKey("capWidth") || ofxArgParser::hasKey("capHeight"))
	{
		videoSource = CreateVideoSource(webCamDeviceId, captureWidth, captureHeight);
	}
	else
	{
		auto camera = new PGRCamera();
		if (!camera->setup())
		{
			cout << "There was an error initializing the PointGrey camera.";
			return -1;
		}

		videoSource = ofPtr<ofBaseVideoDraws>(camera);
	}

	if (totemMode)
	{
		auto totemApp = (ofTotemApp*)app.get();
		if (ofxArgParser::hasKey("showInput"))
		{
			totemApp->rawSource = videoSource;
		}
		else if (ofxArgParser::hasKey("netSource"))
		{
			// Use a file as a network video source
			auto filename = ofxArgParser::getValue("netSource");
			auto fullPath = ofToDataPath(filename);
			if (!ofFile::doesFileExist(fullPath))
			{
				cout << "The specified file, \"" << fullPath << "\" does not exist.";
				return -1;
			}

			auto videoSource = Utils::CreateVideoSourceFromFile(fullPath);
			totemApp->ImporsonateRemoteClient(videoSource);
		}
	}

	if (!totemMode || ofxArgParser::hasKey("dontUnwrap"))
	{
		app->videoSource = videoSource;
	}
	else
	{
		auto unwrapper = new ThreeSixtyUnwrap();
		auto outputSize = ThreeSixtyUnwrap::CalculateUnwrappedSize(ofVec2f(videoSource->getPixelsRef().getWidth(), videoSource->getPixelsRef().getHeight()), UNWRAPPED_DISPLAYRATIO);
		unwrapper->initUnwrapper(videoSource, outputSize);
		app->videoSource = ofPtr<ofBaseVideoDraws>(unwrapper);
	}

	ofSetWindowPosition(-xMargin, -yMargin);
	ofRunApp(app);
}
