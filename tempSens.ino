#include <OneWire.h>

// message from computer
String msg = "";

// one wire object
OneWire  ds(10);

// array of addresses
byte addrArr [10][8];

// device amount
byte sensAmount = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  initSens();
  getTemp();
}


void loop() {
  if (Serial.available() > 0) {
    // read the incoming byte:
    msg = Serial.readString();

    // compare string to getTemp
    if (msg.startsWith("getTemp")) {
      getTemp();
    }

    if (msg.startsWith("init")) {
      Serial.print("*** init");
      initSens();
    }

    if (msg.startsWith("getSens")) {
      Serial.println("get Sens info");
      getSens();
    }
  }
}

void initSens() {
  byte addr[8];
  byte j; //index of byte
  byte i = 0; //index of device
  while (ds.search(addr))
  {
    Serial.println(i);
    for ( j = 0; j < 8; j++) {
      Serial.write(' ');
      Serial.print(addr[j]);
      addrArr[i][j] = addr[j];
    }
    i++;
    sensAmount++;
    Serial.println();
  }
}

void getSens() {
  byte addr[8];
  byte j; //index of byte
  byte i; //index of device
  for (i = 0; i < sensAmount; i++) {
    Serial.println(i);
    for ( j = 0; j < 8; j++) {
      Serial.print(addrArr[i][j], HEX);
    }
    Serial.println();
  }
}

void getTemp() {
  byte i,j;
  byte present = 0;
  byte data[12];
  byte addr[8];
  float celsius;


  
  for (i = 0; i < sensAmount; i ++) {
        
    ds.reset();
    ds.select(addrArr[i]);
    // start conversion, with parasite power on at the end
    ds.write(0x44, 1);

    delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.

    ds.reset();
    ds.select(addrArr[i]);
    ds.write(0xBE);         // Read Scratchpad

    for ( j = 0; j < 9; j++) {           // we need 9 bytes
      data[j] = ds.read();
    }
    celsius = convert(data);

    Serial.print(celsius);
    Serial.print(" ");
  }
  Serial.println();
}


float convert(byte data[9]) {
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  byte type_s;
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
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
    return (float)raw / 16.0;
  }
}
