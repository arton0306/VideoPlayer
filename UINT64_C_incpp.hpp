#ifndef UINT64_C_INCPP_HPP
#define UINT64_C_INCPP_HPP

/*
libav use type UINT64_C
it can not compile with cplusplus compiler directly
reference: http://blog.csdn.net/cjsafty/article/details/7041518
*/

extern "C"{
#ifdef __cplusplus
    #define __STDC_CONSTANT_MACROS
    #ifdef _STDINT_H
        #undef _STDINT_H
    #endif
    #include <stdint.h>
#endif
}

#endif // UINT64_C_INCPP_HPP
