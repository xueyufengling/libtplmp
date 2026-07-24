#ifndef _TPLMP_EXPR
#define _TPLMP_EXPR

#include "base.h"
#include "tuple.h"
#include "type_check.h"

namespace tplmp
{
/**
 * @brief _T类型的表达式模板，用于延迟计算、优化计算过程中的临时结果对象。
 * 		  1. 延迟计算
 * 		  表达式模板仅记录运算的步骤到模板参数中，运算的操作数引用记录在tuple中，不进行实际的运算操作。
 * 		  重写延迟求值目标类型_T的operator=()，并实现运算模板的计算函数，在operator=()中调用计算函数来进行实际的计算求值和赋值给目标。
 * 		  2. 中间值临时对象优化
 * 		  对向量而言，原本vector v = a + b + c;会先计算a+b的结果向量并储存到临时对象x，再由x+c计算得到v。
 * 		  使用表达式模板后，可以将运算变为v[i] = (a[i] + b[i]) + c[i]，从而优化掉向量临时对象。
 * 		  因此，表达式模板只能优化掉向量计算产生的临时对象，标量临时对象依然存在。在纯标量运算中只能延迟求值，无临时对象优化。
 * @param _T 表达式的值类型
 * @param _Op 表达式的最外层操作类型，必须有static _T eval(...)函数用于求值
 * @param _OperandTypes 表达式的操作数，必须是表达式模板
 */
template<typename _T, typename _Op = _T, typename ..._OperandTypes>
struct expr
{
	typedef expr<_T, _Op, _OperandTypes...> type;

	/**
	 * _Op操作所需的操作数
	 */
	tuple<_OperandTypes...> operands;

	inline expr(_OperandTypes&& ... ops) noexcept(noexcept(tuple<_OperandTypes...>(forward<_OperandTypes>(ops) ...))) :
			operands(forward<_OperandTypes>(ops) ...)
	{
	}

protected:
	template<size_t ..._OperandIndexes>
	inline constexpr _T __value_impl(type_pack<_size_t<_OperandIndexes> ...>)
			noexcept(noexcept(_Op::eval(((_T)operands.template at<_OperandIndexes>().value()) ...)))
	{
		return _Op::eval(((_T)operands.template at<_OperandIndexes>().value()) ...);
	}

public:
	inline constexpr expr<_T> value() noexcept
	{
		return __value_impl(index_sequence_t<size_t, 0, sizeof...(_OperandTypes)>());
	}
};

template<typename _T>
struct expr<_T, _T>
{
	typedef _T type;

	_T operand;

	inline expr(_T op) noexcept(noexcept(_T(op))) :
			operand(op)
	{
	}

	/**
	 * 转换为目标值类型。
	 * 仅在表达式操作类型_Op与_T相同时才能直接转换。
	 * 对于无操作数的操作类型，例如取反、取共轭，其_Op是对应的操作类如_inv、_conj，而非_T
	 */
	inline constexpr operator _T&() noexcept
	{
		return operand;
	}

	inline constexpr operator const _T&() const noexcept
	{
		return operand;
	}

	inline constexpr _T& value() noexcept
	{
		return operand;
	}
};

}

#endif//_TPLMP_EXPR
