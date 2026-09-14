#ifndef PEACE0X_PLATFORM_H
#define PEACE0X_PLATFORM_H

#include "peace0x/types.h"

int64_t platform_get_time_ms(void);
bool    platform_input_waiting(void);
void    platform_read_input(SearchInfo *info);

#endif /* PEACE0X_PLATFORM_H */
