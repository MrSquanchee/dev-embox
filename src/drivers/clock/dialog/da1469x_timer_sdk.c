/**
 * @file
 * @brief General Purpose Timers for Dialog DA1469x
 *
 * @date   07.10.2020
 * @author Alexander Kalmuk
 */

#include <assert.h>
#include <kernel/irq.h>
#include <kernel/time/clock_source.h>
#include <util/log.h>
#include <framework/mod/options.h>
#include <embox/unit.h>

#include <config/custom_config_qspi.h>
#include <hw_timer.h>

#define TMR                  OPTION_GET(NUMBER, timer_n)
#define DA1469X_TIMER_IRQ    OPTION_GET(NUMBER, irq)
#define TICK_RATE_HZ         OPTION_GET(NUMBER, hz)

#if TMR == 1
#define TIMER_ID       HW_TIMER
#elif TMR == 2
#define TIMER_ID       HW_TIMER2
#elif TMR == 3
#define TIMER_ID       HW_TIMER3
#elif TMR == 4
#define TIMER_ID       HW_TIMER4
#else
	#error Unavailable timer
#endif

/* TODO We assume xtal32k is used as LP clock.
 * But there should be more flexible setup. */
#define CLOCK_FREQ     dg_configXTAL32K_FREQ

#define TICK_PERIOD    (CLOCK_FREQ / TICK_RATE_HZ)

EMBOX_UNIT_INIT(da1469x_timer_init);

static __RETAINED_CODE irq_return_t da1469x_timer_irq_handler(
		unsigned int irq_nr, void *data) {

	(void) data;

	switch ((int) TIMER_ID) {
	case (int) HW_TIMER:
		TIMER->TIMER_CLEAR_IRQ_REG = 1;
		break;
	case (int) HW_TIMER2:
		TIMER2->TIMER2_CLEAR_IRQ_REG = 1;
		break;
	case (int) HW_TIMER3:
		TIMER3->TIMER3_CLEAR_IRQ_REG = 1;
		break;
	case (int) HW_TIMER4:
		TIMER4->TIMER4_CLEAR_IRQ_REG = 1;
		break;
	default:
		return IRQ_HANDLED;
	}

	log_debug("irq = %d", irq_nr);

	hw_timer_disable(TIMER_ID);

	return IRQ_HANDLED;
}

static int da1469x_timer_set_next_event(uint32_t next_event) {
	hw_timer_set_reload(TIMER_ID, next_event * TICK_PERIOD);

	hw_timer_enable_clk(TIMER_ID);
	while (hw_timer_get_count(TIMER_ID) != (next_event * TICK_PERIOD));

	hw_timer_enable(TIMER_ID);

	return 0;
}

static int da1469x_timer_config(struct time_dev_conf *conf) {
	return 0;
}

static struct time_event_device da1469x_timer_event = {
	.config = da1469x_timer_config,
	.set_next_event = da1469x_timer_set_next_event,
	.event_hz = TICK_RATE_HZ,
	.irq_nr = DA1469X_TIMER_IRQ,
};

static struct clock_source da1469x_timer_clock_source = {
	.name = "da1469x_timer",
	.event_device = &da1469x_timer_event,
	.read = clock_source_read,
};

static int da1469x_timer_init(void) {
	timer_config timer_cfg = {
		.clk_src = HW_TIMER_CLK_SRC_INT,
		.prescaler = 0,
		.timer = {
			.direction = HW_TIMER_DIR_DOWN,
			.reload_val = TICK_PERIOD - 1,
			.free_run = false,
		},
		.pwm = { .frequency = 0, .duty_cycle = 0 },
	};

	hw_timer_init(TIMER_ID, &timer_cfg);

	if (irq_attach(DA1469X_TIMER_IRQ, da1469x_timer_irq_handler, 0, NULL, "") < 0) {
		return -1;
	}
	HW_TIMER_REG_SETF(TIMER_ID, TIMER_CTRL_REG, TIM_IRQ_EN, 1);

	/* Disable by default */
	hw_timer_disable(TIMER_ID);

	return clock_source_register(&da1469x_timer_clock_source);
}

STATIC_IRQ_ATTACH(DA1469X_TIMER_IRQ, da1469x_timer_irq_handler, NULL);
