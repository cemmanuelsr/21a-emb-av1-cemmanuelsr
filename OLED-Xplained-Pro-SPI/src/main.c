#include <asf.h>
#include "string.h"

#include "oled/gfx_mono_ug_2832hsweg04.h"
#include "oled/gfx_mono_text.h"
#include "oled/sysfont.h"

#define LED_1_PIO PIOA
#define LED_1_PIO_ID ID_PIOA
#define LED_1_IDX 0
#define LED_1_IDX_MASK (1 << LED_1_IDX)

#define LED_2_PIO PIOC
#define LED_2_PIO_ID ID_PIOC
#define LED_2_IDX 30
#define LED_2_IDX_MASK (1 << LED_2_IDX)

#define LED_3_PIO PIOB
#define LED_3_PIO_ID ID_PIOB
#define LED_3_IDX 2
#define LED_3_IDX_MASK (1 << LED_3_IDX)

#define BUT_1_PIO PIOD
#define BUT_1_PIO_ID ID_PIOD
#define BUT_1_IDX 28
#define BUT_1_IDX_MASK (1u << BUT_1_IDX)

#define BUT_2_PIO PIOC
#define BUT_2_PIO_ID ID_PIOC
#define BUT_2_IDX 31
#define BUT_2_IDX_MASK (1u << BUT_2_IDX)

#define BUT_3_PIO PIOA
#define BUT_3_PIO_ID ID_PIOA
#define BUT_3_IDX 19
#define BUT_3_IDX_MASK (1u << BUT_3_IDX)

volatile char flag_redraw = 1;
volatile char flag_aberto = 0;
volatile int star_position = 0;
volatile Bool waiting = false;
char password[] = "0000";

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN | RTT_MR_RTTINCIEN);
}

void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		//f_rtt_alarme = false;
		//pin_toggle(LED_PIO, LED_IDX_MASK);    // BLINK Led
		
	}

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		// pin_toggle(LED_PIO, LED_IDX_MASK);    // BLINK Led
		waiting = false;                  // flag RTT alarme
		gfx_mono_draw_string("             ", 0, 0, &sysfont);
		if(flag_aberto) {
			gfx_mono_draw_string("Cofre aberto", 0, 0, &sysfont);
			} else {
			gfx_mono_draw_string("Cofre fechado", 0, 0, &sysfont);
		}
	}
}

void but1_callback(void) {
	if(!waiting && !flag_aberto) {
		password[star_position] = '1';
		gfx_mono_draw_string("*", star_position*5, 16, &sysfont);
		star_position += 1;
	} else if(!waiting && flag_aberto) {
		flag_aberto = 0;
		pio_clear(LED_1_PIO, LED_1_IDX_MASK);
		pio_clear(LED_2_PIO, LED_2_IDX_MASK);
		pio_clear(LED_3_PIO, LED_3_IDX_MASK);
		flag_redraw = 1;
	}
}

void but2_callback(void) {
	if(!waiting && !flag_aberto) {
		password[star_position] = '2';
		gfx_mono_draw_string("*", star_position*5, 16, &sysfont);
		star_position += 1;
	}
}

void but3_callback(void) {
	if(!waiting && !flag_aberto) {
		password[star_position] = '3';
		gfx_mono_draw_string("*", star_position*5, 16, &sysfont);
		star_position += 1;
	}
}

void io_init(void) {
  pmc_enable_periph_clk(LED_1_PIO_ID);
  pmc_enable_periph_clk(LED_2_PIO_ID);
  pmc_enable_periph_clk(LED_3_PIO_ID);
  pmc_enable_periph_clk(BUT_1_PIO_ID);
  pmc_enable_periph_clk(BUT_2_PIO_ID);
  pmc_enable_periph_clk(BUT_3_PIO_ID);

  pio_configure(LED_1_PIO, PIO_OUTPUT_0, LED_1_IDX_MASK, PIO_DEFAULT);
  pio_configure(LED_2_PIO, PIO_OUTPUT_0, LED_2_IDX_MASK, PIO_DEFAULT);
  pio_configure(LED_3_PIO, PIO_OUTPUT_0, LED_3_IDX_MASK, PIO_DEFAULT);

  pio_configure(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
  pio_configure(BUT_2_PIO, PIO_INPUT, BUT_2_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
  pio_configure(BUT_3_PIO, PIO_INPUT, BUT_3_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);

  pio_handler_set(BUT_1_PIO, BUT_1_PIO_ID, BUT_1_IDX_MASK, PIO_IT_RISE_EDGE,
  but1_callback);
  pio_handler_set(BUT_2_PIO, BUT_2_PIO_ID, BUT_2_IDX_MASK, PIO_IT_RISE_EDGE,
  but2_callback);
  pio_handler_set(BUT_3_PIO, BUT_3_PIO_ID, BUT_3_IDX_MASK, PIO_IT_RISE_EDGE,
  but3_callback);

  pio_enable_interrupt(BUT_1_PIO, BUT_1_IDX_MASK);
  pio_enable_interrupt(BUT_2_PIO, BUT_2_IDX_MASK);
  pio_enable_interrupt(BUT_3_PIO, BUT_3_IDX_MASK);

  pio_get_interrupt_status(BUT_1_PIO);
  pio_get_interrupt_status(BUT_2_PIO);
  pio_get_interrupt_status(BUT_3_PIO);

  NVIC_EnableIRQ(BUT_1_PIO_ID);
  NVIC_SetPriority(BUT_1_PIO_ID, 4);

  NVIC_EnableIRQ(BUT_2_PIO_ID);
  NVIC_SetPriority(BUT_2_PIO_ID, 4);

  NVIC_EnableIRQ(BUT_3_PIO_ID);
  NVIC_SetPriority(BUT_3_PIO_ID, 4);
}

int main(void) {
  board_init();
  sysclk_init();
  delay_init();
  WDT->WDT_MR = WDT_MR_WDDIS;
  io_init();
  gfx_mono_ssd1306_init();
  
  while (1) {
	if(!waiting) {
		if(flag_redraw) {
			gfx_mono_draw_string("             ", 0, 0, &sysfont);
			if(flag_aberto) {
				gfx_mono_draw_string("Cofre aberto", 0, 0, &sysfont);
				} else {
				gfx_mono_draw_string("Cofre fechado", 0, 0, &sysfont);
			}
			
			flag_redraw = 0;
		}
		
		if(star_position > 3) {
			if(strcmp(password, "1123") == 0) {
				flag_aberto = 1;
				pio_set(LED_1_PIO, LED_1_IDX_MASK);
				pio_set(LED_2_PIO, LED_2_IDX_MASK);
				pio_set(LED_3_PIO, LED_3_IDX_MASK);
				flag_redraw = 1;
			} else {
				waiting = true;
				gfx_mono_draw_string("             ", 0, 0, &sysfont);
				gfx_mono_draw_string("Bloqueado", 0, 0, &sysfont);
				uint16_t pllPreScale = (int) (((float) 32768) / 4.0);
				uint32_t irqRTTvalue = 16;
				RTT_init(pllPreScale, irqRTTvalue);
			}
			
			star_position = 0;
			gfx_mono_draw_string("             ", 0, 16, &sysfont);
		}
	}
  }
}
