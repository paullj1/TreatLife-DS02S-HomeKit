#include "includes.h"

void tuya_send_cmd(uint8_t cmd, uint8_t payload[] = nullptr, uint16_t payload_len = 0) {

  uint8_t checksum = (0xFF + cmd + (payload_len >> 8) + (payload_len & 0xFF));
  Serial.write(0x55);                  // Tuya header 55AA
  Serial.write(0xAA);
  Serial.write((uint8_t)0x00);         // version 00
  Serial.write(cmd);                   // Tuya command
  Serial.write(payload_len >> 8);      // following data length (Hi)
  Serial.write(payload_len & 0xFF);    // following data length (Lo)
  for (uint32_t i = 0; i < payload_len; ++i) {
    Serial.write(payload[i]);
    checksum += payload[i];
  }
  Serial.write(checksum);
  Serial.flush();
}

void tuya_send_state(uint8_t id, uint8_t type, uint8_t* value) {
  uint16_t payload_len = 4;
  uint8_t payload_buffer[8];
  payload_buffer[0] = id;
  payload_buffer[1] = type;
  switch (type) {
    case TUYA_TYPE_BOOL:
    case TUYA_TYPE_ENUM:
      payload_len += 1;
      payload_buffer[2] = 0x00;
      payload_buffer[3] = 0x01;
      payload_buffer[4] = value[0];
      break;
    case TUYA_TYPE_VALUE:
      payload_len += 4;
      payload_buffer[2] = 0x00;
      payload_buffer[3] = 0x04;
      payload_buffer[4] = value[3];
      payload_buffer[5] = value[2];
      payload_buffer[6] = value[1];
      payload_buffer[7] = value[0];
      break;
  }
  tuya_send_cmd(TUYA_CMD_SET_DP, payload_buffer, payload_len);
}

void tuya_send_bool(uint8_t id, bool value) {
  tuya_send_state(id, TUYA_TYPE_BOOL, (uint8_t*)&value);
}

void tuya_send_value(uint8_t id, uint32_t value) {
  tuya_send_state(id, TUYA_TYPE_VALUE, (uint8_t*)(&value));
}

void tuya_send_enum(uint8_t id, uint32_t value) {
  tuya_send_state(id, TUYA_TYPE_ENUM, (uint8_t*)(&value));
}

void tuya_send_string(uint8_t id, char data[]) {

  uint16_t len = strlen(data);
  uint16_t payload_len = 4 + len;
  uint8_t payload_buffer[payload_len];
  payload_buffer[0] = id;
  payload_buffer[1] = TUYA_TYPE_STRING;
  payload_buffer[2] = len >> 8;
  payload_buffer[3] = len & 0xFF;

  for (uint16_t i = 0; i < len; i++) {
    payload_buffer[4 + i] = data[i];
  }

  tuya_send_cmd(TUYA_CMD_SET_DP, payload_buffer, payload_len);
}

void tuya_set_wifi(uint8_t mode) {
  uint16_t payload_len = 1;
  uint8_t payload_buffer[payload_len];
  payload_buffer[0] = mode;
  tuya_send_cmd(TUYA_CMD_WIFI_STATE, payload_buffer, payload_len);
}

void tuya_init(uint32_t baud) {
  Tuya.buffer = (char*)(malloc(TUYA_BUFFER_SIZE));
  if (Tuya.buffer != nullptr) {

    Serial.begin(baud);
    Serial.setDebugOutput(false);

  }
  Tuya.heartbeat_timer = 0; // init heartbeat timer when dimmer init is done
  tuya_request_state(0);

  tuya_set_wifi(TUYA_WIFI_DISCONNECTED);
}

void tuya_loop() {
  get_tuya_input();

  // if it's a second
  uint32_t currentMillis = millis();
  if (currentMillis - Tuya.previousMillis > 1000) {
    Tuya.previousMillis = currentMillis;
    Tuya.heartbeat_timer++;

    if (Tuya.heartbeat_timer > 10) {
      Tuya.heartbeat_timer = 0;
      tuya_send_cmd(TUYA_CMD_HEARTBEAT);
    }
  }
}

// TuyaSerialInput();
void get_tuya_input() {
  while (Serial.available()) {
    yield();
    uint8_t serial_in_byte = Serial.read();

    if (serial_in_byte == 0x55) {            // Start TUYA Packet
      Tuya.cmd_status = 1;
      Tuya.buffer[Tuya.byte_counter++] = serial_in_byte;
      Tuya.cmd_checksum += serial_in_byte;
    }
    else if (Tuya.cmd_status == 1 && serial_in_byte == 0xAA) { // Only packets with header 0x55AA are valid
      Tuya.cmd_status = 2;

      Tuya.byte_counter = 0;
      Tuya.buffer[Tuya.byte_counter++] = 0x55;
      Tuya.buffer[Tuya.byte_counter++] = 0xAA;
      Tuya.cmd_checksum = 0xFF;
    }
    else if (Tuya.cmd_status == 2) {
      if (Tuya.byte_counter == 5) { // Get length of data
        Tuya.cmd_status = 3;
        Tuya.data_len = serial_in_byte;
      }
      Tuya.cmd_checksum += serial_in_byte;
      Tuya.buffer[Tuya.byte_counter++] = serial_in_byte;
    }
    else if ((Tuya.cmd_status == 3) && (Tuya.byte_counter == (6 + Tuya.data_len)) && (Tuya.cmd_checksum == serial_in_byte)) { // Compare checksum and process packet
      Tuya.buffer[Tuya.byte_counter++] = serial_in_byte;

      char hex_char[(Tuya.byte_counter * 2) + 2];
      uint16_t len = Tuya.buffer[4] << 8 | Tuya.buffer[5];

      uint16_t DataVal = 0;
      uint8_t dpId = 0;
      uint8_t dpDataType = 0;

      if (len > 0) {
        if (TUYA_CMD_STATE == Tuya.buffer[3]) {
          //55 AA 03 07 00 0D 01 04 00 01 02 02 02 00 04 00 00 00 1A 40
          // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
          uint8_t dpidStart = 6;
          while (dpidStart + 4 < Tuya.byte_counter) {
            dpId = Tuya.buffer[dpidStart];
            dpDataType = Tuya.buffer[dpidStart + 1];
            uint16_t dpDataLen = Tuya.buffer[dpidStart + 2] << 8 | Tuya.buffer[dpidStart + 3];
            const unsigned char *dpData = (unsigned char*)&Tuya.buffer[dpidStart + 4];

            if (TUYA_CMD_STATE == Tuya.buffer[3]) {
              if (TUYA_TYPE_BOOL == dpDataType && dpDataLen == 1) {
                DataVal = dpData[0];
              } else if (TUYA_TYPE_VALUE == dpDataType && dpDataLen == 4) {
                uint32_t dpValue = (uint32_t)dpData[0] << 24 | (uint32_t)dpData[1] << 16 | (uint32_t)dpData[2] << 8 | (uint32_t)dpData[3] << 0;
                DataVal = dpValue;
              } else if (TUYA_TYPE_STRING == dpDataType) {
                // nothing?
              } else if (TUYA_TYPE_ENUM == dpDataType && dpDataLen == 1) {
                DataVal = dpData[0];
              } else {
                // nothing?
              }
            }
            dpidStart += dpDataLen + 4;
          }
        }
      }

      tuya_process_packet();

      Tuya.byte_counter = 0;
      Tuya.cmd_status = 0;
      Tuya.cmd_checksum = 0;
      Tuya.data_len = 0;
    }                                                    // read additional packets from TUYA
    else if (Tuya.byte_counter < TUYA_BUFFER_SIZE - 1) { // add char to string if it still fits
      Tuya.buffer[Tuya.byte_counter++] = serial_in_byte;
      Tuya.cmd_checksum += serial_in_byte;
    } else {
      Tuya.byte_counter = 0;
      Tuya.cmd_status = 0;
      Tuya.cmd_checksum = 0;
      Tuya.data_len = 0;
    }
  }
}

void reset_wifi() {
  tuya_set_wifi(TUYA_WIFI_DISCONNECTED);
  WiFiManager wm;
  wm.setDebugOutput(false);
  wm.setConfigPortalTimeout(180);
  wm.startConfigPortal();
  tuya_set_wifi(TUYA_WIFI_CONNECTED);
}

// TuyaNormalPowerModePacketProcess();
void tuya_process_packet() {

  switch (Tuya.buffer[3]) {
    case TUYA_CMD_QUERY_PRODUCT:
      tuya_handle_product_info();
      tuya_send_cmd(TUYA_CMD_MCU_CONF);
      break;

    case TUYA_CMD_WIFI_RESET:
    case TUYA_CMD_WIFI_SELECT:
      reset_wifi();
      break;

    case TUYA_CMD_STATE:
      tuya_process_state_packet();
      break;

    case TUYA_CMD_MCU_CONF:
      tuya_request_state(0);
      break;
  }
}

void tuya_request_state(uint8_t state_type) {
  switch (state_type) {
    case 0:
      tuya_send_cmd(TUYA_CMD_QUERY_STATE);
      break;
    case 8:
      tuya_send_cmd(TUYA_CMD_QUERY_PRODUCT);
      break;
  }
}

void tuya_handle_product_info() {
  uint16_t dataLength = Tuya.buffer[4] << 8 | Tuya.buffer[5];
  char *data = &Tuya.buffer[6];
  // Log it eventually
}

