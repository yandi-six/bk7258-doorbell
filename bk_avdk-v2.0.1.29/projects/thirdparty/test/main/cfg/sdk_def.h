

#ifndef _SDK_DEF_H_
#define _SDK_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#define __SDK_API extern



typedef unsigned char       SDK_U8;         /*!< 8 bit unsigned integer. */
typedef unsigned short      SDK_U16;        /*!< 16 bit unsigned integer. */
typedef unsigned int        SDK_U32;        /*!< 32 bit unsigned integer. */
typedef unsigned long long  SDK_U64;        /*!< 64 bit unsigned integer. */
typedef char                SDK_S8;         /*!< 8 bit signed integer. */
typedef signed short        SDK_S16;        /*!< 16 bit signed integer. */
typedef signed int          SDK_S32;        /*!< 32 bit signed integer. */
typedef signed long long    SDK_S64;        /*!< 64 bit signed integer. */
typedef signed int          SDK_ERR;        /*!< error code type .*/
typedef unsigned int        SDK_HANDLE;     /*!< 32 bit unsigned integer. */
typedef char                SDK_CHAR;       /*!< char */
typedef unsigned char       uint8_t;         /*!< 8 bit unsigned integer. */




typedef enum
{
    SDK_FALSE = 0,  /*!< Logical false. */
    SDK_TRUE  = 1   /*!< Logical true. */
}SDK_BOOL;

#define SDK_SUCCESS     0

#ifdef __cplusplus
};
#endif
#endif
