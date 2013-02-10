
/*/////////////////////////////////////////////////////////////////////////////
/// @summary Defines some useful global constants, macros and includes.
///////////////////////////////////////////////////////////////////////////80*/
#ifndef CMN_COMMONDEFS_HPP_INCLUDED
#define CMN_COMMONDEFS_HPP_INCLUDED

/*////////////////
//   Includes   //
////////////////*/
#include "commondefs_config.hpp"

#ifdef CMN_HAVE_STDDEF_H
    #include <stddef.h>
#endif

#ifdef CMN_HAVE_STDINT_H
    #include <stdint.h>
#elif _MSC_VER > 1000
    #include "commondefs_stdint.h"
#else
    #error   No C99 stdint.h for your platform!
#endif /* CMN_HAVE_STDINT_H */

#ifdef CMN_HAVE_INTTYPES_H
    #include <inttypes.h>
#elif _MSC_VER > 1000
    #include "commondefs_inttypes.h"
#else
    #error   No C99 inttypes.h for your platform!
#endif /* CMN_HAVE_INTTYPES_H */

/*////////////////////////////
//   Forward Declarations   //
////////////////////////////*/

/*///////////////////////
//   Namespace Begin   //
///////////////////////*/

/*//////////////
//   Macros   //
//////////////*/

/*/////////////////////////////////////////////////////////////////////////80*/

#if defined(WIN32) || defined(WIN64) || defined(_WINDOWS)
    #define CMN_IS_WINDOWS              1
#endif /* defined(WIN32) || defined(WIN64) || defined(_WINDOWS) */

/*/////////////////////////////////////////////////////////////////////////80*/

#if defined(__APPLE__)
    #define CMN_IS_APPLE                1
    #if TARGET_OS_IPHONE
        #define CMN_IS_iOS              1
        #define CMN_IS_iOS_DEV          1
    #elif TARGET_IPHONE_SIMULATOR
        #define CMN_IS_iOS              1
        #define CMN_IS_iOS_SIM          1
    #else
        #define CMN_IS_MACOSX           1
    #endif /* TARGET_OS_IPHONE | TARGET_IPHONE_SIMULATOR */
#endif /* defined(__APPLE__) */

/*/////////////////////////////////////////////////////////////////////////80*/

#if defined(__linux__)
    #define CMN_IS_LINUX                1
#endif /* defined(__linux__) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_CALL_C
    #ifdef  _MSC_VER
        #define CMN_CALL_C              __cdecl
    #else
        #define CMN_CALL_C
    #endif /* _MSC_VER */
#endif /* !defined(CMN_CALL_C) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_IMPORT
    #ifdef _MSC_VER
        #define CMN_IMPORT              __declspec(dllimport)
    #else
        #define CMN_IMPORT
    #endif /* _MSC_VER */
#endif /* !defined(CMN_IMPORT) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_EXPORT
    #ifdef _MSC_VER
        #define CMN_EXPORT              __declspec(dllexport)
    #else
        #define CMN_EXPORT
    #endif /* _MSC_VER */
#endif /* !defined(CMN_EXPORT) */

/*/////////////////////////////////////////////////////////////////////////80*/

#if CMN_SHARED
    #ifdef CMN_EXPORTS
        #define CMN_PUBLIC              CMN_EXPORT
    #else
        #define CMN_PUBLIC              CMN_IMPORT
    #endif /* defined(CMN_EXPORTS) */
#else
    #ifdef CMN_EXPORTS
        #define CMN_PUBLIC
    #else
        #define CMN_PUBLIC
    #endif /* defined(CMN_EXPORTS) */
#endif /* CMN_SHARED */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_FORCE_32BIT
#define CMN_FORCE_32BIT                 0x7FFFFFFF
#endif /* !defined(CMN_FORCE_32BIT) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_MLMACRO_BEGIN
#define CMN_MLMACRO_BEGIN               do {
#endif /* !defined(CMN_MLMACRO_BEGIN) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_MLMACRO_END
    #ifdef _MSC_VER
        #define CMN_MLMACRO_END                                               \
            __pragma(warning(push))                                           \
            __pragma(warning(disable:4127))                                   \
            } while (0)                                                       \
            __pragma(warning(pop))
    #else
        #define CMN_MLMACRO_END                                               \
            } while (0)
    #endif /* defined(_MSC_VER) */
#endif /* !defined(CMN_MLMACRO_END) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_UNUSED_X
    #ifdef _MSC_VER
        #define CMN_UNUSED_X(x)         (x)
    #else
        #define CMN_UNUSED_X(x)         (void)sizeof((x))
    #endif /* _MSC_VER */
#endif /* !defined(CMN_UNUSED_X) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_UNUSED
#define CMN_UNUSED(x)                                                         \
    CMN_MLMACRO_BEGIN                                                         \
    CMN_UNUSED_X(x);                                                          \
    CMN_MLMACRO_END
#endif /* !defined(CMN_UNUSED) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_MIN
#define CMN_MIN(x, y)                   ((x) < (y) ? (x) : (y))
#endif /* !defined(CMN_MIN) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_MAX
#define CMN_MAX(x, y)                   ((x) > (y) ? (x) : (y))
#endif /* !defined(CMN_MAX) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_C_API
#define CMN_C_API(return_type)          extern return_type CMN_CALL_C
#endif /* !defined(CMN_C_API) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_ALIGNMENT_DEFINED
    #ifdef _MSC_VER
        #define CMN_ALIGN_BEGIN(_al)    __declspec(align(_al))
        #define CMN_ALIGN_END(_al)
        #define CMN_ALIGN_OF(_type)     __alignof(_type)
        #define CMN_ALIGNMENT_DEFINED
    #endif /* defined(_MSC_VER) */
    #ifdef __GNUC__
        #define CMN_ALIGN_BEGIN(_al)
        #define CMN_ALIGN_END(_al)      __attribute__((aligned(_al)))
        #define CMN_ALIGN_OF(_type)     __alignof__(_type)
        #define CMN_ALIGNMENT_DEFINED
    #endif /* defined(__GNUC__) */
#endif /* !defined(CMN_ALIGNMENT_DEFINED) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_FORCE_INLINE
    #ifdef _MSC_VER
        #define CMN_FORCE_INLINE        __forceinline
    #endif /* defined(_MSC_VER) */
    #ifdef __GNUC__
        #define CMN_FORCE_INLINE        __attribute__((always_inline))
    #endif /* defined(__GNUC__) */
#endif /* !defined(CMN_FORCE_INLINE) */

/*/////////////////////////////////////////////////////////////////////////80*/

#ifndef CMN_RESTRICT
    #ifdef _MSC_VER
        #define CMN_RESTRICT            __restrict
    #endif /* defined(_MSC_VER) */
    #ifdef __GNUC__
        #define CMN_RESTRICT            __restrict
    #endif /* defined(__GNUC__) */
#endif /*!defined(CMN_RESTRICT) */

/*/////////////////////////////////////////////////////////////////////////80*/

/*//////////////////////////////////
//   Public Types and Functions   //
//////////////////////////////////*/

/*/////////////////////
//   Namespace End   //
/////////////////////*/

#endif /* CMN_COMMONDEFS_HPP_INCLUDED */

/*/////////////////////////////////////////////////////////////////////////////
//    $Id$
///////////////////////////////////////////////////////////////////////////80*/
