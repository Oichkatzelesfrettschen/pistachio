/* Revision 1.0  92/06/06  20:28:35  iwd
 *      Created.
 * 	[92/06/06            iwd]
 */
/*
 * Define macros to allow support for running handling little endian
 * executables on a big endian machine and visa versa. These have
 * to be macros and not functions because they use "sizeof".
 * Originally created by Ian Dall (idall@augean.eleceng.adelaide.edu.au).
 */


#define CROSS_LINKER		/* Probably should be a 'config' option */
#ifdef CROSS_LINKER
#define INTERN(field) (get_num(&(field), sizeof(field)))

#define PUT_EXTERN(buf, val) (put_num(&(buf), (val), sizeof(buf)))
#define EXTERNALIZE(x) (put_num(&(x), (x), sizeof(x)))
#define INTERNALIZE(x) ((x) = get_num(&(x), sizeof(x)), sizeof(x))

#else
#define INTERN(field) (field)
#define PUT_EXTERN(buf, val) ((buf) = (val))
#define EXTERNALIZE(x) (x)
#define INTERNALIZE(x) (x)
#endif
