#include "includes.h"

uint8_t target_heating_cooling_state = 1;
float target_temperature = 30.0;
uint8_t current_heating_cooling_state = 1;
float current_temperature = 30.0;
uint8_t temperature_display_unit = 1;
bool fan_on = 0;
bool hold_on = 0;

void setup() {  
  tuya_init(115200);
  tuya_set_wifi(TUYA_WIFI_DISCONNECTED);

  WiFiManager wm;
  wm.setDebugOutput(false);
  wm.setConfigPortalTimeout(WIFI_BOOT_CONFIG_TIMEOUT);
  if (!wm.autoConnect()) {
    ESP.restart();
  }
  WiFi.mode(WIFI_STA);
  tuya_set_wifi(TUYA_WIFI_CLOUD);

  homekit_setup();
}

void loop() {
  arduino_homekit_loop();
  tuya_loop();
  thermostat_state_loop();
  
}

uint32_t previousMillis = 0;
uint16_t heartbeat_timer = 0;

void thermostat_state_loop() {
  // if it's been 700 ms - somewhat arbitarary
  uint32_t currentMillis = millis();
  if (currentMillis - previousMillis > 700) {
    previousMillis = currentMillis;
    heartbeat_timer++;

//    if (heartbeat_timer == 15)
//            homekit_characteristic_notify(&h_thermo_c_st, HOMEKIT_UINT8(OFF_STATE));
    if (heartbeat_timer > 30) {
      heartbeat_timer = 0;
      //will trigger the thermostat to send back it's state information which will be processed in tuya_process_state_packet
      if (WiFi.status() == WL_CONNECTED){
        //printf("Sending TUYA_WIFI_CLOUD\n");
        tuya_set_wifi(TUYA_WIFI_CLOUD);
      }
      else{
        //printf("Sending TUYA_WIFI_DISCONNECTED\n");
        tuya_set_wifi(TUYA_WIFI_DISCONNECTED);
      }
    }
  }
}


void tuya_process_state_packet() {

  uint8_t dpidStart = 6;
  uint16_t val = 0;

  uint16_t dpDataLen  = Tuya.buffer[dpidStart + 2] << 8 | Tuya.buffer[dpidStart + 3];
  uint8_t  dpId       = Tuya.buffer[dpidStart];
  uint8_t  dpDataType = Tuya.buffer[dpidStart + 1];

  //printf("Process DPID: %d\n", dpId);

  switch (dpId) {

    case 1:
      // 46 hold = 1, not hold = 0
      // 46 is actually hold, but I can't figure out how to control temperature in hold
      // mode without resetting the mode to temporary, so using PR0 (non scheduled
      // programming mode) to set hold mode.  Hold mode off then sets back to PR7
      // 73 PR7 mode = 2, PR0 mode = 0
      val = Tuya.buffer[dpidStart + 73];
      //if(val == 2){
        hold_on = 0;
        //printf("Hold Off\n");
      //}
      if(val == 0){
        val = Tuya.buffer[dpidStart + 45];      
        if( val == 0 ){
          hold_on = 1;
          //printf("Hold On\n");
        }
      }
      if (cha_hold_switch_on.value.bool_value != hold_on){
        cha_hold_switch_on.value.bool_value = hold_on;
        homekit_characteristic_notify(&cha_hold_switch_on, HOMEKIT_BOOL(hold_on));
      }
    
        // 41 fan auto = 1, fan off = 0
      val = Tuya.buffer[dpidStart + 35];
      if(val == 0){
        fan_on = 1;
        //printf("Fan On\n");
      }
      if(val == 1){
        fan_on = 0;
        //printf("Fan Auto\n");
      }
      if(cha_fan_switch_on.value.bool_value != fan_on){
        cha_fan_switch_on.value.bool_value = fan_on;
        homekit_characteristic_notify(&cha_fan_switch_on, HOMEKIT_BOOL(fan_on));
      }
      
      
      //Target temperature in 10 * degrees f
      val = Tuya.buffer[dpidStart + 16] << 8  |
            Tuya.buffer[dpidStart + 17];

      if(val > 990) val = 990;
      if(val < 550) val = 550;

      //printf("Temp val:%d\n", val);
      //Convert to celcius
      target_temperature = ((float(val) / 10) - 32 ) / 1.8;// -0.5;
      
      //printf("Update Temp: %f\n", current_temperature);
      if( h_thermo_t_temp.value.uint8_value != target_temperature ) {
        h_thermo_t_temp.value.uint8_value = target_temperature;
        homekit_characteristic_notify(&h_thermo_t_temp, HOMEKIT_FLOAT(target_temperature));
      }
      //Current temperature in 10 * degrees f
      val = Tuya.buffer[dpidStart + 24] << 8  |
            Tuya.buffer[dpidStart + 25];

      if(val > 990) val = 990;
      if(val < 320) val = 320;

      //printf("Temp val:%d\n", val);
      //Convert to celcius
      current_temperature = ((float(val) / 10) - 32 ) / 1.8;// -0.5;

      //printf("Update Temp: %f\n", current_temperature);
      if( h_thermo_c_temp.value.uint8_value != current_temperature ){
        h_thermo_c_temp.value.uint8_value = current_temperature;
        homekit_characteristic_notify(&h_thermo_c_temp, HOMEKIT_FLOAT(current_temperature));
      }

      
      //heat = 0 , cool = 1, 04 = off
      val = Tuya.buffer[dpidStart + 30];
      if(val == 0){
        target_heating_cooling_state = HEATING_STATE;
        if( current_temperature < target_temperature ) current_heating_cooling_state = HEATING_STATE;
        else current_heating_cooling_state = OFF_STATE;
      }
      if(val == 1){
        target_heating_cooling_state = COOLING_STATE;
        if( current_temperature > target_temperature ) current_heating_cooling_state = COOLING_STATE;
        else current_heating_cooling_state = OFF_STATE;
      }
      if(val == 4){
        target_heating_cooling_state = OFF_STATE;
        current_heating_cooling_state = OFF_STATE;
      }
      if( h_thermo_t_st.value.uint8_value != target_heating_cooling_state ){
        h_thermo_t_st.value.uint8_value = target_heating_cooling_state;
        homekit_characteristic_notify(&h_thermo_t_st, HOMEKIT_UINT8(target_heating_cooling_state));
      }
      if( h_thermo_c_st.value.uint8_value != current_heating_cooling_state ){
        h_thermo_c_st.value.uint8_value = current_heating_cooling_state;
        homekit_characteristic_notify(&h_thermo_c_st, HOMEKIT_UINT8(current_heating_cooling_state));
      }

      
      break;
  }
}

void identify() {

}


// ---------------------------------------------------------
// HOMEKIT Stuff
// ---------------------------------------------------------

void homekit_setup() {
  sprintf(serial, "R0B0%X\0", ESP.getChipId());
  sprintf(device_name, "%s %X\0", ROBO_MODEL, ESP.getChipId());

  h_thermo_t_st.getter = get_target_heating_cooling_state;
  h_thermo_t_st.setter = set_target_heating_cooling_state;
  h_thermo_t_temp.getter = get_target_temperature;
  h_thermo_t_temp.setter = set_target_temperature;
  h_thermo_c_st.getter = get_current_heating_cooling_state;
  h_thermo_c_temp.getter = get_current_temperature;
  h_thermo_temp_unit.getter = get_temperature_display_unit;
  h_thermo_temp_unit.setter = set_temperature_display_unit;
//  cha_fan_switch_on.getter = get_cha_fan_switch_on;
  cha_fan_switch_on.setter = set_cha_fan_switch_on;
//  cha_hold_switch_on.getter = get_cha_hold_switch_on;
  cha_hold_switch_on.setter = set_cha_hold_switch_on;
//  cha_runprog_switch_on.getter = get_cha_runprog_switch_on;
//  cha_runprog_switch_on.setter = set_cha_runprog_switch_on;
//  cha_tempmode_switch_on.getter = get_cha_tempmode_switch_on;
//  cha_tempmode_switch_on.setter = set_cha_tempmode_switch_on;
  
  arduino_homekit_setup(&config);
}

/*void set_cha_runprog_switch_on(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_fan_switch_on.value.bool_value = on;  //sync the value
  tuya_send_bool(5,!on);
}
void set_cha_tempmode_switch_on(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_fan_switch_on.value.bool_value = on;  //sync the value
  tuya_send_bool(5,!on);
}
*/
void set_cha_fan_switch_on(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_fan_switch_on.value.bool_value = on;  //sync the value
  tuya_send_bool(5,!on);
}
void set_cha_hold_switch_on(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_hold_switch_on.value.bool_value = on;  //sync the value

  // I don't know how to set hold mode (like pressing the top button)
  // 101 is actually leave mode, but I can't figure out how to control temperature in leave
  // mode without resetting the mode to temporary, so using PR0 (non scheduled
  // programming mode) to set hold mode.  Hold mode off then sets back to PR7
  tuya_send_bool(102,!on);
}

homekit_value_t get_target_heating_cooling_state(void) {
  return HOMEKIT_UINT8(target_heating_cooling_state);
}

void set_target_heating_cooling_state(const homekit_value_t value) {
  if ( target_heating_cooling_state != value.uint8_value ){
    target_heating_cooling_state = value.uint8_value;
    h_thermo_t_st.value = value;  //sync the value
    if(value.uint8_value == OFF_STATE){
      tuya_send_bool(1,0);
    }
    if(value.uint8_value == HEATING_STATE){
      tuya_send_bool(1,1);
      tuya_send_bool(4,0);
    }
    if(value.uint8_value == COOLING_STATE){
      tuya_send_bool(1,1);
      tuya_send_bool(4,1);
    }
    if(value.uint8_value == AUTO_STATE){
      //??
    }
    //printf("set_target_heating_cooling_state value: %d\n", value.uint8_value);
  }
}

homekit_value_t get_target_temperature()
{
    //printf("%s,%f++\n", __FUNCTION__, target_temperature);
    return HOMEKIT_FLOAT(target_temperature);
}

void set_target_temperature(homekit_value_t value)
{
    //printf("%s++\n", __FUNCTION__);
    if (homekit_format_float != value.format)
    {
        //printf("Invalid on-value format: %f\n", value.format);
        return;
    }
    target_temperature = value.float_value;
    tuya_send_value(2, 10*((target_temperature * 1.8) + 32) + 0.5);

  h_thermo_t_temp.value = value;  //sync the value

}

homekit_value_t get_current_heating_cooling_state()
{
    //printf("%s++\n", __FUNCTION__);
    return HOMEKIT_UINT8(current_heating_cooling_state);
}

homekit_value_t get_temperature_display_unit()
{
    //printf("%s,%d++\n", __FUNCTION__, temperature_display_unit);
    return HOMEKIT_UINT8(temperature_display_unit);
}

void set_temperature_display_unit(homekit_value_t value)
{
  //printf("%s++\n", __FUNCTION__);
  if (homekit_format_uint8 != value.format)
  {
      //printf("Invalid on-value format: %d\n", value.format);
      return;
  }

  temperature_display_unit = value.uint8_value;
  //printf("value: %d\n", value.uint8_value);

  h_thermo_temp_unit.value = value;  //sync the value
  // TODO, send cmd
}

homekit_value_t get_current_temperature()
{
  //printf("%s,%f++\n", __FUNCTION__, current_temperature);
  return HOMEKIT_FLOAT(current_temperature);
}
