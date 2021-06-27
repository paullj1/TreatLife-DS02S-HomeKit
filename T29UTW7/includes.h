#define WM_DEBUG_LEVEL DEBUG_ERROR
#define HOMEKIT_LOG_LEVEL HOMEKIT_NO_LOG
//#define HOMEKIT_LOG_LEVEL HOMEKIT_LOG_INFO

#include <WiFiManager.h>
#include <arduino_homekit_server.h>

#include "tuya.h"
#include "model.h"

#define OFF_STATE              0
#define HEATING_STATE          1
#define COOLING_STATE          2
#define AUTO_STATE             3

#define CELCIUS                0
#define FAHRENHEIT             1


extern "C" homekit_server_config_t config;
//extern "C" homekit_characteristic_t active;
extern "C" homekit_characteristic_t h_thermo_c_st;
extern "C" homekit_characteristic_t h_thermo_t_st;
extern "C" homekit_characteristic_t h_thermo_c_temp;
extern "C" homekit_characteristic_t h_thermo_t_temp;
extern "C" homekit_characteristic_t h_thermo_temp_unit;
extern "C" homekit_characteristic_t cha_fan_switch_on;
//extern "C" homekit_characteristic_t cha_fan_switch_name;
extern "C" homekit_characteristic_t cha_hold_switch_on;
//extern "C" homekit_characteristic_t cha_hold_switch_name;
//extern "C" homekit_characteristic_t cha_runprog_switch_on;
//extern "C" homekit_characteristic_t cha_runprog_switch_name;
//extern "C" homekit_characteristic_t cha_tempmode_switch_on;
//extern "C" homekit_characteristic_t cha_tempmode_switch_name;

extern "C" char serial[16];
extern "C" char device_name[32];
extern "C" void identify();
