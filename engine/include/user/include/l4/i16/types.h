/*********************************************************************
 * Minimal 16 bit architecture type definitions
 ********************************************************************/
#ifndef __L4__I16__TYPES_H__
#define __L4__I16__TYPES_H__

#define L4_LITTLE_ENDIAN

typedef unsigned long long       L4_Word64_t;
typedef unsigned long            L4_Word32_t;
typedef unsigned short           L4_Word16_t;
typedef unsigned char            L4_Word8_t;

typedef unsigned short           L4_Word_t;

typedef signed long long         L4_SignedWord64_t;
typedef signed long              L4_SignedWord32_t;
typedef signed short             L4_SignedWord16_t;
typedef signed char              L4_SignedWord8_t;

typedef unsigned int             L4_Size_t;
typedef L4_Word64_t              L4_Paddr_t;

#endif /* !__L4__I16__TYPES_H__ */
