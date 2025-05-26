#ifndef __GLUE_V4_I16__CONFIG_H__
#define __GLUE_V4_I16__CONFIG_H__

#define INC_ARCH_SA(x)             <arch/__ARCH__/__SUBARCH__/x>
#define INC_GLUE_SA(x)             <glue/__API__-__ARCH__/__SUBARCH__/x>

#include INC_GLUE_SA(config.h)

#endif /* !__GLUE_V4_I16__CONFIG_H__ */
