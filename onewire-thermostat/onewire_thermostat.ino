#include <OneWire.h>
// Based on
// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

// Read temperature from ds1820 and switch ds2405 on when temperature is below
// 10°C and switch ds2405 off when temperature is above 12°C.

OneWire  dst(10);  // Temperature on pin 10 (a 4.7K resistor is necessary)
OneWire  dss(9);  // Switch on pin 11

void PrintBytes(byte* addr, uint8_t count, bool newline=0) {
  for (uint8_t i = 0; i < count; i++) {
    Serial.print(addr[i]>>4, HEX);
    Serial.print(addr[i]&0x0f, HEX);
  }
  if (newline)
    Serial.println();
}

float temperature(OneWire* ds, uint8_t* addr)
{
  int i;
  byte data[12];
  float celsius;
  ds->reset();
  ds->select(addr);

  ds->write(0x44, 1);        // start conversion, with parasite power on at the end
  delay(1000);     // maybe 750ms is enough, maybe not
  ds->reset();
  ds->select(addr);
  ds->write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds->read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (0) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  return celsius;
}

void setup(void) {
  Serial.begin(9600);
}

void loop(void) {
  byte taddr[8];
  byte saddr[8];
  byte saaddr[8];
  float celsius;
  boolean pstate = true;
  boolean sstate = true;

// pseudo code
// target_search(temperature)
// read(temperature)
// target_search(switch)
// target_search_active(switch)
// if under threshold && switch not active -> toggle
// elif over threshold && switch active -> toggle

  while (1)
  {
    if ( !dst.search(taddr)) {
      Serial.println("Didn't find temperature sensor!");
      dst.reset_search();
      delay(250);
      return;
    }
    if ( !dss.search(saddr)) {
      Serial.println("Didn't find switch!");
      dss.reset_search();
      delay(250);
      return;
    }

    while(1)
    {
      celsius = temperature(&dst, taddr);
      Serial.println(celsius);
      if (!dss.search(saaddr, false))
      {
        Serial.println("Power on");
        sstate = true;
      }
      else
      {
        Serial.println("Power off");
        sstate = false;
      }
      dss.reset_search();
      if (celsius < 10 && sstate == false)
      {
        Serial.println("Switching on");
        pstate = true;
        dss.reset();
        dss.select(saddr);
      }
      if (celsius > 12 && sstate == true)
      {
        Serial.println("Switching off");
        pstate = false;
        dss.reset();
        dss.select(saddr);
      }

      delay(60000);
    }
  }
}
