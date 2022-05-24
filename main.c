/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021-2022 Dakkshesh <dakkshesh5@gmail.com>.
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#ifdef CONFIG_AUTO_KPROFILES_MSM_DRM
#include <linux/msm_drm_notify.h>
#elif defined(CONFIG_AUTO_KPROFILES_FB)
#include <linux/fb.h>
#endif
#include "version.h"

static int mode = 0;
module_param(mode, uint, 0664);

static unsigned int override_mode;
static bool override = false;

static bool auto_kprofiles = true;
module_param(auto_kprofiles, bool, 0664);

#ifdef CONFIG_AUTO_KPROFILES
static bool screen_on = true;
#endif

DEFINE_MUTEX(kplock);

void kprofiles_set_mode_rollback(unsigned int level, unsigned int duration_ms)
{
	mutex_lock(&kplock);
	if (level && duration_ms && auto_kprofiles) {
		override_mode = level;
		override = true;
		msleep(duration_ms);
		override = false;
	}
	mutex_unlock(&kplock);
}

void kprofiles_set_mode(unsigned int level)
{
	if (level && auto_kprofiles)
		mode = level;
}

#ifdef CONFIG_AUTO_KPROFILES
static inline int kp_notifier_callback(struct notifier_block *self,
	unsigned long event, void *data)
{
	if (!auto_kprofiles)
		goto out;
#ifdef CONFIG_AUTO_KPROFILES_MSM_DRM
	struct msm_drm_notifier *evdata = data;
	int *blank;

	if (event != MSM_DRM_EVENT_BLANK)
		goto out;

	if (!evdata || !evdata->data || evdata->id != MSM_DRM_PRIMARY_DISPLAY)
		goto out;

	blank = evdata->data;
	switch (*blank) {
	case MSM_DRM_BLANK_POWERDOWN:
		if (!screen_on)
			break;
		screen_on = false;
		break;
	case MSM_DRM_BLANK_UNBLANK:
		if (screen_on)
			break;
		screen_on = true;
		break;
	}
#elif defined(CONFIG_AUTO_KPROFILES_FB)
	struct fb_event *evdata = data;
	int *blank;

	if (event != FB_EVENT_BLANK)
		goto out;

	blank = evdata->data;
	switch (*blank) {
	case FB_BLANK_POWERDOWN:
		if (!screen_on)
			break;
		screen_on = false;
		break;
	case FB_BLANK_UNBLANK:
		if (screen_on)
			break;
		screen_on = true;
		break;
	}
#endif

out:
	return NOTIFY_OK;
}
#endif

int active_mode(void)
{
#ifdef CONFIG_AUTO_KPROFILES
	if (!screen_on)
		return 1;
#endif

	if (override)
		return override_mode;

	if (mode < 4)
		return mode;

	pr_info("Invalid value passed, falling back to level 0\n");
	return 0;
}

static struct notifier_block kp_notifier_block = {
	.notifier_call = kp_notifier_callback,
};

static int __init kp_init(void)
{
#ifdef CONFIG_AUTO_KPROFILES_MSM_DRM
	int ret;
	ret = msm_drm_register_client(&kp_notifier_block);
	if (ret) {
		pr_err("Failed to register msm_drm notifier, err: %d\n", ret);
		msm_drm_unregister_client(&kp_notifier_block);
	}
#elif defined(CONFIG_AUTO_KPROFILES_FB)
	int ret;
	ret = fb_register_client(&kp_notifier_block);
	if (ret) {
		pr_err("Failed to register fb notifier, err: %d\n", ret);
		fb_unregister_client(&kp_notifier_block);
	}
#endif
	return 0;
}

static int __exit kp_exit(void)
{
#ifdef CONFIG_AUTO_KPROFILES_MSM_DRM
	msm_drm_unregister_client(&kp_notifier_block);
#elif defined(CONFIG_AUTO_KPROFILES_FB)
	fb_unregister_client(&kp_notifier_block);
#endif
	return 0;
}

module_init(kp_init);
module_exit(kp_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("KernelSpace Profiles");
MODULE_AUTHOR("Dakkshesh <dakkshesh5@gmail.com>");
MODULE_VERSION(KPROFILES_VERSION);
