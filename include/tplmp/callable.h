#ifndef _TPLMP_CALLABLE
#define _TPLMP_CALLABLE

#include <tplmp/tplmp.h>

namespace tplmp
{
struct __callable_retv_impl_base
{
protected:
	template<typename _FuncType>
	struct __callable_retv_impl_non_void
	{
		_FuncType _callable;

		__attribute__((always_inline)) inline __callable_retv_impl_non_void(_FuncType callable) :
				_callable(callable)
		{
		}

		template<typename ..._ArgTypes>
		__attribute__((always_inline)) inline typename eval_type<_FuncType>::type operator()(_ArgTypes ... args)
		{
			return _callable(args...);
		}
	};

	template<typename _FuncType, typename _RetType, _RetType _VoidRet>
	struct __callable_retv_impl_void
	{
		_FuncType _callable;

		__attribute__((always_inline)) inline __callable_retv_impl_void(_FuncType callable) :
				_callable(callable)
		{
		}

		template<typename ..._ArgTypes>
		__attribute__((always_inline)) inline _RetType operator()(_ArgTypes ... args)
		{
			_callable(args...);
			return _VoidRet;
		}
	};
};

/**
 * @brief 将任意函数包装为带返回值的函数，防止在模板中出现void返回值作为右值。如果函数为void返回类型，则包装为返回_RetType，且其返回值为_VoidRet
 */
template<typename _FuncType, typename _RetType, _RetType _VoidRet>
struct __callable_retv: public __callable_retv_impl_base, public if_else<type_equal<typename eval_type<_FuncType>::type, void>::value>
		::resolve_t<
				__callable_retv_impl_base:: __callable_retv_impl_void <_FuncType, _RetType, _VoidRet>,
				__callable_retv_impl_base:: __callable_retv_impl_non_void <_FuncType>
		>::type
{
};

template<typename _FuncType>
using callable_retv = __callable_retv<_FuncType, int, 0>;

}

#endif //_TPLMP_CALLABLE
