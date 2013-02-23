/*
 *  Board-specific setup code for Metodo2 Display 7"
 *
 *  Copyright (C) 2010 Atmel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/flash.h>
#include <linux/spi/spi.h>
#include <linux/fb.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/leds.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <mach/cpu.h>

#include <video/atmel_lcdfb.h>
#include <media/soc_camera.h>
#include <media/atmel-isi.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/atmel_hlcdc.h>
#include <mach/at91sam9_smc.h>
#include <mach/at91_shdwc.h>

#include "sam9_smc.h"
#include "generic.h"
#include <mach/board-sam9x5.h>


static void __init ek_map_io(void)
{
	/* Initialize processor and DBGU */
	cm_map_io();

	/* USART0 on ttyS1. (Rx, Tx) */
	at91_register_uart(AT91SAM9X5_ID_USART0, 1, ATMEL_UART_RTS | ATMEL_UART_CTS);
	/* set RTS down */
	//at91_set_gpio_output(AT91_PIN_PA2, 0);
	/* set WTRES down */
	at91_set_gpio_output(AT91_PIN_PD16, 1);

	/* USART1 on ttyS2. (Rx, Tx) */
	at91_register_uart(AT91SAM9X5_ID_USART1, 2, 0);

}

/*
 * USB Host port (OHCI)
 */
/* Port A is shared with gadget port & Port C is full-speed only */
static struct at91_usbh_data __initdata ek_usbh_fs_data = {
	.ports		= 3,

};

/*
 * USB HS Host port (EHCI)
 */
/* Port A is shared with gadget port */
static struct at91_usbh_data __initdata ek_usbh_hs_data = {
	.ports		= 2,
};


/*
 * USB HS Device port
 */
static struct usba_platform_data __initdata ek_usba_udc_data;


/*
 * MACB Ethernet devices
 */
static struct at91_eth_data __initdata ek_macb0_data = {
	.is_rmii	= 1,
	.phy_irq_pin = AT91_PIN_PB8,
};


/*
 * MCI (SD/MMC)
 */
/* mci0 detect_pin is revision dependent */
static struct mci_platform_data __initdata mci0_data = {
	.slot[0] = {
		.bus_width	= 4,
		.detect_pin	= AT91_PIN_PD15,
		.wp_pin		= -1,
	},
};

#ifdef CONFIG_M2_V9X5_REDPINE_WIFI
static struct mci_platform_data __initdata mci1_data = {
	.slot[0] = {
		.bus_width	= 4,
		.detect_pin	= -1,
		.wp_pin		= -1,
	},
};

static void __init vulcano9x5_wifi_init(void)
{
	/* If enabled, give a reset impulse */
	at91_set_gpio_output(AT91_PIN_PD14, 0);
	mdelay(30);
	at91_set_gpio_output(AT91_PIN_PD14, 1);
}

#endif


/*
 * LCD Controller
 */

#ifdef CONFIG_M2_V9X5_DISPLAY_7

#if defined(CONFIG_FB_ATMEL) || defined(CONFIG_FB_ATMEL_MODULE)
static struct fb_videomode at91_tft_vga_modes[] = {
	{
		.name           = "GFT",
		.refresh	= 60,
		.xres		= 800,		.yres		= 480,
		.pixclock	= 33264,

		.left_margin	= 1,		.right_margin	= 256,
		.upper_margin	= 1,		.lower_margin	= 45,
		.hsync_len	= 1,		.vsync_len	= 1,

		.sync		= FB_SYNC_HOR_HIGH_ACT+FB_SYNC_VERT_HIGH_ACT,
		.vmode		= FB_VMODE_NONINTERLACED,
	},
};

static struct fb_monspecs at91fb_default_monspecs = {
	.manufacturer	= "GFT",
	.monitor        = "GFT070DF",

	.modedb		= at91_tft_vga_modes,
	.modedb_len	= ARRAY_SIZE(at91_tft_vga_modes),
	.hfmin		= 29000,
	.hfmax		= 43000,
	.vfmin		= 57,
	.vfmax		= 77,
};

/* Default output mode is TFT 24 bit */
#define AT91SAM9X5_DEFAULT_LCDCFG5	(LCDC_LCDCFG5_MODE_OUTPUT_18BPP)

/* Driver datas */
static struct atmel_lcdfb_info __initdata ek_lcdc_data = {
	.lcdcon_is_backlight		= true,
	.alpha_enabled			= false,
	.default_bpp			= 16,
	/* Reserve enough memory for 32bpp */
	.smem_len			= 800 * 480 * 4,
	/* In 9x5 default_lcdcon2 is used for LCDCFG5 */
	.default_lcdcon2		= AT91SAM9X5_DEFAULT_LCDCFG5+LCDC_LCDCFG5_VSPDLYE,
	.default_monspecs		= &at91fb_default_monspecs,
	.guard_time			= 2,
	.lcd_wiring_mode		= ATMEL_LCDC_WIRING_RGB,
};

#else
static struct atmel_lcdfb_info __initdata ek_lcdc_data;
#endif
#endif


/*
 * Touchscreen
 */
static struct at91_tsadcc_data ek_tsadcc_data = {
	.adc_clock		= 300000,
	.filtering_average	= 0x03,	/* averages 2^filtering_average ADC conversions */
	.pendet_debounce	= 0x08,
	.pendet_sensitivity	= 0x02,	/* 2 = set to default */
	.ts_sample_hold_time	= 0x0a,
};

static void __init ek_board_init(void)
{
	u32 cm_config;

	cm_board_init(&cm_config);

	#ifdef CONFIG_M2_V9X5_REDPINE_WIFI
	/* WiFi support */
	vulcano9x5_wifi_init();

	/* MMC1 */
	at91_add_device_mci(1, &mci1_data);
	#endif

	/* Serial */
	at91_add_device_serial();

	/* USB HS Host */
	at91_add_device_usbh_ohci(&ek_usbh_fs_data);
	at91_add_device_usbh_ehci(&ek_usbh_hs_data);
	/* USB HS Device */
	at91_add_device_usba(&ek_usba_udc_data);
	/* Ethernet */
	at91_add_device_eth(0, &ek_macb0_data);
	/* MMC0 */
	at91_add_device_mci(0, &mci0_data);

	/* LCD Controller */
	at91_add_device_lcdc(&ek_lcdc_data);
	/* Touch Screen */
	at91_add_device_tsadcc(&ek_tsadcc_data);

	/* SMD */
	at91_add_device_smd();

}

MACHINE_START(AT91SAM9X5EK, "Atmel AT91SAM9X5-EK")
	/* Maintainer: Atmel */
/* XXX/ukl: can we drop .boot_params? */
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91sam926x_timer,
	.map_io		= ek_map_io,
	.init_irq	= cm_init_irq,
	.init_machine	= ek_board_init,
MACHINE_END
