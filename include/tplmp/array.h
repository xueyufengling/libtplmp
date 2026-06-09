#ifndef _TPLMP_ARRAY
#define _TPLMP_ARRAY

#include <tplmp/base.h>

namespace tplmp
{

/**
 * @brief 子定长数组引用
 */
//@formatter:off
template<size_t _Begin, size_t _End, typename _T, size_t _Size>
inline constexpr _T (& slice(_T (&arr)[_Size]))[_End - _Begin]
{
	static_assert(is_index_valid(_Begin, _Size) && is_index_valid(_End, _Size) && _Begin <= _End, "invalid begin and end index");
	return (_T (&)[_End - _Begin])*(((_T*)&arr) + _Begin);
}
//@formatter:on
template<
size_t _Index,
typename _FirstType, typename ..._RestTypes>
struct __at_impl
{
	inline static _FirstType value(_FirstType first, _RestTypes ...rest)
	{
		return __at_impl<_Index, _RestTypes...>::value(rest...);
	}
};

template<typename _FirstType, typename ..._RestTypes>
struct __at_impl<0, _FirstType, _RestTypes...>
{
	inline static _FirstType value(_FirstType first, _RestTypes ...rest)
	{
		return first;
	}
};

/**
 * @brief 函数模板变长实参包取值
 */
template<size_t _Index, typename ... _ParamTypes>
inline auto at(_ParamTypes ...params) -> decltype(__at_impl<_Index, _ParamTypes...>::value(params...))
{
	return __at_impl<_Index, _ParamTypes...>::value(params...);
}

#define __eval_array_assign_expr__(eval_name, arr, indexes_tp, init_expr)\
	__eval_pack_expr__(eval_name, arr[indexes_tp] = init_expr)

/**
 * 定义数组赋值初始化函数宏
 * 传入的变长参数列表奖初始化数组的前面元素，后续元素都是以传入的最后一个元素赋初值。
 */
#define __def_array_assign__(size, type, indexes_tp, param_types, array_init_name, init_expr, params)\
template<size_t size, typename type, size_t ...indexes_tp, typename ...param_types>\
inline void __##array_init_name##_fetch_impl(type (&arr)[size], tplmp::size_t_sequence<indexes_tp ...>, param_types... params)\
{\
	__eval_array_assign_expr__(__expr_eval, arr, indexes_tp, init_expr)\
}\
template<size_t size, typename type, size_t ...indexes_tp>\
inline void __##array_init_name##_fill_impl(type (&arr)[size], tplmp::size_t_sequence<indexes_tp ...>, type default_val)\
{\
	__eval_array_assign_expr__(__expr_eval, arr, indexes_tp, default_val)\
}\
template<size_t size, typename type, typename ...param_types>\
inline void array_init_name(type (&arr)[size], param_types... params)\
{\
	constexpr size_t pass_val_num = sizeof...(param_types);\
	__##array_init_name##_fetch_impl(arr, tplmp::index_sequence_t<size_t, 0, pass_val_num>(), params...);\
	__##array_init_name##_fill_impl(arr, tplmp::index_sequence_t<size_t, pass_val_num, size>(), tplmp::at<pass_val_num - 1>(params...));\
}

__def_array_assign__(_Size, _T, _Indexes, _Ts, fill_array, params, params)

}

#endif//_TPLMP_ARRAY
