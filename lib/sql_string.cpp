#include "sql_string.h"

using namespace std;

namespace mysqlpp {

SQLString::SQLString() :
is_string(false),
dont_escape(false),
processed(false)
{
}

SQLString::SQLString(const string& str) :
string(str),
is_string(true),
dont_escape(false),
processed(false)
{
}

SQLString::SQLString(const char* str) : 
string(str),
is_string(true),
dont_escape(false),
processed(false)
{
}

SQLString::SQLString(char i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[6];
	snprintf(s, sizeof(s), "%hd", static_cast<short int>(i));
	*this = s;
}

SQLString::SQLString(unsigned char i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[6];
	snprintf(s, sizeof(s), "%hu", static_cast<short int>(i));
	*this = s;
}

SQLString::SQLString(short int i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[6];
	snprintf(s, sizeof(s), "%hd", i);
	*this = s;
}

SQLString::SQLString(unsigned short int i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[6];
	snprintf(s, sizeof(s), "%hu", i);
	*this = s;
}

SQLString::SQLString(int i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[22];
	snprintf(s, sizeof(s), "%d", i);
	*this = s;
}

SQLString::SQLString(unsigned int i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[22];
	snprintf(s, sizeof(s), "%u", i);
	*this = s;
}

SQLString::SQLString(longlong i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[22];
	snprintf(s, sizeof(s), "%lld", i); 
	*this = s;
}

SQLString::SQLString(ulonglong i) :
is_string(false),
dont_escape(false),
processed(false) 
{
	char s[22];
	snprintf(s, sizeof(s), "%llu", i);
	*this = s;
}

SQLString::SQLString(float i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[40];
	snprintf(s, sizeof(s), "%g", i);
	*this = s;
}

SQLString::SQLString(double i) :
is_string(false),
dont_escape(false),
processed(false)
{
	char s[40];
	snprintf(s, sizeof(s), "%g", i);
	*this = s;
}

} // end namespace mysqlpp

