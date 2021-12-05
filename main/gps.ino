/*

  GPS module

  Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <TinyGPS++.h>
#include "configuration.h"

uint32_t LatitudeBinary;
uint32_t LongitudeBinary;
uint16_t altitudeGps;
uint8_t hdopGps;
uint8_t sats;
int speed;
char t[32]; // used to sprintf for Serial output

TinyGPSPlus _gps;
HardwareSerial _serial_gps(GPS_SERIAL_NUM);

void gps_time(char * buffer, uint8_t size) {
    snprintf(buffer, size, "%02d:%02d:%02d", _gps.time.hour(), _gps.time.minute(), _gps.time.second());
}

float gps_latitude() {
    return _gps.location.lat();
}

float gps_distanceBetween(float last_lat, float last_lon, float lat, float lon) {
    return _gps.distanceBetween(last_lat, last_lon, lat, lon);
}

float gps_longitude() {
    return _gps.location.lng();
}

float gps_altitude() {
    return _gps.altitude.meters();
}

float gps_hdop() {
    return _gps.hdop.hdop();
}

uint8_t gps_sats() {
    return _gps.satellites.value();
}

void gps_setup() {
    _serial_gps.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

static void gps_loop() {
    while (_serial_gps.available()) {
        _gps.encode(_serial_gps.read());
    }
}


#if defined(PAYLOAD_USE_MAPPER)
// Same format as CubeCell mappers
void buildPacket(uint8_t txBuffer[11]) {
  LatitudeBinary = ((_gps.location.lat() + 90) / 180.0) * 16777215;
  LongitudeBinary = ((_gps.location.lng() + 180) / 360.0) * 16777215;
  altitudeGps = (uint16_t)_gps.altitude.meters();
  speed = (uint16_t)_gps.speed.kmph(); // convert from float
  sats = _gps.satellites.value();
  // hdopGps = _gps.hdop.value() / 10;

  sprintf(t, "Lat: %f", _gps.location.lat());
  Serial.println(t);
  sprintf(t, "Lng: %f", _gps.location.lng());
  Serial.println(t);
  sprintf(t, "Alt: %f", _gps.altitude.meters());
  Serial.println(t);
//  sprintf(t, "Hdop: %d", hdopGps);
//  Serial.println(t);
  sprintf(t, "Sats: %d", sats);
  Serial.println(t);

  txBuffer[0] = (LatitudeBinary >> 16) & 0xFF;
  txBuffer[1] = (LatitudeBinary >> 8) & 0xFF;
  txBuffer[2] = LatitudeBinary & 0xFF;
  txBuffer[3] = (LongitudeBinary >> 16) & 0xFF;
  txBuffer[4] = (LongitudeBinary >> 8) & 0xFF;
  txBuffer[5] = LongitudeBinary & 0xFF;
  txBuffer[6] = (altitudeGps >> 8) & 0xFF;
  txBuffer[7] = altitudeGps & 0xFF;

  txBuffer[8] = ((unsigned char *)(&speed))[0];

  uint16_t batteryVoltage = ((float_t)((float_t)(axp.getBattVoltage()) / 10.0) + .5);
  txBuffer[9] = (uint8_t)((batteryVoltage - 200) & 0xFF);
 
  txBuffer[10] = sats & 0xFF;
}

#elif defined(PAYLOAD_USE_FULL)

    // More data than PAYLOAD_USE_CAYENNE
    void buildPacket(uint8_t txBuffer[10])
    {
        LatitudeBinary = ((_gps.location.lat() + 90) / 180.0) * 16777215;
        LongitudeBinary = ((_gps.location.lng() + 180) / 360.0) * 16777215;
        altitudeGps = _gps.altitude.meters();
        hdopGps = _gps.hdop.value() / 10;
        sats = _gps.satellites.value();

        sprintf(t, "Lat: %f", _gps.location.lat());
        Serial.println(t);
        sprintf(t, "Lng: %f", _gps.location.lng());
        Serial.println(t);
        sprintf(t, "Alt: %d", altitudeGps);
        Serial.println(t);
        sprintf(t, "Hdop: %d", hdopGps);
        Serial.println(t);
        sprintf(t, "Sats: %d", sats);
        Serial.println(t);

        txBuffer[0] = ( LatitudeBinary >> 16 ) & 0xFF;
        txBuffer[1] = ( LatitudeBinary >> 8 ) & 0xFF;
        txBuffer[2] = LatitudeBinary & 0xFF;
        txBuffer[3] = ( LongitudeBinary >> 16 ) & 0xFF;
        txBuffer[4] = ( LongitudeBinary >> 8 ) & 0xFF;
        txBuffer[5] = LongitudeBinary & 0xFF;
        txBuffer[6] = ( altitudeGps >> 8 ) & 0xFF;
        txBuffer[7] = altitudeGps & 0xFF;

        txBuffer[8] = hdopGps & 0xFF;
        txBuffer[9] = sats & 0xFF;
    }

#elif defined(PAYLOAD_USE_CAYENNE)

    // CAYENNE DF
    void buildPacket(uint8_t txBuffer[11])
    {
        sprintf(t, "Lat: %f", _gps.location.lat());
        Serial.println(t);
        sprintf(t, "Lng: %f", _gps.location.lng());
        Serial.println(t);        
        sprintf(t, "Alt: %f", _gps.altitude.meters());
        Serial.println(t);        
        int32_t lat = _gps.location.lat() * 10000;
        int32_t lon = _gps.location.lng() * 10000;
        int32_t alt = _gps.altitude.meters() * 100;
        
        txBuffer[2] = lat >> 16;
        txBuffer[3] = lat >> 8;
        txBuffer[4] = lat;
        txBuffer[5] = lon >> 16;
        txBuffer[6] = lon >> 8;
        txBuffer[7] = lon;
        txBuffer[8] = alt >> 16;
        txBuffer[9] = alt >> 8;
        txBuffer[10] = alt;
    }

#endif
