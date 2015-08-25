#include "ofxArgParser.h"
#include <ctype.h>

namespace
{
	std::string strToLower(const std::string& a)
	{
		unsigned int sz = a.size();
		std::string rval(a);
		for (unsigned int i = 0; i < sz; ++i)
		{
			rval[i] = (char)tolower(a[i]);
		}

		return rval;
	}
}

typedef std::map<std::string, std::string> ArgsMapType;
ArgsMapType ofxArgParser::args;

void ofxArgParser::init(int argc, const char** argv)
{
	int n = 0;
	while (++n < argc)
	{
		std::string param = argv[n];
		if (param.length() >= 2 && param[0] == '-')
		{
			param = param.substr(1);
			while (param.length() >= 2 && param[0] == '-')
			{
				param = param.substr(1);
			}

			// got a param, now look for a key/value syntax
			size_t equalsIndex = param.find('=');
			if (equalsIndex == -1)
			{
				args[strToLower(param)] = "";
			}
			else
			{
				args[strToLower(param.substr(0, equalsIndex))] = param.substr(equalsIndex + 1);
			}
		}
	}
}

bool ofxArgParser::hasKey(const std::string& key)
{
	return args.find(strToLower(key)) != args.end();
}

std::vector<std::string> ofxArgParser::allKeys()
{
	std::vector<std::string> result;

	std::map<std::string, std::string>::iterator it = args.begin();
	while (it != args.end())
	{
		result.push_back((*it).first);
		it++;
	}

	return result;
}

std::string ofxArgParser::getValue(const std::string& key)
{
	ArgsMapType::iterator found = args.find(strToLower(key));
	return found->second;
}