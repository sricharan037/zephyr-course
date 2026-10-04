#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <my_sensor/led_sensor.h>

int main(void)
{
    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(led_sensor_node));

    if (!device_is_ready(dev))
    {
        printk("Led Sensor not ready...\n");
    }

    printk("LED sensor driver initialized...Changing parameter...Starting loop...\n");

    int newValue = 50;

    while (1)
    {

        led_sensor_set_param(dev, newValue);
        newValue += 10;
        /* Turn LED ON */
        sensor_sample_fetch(dev);
        k_msleep(1000);

        /* Turn LED OFF and read state */
        struct sensor_value val;
        sensor_channel_get(dev, SENSOR_CHAN_ALL, &val);
        printk("Read -> sensor state: %d & sensor custom value: %d\n", val.val1, val.val2);
        k_msleep(1000);
    }

    return 0;
}

