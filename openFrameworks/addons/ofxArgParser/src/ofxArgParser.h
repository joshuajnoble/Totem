#pragma once

#include <vector>
#include <map>

class ofxArgParser
{
public:
	
	static void init(int argc, const char** argv);
	static bool hasKey(const std::string& key);
	static std::vector<std::string> allKeys();
	static std::string getValue(const std::string& key);

protected:
	static std::map<std::string, std::string> args;
};
