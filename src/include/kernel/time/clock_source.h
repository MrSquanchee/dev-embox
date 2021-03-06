/**
 * @file
 * @brief API for registration of time's device
 *
 * @date 06.07.2011
 * @author Ilia Vaprol
 * @author Roman Kurbatov
 *         - Interface for iterating time devices (clock_source_get_list() and
 *           clock_source_foreach()).
 */

#ifndef KERNEL_CLOCK_SOURCE_H_
#define KERNEL_CLOCK_SOURCE_H_

#include <stdint.h>
#include <util/dlist.h>
#include <kernel/time/time_device.h>
#include <kernel/time/ktime.h>
#include <kernel/time/time.h>

#include <util/array.h>

enum clock_source_property {
	CS_WITH_IRQ = 0,
	CS_ANY = 1,
	CS_WITHOUT_IRQ = 2
};

/**
 * Time source of hardware time - events and cycles.
 * @param read - return count of ns
 * @param link - link in time sources list
 */
struct clock_source {
	const char *name;
	struct time_event_device *event_device;
	struct time_counter_device *counter_device;
	volatile clock_t jiffies; /**< count of jiffies since clock source started */
	uint32_t flags; /**< periodical or not */
	struct timespec (*read)(struct clock_source *cs);
	uint32_t counter_mult;
	uint32_t counter_shift;

	struct dlist_head lnk;
};

extern struct clock_source *clock_source_get_best(enum clock_source_property property);
extern struct clock_source *clock_source_get_by_name(const char *name);

static inline int clock_source_set_next_event(struct clock_source *cs,
		uint32_t next_event) {
	assert(cs && cs->event_device);

	return cs->event_device->set_next_event(next_event);
}

/**
 * Read cycles from clock source since moment when it started. This function may be used exactly
 * for three types of clock sources:
 * 1. Clock source, that generates IRQ on each overflow of internal counter (e.g. PIT).
 * 2. Clock source, that generates IRQ with fixed time interval without counter (e.g. ACPI_PM, RTC).
 * 3. Clock source, that did'nt generate IRQ. It is simple counter (e.g. RDTSC).
 * @param cs - clock source read from
 * @return count of nanoseconds from moment when clock source started
 */
extern struct timespec clock_source_read(struct clock_source *cs);
//extern time64_t clock_source_counter_read(struct clock_source *cs);

extern int clock_source_register(struct clock_source *cs);
extern int clock_source_unregister(struct clock_source *cs);

extern time64_t clock_source_get_hwcycles(struct clock_source *cs);

static inline uint32_t clock_sourcehz2mult(uint32_t hz, uint32_t shift) {
	uint64_t tmp = ((uint64_t)NSEC_PER_SEC) << shift;
	return tmp / hz;
}

#endif /* KERNEL_CLOCK_SOURCE_H_ */
