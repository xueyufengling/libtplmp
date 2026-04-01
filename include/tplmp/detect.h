#ifndef _TPLMP_DETECT
#define _TPLMP_DETECT

namespace tplmp
{
/**
 * @brief 是否存在指定名称的成员
 */
#define __check_exist_memb__(check_name, class_name) (check_name<class_name>::value)

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
 * @brief 是否存在指定名称和类型的成员
 */
#define __check_exist_memb_with_type__(check_name, class_name, type) (check_name<class_name, type>::value)

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
}

#endif //_TPLMP_DETECT
