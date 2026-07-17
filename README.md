[中文](README_zh.md)

# libtplmp

    libtplmp is a header-only library that provides a basic implementation for C++ template meta programming. Standard not lower than C++11 is required.<br>
    In addition, some other template-based techniques are also included in this library.<br>
    This library depends on [libtplmp](https://github.com/xueyufengling/libtplmp/tree/main) and [libppmp](https://github.com/xueyufengling/libppmp/tree/main). Since preprocessor metaprogramming relies on a large number of predefined macros, an excessive number of macros will significantly slow down compilation. To improve compilation speed, the parameters for automatically generating macros need to be adjusted according to the project's requirements when installing libppmp.<br>

# License

    libtplmp is distributed under the LGPL-3.0 with Linking Exception. This license removes the obligation to provide Minimal Corresponding Source when statically linking, which is otherwise required by the original LGPL-3.0 license. In short, as long as you do not modify the source code of this library, you are not required to open-source your code, regardless of whether you choose static or dynamic linking.<br>