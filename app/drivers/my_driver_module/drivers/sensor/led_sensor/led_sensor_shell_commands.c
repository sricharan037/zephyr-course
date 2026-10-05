#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <my_sensor/led_sensor.h>
#include <zephyr/shell/shell.h>

static const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(led_sensor_node));

static int sensor_fetch_command_handler(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_info(sh, "Fetching the LED Sensor...\n");

    int ret = sensor_sample_fetch(dev);
    if (ret < 0)
    {
        shell_error(sh, "sensor_sample_fetch failed (%d)\n", ret);
        return ret;
    }

    return 0;
}

static int sensor_read_command_handler(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    struct sensor_value val;

    shell_info(sh, "Getting the LED Sensor channel...\n");

    int ret = sensor_channel_get(dev, SENSOR_CHAN_ALL, &val);
    if (ret < 0)
    {
        shell_error(sh, "sensor_channel_get failed (%d)\n", ret);
        return ret;
    }

    shell_print(sh, "Read -> sensor state: %d & sensor custom value: %d\n", val.val1, val.val2);

    return 0;
}

static int sensor_info_command_handler(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "Device: %s\n", dev->name);
    shell_print(sh, "Ready: %s\n", device_is_ready(dev) ? "yes" : "no");

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensor,
                               SHELL_CMD(fetch, NULL, "Fetch the driver API.", sensor_fetch_command_handler),
                               SHELL_CMD(read, NULL, "Read the driver API state.", sensor_read_command_handler),
                               SHELL_CMD(info, NULL, "Show the device name and ready state.", sensor_info_command_handler),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sensor, &sub_sensor, "Sensor Control.", NULL);
