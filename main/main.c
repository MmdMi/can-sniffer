#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "driver/can.h"
#include "sdkconfig.h"

enum StateCodes
{
    initCan125,
    initCan250,
    initCan500,
    initCan1000,
    readCan,
    deinitCan
};

enum ReturnCodes
{
    ok,
    fail,
    skip,
    review,
    repeat
};

struct Transition
{
    enum StateCodes SrcState;
    enum ReturnCodes RetCode;
    enum StateCodes DstState;
};

struct Transition StateTransitions[] = {
    {initCan125, ok, readCan},
    {initCan250, ok, readCan},
    {initCan500, ok, readCan},
    {initCan1000, ok, readCan},

    {initCan125, skip, initCan250},
    {initCan250, skip, initCan500},
    {initCan500, skip, initCan1000},
    {initCan1000, skip, initCan125},

    {initCan125, fail, deinitCan},
    {initCan250, fail, deinitCan},
    {initCan500, fail, deinitCan},
    {initCan1000, fail, deinitCan},

    {readCan, ok, readCan},
    {readCan, fail, deinitCan},
    {deinitCan, ok, initCan125},
    {deinitCan, fail, initCan125}};

enum StateCodes cs = initCan125;
enum ReturnCodes rc;
enum StateCodes nextInitState;
int (*StateFunc)(void);

enum ReturnCodes init_can_125();
enum ReturnCodes init_can_250();
enum ReturnCodes init_can_500();
enum ReturnCodes init_can_1000();
enum ReturnCodes read_can();
enum ReturnCodes deinit_can();

int (*state[])(void) = {
    init_can_125,
    init_can_250,
    init_can_500,
    init_can_1000,
    read_can,
    deinit_can};

esp_err_t gpioConfig(int gpio_num, int mode, int pullup, int pulldown)
{
    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = mode;
    gpio_conf.pin_bit_mask = (1ULL << gpio_num);
    gpio_conf.pull_down_en = pulldown;
    gpio_conf.pull_up_en = pullup;
    esp_err_t err = gpio_config(&gpio_conf);
    return err;
}

void app_main(void)
{
    gpioConfig(GPIO_NUM_2,GPIO_MODE_OUTPUT,0,0);
    gpio_set_level(GPIO_NUM_2,1);
    
    while (1)
    {
        StateFunc = state[cs];
        rc = StateFunc();
        for (int i = 0; i < sizeof(StateTransitions) / sizeof(StateTransitions[0]); i++)
        {
            if (StateTransitions[i].SrcState == cs && StateTransitions[i].RetCode == rc)
            {
                cs = StateTransitions[i].DstState;
                break;
            }
        }

        TIMERG0.wdtwprotect.val = TIMG_WDT_WKEY_VALUE;
        TIMERG0.wdtfeed.val = 1;
        TIMERG0.wdtwprotect.val = 0;
    }
}

enum ReturnCodes init_can_125()
{
    if (nextInitState != initCan125)
        return skip;
    const char TAG[] = "CAN125";
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CONFIG_CAN_TX, CONFIG_CAN_RX, CAN_MODE_NORMAL);
    can_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    // Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver installed");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to install driver");
        return fail;
    }

    // Start CAN driver
    if (can_start() == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver started");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to start driver");
        return fail;
    }

    nextInitState = initCan250;
    return ok;
}

enum ReturnCodes init_can_250()
{
    if (nextInitState != initCan250)
        return skip;
    const char TAG[] = "CAN250";
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CONFIG_CAN_TX, CONFIG_CAN_RX, CAN_MODE_NORMAL);
    can_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    // Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver installed");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to install driver");
        return fail;
    }

    // Start CAN driver
    if (can_start() == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver started");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to start driver");
        return fail;
    }

    nextInitState = initCan500;
    return ok;
}

enum ReturnCodes init_can_500()
{
    if (nextInitState != initCan500)
        return skip;
    const char TAG[] = "CAN500";
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CONFIG_CAN_TX, CONFIG_CAN_RX, CAN_MODE_NORMAL);
    can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    // Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver installed");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to install driver");
        return fail;
    }

    // Start CAN driver
    if (can_start() == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver started");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to start driver");
        return fail;
    }

    nextInitState = initCan1000;
    return ok;
}

enum ReturnCodes init_can_1000()
{
    if (nextInitState != initCan1000)
        return skip;
    const char TAG[] = "CAN1000";
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CONFIG_CAN_TX, CONFIG_CAN_RX, CAN_MODE_NORMAL);
    can_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    // Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver installed");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to install driver");
        return fail;
    }

    // Start CAN driver
    if (can_start() == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver started");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to start driver");
        return fail;
    }

    nextInitState = initCan125;
    return ok;
}

enum ReturnCodes read_can()
{
    const char TAG[] = "READ";
    can_message_t message;
    if (can_receive(&message, pdMS_TO_TICKS(1000)) != ESP_OK)
    {
        return fail;
    }

    printf("%x,", message.identifier);
    for (int i = 0; i < message.data_length_code; i++)
    {
        if (message.data[i] < 0x10)
            printf("0");
        printf("%x", message.data[i]);
    }
    printf("\n");

    return ok;
}

enum ReturnCodes deinit_can()
{
    const char TAG[] = "DEINIT";

    // Stop the CAN driver
    if (can_stop() == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver stopped");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to stop driver");
        return fail;
    }

    // Uninstall the CAN driver
    if (can_driver_uninstall() == ESP_OK)
    {
        ESP_LOGI(TAG, "Driver uninstalled");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to uninstall driver");
        return fail;
    }

    return ok;
}