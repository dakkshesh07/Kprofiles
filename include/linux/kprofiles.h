// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Dakkshesh <dakkshesh5@gmail.com>.
 */

#ifndef _KPROFILES_H_
#define _KPROFILES_H_

#include <linux/types.h>

#ifdef CONFIG_KPROFILES
void kprofiles_set_mode_rollback(unsigned int level, unsigned int duration_ms);
void kprofiles_set_mode(unsigned int level);
unsigned int active_mode(void);
#else
static inline void kprofiles_set_mode_rollback(unsigned int level, unsigned int duration_ms)
{
}
static inline void kprofiles_set_mode(unsigned int level)
{
}
static inline unsigned int active_mode(void)
{
	return 0;
}
#endif

#endif /* _KPROFILES_H_ */ 
