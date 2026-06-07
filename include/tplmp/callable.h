#ifndef _TPLMP_CALLABLE
#define _TPLMP_CALLABLE

#include <tplmp/base.h>
#include <tplmp/tuple.h>

namespace tplmp
{
/**
 * @brief 函数包装
 */
template<typename _FuncType>
struct callable
{
	_FuncType _callable;

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

/**
 * @brief 占位符
 */
template<size_t _Index>
struct placeholder
{
	static constexpr size_t index = _Index;

	/**
	 * @brief 占位符的值
	 */
	static const placeholder<_Index> value;
};

template<size_t _Index>
const placeholder<_Index> placeholder<_Index>::value{};

/**
 * 返回值的占位符，由于只有一个返回值故取0
 */
using placeholder_ret = placeholder<0>;

template<typename _T>
struct __is_placeholder_impl
{
	static constexpr bool value = false;
};

template<size_t _Index>
struct __is_placeholder_impl<placeholder<_Index> >
{
	static constexpr bool value = true;
};

template<typename _T>
struct is_placeholder_t
{
	static constexpr bool value = __is_placeholder_impl<typename decay_type<_T>::type>::value;
};

template<typename _T>
constexpr bool is_placeholder(_T)
{
	return __is_placeholder_impl<typename decay_type<_T>::type>::value;
}

//避免模板多次重复实例化
template<typename ..._BoundArgTypes>
struct __bound_args_callable_impl_base
{
public:
	typedef type_pack<_BoundArgTypes...> bound_arg_types;

protected:
	template<size_t _Index>
	struct __fetch_arg_bound_impl
	{
		template<typename ... _ForwardCallTypes>
		__attribute__((always_inline)) inline static typename _const<typename bound_arg_types::at<_Index>::type>::type&
		value(const tuple<_BoundArgTypes...>& bound_args, const tuple<_ForwardCallTypes...>& call_args)
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
				decay_type<typename bound_arg_types::template at<_Index>::type>::type::index
		>::type
		value(const tuple<_BoundArgTypes...>& bound_args, const tuple<_ForwardCallTypes...>& call_args)
		{
			//是占位符则返回占位符索引对应的call_arg
			return call_args.template at<decay_type<typename bound_arg_types::template at<_Index>::type>::type::index>();
		}
	};

	template<size_t _Index>
	struct fetch_arg:
			public if_else<is_placeholder_t<typename bound_arg_types::at<_Index>::type>::value>
			::resolve_t<__fetch_arg_call_impl <_Index>, __fetch_arg_bound_impl <_Index> >::type
	{
	};
};

/**
 * @brief 绑定参数的函数
 */
template<typename _FuncType, typename ..._BoundArgTypes>
struct __bound_args_callable_impl: public __bound_args_callable_impl_base<_BoundArgTypes...>
{
	//绑定的参数值，数量与原函数一致，参数类型要么与_FuncType对应位置类型相同，要么必须是tplmp::placeholder<>
	tuple<_BoundArgTypes...> bound_args;

	//利用编译器对函数的类型自动推导从type_pack<>中提取_Indexes
	template<size_t ..._Indexes, typename ..._ForwardCallTypes>
	__attribute__((always_inline)) inline auto __call_impl(_FuncType&& _callable, type_pack<_size_t<_Indexes> ...>, _ForwardCallTypes&& ... call_args) -> decltype(
			_callable(__bound_args_callable_impl_base<_BoundArgTypes...>::template fetch_arg<_Indexes>
					::template value(bound_args, (const tuple<_ForwardCallTypes...>&)tuple<_ForwardCallTypes...> (forward<_ForwardCallTypes>(call_args)...))...
			)
	)
	{
		tuple<_ForwardCallTypes...> call_args_t(forward<_ForwardCallTypes>(call_args)...);
		//为每个_Indexes展开一个fetch_arg<>()调用并形成参数包
		return _callable(__bound_args_callable_impl_base<_BoundArgTypes...>::template fetch_arg<_Indexes>::template value(bound_args, call_args_t)...);
	}

	__attribute__((always_inline)) inline __bound_args_callable_impl(_BoundArgTypes&& ...args) :
			bound_args(args...)
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline auto call(_FuncType&& _callable, _CallTypes&& ... call_args) -> decltype(
			__call_impl(forward<_FuncType>(_callable), typename index_sequence<size_t, 0, sizeof...(_BoundArgTypes)>::type(), forward<_CallTypes>(call_args)...)
	)
	{
		return __call_impl(forward<_FuncType>(_callable), typename index_sequence<size_t, 0, sizeof...(_BoundArgTypes)>::type(), forward<_CallTypes>(call_args)...);
	}
};

/**
 * @brief 绑定返回值的函数
 */
template<typename _FuncType, typename _BoundRetType>
struct __bound_ret_callable_impl
{
	_BoundRetType bound_ret;

	typedef _BoundRetType ret_type;

	__attribute__((always_inline)) inline __bound_ret_callable_impl(_BoundRetType&& ret) :
			bound_ret(ret)
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline _BoundRetType call(_FuncType&& _callable, _CallTypes&& ... call_args)
	{
		_callable(call_args...);
		return bound_ret;
	}
};

template<typename _FuncType>
struct __bound_ret_callable_impl<_FuncType, void>
{
	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline void call(_FuncType&& _callable, _CallTypes&& ... call_args)
	{
		_callable(call_args...);
	}
};

template<typename _FuncType>
struct __bound_ret_callable_impl<_FuncType, placeholder_ret>
{
	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline auto call(_FuncType&& _callable, _CallTypes&& ... call_args) -> decltype(_callable(call_args...))
	{
		return _callable(call_args...);
	}
};

template<typename _FuncType, typename _BoundRetType, typename ..._BoundArgTypes>
struct bound_callable:
		public callable<_FuncType>,
		public __bound_ret_callable_impl<_FuncType, _BoundRetType>,
		public __bound_args_callable_impl<_FuncType, _BoundArgTypes...>
{
	using callable<_FuncType>::_callable;

	bound_callable(_FuncType&& c, _BoundRetType&& ret, _BoundArgTypes&& ...args) :
			callable<_FuncType>(forward<_FuncType>(c)),
					__bound_ret_callable_impl<_FuncType, _BoundRetType>(forward<_BoundRetType>(ret)),
					__bound_args_callable_impl<_FuncType, _BoundArgTypes...>(forward<_BoundArgTypes>(args)...)
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline _BoundRetType operator()(_CallTypes&& ... call_args)
	{
		//参数包call_args是左值，直接展开所有值都变成左值，需要套一层forward函数将右值属性保留
		__bound_args_callable_impl<_FuncType, _BoundArgTypes...>::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...);
		return __bound_ret_callable_impl<_FuncType, _BoundRetType>::bound_ret;
	}
};

template<typename _FuncType, typename ..._BoundArgTypes>
struct bound_callable<_FuncType, void, _BoundArgTypes...> :
		public callable<_FuncType>,
		public __bound_args_callable_impl<_FuncType, _BoundArgTypes...>
{
	using callable<_FuncType>::_callable;

	bound_callable(_FuncType&& c, _BoundArgTypes&& ...args) :
			callable<_FuncType>(forward<_FuncType>(c)),
					__bound_args_callable_impl<_FuncType, _BoundArgTypes...>(forward<_BoundArgTypes>(args)...)
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline void operator()(_CallTypes&& ... call_args)
	{
		__bound_args_callable_impl<_FuncType, _BoundArgTypes...>::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...);
	}
};

template<typename _FuncType, typename ..._BoundArgTypes>
struct bound_callable<_FuncType, placeholder_ret, _BoundArgTypes...> :
		public callable<_FuncType>,
		public __bound_args_callable_impl<_FuncType, _BoundArgTypes...>
{
	using callable<_FuncType>::_callable;

	bound_callable(_FuncType&& c, _BoundArgTypes&& ...args) :
			callable<_FuncType>(forward<_FuncType>(c)),
					__bound_args_callable_impl<_FuncType, _BoundArgTypes...>(forward<_BoundArgTypes>(args)...)
	{
	}

	//添加一个const placeholder_ret&省略参数，与bound_callable<_FuncType, _BoundRetType, ._BoundArgTypes...>构造函数保持同一形式，如此可以统一调用
	__attribute__((always_inline)) inline bound_callable(_FuncType&& c, const placeholder_ret&, _BoundArgTypes&& ...args) :
			bound_callable(forward<_FuncType>(c), forward<_BoundArgTypes>(args)...)
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline auto operator()(_CallTypes&& ... call_args) -> decltype(
			__bound_args_callable_impl<_FuncType, _BoundArgTypes...>::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...)
	)
	{
		return __bound_args_callable_impl<_FuncType, _BoundArgTypes...>::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...);
	}
};

template<typename _FuncType, typename ..._BoundArgTypes>
struct bound_callable<_FuncType, const placeholder_ret&, _BoundArgTypes...> : public bound_callable<_FuncType, placeholder_ret, _BoundArgTypes...>
{
	using bound_callable<_FuncType, placeholder_ret, _BoundArgTypes...>::bound_callable;
};

template<typename _FuncType, typename _BoundRetType>
struct bound_callable<_FuncType, _BoundRetType> :
		public callable<_FuncType>,
		public __bound_ret_callable_impl<_FuncType, _BoundRetType>
{
	using callable<_FuncType>::_callable;

	bound_callable(_FuncType&& c, _BoundRetType&& ret) :
			callable<_FuncType>(forward<_FuncType>(c)),
					__bound_ret_callable_impl<_FuncType, _BoundRetType>(forward<_BoundRetType>(ret))
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline _BoundRetType operator()(_CallTypes&& ... call_args)
	{
		return __bound_ret_callable_impl<_FuncType, _BoundRetType>::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...);
	}
};

template<typename _FuncType, typename ..._BoundArgTypes>
inline bound_callable<_FuncType, placeholder_ret, _BoundArgTypes...> bind_args(_FuncType&& c, _BoundArgTypes&& ...bound_args)
{
	return bound_callable<_FuncType, placeholder_ret, _BoundArgTypes...>(forward<_FuncType>(c), forward<_BoundArgTypes>(bound_args)...);
}

template<typename _FuncType, typename _BoundRetType>
inline bound_callable<_FuncType, _BoundRetType> bind_ret(_FuncType&& c, _BoundRetType&& bound_ret)
{
	return bound_callable<_FuncType, _BoundRetType>(forward<_FuncType>(c), forward<_BoundRetType>(bound_ret));
}

template<typename _FuncType>
inline bound_callable<_FuncType, void> bind_ret(_FuncType&& c)
{
	return bound_callable<_FuncType, void>(forward<_FuncType>(c));
}

template<typename _FuncType, typename _BoundRetType, typename ..._BoundArgTypes>
inline bound_callable<_FuncType, _BoundRetType, _BoundArgTypes...> bind(_FuncType&& c, _BoundRetType&& bound_ret, _BoundArgTypes&& ...bound_args)
{
	return bound_callable<_FuncType, _BoundRetType, _BoundArgTypes...>(forward<_FuncType>(c), forward<_BoundRetType>(bound_ret), forward<_BoundArgTypes>(bound_args)...);
}

}

#endif //_TPLMP_CALLABLE
