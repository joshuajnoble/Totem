#pragma once

#include "ofMain.h"
#include "ofxPlaylist/src/ofxEventKeyframe.h"
#include <cctype>
#include <locale>

class Utils
{
public:
	template <class T>
	static void DrawCroppedToFit(T& source, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofImage& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofImage& source, int x, int y, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofFbo& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofFbo& source, int x, int y, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofBaseVideoDraws& source, ofRectangle area);
	static void DrawCroppedToFit(ofBaseVideoDraws& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofBaseVideoDraws& source, int x, int y, int displayWidth, int displayHeight);

	static ofRectangle CalculateCroppedRegion(int displayWidth, int displayHeight, int sourceWidth, int sourceHeight);
	
	static ofPtr<ofBaseVideoDraws> CreateVideoSourceFromCamera(int deviceId, int width, int height);
	static ofPtr<ofBaseVideoDraws> CreateVideoSourceFromFile(std::string path);
};

class ofxFunctionKeyframe : public ofxBaseKeyframe {
	bool isDelayed;
	const int delay_steps;

	long startValue;
	const bool isFrameBased;
	int step;

	const std::function<void()> _callback;

public:
	static shared_ptr<ofxBaseKeyframe> create(std::function<void()> callback);

	ofxFunctionKeyframe(const std::function<void()>& callback)
		: isDelayed(FALSE)
		, delay_steps(0)
		, startValue(0)
		, isFrameBased(TRUE)
		, step(0)
		, _callback(callback)
	{							// initialise with message .
		is_idle = FALSE;
		hasStarted = FALSE;
	};

	void start();
	void execute();
	bool delayHasEnded();
	int getDuration()	{ return delay_steps; };
};

template<typename charT>
struct upper_compare {
	upper_compare(const std::locale &loc) : locale(loc) {}
	bool operator()(charT ch1, charT ch2)
	{
		return std::toupper(ch1, this->locale) == std::toupper(ch2, this->locale);
	}
private:
	const std::locale& locale;
};

template<typename T>
int find_indexof_substr_nocase(const T& string1, const T& string2, const std::locale& locale = std::locale())
{
	auto found = std::search(string1.begin(), string1.end(), string2.begin(), string2.end(), upper_compare<typename T::value_type>(locale));
	if (found != string1.end())
	{
		return found - string1.begin();
	}
	else
	{
		return -1;
	}
}