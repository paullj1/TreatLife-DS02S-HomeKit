#include "model.h"
#include "version.h"

#define HOMEKIT_LOG_LEVEL HOMEKIT_NO_LOG
//#define HOMEKIT_LOG_LEVEL HOMEKIT_LOG_DEBUG

#include <homekit/homekit.h>
#include <homekit/characteristics.h>



// Called to identify this accessory. See HAP section 6.7.6 Identify Routine
// Generally this is called when paired successfully or click the "Identify Accessory" button in Home APP.
void my_accessory_identify(homekit_value_t _value) {
  identify();
}

char serial[16] = "XXXXXX\0";
char device_name[32] = "XXXXXX\0";

// Thermostat Service, HOMEKIT_SERVICE_THERMOSTAT

// is active how we report IDLE/HEATING/COOLING??
//homekit_characteristic_t active = HOMEKIT_CHARACTERISTIC_(ACTIVE, 0);
homekit_characteristic_t h_thermo_c_st = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATING_COOLING_STATE, 0);
homekit_characteristic_t h_thermo_t_st = HOMEKIT_CHARACTERISTIC_(TARGET_HEATING_COOLING_STATE, 0);
homekit_characteristic_t h_thermo_c_temp = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0);
homekit_characteristic_t h_thermo_t_temp = HOMEKIT_CHARACTERISTIC_(TARGET_TEMPERATURE, 0);
homekit_characteristic_t h_thermo_temp_unit = HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS, 0);
homekit_characteristic_t cha_fan_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
//homekit_characteristic_t cha_fan_switch_name = HOMEKIT_CHARACTERISTIC_(NAME, "Fan Switch");
homekit_characteristic_t cha_hold_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
//homekit_characteristic_t cha_hold_switch_name = HOMEKIT_CHARACTERISTIC_(NAME, "Hold Mode");
//homekit_characteristic_t cha_runprog_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
//homekit_characteristic_t cha_runprog_switch_name = HOMEKIT_CHARACTERISTIC_(NAME, "Run Program");
//homekit_characteristic_t cha_tempmode_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
//homekit_characteristic_t cha_tempmode_switch_name = HOMEKIT_CHARACTERISTIC_(NAME, "Temporary Mode");

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_thermostat, .services=(homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, device_name),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, ROBO_MANUFACTURER),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, serial),
            HOMEKIT_CHARACTERISTIC(MODEL, ROBO_MODEL),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, ROBO_VERSION),
      NULL
    }),
    HOMEKIT_SERVICE(THERMOSTAT, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Air Conditioner"),
//                    &active,
                    &h_thermo_c_st,
                    &h_thermo_t_st,
                    &h_thermo_c_temp,
                    &h_thermo_t_temp,
                    &h_thermo_temp_unit,
                    NULL    }),
    NULL
  }),
  HOMEKIT_ACCESSORY(.id=2, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Fan"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
      HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
      &cha_fan_switch_on,
  //    &cha_fan_switch_name,
      NULL
    }),
    NULL
  }),
  HOMEKIT_ACCESSORY(.id=3, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Hold"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
      HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
      &cha_hold_switch_on,
  //    &cha_hold_switch_name,
      NULL
    }),
    NULL
  }),
/*  HOMEKIT_ACCESSORY(.id=4, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Run Program"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
      HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
      &cha_runprog_switch_on,
  //    &cha_runprog_switch_name,
      NULL
    }),
    NULL
  }),
  HOMEKIT_ACCESSORY(.id=5, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Temporary Mode"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
      HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
      &cha_tempmode_switch_on,
  //    &cha_tempmode_switch_name,
      NULL
    }),
    NULL
  }),
  */
  NULL
};



homekit_server_config_t config = {
  .accessories = accessories,
  .password = ROBO_SETUP_CODE,
};
