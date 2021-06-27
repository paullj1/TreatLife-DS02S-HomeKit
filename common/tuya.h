#ifndef TUYA_ROBO
#define TUYA_ROBO

#define TUYA_BUFFER_SIZE         256
#define WIFI_BOOT_CONFIG_TIMEOUT  30
#define MINIMUM_DIMMER_VALUE      10

#define TUYA_CMD_HEARTBEAT     0x00
#define TUYA_CMD_QUERY_PRODUCT 0x01
#define TUYA_CMD_MCU_CONF      0x02
#define TUYA_CMD_WIFI_STATE    0x03
#define TUYA_CMD_WIFI_RESET    0x04
#define TUYA_CMD_WIFI_SELECT   0x05
#define TUYA_CMD_SET_DP        0x06
#define TUYA_CMD_STATE         0x07
#define TUYA_CMD_QUERY_STATE   0x08
#define TUYA_CMD_SET_TIME      0x1C

#define TUYA_WIFI_CONFIG_MODE  0x01
#define TUYA_WIFI_DISCONNECTED 0x02
#define TUYA_WIFI_CONNECTED    0x03
#define TUYA_WIFI_CLOUD        0x04

#define TUYA_TYPE_BOOL         0x01
#define TUYA_TYPE_VALUE        0x02
#define TUYA_TYPE_STRING       0x03
#define TUYA_TYPE_ENUM         0x04

struct TUYA {
  uint8_t cmd_status = 0;                 // Current status of serial-read
  uint8_t cmd_checksum = 0;               // Checksum of tuya command
  uint8_t data_len = 0;                   // Data lenght of command
  uint8_t heartbeat_timer = 0;            // 10 second heartbeat timer for tuya module
  char *buffer = nullptr;                 // Serial receive buffer
  int byte_counter = 0;                   // Index in serial receive buffer
  uint32_t previousMillis = 0;            // Use to time heartbeats
} Tuya;

#endif
