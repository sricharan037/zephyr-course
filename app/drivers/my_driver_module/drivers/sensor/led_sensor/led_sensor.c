#define DT_DRV_COMPAT my_led_sensor

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <my_sensor/led_sensor.h>

LOG_MODULE_REGISTER(led_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct led_sensor_config
{
    struct gpio_dt_spec led;
};

struct led_sensor_data
{
    int32_t state;
    int32_t custom_threshhold;
};

static int led_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    const struct led_sensor_config *cfg = dev->config;
    struct led_sensor_data *data = dev->data;

    gpio_pin_set_dt(&cfg->led, 1);
    data->state = 1;

    LOG_INF("sensor_sample_fetch: LED turned ON (custom value = %d)", data->custom_threshhold);
    return 0;
}

static int led_sensor_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    const struct led_sensor_config *cfg = dev->config;
    struct led_sensor_data *data = dev->data;

    val->val1 = data->state;
    val->val2 = data->custom_threshhold;

    gpio_pin_set_dt(&cfg->led, 0);
    data->state = 0;

    LOG_INF("sensor_channel_get: LED turned OFF");
    return 0;
}

static int led_sensor_impl_set_param(const struct device *dev, int32_t val)
{
    struct led_sensor_data *data = dev->data;
    data->custom_threshhold = val;

    LOG_INF("Custom parameter updated = %d", val);
    return 0;
}

static const struct led_sensor_driver_api led_sensor_api = {
    .sensor_api = {
        .sample_fetch = led_sensor_sample_fetch,
        .channel_get = led_sensor_channel_get,
    },
    .set_param = led_sensor_impl_set_param,
};

static int led_sensor_init(const struct device *dev)
{
    const struct led_sensor_config *cfg = dev->config;

    if (!gpio_is_ready_dt(&cfg->led))
    {
        LOG_ERR("LED GPIO controller not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&cfg->led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Failed to configure LED pin (%d)", ret);
        return ret;
    }

    LOG_INF("LED Sensor initialized successfully");
    return 0;
}

#define LED_SENSOR_INIT(inst)                                          \
    static struct led_sensor_data led_sensor_data_##inst = {           \
        .state = 0,                                                    \
        .custom_threshhold = 10, /* Default initial value */           \
    };                                                                 \
    static const struct led_sensor_config led_sensor_config_##inst = { \
        .led = GPIO_DT_SPEC_INST_GET(inst, led_gpios),                 \
    };                                                                 \
    DEVICE_DT_INST_DEFINE(inst,                                        \
                          led_sensor_init,                             \
                          NULL,                                        \
                          &led_sensor_data_##inst,                     \
                          &led_sensor_config_##inst,                   \
                          POST_KERNEL,                                 \
                          CONFIG_SENSOR_INIT_PRIORITY,                 \
                          &led_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(LED_SENSOR_INIT)
