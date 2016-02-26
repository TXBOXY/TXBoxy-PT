/*
 * TXBoxy-AR firmware for ARDrone2
 * 
 * Copyright (c) Bart Slinger
 * 
 */
 
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "txb_config.h"
#include "cppm.h"
#include "ota.h"
#include "at_commands.h"
#include "navdata.h"
#include "telnet_debug.h"

/* Configuration */
const char* drone_ip = "192.168.1.1";       /* IP Address of the ARDrone2         */

/* Global variables */
char ssid[20] = {" "};                      /* SSID to be filled by the WiFi-scan */
const char* pass = "";                      /* ARDrone2 does not have a password by default */

enum txboxy_states {
  TXB_INITIALIZING,
  TXB_CPPMSTART,
  TXB_WIFISCAN,
  TXB_CONNECTING,
  TXB_CONNECTED,
  TXB_OTA_UPDATE,
  TXB_PREFLIGHT,
  TXB_ARMED,
  TXB_TAKING_OFF,
  TXB_FLYING,
  TXB_LANDING,
  TXB_KILLED,
  TXB_UNKILL,
} txb_state = TXB_INITIALIZING;

enum preflight_modes{
  preflight_starting,
  preflight_status_received,
  preflight_atcom_sent,
  preflight_navdata_ready,
  preflight_check_stream,
  preflight_navdata_streaming,
  preflight_set_max_angle,
  preflight_set_max_vspeed,
  preflight_set_max_rate,
  preflight_set_max_alt,
} preflight_mode = preflight_starting;

void setup() {
  /* Setup serial link for testing */
  Serial.begin(115200);

  /* Configure pins */
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  //SMART(debug_client).write("nodata");
  //Serial.println("Debugger not crashed");
  
  /* CPPM initialization */
  cppm_init();
  
  /* Update state */
  txb_state = TXB_CPPMSTART;
}

void loop() {
  /* Always update CPPM values from the transmitter */
  cppm_update();

  /* Check for telnet debug clients */
  //debug_check_clients();
  
  /* State machine */
  switch(txb_state) {
    
    case TXB_INITIALIZING:
      /* State should have changed in setup */
      break;

    case TXB_CPPMSTART:
      /* Capture current stick position and remember for later */
      if (cppm.ok) {
        cppm_capture_special_position();  // Value saved in cppm.captured_position;
        digitalWrite(LED_PIN, LED_ON);
        Serial.printf("Pos %d\n", cppm.captured_position);
        /* Continue to scan WiFi */
        txb_state = TXB_WIFISCAN;
      }
      //txb_state = TXB_WIFISCAN;           ///< THIS SHOULD BE REMOVED/COMMENTED, ONLY FOR TESTING
      break;

    case TXB_WIFISCAN:
      digitalWrite(LED_PIN, LED_OFF);
      /* Scan networks for an ARDrone2 */
      cppm_pause(); ///< To prevent ESP crash
      WiFi.mode(WIFI_STA);
      while(!find_ardrone2()) 
      {
        delay(500);
      }
      cppm_continue();
      /* ARDrone2 found, continue to connect */
      txb_state = TXB_CONNECTING;
      break;

    case TXB_CONNECTING:
      /* Connect to the ARDrone2 using ssid from find_ardrone2().
       * Only try 50 times, then go back to WiFi scan mode         */
      {
        Serial.print("Connecting to "); Serial.println(ssid);
        
        cppm_pause();///< To prevent ESP crash
        WiFi.begin(ssid, pass);
        delay(500);
        cppm_continue();
        
        int attempts = 0;
        bool success = false;
        for (int attempts = 0; attempts < 50; attempts++) {
          if (WiFi.status() == WL_CONNECTED) {
            success = true;
            Serial.println();
            break; // leave for-loop
          }
          delay(500);
          Serial.print(".");
        }

        /* Change state according to connection result */
        if (success) {
          //debug_init();
          txb_state = TXB_CONNECTED;
        }
        else {
          txb_state = TXB_WIFISCAN;
        }
      }
      break;
    
    case TXB_CONNECTED:
      /* Can go two ways, either flash mode or UDP setup */
      /* (1) OTA Mode (Over the air update) */
      if (cppm.captured_position == TOP_RIGHT) {
        ota_init();
        digitalWrite(BUILTIN_LED, LED_ON);
        txb_state = TXB_OTA_UPDATE;
      }
      /* (2) Proceed to establish UDP link */
      else {
        navdata_init();
        at_connect_udp();
        /* Continue to pre-flight */
        txb_state = TXB_PREFLIGHT;
      }
      break;

    case TXB_OTA_UPDATE:
      /* Perform OTA update */
      ota_handle_client();
      break;

    case TXB_PREFLIGHT:
      /* Process navdata information */
      process_preflight();
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      if (preflight_mode == preflight_navdata_streaming) {
        digitalWrite(LED_PIN, LED_ON);
        txb_state = TXB_ARMED;
      }
      break;

    case TXB_ARMED:
      navdata_get_status();
      if (cppm.values[TAKEOFF_SWITCH] < CPPM_MIN_COMMAND) {
        /* Takeoff */
        Serial.println("Taking off");
        at_ref(AT_REF_TAKEOFF);
        txb_state = TXB_TAKING_OFF;
      }
      flight_common();
      break;

    case TXB_TAKING_OFF:
      navdata_get_status();
      if (navdata_state(ARDRONE_FLY_MASK)) {
        Serial.println("Flying");
        txb_state = TXB_FLYING;
      } else {
        at_ref(AT_REF_TAKEOFF);
      }
      flight_common();
      break;

    case TXB_FLYING:
      if (cppm.values[TAKEOFF_SWITCH] > CPPM_MAX_COMMAND) {
        Serial.println("LANDING");
        at_ref(0);
        txb_state = TXB_LANDING;
      }
      flight_common();
      break;

    case TXB_LANDING:
      navdata_get_status();
      if (!navdata_state(ARDRONE_FLY_MASK)) {
        Serial.println("Landed!");
        txb_state = TXB_ARMED;
      } else {
        at_ref(0);
      }
      flight_common();
      break;

    case TXB_KILLED:
      at_kill();
      navdata_get_status();
      if (cppm.values[KILL_SWITCH] < CPPM_MIN_COMMAND && cppm.values[TAKEOFF_SWITCH] > CPPM_MAX_COMMAND) {
        Serial.println("Try to unkill");
        at_ref(AT_REF_EMERGENCY);
        txb_state = TXB_UNKILL;
      }
      break;

    case TXB_UNKILL:
      /* Only perform actions when new drone state is received */
      if(navdata_get_status()) {
        Serial.println(drone_state, HEX);
        if (!navdata_state(ARDRONE_EMERGENCY_MASK)) {
          Serial.println("Switching back to armed mode");
          txb_state = TXB_ARMED;
        }
        else {
          /* Try again to unset emergency bit */
          at_ref(AT_REF_EMERGENCY);        
        }
      }
      else {
        navdata_send_dummy_packet();
        at_ref(0); // keep watchdog alive
      }
      break;

    default:
      break;
  }

  /* Provide some time for the WiFi loop */
  delay(15);
}

void process_preflight()
{
  switch(preflight_mode) {
    
    case preflight_starting:
      /* Check if there is data on UDP */
      if(navdata_get_status()) {
        preflight_mode = preflight_set_max_angle; //preflight_navdata_streaming; //preflight_status_received;
      } else {
        navdata_send_dummy_packet();
      }
      break;

    case preflight_status_received:
      navdata_get_status();
      /* Check if in bootstrap mode */
      if (!navdata_state(ARDRONE_NAVDATA_BOOTSTRAP)) {
        at_config("general:navdata_demo", "TRUE");
        preflight_mode = preflight_atcom_sent;
      }
      else {
        //at_config("general:navdata_demo", "TRUE");
        //preflight_mode = preflight_navdata_ready;
      }
      break;

    case preflight_atcom_sent:
      if(navdata_get_status()) {
        if(navdata_state(ARDRONE_COMMAND_MASK)) {
          preflight_mode = preflight_navdata_ready;
        }
      }
      break;

    case preflight_set_max_angle:
      if(at_config_with_confirmation("control:euler_angle_max", "0.52")) {
        preflight_mode = preflight_set_max_vspeed;
      }
      break;

    case preflight_set_max_vspeed:
      if(at_config_with_confirmation("control:control_vz_max", "2000")) {
        preflight_mode = preflight_set_max_rate;
      }
      break;

    case preflight_set_max_rate:
      if(at_config_with_confirmation("control:control_yaw", "6.11")) {
        preflight_mode = preflight_set_max_alt;
      }
      break;

    case preflight_set_max_alt:
      if(at_config_with_confirmation("control:altitude_max", "100000")) {
        preflight_mode = preflight_navdata_streaming;
      }

    case preflight_navdata_streaming:
      /* For now it is sufficient to only receive drone state */
      break;

    default:
      break;
  }
}


/* Common functionality for flying-related states */
void flight_common() {
  /* Check for kill signal */
  if (cppm.values[KILL_SWITCH] > CPPM_MAX_COMMAND) {
    Serial.println("Kill signal sent");
    at_kill();
    txb_state = TXB_KILLED;
    return;
  }

  /* Check if crashed */
  if (navdata_state(ARDRONE_EMERGENCY_MASK)) {
    Serial.println("Crashed!");
    txb_state = TXB_UNKILL;
  }
  
  /* Send PCMD */
  at_pcmd(cppm.values);
}

enum at_config_states{
  sending_at,
  at_config_sent,
  send_ack,
  ack_sent,
} at_config_state = sending_at;

bool at_config_with_confirmation(const char* arg_1, const char* arg_2)
{
  /* Only on reception of new navdata */
  if (navdata_get_status()) {
    switch(at_config_state) {
      
      case sending_at:
        at_config(arg_1, arg_2);
        at_config_state = at_config_sent;
        break;

      case at_config_sent:
        if (navdata_state(ARDRONE_COMMAND_MASK)) {
          at_config_state = send_ack;
        } 
        else {
          at_config_state = sending_at;
        }
        break;

      case send_ack:
        at_ack();
        at_config_state = ack_sent;
        break;

      case ack_sent:
        if (!navdata_state(ARDRONE_COMMAND_MASK)) {
          /* Confirmed, reset */
          at_config_state = sending_at;
          return true;
        }
        else {
          at_config_state = send_ack;
        }
        break;
    }
  }
  return false;
}

/* 
 *  Scan WiFi for ardrone2
 *  Return true if found.
 *  Return false if no ardrone was found.
 */
bool find_ardrone2() {
  Serial.println("Starting WiFi scan");
  int n = WiFi.scanNetworks();
  Serial.println("WiFi scan done");
  
  if (n == 0) {
    Serial.println("No networks found");
    return false;
  }
  
  Serial.printf("Found %d networks:\n", n);
  
  int best_rssi = -9999;
  bool ardrone_detected = false;
  for (int i = 0; i < n; i++) {
    /* Check if the SSID starts with "ardrone2_" */
    char str[] ="ardrone2_";
    char tmp[20];
    WiFi.SSID(i).toCharArray(tmp, 20);
    char *pos = strstr (tmp, str);
    
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.print(")");
    Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");

    /* ARDrone2 found */
    if (pos) {
      Serial.print("Found "); Serial.println(WiFi.SSID(i));
      if (WiFi.RSSI(i) > best_rssi) {
        ardrone_detected = true;
        best_rssi = WiFi.RSSI(i);
        WiFi.SSID(i).toCharArray(ssid, 20);
      }
    }
    delay(10);    
  }
  
  if (ardrone_detected) {
    return true;
  }
  
  return false;
}

