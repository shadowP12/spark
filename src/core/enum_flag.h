#pragma once

#ifdef __cplusplus
#ifndef SP_MAKE_ENUM_FLAG
#define SP_MAKE_ENUM_FLAG(TYPE, ENUM_TYPE)                          \
    static inline ENUM_TYPE operator|(ENUM_TYPE a, ENUM_TYPE b)     \
    {                                                               \
        return (ENUM_TYPE)((TYPE)(a) | (TYPE)(b));                  \
    }                                                               \
    static inline ENUM_TYPE operator&(ENUM_TYPE a, ENUM_TYPE b)     \
    {                                                               \
        return (ENUM_TYPE)((TYPE)(a) & (TYPE)(b));                  \
    }                                                               \
    static inline ENUM_TYPE operator|=(ENUM_TYPE& a, ENUM_TYPE b)   \
    {                                                               \
        return a = (a | b);                                         \
    }                                                               \
    static inline ENUM_TYPE operator&=(ENUM_TYPE& a, ENUM_TYPE b)   \
    {                                                               \
        return a = (a & b);                                         \
    }
#endif
#else
#define SP_MAKE_ENUM_FLAG(TYPE, ENUM_TYPE)
#endif