#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "trigger.h"

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("Button pressed at %" PRIu32, k_cycle_get_32());

	zbus_chan_pub(&chan_trigger_event, MSG_TRIGGER_EVT(.type = TRIGGER_EVENT_ACTIVATED),
		      K_NO_WAIT);
}

void button_thread(void)
{
	int err;

	if (!gpio_is_ready_dt(&button)) {
		zbus_chan_pub(
			&chan_trigger_event,
			MSG_TRIGGER_EVT(.type = TRIGGER_EVENT_FAILED, .error = {.code = -ENODEV}),
			K_MSEC(500));

		return;
	}

	err = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (err != 0) {
		zbus_chan_pub(&chan_trigger_event,
			      MSG_TRIGGER_EVT(.type = TRIGGER_EVENT_FAILED, .error = {.code = err}),
			      K_MSEC(500));

		return;
	}

	err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (err != 0) {
		zbus_chan_pub(&chan_trigger_event,
			      MSG_TRIGGER_EVT(.type = TRIGGER_EVENT_FAILED, .error = {.code = err}),
			      K_MSEC(500));

		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));

	gpio_add_callback(button.port, &button_cb_data);

	LOG_INF("Set up button at %s pin %d", button.port->name, button.pin);

	zbus_chan_pub(&chan_trigger_event, MSG_TRIGGER_EVT(.type = TRIGGER_EVENT_READY),
		      K_MSEC(500));
}

K_THREAD_DEFINE(button_thread_id, 1024, button_thread, NULL, NULL, NULL, 3, 0, 0);