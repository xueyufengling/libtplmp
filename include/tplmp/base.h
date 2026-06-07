#ifndef _TPLMP_TPLMP
#define _TPLMP_TPLMP

#include <stdint.h>

namespace tplmp
{

template<bool _AssertCond, typename ..._Ts>
struct __ce_print_types_t
{
	typedef void see_the_following_list;
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
//需要在namespace中声明
#define __ce_print_types__(_AssertCond, ...) template class tplmp::__ce_print_types_t<_AssertCond, ##__VA_ARGS__>
//需要在类体内声明
#define __ce_print_types_in__(_AssertCond, ...) typedef tplmp::__ce_print_types_t<_AssertCond, ##__VA_ARGS__>::see_the_following_list _

#define __assert_classification_equal__(classification1, classification2) static_assert(classification1 == classification2, "classification '" #classification1 "' and '" #classification2 "' not match")

#define __assert_pack_size_equal__(pack1, pack2) static_assert(sizeof...(pack1) == sizeof...(pack2), "pack size of '" #pack1 "' and '" #pack2 "' not match")

#define __assert_index__(index, length) static_assert(index >=0 && index < length, "index '" #index "' is out of length '" #length "'")

#define __assert_pack_index__(index, pack) static_assert(index >=0 && index < sizeof...(pack), "index '" #index "' is out of pack '" #pack "'")

#define __assert_type_equal__(type1, type2) static_assert(tplmp::type_equal<type1, type2>::value, "type '" #type1 "' and '" #type2 "' not match")

#define __assert_not_impl__(type_or_value) static_assert(false, "specialization of '" #type_or_value "' not implemented")

#define __crtp_this__(derived_type) ((derived_type*)this)

/**
 * @brief 左值强制类型转换
 */
template<typename _T1, typename _T2>
__attribute__((always_inline)) inline constexpr _T1& cast(const _T2& lv) noexcept
{
	return (_T1&)(lv);
}

/**
 * @brief 对类型包装一次，可通过type_t<>::type获取储存的类型。
 */
template<typename _T>
struct type_t
{
	typedef _T type;
};

/**
 * @brief 将类型包装为依赖类型，即值依赖于模板参数的类型。
 * 		  C++标准规定，当一个类型模板参数_T直接出现在模板的形参列表、返回类型、函数体中时，那么在实例化模板时，必须保证_T是完整类型。
 * 		  在模板实例化阶段，编译器不深入解析依赖类型内部，只保留语法结构。如果模板参数为type_t<_T>，则不要求依赖类型_T实例化或有定义。
 */
template<typename _TypeHolder>
struct lazy_t
{
	typedef typename _TypeHolder::type type; //type依赖类型_T
};

/**
 * @brief 将模板包装为依赖类型，延迟实例化模板_LazyType<>，用于延迟求值。
 */
template<template<typename ...> typename _LazyType, typename ..._LazyTypeParams>
struct lazy_tpl
{
	typedef typename _LazyType<_LazyTypeParams...>::type type;
};

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
 * @brief 变量
 */
template<typename _T>
struct variable
{
	typedef _T type;
	type value;

	variable() = default;

	variable(const _T& value) :
			value(value)
	{
	}
};

/**
 * @brief 基本类型算术实现定义。
 * 		  result_op_type必须能和op_type进行op指定的运算；
 * 		  且result_type模板定义必须是result_type<result_op_type _Value>。
 */
#define __operator_impl__(op_name, result_type, result_op_type, op, op_type)\
		template<result_op_type _Result, op_type _First, op_type ..._OpValues>\
		struct op_name##_intl\
		{\
			typedef typename op_name##_intl<_Result op _First, _OpValues...>::type type;\
		};\
		template<result_op_type _Result, op_type _First>\
		struct op_name##_intl<_Result, _First>\
		{\
			typedef result_type<_Result op _First> type;\
		};\
		template<result_op_type _Result, op_type ..._OpValues>\
		struct op_name\
		{\
			typedef typename op_name##_intl<_Result, _OpValues...>::type type;\
		};\
		template<result_op_type _Result>\
		struct op_name<_Result>\
		{\
			typedef result_type<_Result> type;\
		};

/**
 * @brief bool类型常量，支持逻辑运算
 */
template<bool _Value>
using _bool = _constexpr<bool, _Value>;

struct __bool_impl_base
{
protected:
	//逻辑与
	__operator_impl__(__and_impl, _bool, bool, &&, bool)

	//逻辑或
	__operator_impl__(__or_impl, _bool, bool, ||, bool)
};

template<bool _Value>
struct _constexpr<bool, _Value> : __bool_impl_base
{
	typedef bool type;
	static constexpr type value = _Value;

	struct _not
	{
		typedef _bool<!_Value> type;
	};

	template<bool ..._OpValues>
	struct _and
	{
		typedef typename __and_impl<_Value, _OpValues...>::type type;
	};

	template<bool ..._OpValues>
	struct _or
	{
		typedef typename __or_impl<_Value, _OpValues...>::type type;
	};
};

using _true = _bool<true>;
using _false = _bool<false>;

/**
 * @brief 数值类型的_constexpr<>封装，支持算术运算
 */
#define __def_integer_constexpr__(constexpr_name, int_type)\
	template<int_type _Value>\
	using constexpr_name = tplmp::_constexpr<int_type, _Value>;\
	struct constexpr_name##_impl_base\
	{\
	protected:\
		__operator_impl__(__add_impl, constexpr_name, int_type, +, int_type)\
		__operator_impl__(__sub_impl, constexpr_name, int_type, -, int_type)\
		__operator_impl__(__mul_impl, constexpr_name, int_type, *, int_type)\
		__operator_impl__(__div_impl, constexpr_name, int_type, /, int_type)\
		__operator_impl__(__mod_impl, constexpr_name, int_type, %, int_type)\
	};\
	template<int_type _Value>\
	struct _constexpr<int_type, _Value> : constexpr_name##_impl_base\
	{\
		typedef int_type type;\
		static constexpr int_type value = _Value;\
		template<int_type ... _OpValues>\
		struct add\
		{\
			typedef typename __add_impl<_Value, _OpValues...>::type type;\
		};\
		template<int_type ... _OpValues>\
		struct sub\
		{\
			typedef typename __sub_impl<_Value, _OpValues...>::type type;\
		};\
		struct inc\
		{\
			typedef typename add<1>::type type;\
		};\
		struct dec\
		{\
			typedef typename sub<1>::type type;\
		};\
		template<int_type ... _OpValues>\
		struct mul\
		{\
			typedef typename __mul_impl<_Value, _OpValues...>::type type;\
		};\
		template<int_type ... _OpValues>\
		struct div\
		{\
			typedef typename __div_impl<_Value, _OpValues...>::type type;\
		};\
		template<int_type ... _OpValues>\
		struct mod\
		{\
			typedef typename __mod_impl<_Value, _OpValues...>::type type;\
		};\
	};

__def_integer_constexpr__(_char, char)
__def_integer_constexpr__(unsigned_char, unsigned char)
__def_integer_constexpr__(_short, short)
__def_integer_constexpr__(unsigned_short, unsigned short)
__def_integer_constexpr__(_int, int)
__def_integer_constexpr__(unsigned_int, unsigned int)
__def_integer_constexpr__(_long, long)
__def_integer_constexpr__(unsigned_long, unsigned long)
__def_integer_constexpr__(long_long, long long)
__def_integer_constexpr__(unsigned_long_long, unsigned long long)

#define __def_integer_constexpr_alias__(constexpr_name, int_type, alias_name)\
	template<int_type _Value>\
	using alias_name = tplmp::constexpr_name<_Value>;\

__def_integer_constexpr_alias__(unsigned_long_long, unsigned long long, _size_t)

#undef __def_integer_constexpr_alias__
#undef __def_integer_constexpr__

template<typename _IntType, _IntType _Index>
struct integer_iterator
{
	static const size_t index = _Index;

	typedef _constexpr<_IntType, index> type;

	template<_IntType _Stride>
	struct prev_i
	{
		typedef integer_iterator<_IntType, index - _Stride> type;
	};

	template<_IntType _Stride>
	struct next_i
	{
		typedef integer_iterator<_IntType, index + _Stride> type;
	};

	typedef typename prev_i<1>::type prev; //上一个迭代器
	typedef typename next_i<1>::type next; //下一个迭代器

	static const bool has_prev = true;
	static const bool has_next = true;
};

#define __def_integer_iterator__(iterator_name, int_type)\
		template<int_type _Index>\
		using iterator_name = integer_iterator<int_type, _Index>;

__def_integer_iterator__(char_iterator, char)
__def_integer_iterator__(unsigned_char_iterator, unsigned char)
__def_integer_iterator__(short_iterator, short)
__def_integer_iterator__(unsigned_short_iterator, unsigned short)
__def_integer_iterator__(int_iterator, int)
__def_integer_iterator__(unsigned_int_iterator, unsigned int)
__def_integer_iterator__(long_iterator, long)
__def_integer_iterator__(unsigned_long_iterator, unsigned long)
__def_integer_iterator__(long_long_iterator, long long)
__def_integer_iterator__(unsigned_long_long_iterator, unsigned long long)
__def_integer_iterator__(size_t_iterator, size_t)

#undef __def_integer_iterator__

template<typename ..._Params>
struct _sizeof
{
	static const size_t value = sizeof...(_Params);

	typedef _int<value> type;
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
struct decl: __decl_impl_base, __decl_impl_base::__decl_impl<_T>
{
};

// 函数类型的特化，函数类型是不可被声明的，只有函数指针可以声明，这里将函数类型转换为函数指针
template<typename _RetType, typename ... _ArgTypes>
struct decl<_RetType(_ArgTypes...)> : __decl_impl_base, __decl_impl_base::__decl_impl<_RetType(_ArgTypes...)>
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
	 * @brief 先实例化两个模板参数，再选择分支。
	 * 		  判断_Cond是否为true，如果是则type类型为_TrueType，否则为_FalseType
	 */
	template<typename _TrueType, typename _FalseType>
	struct resolve_t
	{
		typedef _TrueType type;
	};

	/**
	 * @brief 先选择分支，再实例化选中分支的模板参数。
	 * 		  两个参数的类型必须是lazy_t<>
	 */
	template<typename _TrueLazyT, typename _FalseLazyT>
	struct lazy_resolve_t
	{
		__assert_not_impl__(_TrueLazyT);
		__assert_not_impl__(_FalseLazyT);
	};

	template<typename _TrueTypeHolder, typename _FalseTypeHolder>
	struct lazy_resolve_t<lazy_t<_TrueTypeHolder>, lazy_t<_FalseTypeHolder> >
	{
		typedef typename _TrueTypeHolder::type::type type;
	};

	template<typename _T>
	struct enable
	{
		typedef _T type;
	};

	template<typename _TrueReturn, typename _FalseReturn>
	__attribute__((always_inline)) inline static constexpr _TrueReturn&& _return(_TrueReturn&& true_val, _FalseReturn&& false_val) noexcept
	{
		return true_val;
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

	template<typename _TrueLazyT, typename _FalseLazyT>
	struct lazy_resolve_t
	{
		__assert_not_impl__(_TrueLazyT);
		__assert_not_impl__(_FalseLazyT);
	};

	template<typename _TrueTypeHolder, typename _FalseTypeHolder>
	struct lazy_resolve_t<lazy_t<_TrueTypeHolder>, lazy_t<_FalseTypeHolder> >
	{
		typedef typename _FalseTypeHolder::type::type type;
	};

	template<typename _T>
	struct enable
	{
	};

	template<typename _TrueReturn, typename _FalseReturn>
	__attribute__((always_inline)) inline static constexpr _FalseReturn&& _return(_TrueReturn&& true_val, _FalseReturn&& false_val) noexcept
	{
		return false_val;
	}
};

/**
 * @brief 类型的分类，必定是以下之一
 */
enum classify_type : int
{
	VARIABLE, //普通变量
	FUNCTION, //普通函数
	ORID_NUM,
	MEMB_FIELD = ORID_NUM, //成员字段
	MEMB_FUNCTION, //成员函数
};

using classify_type_variable_t = __constexpr__(classify_type::VARIABLE);
using classify_type_function_t = __constexpr__(classify_type::FUNCTION);
using classify_type_memb_field_t = __constexpr__(classify_type::MEMB_FIELD);
using classify_type_memb_function_t = __constexpr__(classify_type::MEMB_FUNCTION);

/**
 * @brief 值的种类判断
 */
template<typename _T>
struct classify_type_of_t
{
	static const classify_type value = classify_type::VARIABLE;
};

template<typename _RetType, typename ... _ArgTypes>
struct classify_type_of_t<_RetType(_ArgTypes...)>
{
	static const classify_type value = classify_type::FUNCTION;
};

template<typename _RetType, typename ... _ArgTypes>
struct classify_type_of_t<_RetType (*)(_ArgTypes...)>
{
	static const classify_type value = classify_type::FUNCTION;
};

template<typename _Class, typename _T>
struct classify_type_of_t<_T _Class::*>
{
	static const classify_type value = classify_type::MEMB_FIELD;
};

template<typename _Class, typename _RetType, typename ... _ArgTypes>
struct classify_type_of_t<_RetType (_Class::*)(_ArgTypes...)>
{
	static const classify_type value = classify_type::MEMB_FUNCTION;
};

template<typename _T>
inline constexpr classify_type classify_type_of(_T*)
{
	return classify_type::VARIABLE;
}

template<typename _RetType, typename ... _ArgTypes>
inline constexpr classify_type classify_type_of(_RetType (*)(_ArgTypes...))
{
	return classify_type::FUNCTION;
}

template<typename _Class, typename _T>
inline constexpr classify_type classify_type_of(_T _Class::*)
{
	return classify_type::MEMB_FIELD;
}

template<typename _Class, typename _RetType, typename ... _ArgTypes>
inline constexpr classify_type classify_type_of(_RetType (_Class::*)(_ArgTypes...))
{
	return classify_type::MEMB_FUNCTION;
}

inline constexpr classify_type to_orid_classification(classify_type classification)
{
	return classification > classify_type::ORID_NUM ? (classify_type)(classification - classify_type::ORID_NUM) : classification;
}

inline constexpr classify_type to_memb_classification(classify_type classification)
{
	return classification < classify_type::ORID_NUM ? (classify_type)(classification + classify_type::ORID_NUM) : classification;
}

inline constexpr bool is_memb_classification(classify_type classification)
{
	return classification == classify_type::MEMB_FIELD || classification == classify_type::MEMB_FUNCTION;
}

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

/**
 * @brief 可以得到指针、引用等的原本类型，即传入T*或T**甚至更高次数的指针可以得到T
 */
template<typename _T>
struct decay_type
{
	typedef _T type;
};

template<typename _T>
struct decay_type<const _T>
{
	typedef typename decay_type<_T>::type type;
};

template<typename _T>
struct decay_type<_T*>
{
	typedef typename decay_type<_T>::type type; //递归地获取指针类型
};

template<typename _T>
struct decay_type<_T&>
{
	typedef typename decay_type<_T>::type type;
};

template<typename _T>
struct decay_type<const _T&>
{
	typedef typename decay_type<_T>::type type;
};

template<typename _T>
struct decay_type<_T&&>
{
	typedef typename decay_type<_T>::type type;
};

/**
 * @brief 去除引用
 */
template<typename _T>
struct no_ref
{
	typedef _T type;
};

// 左值引用
template<typename _T>
struct no_ref<_T&>
{
	typedef _T type;
};

// 右值引用
template<typename _T>
struct no_ref<_T&&>
{
	typedef _T type;
};

/**
 * @brief 添加const修饰符
 */
template<typename _T>
struct _const
{
	typedef const _T type;
};

template<typename _T>
struct _const<const _T>
{
	typedef _T type;
};

/**
 * @brief 完美转发
 * 模板参数必须显示指定，因参数是依赖_T的类型，无法根据参数反推_T
 */
//非const左值引用不能绑定右值，无法匹配本函数
template<typename _T>
__attribute__((always_inline)) inline constexpr _T&& forward(typename no_ref<_T>::type& v) noexcept
{
	return (_T&&)v;
}

//右值重载
template<typename _T>
__attribute__((always_inline)) inline constexpr _T&& forward(typename no_ref<_T>::type&& v) noexcept
{
	return (_T&&)v;
}

/**
 * @brief 储存类型
 */
template<typename _T>
struct store_type
{
	typedef _T type;
};

//左值保持引用
template<typename _T>
struct store_type<_T&>
{
	typedef _T& type;
};

//右值直接储存值类型而非引用
template<typename _T>
struct store_type<_T&&>
{
	typedef _T type;
};

/**
 * @brief 包装指针的类型，_Class=void则视作非成员类型，否则视作成员类型
 */
template<typename _Class, typename _T>
struct ptr_type
{
	static const classify_type classification = to_memb_classification(classify_type_of_t<_T>::value); //_Class不为void则转换为对应的成员类型

	typedef _T _Class::*type;
};

template<typename _T>
struct ptr_type<void, _T>
{
	static const classify_type classification = to_orid_classification(classify_type_of_t<_T>::value);

	typedef _T* type;
};

template<typename _Constexpr1, typename _Constexpr2>
struct max
{
	__assert_not_impl__(_Constexpr1);
	__assert_not_impl__(_Constexpr2);
};

template<typename _T1, _T1 _N1, typename _T2, _T2 _N2>
struct max<_constexpr<_T1, _N1>, _constexpr<_T2, _N2> >
{
	static constexpr typename if_else<(_N1 > _N2)>::resolve_t<_T1, _T2>::type value = if_else<(_N1 > _N2)>::_return(_N1, _N2);
};

template<typename _Constexpr1, typename _Constexpr2>
struct min
{
	__assert_not_impl__(_Constexpr1);
	__assert_not_impl__(_Constexpr2);
};

template<typename _T1, _T1 _N1, typename _T2, _T2 _N2>
struct min<_constexpr<_T1, _N1>, _constexpr<_T2, _N2> >
{
	static constexpr typename if_else<(_N1 < _N2)>::resolve_t<_T1, _T2>::type value = if_else<(_N1 < _N2)>::_return(_N1, _N2);
};

/**
 * @brief 无效类型
 */
struct invalid_type;

constexpr size_t invalid_index = -1;

// ***** 类型参数包 *****

inline static constexpr bool is_index_valid(size_t idx, size_t length)
{
	return idx != invalid_index && idx < length;
}

template<size_t _Index, typename ... _Params>
struct type_at;

struct __type_at_impl_base //避免外部模板类的不同实例都实例化相同的__type_at_impl<>模板，又限制外部访问
{
protected:
	struct __type_at_impl_oob //索引越界
	{
		typedef invalid_type type;
	};

	template<size_t _Index, typename ... _Params>
	struct __type_at_impl
	{
	};

	template<size_t _Index, typename _First, typename ... _RestParams>
	struct __type_at_impl<_Index, _First, _RestParams...>
	{
		typedef typename if_else<_Index <= 0>::resolve_t<type_t<_First>, type_at<_Index - 1, _RestParams...> //此处不使用type_at<>::type避免实例化type_at<>
		>::type::type type;
	};

	template<size_t _Index>
	struct __type_at_impl<_Index> : __type_at_impl_oob
	{
	};
};

/**
 * @brief 获取指定索引的参数类型，索引从0开始
 * 		  如果_Params为空则将断言失败，索引越界。
 */
template<size_t _Index, typename ... _Params>
struct type_at: __type_at_impl_base, public if_else<is_index_valid(_Index, sizeof...(_Params))>::resolve_t<__type_at_impl_base:: __type_at_impl <_Index, _Params...>, __type_at_impl_base:: __type_at_impl_oob >::type
{
};

/**
 * @brief 类型参数包迭代器
 */
template<size_t _Index, typename ..._Params>
struct iterator;

struct __iterator_impl_base
{
protected:
	// 超出索引且不是end迭代器的无效迭代器
	struct __iterator_impl_oob
	{
		static const size_t index = invalid_index;

		typedef invalid_type type;

		typedef __iterator_impl_oob prev;
		typedef __iterator_impl_oob next; //下一个还是本类

		static const bool has_prev = false;
		static const bool has_next = false;
	};

	template<size_t _Index, typename ..._Params>
	struct __iterator_impl
	{
		static const size_t index = _Index;

		__assert_pack_index__(index, _Params); //检查边界

		typedef typename type_at<index, _Params...>::type type;

		typedef iterator<index - 1, _Params...> prev; //上一个迭代器
		typedef iterator<index + 1, _Params...> next; //下一个迭代器

		static const bool has_prev = true;
		static const bool has_next = true;
	};

//正向迭代最后一个元素之后的下一个迭代器，即end迭代器
	template<typename ..._Params>
	struct __iterator_impl<sizeof...(_Params), _Params...>
	{
		static const size_t index = sizeof...(_Params);

		typedef invalid_type type;

		typedef iterator<index - 1, _Params...> prev; //上一个迭代器是最后一个元素
		typedef __iterator_impl_oob next;

		static const bool has_prev = sizeof...(_Params) == 0 ? false : true;
		static const bool has_next = false;
	};

//逆向迭代时，参数包第一个元素之前的下一个迭代器，即rend迭代器
	template<typename ..._Params>
	struct __iterator_impl<invalid_index, _Params...>
	{
		static const size_t index = invalid_index;

		typedef invalid_type type;

		typedef __iterator_impl_oob prev;
		typedef iterator<index + 1, _Params...> next;

		static const bool has_prev = false;
		static const bool has_next = sizeof...(_Params) == 0 ? false : true;
	};
};

/**
 * @brief 参数包类型迭代器。
 * 		  正向迭代时需要使用next进行迭代，逆向迭代时需要使用prev进行迭代
 */
template<size_t _Index, typename ..._Params>
struct iterator: __iterator_impl_base, public if_else<(_Index >= -1 && _Index <= sizeof...(_Params))>::resolve_t<__iterator_impl_base:: __iterator_impl <_Index, _Params...>, __iterator_impl_base:: __iterator_impl_oob >::type
{
};

//正向迭代起始迭代器
template<typename ..._Params>
using begin_iterator = iterator<0, _Params...>;

//正向迭代终止迭代器
template<typename ..._Params>
using end_iterator = iterator<sizeof...(_Params), _Params...>;

//逆向迭代起始迭代器
template<typename ..._Params>
using rbegin_iterator = iterator<sizeof...(_Params) - 1, _Params...>;

//逆向迭代终止迭代器
template<typename ..._Params>
using rend_iterator = iterator<invalid_index, _Params...>;

//***** 迭代算法 *****

template<typename _BeginIter, typename _EndIter, template<typename, typename, typename ...> typename _Op,
template<typename, typename, typename ...> typename _Cond,
typename _Result,
typename ..._OpParams>
struct forward_iterate;

template<typename _BeginIter, typename _EndIter, template<typename, typename, typename ...> typename _Op,
template<typename, typename, typename ...> typename _Cond,
typename _Result,
typename ..._OpParams>
struct inverse_iterate;

struct __iterate_impl_base
{
protected:
	template<typename _Result>
	struct __iterate_impl_end
	{
		typedef _Result type; //迭代未开始或提前结束时的最终结果
	};

	template<typename _CurrentIter, typename _EndIter, template<typename, typename, typename ...> typename _Op,  //_Op是模板，模板参数中声明模板实现类型的Callable
	template<typename, typename, typename ...> typename _Cond,//迭代进行的条件
	typename _Result,
	typename ..._OpParams
	>
	struct __forward_iterate_impl
	{
		typedef typename forward_iterate<
		typename _CurrentIter::next,
		_EndIter,
		_Op,
		_Cond,
		typename _Op<_CurrentIter, _Result, _OpParams...>::type, //每次迭代时都将本次_Op处理_CurrentIter得到的结果传入下一次迭代
		_OpParams...
		>::type type;
	};

	template<
	typename _CurrentIter,
	typename _EndIter,
	template<typename, typename, typename ...> typename _Op,
	template<typename, typename, typename ...> typename _Cond,
	typename _Result,
	typename ..._OpParams
	>
	struct __inverse_iterate_impl
	{
		typedef typename inverse_iterate<
		typename _CurrentIter::prev,
		_EndIter,
		_Op,
		_Cond,
		typename _Op<_CurrentIter, _Result, _OpParams...>::type,
		_OpParams...
		>::type type;
	};
};

/**
 * @brief 正向迭代，迭代包含_BeginIter但不包含_EndIter。
 * 		  参数_Op、_Cond必须是模板，其中：
 * 		  接收的第一个类型参数必须是当前迭代器，接收的第二个参数必须是上一次（或初始）的结果，在此之后可以有任意多个其他参数，这些参数在迭代过程中保持不变。
 * 		  _Op必须定义'type'作为当前迭代结果；
 * 		  _Cond必须定义static const bool iterate_next;用于决定是否迭代下一个元素，如果为false则终止迭代。
 * 		  如果在一次迭代中要返回多个值，则需要将多个值打包到_Result中。
 */
template<typename _BeginIter, typename _EndIter, template<typename, typename, typename ...> typename _Op,
template<typename, typename, typename ...> typename _Cond,
typename _Result,
typename ..._OpParams>
struct forward_iterate: __iterate_impl_base, public if_else<(_BeginIter::index < _EndIter::index)>
::resolve_t<
typename if_else<_Cond<_BeginIter, _Result, _OpParams...>::iterate_next> //当前迭代器能进行下一次迭代且下一个迭代器不是end迭代器
::resolve_t<
__iterate_impl_base::__forward_iterate_impl<_BeginIter, _EndIter, _Op, _Cond, _Result, _OpParams...>,
//如果使用_Op<_BeginIter, _Result, _OpParams...>::type则会引发该_Op<>的初始化，一旦_BeginIter不是有效的元素迭代器就会导致_Op访问无效元素
//因此此处使用lazy_tpl<>且不直接访问lazy_tpl<>::type，进行延迟实例化，防止_Op<>直接实例化
lazy_tpl<_Op, _BeginIter, _Result, _OpParams...>
>::type,//将当前结果作为迭代结果并终止迭代
__iterate_impl_base::__iterate_impl_end<_Result>
>::type
{
};

/**
 * @brief 逆向迭代
 */
template<typename _BeginIter, typename _EndIter, template<typename, typename, typename ...> typename _Op,
template<typename, typename, typename ...> typename _Cond,
typename _Result,
typename ..._OpParams>
struct inverse_iterate: __iterate_impl_base, public if_else<(_BeginIter::index > _EndIter::index)>
::resolve_t<
typename if_else<_Cond<_BeginIter, _Result, _OpParams...>::iterate_next>
::resolve_t<
__iterate_impl_base::__inverse_iterate_impl<_BeginIter, _EndIter, _Op, _Cond, _Result, _OpParams...>,
lazy_tpl<_Op, _BeginIter, _Result, _OpParams...>
>::type,
__iterate_impl_base::__iterate_impl_end<_Result>
>::type
{
};

namespace iterate_cond
{
/**
 * @brief 始终迭代下一个
 */
template<typename _CurrentIter, typename _Result, typename ...>
struct always
{
	static const bool iterate_next = true;
};

/**
 * @brief 始终不迭代下一个
 */
template<typename _CurrentIter, typename _Result, typename ...>
struct never
{
	static const bool iterate_next = false;
};

/**
 * @brief 查找直到计数为0
 */
template<size_t _Counter>
struct until_appear
{
	template<size_t _CurrentIndex>
	struct appear_counter
	{
		static const size_t value = _Counter;
		static const size_t index = _CurrentIndex;

		typedef appear_counter<_CurrentIndex> type;
	};

	// 迭代起始的计数器
	using begin_appear_counter = appear_counter<invalid_index>;
};

template<typename _CurrentIter, typename _AppearCounter, typename _T>
struct until_appear_cond
{
	static const bool iterate_next = (_AppearCounter::value > 0) || (_AppearCounter::value == 0 && !type_equal<typename _CurrentIter::type, _T>::value); //计数器>0或等于0但类型不相同则继续迭代下一个元素
};

/**
 * @brief 迭代中更新计数
 */
template<typename _CurrentIter, typename _AppearCounter, typename _T>
struct until_appear_op
{
	static const bool equal = type_equal<typename _CurrentIter::type, _T>::value; //类内初始化为编译期内联常量，不会进入编译后的二进制文件

	typedef typename if_else<(_AppearCounter::value > 0)>::resolve_t<typename until_appear<equal ? _AppearCounter::value - 1 : _AppearCounter::value>::appear_counter<_AppearCounter::index>, //计数不为0则减少计数，保持索引不变
			typename until_appear<_AppearCounter::value>::appear_counter<equal ? _CurrentIter::index : _AppearCounter::index> //计数器为0，判断当前类型是否与给定目标类型相同，相同则返回索引，不同则索引不变
	>::type type;
};

/**
 * @brief 迭代中收集当前类型到目标type_pack<>末尾
 */
template<typename _CurrentIter, typename _CollectedPack>
struct append_iter_type_op
{
	typedef typename _CollectedPack::append<typename _CurrentIter::type>::type type; //在当前收集的类型的末尾添加当前迭代器的类型
};

/**
 * @brief 迭代中收集_Op的不变参数类型到目标type_pack<>末尾
 */
template<typename _CurrentIter, typename _CollectedPack, typename ...OpParams>
struct append_param_type_op
{
	typedef typename _CollectedPack::append<OpParams...>::type type; //在当前收集的类型的末尾添加当前迭代器的类型
};

/**
 * @brief 迭代中收集迭代器的索引_constexpr<>类型到目标type_pack<>末尾
 */
template<typename _CurrentIter, typename _CollectedPack>
struct append_iter_index_op
{
	typedef typename _CollectedPack::append<typename _CurrentIter::type>::type type;
};

}

template<typename _T, size_t _Num>
struct pack_of_t;

/**
 * @brief 类型参数包，用于保存模板参数而不展开，对于值参数，需要使用_constexpr<>将值封装为类型
 */
template<typename ..._Params>
struct type_pack
{
	/**
	 * @brief 参数包长度
	 */
	static const size_t size = sizeof...(_Params);

	typedef type_pack<_Params...> type;

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
	 * @brief 在类型列表最前方添加新类型
	 */
	template<typename ... _PrependParames>
	struct prepend‌
	{
		typedef type_pack<_PrependParames..., _Params...> type;
	};

	template<typename _PrependPack>
	struct prepend‌_pack
	{
		__assert_not_impl__(_PrependPack);
	};

	/**
	 * @brief 在开头添加单个参数包
	 */
	template<typename ... _PrependParames>
	struct prepend‌_pack<type_pack<_PrependParames...> >
	{
		typedef typename type::prepend‌<_PrependParames...>::type type;
	};

	/**
	 * @brief 在类型列表最后方添加新类型
	 */
	template<typename ... _AppendParames>
	struct append
	{
		typedef type_pack<_Params..., _AppendParames...> type;
	};

	template<typename _AppendPack>
	struct append_pack
	{
		__assert_not_impl__(_AppendPack);
	};

	/**
	 * @brief 在末尾添加单个参数包
	 */
	template<typename ... _AppendParames>
	struct append_pack<type_pack<_AppendParames...> >
	{
		typedef typename type::append<_AppendParames...>::type type;
	};

	/**
	 * @brief 指定索引的类型
	 */
	template<size_t _Index>
	struct at
	{
		typedef typename type_at<_Index, _Params...>::type type;
	};

	typedef typename at<0>::type front;
	typedef typename at<size - 1>::type back;

	/**
	 * @brief 指定索引的迭代器
	 */
	template<size_t _Index>
	using iterator_at = iterator<_Index, _Params...>;

	using begin = begin_iterator<_Params...>; //正向第一个元素的迭代器
	using end = end_iterator<_Params...>; //正向迭代递归终止，必须通过对迭代器参数类型偏特化为end来终止递归，否则将无限递归直到报错

	using rbegin = rbegin_iterator<_Params...>;
	using rend = rend_iterator<_Params...>;

	/**
	 * @brief 获取索引为[_BeginIndex, _EndIndex)的类型中指定类型正向第_Count次出现时的索引
	 */
	template<typename _T, size_t _BeginIndex, size_t _EndIndex, size_t _Count>
	struct find_in
	{
		static const size_t value = forward_iterate<iterator_at<_BeginIndex>, iterator_at<_EndIndex>, iterate_cond::until_appear_op, iterate_cond::until_appear_cond, typename iterate_cond::until_appear<_Count - 1>::begin_appear_counter, _T>::type::index;
	};

	template<typename _T, size_t _Count>
	using find = find_in<_T, begin::index, end::index, _Count>;

	template<typename _T>
	using find_first = find<_T, 1>;

	/**
	 * @brief 获取索引为[_BeginIndex, _EndIndex)的类型中指定类型逆向第_Count次出现时的索引
	 */
	template<typename _T, size_t _BeginIndex, size_t _EndIndex, size_t _Count>
	struct rfind_in
	{
		static const size_t value = inverse_iterate<iterator_at<_BeginIndex>, iterator_at<_EndIndex>, iterate_cond::until_appear_op, iterate_cond::until_appear_cond, typename iterate_cond::until_appear<_Count - 1>::begin_appear_counter, _T>::type::index;
	};

	template<typename _T, size_t _Count>
	using rfind = rfind_in<_T, rbegin::index, rend::index, _Count>;

	template<typename _T>
	using rfind_first = rfind<_T, 1>;

	template<typename _T>
	using find_last = rfind_first<_T>;

	template<typename _T>
	using rfind_last = find_first<_T>;

	/**
	 * @brief 提取索引为[_BeginIndex, _EndIndex)的数组切片
	 */
	template<size_t _BeginIndex, size_t _EndIndex>
	struct slice
	{
		typedef typename forward_iterate<iterator_at<_BeginIndex>, iterator_at<_EndIndex>, iterate_cond::append_iter_type_op, iterate_cond::always, type_pack<> >::type type;
	};

	/**
	 * @brief 所有类型反序
	 */
	struct inverse
	{
		typedef typename inverse_iterate<rbegin, rend, iterate_cond::append_iter_type_op, iterate_cond::always, type_pack<> >::type type;
	};

	/**
	 * @brief 左侧_Num个类型
	 */
	template<size_t _Num>
	using left = slice<begin::index, _Num>;

	/**
	 * @brief 右侧_Num个类型
	 */
	template<size_t _Num>
	using right = slice<size - _Num, end::index>;

	/**
	 * @brief 在_InsertIndex处插入新类型
	 */
	template<size_t _InsertIndex, typename ... _InsertParames>
	struct insert
	{
		typedef typename left<_InsertIndex>::type::append<_InsertParames...>::type::append_pack<typename type::slice<_InsertIndex, end::index>::type>::type type;
	};

	/**
	 * @brief 移除索引为[_BeginIndex, _EndIndex)的元素
	 */
	template<size_t _BeginIndex, size_t _EndIndex>
	struct erase
	{
		typedef typename left<_BeginIndex>::type::append_pack<typename type::slice<_EndIndex, end::index>::type>::type type;
	};

	template<size_t _Index>
	using erase_at = erase<_Index, _Index + 1>;

	using erase_first = erase_at<0>;

	using erase_last = erase_at<size - 1>;

	/**
	 * @brief 设置指定索引的类型
	 */
	template<size_t _Index, typename _T>
	struct put
	{
		typedef typename left<_Index>::type::append<_T>::type::append_pack<typename type::slice<_Index + 1, end::index>::type>::type type;
	};

	/**
	 * @brief 调整type_pack<>的大小，如果目标_Size小于当前则只保留前_Size个元素，如果大于当前则后面扩容的部分用_FillType进行填充
	 */
	template<size_t _Size, typename _FillType>
	struct resize
	{
		typedef typename if_else<(_Size == size)>::resolve_t<type, //size无变化直接返回本身
				typename if_else<(_Size > size)>::resolve_t<typename append_pack<typename pack_of_t<_FillType, _Size - size>::type>::type, typename left<_Size>::type>::type>::type type;
	};
};

/**
 * @brief 构造一个长度为_Num且全部元素为_T的type_pack<>
 */
template<typename _T, size_t _Num>
struct pack_of_t
{
	typedef typename forward_iterate<size_t_iterator<0>, size_t_iterator<_Num>, iterate_cond::append_param_type_op, iterate_cond::always, type_pack<>, _T>::type type;
};

/**
 * @brief 多个type_pack<>拼接，参数必须全是type_pack<>否则断言失败。
 */
template<typename ... _CatPacks>
struct cat_pack
{
private:
	template<typename _CurrentIter, typename _CatPack>
	struct __cat_pack_op
	{
		typedef typename _CatPack::append_pack<typename _CurrentIter::type>::type type;
	};

public:
	typedef type_pack<_CatPacks...> packs;

	typedef typename forward_iterate<typename packs::begin, typename packs::end, __cat_pack_op, iterate_cond::always, type_pack<> >::type type;
};

/**
 * 连续的索引数列生成
 */
template<typename _IntType, _IntType _Start, _IntType _Num>
struct index_sequence
{
	typedef typename forward_iterate<integer_iterator<_IntType, _Start>, integer_iterator<_IntType, _Start + _Num>, iterate_cond::append_iter_index_op, iterate_cond::always, type_pack<> >::type type;
};

template<typename _Value>
struct _switch
{
	template<typename ... _Cases>
	struct _case
	{
		typedef type_pack<_Cases...> cases;

		static const size_t value = cases::template find_first<_Value>::value;

		template<typename _Default, typename ... _Results>
		struct resolve_t
		{
			__assert_pack_size_equal__(_Cases, _Results);

			typedef typename if_else<is_index_valid(value, cases::size)>::resolve_t<typename type_at<value, _Results...>::type, _Default>::type type;
		};
	};
};

}

#endif //_TPLMP_TPLMP