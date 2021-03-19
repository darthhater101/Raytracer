#pragma once
#include <string>
#include <fstream>
#include <sstream>

static std::string read_file(std::string path)
{
	std::ifstream file(path);
	std::string str;
	if (file)
	{
		std::ostringstream ss;
		ss << file.rdbuf();
		str = ss.str();
	}
	return str;
}
