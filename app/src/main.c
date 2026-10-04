#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(led_sensor_node));

    if (!device_is_ready(dev))
    {
        printk("Led Sensor not ready...\n");
    }

    printk("LED sensor driver initialized. Starting loop...\n");

    while (1)
    {
        /* Turn LED ON */
        sensor_sample_fetch(dev);
        k_msleep(1000);

        /* Turn LED OFF and read state */
        struct sensor_value val;
        sensor_channel_get(dev, SENSOR_CHAN_ALL, &val);
        printk("Read sensor state: %d\n", val.val1);
        k_msleep(1000);
    }

    return 0;
}

