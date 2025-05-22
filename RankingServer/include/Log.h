#pragma once
#include <iostream>
#include <string>

namespace Utility
{
	static void Debug(const std::string space, const std::string object, const std::string value)
	{
		std::string result = "[" + space +"] [" + object + "] " + value;
		std::cout << result << std::endl;
	}
}