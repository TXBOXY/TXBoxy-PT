/*
 * TXBoxy-PT firmware for ARDrone2
 * Copyright (C) 2016 Bart Slinger
 * 
 * This file is part of TXBoxy-PT.
 *
 * TXBoxy-PT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TXBoxy-PT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with TXBoxy-PT.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "txb_config.h"
#include "cppm.h"
#include "ota.h"

/* Configuration */
const char* ardrone_ip = "192.168.1.1";
const char* bebop_ip = "192.168.42.1";       /* IP Address of the ARDrone2         */
const char* esp_ip   = "192.168.4.1";        /* IP Address of ESP8266 chip         */

/* Global variables */
char ssid[20] = {" "};                      /* SSID to be filled by the WiFi-scan */
const char* hotspotpass = "LHlqaSgu";
const char* pass = "";                      /* ARDrone2 does not have a password by default */
IPAddress myIP;
IPAddress ardrone_broadcast_ip(192,168,1,255);
IPAddress bebop_broadcast_ip(192,168,42,255);
IPAddress hotspot_broadcast_ip(10,42,0,255);

WiFiClient tn;
WiFiUDP pprz_udp;
uint16_t pprz_udp_rx_port = 4242;
uint16_t pprz_udp_tx_port = 4243;
uint8_t ac_id = 0;
char msgBuffer[256]; //buffer to hold incoming packet
uint8_t tn_readback_i = 0;
const char psaux_cmd[] = "ps | grep ap.elf\n";
const char ardrone2_start_cmd[] = "/data/video/paparazzi/ap.elf > /dev/null 2>&1 &\n";
const char bebop_start_cmd[]    = "/data/ftp/internal_000/paparazzi/ap.elf > /dev/null 2>&1 &\n";

enum drone_types {
  ARDRONE2,
  BEBOP,
  BEBOP2,
  HOTSPOT,
} drone_type = ARDRONE2;

enum txboxy_states {
  TXB_INITIALIZING,
  TXB_CPPMSTART,
  TXB_WIFISCAN,
  TXB_CONNECTING,
  TXB_CONNECTED,
  TXB_OTA_UPDATE,
  TXB_PREFLIGHT,
  TXB_TELNET_CONNECTING,
  TXB_TELNET_REQUESTING_PSAUX,
  TXB_TELNET_WAITING_PSAUX,
  TXB_TELNET_START_AP,
  TXB_START_UDP,
  TXB_READ_AC_ID,
  TXB_STREAMING,
} txb_state = TXB_INITIALIZING;

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

  /* EEPROM init */
  EEPROM.begin(512); // only 512 bytes available, but is sufficient ;-)
  /*EEPROM.write(0, 'a');
  EEPROM.write(1, 'a');
  EEPROM.write(2, 'a');
  EEPROM.write(3, '\0');
  EEPROM.commit();*/
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
        /* In case of bottom left, proceed immediately with wifi scan */
        if (cppm.captured_position == BOTTOM_LEFT) {
          txb_state = TXB_WIFISCAN;
        }
        else {
          /* Get SSID from memory and try to connect */
          for (int i = 0; i < sizeof(ssid); i++) {
            ssid[i] = EEPROM.read(i);
          }
          drone_type = (drone_types) EEPROM.read(21);
          txb_state = TXB_CONNECTING;
        }
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
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(500);
      }
      digitalWrite(LED_PIN, LED_OFF);
      cppm_continue();
      /* ARDrone2 found, continue to connect */
      txb_state = TXB_CONNECTING;
      break;

    case TXB_CONNECTING:
      /* Connect to the SSID.
       * Only try 60 times, then go back to WiFi scan mode         */
      {
        Serial.print("Connecting to "); Serial.println(ssid);
        
        cppm_pause();///< To prevent ESP crash
        if (drone_type == HOTSPOT) {
          WiFi.begin(ssid, hotspotpass); 
        }
        else {
          WiFi.begin(ssid, pass);
        }
        delay(500);
        
        int attempts = 0;
        bool success = false;
        for (int attempts = 0; attempts < 60; attempts++) {
          if (WiFi.status() == WL_CONNECTED) {
            success = true;
            Serial.println();
            break; // leave for-loop
          }
          digitalWrite(LED_PIN, !digitalRead(LED_PIN));
          delay(500);
          Serial.print(".");
        }
        
        /* Change state according to connection result */
        if (success) {
          myIP = WiFi.localIP();
          Serial.print("IP:"); Serial.println(myIP);
          //debug_init();
          txb_state = TXB_CONNECTED;
        }
        else {
          txb_state = TXB_WIFISCAN;
        }
      }
      break;
    
    case TXB_CONNECTED:
      /* Connection succesful, save to EEPROM: */
      for (int i = 0; i < sizeof(ssid); i++) {
        EEPROM.write(i, ssid[i]);
      }
      EEPROM.write(21, drone_type);
      EEPROM.commit();
      /* Can go two ways, either flash mode or UDP setup */
      /* (1) OTA Mode (Over the air update) */
      if (cppm.captured_position == TOP_RIGHT) {
        ota_init();
        digitalWrite(BUILTIN_LED, LED_ON);
        txb_state = TXB_OTA_UPDATE;
      }
      /* (2) Proceed to establish UDP link */
      else {
        /* Continue to pre-flight */
        txb_state = TXB_PREFLIGHT;
        Serial.println("Preflight");
      }
      break;

    case TXB_OTA_UPDATE:
      /* Perform OTA update */
      static uint8_t ota_led = 0;
      if (ota_led == 5) digitalWrite(LED_PIN, LED_OFF);
      if (ota_led == 0)  digitalWrite(LED_PIN, LED_ON);
      ota_led++; ota_led %= 80;
      ota_handle_client();
      break;

    case TXB_PREFLIGHT:
      /* Process  */
      if (drone_type == BEBOP || drone_type == BEBOP2) {
        tn.connect(bebop_ip, 23);
        txb_state = TXB_TELNET_CONNECTING;
      }
      else if (drone_type == ARDRONE2) {
        tn.connect(ardrone_ip, 23);
        txb_state = TXB_TELNET_CONNECTING;
      }
      else if (drone_type == HOTSPOT) {
        txb_state = TXB_START_UDP;
      }
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      break;

    case TXB_TELNET_CONNECTING:
      if (tn.connected()) {
        txb_state = TXB_TELNET_REQUESTING_PSAUX;
      }
      break;

    case TXB_TELNET_REQUESTING_PSAUX:
      while (tn.available()) Serial.print(tn.read());
      delay(10);
      tn.write(psaux_cmd, sizeof(psaux_cmd));
      tn_readback_i = 0;
      txb_state = TXB_TELNET_WAITING_PSAUX;
      break;
    
    case TXB_TELNET_WAITING_PSAUX:
    {
      if (tn.connected()) {
        if (tn.available() && tn_readback_i < 255) {
          msgBuffer[tn_readback_i] = tn.read();
          Serial.print(msgBuffer[tn_readback_i]);
          tn_readback_i++;
        }
        else {
          /* Parse respone, see if ap.elf is running */
          char str[] ="paparazzi";
          msgBuffer[tn_readback_i] = '\0';
          char *pos = strstr(msgBuffer, str);
          if(pos) {
            Serial.println("FOUND running ap.elf");
            txb_state = TXB_START_UDP;
          }
          else { 
            Serial.println("ap.elf not found!");
            txb_state = TXB_TELNET_START_AP;
          }
        }
      }
      break;
    }

    case TXB_TELNET_START_AP:
      Serial.println("Starting ap.elf");
      if (drone_type == BEBOP) {
        tn.write(bebop_start_cmd, sizeof(bebop_start_cmd));
      }
      else {
        tn.write(ardrone2_start_cmd, sizeof(ardrone2_start_cmd));
      }
      txb_state = TXB_START_UDP;
      break;
    
    case TXB_START_UDP:
      Serial.println("Start UDP");
      pprz_udp.begin(pprz_udp_rx_port);
      txb_state = TXB_READ_AC_ID;
      break;

    case TXB_READ_AC_ID:
    {
      /* Check for UDP data from host */
      int packetSize = pprz_udp.parsePacket();
      int len = 0;
      if(packetSize) { /* data received on udp line*/
        Serial.println("Rcv packet");
        // read the packet into packetBufffer
        len = pprz_udp.read(msgBuffer, 256);
        ac_id = (uint8_t) msgBuffer[2];
        //Serial.print("Sender: ");
        //Serial.println((uint8_t)packetBuffer[2]);
        digitalWrite(LED_PIN, LED_ON);
        cppm_continue();
        txb_state = TXB_STREAMING;
      }
      break;
    }

    case TXB_STREAMING:
      send_pprz_rc_msg();
      break;

    default:
      break;
  }

  /* Provide some time for the WiFi loop */
  delay(15);
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
    
    char bebop[] = "Bebop";
    char *pos2 = strstr (tmp, bebop);

    char esp[] = "e540";
    char *pos3 = strstr (tmp, esp);
    
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
        drone_type = ARDRONE2;
        best_rssi = WiFi.RSSI(i);
        WiFi.SSID(i).toCharArray(ssid, 20);
      }
    }
    /* Bebop found */
    else if (pos2) {
      Serial.print("Found "); Serial.println(WiFi.SSID(i));
      if (WiFi.RSSI(i) > best_rssi) {
        drone_type = BEBOP;
        ardrone_detected = true;
        best_rssi = WiFi.RSSI(i);
        WiFi.SSID(i).toCharArray(ssid, 20);
      }
    }
    /* Hotspot found */
    else if (pos3) {
      Serial.print("Found "); Serial.println(WiFi.SSID(i));
      if (WiFi.RSSI(i) > best_rssi) {
        drone_type = HOTSPOT;
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

/*
 * <message name="RC_4CH" id="52" link="broadcasted">
      <field name="ac_id"       type="uint8"/>
      <field name="mode"        type="uint8"/>
      <field name="throttle"    type="uint8"/>
      <field name="roll"        type="int8"/>
      <field name="pitch"       type="int8"/>
      <field name="yaw"         type="int8"/>
    </message>
 */
void send_pprz_rc_msg() {
  size_t len = 12;
  uint8_t buff[len];
  uint8_t crcA = 0;
  uint8_t crcB = 0;
  
  /* Start byte */
  buff[0] = 0x99;
  
  /* Length */
  buff[1] = 12;
  
  /* Sender ID */
  buff[2] = 0;
  
  /* Msg ID */
  buff[3] = 52;
  
  /* Payload AC ID*/
  buff[4] = ac_id; // AC ID

  /* Payload mode switch */
  uint8_t MODE_SWITCH = AUX1;
  if (cppm.values[MODE_SWITCH] > CPPM_MAX_COMMAND) {
    buff[5] = 2;
  }
  else if (cppm.values[MODE_SWITCH] > CPPM_MIN_COMMAND) {
    buff[5] = 1;
  }
  else {
    buff[5] = 0;
  }

  /* Payload 4 channels */
  buff[6] = map(cppm.values[THROTTLE], 1000, 2000, 0, 127); // throttle
  buff[7] = map(cppm.values[AILERON], 1000, 2000, -128, 127); // roll
  buff[8] = map(cppm.values[ELEVATOR], 1000, 2000, 127, -128); // pitch
  buff[9] = map(cppm.values[RUDDER], 1000, 2000, -128, 127); // yaw

  /* CRC calculation */
  for (int i = 1; i < 10; i++) {
    crcA += buff[i];
    crcB += crcA;
  }
  buff[10] = crcA;
  buff[11] = crcB;

  if (drone_type == BEBOP || drone_type == BEBOP2) {
    pprz_udp.beginPacketMulticast(bebop_broadcast_ip, pprz_udp_tx_port, myIP);
  }
  else if (drone_type == ARDRONE2) {
    pprz_udp.beginPacketMulticast(ardrone_broadcast_ip, pprz_udp_tx_port, myIP);
  }
  else {
    pprz_udp.beginPacketMulticast(hotspot_broadcast_ip, pprz_udp_tx_port, myIP);
  }
  pprz_udp.write(buff, len);
  pprz_udp.endPacket();
  //Serial.println("sent");
}

