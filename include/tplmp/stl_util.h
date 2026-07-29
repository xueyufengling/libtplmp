#ifndef _TPLMP_STLUTIL
#define _TPLMP_STLUTIL

#include <string.h>

namespace tplmp
{

struct cstr_djb2_hash_op
{
	size_t operator()(const char* s) const noexcept
	{
		size_t hash = 5381;
		while(unsigned char c = (unsigned char)(*s++))
			hash = ((hash << 5) + hash) + c;
		return hash;
	}
};

struct cstr_equal_op
{
	bool operator()(const char* str1, const char* str2) const noexcept
	{
		return !strcmp(str1, str2);
	}
};

}

#endif//_TPLMP_STLUTIL
