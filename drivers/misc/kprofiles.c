// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Dakkshesh <dakkshesh5@gmail.com>.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kprofiles.h>
#include <linux/msm_drm_notify.h>

static bool screen_on = true;
static unsigned int mode = 0;
module_param(mode, uint, 0664);

static int msm_drm_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
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
		mode = 0;
		break;
	}

out:
	return NOTIFY_OK;
}

inline unsigned int active_mode(void)
{
  switch(mode)
    {
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

static struct notifier_block fb_notifier_block = {
	.notifier_call = msm_drm_notifier_callback,
};

static int  __init kprofiles_notifier_init(void)
{

	msm_drm_register_client(&fb_notifier_block);

	return 0;
}

late_initcall(kprofiles_notifier_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dakkshesh");
MODULE_DESCRIPTION("KernelSpace Profiles");
MODULE_VERSION("1.0.0"); 
