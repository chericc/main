#include "xstring.hpp"

std::list<std::string> xstring_split(const std::string& str, const std::string& token)
{
	std::list<std::string> list;

	size_t pos = 0;
	size_t token_size = token.size();

	for (;;)
	{
		if (pos >= str.size())
		{
			break;
		}

		size_t ret = str.find(token, pos);

		if (ret == std::string::npos)
		{
			std::string piece(str, pos);
			list.push_back(piece);
			break;
		}
		else
		{
			std::string piece(str, pos, ret - pos);
			list.push_back(piece);
			pos = ret + token_size;
		}
	}

	return list;
}