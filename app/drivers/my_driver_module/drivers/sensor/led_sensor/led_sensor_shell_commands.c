#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <my_sensor/led_sensor.h>
#include <zephyr/shell/shell.h>

const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(led_sensor_node));

static int sensor_fetch_command_handler(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_info(sh, "Fetching the LED Sensor...\n");
    return sensor_sample_fetch(dev);
}

static int sensor_get_command_handler(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    struct sensor_value val;

    shell_info(sh, "getting the LED Sensor channel...\n");
    sensor_channel_get(dev, SENSOR_CHAN_ALL, &val);

    printk("Read -> sensor state: %d & sensor custom value: %d\n", val.val1, val.val2);
    return 0;
}

static int sensor_info_command_handler(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argv);
    ARG_UNUSED(argc);

    shell_info(sh, "COPYRIGHT 2026 - @DarkProgrammer. \n Version -> 1.0.0\n Help \n --- \n fetch: fetch the Api state \n read: get channel info.\n Thank you for using the utility.\n");
    return 0;
}

static int sensor_custom_value_set(const struct shell *sh, size_t argc, char **argv)
{
    int8_t custom_value = atoi(argv[1]);

    if (custom_value < 1 || custom_value > 100)
    {
        shell_error(sh, "Invalid Range\n");
        return -EINVAL;
    }

    int result = led_sensor_set_param(dev, custom_value);
    return result;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensor,
                               SHELL_CMD(fetch, NULL, "Fetch the driver API.", sensor_fetch_command_handler),
                               SHELL_CMD(read, NULL, "Read the driver API state.", sensor_get_command_handler),
                               SHELL_CMD(info, NULL, "Utility Information.", sensor_info_command_handler),
                               SHELL_CMD_ARG(set, NULL, "Set a custom threshhold to the sensor. Range: [1-100].", sensor_custom_value_set, 2, 0),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sensorroot, &sub_sensor, "Sensor Control.", NULL);
