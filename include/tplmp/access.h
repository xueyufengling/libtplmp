#ifndef _TPLMP_ACCESS
#define _TPLMP_ACCESS

#include <tplmp/base.h>

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

		template<typename _T>
		inline void store(_T* pvar)
		{
			variable = pvar;
		}

		template<typename _RetType, typename ... _ArgTypes>
		inline void store(_RetType (*pfunc)(_ArgTypes...))
		{
			function = (void (*)())pfunc;
		}

		template<typename _T>
		struct __cast_impl
		{
			typedef typename tplmp::ptr_type<void, _T>::type type;

			inline static constexpr type cast(__univptr_orid pord)
			{
				return (type)pord.variable;
			}
		};

		//成员函数不能偏特化，因此使用模板类偏特化实现cast
		template<typename _RetType, typename ... _ArgTypes>
		struct __cast_impl<_RetType(_ArgTypes...)>
		{
			typedef typename tplmp::ptr_type<void, _RetType(_ArgTypes...)>::type type;

			inline static constexpr type cast(__univptr_orid pord)
			{
				return (type)pord.function;
			}
		};

	public:
		__univptr_orid() = default;

		template<typename _T>
		inline __univptr_orid(_T* pord)
		{
			store(pord);
		}

		template<typename _T>
		inline constexpr typename tplmp::ptr_type<void, _T>::type cast()
		{
			return __cast_impl<_T>::cast(*this);
		}

		template<typename _T>
		inline constexpr operator typename tplmp::ptr_type<void, _T>::type()
		{
			return cast<_T>();
		}

		template<typename _T>
		inline __univptr_orid& operator=(_T* pord)
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

		template<typename _T>
		inline void store(_T _Class::*pfield)
		{
			field = (int _Class::*)pfield;
		}

		template<typename _RetType, typename ... _ArgTypes>
		inline void store(_RetType (_Class::*pfunc)(_ArgTypes...))
		{
			function = (void (_Class::*)())pfunc;
		}

		template<typename _T>
		struct __cast_impl
		{
			typedef typename tplmp::ptr_type<_Class, _T>::type type;

			inline static constexpr type cast(__univptr_memb <_Class> pmemb)
			{
				return (type)pmemb.field;
			}
		};

		//成员函数不能偏特化，因此使用模板类偏特化实现cast
		template<typename _RetType, typename ... _ArgTypes>
		struct __cast_impl<_RetType(_ArgTypes...)>
		{
			typedef typename tplmp::ptr_type<_Class, _RetType(_ArgTypes...)>::type type;

			inline static constexpr type cast(__univptr_memb <_Class> pmemb)
			{
				return (type)pmemb.function;
			}
		};

	public:
		__univptr_memb() = default;

		template<typename _T>
		inline __univptr_memb(_T _Class::*pmemb)
		{
			store(pmemb);
		}

		template<typename _T>
		inline constexpr typename tplmp::ptr_type<_Class, _T>::type cast()
		{
			return __cast_impl<_T>::cast(*this);
		}

		template<typename _T>
		inline constexpr operator typename tplmp::ptr_type<_Class, _T>::type()
		{
			return cast();
		}

		template<typename _T>
		inline __univptr_memb <_Class>& operator=(_T _Class::*pmemb)
		{
			store(pmemb);
			return *this;
		}
	};
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
	inline univptr_t(_T* pord)
	{
		orid_ptr = pord;
	}

	template<typename _T>
	inline univptr_t(_T _Class::*pmemb)
	{
		memb_ptr = pmemb;
	}

	template<typename _T>
	inline constexpr typename tplmp::ptr_type<_Class, _T>::type cast()
	{
		return if_else<type_equal<_Class, void>::value>
		::_return(orid_ptr.template cast<_T>(),
				memb_ptr.template cast<_T>());
	}

	template<typename _T>
	inline constexpr operator typename tplmp::ptr_type<_Class, _T>::type()
	{
		return cast<_T>();
	}

	template<typename _T>
	inline univptr_t<_Class>& operator=(_T* pord)
	{
		orid_ptr.store(pord);
		return *this;
	}

	template<typename _T>
	inline univptr_t<_Class>& operator=(_T _Class::*pmemb)
	{
		memb_ptr.store(pmemb);
		return *this;
	}
};

template<>
struct univptr_t<void> : __univptr_impl_base, __univptr_impl_base::__univptr_orid
{
};

/**
 * @brief 成员指针的标识符，仅用作占位符用以区分不同成员，任何一个成员都需要具有其独特的标识符，即定义一个类继承自该类
 */
template<typename _Class>
struct __pmemb_identifier
{
	__assert_is_class__(_Class); //静态断言，只有类成员可以有__access_identifier
	typedef _Class decl_class; //成员所属类

	__pmemb_identifier() = delete; //不能直接使用__access_identifier作为标识符，必须写一个派生类继承自本类

};

/**
 * @brief 实际储存了成员指针的值，但使用前需要强制转换成目标类型。
 */
template<typename _pMembIdentifier>
struct __pmemb_value
{
	typedef typename _pMembIdentifier::decl_class decl_class;

	static tplmp::classify_type classification; //该标识符对应的类型

	static univptr_t<decl_class> _pmemb;

	__pmemb_value() = delete;

private: //template class显式实例化可以无视访问修饰符
	//初始化__pmemb_value::_pmemb
	template<typename _pMemb, _pMemb _pMembValue>
	struct __initializer
	{
	private:
		//本类只会在template class显式实例化时能访问或引用，因此必须在模板实例化时就将成员指针值传出。但在C++11没有任何办法可以在编译时传出。
		__initializer()
		{
			__pmemb_value<_pMembIdentifier>::classification = tplmp::classify_type_of_t<_pMemb>::value;
			__pmemb_value<_pMembIdentifier>::_pmemb = _pMembValue;
		}

		static const __initializer <_pMemb, _pMembValue> _init;
	};
};
template<typename _pMembIdentifier>
tplmp::classify_type __pmemb_value<_pMembIdentifier>::classification = tplmp::classify_type::MEMB_FIELD;

template<typename _pMembIdentifier>
univptr_t<typename _pMembIdentifier::decl_class> __pmemb_value<_pMembIdentifier>::_pmemb = univptr_t<typename _pMembIdentifier::decl_class>();

template<typename _pMembIdentifier>
template<typename _pMemb, _pMemb _pMembValue>
const typename __pmemb_value<_pMembIdentifier>::__initializer<_pMemb, _pMembValue> __pmemb_value<_pMembIdentifier>::__initializer<_pMemb, _pMembValue>::_init{};

/**
 * @brief 用于定义一个访问的ID标识
 */
#define __decl_pmemb__(pmemb_id, target_class)\
	struct pmemb_id: tplmp::__pmemb_identifier<target_class> {}

/**
 * @brief 初始化，即执行取成员指针的行为。
 * 		  此宏将为decl_pmemb()声明的成员指针标识符赋值。
 * 		  此宏只能定义一次，因为模板的显式实例化只能声明一次，多次声明将抛出编译错误。
 */
#define __fetch_pmemb__(pmemb_id, memb_name) template class tplmp::__pmemb_value<pmemb_id>::__initializer<decltype(&pmemb_id::decl_class::memb_name), &pmemb_id::decl_class::memb_name>

#define __read_pmemb__(pmemb_id) (tplmp::__pmemb_value<pmemb_id>::_pmemb)

/**
 * @brief 访问标识符，将成员指针标识符绑定一个类型。
 */
template<typename _pMembIdentifier, typename _MembType>
struct __access_identifier
{
	typedef _pMembIdentifier pmemb_identifier;
	typedef typename _pMembIdentifier::decl_class decl_class;

	typedef _MembType decl_type; //成员声明的类型
	typedef typename tplmp::ptr_type<decl_class, _MembType>::type pmemb_type; //成员指针类型
	typedef typename tplmp::eval_type<_MembType>::type eval_type; //成员求值类型，对字段而言是声明类型，对函数而言是返回类型

	static const tplmp::classify_type classification = tplmp::classify_type_of_t<pmemb_type>::value; //成员分类，判断是成员字段还是成员函数

	__access_identifier() = delete;
};

struct __accessor_impl_base
{
private:
	template<typename _AccessIdentifier>
	struct __accessor_base
	{
		typedef typename _AccessIdentifier::pmemb_identifier pmemb_identifier;
		typedef typename _AccessIdentifier::decl_class decl_class;

		typedef typename _AccessIdentifier::pmemb_type pmemb_type;
		typedef typename _AccessIdentifier::eval_type eval_type;

		decl_class* pobj;

		__accessor_base(decl_class* pobj)
		{
			this->pobj = pobj;
		}

		__accessor_base(decl_class& obj)
		{
			pobj = &obj;
		}

		template<typename _Derived>
		_Derived operator=(const __accessor_base&) = delete;
	};

protected:
	template<typename _AccessIdentifier, tplmp::classify_type _Classification>
	struct __accessor_impl
	{
	};

	template<typename _AccessIdentifier>
	struct __accessor_impl<_AccessIdentifier, tplmp::classify_type::MEMB_FIELD> : public __accessor_base<_AccessIdentifier>
	{
		using typename __accessor_base<_AccessIdentifier>::pmemb_identifier;
		using typename __accessor_base<_AccessIdentifier>::decl_class;

		using typename __accessor_base<_AccessIdentifier>::pmemb_type;
		using typename __accessor_base<_AccessIdentifier>::eval_type;

		using __accessor_base<_AccessIdentifier>::pobj;

		static pmemb_type pmemb;

		__attribute__((always_inline)) inline __accessor_impl(decl_class* pobj) :
				__accessor_base<_AccessIdentifier>(pobj)
		{
		}

		__attribute__((always_inline)) inline __accessor_impl(decl_class& obj) :
				__accessor_base<_AccessIdentifier>(obj)
		{
		}

		__attribute__((always_inline)) inline operator eval_type&()
		{
			return pobj->*pmemb;
		}

		__attribute__((always_inline)) inline __accessor_impl <_AccessIdentifier, tplmp::classify_type::MEMB_FIELD>& operator=(const eval_type& value)
		{
			pobj->*pmemb = value;
			return *this;
		}

		__attribute__((always_inline)) inline static eval_type& load(decl_class* pobj)
		{
			return pobj->*pmemb;
		}

		__attribute__((always_inline)) inline static eval_type& load(decl_class& obj)
		{
			return obj.*pmemb;
		}

		__attribute__((always_inline)) inline static void store(decl_class* pobj, eval_type&& value)
		{
			pobj->*pmemb = value;
		}

		__attribute__((always_inline)) inline static void store(decl_class& obj, eval_type&& value)
		{
			obj.*pmemb = value;
		}
	};

	//成员函数访问的实现
	template<typename _AccessIdentifier>
	struct __accessor_impl<_AccessIdentifier, tplmp::classify_type::MEMB_FUNCTION> : public __accessor_base<_AccessIdentifier>
	{
		using typename __accessor_base<_AccessIdentifier>::pmemb_identifier;
		using typename __accessor_base<_AccessIdentifier>::decl_class;

		using typename __accessor_base<_AccessIdentifier>::pmemb_type;
		using typename __accessor_base<_AccessIdentifier>::eval_type;

		using __accessor_base<_AccessIdentifier>::pobj;

		static pmemb_type pmemb;

		__attribute__((always_inline)) inline __accessor_impl(decl_class* pobj) :
				__accessor_base<_AccessIdentifier>(pobj)
		{
		}

		__attribute__((always_inline)) inline __accessor_impl(decl_class& obj) :
				__accessor_base<_AccessIdentifier>(obj)
		{
		}

		template<typename ..._ArgTypes>
		__attribute__((always_inline)) inline auto operator()(_ArgTypes ... args) -> decltype((pobj->*pmemb)(args...))
		{
			return (pobj->*pmemb)(args...);
		}

		template<typename ..._ArgTypes>
		__attribute__((always_inline)) inline static auto call(decl_class* pobj, _ArgTypes ... args) -> decltype((pobj->*pmemb)(args...))
		{
			return (pobj->*pmemb)(args...);
		}

		template<typename ..._ArgTypes>
		__attribute__((always_inline)) inline static auto call(decl_class& obj, _ArgTypes ... args) -> decltype((obj.*pmemb)(args...))
		{
			return (obj.*pmemb)(args...);
		}
	};

	template<typename _AccessIdentifier>
	using field_accessor = __accessor_impl<_AccessIdentifier, tplmp::classify_type::MEMB_FIELD>;

	template<typename _AccessIdentifier>
	using function_accessor = __accessor_impl<_AccessIdentifier, tplmp::classify_type::MEMB_FUNCTION>;
};

template<typename _AccessIdentifier>
typename __accessor_impl_base::field_accessor<_AccessIdentifier>::pmemb_type
__accessor_impl_base::field_accessor<_AccessIdentifier>::pmemb =
		(typename __accessor_impl_base::field_accessor<_AccessIdentifier>::pmemb_type)__pmemb_value<
				typename __accessor_impl_base::field_accessor<_AccessIdentifier>::pmemb_identifier
		>::_pmemb;

template<typename _AccessIdentifier>
typename __accessor_impl_base::function_accessor<_AccessIdentifier>::pmemb_type
__accessor_impl_base::function_accessor<_AccessIdentifier>::pmemb =
		(typename __accessor_impl_base::function_accessor<_AccessIdentifier>::pmemb_type)__pmemb_value<
				typename __accessor_impl_base::function_accessor<_AccessIdentifier>::pmemb_identifier
		>::_pmemb;

/**
 * @brief 访问私有成员，若目标成员不存在则会直接抛出编译错误
 * 		  原理：在显式实例化模板给其指针类型模板参数直接赋值时，可以绕过访问修饰符直接取到成员指针，需要将这个取到的成员指针在本模板以外之处储存起来在使用。
 * 		  注意在显式实例化完成以后，因其模板参数包含了对private成员取指针，故被显示实例化的模板将无法声明或使用，否则会受到访问修饰符限制并报错。
 */
template<typename _AccessIdentifier>
struct __accessor: public __accessor_impl_base, public __accessor_impl_base::__accessor_impl<_AccessIdentifier, _AccessIdentifier::classification>
{
	typedef _AccessIdentifier identifier;

	static constexpr tplmp::classify_type classification = identifier::classification;

	using typename __accessor_impl<identifier, classification>::decl_class;

	__accessor(decl_class* pobj) :
			__accessor_impl<identifier, classification>(pobj)
	{
	}

	__accessor(decl_class& obj) :
			__accessor_impl<identifier, classification>(obj)
	{
	}
};

/**
 * @brief 对于指定类名和成员名的accessor类名
 */
#define __accessor__(acc_id) tplmp::__accessor<acc_id>

/**
 * @brief 用于定义一个访问的ID标识
 */
#define __decl_accessor__(acc_id, pmemb_id, decl_type)\
	struct acc_id: tplmp::__access_identifier<pmemb_id, decl_type> {}
}

#endif //_TPLMP_ACCESS
