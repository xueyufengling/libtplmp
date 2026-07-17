#ifndef _TPLMP_FRIENDINJECT
#define _TPLMP_FRIENDINJECT

#include <ppmp/base.h>
#include <ppmp/linguistic.h>

// expand_id统一使用0
#define __tplmp_friend_inject_expand_id__() 0

/**
 * @brief 编译期通过tag进行ADL查找的函数
 */
#define __def_friend_inject_constexpr_tag_dispatch__(tag, ret_type, func_name, ...)\
	friend inline constexpr __entity_val__(ret_type) func_name(::tplmp::type_t<__entity_val__(tag)> __va_opt_comma__(__VA_ARGS__) __declaration_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__))

#define __decl_friend_inject_constexpr_tag_dispatch__(tag, ret_type, func_name, ...)\
	inline constexpr __entity_val__(ret_type) func_name(::tplmp::type_t<__entity_val__(tag)> __va_opt_comma__(__VA_ARGS__) __declaration_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__));\
	template<typename _Tag>\
	inline constexpr auto func_name(__declaration_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__)) -> decltype(\
			func_name(::tplmp::type_t<_Tag>()  __va_opt_comma__(__VA_ARGS__) __declaration_name_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__))\
	);\
	template<>\
	inline constexpr auto func_name<__entity_val__(tag)>(__declaration_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__)) -> decltype(\
			func_name(::tplmp::type_t<__entity_val__(tag)>() __va_opt_comma__(__VA_ARGS__) __declaration_name_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__))\
	)\
	{\
		return func_name(::tplmp::type_t<__entity_val__(tag)>() __va_opt_comma__(__VA_ARGS__) __declaration_name_list__(__tplmp_friend_inject_expand_id__(), __VA_ARGS__));\
	}

#endif//_TPLMP_FRIENDINJECT
