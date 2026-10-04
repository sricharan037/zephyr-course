#ifndef MY_SENSOR_LED_SENSOR_H_
#define MY_SENSOR_LED_SENSOR_H_

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int (*led_sensor_set_param_t)(const struct device *dev, int32_t val);

    struct led_sensor_driver_api
    {
        struct sensor_driver_api sensor_api;
        led_sensor_set_param_t set_param;
    };

    static inline int led_sensor_set_param(const struct device *dev, int32_t val)
    {
        const struct led_sensor_driver_api *api =
            (const struct led_sensor_driver_api *)dev->api;

        if (api->set_param == NULL)
        {
            return -ENOSYS;
        }

        return api->set_param(dev, val);
    }

#ifdef __cplusplus
}
#endif

#endif /* MY_SENSOR_LED_SENSOR_H_ */
