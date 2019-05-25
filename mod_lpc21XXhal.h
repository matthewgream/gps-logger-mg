
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_lpc21XXhal.h: gps logger module for LPC21XX hardware interface.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef MOD_LPC21XXHAL_H_
#define MOD_LPC21XXHAL_H_

/*@-exportlocal@*/

/* ----------------------------------------------------------------------------------------------------*/

boolean lpc21XX_init (const int argc, /*@notnull@*/ const char ** const argv);
boolean lpc21XX_term (void);

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_delay (const unsigned int count);

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_idle (void);
/*@noreturn@*/ void lpc21XX_down (void) __attribute__ ((noreturn));
/*@noreturn@*/ void lpc21XX_halt (void) __attribute__ ((noreturn));
/*@noreturn@*/ void lpc21XX_reset (void) __attribute__ ((noreturn));

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_led_init (void);
unsigned char lpc21XX_led_size (void);
void lpc21XX_led_enable (const unsigned char led);
void lpc21XX_led_disable (const unsigned char led);
void lpc21XX_led_toggle (const unsigned char led);

/* ----------------------------------------------------------------------------------------------------*/

#define LPC21XX_UART_FIFO_LEVEL_NONE  ((unsigned char) 0)
#define LPC21XX_UART_FIFO_LEVEL_MIN	  ((unsigned char) 1)
#define LPC21XX_UART_FIFO_LEVLE_MED	  ((unsigned char) 2)
#define LPC21XX_UART_FIFO_LEVEL_MAX	  ((unsigned char) 3)

boolean lpc21XX_uart_baud_supported (const unsigned int baud_rate);

void lpc21XX_uart0_init (const unsigned int baud_rate, const boolean enable_interrupts);
void lpc21XX_uart0_term (void);
void lpc21XX_uart0_set_fifo_level (const unsigned char value);
unsigned char lpc21XX_uart0_getc (void);
unsigned int lpc21XX_uart0_gets (/*@notnull@*/ /*@out@*/ unsigned char * const buffer, const unsigned int length);
void lpc21XX_uart0_putc (const unsigned char ch);
void lpc21XX_uart0_write (/*@notnull@*/ const unsigned char * const buffer, const unsigned int length);

void lpc21XX_uart1_init (const unsigned int baud_rate, const boolean enable_interrupts);
void lpc21XX_uart1_term (void);
void lpc21XX_uart1_set_fifo_level (const unsigned char value);
unsigned char lpc21XX_uart1_getc (void);
unsigned int lpc21XX_uart1_gets (/*@notnull@*/ /*@out@*/ unsigned char * const buffer, const unsigned int length);
void lpc21XX_uart1_putc (const unsigned char ch);
void lpc21XX_uart1_write (/*@notnull@*/ const unsigned char * const buffer, const unsigned int length);

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_spi_init (void);
void lpc21XX_spi_term (void);
void lpc21XX_spi_acquire (void);
void lpc21XX_spi_release (void);
unsigned char lpc21XX_spi_get (void);
void lpc21XX_spi_put (const unsigned char ch);

/* ----------------------------------------------------------------------------------------------------*/

typedef void (*lpc21XX_timer_callback_t) (void);

void lpc21XX_timer0_init (/*@notnull@*/ const lpc21XX_timer_callback_t callback);
void lpc21XX_timer0_term (void);
void lpc21XX_timer0_suspend (void);
void lpc21XX_timer0_resume (void);
void lpc21XX_timer0_start (const unsigned long secs);
void lpc21XX_timer0_stop (void);

/* ----------------------------------------------------------------------------------------------------*/

#define LPC21XX_RTC_ELEM_YY ((unsigned int) 0)
#define LPC21XX_RTC_ELEM_MO ((unsigned int) 1)
#define LPC21XX_RTC_ELEM_DD ((unsigned int) 2)
#define LPC21XX_RTC_ELEM_HH ((unsigned int) 3)
#define LPC21XX_RTC_ELEM_MM ((unsigned int) 4)
#define LPC21XX_RTC_ELEM_SS ((unsigned int) 5)
#define LPC21XX_RTC_ELEM_XX ((unsigned int) 6)

typedef struct {
		unsigned char e [LPC21XX_RTC_ELEM_XX];
} lpc21XX_rtc_t;

boolean lpc21XX_rtc_enabled (void);
void lpc21XX_rtc_enable (void);
void lpc21XX_rtc_disable (void);
void lpc21XX_rtc_get (/*@notnull@*/ /*@out@*/ lpc21XX_rtc_t * const c);
void lpc21XX_rtc_set (/*@notnull@*/ const lpc21XX_rtc_t * const c);

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_watchdog_enable (const unsigned long secs);
void lpc21XX_watchdog_disable (void);
void lpc21XX_watchdog_kick (void);

/* ----------------------------------------------------------------------------------------------------*/

typedef void (*lpc21XX_powerfail_callback_t) (void);

void lpc21XX_powerfail_enable (/*@notnull@*/ const lpc21XX_powerfail_callback_t callback);
void lpc21XX_powerfail_disable (void);

/* ----------------------------------------------------------------------------------------------------*/

typedef void (*lpc21XX_shutdown_callback_t) (void);

void lpc21XX_shutdown_enable (/*@notnull@*/ const lpc21XX_shutdown_callback_t callback);
void lpc21XX_shutdown_disable (void);
void lpc21XX_shutdown_trigger (void);

/* ----------------------------------------------------------------------------------------------------*/

void lpc21XX_gpshw_enable (void);
void lpc21XX_gpshw_disable (void);

/* ----------------------------------------------------------------------------------------------------*/

#ifdef TEST_ENABLED
test_result_t lpc21XX_test (void);
#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifndef EMULATE
#ifdef INLINE
#include "mod_lpc21XXhal.i"
#endif
#endif

/* ----------------------------------------------------------------------------------------------------*/

#endif /*MOD_LPC21XXHAL_H_*/

