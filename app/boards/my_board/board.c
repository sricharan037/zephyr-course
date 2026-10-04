/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/sys/printk.h>

/*
 * Runs during kernel initialization (POST_KERNEL), i.e. before the
 * application's main() entry point is entered.
 */
static int board_my_board_init(void)
{
	printk("Board Initialized\n");
	return 0;
}

SYS_INIT(board_my_board_init, POST_KERNEL, 90);
