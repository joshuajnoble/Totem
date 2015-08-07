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

class FrameRateGate
{
public:
	FrameRateGate(int framesPerSecond) : timeDelta(framesPerSecond / 1.0) {};
	FrameRateGate(float frameTime) : timeDelta(frameTime) {};

	bool IsGateOpen()
	{
		auto time = ofGetElapsedTimef();
		if (this->nextTime == -1)
		{
			this->nextTime = time + this->timeDelta;
			return false;
		}
		else if (time >= this->nextTime)
		{
			this->nextTime = time + this->timeDelta;
			return true;
		}

		return false;
	}

private:
	const float timeDelta;
	float startTime = -1;
	float nextTime = -1;
};

class RingBuffer
{
public:

	uint8_t *pWriter;
	uint8_t *pReader;
	uint8_t *pEnd;
	volatile LONG written;
	std::vector<uint8_t> buffer;

	RingBuffer(int size) : buffer(size), written(0)
	{
		pWriter = pReader = buffer.data();
		pEnd = pWriter + size;
		memset(pWriter, 0, size);
	}

	void Write(const uint8_t* source, int cbSource)
	{
		while (cbSource)
		{
			auto remaining = pEnd - pWriter;
			auto toWrite = cbSource > remaining ? remaining : cbSource;
			memcpy(pWriter, source, toWrite);
			InterlockedExchangeAdd(&written, toWrite);
			cbSource -= toWrite;
			pWriter += toWrite;
			if (pWriter == pEnd)
			{
				pWriter = buffer.data();
			}
		}
	}

	int Read(uint8_t* output, int cbOutput)
	{
		int outputWritten = 0;
		int requested = written < cbOutput ? written : cbOutput;
		while (outputWritten < requested)
		{
			auto srcRequested = requested - outputWritten;
			auto remaining = pEnd - pReader;
			auto toWrite = srcRequested < remaining ? srcRequested : remaining;
			memcpy(output + outputWritten, pReader, toWrite);
			InterlockedExchangeAdd(&written, -toWrite);
			outputWritten += toWrite;
			pReader += toWrite;
			if (pReader == pEnd)
			{
				pReader = buffer.data();
			}
		}

		return outputWritten;
	}
};


long long milliseconds_now();