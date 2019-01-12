/*
 * linux/arch/m68k/amiga/amikeyb.c
 *
 * Amiga Keyboard driver for Linux/m68k
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

/*
 * Amiga support by Hamish Macdonald
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/keyboard.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/kd.h>
#include <linux/random.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kbd_ll.h>

#include <asm/amigaints.h>
#include <asm/amigahw.h>
#include <asm/irq.h>

#define AMIKEY_CAPS	(0x62)
#define BREAK_MASK	(0x80)
#define RESET_WARNING	(0xf0)	/* before rotation */

static u_short amiplain_map[NR_KEYS] __initdata = {
	0xf060, 0xf031, 0xf032, 0xf033, 0xf034, 0xf035, 0xf036, 0xf037,
	0xf038, 0xf039, 0xf030, 0xf02d, 0xf03d, 0xf05c, 0xf200, 0xf300,
	0xfb71, 0xfb77, 0xfb65, 0xfb72, 0xfb74, 0xfb79, 0xfb75, 0xfb69,
	0xfb6f, 0xfb70, 0xf05b, 0xf05d, 0xf200, 0xf301, 0xf302, 0xf303,
	0xfb61, 0xfb73, 0xfb64, 0xfb66, 0xfb67, 0xfb68, 0xfb6a, 0xfb6b,
	0xfb6c, 0xf03b, 0xf027, 0xf200, 0xf200, 0xf304, 0xf305, 0xf306,
	0xf200, 0xfb7a, 0xfb78, 0xfb63, 0xfb76, 0xfb62, 0xfb6e, 0xfb6d,
	0xf02c, 0xf02e, 0xf02f, 0xf200, 0xf310, 0xf307, 0xf308, 0xf309,
	0xf020, 0xf07f, 0xf009, 0xf30e, 0xf201, 0xf01b, 0xf07f, 0xf200,
	0xf200, 0xf200, 0xf30b, 0xf200, 0xf603, 0xf600, 0xf602, 0xf601,
	0xf100, 0xf101, 0xf102, 0xf103, 0xf104, 0xf105, 0xf106, 0xf107,
	0xf108, 0xf109, 0xf312, 0xf313, 0xf30d, 0xf30c, 0xf30a, 0xf11b,
	0xf700, 0xf700, 0xf207, 0xf702, 0xf703, 0xf701, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
};

static u_short amishift_map[NR_KEYS] __initdata = {
	0xf07e, 0xf021, 0xf040, 0xf023, 0xf024, 0xf025, 0xf05e, 0xf026,
	0xf02a, 0xf028, 0xf029, 0xf05f, 0xf02b, 0xf07c, 0xf200, 0xf300,
	0xfb51, 0xfb57, 0xfb45, 0xfb52, 0xfb54, 0xfb59, 0xfb55, 0xfb49,
	0xfb4f, 0xfb50, 0xf07b, 0xf07d, 0xf200, 0xf301, 0xf302, 0xf303,
	0xfb41, 0xfb53, 0xfb44, 0xfb46, 0xfb47, 0xfb48, 0xfb4a, 0xfb4b,
	0xfb4c, 0xf03a, 0xf022, 0xf200, 0xf200, 0xf304, 0xf305, 0xf306,
	0xf200, 0xfb5a, 0xfb58, 0xfb43, 0xfb56, 0xfb42, 0xfb4e, 0xfb4d,
	0xf03c, 0xf03e, 0xf03f, 0xf200, 0xf310, 0xf307, 0xf308, 0xf309,
	0xf020, 0xf07f, 0xf009, 0xf30e, 0xf201, 0xf01b, 0xf07f, 0xf200,
	0xf200, 0xf200, 0xf30b, 0xf200, 0xf603, 0xf600, 0xf602, 0xf601,
	0xf10a, 0xf10b, 0xf10c, 0xf10d, 0xf10e, 0xf10f, 0xf110, 0xf111,
	0xf112, 0xf113, 0xf208, 0xf209, 0xf30d, 0xf30c, 0xf30a, 0xf203,
	0xf700, 0xf700, 0xf207, 0xf702, 0xf703, 0xf701, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
};

static u_short amialtgr_map[NR_KEYS] __initdata = {
	0xf200, 0xf200, 0xf040, 0xf200, 0xf024, 0xf200, 0xf200, 0xf07b,
	0xf05b, 0xf05d, 0xf07d, 0xf05c, 0xf200, 0xf200, 0xf200, 0xf300,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf07e, 0xf200, 0xf301, 0xf302, 0xf303,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf304, 0xf305, 0xf306,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf310, 0xf307, 0xf308, 0xf309,
	0xf200, 0xf07f, 0xf200, 0xf30e, 0xf201, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf30b, 0xf200, 0xf603, 0xf600, 0xf602, 0xf601,
	0xf50c, 0xf50d, 0xf50e, 0xf50f, 0xf510, 0xf511, 0xf512, 0xf513,
	0xf514, 0xf515, 0xf208, 0xf209, 0xf30d, 0xf30c, 0xf30a, 0xf204,
	0xf700, 0xf700, 0xf207, 0xf702, 0xf703, 0xf701, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
};

static u_short amictrl_map[NR_KEYS] __initdata = {
	0xf000, 0xf200, 0xf000, 0xf01b, 0xf01c, 0xf01d, 0xf01e, 0xf01f,
	0xf07f, 0xf200, 0xf200, 0xf01f, 0xf200, 0xf01c, 0xf200, 0xf300,
	0xf011, 0xf017, 0xf005, 0xf012, 0xf014, 0xf019, 0xf015, 0xf009,
	0xf00f, 0xf010, 0xf01b, 0xf01d, 0xf200, 0xf301, 0xf302, 0xf303,
	0xf001, 0xf013, 0xf004, 0xf006, 0xf007, 0xf008, 0xf00a, 0xf00b,
	0xf00c, 0xf200, 0xf007, 0xf200, 0xf200, 0xf304, 0xf305, 0xf306,
	0xf200, 0xf01a, 0xf018, 0xf003, 0xf016, 0xf002, 0xf00e, 0xf00d,
	0xf200, 0xf200, 0xf07f, 0xf200, 0xf310, 0xf307, 0xf308, 0xf309,
	0xf000, 0xf07f, 0xf200, 0xf30e, 0xf201, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf30b, 0xf200, 0xf603, 0xf600, 0xf602, 0xf601,
	0xf100, 0xf101, 0xf102, 0xf103, 0xf104, 0xf105, 0xf106, 0xf107,
	0xf108, 0xf109, 0xf208, 0xf209, 0xf30d, 0xf30c, 0xf30a, 0xf202,
	0xf700, 0xf700, 0xf207, 0xf702, 0xf703, 0xf701, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
};

static u_short amishift_ctrl_map[NR_KEYS] __initdata = {
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf300,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf301, 0xf302, 0xf303,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf304, 0xf305, 0xf306,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf310, 0xf307, 0xf308, 0xf309,
	0xf200, 0xf07f, 0xf200, 0xf30e, 0xf201, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf30b, 0xf200, 0xf603, 0xf600, 0xf602, 0xf601,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf208, 0xf209, 0xf30d, 0xf30c, 0xf30a, 0xf200,
	0xf700, 0xf700, 0xf207, 0xf702, 0xf703, 0xf701, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
};

static u_short amialt_map[NR_KEYS] __initdata = {
	0xf860, 0xf831, 0xf832, 0xf833, 0xf834, 0xf835, 0xf836, 0xf837,
	0xf838, 0xf839, 0xf830, 0xf82d, 0xf83d, 0xf85c, 0xf200, 0xf900,
	0xf871, 0xf877, 0xf865, 0xf872, 0xf874, 0xf879, 0xf875, 0xf869,
	0xf86f, 0xf870, 0xf85b, 0xf85d, 0xf200, 0xf901, 0xf902, 0xf903,
	0xf861, 0xf873, 0xf864, 0xf866, 0xf867, 0xf868, 0xf86a, 0xf86b,
	0xf86c, 0xf83b, 0xf827, 0xf200, 0xf200, 0xf904, 0xf905, 0xf906,
	0xf200, 0xf87a, 0xf878, 0xf863, 0xf876, 0xf862, 0xf86e, 0xf86d,
	0xf82c, 0xf82e, 0xf82f, 0xf200, 0xf310, 0xf907, 0xf908, 0xf909,
	0xf820, 0xf87f, 0xf809, 0xf30e, 0xf80d, 0xf81b, 0xf87f, 0xf200,
	0xf200, 0xf200, 0xf30b, 0xf200, 0xf603, 0xf600, 0xf602, 0xf601,
	0xf500, 0xf501, 0xf502, 0xf503, 0xf504, 0xf505, 0xf506, 0xf507,
	0xf508, 0xf509, 0xf208, 0xf209, 0xf30d, 0xf30c, 0xf30a, 0xf204,
	0xf700, 0xf700, 0xf207, 0xf702, 0xf703, 0xf701, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
};

static u_short amictrl_alt_map[NR_KEYS] __initdata = {
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf300,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf301, 0xf302, 0xf303,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf304, 0xf305, 0xf306,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf20c, 0xf307, 0xf308, 0xf309,
	0xf200, 0xf07f, 0xf200, 0xf30e, 0xf201, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf30b, 0xf200, 0xf603, 0xf600, 0xf602, 0xf601,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf208, 0xf209, 0xf30d, 0xf30c, 0xf30a, 0xf200,
	0xf700, 0xf700, 0xf207, 0xf702, 0xf703, 0xf701, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
	0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200, 0xf200,
};

#define	DEFAULT_KEYB_REP_DELAY	(HZ/4)
#define	DEFAULT_KEYB_REP_RATE	(HZ/25)

/* These could be settable by some ioctl() in future... */
static unsigned int key_repeat_delay = DEFAULT_KEYB_REP_DELAY;
static unsigned int key_repeat_rate  = DEFAULT_KEYB_REP_RATE;

static unsigned char rep_scancode;
static void amikeyb_rep(unsigned long ignore);
static struct timer_list amikeyb_rep_timer = {NULL, NULL, 0, 0, amikeyb_rep};

static void amikeyb_rep(unsigned long ignore)
{
    unsigned long flags;
    save_flags(flags);
    cli();

    kbd_pt_regs = NULL;

    amikeyb_rep_timer.expires = jiffies + key_repeat_rate;
    amikeyb_rep_timer.prev = amikeyb_rep_timer.next = NULL;
    add_timer(&amikeyb_rep_timer);
    handle_scancode(rep_scancode, 1);

    restore_flags(flags);
}

static void keyboard_interrupt(int irq, void *dummy, struct pt_regs *fp)
{
    unsigned char scancode, break_flag, keycode;
    static int reset_warning = 0;

    /* save frame for register dump */
    kbd_pt_regs = fp;

    /* get and invert scancode (keyboard is active low) */
    scancode = ~ciaa.sdr;

    /* switch SP pin to output for handshake */
    ciaa.cra |= 0x40;

#if 0 /* No longer used */
    /*
     *  On receipt of the second RESET_WARNING, we must not pull KDAT high
     *  again to delay the hard reset as long as possible.
     *
     *  Note that not all keyboards send reset warnings...
     */
    if (reset_warning)
	if (scancode == RESET_WARNING) {
	    printk(KERN_ALERT "amikeyb: Ctrl-Amiga-Amiga reset warning!!\n"
		   "The system will be reset within 10 seconds!!\n");
	    /* Panic doesn't sync from within an interrupt, so we do nothing */
	    return;
	} else
	    /* Probably a mistake, cancel the alert */
	    reset_warning = 0;
#endif

    /* wait until 85 us have expired */
    udelay(85);
    /* switch CIA serial port to input mode */
    ciaa.cra &= ~0x40;

    mark_bh(KEYBOARD_BH);

    /* rotate scan code to get up/down bit in proper position */
    scancode = ((scancode >> 1) & 0x7f) | ((scancode << 7) & 0x80);

    /*
     * Check make/break first
     */
    break_flag = scancode & BREAK_MASK;
    keycode = scancode & (unsigned char)~BREAK_MASK;

    if (keycode == AMIKEY_CAPS) {
	/* if the key is CAPS, fake a press/release. */
	handle_scancode(AMIKEY_CAPS, 1);
	handle_scancode(AMIKEY_CAPS, 0);
    } else if (keycode < 0x78) {
	/* handle repeat */
	if (break_flag) {
	    del_timer(&amikeyb_rep_timer);
	    rep_scancode = 0;
	} else {
	    del_timer(&amikeyb_rep_timer);
	    rep_scancode = keycode;
	    amikeyb_rep_timer.expires = jiffies + key_repeat_delay;
	    amikeyb_rep_timer.prev = amikeyb_rep_timer.next = NULL;
	    add_timer(&amikeyb_rep_timer);
	}
	handle_scancode(scancode, !break_flag);
    } else
	switch (keycode) {
	    case 0x78:
		reset_warning = 1;
		break;
	    case 0x79:
		printk(KERN_WARNING "amikeyb: keyboard lost sync\n");
		break;
	    case 0x7a:
		printk(KERN_WARNING "amikeyb: keyboard buffer overflow\n");
		break;
#if 0 /* obsolete according to the HRM */
	    case 0x7b:
		printk(KERN_WARNING "amikeyb: keyboard controller failure\n");
		break;
#endif
	    case 0x7c:
		printk(KERN_ERR "amikeyb: keyboard selftest failure\n");
		break;
	    case 0x7d:
		printk(KERN_INFO "amikeyb: initiate power-up key stream\n");
		break;
	    case 0x7e:
		printk(KERN_INFO "amikeyb: terminate power-up key stream\n");
		break;
#if 0 /* obsolete according to the HRM */
	    case 0x7f:
		printk(KERN_WARNING "amikeyb: keyboard interrupt\n");
		break;
#endif
	    default:
		printk(KERN_WARNING "amikeyb: unknown keyboard communication code 0x%02x\n",
		       scancode);
		break;
	}
}

__initfunc(int amiga_keyb_init(void))
{
    if (!AMIGAHW_PRESENT(AMI_KEYBOARD))
        return -EIO;

    /* setup key map */
    memcpy(key_maps[0], amiplain_map, sizeof(plain_map));
    memcpy(key_maps[1], amishift_map, sizeof(plain_map));
    memcpy(key_maps[2], amialtgr_map, sizeof(plain_map));
    memcpy(key_maps[4], amictrl_map, sizeof(plain_map));
    memcpy(key_maps[5], amishift_ctrl_map, sizeof(plain_map));
    memcpy(key_maps[8], amialt_map, sizeof(plain_map));
    memcpy(key_maps[12], amictrl_alt_map, sizeof(plain_map));

    /*
     * Initialize serial data direction.
     */
    ciaa.cra &= ~0x41;	     /* serial data in, turn off TA */

    /*
     * arrange for processing of keyboard interrupt
     */
    request_irq(IRQ_AMIGA_CIAA_SP, keyboard_interrupt, 0, "keyboard", NULL);

    return 0;
}

int amiga_kbdrate( struct kbd_repeat *k )
{
    if (k->delay > 0) {
	/* convert from msec to jiffies */
	key_repeat_delay = (k->delay * HZ + 500) / 1000;
	if (key_repeat_delay < 1)
            key_repeat_delay = 1;
    }
    if (k->rate > 0) {
	key_repeat_rate = (k->rate * HZ + 500) / 1000;
	if (key_repeat_rate < 1)
            key_repeat_rate = 1;
    }

    k->delay = key_repeat_delay * 1000 / HZ;
    k->rate  = key_repeat_rate  * 1000 / HZ;
    
    return( 0 );
}

/* for "kbd-reset" cmdline param */
__initfunc(void amiga_kbd_reset_setup(char *str, int *ints))
{
}
