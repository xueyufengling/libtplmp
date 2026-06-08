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
			_callable(forward<_FuncType>(c))
	{
	}

	template<typename ..._ArgTypes>
	__attribute__((always_inline)) inline auto operator()(_ArgTypes&& ... args) const -> decltype(_callable(forward<_ArgTypes>(args)...))
	{
		return _callable(forward<_ArgTypes>(args)...);
	}

	template<typename ..._ArgTypes>
	__attribute__((always_inline)) inline auto call(tuple<_ArgTypes&&...>& args) const -> decltype(_callable(forward<_ArgTypes>(args)...))
	{
		return _callable(forward<_ArgTypes>(args)...);
	}
};

/**
 * @brief 占位符
 */
template<size_t _Index, typename _MappingFunc = void>
struct placeholder: public callable<_MappingFunc>
{
	static constexpr size_t index = _Index;
};

template<size_t _Index>
struct placeholder<_Index, void>
{
	static constexpr size_t index = _Index;

	template<typename _ArgType>
	__attribute__((always_inline)) inline _ArgType&& operator()(_ArgType&& arg) const
	{
		return forward<_ArgType>(arg);
	}

	static const placeholder<_Index> value;

	/**
	 * @brief 创建一个映射占位符
	 * @param m_func 参数映射，形式必须是_ArgType(*)(_ArgType)，传入占位符对应的call参数，返回映射后的最终参数
	 */
	template<typename _MappingFunc>
	__attribute__((always_inline)) inline placeholder<_Index, _MappingFunc> map(_MappingFunc&& m_func)
	{
		return placeholder<_Index, _MappingFunc>(forward<_MappingFunc>(m_func));
	}
};

template<size_t _Index>
const placeholder<_Index, void> placeholder<_Index, void>::value{};

/**
 * 返回值的占位符，由于只有一个返回值故取0
 */
using placeholder_ret = placeholder<0, void>;

template<typename _T>
struct __is_placeholder_impl
{
	static constexpr bool value = false;
};

template<size_t _Index, typename _MappingFunc>
struct __is_placeholder_impl<placeholder<_Index, _MappingFunc> >
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

/**
 * 连续的默认占位符序列生成
 */
template<typename _IntType, _IntType _Start, _IntType _Num>
struct placeholder_sequence
{
protected:
	template<typename _CurrentIter, typename _CollectedPack>
	struct append_iter_index_placeholder_op
	{
		typedef typename _CollectedPack::append<placeholder<_CurrentIter::type::value> >::type type;
	};

public:
	typedef typename forward_iterate<integer_iterator<_IntType, _Start>, integer_iterator<_IntType, _Start + _Num>, append_iter_index_placeholder_op, iterate_cond::always, type_pack<> >::type type;
};

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
			using placeholder_type = typename decay_type<typename bound_arg_types::template at<_Index>::type>::type;
			//是占位符则返回占位符索引对应的call_arg，并在映射后返回
			return bound_args.template at<_Index>()(
					call_args.template at<placeholder_type::index>()
					);
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
 * @brief 绑定参数映射的函数
 */
template<typename _FuncType, typename ..._BoundArgTypes>
struct __bound_args_callable_impl: public __bound_args_callable_impl_base<_BoundArgTypes...>
{
	//绑定的参数值，数量与原函数一致，参数类型要么与_FuncType对应位置类型相同，要么必须是tplmp::placeholder<>
	tuple<_BoundArgTypes...> bound_args;

	__attribute__((always_inline)) inline __bound_args_callable_impl(_BoundArgTypes&& ...args) :
			bound_args(args...)
	{
	}

	//利用编译器对函数的类型自动推导从type_pack<>中提取_Indexes
	template<size_t ..._Indexes, typename ..._CallTypes>
	__attribute__((always_inline)) inline auto __call_impl(_FuncType&& _callable, type_pack<_size_t<_Indexes> ...>, _CallTypes&& ... call_args) const -> decltype(
			_callable(__bound_args_callable_impl_base<_BoundArgTypes...>::template fetch_arg<_Indexes>
					::template value(bound_args, decl<tuple<_CallTypes...> >::ref())...
			)
	)
	{
		tuple<_CallTypes...> call_args_t(forward<_CallTypes>(call_args)...);
		//为每个_Indexes展开一个fetch_arg<>()调用并形成参数包
		return _callable(__bound_args_callable_impl_base<_BoundArgTypes...>::template fetch_arg<_Indexes>::template value(bound_args, call_args_t)...);
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline auto call(_FuncType&& _callable, _CallTypes&& ... call_args) const -> decltype(
			__call_impl(forward<_FuncType>(_callable), typename index_sequence<size_t, 0, sizeof...(_BoundArgTypes)>::type(), forward<_CallTypes>(call_args)...)
	)
	{
		return __call_impl(forward<_FuncType>(_callable), typename index_sequence<size_t, 0, sizeof...(_BoundArgTypes)>::type(), forward<_CallTypes>(call_args)...);
	}
};

/**
 * @brief 绑定返回值映射的函数
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

	template<typename _RetType>
	__attribute__((always_inline)) inline _BoundRetType _return(_RetType&& ret) const
	{
		return bound_ret;
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline _BoundRetType call(_FuncType&& _callable, _CallTypes&& ... call_args) const
	{
		_callable(forward<_CallTypes>(call_args)...);
		return bound_ret;
	}
};

template<typename _FuncType>
struct __bound_ret_callable_impl<_FuncType, void>
{
	template<typename _RetType>
	__attribute__((always_inline)) inline void _return(_RetType&& ret) const
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline void call(_FuncType&& _callable, _CallTypes&& ... call_args) const
	{
		_callable(forward<_CallTypes>(call_args)...);
	}
};

template<typename _FuncType, size_t _RetIndex, typename _RetMappingFunc>
struct __bound_ret_callable_impl<_FuncType, placeholder<_RetIndex, _RetMappingFunc> >
{
	typedef placeholder<_RetIndex, _RetMappingFunc> placeholder_type;

	placeholder_type bound_ret;

	__attribute__((always_inline)) inline __bound_ret_callable_impl(const placeholder_type& ret_m) :
			bound_ret(ret_m)
	{
	}

	template<typename _RetType>
	__attribute__((always_inline)) inline auto _return(_RetType&& ret) const -> decltype(bound_ret(forward<_RetType>(ret)))
	{
		return bound_ret(forward<_RetType>(ret));
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline auto call(_FuncType&& _callable, _CallTypes&& ... call_args) const -> decltype(
			_return(_callable(forward<_CallTypes>(call_args)...))
	)
	{
		return _return(_callable(forward<_CallTypes>(call_args)...));
	}
};

// ----- 绑定固定返回值、参数映射的特化 -----
template<typename _FuncType, typename _BoundRetType, typename ..._BoundArgTypes>
struct bound_callable:
		public callable<_FuncType>,
		public __bound_ret_callable_impl<_FuncType, _BoundRetType>,
		public if_else<sizeof...(_BoundArgTypes)>
		::resolve_t<
				__bound_args_callable_impl <_FuncType, _BoundArgTypes...>,
				empty<>
		>::type
{
	typedef __bound_ret_callable_impl <_FuncType, _BoundRetType> ret_callable_base;
	typedef __bound_args_callable_impl <_FuncType, _BoundArgTypes...> args_callable_base_impl;
	typedef typename if_else<sizeof...(_BoundArgTypes)>
	::resolve_t<
			args_callable_base_impl,
			empty<>
	>::type args_callable_base;

	static const size_t bound_args_num = sizeof...(_BoundArgTypes);
	static constexpr bool has_bound_args = bound_args_num;

	using callable<_FuncType>::_callable;

	bound_callable(_FuncType&& c, _BoundRetType&& ret, _BoundArgTypes&& ...args) :
			callable<_FuncType>(forward<_FuncType>(c)),
					ret_callable_base(forward<_BoundRetType>(ret)),
					args_callable_base(forward<_BoundArgTypes>(args)...)
	{
	}

	//bound_ret是可变的，因此函数不加const修饰符
	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline typename if_else<sizeof...(_CallTypes) <= bound_args_num && has_bound_args>::def<_BoundRetType> operator()(_CallTypes&& ... call_args)
	{
		//参数包call_args是左值，直接展开所有值都变成左值，需要套一层forward函数将右值属性保留
		args_callable_base_impl::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...);
		return ret_callable_base::bound_ret;
	}

	//sizeof...(_CallTypes) >= bound_args_num实际上恒成立，但为了让if_else<>模板参数依赖于_CallTypes以强制编译器进行类型推导，故添加前置条件
	//如果直接写typename if_else<!has_bound_args>::def<_BoundRetType>，由于has_bound_args是已知编译期常量无需推导，则def<>类型就是完全确定的无需推导，此时会引发编译硬错误而非SFINAE
	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline typename if_else<sizeof...(_CallTypes) >= bound_args_num && !has_bound_args>::def<_BoundRetType> operator()(_CallTypes&& ... call_args)
	{
		_callable(forward<_CallTypes>(call_args)...);
		return ret_callable_base::bound_ret;
	}
};

// ----- 去除函数返回值、绑定参数映射的特化 -----
template<typename _FuncType, typename ..._BoundArgTypes>
struct bound_callable<_FuncType, void, _BoundArgTypes...> :
		public callable<_FuncType>,
		public if_else<sizeof...(_BoundArgTypes)>
		::resolve_t<
				__bound_args_callable_impl <_FuncType, _BoundArgTypes...>,
				empty<>
		>::type
{
	typedef typename if_else<sizeof...(_BoundArgTypes)>
	::resolve_t<
			__bound_args_callable_impl <_FuncType, _BoundArgTypes...>,
			empty<>
	>::type args_callable_base;

	static const size_t bound_args_num = sizeof...(_BoundArgTypes);
	static constexpr bool has_bound_args = bound_args_num;

	using callable<_FuncType>::_callable;

	bound_callable(_FuncType&& c, _BoundArgTypes&& ...args) :
			callable<_FuncType>(forward<_FuncType>(c)),
					args_callable_base(forward<_BoundArgTypes>(args)...)
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline typename if_else<sizeof...(_CallTypes) <= bound_args_num && has_bound_args>::def<void> operator()(_CallTypes&& ... call_args) const
	{
		args_callable_base::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...);
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline typename if_else<sizeof...(_CallTypes) >= bound_args_num && !has_bound_args>::def<void> operator()(_CallTypes&& ... call_args) const
	{
		_callable(forward<_CallTypes>(call_args)...);
	}
};

// ----- 绑定返回值映射、参数映射的特化 -----
template<typename _FuncType, size_t _Index, typename _MappingFunc, typename ..._BoundArgTypes>
struct bound_callable<_FuncType, placeholder<_Index, _MappingFunc>, _BoundArgTypes...> :
		public callable<_FuncType>,
		public __bound_ret_callable_impl<_FuncType, placeholder<_Index, _MappingFunc> >,
		public if_else<sizeof...(_BoundArgTypes)>
		::resolve_t<
				__bound_args_callable_impl <_FuncType, _BoundArgTypes...>,
				empty<>
		>::type
{
	typedef placeholder<_Index, _MappingFunc> placeholder_type;
	typedef __bound_ret_callable_impl <_FuncType, placeholder_type> ret_callable_base;
	typedef __bound_args_callable_impl <_FuncType, _BoundArgTypes...> args_callable_base_impl;
	typedef typename if_else<sizeof...(_BoundArgTypes)>
	::resolve_t<
			args_callable_base_impl,
			empty<>
	>::type args_callable_base;

	static const size_t bound_args_num = sizeof...(_BoundArgTypes);
	static constexpr bool has_bound_args = bound_args_num;

	using callable<_FuncType>::_callable;

	//与bound_callable<_FuncType, _BoundRetType, ._BoundArgTypes...>构造函数保持同一形式，如此可以统一调用
	bound_callable(_FuncType&& c, const placeholder_type& ret, _BoundArgTypes&& ...args) :
			callable<_FuncType>(forward<_FuncType>(c)),
					ret_callable_base(ret),
					args_callable_base(forward<_BoundArgTypes>(args)...)
	{
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline auto operator()(_CallTypes&& ... call_args) ->
	typename if_else<sizeof...(_CallTypes) <= bound_args_num && has_bound_args>::def<decltype(
			ret_callable_base::_return(args_callable_base_impl::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...))
	)>
	{
		return ret_callable_base::_return(args_callable_base_impl::call(forward<_FuncType>(_callable), forward<_CallTypes>(call_args)...));
	}

	template<typename ..._CallTypes>
	__attribute__((always_inline)) inline auto operator()(_CallTypes&& ... call_args) ->
	typename if_else<sizeof...(_CallTypes) >= bound_args_num && !has_bound_args>::def<decltype(
			ret_callable_base::_return(_callable(forward<_CallTypes>(call_args)...))
	)>
	{
		return ret_callable_base::_return(_callable(forward<_CallTypes>(call_args)...));
	}
};

template<typename _FuncType, typename ..._BoundArgTypes>
struct bound_callable<_FuncType, const placeholder_ret&, _BoundArgTypes...> : public bound_callable<_FuncType, placeholder_ret, _BoundArgTypes...>
{
	using bound_callable<_FuncType, placeholder_ret, _BoundArgTypes...>::bound_callable;
};

template<typename _FuncType, typename _BoundRetType, typename ..._BoundArgTypes>
inline bound_callable<_FuncType, _BoundRetType, _BoundArgTypes...> bind(_FuncType&& c, _BoundRetType&& bound_ret, _BoundArgTypes&& ...bound_args)
{
	return bound_callable<_FuncType, _BoundRetType, _BoundArgTypes...>(forward<_FuncType>(c), forward<_BoundRetType>(bound_ret), forward<_BoundArgTypes>(bound_args)...);
}

template<typename _FuncType, typename ..._BoundArgTypes>
inline bound_callable<_FuncType, placeholder_ret, _BoundArgTypes...> bind_args(_FuncType&& c, _BoundArgTypes&& ...bound_args)
{
	return bind(forward<_FuncType>(c), placeholder_ret::value, forward<_BoundArgTypes>(bound_args)...);
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

}

#endif //_TPLMP_CALLABLE
