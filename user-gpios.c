/*
 * user-gpios
 *
 * Copyright (c) 2019 Data Respons Solutions AB
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

static int __init user_gpios_init(void)
{
	struct device_node* user_gpios = NULL;
	struct device_node* it = NULL;
	int gpio_nr = 0;
	int ret = 0;
	u32 val = 0;
	unsigned long flags = 0;
	enum of_gpio_flags of_flags = OF_GPIO_ACTIVE_LOW;

	user_gpios = of_find_node_by_name(NULL, "user-gpios");
	if (user_gpios) {	/* Iterate nodes */
		it = NULL;
		// Test whether all nodes are ready, if not ask to try again later
		while ((it = of_get_next_available_child(user_gpios, it))) {
			gpio_nr = of_get_gpio_flags(it, 0, &of_flags);
			if (!gpio_is_valid(gpio_nr)) {
				pr_warn("%s: gpio %s [%d] not ready: defer driver init\n", __func__, of_node_full_name(it), gpio_nr);
				return -EPROBE_DEFER;
			}
		}

		it = NULL;
		while ((it = of_get_next_available_child(user_gpios, it))) {
			gpio_nr = of_get_gpio_flags(it, 0, &of_flags);

			if (!gpio_is_valid(gpio_nr)) {
				pr_err("%s: Could not get gpio for %s [%d]\n", __func__, of_node_full_name(it), gpio_nr);
				continue;
			}

			flags = GPIOF_EXPORT_DIR_CHANGEABLE;

			int is_active_low = 0;
			if (of_flags & OF_GPIO_ACTIVE_LOW) {
				is_active_low = 1;
				flags |= GPIOF_ACTIVE_LOW;
			}

			val = 0;
			if (of_property_read_u32(it, "value", &val) == 0)	{
				if (is_active_low) {
					flags |= val ? GPIOF_OUT_INIT_LOW : GPIOF_OUT_INIT_HIGH;
				}
				else {
					flags |= val ? GPIOF_OUT_INIT_HIGH : GPIOF_OUT_INIT_LOW;
				}

			}
			else {
				flags |= GPIOF_DIR_IN;
			}

			ret = gpio_request_one(gpio_nr, flags, it->name);
			if (ret < 0) {
				pr_err("%s: Could not request gpio %d\n", __func__, gpio_nr);
				continue;
			}
			pr_info("%s: Setting up gpio %s [%d], active=%s, value=%d\n", __func__,
					of_node_full_name(it), gpio_nr, is_active_low ? "low" : "high", val);
		}
		of_node_put(user_gpios);
	}

	return 0;
}

module_init(user_gpios_init);

MODULE_AUTHOR("Mikko Salomäki <ms@datarespons.se>");
MODULE_AUTHOR("Hans Christian Lonstad <hcl@datarespons.no>");
MODULE_DESCRIPTION("user-gpios autoexporter");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("user-gpios");
