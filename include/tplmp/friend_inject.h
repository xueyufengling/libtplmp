#ifndef _TPLMP_FRIENDINJECT
#define _TPLMP_FRIENDINJECT

#include <ppmp/base.h>
#include <ppmp/linguistic.h>

// expand_id统一使用0
#if !defined(__tplmp_friend_inject_expand_id__)
#define __tplmp_friend_inject_expand_id__() 0
#endif

#define __decl_friend_inject_tag_dispatch__(tag, ret_type, func_name, ...)\
	__entity_val__(ret_type) func_name(::tplmp::type_t<__entity_val__(tag)> __va_opt_comma__(__VA_ARGS__) __declaration_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__))

/**
 * @brief 编译期通过tag进行ADL查找的函数
 */
#define __def_friend_inject_tag_dispatch__(tag, ret_type, func_name, ...)\
	friend __decl_friend_inject_tag_dispatch__(tag, ret_type, func_name, __VA_ARGS__)

#define __def_friend_inject_tag_dispatch_wrapper_intl__(is_constexpr, tag, ret_type, func_name, opt_comma, decl_list, decl_name_list)\
	template<typename _Tag>\
	inline __if__(is_constexpr)(constexpr) auto func_name(decl_list)\
		noexcept(noexcept(func_name(::tplmp::type_t<_Tag>() opt_comma decl_name_list)))\
		-> decltype(\
			func_name(::tplmp::type_t<_Tag>() opt_comma decl_name_list)\
		);\
	template<>\
	inline __if__(is_constexpr)(constexpr) auto func_name<__entity_val__(tag)>(decl_list)\
		noexcept(noexcept(func_name(::tplmp::type_t<__entity_val__(tag)>() opt_comma decl_name_list)))\
		-> decltype(\
			func_name(::tplmp::type_t<__entity_val__(tag)>() opt_comma decl_name_list)\
		)\
	{\
		return func_name(::tplmp::type_t<__entity_val__(tag)>() opt_comma decl_name_list);\
	}

#define __def_friend_inject_tag_dispatch_wrapper__(is_constexpr, tag, ret_type, func_name, ...)\
	__def_friend_inject_tag_dispatch_wrapper_intl__(is_constexpr, tag, ret_type, func_name, __va_opt_comma__(__VA_ARGS__),\
		__declaration_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__),\
		__declaration_name_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__)\
	)

#endif//_TPLMP_FRIENDINJECT
