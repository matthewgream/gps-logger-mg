
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_lpc21XXhal.c: gps logger module for LPC21XX hardware interface.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#include "common.h"
#include "mod_lpc21XXhal.h"

#ifndef INLINE
/*@-namechecks@*/
#include "targets/lpc213x.h"
/*@+namechecks@*/
#endif

/*@-exportheader@*/
/*@-exportlocal@*/

/* ----------------------------------------------------------------------------------------------------*/

#ifndef SPLINT
#define LPC21XX_NOP()		asm volatile ("nop")
#else
#define LPC21XX_NOP()		
#endif

#ifndef INLINE
#define LPC21XX_SPI_CS	((unsigned long) 0x00000080)
#endif

/* ----------------------------------------------------------------------------------------------------*/

/*@i@*/ void __div0 (void)
{
		HALT (("divide_by_zero"));
}

/* ----------------------------------------------------------------------------------------------------*/

void __attribute__ ((interrupt ("UNDEF"))) undef_handler (void)
{
		HALT (("undef_handler"));
}

void __attribute__ ((interrupt ("SWI"))) swi_handler (void)
{
		HALT (("swi_handler"));
}

void __attribute__ ((interrupt ("PABORT"))) pabort_handler (void)
{
		HALT (("pabort_handler"));
}

void __attribute__ ((interrupt ("DABORT"))) dabort_handler (void)
{
		HALT (("dabort_handler"));
}

void __attribute__ ((interrupt ("IRQ"))) irq_handler (void)
{
		HALT (("irq_handler"));
}

void __attribute__ ((interrupt ("FIQ"))) fiq_handler (void)
{
		HALT (("fiq_handler"));
}

/* ----------------------------------------------------------------------------------------------------*/

#define LPC21XX_IRQ_MASK	  ((unsigned long) 0x00000080)
#define LPC21XX_FIQ_MASK	  ((unsigned long) 0x00000040)

#define LPC21XX_IRQ_CLEAR	  VICVectAddr = ((unsigned long) 0x00000000)

#ifndef SPLINT
#define LPC21XX_CPSR_GET(r)	  asm volatile (" mrs %0, cpsr" : "=r" (r) : /* null */ )
#define LPC21XX_CPSR_SET(r)	  asm volatile (" msr cpsr, %0" : /* null */ : "r" (r) )
#else
#define LPC21XX_CPSR_GET(r)	  r = 0
#define LPC21XX_CPSR_SET(r)	  UNUSED (r)
#endif

static inline unsigned long lpc21XX_irq_disable (void)
{
		unsigned long r;
		LPC21XX_CPSR_GET (r);
		LPC21XX_CPSR_SET (r | LPC21XX_IRQ_MASK);
		return r;
}

static inline unsigned long lpc21XX_irq_enable (void)
{
		unsigned long r;
		LPC21XX_CPSR_GET (r);
		LPC21XX_CPSR_SET (r & ~LPC21XX_IRQ_MASK);
		return r;
}

static inline unsigned long lpc21XX_irq_restore (const unsigned long v)
{
		unsigned long r;
		LPC21XX_CPSR_GET (r);
		LPC21XX_CPSR_SET ((r & ~LPC21XX_IRQ_MASK) | (v & LPC21XX_IRQ_MASK));
		return r;
}

#define lpc21XX_irq_complete() LPC21XX_IRQ_CLEAR

static inline unsigned long lpc21XX_fiq_disable (void)
{
		unsigned long r;
		LPC21XX_CPSR_GET (r);
		LPC21XX_CPSR_SET (r | LPC21XX_FIQ_MASK);
		return r;
}

#ifdef LPC21XX_ENABLE_FIQ

static inline unsigned long lpc21XX_fiq_enable (void)
{
		unsigned long r;
		LPC21XX_CPSR_GET (r);
		LPC21XX_CPSR_SET (r & ~LPC21XX_FIQ_MASK);
		return r;
}

static inline unsigned long lpc21XX_fiq_restore (const unsigned long v)
{
		unsigned long r;
		LPC21XX_CPSR_GET (r);
		LPC21XX_CPSR_SET ((r & ~LPC21XX_FIQ_MASK) | (v & LPC21XX_FIQ_MASK));
		return r;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

/* add others as required */
#define LPC21XX_IRQ_CHANNEL_WDT		((unsigned char) 0)
#define LPC21XX_IRQ_CHANNEL_TIMER0	((unsigned char) 4)
#define LPC21XX_IRQ_CHANNEL_TIMER1	((unsigned char) 5)
#define LPC21XX_IRQ_CHANNEL_UART0	((unsigned char) 6)
#define LPC21XX_IRQ_CHANNEL_UART1	((unsigned char) 7)
#define LPC21XX_IRQ_CHANNEL_EINT0	((unsigned char) 14)
#define LPC21XX_IRQ_CHANNEL_BOD		((unsigned char) 20)
#define LPC21XX_IRQ_CHANNEL_MAX		((unsigned char) 22)

#define LPC21XX_IRQ_VECTOR_SIZE		((unsigned char) 16)
#define LPC21XX_IRQ_VECTOR_ADDR(o)	(*((volatile unsigned long *) (0xFFFFF100 + (((unsigned long) o) << 2))))
#define LPC21XX_IRQ_VECTOR_CTRL(o)	(*((volatile unsigned long *) (0xFFFFF200 + (((unsigned long) o) << 2))))

static void lpc21XX_irq_vector_attach (const unsigned char irq, const unsigned long handler)
{
		unsigned char offset;

		assert (irq < LPC21XX_IRQ_CHANNEL_MAX);
		assert (handler != 0);

		for (offset = (unsigned char) 0; offset < LPC21XX_IRQ_VECTOR_SIZE; offset += (unsigned char) 1)
		{
				if ((LPC21XX_IRQ_VECTOR_CTRL (offset) & 0x20) == 0x00)
				{
						const unsigned long c = lpc21XX_irq_disable ();

						LPC21XX_IRQ_VECTOR_ADDR (offset) = handler;
						LPC21XX_IRQ_VECTOR_CTRL (offset) = (unsigned long) (0x20 | irq);

						VICIntSelect &= ~(1L << irq); /* set IRQ to be vectored */
						VICIntEnable |= (1L << irq);  /* enable IRQ */

						(void) lpc21XX_irq_restore (c);

						break;
				}
		}

		assert (offset < LPC21XX_IRQ_VECTOR_SIZE);
}

static void lpc21XX_irq_vector_detach (const unsigned char irq)
{
		unsigned char offset;

		assert (irq < LPC21XX_IRQ_CHANNEL_MAX);

		for (offset = (unsigned char) 0; offset < LPC21XX_IRQ_VECTOR_SIZE; offset += (unsigned char) 1)
		{
				if ((unsigned char) (LPC21XX_IRQ_VECTOR_CTRL (offset) & 0x20) != (unsigned char) 0x00 && (unsigned char) (LPC21XX_IRQ_VECTOR_CTRL (offset) & 0x1F) == irq)
				{
						const unsigned long c = lpc21XX_irq_disable ();

						LPC21XX_IRQ_VECTOR_CTRL (offset) = 0x00000000;
						LPC21XX_IRQ_VECTOR_ADDR (offset) = 0x00000000;

						(void) lpc21XX_irq_restore (c);

						break;
				}
		}

		VICIntEnClr = (unsigned long) (1L << irq); /* disable IRQ */
}

/* ----------------------------------------------------------------------------------------------------*/

static void lpc21XX_irq_init (void)
{
		/* interrupt vectors in flash */
		MEMMAP = (unsigned char) (0x01 & MEMMAP_MAP1_0_MASK);

		/* interrupt config: nothing */
		VICIntEnClr	 = (unsigned long) 0xFFFFFFFF;
		VICIntSelect = (unsigned long) 0x00000000;
		VICIntEnable = (unsigned long) 0x00000000;
		VICDefVectAddr = (unsigned long) irq_handler;

		/* irq enabled, fiq disabled */
		(void) lpc21XX_irq_enable ();
		(void) lpc21XX_fiq_disable ();

		/* EXTWAKE */
}

static void lpc21XX_irq_term (void)
{
		/* irq disabled, fiq disabled */
		(void) lpc21XX_irq_disable ();
		(void) lpc21XX_fiq_disable ();

		/* clear default vector address */
		VICDefVectAddr = (unsigned long) 0x00000000;
}

/* ----------------------------------------------------------------------------------------------------*/

#define LPC21XX_FOSC	(14745)
#define LPC21XX_MSEL	(3)
#define LPC21XX_PSEL	(1)
#define LPC21XX_VPBD	(1)

#define LPC21XX_CCLK	(LPC21XX_FOSC * LPC21XX_MSEL)
#define LPC21XX_PCLK	(LPC21XX_CCLK)

boolean lpc21XX_init (const int argc, const char ** const argv)
{
		UNUSED (argc);
		UNUSED (argv);

		DPRINTF (("lpc21XX_init\n"));

		/* setup, enable, waitfor & connect PLL */
		PLLCFG = (unsigned char) (LPC21XX_PSEL<<PLLCFG_PSEL1_0_BIT)|(LPC21XX_MSEL<<PLLCFG_MSEL4_0_BIT);
		PLLFEED = (unsigned char) 0xAA; PLLFEED = (unsigned char) 0x55;
		PLLCON = (unsigned char) PLLCON_PLLE;
		PLLFEED = (unsigned char) 0xAA; PLLFEED = (unsigned char) 0x55;
		while ((PLLSTAT & PLLSTAT_PLOCK) == 0x00)
				/*@i@*/ ;
		PLLCON = (unsigned char) (PLLCON_PLLE|PLLCON_PLLC);
		PLLFEED = (unsigned char) 0xAA; PLLFEED = (unsigned char) 0x55;

		/* enable memory acceleration module (MAM) w/ cclks=4 */
		MAMCR = (unsigned char) (0x02 & MAMCR_MAM_mode_control_MASK); /* MAM fully enabled */
		MAMTIM = (unsigned char) (0x04 & MAMTIM_MAM_Fetch_Cycle_timing_MASK); /* MAM fetch is 4 clock cycles */
		/* XXX increase MAMTIM ? */

		/* config peripheral clock (pclk) to system clock (cclk) */
		VPBDIV = (unsigned char) (LPC21XX_VPBD & VPBDIV_VPBDIV_MASK); /* XXX try 0x02 for half */

		/* configure power control: disable BOD entirely */
		PCON = (unsigned char) 0x1C;

		/* disable all peripherals by default (will be enabled as
		   required by specific software modules) */
		PCONP = (unsigned char) 0x00;

#ifdef HARDWARE_24
		/* XXX what is this doing for V2.4 ? */
		IO0DIR |= 0x00020000;
#endif

		/* initialise interrupt handling */
		lpc21XX_irq_init ();

		/* initialise leds */
		lpc21XX_led_init ();

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean lpc21XX_term (void)
{
		DPRINTF (("lpc21XX_term\n"));

		/* disable irq's */
		(void) lpc21XX_irq_term ();

		/* disable watchdog */
		WDMOD = (unsigned long) 0;
		/* disable powerfail */
		PCON &= ~(1<<4);
		/* disable shutdown */
		EXTINT = (unsigned char) ((1<<EXTINT_EINT0_BIT) & EXTINT_EINT0_MASK);
		EXTMODE &= ~(1<<0);
		/* disable timer0 */
		T0TCR = (unsigned char) 0x00;
		/* disable spi */
		PINSEL0 &= ~((1L<<8)|(1L<<10)|(1L<<12)); /* 3 SPI pins */
		IO0DIR &= ~LPC21XX_SPI_CS;
		/* disable RTC */
		CCR &= (unsigned long) ~CCR_CLKEN;

		/* disable all peripherals */
		PCONP = (unsigned char) 0x00;

		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

boolean lpc21XX_rtc_enabled (void)
{
		return ((PCONP & PCONP_PCRTC) != 0x00) ? TRUE : FALSE;
}

void lpc21XX_rtc_enable (void)
{
		PCONP |= PCONP_PCRTC;

		CCR &= (unsigned long) ~CCR_CLKEN;
		YEAR = (unsigned long) 1980;
		MONTH = (unsigned long) 1;
		DOM = (unsigned long) 1;
		HOUR = (unsigned long) 0;
		MIN = (unsigned long) 0;
		SEC = (unsigned long) 0;
		CCR = (unsigned long) (0x10|CCR_CLKEN); /* CLKSRC=1 (use the pre-scalar) */
}

void lpc21XX_rtc_disable (void)
{
		CCR &= (unsigned long) ~CCR_CLKEN;
		PCONP &= ~PCONP_PCRTC;
}

void lpc21XX_rtc_get (lpc21XX_rtc_t * const c)
{
		assert (c != NULL);

		c->e [LPC21XX_RTC_ELEM_YY] = (unsigned char) (YEAR - 1980);
		c->e [LPC21XX_RTC_ELEM_MO] = (unsigned char) MONTH;
		c->e [LPC21XX_RTC_ELEM_DD] = (unsigned char) DOM;
		c->e [LPC21XX_RTC_ELEM_HH] = (unsigned char) HOUR;
		c->e [LPC21XX_RTC_ELEM_MM] = (unsigned char) MIN;
		c->e [LPC21XX_RTC_ELEM_SS] = (unsigned char) SEC;
}

void lpc21XX_rtc_set (const lpc21XX_rtc_t * const c)
{
		unsigned long occr = CCR;

		assert (c != NULL);
		assert (c->e [LPC21XX_RTC_ELEM_YY] < (unsigned char) 90 && c->e [LPC21XX_RTC_ELEM_MO] <= (unsigned char) 12 && c->e [LPC21XX_RTC_ELEM_DD] <= (unsigned char) 31 &&
				c->e [LPC21XX_RTC_ELEM_HH] < (unsigned char) 24 && c->e [LPC21XX_RTC_ELEM_MM] < (unsigned char) 60 && c->e [LPC21XX_RTC_ELEM_SS] < (unsigned char) 60);

		CCR &= (unsigned long) ~CCR_CLKEN;
		YEAR = (unsigned long) ((int) c->e [LPC21XX_RTC_ELEM_YY] + (int) 1980);
		MONTH = (unsigned long) c->e [LPC21XX_RTC_ELEM_MO];
		DOM = (unsigned long) c->e [LPC21XX_RTC_ELEM_DD];
		HOUR = (unsigned long) c->e [LPC21XX_RTC_ELEM_HH];
		MIN = (unsigned long) c->e [LPC21XX_RTC_ELEM_MM];
		SEC = (unsigned long) c->e [LPC21XX_RTC_ELEM_SS];
		CCR = occr; /* CLKEN restored */
}

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_delay (const unsigned int count)
{
		unsigned int index;

		for (index = 0; index < count; index++)
		{
				LPC21XX_NOP ();
				LPC21XX_NOP ();
				LPC21XX_NOP ();
				LPC21XX_NOP ();
				LPC21XX_NOP ();
		}
}

/* ----------------------------------------------------------------------------------------------------*/

#ifndef INLINE
#ifndef SPLINT
#define LPC21XX_POWER_IDLE		do { PCON |= PCON_IDL; } while (FALSE)
#define LPC21XX_POWER_DOWN		do { PCON |= PCON_PD; } while (FALSE)
#else
#define LPC21XX_POWER_IDLE		/*@i@*/ do { } while (FALSE)
#define LPC21XX_POWER_DOWN		/*@i@*/ do { } while (FALSE)
#endif
#endif

#ifndef INLINE
INLINE_DECL void lpc21XX_idle (void)
{
		LPC21XX_POWER_IDLE;
}
#endif

void lpc21XX_down (void)
{
		unsigned int led; 

		/* leds: all off for a "normal" shutdown */
		for (led = (unsigned int) 0; led < (unsigned int) lpc21XX_led_size (); led++)
		{
				lpc21XX_led_disable ((unsigned char) led);
		}

		lpc21XX_delay ((unsigned int) 64);

		(void) lpc21XX_term ();

		/*@i@*/ while (TRUE)
		{
				LPC21XX_POWER_DOWN;
		}
}

void lpc21XX_halt (void)
{
		unsigned int led; 

		/* leds: all on for a "fault" shutdown */
		for (led = (unsigned int) 0; led < (unsigned int) lpc21XX_led_size (); led++)
		{
				lpc21XX_led_enable ((unsigned char) led);
		}

		lpc21XX_delay ((unsigned int) 64);

		(void) lpc21XX_term ();

		/*@i@*/ while (TRUE)
		{
				LPC21XX_POWER_DOWN;
		}
}

void lpc21XX_reset (void)
{
		unsigned int led; 

		/* leds: all off for a "normal" shutdown */
		for (led = (unsigned int) 0; led < (unsigned int) lpc21XX_led_size (); led++)
		{
				lpc21XX_led_disable ((unsigned char) led);
		}

		lpc21XX_delay ((unsigned int) 64);

		(void) lpc21XX_term ();

		WDMOD = (unsigned long) WDMOD_WDRESET;
		WDTC = (unsigned long) LPC21XX_PCLK; /* one CLOCK */
		WDFEED = (unsigned long) 0xAA;
		WDFEED = (unsigned long) 0x55;
}

/* ----------------------------------------------------------------------------------------------------*/

#define LPC21XX_LED_TOGGLE_COUNT  ((unsigned char) 4)
#define LPC21XX_LED_TOGGLE_DELAY  ((unsigned int) (128*1024))

static const unsigned long lpc21XX_led_cfg_list [] =
{
		(unsigned long) 0x00000004, /* v1.0: stat0 (red), v2.4: stat (red) */
#ifdef HARDWARE_10
		(unsigned long) 0x00000400	/* v1.0: stat1 (red) */
#endif
#ifdef HARDWARE_24
		(unsigned long) 0x00000400, /* v2.4: stat (green) */
		(unsigned long) 0x00002000	/* v2.4: stat (blue) */
#endif

};
#define lpc21XX_led_cfg_size ((unsigned char) (sizeof (lpc21XX_led_cfg_list) / sizeof (unsigned long)))
#ifdef HARDWARE_24
#define lpc21XX_led_cfg_mask ((unsigned long) 0x00002404)
#else
#define lpc21XX_led_cfg_mask ((unsigned long) 0x00000404)
#endif

void lpc21XX_led_enable (const unsigned char led)
{
		assert (led <= lpc21XX_led_cfg_size);

		IO0CLR = lpc21XX_led_cfg_list [led];
}

void lpc21XX_led_disable (const unsigned char led)
{
		assert (led <= lpc21XX_led_cfg_size);

		IO0SET = lpc21XX_led_cfg_list [led];
}

void lpc21XX_led_toggle (const unsigned char led)
{
		assert (led <= lpc21XX_led_cfg_size);

		if ((IO0PIN & lpc21XX_led_cfg_list [led]) == 0x00)
		{
				IO0SET = lpc21XX_led_cfg_list [led];
		}
		else
		{
				IO0CLR = lpc21XX_led_cfg_list [led];
		}
}

unsigned char lpc21XX_led_size (void)
{
		return lpc21XX_led_cfg_size;
}

void lpc21XX_led_init (void)
{
		unsigned int cnt, led;
		unsigned int dly;

		IO0DIR |= lpc21XX_led_cfg_mask;
		IO0SET = lpc21XX_led_cfg_mask;

		for (cnt = (unsigned int) 0; cnt < (unsigned int) LPC21XX_LED_TOGGLE_COUNT; cnt++)
		{
				for (led = (unsigned int) 0; led < (unsigned int) lpc21XX_led_cfg_size; led++)
				{
						IO0CLR = lpc21XX_led_cfg_list [led];
						for (dly = (unsigned int) 0; dly < LPC21XX_LED_TOGGLE_DELAY; dly++)
								/*@i@*/ ;
						IO0SET = lpc21XX_led_cfg_list [led];
				}
		}
}

/* ----------------------------------------------------------------------------------------------------*/

#define LPC21XX_UART_BUFFER_SIZE ((unsigned int) 512)	   /* must be a power of 2 !! */

typedef struct
{
		unsigned char data [LPC21XX_UART_BUFFER_SIZE];
		volatile unsigned int begin, end;
}
lpc21XX_uart_buffer_t;

static inline boolean lpc21XX_uart_buffer_empty (/*@notnull@*/ lpc21XX_uart_buffer_t * const bp)
{
		return (bp->begin == bp->end) ? TRUE : FALSE;
}

static inline void lpc21XX_uart_buffer_init (/*@notnull@*/ lpc21XX_uart_buffer_t * const bp)
{
		bp->end = bp->begin = 0;
}

static inline void lpc21XX_uart_buffer_enqueue (/*@notnull@*/ lpc21XX_uart_buffer_t * const bp, const unsigned char ch)
{
		const unsigned int en = (bp->end + (unsigned int) 1) & (LPC21XX_UART_BUFFER_SIZE - (unsigned int) 1);
		if (bp->begin != en) {
				bp->data [bp->end] = ch;
				bp->end = en;
		}
}

static inline unsigned char lpc21XX_uart_buffer_dequeue (/*@notnull@*/ lpc21XX_uart_buffer_t * const bp)
{
		const unsigned char ch = bp->data [bp->begin];
		if (bp->begin != bp->end) {
				bp->begin = (bp->begin + (unsigned int) 1) & (LPC21XX_UART_BUFFER_SIZE - (unsigned int) 1);
		} /* otherwise ch undefined! */
		return ch;
}

/* ----------------------------------------------------------------------------------------------------*/

typedef struct
{
		unsigned int rate;
		unsigned char dlm;
		unsigned char dll;
}
lpc21XX_uart_baud_cfg_t;

static const lpc21XX_uart_baud_cfg_t lpc21XX_uart_baud_cfg_list [] =
{
		{ (unsigned int)  1200, (unsigned char) 0x0C, (unsigned char) 0x00 },
		{ (unsigned int)  2400, (unsigned char) 0x06, (unsigned char) 0x00 },
		{ (unsigned int)  4800, (unsigned char) 0x03, (unsigned char) 0x00 },
		{ (unsigned int)  9600, (unsigned char) 0x01, (unsigned char) 0x80 },
		{ (unsigned int) 19200, (unsigned char) 0x00, (unsigned char) 0xC0 },
		{ (unsigned int) 38400, (unsigned char) 0x00, (unsigned char) 0x60 },
		{ (unsigned int) 57600, (unsigned char) 0x00, (unsigned char) 0x40 },
		{ (unsigned int)115200, (unsigned char) 0x00, (unsigned char) 0x20 }
};
#define lpc21XX_uart_baud_cfg_size ((unsigned char) (sizeof (lpc21XX_uart_baud_cfg_list) / sizeof (lpc21XX_uart_baud_cfg_t)))

/*@null@*/ /*@observer@*/ static const lpc21XX_uart_baud_cfg_t * lpc21XX_uart_baud_cfg_get (const unsigned int rate)
{
		unsigned int index;

		for (index = (unsigned int) 0; index < (unsigned int) lpc21XX_uart_baud_cfg_size; index++)
		{
				if (lpc21XX_uart_baud_cfg_list [index].rate == rate)
				{
						return &lpc21XX_uart_baud_cfg_list [index];
				}
		}

		return NULL;
}

boolean lpc21XX_uart_baud_supported (const unsigned int baud_rate)
{
		return lpc21XX_uart_baud_cfg_get (baud_rate) != NULL ? TRUE : FALSE;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_uart0_irq_enabled;
static lpc21XX_uart_buffer_t lpc21XX_uart0_irq_buffer_rx;

static void __attribute__ ((interrupt ("IRQ"))) lpc21XX_uart0_irq_handler (void)
{
		while ((U0LSR & U0LSR_RDR) != (unsigned char) 0x00)
		{
				lpc21XX_uart_buffer_enqueue (&lpc21XX_uart0_irq_buffer_rx, (unsigned char) (U0RBR & 0xFF));
		}

		/*@i@*/ U0IIR;

		lpc21XX_irq_complete ();
}

void lpc21XX_uart0_init (const unsigned int baud_rate, const boolean enable_interrupts)
{
		const lpc21XX_uart_baud_cfg_t * const uart_cfg = lpc21XX_uart_baud_cfg_get (baud_rate);

		assert (uart_cfg != NULL);
		assert (baud_rate > (unsigned int) 0);

		PCONP |= PCONP_PCURT0;
		PINSEL0 |= 0x00000005; /* UART0 TXD/RXD */

		/* 8 bits, no parity, 1 stop-bit */
		U0LCR = (unsigned char) (U0LCR_Divisor_Latch_Access_Bit|(0x03 & U0LCR_Word_Length_Select_MASK));
		U0DLM = uart_cfg->dlm;
		U0DLL = uart_cfg->dll;
		U0LCR &= ~U0LCR_Divisor_Latch_Access_Bit;
		U0FCR = (unsigned char) U0FCR_FIFO_Enable;

		/* flush interrupts, rx fifo and status register */
		/*@i@*/ U0IIR;
		/*@i@*/ U0RBR;
		/*@i@*/ U0LSR;

		/* configure interrupt handling */
		lpc21XX_uart0_irq_enabled = enable_interrupts;
		if (lpc21XX_uart0_irq_enabled)
		{
				lpc21XX_uart_buffer_init (&lpc21XX_uart0_irq_buffer_rx);
				lpc21XX_irq_vector_attach (LPC21XX_IRQ_CHANNEL_UART0, (unsigned long) lpc21XX_uart0_irq_handler);
				U0IER = (unsigned char) U1IER_RBR_Interrupt_Enable;
		}
		else
		{
				U0IER = (unsigned char) 0x00;
		}
}

void lpc21XX_uart0_term (void)
{
		/* configure interrupt handling */
		if (lpc21XX_uart0_irq_enabled)
		{
				lpc21XX_irq_vector_detach (LPC21XX_IRQ_CHANNEL_UART0);
				U0IER = (unsigned char) 0x00;
		}

		/* flush interrupts, rx fifo and status register */
		/*@i@*/ U0IIR;
		/*@i@*/ U0RBR;
		/*@i@*/ U0LSR;

		PINSEL0 &= ~0x00000005; /* UART0 TXD/RXD */
		PCONP &= ~PCONP_PCURT0;
}

void lpc21XX_uart0_set_fifo_level (const unsigned char value)
{
		U0FCR = ((value << U0FCR_Rx_Trigger_Level_Select_BIT) & U0FCR_Rx_Trigger_Level_Select_MASK) \
				| U0FCR_FIFO_Enable;
}

void lpc21XX_uart0_putc (const unsigned char ch)
{
		while ((U0LSR & U0LSR_THRE) == (unsigned char) 0x00)
				/*@i@*/ ;
		U0THR = ch;
}

unsigned char lpc21XX_uart0_getc (void)
{
		if (lpc21XX_uart0_irq_enabled)
		{
				/*@i@*/ while (lpc21XX_uart_buffer_empty (&lpc21XX_uart0_irq_buffer_rx))
						LPC21XX_POWER_IDLE;
				return lpc21XX_uart_buffer_dequeue (&lpc21XX_uart0_irq_buffer_rx);
		}
		else
		{
				while ((U0LSR & U0LSR_RDR) == (unsigned char) 0x00)
						/*@i@*/ ;
				return (U0RBR);
		}
}

void lpc21XX_uart0_write (const unsigned char * const buffer, const unsigned int length)
{
		unsigned int count = 0;
		while (count < length)
		{
				while ((U0LSR & U0LSR_THRE) == (unsigned char) 0x00)
						/*@i@*/ ;
				U0THR = buffer [count++];
		}
}

unsigned int lpc21XX_uart0_gets (unsigned char * const buffer, const unsigned int length)
{
		unsigned int count = 0;
		while (count < length)
		{
				if (lpc21XX_uart0_irq_enabled)
				{
						/*@i@*/ while (lpc21XX_uart_buffer_empty (&lpc21XX_uart0_irq_buffer_rx))
								LPC21XX_POWER_IDLE;
						if ((buffer [count++] = lpc21XX_uart_buffer_dequeue (&lpc21XX_uart0_irq_buffer_rx)) == (unsigned char) '\n')
								break;
				}
				else
				{
						while ((U0LSR & U0LSR_RDR) == (unsigned char) 0x00)
								/*@i@*/ ;
						if ((buffer [count++] = U0RBR) == (unsigned char) '\n')
								break;
				}
		}
		return count;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean lpc21XX_uart1_irq_enabled;
static lpc21XX_uart_buffer_t lpc21XX_uart1_irq_buffer_rx;

static void __attribute__ ((interrupt ("IRQ"))) lpc21XX_uart1_irq_handler (void)
{
		while ((U1LSR & U1LSR_RDR) != (unsigned char) 0x00)
		{
				lpc21XX_uart_buffer_enqueue (&lpc21XX_uart1_irq_buffer_rx, (unsigned char) (U1RBR & 0xFF));
		}

		/*@i@*/ U1IIR;

		lpc21XX_irq_complete ();
}

void lpc21XX_uart1_init (const unsigned int baud_rate, const boolean enable_interrupts)
{
		const lpc21XX_uart_baud_cfg_t * const uart_cfg = lpc21XX_uart_baud_cfg_get (baud_rate);

		assert (uart_cfg != NULL);
		assert (baud_rate > (unsigned int) 0);

		PCONP |= PCONP_PCURT1;
		PINSEL0 |= 0x00050000; /* UART1 TXD/RXD */

		/* 8 bits, no parity, 1 stop-bit */
		U1LCR = (unsigned char) (U1LCR_Divisor_Latch_Access_Bit|(0x03 & U1LCR_Word_Length_Select_MASK));
		U1DLM = uart_cfg->dlm;
		U1DLL = uart_cfg->dll;
		U1LCR &= ~U1LCR_Divisor_Latch_Access_Bit;
		U1FCR = (unsigned char) U1FCR_FIFO_Enable;
		U1MCR = (unsigned char) 0x00;

		/* flush interrupts, rx fifo and status register */
		/*@i@*/ U1IIR;
		/*@i@*/ U1RBR;
		/*@i@*/ U1LSR;

		/* configure interrupt handling */
		lpc21XX_uart1_irq_enabled = enable_interrupts;
		if (lpc21XX_uart1_irq_enabled)
		{
				lpc21XX_uart_buffer_init (&lpc21XX_uart1_irq_buffer_rx);
				lpc21XX_irq_vector_attach (LPC21XX_IRQ_CHANNEL_UART1, (unsigned long) lpc21XX_uart1_irq_handler);
				U1IER = (unsigned char) U1IER_RBR_Interrupt_Enable;
		}
		else
		{
				U1IER = (unsigned char) 0x00;
		}
}

void lpc21XX_uart1_term (void)
{
		/* configure interrupt handling */
		if (lpc21XX_uart1_irq_enabled)
		{
				lpc21XX_irq_vector_detach (LPC21XX_IRQ_CHANNEL_UART1);
				U1IER = (unsigned char) 0x00;
		}

		/* flush interrupts, rx fifo and status register */
		/*@i@*/ U1IIR;
		/*@i@*/ U1RBR;
		/*@i@*/ U1LSR;

		PINSEL0 &= ~0x00050000; /* UART1 TXD/RXD */
		PCONP &= ~PCONP_PCURT1;
}

void lpc21XX_uart1_set_fifo_level (const unsigned char value)
{
		U1FCR = ((value << U1FCR_Rx_Trigger_Level_Select_BIT) & U1FCR_Rx_Trigger_Level_Select_MASK) \
				| U1FCR_FIFO_Enable;
}

void lpc21XX_uart1_putc (const unsigned char ch)
{
		while ((U1LSR & U1LSR_THRE) != (unsigned char) 0x00)
				/*@i@*/ ;
		U1THR = ch;
}

unsigned char lpc21XX_uart1_getc (void)
{
		if (lpc21XX_uart1_irq_enabled)
		{
				/*@i@*/ while (lpc21XX_uart_buffer_empty (&lpc21XX_uart1_irq_buffer_rx))
						LPC21XX_POWER_IDLE;
				return lpc21XX_uart_buffer_dequeue (&lpc21XX_uart1_irq_buffer_rx);
		}
		else
		{
				while ((U1LSR & U1LSR_RDR) != (unsigned char) 0x00)
						/*@i@*/ ;
				return (U1RBR);
		}
}

void lpc21XX_uart1_write (const unsigned char * const buffer, const unsigned int length)
{
		unsigned int count = 0;
		while (count < length)
		{
				while ((U1LSR & U1LSR_THRE) == (unsigned char) 0x00)
						/*@i@*/ ;
				U1THR = buffer [count++];
		}
}

unsigned int lpc21XX_uart1_gets (unsigned char * const buffer, const unsigned int length)
{
		unsigned int count = 0;
		while (count < length)
		{
				if (lpc21XX_uart1_irq_enabled)
				{
						/*@i@*/ while (lpc21XX_uart_buffer_empty (&lpc21XX_uart1_irq_buffer_rx))
								LPC21XX_POWER_IDLE;
						if ((buffer [count++] = lpc21XX_uart_buffer_dequeue (&lpc21XX_uart1_irq_buffer_rx)) == (unsigned char) '\n')
								break;
				}
				else
				{
						while ((U1LSR & U1LSR_RDR) == (unsigned char) 0x00)
								/*@i@*/ ;
						if ((buffer [count++] = U1RBR) == (unsigned char) '\n')
								break;
				}
		}
		return count;
}

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_spi_init (void)
{
		unsigned int i;

		/* XXX: should we PCONP be after we setup the pins? */
		PCONP |= PCONP_PCSPIO;

		IO0DIR |= LPC21XX_SPI_CS;
		PINSEL0 |= (1L<<8)|(1L<<10)|(1L<<12); /* 3 SPI pins */
		IO0SET |= LPC21XX_SPI_CS;

		/* XXX: for V2.4, this changes to 0x3C */
		SPCCR = (unsigned char) 0x20; /* clock = pclk/32 */
		SPCR = (unsigned char) (SPCR_CPHA|SPCR_CPOL|SPCR_MSTR);

		for (i = (unsigned int) 0; i < (unsigned int) 16; i++)
		{
				/*@i@*/ SPDR;
		}
}

void lpc21XX_spi_term (void)
{
		PCONP &= ~PCONP_PCSPIO;

		PINSEL0 &= ~((1L<<8)|(1L<<10)|(1L<<12)); /* 3 SPI pins */
		IO0DIR &= ~LPC21XX_SPI_CS;
}

#ifndef INLINE
INLINE_DECL void lpc21XX_spi_put (const unsigned char ch)
{
		SPDR = ch;
		while ((SPSR & SPSR_SPIF) == (unsigned char) 0x00)
				/*@i@*/ ;
}
#endif

#ifndef INLINE
INLINE_DECL unsigned char lpc21XX_spi_get (void)
{
		SPDR = (unsigned char) 0xFF;
		while ((SPSR & SPSR_SPIF) == (unsigned char) 0x00)
				/*@i@*/ ;
		return SPDR;
}
#endif

#ifndef INLINE
INLINE_DECL void lpc21XX_spi_acquire (void)
{
		IO0CLR |= LPC21XX_SPI_CS;
}
#endif

#ifndef INLINE
INLINE_DECL void lpc21XX_spi_release (void)
{
		IO0SET |= LPC21XX_SPI_CS;
}
#endif

/* ----------------------------------------------------------------------------------------------------*/

/*@null@*/ static lpc21XX_timer_callback_t lpc21XX_timer0_callback;

static void __attribute__ ((interrupt ("IRQ"))) lpc21XX_timer0_irq_handler (void)
{
		if ((T0IR & T0IR_MR0) != (unsigned long) 0)
		{
				if (lpc21XX_timer0_callback != NULL)
				{
						(*lpc21XX_timer0_callback) ();
				}

				T0IR = (unsigned long) T0IR_MR0;
		}

		lpc21XX_irq_complete ();
}

void lpc21XX_timer0_init (const lpc21XX_timer_callback_t callback)
{
		assert (callback != NULL);

		PCONP |= PCONP_PCTIM0;

		lpc21XX_timer0_callback = callback;
		lpc21XX_irq_vector_attach (LPC21XX_IRQ_CHANNEL_TIMER0, (unsigned long) lpc21XX_timer0_irq_handler);

		T0TC = (unsigned long) 0x00000000; /* current timer-counter, incremented when prescale-counter wraps */
		T0PC = (unsigned long) 0x00000000; /* current prescale-counter, incremented each pclk */
		T0PR = (unsigned long) LPC21XX_CCLK; /* threshold prescale-counter (1/1000'th of a second) */
		T0MR0 = (unsigned long) 0x00000000; /* threshold timer-counter */
		T0MCR = (unsigned long) (T0MCR_Interrupt_on_MR0|T0MCR_Reset_on_MR0|T0MCR_Stop_on_MR0);
		T0CTCR = (unsigned long) (0x00 & T0CTCR_Mode_MASK); /* timer mode */
		T0TCR = (unsigned long) 0x00;
}

void lpc21XX_timer0_term (void)
{
		lpc21XX_irq_vector_detach (LPC21XX_IRQ_CHANNEL_TIMER0);
		lpc21XX_timer0_callback = NULL;

		T0TCR = (unsigned char) 0x00;

		PCONP &= ~PCONP_PCTIM0;
}

void lpc21XX_timer0_start (const unsigned long secs)
{
		assert (secs > (unsigned long) 0);
		assert (lpc21XX_timer0_callback != NULL);

		T0MR0 = (unsigned long) (secs * 1000); /* threshold timer-counter */
		T0TCR = (unsigned long) T0TCR_Counter_Reset;
		LPC21XX_NOP (); /* at least one clock cycle (shouldn't be needed really ...) */
		T0TCR = (unsigned long) T0TCR_Counter_Enable;
}

void lpc21XX_timer0_stop (void)
{
		T0TCR = (unsigned long) 0;
}

#ifndef INLINE
INLINE_DECL void lpc21XX_timer0_suspend (void)
{
		T0TCR = (unsigned long) 0;
}
#endif

#ifndef INLINE
INLINE_DECL void lpc21XX_timer0_resume (void)
{
		T0TCR = (unsigned long) T0TCR_Counter_Enable;
}
#endif

/* ----------------------------------------------------------------------------------------------------*/

/*@null@*/ static lpc21XX_powerfail_callback_t lpc21XX_powerfail_callback;

static void __attribute__ ((interrupt ("IRQ"))) lpc21XX_powerfail_irq_handler (void)
{
		if (lpc21XX_powerfail_callback != NULL)
		{
				(*lpc21XX_powerfail_callback) ();
		}
		lpc21XX_irq_complete ();
}

void lpc21XX_powerfail_enable (const lpc21XX_powerfail_callback_t callback)
{
		assert (callback != NULL);

		lpc21XX_powerfail_callback = callback;
		lpc21XX_irq_vector_attach (LPC21XX_IRQ_CHANNEL_BOD, (unsigned long) lpc21XX_powerfail_irq_handler);

		PCON &= ~(1<<4);
}

void lpc21XX_powerfail_disable (void)
{
		PCON |= (1<<4);

		lpc21XX_irq_vector_detach (LPC21XX_IRQ_CHANNEL_BOD);
		lpc21XX_powerfail_callback = NULL;
}

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_watchdog_enable (const unsigned long secs)
{
		assert (secs > (unsigned long) 0);

		WDMOD = (unsigned long) WDMOD_WDRESET;
		WDTC = (unsigned long) (LPC21XX_PCLK * 1000 / 4) * (unsigned long) secs;
		WDFEED = (unsigned long) 0xAA;
		WDFEED = (unsigned long) 0x55;
}

void lpc21XX_watchdog_disable (void)
{
		WDMOD = (unsigned long) 0;
}

#ifndef INLINE
INLINE_DECL void lpc21XX_watchdog_kick (void)
{
		WDFEED = (unsigned long) 0xAA;
		WDFEED = (unsigned long) 0x55;
}
#endif

/* ----------------------------------------------------------------------------------------------------*/

/*@null@*/ static lpc21XX_shutdown_callback_t lpc21XX_shutdown_callback;

#ifdef HARDWARE_24
static void __attribute__ ((interrupt ("IRQ"))) lpc21XX_shutdown_irq_handler (void)
{
		lpc21XX_shutdown_trigger ();
		EXTINT = (unsigned char) ((1<<EXTINT_EINT0_BIT) & EXTINT_EINT0_MASK);
		lpc21XX_irq_complete ();
}
#endif

void lpc21XX_shutdown_trigger (void)
{
		if (lpc21XX_shutdown_callback != NULL)
		{
				(*lpc21XX_shutdown_callback) ();
		}
}

void lpc21XX_shutdown_enable (const lpc21XX_shutdown_callback_t callback)
{
		assert (callback != NULL);

		lpc21XX_shutdown_callback = callback;

#ifdef HARDWARE_24
		PINSEL1 |= (1L<<0);
		IO0DIR &= ~(1L<<16);

		EXTINT = (unsigned char) ((1<<EXTINT_EINT0_BIT) & EXTINT_EINT0_MASK);
		EXTMODE |= (1<<0);
		EXTPOLAR &= ~(1<<0);

		lpc21XX_irq_vector_attach (LPC21XX_IRQ_CHANNEL_EINT0, (unsigned long) lpc21XX_shutdown_irq_handler);
#endif
}

void lpc21XX_shutdown_disable (void)
{
#ifdef HARDWARE_24
		lpc21XX_irq_vector_detach (LPC21XX_IRQ_CHANNEL_EINT0);

		EXTINT = (unsigned char) ((1<<EXTINT_EINT0_BIT) & EXTINT_EINT0_MASK);
		EXTMODE &= ~(1<<0);

		PINSEL1 &= ~(1L<<0);
#endif

		lpc21XX_shutdown_callback = NULL;
}

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_gpshw_enable (void)
{
#ifdef HARDWARE_24
		IO0DIR |= 0x40000000;
		IO0CLR |= 0x40000000;
#endif
}

void lpc21XX_gpshw_disable (void)
{
#ifdef HARDWARE_24
		IO0DIR &= ~0x40000000;
#endif
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED

static volatile unsigned int lpc21XX_test_trigger;
static void lpc21XX_test_handler1 (void)
{
		lpc21XX_test_trigger++;
}

test_result_t lpc21XX_test (void)
{
		/* lpc21XX irq_control */
		(void) lpc21XX_irq_restore (lpc21XX_irq_disable ());
		(void) lpc21XX_irq_restore (lpc21XX_irq_enable ());
#ifdef LPC21XX_ENABLE_FIQ
		(void) lpc21XX_fiq_restore (lpc21XX_fiq_disable ());
		(void) lpc21XX_fiq_restore (lpc21XX_fiq_enable ());
#endif

		/* lpc21XX irq_vector */
		lpc21XX_test_trigger = 0;
		lpc21XX_irq_vector_attach (LPC21XX_IRQ_CHANNEL_WDT, (unsigned long) lpc21XX_test_handler1);
		lpc21XX_irq_vector_detach (LPC21XX_IRQ_CHANNEL_WDT);
		test_assert (lpc21XX_test_trigger == 0);

		/* lpc21XX rtc */
		lpc21XX_rtc_enable ();
		test_assert (lpc21XX_rtc_enabled ());
		{ lpc21XX_rtc_t c = { { (unsigned char) (2006 - 1980), (unsigned char) 9, (unsigned char) 22,
								(unsigned char) 8, (unsigned char) 52, (unsigned char) 5 } };
		  lpc21XX_rtc_set (&c);
		}
		{ lpc21XX_rtc_t c;
		  lpc21XX_rtc_get (&c);
		  test_assert (c.e [LPC21XX_RTC_ELEM_YY] == (unsigned char) (2006 - 1980) && c.e [LPC21XX_RTC_ELEM_MO] == (unsigned char) 9 &&
					   c.e [LPC21XX_RTC_ELEM_DD] == (unsigned char) 22 && c.e [LPC21XX_RTC_ELEM_HH] == (unsigned char) 8 &&
					   c.e [LPC21XX_RTC_ELEM_MM] == (unsigned char) 52 && c.e [LPC21XX_RTC_ELEM_SS] >= (unsigned char) 5);
		}
		lpc21XX_rtc_disable ();
		test_assert (!lpc21XX_rtc_enabled ());

		/* lpc21XX delay */
		lpc21XX_delay ((unsigned int) 512);

		/* lpc21XX idle */

		/* lpc21XX down */

		/* lpc21XX halt */

		/* lpc21XX led */
		test_assert (lpc21XX_led_size () > (unsigned char) 0);
		lpc21XX_led_enable ((unsigned char) 0);
		lpc21XX_led_disable ((unsigned char) 0);

		/* lpc21XX uart */

		/* lpc21XX spi */

#ifdef XXX /*XXX: we need to get the timer0 handler test working again*/
		/* lpc21XX timer0 */
		lpc21XX_test_trigger = 0;
		lpc21XX_timer0_start (1, lpc21XX_test_handler2, NULL);
		DPRINTF (("."));
		while (lpc21XX_test_trigger == 0)
				;
		lpc21XX_test_trigger = 0;
		lpc21XX_timer0_restart ();
		while (lpc21XX_test_trigger == 0)
				;
		lpc21XX_timer0_stop ();
		/* */
		lpc21XX_test_trigger = 0;
		lpc21XX_timer1_start (1, lpc21XX_test_handler2, NULL);
		DPRINTF (("."));
		while (lpc21XX_test_trigger == 0)
				;
		lpc21XX_test_trigger = 0;
		lpc21XX_timer1_restart ();
		while (lpc21XX_test_trigger == 0)
				;
		lpc21XX_timer1_stop ();
#endif

		/* lpc21XX powerfail */

		/* lpc21XX shutdown */

		/* lpc21XX gpshw */

		/* lpc21XX watchdog */
		lpc21XX_watchdog_enable ((unsigned long) 30);
		lpc21XX_watchdog_kick ();
		lpc21XX_watchdog_disable ();

		return TEST_RESULT_OKAY;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/
