#ifndef _TPLMP_ACCESS
#define _TPLMP_ACCESS

#include <tplmp/tplmp.h>

namespace tplmp
{
/**
 * @brief 储存类_Class的成员指针。
 * 		  字段成员指针和函数成员指针不能互相cast，因此使用union。
 */
template<typename _Class>
class __universal_pmemb
{
	assert_class(_Class);

private:
	union
	{
		int _Class::*field; //任意类型的字段
		void (_Class::*function)(); //任意类型的成员函数
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
		typedef typename tplmp::pmemb_type<_Class, _T>::type type;

		inline static constexpr type cast(__universal_pmemb <_Class> pmemb)
		{
			return (type)pmemb.field;
		}
	};

	//成员函数不能偏特化，因此使用模板类偏特化实现cast
	template<typename _RetType, typename ... _ArgTypes>
	struct __cast_impl<_RetType(_ArgTypes...)>
	{
		typedef typename tplmp::pmemb_type<_Class, _RetType(_ArgTypes...)>::type type;

		inline static constexpr type cast(__universal_pmemb <_Class> pmemb)
		{
			return (type)pmemb.function;
		}
	};

public:
	__universal_pmemb() = default;

	template<typename _T>
	inline __universal_pmemb(_T _Class::*pmemb)
	{
		store(pmemb);
	}

	template<typename _T>
	inline constexpr typename tplmp::pmemb_type<_Class, _T>::type cast()
	{
		return __cast_impl<_T>::cast(*this);
	}

	template<typename _T>
	inline constexpr operator typename tplmp::pmemb_type<_Class, _T>::type()
	{
		return cast();
	}

	template<typename _T>
	inline __universal_pmemb <_Class>& operator=(_T _Class::*pmemb)
	{
		store(pmemb);
		return *this;
	}
};

/**
 * @brief 成员指针的标识符，仅用作占位符用以区分不同成员，任何一个成员都需要具有其独特的标识符，即定义一个类继承自该类
 */
template<typename _Class>
struct __pmemb_identifier
{
	assert_class(_Class); //静态断言，只有类成员可以有__access_identifier

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

	static tplmp::type_classification classification; //该标识符对应的类型

	static __universal_pmemb <decl_class> _pmemb;

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
			__pmemb_value<_pMembIdentifier>::classification = tplmp::type_classification_of_t<_pMemb>::value;
			__pmemb_value<_pMembIdentifier>::_pmemb = _pMembValue;
		}

		static const __initializer <_pMemb, _pMembValue> _init;
	};
};
template<typename _pMembIdentifier>
tplmp::type_classification __pmemb_value<_pMembIdentifier>::classification = tplmp::type_classification::MEMB_FIELD;

template<typename _pMembIdentifier>
__universal_pmemb <typename _pMembIdentifier::decl_class> __pmemb_value<_pMembIdentifier>::_pmemb = __universal_pmemb<typename _pMembIdentifier::decl_class>();

template<typename _pMembIdentifier>
template<typename _pMemb, _pMemb _pMembValue>
const typename __pmemb_value<_pMembIdentifier>::__initializer<_pMemb, _pMembValue> __pmemb_value<_pMembIdentifier>::__initializer<_pMemb, _pMembValue>::_init{};

/**
 * @brief 用于定义一个访问的ID标识
 */
#define decl_pmemb(pmemb_id, target_class)\
	struct pmemb_id: tplmp::__pmemb_identifier<target_class> {}

/**
 * @brief 初始化，即执行取成员指针的行为。
 * 		  此宏将为decl_pmemb()声明的成员指针标识符赋值。
 * 		  此宏只能定义一次，因为模板的显式实例化只能声明一次，多次声明将抛出编译错误。
 */
#define fetch_pmemb(pmemb_id, memb_name) template class tplmp::__pmemb_value<pmemb_id>::__initializer<decltype(&pmemb_id::decl_class::memb_name), &pmemb_id::decl_class::memb_name>

#define read_pmemb(pmemb_id) (tplmp::__pmemb_value<pmemb_id>::_pmemb)

/**
 * @brief 访问标识符，将成员指针标识符绑定一个类型。
 */
template<typename _pMembIdentifier, typename _MembType>
struct __access_identifier
{
	typedef _pMembIdentifier pmemb_identifier;
	typedef typename _pMembIdentifier::decl_class decl_class;

	typedef _MembType decl_type; //成员声明的类型
	typedef typename tplmp::pmemb_type<decl_class, _MembType>::type pmemb_type; //成员指针类型
	typedef typename tplmp::eval_type<_MembType>::type eval_type; //成员求值类型，对字段而言是声明类型，对函数而言是返回类型

	static const tplmp::type_classification classification = tplmp::type_classification_of_t<pmemb_type>::value; //成员分类，判断是成员字段还是成员函数

	__access_identifier() = delete;
};

//成员字段访问的实现
template<typename _AccessIdentifier>
struct __accessor_impl_base
{
	typedef typename _AccessIdentifier::pmemb_identifier pmemb_identifier;
	typedef typename _AccessIdentifier::decl_class decl_class;

	typedef typename _AccessIdentifier::pmemb_type pmemb_type;
	typedef typename _AccessIdentifier::eval_type eval_type;

	decl_class* pobj;

	__accessor_impl_base(decl_class* pobj)
	{
		this->pobj = pobj;
	}

	__accessor_impl_base(decl_class& obj)
	{
		pobj = &obj;
	}

	template<typename _Derived>
	_Derived operator=(const __accessor_impl_base <_AccessIdentifier>&) = delete;
};

/**
 * @brief 访问私有成员，若目标成员不存在则会直接抛出编译错误
 * 		  原理：在显式实例化模板给其指针类型模板参数直接赋值时，可以绕过访问修饰符直接取到成员指针，需要将这个取到的成员指针在本模板以外之处储存起来在使用。
 * 		  注意在显式实例化完成以后，因其模板参数包含了对private成员取指针，故被显示实例化的模板将无法声明或使用，否则会受到访问修饰符限制并报错。
 */
template<typename _AccessIdentifier, tplmp::type_classification _Classification>
struct __accessor_impl
{
};

template<typename _AccessIdentifier>
struct __accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FIELD> : public __accessor_impl_base<_AccessIdentifier>
{
	using typename __accessor_impl_base<_AccessIdentifier>::pmemb_identifier;
	using typename __accessor_impl_base<_AccessIdentifier>::decl_class;

	using typename __accessor_impl_base<_AccessIdentifier>::pmemb_type;
	using typename __accessor_impl_base<_AccessIdentifier>::eval_type;

	using __accessor_impl_base<_AccessIdentifier>::pobj;

	static pmemb_type pmemb;

	__attribute__((always_inline)) inline __accessor_impl(decl_class* pobj) :
			__accessor_impl_base<_AccessIdentifier>(pobj)
	{
	}

	__attribute__((always_inline)) inline __accessor_impl(decl_class& obj) :
			__accessor_impl_base<_AccessIdentifier>(obj)
	{
	}

	__attribute__((always_inline)) inline operator eval_type&()
	{
		return pobj->*pmemb;
	}

	__attribute__((always_inline)) inline __accessor_impl <_AccessIdentifier, tplmp::type_classification::MEMB_FIELD>& operator=(const eval_type& value)
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
template<typename _AccessIdentifier>
typename __accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FIELD>::pmemb_type
__accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FIELD>::pmemb =
		(typename __accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FIELD>::pmemb_type)__pmemb_value<
				typename __accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FIELD>::pmemb_identifier
		>::_pmemb;

//成员函数访问的实现
template<typename _AccessIdentifier>
struct __accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FUNCTION> : public __accessor_impl_base<_AccessIdentifier>
{
	using typename __accessor_impl_base<_AccessIdentifier>::pmemb_identifier;
	using typename __accessor_impl_base<_AccessIdentifier>::decl_class;

	using typename __accessor_impl_base<_AccessIdentifier>::pmemb_type;
	using typename __accessor_impl_base<_AccessIdentifier>::eval_type;

	using __accessor_impl_base<_AccessIdentifier>::pobj;

	static pmemb_type pmemb;

	__attribute__((always_inline)) inline __accessor_impl(decl_class* pobj) :
			__accessor_impl_base<_AccessIdentifier>(pobj)
	{
	}

	__attribute__((always_inline)) inline __accessor_impl(decl_class& obj) :
			__accessor_impl_base<_AccessIdentifier>(obj)
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
typename __accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FUNCTION>::pmemb_type
__accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FUNCTION>::pmemb =
		(typename __accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FUNCTION>::pmemb_type)__pmemb_value<
				typename __accessor_impl<_AccessIdentifier, tplmp::type_classification::MEMB_FUNCTION>::pmemb_identifier
		>::_pmemb;

template<typename _AccessIdentifier>
struct __accessor: public __accessor_impl<_AccessIdentifier, _AccessIdentifier::classification>
{
	typedef _AccessIdentifier identifier;

	static constexpr tplmp::type_classification classification = identifier::classification;

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
#define accessor(acc_id) tplmp::__accessor<acc_id>

/**
 * @brief 用于定义一个访问的ID标识
 */
#define decl_accessor(acc_id, pmemb_id, decl_type)\
	struct acc_id: tplmp::__access_identifier<pmemb_id, decl_type> {}

/**
 * @brief 是否存在指定名称的成员
 */
#define exist_memb(...) __va_macro__(exist_memb, __VA_ARGS__)

#define exist_memb1(memb_name) __cats__(2)(__exist_memb_, memb_name)

#define exist_memb2(class_name, memb_name) exist_memb1(memb_name)<class_name>

/*
 * @brief 判断类中是否存在某个名称的成员，即具有指定名称的字段、函数、类型均可被判定为具有该成员，绕过访问权限。
 *		  用法：在文件开头decl_exist_memb(成员名)，并使用exist_memb(类名, 成员名)::result得到结果。
 *		  原理：
 *		  在模板参数推导过程中，对存在歧义的成员取成员取成员指针会不是硬错误，可以根据SFINAE机制不实例化该模板函数，转而调用__check(...)函数。注意如果在模板默认值而非推导中对不存在的成员取成员指针时会报错有歧义。
 *		  需要构造一个继承自被检测目标类(_Class)并继承自另一个具有public权限的与被检测目标同名的成员的类(__dummy_decl，此时两个歧义成员都是继承而来的同一层级，不会互相覆盖)的伪造类(__dummy_class)，这样伪造类就具有一个或两个相同名称的成员，如果有两个则产生歧义。该方案缺陷是无法检测ubion和final类，因为它们不能被继承。
 *		  因此，不能直接对伪造类取成员指针，而应该将伪造类作为模板参数，对这个模板参数取成员指针，此时产生歧义就是在参数推导过程中取指针失败，可以通过SFINAE机制解决而非抛出编译错误。
 */
#define decl_exist_memb(memb_name)\
		template<typename _Class>\
		struct exist_memb(memb_name)\
		{\
		private:\
			exist_memb(memb_name)() = delete;\
			struct __dummy_decl\
			{\
				char memb_name;\
			};\
			struct __dummy_class: _Class, __dummy_decl\
			{\
			};\
			template<typename _Dummy>\
			inline static constexpr bool __check(decltype(&_Dummy::memb_name))\
			{\
				return false;\
			}\
			template<typename _Dummy>\
			inline static constexpr bool __check(...)\
			{\
				return true;\
			}\
		public:\
			static const bool result = __check<__dummy_class>(nullptr);\
		};

/**
 * @brief 是否存在指定名称和类型的成员
 */
#define exist_memb_with_type(...) __va_macro__(exist_memb_with_type, __VA_ARGS__)

#define exist_memb_with_type1(memb_name) __exist_memb_with_type_##memb_name

#define exist_memb_with_type3(class_name, type, memb_name) exist_memb_with_type1(memb_name)<class_name, type>

/**
 * @brief 判断类中是否存在某个名称和类型的成员，需要public访问权限或将本类添加为目标类的friend class。
 * 		  __check()必须有个区别于_Class的类型参数_DeducedClass，实际调用时_DeducedClass被替换成_Class，如果目标不存在则属于推导错误，可以使用SFINAE机制。如果直接&_Class::memb_name由于_Class在推导阶段前就已知，目标不存在导致硬错误编译失败
 * 		  注：在gcc version 14.1.0 (Rev3, Built by MSYS2 project)版本中，检查类中存在但类型不匹配的成员时，__check<>()推导过程中抛出编译器内部错误：internal compiler error: in instantiate_type, at cp/class.cc:9115
 */
#define decl_exist_memb_with_type(memb_name)\
		template<typename _Class, typename _T>\
		struct exist_memb_with_type(memb_name)\
		{\
		private:\
			exist_memb_with_type(memb_name)() = delete;\
			template<typename _DeducedClass = _Class, _T _DeducedClass::*_MembPtr = &_DeducedClass::memb_name>\
			static constexpr bool __check(int)\
			{\
				return true;\
			}\
			static constexpr bool __check(...)\
			{\
				return false;\
			}\
		public:\
			static const bool result = __check(int());\
		};

/**
 * @brief 带命名空间的类型
 */
#define exist_memb_with_type_namespaced2(memb_name, _namespace) _namespace::__cats__(2)(__exist_memb_with_type_, memb_name)

#define exist_memb_with_type_namespaced4(class_name, type, memb_name, _namespace) exist_memb_with_type_namespaced2(memb_name, _namespace)<class_name, type>

/*
 * @brief 判断类中是否存在某个名称的函数，要求是public访问权限或者本类是目标类的friend class。
 * @params ... 名称列表
 */
#define decl_exist_memb_with_type(...) __repeat_each__(decl_exist_memb_with_type_intl, ##__VA_ARGS__)

#define __exist_memb_with_type_template_params__() typename, typename

/**
 * @brief 在要检测的目标类中使用，将检测类全部设置为目标类的friend class。
 * @param _namespace_macro 为空参数宏的宏名，指定检测类所属的命名空间，全局命名空间则留空，例如
 * 		  #define detect_namespace() andromeda::common
 * 		  注：必须指定命名空间的原因是，如果检测类Detect的命名空间A与目标类的命名空间B不同，那么不带命名空间所声明的friend class实际上是B::Detect，而不是A::Detect，将导致设置友元失败
 */
#define enable_exist_memb_with_type(_namespace_macro, ...) __template_friend_classes__(__exist_memb_with_type_template_params__, __op_each_extras__(exist_memb_with_type_namespaced, _namespace_macro, __VA_ARGS__))
}

#endif //_TPLMP_ACCESS
