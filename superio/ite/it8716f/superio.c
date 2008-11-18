/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2006 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2007 AMD
 * (Written by Yinghai Lu <yinghai.lu@amd.com> for AMD)
 * Copyright (C) 2007 Ward Vandewege <ward@gnu.org>
 * Copyright (C) 2008 Peter Stuge <peter@stuge.se>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <io.h>
#include <lib.h>
#include <device/device.h>
#include <device/pnp.h>
#include <console.h>
#include <string.h>
#include <uart8250.h>
#include <keyboard.h>
#include <statictree.h>
#include "it8716f.h"

#ifdef HAVE_FANCTL
extern void init_ec(u16 base);
#else
static void pnp_write_index(u16 port_base, u8 reg, u8 value)
{
	outb(reg, port_base);
	outb(value, port_base + 1);
}

static u8 pnp_read_index(u16 port_base, u8 reg)
{
	outb(reg, port_base);
	return inb(port_base + 1);
}

static void init_ec(u16 base)
{
	u8 value;

	/* Read out current value of FAN_CTL control register (0x14). */
	value = pnp_read_index(base, 0x14);
	printk(BIOS_DEBUG, "FAN_CTL: reg = 0x%04x, read value = 0x%02x\n",
		base + 0x14, value);

	/* Set FAN_CTL control register (0x14) polarity to high, and
	 * activate fans 1, 2 and 3. */
	pnp_write_index(base, 0x14, value | 0x87);
	printk(BIOS_DEBUG, "FAN_CTL: reg = 0x%04x, writing value = 0x%02x\n",
		base + 0x14, value | 0x87);
}
#endif

static void it8716f_pnp_set_resources(struct device *dev)
{
	pnp_enter_ite(dev);
	pnp_set_resources(dev);
	pnp_exit_ite(dev);
}

static void it8716f_pnp_enable_disable(struct device *dev)
{
	pnp_enter_ite(dev);
	pnp_enable_resources(dev);
	pnp_exit_ite(dev);
}

static void it8716f_pnp_enable_resources(struct device *dev)
{
	pnp_enter_ite(dev);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, dev->enabled);
	pnp_exit_ite(dev);
}

static void it8716f_init(struct device *dev)
{
	struct superio_ite_it8716f_dts_config *conf;
	struct resource *res0, *res1;
	struct pc_keyboard kbd;

	if (!dev->enabled)
		return;

	conf = dev->device_configuration;

	/* TODO: FDC, PP, KBCM, MIDI, GAME, IR. */
	switch (dev->path.pnp.device) {
	case IT8716F_SP1:
		res0 = find_resource(dev, PNP_IDX_IO0);
//		init_uart8250(res0->base, &conf->com1);
		break;
	case IT8716F_SP2:
		res0 = find_resource(dev, PNP_IDX_IO0);
//		init_uart8250(res0->base, &conf->com2);
		break;
	case IT8716F_EC:
		res0 = find_resource(dev, PNP_IDX_IO0);
#define EC_INDEX_PORT 5
		init_ec(res0->base + EC_INDEX_PORT);
		break;
	case IT8716F_KBCK:
		res0 = find_resource(dev, PNP_IDX_IO0);
		res1 = find_resource(dev, PNP_IDX_IO1);
		init_pc_keyboard(res0->base, res1->base, &kbd);
		break;
	}
}

static void it8716f_setup_scan_bus(struct device *dev);

struct device_operations it8716f_ops = {
	.phase3_chip_setup_dev	 = it8716f_setup_scan_bus,
	.phase3_enable		 = it8716f_pnp_enable_disable,
	.phase4_read_resources	 = pnp_read_resources,
	.phase4_set_resources	 = it8716f_pnp_set_resources,
	.phase5_enable_resources = it8716f_pnp_enable_resources,
	.phase6_init		 = it8716f_init,
};

static struct pnp_info pnp_dev_info[] = {
				/* Enable,  All resources need by dev,  io_info_structs */
	{&it8716f_ops, IT8716F_FDC, 0, PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x07f8, 0},},
	{&it8716f_ops, IT8716F_SP1, 0, PNP_IO0 | PNP_IRQ0, {0x7f8, 0},},
	{&it8716f_ops, IT8716F_SP2, 0, PNP_IO0 | PNP_IRQ0, {0x7f8, 0},},
	{&it8716f_ops, IT8716F_PP, 0, PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x07f8, 0},},
	{&it8716f_ops, IT8716F_EC, 0, PNP_IO0 | PNP_IO1 | PNP_IRQ0, {0x7f8, 0},
	 {0x7f8, 0x4},},
	{&it8716f_ops, IT8716F_KBCK, 0, PNP_IO0 | PNP_IO1 | PNP_IRQ0, {0x7ff, 0},
	 {0x7ff, 0x4},},
	{&it8716f_ops, IT8716F_KBCM, 0, PNP_IRQ0,},
	{&it8716f_ops, IT8716F_GPIO, 0, PNP_IO1 | PNP_IO2, {0, 0}, {0x7f8, 0}, {0x7f8, 0},},
	{&it8716f_ops, IT8716F_MIDI, 0, PNP_IO0 | PNP_IRQ0, {0x7fe, 0x4},},
	{&it8716f_ops, IT8716F_GAME, 0, PNP_IO0, {0x7ff, 0},},
	{&it8716f_ops, IT8716F_IR, 0,},
};

static void it8716f_setup_scan_bus(struct device *dev)
{
	const struct superio_ite_it8716f_dts_config * const conf = dev->device_configuration;

	/* Floppy */
	pnp_dev_info[IT8716F_FDC].enable = conf->floppyenable;
	pnp_dev_info[IT8716F_FDC].io0.val = conf->floppyio;
	pnp_dev_info[IT8716F_FDC].irq0.val = conf->floppyirq;
	pnp_dev_info[IT8716F_FDC].drq0.val = conf->floppydrq;

	/* COM1 */
	pnp_dev_info[IT8716F_SP1].enable = conf->com1enable;
	pnp_dev_info[IT8716F_SP1].io0.val = conf->com1io;
	pnp_dev_info[IT8716F_SP1].irq0.val = conf->com1irq;

	/* COM2 */
	pnp_dev_info[IT8716F_SP2].enable = conf->com2enable;
	pnp_dev_info[IT8716F_SP2].io0.val = conf->com2io;
	pnp_dev_info[IT8716F_SP2].irq0.val = conf->com2irq;

	/* Parallel port */
	pnp_dev_info[IT8716F_PP].enable = conf->ppenable;
	pnp_dev_info[IT8716F_PP].io0.val = conf->ppio;
	pnp_dev_info[IT8716F_PP].irq0.val = conf->ppirq;

	/* Environment controller */
	pnp_dev_info[IT8716F_EC].enable = conf->ecenable;
	pnp_dev_info[IT8716F_EC].io0.val = conf->ecio;
	pnp_dev_info[IT8716F_EC].irq0.val = conf->ecirq;

	/* Keyboard */
	pnp_dev_info[IT8716F_KBCK].enable = conf->kbenable;
	pnp_dev_info[IT8716F_KBCK].io0.val = conf->kbio;
	pnp_dev_info[IT8716F_KBCK].io1.val = conf->kbio2;
	pnp_dev_info[IT8716F_KBCK].irq0.val = conf->kbirq;

	/* PS/2 mouse */
	pnp_dev_info[IT8716F_KBCM].enable = conf->mouseenable;
	pnp_dev_info[IT8716F_KBCM].irq0.val = conf->mouseirq;

	/* GPIO */
	pnp_dev_info[IT8716F_GPIO].enable = conf->gpioenable;

	/* MIDI port */
	pnp_dev_info[IT8716F_MIDI].enable = conf->midienable;
	pnp_dev_info[IT8716F_MIDI].io0.val = conf->midiio;
	pnp_dev_info[IT8716F_MIDI].irq0.val = conf->midiirq;

	/* Game port */
	pnp_dev_info[IT8716F_GAME].enable = conf->gameenable;
	pnp_dev_info[IT8716F_GAME].io0.val = conf->gameio;

	/* Consumer IR */
	pnp_dev_info[IT8716F_IR].enable = conf->cirenable;
	pnp_dev_info[IT8716F_IR].io0.val = conf->cirio;
	pnp_dev_info[IT8716F_IR].irq0.val = conf->cirirq;

	/* Initialize SuperIO for PNP children. */
	if (!dev->links) {
		dev->links = 1;
		dev->link[0].dev = dev;
		dev->link[0].children = NULL;
		dev->link[0].link = 0;
	}

	/* Call init with updated tables to create children. */
	pnp_enable_devices(dev, &it8716f_ops,
			   ARRAY_SIZE(pnp_dev_info), pnp_dev_info);
}
