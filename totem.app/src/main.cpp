#include "ofMain.h"
#include "ofxPGR\src\PGRCamera.h"
#include "ofxArgParser\src\ofxArgParser.h"
#include "ofTotemApp.h"
#include "ofRemoteApp.h"
#include "ofSurfaceHubApp.h"
#include "Utils.h"
#include "..\..\sharedCode\VideoConverters.h"

namespace
{
	enum TotemMode
	{
		TotemMode_Totem,
		TotemMode_Remote,
		TotemMode_SurfaceHub
	};

	const ofVec2f UNWRAPPED_DISPLAYRATIO(4.85, 1.0);

	ofPtr<ofBaseVideoDraws> CreateVideoSource()
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
			auto webCamDeviceId = 0;
			if (ofxArgParser::hasKey("capDevice"))
			{
				webCamDeviceId = ofToInt(ofxArgParser::getValue("capDevice"));
			}

			auto captureWidth = 640;
			if (ofxArgParser::hasKey("capWidth"))
			{
				captureWidth = ofToInt(ofxArgParser::getValue("capWidth"));
			}

			auto captureHeight = 480;
			if (ofxArgParser::hasKey("capHeight"))
			{
				captureHeight = ofToInt(ofxArgParser::getValue("capHeight"));
			}

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

	void RemoveWindowChrome(int windowWidth, int windowHeight)
	{
#if WIN32
		auto hwnd = ofGetWin32Window();
		auto lStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
		SetWindowLongPtr(hwnd, GWL_STYLE, lStyle);

		auto lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, lExStyle);

		SetWindowPos(hwnd, NULL, 0, 0, windowWidth, windowHeight, SWP_FRAMECHANGED | /*SWP_NOMOVE | SWP_NOSIZE |*/ SWP_NOZORDER | SWP_NOOWNERZORDER);
#else
		ofSetWindowShape(windowWidth, windowHeight);
#endif
	}

	ofPtr<IVideoCaptureAppBase> CreateTotemAppInstance(int networkInterfaceId)
	{
		int captureWidth = 2160;
		int captureHeight = captureWidth;

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
			RemoveWindowChrome((int)totemApp->displayWidth(), (int)totemApp->displayHeight());
		}

		ofPtr<ofBaseVideoDraws> videoSource;
		if (ofxArgParser::hasKey("capSource"))
		{
			videoSource = CreateVideoSource();
		}
		else
		{
			auto camera = new PGRCamera();
			if (!camera->setup())
			{
				cout << "There was an error initializing the PointGrey camera.";
				return ofPtr<IVideoCaptureAppBase>();
			}

			videoSource = ofPtr<ofBaseVideoDraws>(camera);
		}

		if (ofxArgParser::hasKey("showInput") || ofxArgParser::hasKey("dontUnwrap"))
		{
			totemApp->rawSource = videoSource;
		}
		else
		{
			auto unwrapper = new ThreeSixtyUnwrap();
			auto outputSize = ThreeSixtyUnwrap::CalculateUnwrappedSize(ofVec2f(videoSource->getPixelsRef().getWidth(), videoSource->getPixelsRef().getHeight()), UNWRAPPED_DISPLAYRATIO);
			unwrapper->initUnwrapper(videoSource, outputSize);
			videoSource = ofPtr<ofBaseVideoDraws>(unwrapper);
		}

		totemApp->videoSource = videoSource;

		if (ofxArgParser::hasKey("netSource"))
		{
			// Use a file as a network video source
			auto filename = ofxArgParser::getValue("netSource");
			auto fullPath = ofToDataPath(filename);
			if (!ofFile::doesFileExist(fullPath))
			{
				cout << "The specified file, \"" << fullPath << "\" does not exist.";
				return ofPtr<IVideoCaptureAppBase>();
			}

			auto videoSource = Utils::CreateVideoSourceFromFile(fullPath);
			totemApp->ImporsonateRemoteClient(videoSource);
		}

		return ofPtr<IVideoCaptureAppBase>(totemApp);
	}

	ofPtr<IVideoCaptureAppBase> CreateRemoteAppInstance(int networkInterfaceId)
	{
		auto remoteApp = new ofRemoteApp();

		ofSetupOpenGL(1, 1, OF_WINDOW);
		auto screenWidth = min(1920, ofGetScreenWidth());
		auto screenHeight = min(1080, ofGetScreenHeight());

		int windowWidth = screenWidth;
		int windowHeight = screenHeight;

		if (ofxArgParser::hasKey("display"))
		{
			auto displayMode = ofxArgParser::getValue("display");
			if (displayMode == "16x9") ForceDisplayToBestFitOf(16, 9, windowWidth, windowHeight);
			else if (displayMode == "16x10") ForceDisplayToBestFitOf(16, 10, windowWidth, windowHeight);
			else if (displayMode == "3x2") ForceDisplayToBestFitOf(3, 2, windowWidth, windowHeight);
			else if (displayMode == "720")
			{
				windowWidth = 1280;
				windowHeight = 720;
			}
			else if (displayMode == "768")
			{
				windowWidth = 1366;
				windowHeight = 768;
			}
			else if (displayMode == "1080")
			{
				windowWidth = 1920;
				windowHeight = 1080;
			}
		}

		RemoveWindowChrome(windowWidth, windowHeight);

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

			auto unwrappedVideo = ofPtr<ofBaseVideoDraws>(unwrapper);
			auto drawableVideoSource = ofPtr<CroppedDrawable>(new CroppedDrawableVideoDraws(unwrappedVideo));
			RemoteVideoInfo remote;
			remote.peerStatus.id = "remoteTotemImpersonator";
			//remote.videoSource = drawableVideoSource;
			remote.peerStatus.videoWidth = unwrappedVideo->getWidth();
			remote.peerStatus.videoHeight = unwrappedVideo->getHeight();
			remote.peerStatus.isTotem = true;
			remote.videoDraws = unwrappedVideo;
			remoteApp->NewConnection(remote);
			//remoteApp->Handle_ClientStreamAvailable(remote);
		}

		ofPtr<ofBaseVideoDraws> videoSource = CreateVideoSource();
		remoteApp->videoSource = videoSource;

		return ofPtr<IVideoCaptureAppBase>(remoteApp);
	}

	ofPtr<IVideoCaptureAppBase> CreateHubAppInstance(int networkInterfaceId)
	{
		auto app = new ofSurfaceHubApp();
		app->earlyinit(networkInterfaceId);
		ofSetupOpenGL((int)app->displayWidth(), (int)app->displayHeight(), OF_WINDOW);
		RemoveWindowChrome((int)app->displayWidth(), (int)app->displayHeight());
		return ofPtr<IVideoCaptureAppBase>(app);
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
			"  -display=<mode>     (Force the window to a specific size or aspect ratio)" << endl <<
			"                      (supported modes: 16x10, 10x9, 3x2, 768, 1080)" << endl <<

			endl <<
			"-hub       (Run as an emulated Surface Hub)" << endl <<

			endl << "NETWORK SETTINGS" << endl <<
			"-netList                     (Show all network interfaces)" << endl <<
			"-netInterface=<interface id> (Select a specific network interface)" << endl <<

			endl << "CAPTURE SETTINGS" << endl <<
			"-capSource=<path>           (Use a test file instead of the camera for input)" << endl <<
			"-capDevice=<device number>  (Only needed when there are multiple capture devices)" << endl <<
			"-capDeviceList              (Show all available capture devices)" << endl <<
			"-capWidth=<capture width>   (default=640 pixels)" << endl <<
			"-capHeight=<capture height> (default=480 pixels)" << endl;
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
		auto modeCount = ofxArgParser::hasKey("totem") ? 1 : 0;
		modeCount += ofxArgParser::hasKey("remote") ? 1 : 0;
		modeCount += ofxArgParser::hasKey("hub") ? 1 : 0;
		if (modeCount == 0)
		{
			std::cout << "You must specify which mode to run in, one of: -totem, -remote, -hub (-help for more details).";
			return -1;
		}

		if (modeCount > 1)
		{
			std::cout << "You must specify only one of: -totem, -remote, -hub.  (-help for more details).";
			return -1;
		}

		vector<string> requireParams = { "totemSource", "netSource", "capDevice", "capWidth", "capHeight", "capSource", "netInterface", "display" };
		for (auto i = requireParams.begin(); i != requireParams.end(); ++i)
		{
			if (ofxArgParser::hasKey(*i) && ofxArgParser::getValue(*i) == "")
			{
				std::cout << "The \"" << *i << "\" param requires a value to be provided.  (-help for more details).";
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

	InitFFmpeg();

	TotemMode totemMode;
	if (ofxArgParser::hasKey("totem")) totemMode = TotemMode_Totem;
	else if (ofxArgParser::hasKey("remote")) totemMode = TotemMode_Remote;
	else if (ofxArgParser::hasKey("hub")) totemMode = TotemMode_SurfaceHub;

	auto networkId = -1;
	if (ofxArgParser::hasKey("netInterface"))
	{
		networkId = ofToInt(ofxArgParser::getValue("netInterface"));
	}

	ofPtr<IVideoCaptureAppBase> app;
	switch (totemMode)
	{
	case TotemMode_Totem: app = CreateTotemAppInstance(networkId); break;
	case TotemMode_Remote: app = CreateRemoteAppInstance(networkId); break;
	case TotemMode_SurfaceHub: app = CreateHubAppInstance(networkId); break;
	}

	if (!app)
	{
		return -1;
	}

	ofRunApp(app);
}