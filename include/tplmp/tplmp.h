#ifndef _TPLMP_TPLMP
#define _TPLMP_TPLMP

namespace tplmp
{

template<bool _AssertCond, typename ..._Ts>
struct __ce_print_types_t
{
};

template<typename ..._Ts>
struct __ce_print_types_t<false, _Ts...>
{
private:
	/**
	 * @brief 使编译器在错误信息中打印_Ts...参数包的实际推导类型
	 */
	template<typename ...>
	struct __deduced_type_list
	{
	};

public:
	typedef typename __deduced_type_list<_Ts...>::see_the_following_list _;
};

/**
 * @brief 断言_AssertCond，断言失败则在错误信息中打印后面传入的全部类型
 */
#define __ce_print_types__(_AssertCond, ...) template class tplmp::__ce_print_types_t<_AssertCond, ##__VA_ARGS__>

#define __assert_is_class__(class_name) static_assert(tplmp::is_class<class_name>::value, "'" #class_name "' must be class type")

#define __assert_classification_equal__(classification1, classification2) static_assert(classification1 == classification2, "classification '" #classification1 "' and '" #classification2 "' not match")

#define __assert_pack_size_equal__(pack1, pack2) static_assert(sizeof...(pack1) == sizeof...(pack2), "pack size of '" #pack1 "' and '" #pack2 "' not match")

#define __assert_index__(index, length) static_assert(index >=0 && index < length, "index '" #index "' is out of length '" #length "'")

#define __assert_pack_index__(index, pack) static_assert(index >=0 && index < sizeof...(pack), "index '" #index "' is out of pack '" #pack "'")

#define __assert_type_equal__(type1, type2) static_assert(tplmp::type_equal<type1, type2>::value, "type '" #type1 "' and '" #type2 "' not match")

#define __assert_not_impl__(type_or_value) static_assert(false, "specialization of '" #type_or_value "' not implemented")

/**
 * @brief 左值强制类型转换
 */
template<typename _T1, typename _T2>
__attribute__((always_inline)) inline constexpr _T1& cast(_T2& lv)
{
	return *(_T1*)(&lv);
}

template<typename _T1, typename _T2>
__attribute__((always_inline)) inline constexpr _T1& cast(_T2&& rv)
{
	return *(_T1*)(&(const _T2&)rv);
}

/**
 * @brief 将常量值打包成类型。
 * 		  模板元编程的一等公民。
 */
template<typename _T, _T _Value>
struct _constexpr
{
	typedef _T type;
	static constexpr type value = _Value;
};

#define __constexpr__(value) tplmp::_constexpr<decltype(value), value>

/**
 * @brief 判断目标是否是_constexpr<>
 */
template<typename _Constexpr>
struct is_constexpr
{
	typedef void type;
	static constexpr bool value = false;
};

template<typename _T, _T _Value>
struct is_constexpr<_constexpr<_T, _Value>>
{
	typedef typename _constexpr<_T, _Value>::type type;
	static constexpr bool value = true;
};

/**
 * @brief 判断两个类型是否相同
 */
template<typename _T1, typename _T2>
struct type_equal
{
	static const bool value = false;
};

template<typename _T>
struct type_equal<_T, _T>
{
	static const bool value = true;
};

/**
 * @brief 声明变量，val()、ref()、x()函数只有声明没有定义，只能在decltype()表达式中使用
 * 		  placeholder()返回值可以用于作为参数传递协助类型推导，但不能访问
 */
struct __decl_impl_base
{
protected:
	template<typename _T>
	struct __decl_impl
	{
	public:
		typedef _T type;

		static constexpr type val() noexcept;

		static constexpr type&& x() noexcept;

		inline static constexpr type* ptr() noexcept
		{
			return nullptr;
		}

		/**
		 * @brief 返回一个占位符值，可以赋值、传值，但不能访问
		 * 		  此函数不是constexpr，编译时如果对nullptr解引用会抛出编译错误。但此函数仍可在编译时在给static const变量赋值表达式中使用
		 */
		static type& ref() noexcept
		{
			static type* _nullptr = ptr();
			return *_nullptr;
		}
	};
};

template<typename _T>
struct decl: __decl_impl_base
{
	typedef typename __decl_impl<_T>::type type;
};

// 函数类型的特化，函数类型是不可被声明的，只有函数指针可以声明，这里将函数类型转换为函数指针
template<typename _RetType, typename ... _ArgTypes>
struct decl<_RetType(_ArgTypes...)> : __decl_impl_base
{
	typedef typename __decl_impl<_RetType (*)(_ArgTypes...)>::type type;
};

/**
 * @brief 返回目标表达式的类型
 * 		  用法例如typedef decltype(type_of_expr(x)) x_type;
 */
template<typename _T>
constexpr _T type_of_expr(_T) noexcept;

/**
 * @brief 对目标表达式进行求值返回结果
 */
template<typename _T>
constexpr _T eval_expr(_T) noexcept;

template<typename _Class, typename _T>
constexpr _T eval_expr(_T _Class::*) noexcept;

/**
 * @brief 对函数求值，得到函数返回值类型，由于只有声明没有定义，该函数仅能在decltype()内用作类型推导
 */
template<typename _RetType, typename ... _ArgTypes>
constexpr _RetType eval_expr(_RetType (*)(_ArgTypes...)) noexcept;

template<typename _Class, typename _RetType, typename ... _ArgTypes>
constexpr _RetType eval_expr(_RetType (_Class::*)(_ArgTypes...)) noexcept;

template<typename _T>
struct eval_type
{
	typedef decltype(eval_expr(decl<_T>::val())) type; //结果类型
};

/**
 * @brief 条件判断和相关功能
 */
template<bool _Cond>
struct if_else
{
	__assert_not_impl__(_Cond);
};

template<>
struct if_else<true>
{
	static const bool value = true;

	/**
	 * @brief 可用于选择性实例化类模板
	 * 		  判断_Cond是否为true，如果是则type类型为_TrueType，否则为_FalseType
	 */
	template<typename _TrueType, typename _FalseType>
	struct resolve_t
	{
		typedef _TrueType type;
	};

	template<typename _TrueType, _TrueType _TrueValue, typename _FalseType, _FalseType _FalseValue>
	struct resolve_t<_constexpr<_TrueType, _TrueValue>, _constexpr<_FalseType, _FalseValue>>
	{
		typedef _TrueType type;
		static const _TrueType value = _TrueValue;
	};

	template<typename _T>
	struct enable
	{
		typedef _T type;
	};

	template<typename _TrueReturn, typename _FalseReturn>
	__attribute__((always_inline)) inline static constexpr _TrueReturn _return(_TrueReturn true_val, _FalseReturn false_val) noexcept
	{
		return true_val;
	}

	/**
	 * @brief 条件成立时可以调用，否则忽略
	 */
	template<typename _Callable, typename ..._ArgTypes>
	__attribute__((always_inline)) inline static auto call(_Callable callable, _ArgTypes ...args) -> decltype(callable(args...))
	{
		return callable(args...);
	}
};

template<>
struct if_else<false>
{
	static const bool value = false;

	template<typename _TrueType, typename _FalseType>
	struct resolve_t
	{
		typedef _FalseType type;
	};

	template<typename _TrueType, _TrueType _TrueValue, typename _FalseType, _FalseType _FalseValue>
	struct resolve_t<_constexpr<_TrueType, _TrueValue>, _constexpr<_FalseType, _FalseValue>>
	{
		typedef _FalseType type;
		static const _FalseType value = _FalseValue;
	};

	template<typename _T>
	struct enable
	{
	};

	template<typename _TrueReturn, typename _FalseReturn>
	__attribute__((always_inline)) inline static constexpr _FalseReturn _return(_TrueReturn true_val, _FalseReturn false_val) noexcept
	{
		return false_val;
	}

	template<typename _Callable, typename ..._ArgTypes>
	__attribute__((always_inline)) inline static auto call(_Callable callable, _ArgTypes ...args) -> decltype(callable(args...))
	{
	}
};

struct bool_type
{
	using _true = __constexpr__(true);
	using _false = __constexpr__(false);

	template<typename _T>
	inline static constexpr bool bool_of(void)
	{
		return type_equal<_T, _true>::value;
	}

	template<bool _Value>
	struct of
	{
		typedef typename if_else<_Value>
		::resolve_t<
				_true,
				_false>
		::type type;
	};
};

/**
 * 得到表达式的bool_type返回类型对应的值，不会实际计算表达式，能避免一些编译错误
 */
#define __bool_type_expr__(expr) tplmp::bool_type::bool_of<decltype(expr)>()

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
	static bool_type::_false __check(_Target, int) noexcept;

	template<typename _Any>
	static bool_type::_true __check(_Original, _Any) noexcept;

public:
	/**
	 * 非const对象，若可转换则__conversion_placeholder()返回类型为_Original，否则返回类型有两种候选：_Original、_Target，需要与非模板函数进一步匹配决议。
	 * bool_of_expr()内部没有实际进行计算，不会抛出编译错误
	 */
	static const bool value = __bool_type_expr__(__check(__conversion_placeholder<_Target, _Original>(), int()));
};

/**
 * @brief 类型的分类，必定是以下之一
 */
enum type_classification : int
{
	VARIABLE, //普通变量
	FUNCTION, //普通函数
	ORID_NUM,
	MEMB_FIELD = ORID_NUM, //成员字段
	MEMB_FUNCTION, //成员函数
};

using type_classification_variable_t = __constexpr__(type_classification::VARIABLE);
using type_classification_function_t = __constexpr__(type_classification::FUNCTION);
using type_classification_memb_field_t = __constexpr__(type_classification::MEMB_FIELD);
using type_classification_memb_function_t = __constexpr__(type_classification::MEMB_FUNCTION);

/**
 * @brief 值的种类判断
 */
template<typename _T>
struct type_classification_of_t
{
	static const type_classification value = type_classification::VARIABLE;
};

template<typename _RetType, typename ... _ArgTypes>
struct type_classification_of_t<_RetType(_ArgTypes...)>
{
	static const type_classification value = type_classification::FUNCTION;
};

template<typename _RetType, typename ... _ArgTypes>
struct type_classification_of_t<_RetType (*)(_ArgTypes...)>
{
	static const type_classification value = type_classification::FUNCTION;
};

template<typename _Class, typename _T>
struct type_classification_of_t<_T _Class::*>
{
	static const type_classification value = type_classification::MEMB_FIELD;
};

template<typename _Class, typename _RetType, typename ... _ArgTypes>
struct type_classification_of_t<_RetType (_Class::*)(_ArgTypes...)>
{
	static const type_classification value = type_classification::MEMB_FUNCTION;
};

template<typename _T>
inline constexpr type_classification type_classification_of(_T*)
{
	return type_classification::VARIABLE;
}

template<typename _RetType, typename ... _ArgTypes>
inline constexpr type_classification type_classification_of(_RetType (*)(_ArgTypes...))
{
	return type_classification::FUNCTION;
}

template<typename _Class, typename _T>
inline constexpr type_classification type_classification_of(_T _Class::*)
{
	return type_classification::MEMB_FIELD;
}

template<typename _Class, typename _RetType, typename ... _ArgTypes>
inline constexpr type_classification type_classification_of(_RetType (_Class::*)(_ArgTypes...))
{
	return type_classification::MEMB_FUNCTION;
}

inline constexpr type_classification to_orid_classification(type_classification classification)
{
	return classification > type_classification::ORID_NUM ? (type_classification)(classification - type_classification::ORID_NUM) : classification;
}

inline constexpr type_classification to_memb_classification(type_classification classification)
{
	return classification < type_classification::ORID_NUM ? (type_classification)(classification + type_classification::ORID_NUM) : classification;
}

inline constexpr bool is_memb_classification(type_classification classification)
{
	return classification == type_classification::MEMB_FIELD || classification == type_classification::MEMB_FUNCTION;
}

// ***** 类型参数包 *****

inline static constexpr bool is_index_valid(int idx, int length)
{
	return idx >= 0 && idx < length;
}

struct __type_at_impl_base //避免外部模板类的不同实例都实例化相同的__type_at_impl<>模板，又限制外部访问
{
protected:
	struct __type_at_impl_oob //索引越界
	{
	private:
		struct oob;

	public:
		typedef oob type; //无效索引
	};

	template<int _Index, typename _First, typename ... _RestParams>
	struct __type_at_impl
	{
		typedef typename if_else<_Index <= 0>
		::resolve_t<
				_First,
				typename __type_at_impl<_Index - 1, _RestParams...>::type
		>::type type;
	};

	template<int _Index, typename _First>
	struct __type_at_impl<_Index, _First> //迭代终止偏特化
	{
		typedef _First type;
	};
};

/**
 * @brief 获取指定索引的参数类型，索引从0开始
 * 		  如果_Params为空则将断言失败，索引越界。
 */
template<int _Index, typename ... _Params>
struct type_at: __type_at_impl_base, public if_else<is_index_valid(_Index, sizeof...(_Params))>
		::resolve_t<
				__type_at_impl_base:: __type_at_impl <_Index, _Params...>,
				__type_at_impl_base:: __type_at_impl_oob
				>::type
{
};

/**
 * @brief 类型参数包迭代器
 */
template<int _Index, typename ..._Params>
struct iterator;

struct __iterator_impl_base
{
protected:
	// 超出索引且不是end迭代器的无效迭代器
	struct __iterator_impl_oob
	{
		static const int index = -1;

		typedef __iterator_impl_oob prev;
		typedef __iterator_impl_oob next; //下一个还是本类

		static const bool has_prev = false;
		static const bool has_next = false;
	};

	template<int _Index, typename ..._Params>
	struct __iterator_impl
	{
		static const int index = _Index;

		__assert_pack_index__(index, _Params); //检查边界

		typedef typename type_at<index, _Params...>::type type;

		typedef iterator<index - 1, _Params...> prev; //上一个迭代器
		typedef iterator<index + 1, _Params...> next; //下一个迭代器

		static const bool has_prev = true;
		static const bool has_next = true;
	};

	//最后一个元素之后的下一个迭代器，即end迭代器
	template<typename ..._Params>
	struct __iterator_impl<sizeof...(_Params), _Params...>
	{
		static const int index = sizeof...(_Params);

		typedef iterator<index - 1, _Params...> prev; //上一个迭代器是最后一个元素
		typedef __iterator_impl_oob next;

		static const bool has_prev = sizeof...(_Params) == 0 ? false : true;
		static const bool has_next = false;
	};
};

template<int _Index, typename ..._Params>
struct iterator: __iterator_impl_base, public if_else<_Index >= 0 && _Index <= sizeof...(_Params)>
		::resolve_t<
				__iterator_impl_base:: __iterator_impl <_Index, _Params...>,
				__iterator_impl_base:: __iterator_impl_oob
				>::type
{
};

/**
 * @brief 类型参数包，用于保存模板参数而不展开，对于值参数，需要使用_constexpr<>将值封装为类型
 */
template<typename ..._Params>
struct type_pack
{
	/**
	 * @brief 参数包长度
	 */
	static const int size = sizeof...(_Params);

	/**
	 * @brief 将参数包用作成员函数参数类型并获取成员函数指针的类型
	 */
	template<typename _Class, typename _RetType>
	struct func_type
	{
		typedef _Class decl_class;
		typedef _RetType (_Class::*type)(_Params ...);
		typedef _RetType return_type;
	};

	/**
	 * @brief 将参数包用作函数参数类型并获取函数指针的类型
	 */
	template<typename _RetType>
	struct func_type<void, _RetType>
	{
		typedef void decl_class;
		typedef _RetType (*type)(_Params ...);
		typedef _RetType return_type;
	};

	/**
	 * @brief 指定索引的类型
	 */
	template<int _Index>
	struct at
	{
		typedef typename type_at<_Index, _Params...>::type type;
	};

	template<int _Index>
	using iterator_at = iterator<_Index, _Params...>;

	using begin = iterator_at<0>; //第一个元素的迭代器
	using end = iterator_at<size>; //迭代递归终止，必须通过对迭代器参数类型偏特化为end来终止递归，否则将无限递归直到报错

private:
	template<typename _T, typename _Iter>
	struct __first_index_of_impl
	{
		static const int value = if_else<type_equal<_T, typename _Iter::type>::value>
				::_return(
						_Iter::index, //类型相同则当前索引
						__first_index_of_impl<_T, typename _Iter::next>::value //类型不同则继续迭代下一个元素
						);
	};

	template<typename _T>
	struct __first_index_of_impl<_T, end>
	{
		static const int value = -1; //迭代至最后一个元素返回-1
	};

public:
	/**
	 * @brief 获取指定类型第一次出现时的索引
	 */
	template<typename _T>
	struct first_index_of
	{
		static const int value = size == 0 ? -1 : __first_index_of_impl<_T, begin>::value;
	};
};

template<typename _Value>
struct _switch
{
	template<typename ... _Cases>
	struct _case
	{
		typedef type_pack<_Cases...> cases;

		static const int value = cases::template first_index_of<_Value>::value;

		template<typename _Default, typename ... _Results>
		struct resolve_t
		{
			__assert_pack_size_equal__(_Cases, _Results);

			typedef typename if_else<value == -1>
			::resolve_t<
					_Default,
					typename type_at<
							value,
							_Results...
					>::type
			>::type type;
		};
	};
};

/**
 * @brief 变量或函数所属的类，如果不是成员则type为void
 */
template<typename _T>
struct decl_class
{
	typedef void type;
};

template<typename _Class, typename _T>
struct decl_class<_T _Class::*>
{
	typedef _Class type;
};

/**
 * @brief 变量或函数的声明类型，声明类型将成员指针类型转换为普通类型，成员函数丢失this参数
 */
template<typename _T>
struct decl_type
{
	typedef _T type;
};

template<typename _Class, typename _T>
struct decl_type<_T _Class::*>
{
	typedef _T type;
};

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
 * @brief 可以得到指针、引用等的原本类型，即传入T*或T**甚至更高次数的指针可以得到T
 */
template<typename _T>
struct type_of
{
	typedef _T type;
};

template<typename _T>
struct type_of<_T*>
{
	typedef typename type_of<_T>::type type; //递归地获取指针类型
};

template<typename _T>
struct type_of<_T&>
{
	typedef _T type;
};

template<typename _T>
struct type_of<_T&&>
{
	typedef _T type;
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

/**
 * @brief 包装指针的类型，_Class=void则视作非成员类型，否则视作成员类型
 */
template<typename _Class, typename _T>
struct ptr_type
{
	static const type_classification classification = to_memb_classification(type_classification_of_t<_T>::value); //_Class不为void则转换为对应的成员类型

	typedef _T _Class::*type;
};

template<typename _T>
struct ptr_type<void, _T>
{
	static const type_classification classification = to_orid_classification(type_classification_of_t<_T>::value);

	typedef _T* type;
};

}

#endif //_TPLMP_TPLMP