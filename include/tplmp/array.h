#ifndef _TPLMP_ARRAY
#define _TPLMP_ARRAY

#include <tplmp/base.h>

namespace tplmp
{
//数组元素初始化
template<size_t _Index, typename _T>
__attribute__((always_inline)) inline constexpr _T&& element(_T&& e) noexcept
{
	return forward<_T>(e);
}

template<size_t _Size, typename _T, size_t _Index>
struct __fill_array_impl
{
	inline static void fill(_T (&arr)[_Size], _T&& val)
	{
		arr[_Index] = val;
		__fill_array_impl<_Size, _T, _Index + 1>::fill(arr, val);
	}
};

template<size_t _Size, typename _T>
struct __fill_array_impl<_Size, _T, _Size>
{
	inline static void fill(_T (&)[_Size], _T&&)
	{
	}
};

template<size_t _Size, typename _T>
inline void fill_array(_T (&arr)[_Size], _T&& val)
{
	__fill_array_impl<_Size, _T, 0>::fill(arr, forward<_T>(val));
}

}

#endif//_TPLMP_ARRAY
