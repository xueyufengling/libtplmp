#ifndef _TPLMP_ACCESS
#define _TPLMP_ACCESS

#include "univptr.h"

namespace tplmp
{
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
 * @brief pmemb_id类用于定义一个访问的ID标识。
 * 		  pmemb实际储存了成员指针的值，但使用前需要强制转换成目标类型。
 * 		  原理：
 * 		  template class显式实例化可以无视访问修饰符。
 * 		  __initializer类只会在template class显式实例化时能访问到private成员指针，因此必须在模板实例化时就想办法将成员指针值传出。
 * 		  这里使用模板友元注入，friend函数来实现传出成员指针：模板类内定义的friend函数，可以使用模板参数，同时它又不属于该模板，而是命名空间下的普通函数，可以实现在不写出模板类型的前提下，得到模板参数。
 * 		  利用函数参数自动推导，可以匹配到与_pMembIdentifier对应的__initializer_pmemb_value()函数重载，巧妙地将值在编译期就传递出来。如果不这样做，则只能在__initializer内部运行时通过静态初始化赋值才能传递出去。
 * 		  ADL要求__initializer_pmemb_value()的参数与所属模板类在同一命名空间。
 * 		  此宏只能定义一次，因为模板的显式实例化只能声明一次，多次声明将抛出编译错误。
 *
 * 		  __memb_ptr()函数用于编译期获取成员指针，在使用前需要手动声明一次。
 */
#define __decl_pmemb__(pmemb_id, class_name, memb_name)\
	struct pmemb_id: ::tplmp::__pmemb_identifier<class_name>\
	{\
		typedef class_name decl_class;\
		static const ::tplmp::classify_type classification;\
		static const ::tplmp::univptr_t<decl_class> pmemb;\
		pmemb_id() = delete;\
	private:\
		template<typename _pMemb, _pMemb _pMembValue>\
		struct __initializer\
		{\
			friend inline constexpr ::tplmp::classify_type __initializer_pmemb_classification(::tplmp::type_t<pmemb_id>)\
			{\
				return ::tplmp::classify_type_of_t<_pMemb>::value;\
			}\
			friend inline constexpr ::tplmp::univptr_t<class_name> __initializer_pmemb_value(::tplmp::type_t<pmemb_id>)\
			{\
				return _pMembValue;\
			}\
			friend inline constexpr _pMemb __memb_ptr_intl(::tplmp::type_t<pmemb_id>)\
			{\
				return _pMembValue;\
			}\
		};\
	};\
	template class pmemb_id::__initializer<decltype(&class_name::memb_name), &class_name::memb_name>;\
	inline constexpr ::tplmp::classify_type __initializer_pmemb_classification(::tplmp::type_t<pmemb_id>);\
	inline constexpr ::tplmp::univptr_t<class_name> __initializer_pmemb_value(::tplmp::type_t<pmemb_id>);\
	constexpr ::tplmp::classify_type pmemb_id::classification = __initializer_pmemb_classification(::tplmp::type_t<pmemb_id>());\
	const ::tplmp::univptr_t<class_name> pmemb_id::pmemb = __initializer_pmemb_value(::tplmp::type_t<pmemb_id>());

/**
 * @brief 定义取成员指针值的constexpr函数，decl_type必须与实际声明类型严格保持一致。
 * 		  __decl_memb_ptr__()与__decl_pmemb__()必须在同一命名空间下。
 */
#define __decl_memb_ptr__(pmemb_id, decl_type)\
	inline constexpr typename ::tplmp::ptr_type<typename pmemb_id::decl_class, decl_type>::type __memb_ptr_intl(::tplmp::type_t<pmemb_id>);\
	template<typename _pMembIdentifier>\
	inline constexpr auto __memb_ptr() -> decltype(__memb_ptr_intl(tplmp::type_t<_pMembIdentifier>()));\
	template<>\
	inline constexpr auto __memb_ptr<pmemb_id>() -> decltype(__memb_ptr_intl(tplmp::type_t<pmemb_id>()))\
	{\
		return __memb_ptr_intl(::tplmp::type_t<pmemb_id>());\
	}

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
	template<typename _AccessIdentifier>
	struct field_accessor: public __accessor_base<_AccessIdentifier>
	{
		typedef __accessor_base <_AccessIdentifier> base;

		using typename base::pmemb_identifier;
		using typename base::decl_class;

		using typename base::pmemb_type;
		using typename base::eval_type;

		using base::pobj;

		static pmemb_type pmemb;

		__attribute__((always_inline)) inline field_accessor(decl_class* pobj) :
				base(pobj)
		{
		}

		__attribute__((always_inline)) inline field_accessor(decl_class& obj) :
				base(obj)
		{
		}

		__attribute__((always_inline)) inline operator eval_type&()
		{
			return pobj->*pmemb;
		}

		__attribute__((always_inline)) inline operator const eval_type&() const
		{
			return pobj->*pmemb;
		}

		__attribute__((always_inline)) inline static eval_type& load(decl_class* pobj)
		{
			return pobj->*pmemb;
		}

		__attribute__((always_inline)) inline static const eval_type& load(const decl_class* pobj)
		{
			return pobj->*pmemb;
		}

		__attribute__((always_inline)) inline static eval_type& load(decl_class& obj)
		{
			return obj.*pmemb;
		}

		__attribute__((always_inline)) inline static const eval_type& load(const decl_class& obj)
		{
			return obj.*pmemb;
		}

		__attribute__((always_inline)) inline static void store(decl_class* pobj, const eval_type& value)
		{
			pobj->*pmemb = value;
		}

		__attribute__((always_inline)) inline static void store(decl_class& obj, const eval_type& value)
		{
			obj.*pmemb = value;
		}
	};

	//成员函数访问的实现
	template<typename _AccessIdentifier>
	struct function_accessor: public __accessor_base<_AccessIdentifier>
	{
		typedef __accessor_base <_AccessIdentifier> base;

		using typename base::pmemb_identifier;
		using typename base::decl_class;

		using typename base::pmemb_type;
		using typename base::eval_type;

		using base::pobj;

		static pmemb_type pmemb;

		__attribute__((always_inline)) inline function_accessor(decl_class* pobj) :
				base(pobj)
		{
		}

		__attribute__((always_inline)) inline function_accessor(decl_class& obj) :
				base(obj)
		{
		}

		template<typename ..._ArgTypes>
		__attribute__((always_inline)) inline static auto call(decl_class* pobj, _ArgTypes&& ... args) -> decltype((pobj->*pmemb)(forward<_ArgTypes>(args)...))
		{
			return (pobj->*pmemb)(forward<_ArgTypes>(args)...);
		}

		template<typename ..._ArgTypes>
		__attribute__((always_inline)) inline static auto call(decl_class& obj, _ArgTypes&& ... args) -> decltype((obj.*pmemb)(forward<_ArgTypes>(args)...))
		{
			return (obj.*pmemb)(forward<_ArgTypes>(args)...);
		}
	};
};

template<typename _AccessIdentifier>
typename __accessor_impl_base::field_accessor<_AccessIdentifier>::pmemb_type
__accessor_impl_base::field_accessor<_AccessIdentifier>::pmemb = (typename __accessor_impl_base::field_accessor<_AccessIdentifier>::pmemb_type)_AccessIdentifier::pmemb_identifier::pmemb;

template<typename _AccessIdentifier>
typename __accessor_impl_base::function_accessor<_AccessIdentifier>::pmemb_type
__accessor_impl_base::function_accessor<_AccessIdentifier>::pmemb = (typename __accessor_impl_base::function_accessor<_AccessIdentifier>::pmemb_type)_AccessIdentifier::pmemb_identifier::pmemb;

/**
 * @brief 访问私有成员，若目标成员不存在则会直接抛出编译错误
 * 		  原理：在显式实例化模板给其指针类型模板参数直接赋值时，可以绕过访问修饰符直接取到成员指针，需要将这个取到的成员指针在本模板以外之处储存起来在使用。
 * 		  注意在显式实例化完成以后，因其模板参数包含了对private成员取指针，故被显示实例化的模板将无法声明或使用，否则会受到访问修饰符限制并报错。
 */
template<typename _AccessIdentifier, tplmp::classify_type _Classification = _AccessIdentifier::classification>
struct __accessor_impl;

template<typename _AccessIdentifier>
struct __accessor_impl<_AccessIdentifier, tplmp::classify_type::classify_type_memb_field> : public __accessor_impl_base, public __accessor_impl_base::field_accessor<_AccessIdentifier>
{
	typedef _AccessIdentifier identifier;
	typedef __accessor_impl <_AccessIdentifier> type;

	static constexpr tplmp::classify_type classification = identifier::classification;

	typedef __accessor_impl_base ::field_accessor<_AccessIdentifier> base;
	typedef typename base::eval_type eval_type;

	using typename base::decl_class;
	using base::pobj;

	__accessor_impl(decl_class* pobj) :
			base(pobj)
	{
	}

	__accessor_impl(decl_class& obj) :
			base(obj)
	{
	}

	__attribute__((always_inline)) inline type& operator=(const eval_type& value)
	{
		base::store(pobj, value);
		return *this;
	}
};

template<typename _AccessIdentifier>
struct __accessor_impl<_AccessIdentifier, tplmp::classify_type::classify_type_memb_function> : public __accessor_impl_base, public __accessor_impl_base::function_accessor<_AccessIdentifier>
{
	typedef _AccessIdentifier identifier;
	typedef __accessor_impl <_AccessIdentifier> type;

	static constexpr tplmp::classify_type classification = identifier::classification;

	typedef __accessor_impl_base ::function_accessor<_AccessIdentifier> base;
	typedef typename base::eval_type eval_type;

	using typename base::decl_class;
	using base::pobj;

	__accessor_impl(decl_class* pobj) :
			base(pobj)
	{
	}

	__accessor_impl(decl_class& obj) :
			base(obj)
	{
	}

	template<typename ..._ArgTypes>
	__attribute__((always_inline)) inline auto operator()(_ArgTypes&& ... args) -> decltype(base::call(pobj, forward<_ArgTypes>(args)...))
	{
		return base::call(pobj, forward<_ArgTypes>(args)...);
	}
};

/**
 * @brief 对于指定类名和成员名的accessor类名
 */
template<typename _AccessIdentifier>
using accessor = __accessor_impl<_AccessIdentifier>;

/**
 * @brief 用于定义一个访问的ID标识
 */
#define __decl_accessor__(acc_id, namespaced_pmemb_id, decl_type)\
	struct acc_id: ::tplmp::__access_identifier<namespaced_pmemb_id, decl_type> {};
}

#endif //_TPLMP_ACCESS
