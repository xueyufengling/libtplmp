#ifndef _TPLMP_TYPECHECK
#define _TPLMP_TYPECHECK

#include <tplmp/tplmp.h>

namespace tplmp
{

#define __assert_is_class__(class_name) static_assert(tplmp::is_class<class_name>::value, "'" #class_name "' must be class type")

template<typename _T>
struct is_ptr
{
	static const bool value = false;
};

template<typename _T>
struct is_ptr<_T*>
{
	static const bool value = true;
};

template<typename _T>
struct is_ref
{
	static const bool value = false;
};

template<typename _T>
struct is_ref<_T&>
{
	static const bool value = true;
};

template<typename _T>
struct is_x
{
	static const bool value = false;
};

template<typename _T>
struct is_x<_T&&>
{
	static const bool value = true;
};

/**
 * @brief 用作匹配具有非模板重载的重载模板函数的占位符。
 * 		  这个占位符对应的非模板重载函数参数类型必须是_T1/_T2，重载模板函数参数类型必须是_T2/_T1，且这两个参数必须传入本类对象。
 * 		  非模板重载函数的参数类型为_T1，以匹配的重载模板函数参数类型为_T2为例：
 * 		  (1)如果_T2可以转换为_T1，则目标重载模板函数的_T1模板重载和_T2模板重载均为有效候选者，但
 * 		  _T1模板重载接收const __conversion_placeholder* this参数，最终为_T1；
 * 		  _T2模板重载接收__conversion_placeholder* this参数，之后_T2可以二次转换最终为_T1。
 * 		  在通常情况下，这两个模板函数实例需要按照后述规则1，根据调用对象是否有const修饰符确定优先级。
 * 		  如果调用对象为非const对象，则优先调用operator _T2&()；若为const对象，则优先调用operator _T1&() const
 * 		  (2)如果_T2不能转换为_T1，则目标重载模板函数的_T1模板重载和_T2模板重载均为有效候选者，但
 * 		  _T1模板重载接收const __conversion_placeholder* this参数，最终为_T1；
 * 		  _T2模板重载接收__conversion_placeholder* this参数，最终为_T2。
 * 		  两种重载都合法，因此需要考虑非模板重载进一步决议。
 * 		  _T2模板重载与_T2非重载模板函数都接收非const对象，优先级高于_T1模板重载，但由于优先匹配非模板函数，因此重载决议为非模板重载函数_T2.
 * 		  注：在同等优先级下，函数匹配优先非模板函数，仅当非模板函数不存在时才实例化重载模板函数。函数匹配的优先级为：
 * 		  1. 参数类型完全匹配，包括形参和实参的const修饰符也能匹配
 * 		  2. 形参接收const T，实参为T
 * 		  3. 推广转换，小类型无损自动转为大类型，例如形参接收long long int，实参为int
 * 		  4. 标准转换，参数类型原生的可隐式转换，例如整形转浮点型，派生类->基类
 * 		  5. 用户定义转换（UDC），即构造函数和operator T()
 * 		  6. 能匹配上述任意一条规则，且具有变长参数列表，即最后形参为...
 * 		  综上所述，当传入实参为非const对象时，若_T2可转换为_T1，则始终匹配_T2&模板重载实例化；否则始终匹配_T1&非模板重载
 */
template<typename _T1, typename _T2>
struct __conversion_placeholder
{
	/**
	 * const关键字用于标记operator _T1&()是const函数，即operator _T1&(const __conversion_placeholder* this)，this为隐藏参数，其作用为：
	 * 1. 当_T1 = _T2时，两个operator转换函数不会导致重定义
	 * 2. const对象调用优先级更高
	 */
	inline operator _T1&() const
	{
		return decl<_T1>::ref();
	}

	inline operator _T1*() const
	{
		return decl<_T1>::ptr();
	}

	/**
	 * operator _T2&(__conversion_placeholder* this)，this为隐藏参数
	 * 非const对象调用优先级更高
	 */
	inline operator _T2&()
	{
		return decl<_T2>::ref();
	}

	inline operator _T2*()
	{
		return decl<_T2>::ptr();
	}

	/**
	 * @brief 非const对象
	 */
	__conversion_placeholder() = default;

	/**
	 * @brief const对象
	 */
	static constexpr __conversion_placeholder<_T1, _T2> _const{};
};

/**
 * @brief 测试_Original -> _Target是否可转换。
 * 		  包含隐式转换、用户自定义转换，通过函数重载决议优先级区分是否可转换，不需要实际的转换计算，因此避免了private、protected继承的情况下因权限问题而抛出编译错误
 */
template<typename _Original, typename _Target>
struct is_convertible
{
private:
	/**
	 * 注：此处不是SFINAE（因此不能使用变长参数列表...），而是函数重载决议优先级，即从多个有效候选函数中决议出最优的那个使用。
	 * 为了保证非模板函数优先匹配，第二个参数必须与传入__check()的第二个参数类型完全一致
	 */
	static _false __check(_Target, int) noexcept;

	template<typename _Any>
	static _true __check(_Original, _Any) noexcept;

public:
	/**
	 * 非const对象，若可转换则__conversion_placeholder()返回类型为_Original，否则返回类型有两种候选：_Original、_Target，需要与非模板函数进一步匹配决议。
	 * __check()未实际计算，不会抛出编译错误。
	 */
	static const bool value = decltype(__check(__conversion_placeholder<_Target, _Original>(), int()))::value;
};

/**
 * @brief 判断是否是基础类型，如果是则结果为true，如果不是（是类）则返回false。可以传入指针、引用等，得到的依旧是原本类型
 */
template<typename _T>
struct is_primitive
{
	static const bool value = false;
};

#define	__define_primitive_type__(_T)\
		template<>\
		struct is_primitive<_T>\
		{\
			static const bool value = true;\
		};

__define_primitive_type__(char)
__define_primitive_type__(unsigned char)
__define_primitive_type__(short int)
__define_primitive_type__(unsigned short int)
__define_primitive_type__(int)
__define_primitive_type__(unsigned int)
__define_primitive_type__(long int)
__define_primitive_type__(unsigned long int)
__define_primitive_type__(long long int)
__define_primitive_type__(unsigned long long int)
__define_primitive_type__(float)
__define_primitive_type__(double)
__define_primitive_type__(long double)
__define_primitive_type__(bool)
__define_primitive_type__(void)

#undef __define_primitive_type__

/**
 * @brief 判断T是否是类
 */
template<typename _T>
struct is_class
{
	static const bool value = !is_primitive<_T>::value && !is_ptr<_T>::value;
};

/**
 * @brief 检测是否存在继承关系
 */
template<typename _Derived, typename _Base>
struct is_derived
{
	static const bool value = is_class<_Derived>::value && is_class<_Base>::value && is_convertible<_Derived, _Base>::value;
};

/**
 * @brief 检测是否可赋值
 */
template<typename _Src, typename _Target>
struct is_assignable
{
	static const bool value = type_equal<_Src, _Target>::value || is_convertible<_Src, _Target>::value;
};

/*
 * @brief 判断类中是否存在某个名称的成员，即具有指定名称的字段、函数、类型均可被判定为具有该成员，绕过访问权限。
 *		  用法：在文件开头__decl_exist_memb__(检查器名, 成员名)，并使用__check_exist_memb__(检查器名, 类名)::value得到结果。
 *		  原理：
 *		  在模板参数推导过程中，对存在歧义的成员取成员取成员指针会不是硬错误，可以根据SFINAE机制不实例化该模板函数，转而调用__check(...)函数。注意如果在模板默认值而非推导中对不存在的成员取成员指针时会报错有歧义。
 *		  需要构造一个继承自被检测目标类(_Class)并继承自另一个具有public权限的与被检测目标同名的成员的类(__dummy_decl，此时两个歧义成员都是继承而来的同一层级，不会互相覆盖)的伪造类(__dummy_class)，这样伪造类就具有一个或两个相同名称的成员，如果有两个则产生歧义。该方案缺陷是无法检测ubion和final类，因为它们不能被继承。
 *		  因此，不能直接对伪造类取成员指针，而应该将伪造类作为模板参数，对这个模板参数取成员指针，此时产生歧义就是在参数推导过程中取指针失败，可以通过SFINAE机制解决而非抛出编译错误。
 */
#define __decl_exist_memb__(check_name, memb_name)\
		template<typename _Class>\
		struct check_name\
		{\
		private:\
			check_name() = delete;\
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
			static const bool value = __check<__dummy_class>(nullptr);\
		};
/**
 * @brief 是否存在指定名称的成员
 */
#define __check_exist_memb__(check_name, class_name) (check_name<class_name>::value)

/**
 * @brief 判断类中是否存在某个名称和类型的成员，需要public访问权限或将本类添加为目标类的friend class。
 * 		  __check()必须有个区别于_Class的类型参数_DeducedClass，实际调用时_DeducedClass被替换成_Class，如果目标不存在则属于推导错误，可以使用SFINAE机制。如果直接&_Class::memb_name由于_Class在推导阶段前就已知，目标不存在导致硬错误编译失败
 * 		  注：在gcc version 14.1.0 (Rev3, Built by MSYS2 project)版本中，检查类中存在但类型不匹配的成员时，__check<>()推导过程中抛出编译器内部错误：internal compiler error: in instantiate_type, at cp/class.cc:9115
 *   	  在要检测的目标类中使用，将检测类全部设置为目标类的friend class。
 * 		  注：必须指定命名空间的原因是，如果检测类Detect的命名空间A与目标类的命名空间B不同，那么不带命名空间所声明的friend class实际上是B::Detect，而不是A::Detect，将导致设置友元失败
 */
#define __decl_exist_memb_with_type__(check_name, memb_name)\
		template<typename _Class, typename _T>\
		struct check_name\
		{\
		private:\
			check_name() = delete;\
			template<typename _DeducedClass = _Class, _T _DeducedClass::*_pMemb = &_DeducedClass::memb_name>\
			static constexpr bool __check(int)\
			{\
				return true;\
			}\
			static constexpr bool __check(...)\
			{\
				return false;\
			}\
		public:\
			static const bool value = __check(int());\
		};

/**
 * @brief 是否存在指定名称和类型的成员
 */
#define __check_exist_memb_with_type__(check_name, class_name, type) (check_name<class_name, type>::value)

}

#endif //_TPLMP_TYPECHECK
