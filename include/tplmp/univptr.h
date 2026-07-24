#ifndef _TPLMP_UNIVPTR
#define _TPLMP_UNIVPTR

#include "base.h"

namespace tplmp
{
struct __univptr_impl_base
{
protected:
	/**
	 * @brief 储存通常的变量或函数指针。
	 * 		  变量指针和函数指针不能互相cast，因此使用union。
	 */
	struct __univptr_orid
	{
	private:
		union
		{
			void* variable;
			void (*function)();
		};

	public:
		template<typename _T>
		inline void store(_T* pvar) noexcept
		{
			variable = pvar;
		}

		template<typename _RetType, typename ... _ArgTypes>
		inline void store(_RetType (*pfunc)(_ArgTypes...)) noexcept
		{
			function = (void (*)())pfunc;
		}

		template<typename _T>
		struct __cast_impl
		{
			typedef typename tplmp::ptr_type<void, _T>::type type;

			inline static constexpr type cast(__univptr_orid pord) noexcept
			{
				return (type)pord.variable;
			}
		};

		//成员函数不能偏特化，因此使用模板类偏特化实现cast
		template<typename _RetType, typename ... _ArgTypes>
		struct __cast_impl<_RetType(_ArgTypes...)>
		{
			typedef typename tplmp::ptr_type<void, _RetType(_ArgTypes...)>::type type;

			inline static constexpr type cast(__univptr_orid pord) noexcept
			{
				return (type)pord.function;
			}
		};

		__univptr_orid() = default;

		template<typename _T>
		inline __univptr_orid(_T* pord) noexcept
		{
			store(pord);
		}

		template<typename _T>
		inline constexpr typename tplmp::ptr_type<void, _T>::type cast() noexcept
		{
			return __cast_impl<_T>::cast(*this);
		}

		template<typename _T>
		inline constexpr operator typename tplmp::ptr_type<void, _T>::type() noexcept
		{
			return cast<_T>();
		}

		template<typename _T>
		inline __univptr_orid& operator=(_T* pord) noexcept
		{
			store(pord);
			return *this;
		}
	};

	/**
	 * @brief 储存类_Class的成员指针。
	 * 		  字段成员指针和函数成员指针不能互相cast，因此使用union。
	 * 		  成员函数指针在C++标准中未强制要求实现方式，因此在不同编译器上成员指针的大小可能不同。
	 * 		  即便是同一编译器，当类的继承关系不同时，成员函数指针的大小也可能不同，因此必须使用模板类以适配各种情况。
	 */
	template<typename _Class>
	struct __univptr_memb
	{
		__assert_is_class__(_Class);
	private:
		union
		{
			int _Class::*field; //任意类型的字段
			void (_Class::*function)();//任意类型的成员函数
		};

	public:
		template<typename _T>
		inline void store(_T _Class::*pfield) noexcept
		{
			field = (int _Class::*)pfield;
		}

		template<typename _RetType, typename ... _ArgTypes>
		inline void store(_RetType (_Class::*pfunc)(_ArgTypes...)) noexcept
		{
			function = (void (_Class::*)())pfunc;
		}

		template<typename _T>
		struct __cast_impl
		{
			typedef typename tplmp::ptr_type<_Class, _T>::type type;

			inline static constexpr type cast(__univptr_memb <_Class> pmemb) noexcept
			{
				return (type)pmemb.field;
			}
		};

		//成员函数不能偏特化，因此使用模板类偏特化实现cast
		template<typename _RetType, typename ... _ArgTypes>
		struct __cast_impl<_RetType(_ArgTypes...)>
		{
			typedef typename tplmp::ptr_type<_Class, _RetType(_ArgTypes...)>::type type;

			inline static constexpr type cast(__univptr_memb <_Class> pmemb) noexcept
			{
				return (type)pmemb.function;
			}
		};

		__univptr_memb() = default;

		template<typename _T>
		inline __univptr_memb(_T _Class::*pmemb) noexcept
		{
			store(pmemb);
		}

		template<typename _T>
		inline constexpr typename tplmp::ptr_type<_Class, _T>::type cast() const noexcept
		{
			return __cast_impl<_T>::cast(*this);
		}

		template<typename _T>
		inline constexpr operator typename tplmp::ptr_type<_Class, _T>::type() const noexcept
		{
			return cast();
		}

		template<typename _T>
		inline __univptr_memb <_Class>& operator=(_T _Class::*pmemb) noexcept
		{
			store(pmemb);
			return *this;
		}
	};
};

template<>
struct __univptr_impl_base::__univptr_memb<void>: public __univptr_impl_base::__univptr_orid
{
};

/**
 * @brief 可以存取普通变量、普通函数、成员字段、成员函数的指针类型
 */
template<typename _Class>
struct univptr_t: __univptr_impl_base
{
	union
	{
		__univptr_orid orid_ptr;
		__univptr_memb <_Class> memb_ptr;
	};

	univptr_t() = default;

	template<typename _T>
	inline univptr_t(_T* pord) noexcept
	{
		orid_ptr = pord;
	}

	template<typename _T>
	inline univptr_t(_T _Class::*pmemb) noexcept
	{
		memb_ptr = pmemb;
	}

	template<typename _T>
	inline constexpr
	typename tplmp::ptr_type<_Class, _T>::type cast() const noexcept
	{
		return if_else<type_equal<_Class, void>::value>
		::_return(orid_ptr.template cast<_T>(),
				memb_ptr.template cast<_T>());
	}

	//当_Class为非void时才允许转换为类成员指针
	template<typename _T, typename = typename if_else<!type_equal<_Class, void>::value && !type_equal<_T, void>::value>::def<> >
	inline constexpr operator _T _Class::*() const noexcept
	{
		return memb_ptr.template cast<_T>();
	}

	template<typename _T>
	inline constexpr operator _T*() const noexcept
	{
		return orid_ptr.template cast<_T>();
	}

	template<typename _T>
	inline univptr_t<_Class>& operator=(_T* pord) noexcept
	{
		orid_ptr.store(pord);
		return *this;
	}

	template<typename _T>
	inline univptr_t<_Class>& operator=(_T _Class::*pmemb) noexcept
	{
		memb_ptr.store(pmemb);
		return *this;
	}
};

}

#endif//_TPLMP_UNIVPTR
