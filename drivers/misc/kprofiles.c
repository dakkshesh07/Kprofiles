// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Dakkshesh <dakkshesh5@gmail.com>.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kprofiles.h>
#ifdef CONFIG_AUTO_KPROFILES_MSM_DRM
#include <linux/msm_drm_notify.h>
#elif defined(CONFIG_AUTO_KPROFILES_FB)
#include <linux/fb.h>
#endif

static bool screen_on = true;
static unsigned int mode = 0;
#if defined(CONFIG_AUTO_KPROFILES_MSM_DRM) || defined(CONFIG_AUTO_KPROFILES_FB)
static unsigned int set_mode;
#endif
module_param(mode, uint, 0664);

#if defined(CONFIG_AUTO_KPROFILES_MSM_DRM) || defined(CONFIG_AUTO_KPROFILES_FB)
static int common_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
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
		mode = 1;
		break;
	case MSM_DRM_BLANK_UNBLANK:
		if (screen_on)
			break;
		screen_on = true;
		mode = set_mode;
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
		mode = 1;
		break;
	case FB_BLANK_UNBLANK:
		if (screen_on)
			break;
		screen_on = true;
		mode = set_mode;
		break;
	}
#endif

out:
	return NOTIFY_OK;
}
#endif

inline unsigned int active_mode(void)
{
  switch(mode)
    {
    case 0:
      return 0;
      break;
    case 1:
      return 1;
      break;
    case 2:
      return 2;
      break;
    case 3:
      return 3;
      break;
    default:
      pr_info("Invalid value passed, falling back to level 0\n");
      return 0;
    }
}

#if defined(CONFIG_AUTO_KPROFILES_MSM_DRM) || defined(CONFIG_AUTO_KPROFILES_FB)
static struct notifier_block common_notifier_block = {
	.notifier_call = common_notifier_callback,
};

static int  __init kprofiles_notifier_init(void)
{

#ifdef CONFIG_AUTO_KPROFILES_MSM_DRM
	msm_drm_register_client(&common_notifier_block);
#elif defined(CONFIG_AUTO_KPROFILES_FB)
	fb_register_client(&common_notifier_block);
#endif
	return 0;
}

late_initcall(kprofiles_notifier_init);
#endif
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dakkshesh");
MODULE_DESCRIPTION("KernelSpace Profiles");
MODULE_VERSION("2.0.0");
