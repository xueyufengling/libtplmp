#ifndef _TPLMP_CALLABLE
#define _TPLMP_CALLABLE

#include <tplmp/tplmp.h>

namespace tplmp
{
/**
 * @brief 函数包装
 */
template<typename _FuncType>
struct callable
{
protected:
	_FuncType _callable;

public:
	__attribute__((always_inline)) inline callable(_FuncType&& c) :
			_callable(c)
	{
	}

	template<typename ..._ArgTypes>
	__attribute__((always_inline)) inline auto operator()(_ArgTypes&& ... args) -> decltype(_callable(args...))
	{
		return _callable(args...);
	}
};

template<typename ..._BoundTypes>
struct __bound_callable_impl_base
{
public:
	typedef type_pack<_BoundTypes...> bound_types;

protected:
	template<size_t _Index>
	struct __fetch_arg_bound_impl
	{
		template<typename ... _ForwardCallTypes>
		__attribute__((always_inline)) inline static typename _const<typename bound_types::at<_Index>::type>::type&
		value(const tuple<_BoundTypes...>& bound_args, const tuple<_ForwardCallTypes...>& call_args)
		{
			return bound_args.template at<_Index>();
		}
	};

	template<size_t _Index>
	struct __fetch_arg_call_impl
	{
		//bound_types::template at<_Index>::type类型为placeholder<>
		template<typename ... _ForwardCallTypes>
		__attribute__((always_inline)) inline static typename type_pack<_ForwardCallTypes...>::at<
				type_of<typename bound_types::template at<_Index>::type>::type::index
		>::type
		value(const tuple<_BoundTypes...>& bound_args, const tuple<_ForwardCallTypes...>& call_args)
		{
			//是占位符则返回占位符索引对应的call_arg
			return call_args.template at<type_of<typename bound_types::template at<_Index>::type>::type::index>();
		}
	};

	template<size_t _Index>
	struct fetch_arg:
			public if_else<is_placeholder_t<typename bound_types::at<_Index>::type>::value>
			::resolve_t<__fetch_arg_call_impl <_Index>, __fetch_arg_bound_impl <_Index> >::type
	{
	};
};

/**
 * @brief 绑定参数的函数
 */
template<typename _FuncType, typename ..._BoundTypes>
struct bound_callable: public __bound_callable_impl_base<_BoundTypes...>, public callable<_FuncType>
{
protected:
	using callable<_FuncType>::_callable;
	//绑定的参数值，数量与原函数一致，参数类型要么与_FuncType对应位置类型相同，要么必须是tplmp::placeholder<>
	tuple<_BoundTypes...> bound_args;

	//利用编译器对函数的类型自动推导从type_pack<>中提取_Indexes
	template<size_t ..._Indexes, typename ..._ForwardCallTypes>
	__attribute__((always_inline)) inline auto __call_impl(type_pack<_size_t<_Indexes> ...>, _ForwardCallTypes&& ... call_args) -> decltype(
			_callable(__bound_callable_impl_base<_BoundTypes...>::template fetch_arg<_Indexes>
					::template value(bound_args, (const tuple<_ForwardCallTypes...>&)tuple<_ForwardCallTypes...> (forward<_ForwardCallTypes>(call_args)...))...
			)
	)
	{
		tuple<_ForwardCallTypes...> call_args_t(forward<_ForwardCallTypes>(call_args)...);
		//为每个_Indexes展开一个fetch_arg<>()调用并形成参数包
		return _callable(__bound_callable_impl_base<_BoundTypes...>::template fetch_arg<_Indexes>::template value(bound_args, call_args_t)...);
	}

public:
	bound_callable(_FuncType&& c, _BoundTypes&& ...args) :
			callable<_FuncType>(c), bound_args(args...)
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline auto operator()(_CallTypes&& ... call_args) -> decltype(
			__call_impl(typename index_sequence<size_t, 0, sizeof...(_BoundTypes)>::type(), forward<_CallTypes>(call_args)...)
	)
	{
		return __call_impl(typename index_sequence<size_t, 0, sizeof...(_BoundTypes)>::type(), forward<_CallTypes>(call_args)...);
	}
};

template<typename _FuncType, typename ..._BoundTypes>
bound_callable<_FuncType, _BoundTypes...> bind_args(_FuncType&& c, _BoundTypes&& ...bound_args)
{
	//参数包bound_args是左值，直接展开所有值都变成左值，需要套一层forward函数将右值属性保留
	return bound_callable<_FuncType, _BoundTypes...>(c, forward<_BoundTypes>(bound_args)...);
}

}

#endif //_TPLMP_CALLABLE
