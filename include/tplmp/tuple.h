#ifndef _TPLMP_TUPLE
#define _TPLMP_TUPLE

#include <tplmp/base.h>

namespace tplmp
{
/**
 * @brief 元组
 */
struct __tuple_impl_base
{
protected:
	template<size_t _Index>
	struct __at_impl;

	template<size_t _Index>
	struct __at_const_impl;
};

template<typename ..._Types>
struct tuple;

template<>
struct tuple<>
{
	static const size_t size = 0;

	template<typename _FuncType>
	__attribute__((always_inline)) inline auto call(_FuncType&& c) const -> decltype(c())
	{
		return c();
	}
};

template<typename _FirstType, typename ... _RestTypes>
struct tuple<_FirstType, _RestTypes...> : __tuple_impl_base
{
	static const size_t size = sizeof...(_RestTypes) + 1;

	_FirstType front_elem;
	tuple<_RestTypes...> back_elems;

	tuple() = default;
	tuple(const _FirstType& first, const _RestTypes& ... rest)
	:
			front_elem(first), back_elems(rest...)
	{
	}

	//索引越界会在auto -> decltype()推导过程中报错，不会触发函数体内的静态断言！
	//需要人工保证不越界
	template<size_t _Index>
	auto at() -> decltype(__at_impl<_Index>::template value(decl<tuple<_FirstType, _RestTypes...> >::ref()))
	{
		return __at_impl<_Index>::template value(*this);
	}

	template<size_t _Index>
	auto at() const -> decltype(__at_const_impl<_Index>::template value(decl<tuple<_FirstType, _RestTypes...> >::ref()))
	{
		return __at_const_impl<_Index>::template value(*this);
	}

	template<typename _FuncType, size_t ..._Indexes>
	__attribute__((always_inline)) inline auto __call_impl(_FuncType&& c, type_pack<_size_t<_Indexes> ...>) const -> decltype(
			c(decl<tuple<_RestTypes...> >::ref().template at<_Indexes>()...)
	)
	{
		//为每个_Indexes展开一个fetch_arg<>()调用并形成参数包
		return c(this->template at<_Indexes>()...);
	}

	/**
	 * @brief 将元组的成员作为参数传递给目标函数
	 */
	template<typename _FuncType>
	__attribute__((always_inline)) inline auto call(_FuncType&& c) const -> decltype(
			__call_impl(forward<_FuncType>(c), typename index_sequence<size_t, 0, size>::type())
	)
	{
		return __call_impl(forward<_FuncType>(c), typename index_sequence<size_t, 0, size>::type());
	}
};

template<size_t _Index>
struct __tuple_impl_base::__at_impl
{
	template<typename _AtFirst, typename ... _AtRest>
	static auto value(tuple<_AtFirst, _AtRest...>& t) -> decltype(__at_impl<_Index - 1>::template value(t.back_elems))
	{
		return __at_impl<_Index - 1>::template value(t.back_elems);
	}
};

/**
 * at()递归终止条件，索引=0
 */
template<>
struct __tuple_impl_base::__at_impl<0>
{
	template<typename _AtFirst, typename ... _AtRest>
	static _AtFirst& value(tuple<_AtFirst, _AtRest...>& t)
	{
		return t.front_elem;
	}
};

template<size_t _Index>
struct __tuple_impl_base::__at_const_impl
{
	template<typename _AtFirst, typename ... _AtRest>
	static auto value(const tuple<_AtFirst, _AtRest...>& t) -> decltype(__at_const_impl<_Index - 1>::template value(t.back_elems))
	{
		return __at_const_impl<_Index - 1>::template value(t.back_elems);
	}
};

template<>
struct __tuple_impl_base::__at_const_impl<0>
{
	template<typename _AtFirst, typename ... _AtRest>
	static const _AtFirst& value(const tuple<_AtFirst, _AtRest...>& t)
	{
		return t.front_elem;
	}
};

}

#endif//_TPLMP_TUPLE
