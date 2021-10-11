#include "sdk_cfg.h"
#include "wpc_ui.h"

#if WIRELESS_POWER_EN

AT_WPC
void ui_set(UI_CMD cmd)
{
#if (LED_LIGHT_NUM == 1)
    switch (cmd) {
    case UI_CMD_POWER_ON:
    case UI_CMD_STANDBY:
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_ON);
        break;
    case UI_CMD_CHARGEING:
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_BREATH);
        break;
    case UI_CMD_CHARGE_FULL:
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_BLINK_S);
        break;
    case UI_CMD_FOD:
    case UI_CMD_EXCEPTION:
    case UI_CMD_OVER_VOLTAGE:
    case UI_CMD_OVER_CURRENT:
    case UI_CMD_OVER_TEMPERATURE:
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_BLINK_F);
        break;
    default:
        break;
    }
#endif

#if (LED_LIGHT_NUM == 2)
    switch (cmd) {
    case UI_CMD_POWER_ON:
    case UI_CMD_STANDBY:
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_ON);
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_OFF);
        break;
    case UI_CMD_CHARGEING:
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_OFF);
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_BREATH);
        break;
    case UI_CMD_CHARGE_FULL:
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_ON);
        break;
    case UI_CMD_FOD:
    case UI_CMD_EXCEPTION:
    case UI_CMD_OVER_VOLTAGE:
    case UI_CMD_OVER_CURRENT:
    case UI_CMD_OVER_TEMPERATURE:
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_OFF);
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_BLINK_F);
        break;
    default:
        break;
    }
#endif

#if (LED_LIGHT_NUM == 3)
    switch (cmd) {
    case UI_CMD_POWER_ON:
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_BLINK_F);
        wpc_led_struct_set(WPC_LED_GREEN, LED_STATUS_BLINK_F);
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_BLINK_F);
        break;
    case UI_CMD_STANDBY:
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_ON);
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_OFF);
        wpc_led_struct_set(WPC_LED_GREEN, LED_STATUS_OFF);
        break;
    case UI_CMD_CHARGEING:
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_ON);
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_OFF);
        wpc_led_struct_set(WPC_LED_GREEN, LED_STATUS_OFF);
        break;
    case UI_CMD_CHARGE_FULL:
        wpc_led_struct_set(WPC_LED_GREEN, LED_STATUS_ON);
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_OFF);
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_OFF);
        break;
    case UI_CMD_FOD:
    case UI_CMD_EXCEPTION:
    case UI_CMD_OVER_VOLTAGE:
    case UI_CMD_OVER_CURRENT:
    case UI_CMD_OVER_TEMPERATURE:
        wpc_led_struct_set(WPC_LED_BLUE, LED_STATUS_BLINK_F);
        wpc_led_struct_set(WPC_LED_RED, LED_STATUS_OFF);
        wpc_led_struct_set(WPC_LED_GREEN, LED_STATUS_OFF);
        break;
    default:
        break;
    }
#endif
}

#endif
