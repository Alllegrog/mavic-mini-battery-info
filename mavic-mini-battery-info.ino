
 /*
Thanks to:
  - czipis - for the original from which this was forked - https://github.com/czipis/mavic-mini-battery-info
  - PowerCartel - for smart battery routines - https://github.com/PowerCartel/PackProbe
  - Bodmer - for TFT eSPI library - https://github.com/Bodmer/
  - Alain Aeropic - for BatMan inspiration - https://www.thingiverse.com/thing:4235767 
*/

#define VERSION   "v1.2"

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <PolledTimeout.h>
#include <SPI.h>
#include <Wire.h>

byte deviceAddress = 11;

// Standard and common non-standard Smart Battery commands
#define BATTERY_MODE             0x03
#define TEMPERATURE              0x08
#define VOLTAGE                  0x09
#define CURRENT                  0x0A
#define RELATIVE_SOC             0x0D
#define ABSOLUTE_SOC             0x0E
#define REMAINING_CAPACITY       0x0F
#define FULL_CHARGE_CAPACITY     0x10
#define TIME_TO_FULL             0x13
#define CHARGING_CURRENT         0x14
#define CHARGING_VOLTAGE         0x15
#define BATTERY_STATUS           0x16
#define CYCLE_COUNT              0x17
#define DESIGN_CAPACITY          0x18
#define DESIGN_VOLTAGE           0x19
#define SPEC_INFO                0x1A
#define MFG_DATE                 0x1B
#define SERIAL_NUM               0x1C
#define MFG_NAME                 0x20   // String
#define DEV_NAME                 0x21   // String
#define CELL_CHEM                0x22   // String
#define MFG_DATA                 0x23   // String
#define CELL4_VOLTAGE            0x3C
#define CELL3_VOLTAGE            0x3D
#define CELL2_VOLTAGE            0x3E
#define CELL1_VOLTAGE            0x3F
#define STATE_OF_HEALTH          0x4F
#define DJI_SERIAL               0xD8  // String

#define TFT_TXT  TFT_WHITE
#define TFT_BG   TFT_BLACK
#define TFT_GREY 0x6A6C6E
#include "dji_logo-48x48.h"
#include "battery_icon.h"

#define bufferLen 32
size_t wordBufferLen = 2;
uint8_t i2cBuffer[bufferLen];

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup_Select.h/Setup2_ST7735.h

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  tft.init();
  tft.setRotation(0);  // portrait
  tft.fillScreen(TFT_BG);
  tft.drawXBitmap((tft.width() - logoWidth)/2, (tft.height() - logoHeight)/2-40, dji_logo, logoWidth, logoHeight, TFT_TXT);
  tft.setTextSize(1);
  tft.setTextColor(TFT_TXT, TFT_BG);
  tft.drawCentreString("Mavic Mini", 64, 68, 2);
  tft.drawCentreString("Battery Info", 64, 90, 1);
  tft.drawCentreString(VERSION, 64, 136, 1);

  Serial.println("DJI Mavic Mini Battery Info");
  delay(3000);
}

uint8_t read_byte()
{
  while (1)
  {
    if (Wire.available())
    {
      return Wire.read();
    }
  }
}

int fetchWord(byte func)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(func);
  Wire.endTransmission(false);
  delay(1);// FIX wire bug
  Wire.requestFrom(deviceAddress, wordBufferLen, true);  
  uint8_t b1 = read_byte();
  uint8_t b2 = read_byte();
  Wire.endTransmission();
  return (int)b1 | ((( int)b2) << 8);
}

uint8_t i2c_smbus_read_block ( uint8_t command, uint8_t* blockBuffer, size_t blockBufferLen )
{
  uint8_t x, num_bytes;
  Wire.beginTransmission(deviceAddress);
  Wire.write(command);
  Wire.endTransmission(false);
  delay(1);
  Wire.requestFrom(deviceAddress, blockBufferLen, true);
  
  num_bytes = read_byte();
  num_bytes = constrain(num_bytes, 0, blockBufferLen - 2);
  for (x = 0; x < num_bytes - 1; x++) { // -1 because x=num_bytes-1 if x<y; last byte needs to be "nack"'d, x<y-1
    blockBuffer[x] = read_byte();
  }
  blockBuffer[x++] = read_byte(); // this will nack the last byte and store it in x's num_bytes-1 address.
  blockBuffer[x] = 0; // and null it at last_byte+1
  Wire.endTransmission();
  return num_bytes;
}

void loop()
{
  Wire.beginTransmission (deviceAddress);
  if (Wire.endTransmission () == 0)
  {
    Serial.println("  - device found");
    tft.fillScreen(TFT_BG);
    queryDevice();
  }
  tft.drawCentreString("  + --------  ", 64, 136, 1);
    delay(250);
  tft.drawCentreString("   + -------  ", 64, 136, 1);
    delay(230);
  tft.drawCentreString("  - + ------  ", 64, 136, 1);
    delay(210);
  tft.drawCentreString("  -- + -----  ", 64, 136, 1);
    delay(190);
  tft.drawCentreString("  --- + ----  ", 64, 136, 1);
    delay(170);
  tft.drawCentreString("  ---- + ---  ", 64, 136, 1);
    delay(170);
  tft.drawCentreString("  ----- + --  ", 64, 136, 1);
    delay(190);
  tft.drawCentreString("  ------ + -  ", 64, 136, 1);
    delay(210);
  tft.drawCentreString("  ------- +   ", 64, 136, 1);
    delay(230);
  tft.drawCentreString("  -------- +  ", 64, 136, 1);
    delay(250);
  tft.drawCentreString("  -------- +  ", 64, 136, 1);
    delay(250);
  tft.drawCentreString("  ------- +   ", 64, 136, 1);
    delay(230);
  tft.drawCentreString("  ------ + -  ", 64, 136, 1);
    delay(210);
  tft.drawCentreString("  ----- + --  ", 64, 136, 1);
    delay(190);
  tft.drawCentreString("  ---- + ---  ", 64, 136, 1);
    delay(170);
  tft.drawCentreString("  --- + ----  ", 64, 136, 1);
    delay(170);
  tft.drawCentreString("  -- + -----  ", 64, 136, 1);
    delay(190);
  tft.drawCentreString("  - + ------  ", 64, 136, 1);
    delay(210);
  tft.drawCentreString("   + -------  ", 64, 136, 1);
    delay(230);
  tft.drawCentreString("  + --------  ", 64, 136, 1);
    delay(250);
}

void queryDevice()
{
  // Query device and write out to serial
  uint8_t length_read = 0;
    
  Serial.print("Manufacturer Name: ");
  length_read = i2c_smbus_read_block(MFG_NAME, i2cBuffer, bufferLen);
  Serial.write(i2cBuffer, length_read);
  Serial.println("");

  Serial.print("Device Name: ");
  length_read = i2c_smbus_read_block(DEV_NAME, i2cBuffer, bufferLen);
  Serial.write(i2cBuffer, length_read);
  Serial.println("");

  Serial.print("Chemistry ");
  length_read = i2c_smbus_read_block(CELL_CHEM, i2cBuffer, bufferLen);
  Serial.write(i2cBuffer, length_read);
  Serial.println("");

  Serial.print("Manufacturer Data: ");
  length_read = i2c_smbus_read_block(MFG_DATA, i2cBuffer, bufferLen);
  Serial.write(i2cBuffer, length_read);
  Serial.println("");

  Serial.print("Design Capacity: ");
  int designCapacity = (int)fetchWord(DESIGN_CAPACITY);
  Serial.println(designCapacity);

  Serial.print("Design Voltage: ");
  Serial.println(fetchWord(DESIGN_VOLTAGE));

  Serial.print("Manufacture Date (D/M/Y): ");
  String formatted_date;
  int mdate = fetchWord(MFG_DATE);
  int mday = B00011111 & mdate;
  int mmonth = mdate >> 5 & B00001111;
  int myear = 1980 + (mdate >> 9 & B01111111);
  formatted_date += mday;
  formatted_date += "/";
  formatted_date += mmonth;
  formatted_date += "/";
  formatted_date += myear;
  Serial.println(formatted_date);
  int str_len = formatted_date.length() + 1; 
  char mfg_date[str_len];
  formatted_date.toCharArray(mfg_date, str_len);

  Serial.print("Serial Number: ");
  String djiSerial;
  length_read = i2c_smbus_read_block(DJI_SERIAL, i2cBuffer, bufferLen);
  for(int i = 0; i < length_read; i++) {
    djiSerial += (char)i2cBuffer[i];
  }
  Serial.println(djiSerial);
  
  Serial.print("Specification Info: ");
  Serial.println(fetchWord(SPEC_INFO));

  Serial.print("Cycle Count: ");
  uint8_t cycles = fetchWord(CYCLE_COUNT);
  Serial.println(cycles);

  Serial.print("Voltage: ");
  float voltage = (float)fetchWord(VOLTAGE) / 1000;
  Serial.println(voltage);

  Serial.print("Full Charge Capacity: ");
  int fullCapacity = (int)fetchWord(FULL_CHARGE_CAPACITY);
  Serial.println(fullCapacity);
  
  Serial.print("Remaining Capacity: ");
  int remainingCapacity = (int)fetchWord(REMAINING_CAPACITY);
  Serial.println(remainingCapacity);

  Serial.print("Relative Charge(%): ");
  uint8_t charge = fetchWord(RELATIVE_SOC);
  Serial.println(charge);

  Serial.print("Absolute Charge(%): ");
  int chargePercent = (int)fetchWord(ABSOLUTE_SOC);
  Serial.println(chargePercent);

  Serial.print("Minutes remaining for full charge: ");
  Serial.println(fetchWord(TIME_TO_FULL));

  Serial.print("Cell 1 Voltage: ");
  float cell1 = (float)fetchWord(CELL1_VOLTAGE)/1000;
  Serial.println(cell1);
  Serial.print("Cell 2 Voltage: ");
  float cell2 = (float)fetchWord(CELL2_VOLTAGE)/1000;
  Serial.println(cell2);
  char buffer[5];
  String v = dtostrf(voltage, 4, 2, buffer);
  String c1 = dtostrf(cell1, 4, 2, buffer);
  String c2 = dtostrf(cell2, 4, 2, buffer);
  String cells = v + " " + c1 + '/' + c2;
  str_len = cells.length() + 1; 
  char cellsV[str_len];
  cells.toCharArray(cellsV, str_len);

  Serial.print("State of Health: ");
  Serial.println(fetchWord(STATE_OF_HEALTH));

  Serial.print("Battery Mode (BIN): 0b");
  Serial.println(fetchWord(BATTERY_MODE), BIN);

  Serial.print("Battery Status (BIN): 0b");
  Serial.println(fetchWord(BATTERY_STATUS), BIN);

  Serial.print("Charging Current: ");
  Serial.println(fetchWord(CHARGING_CURRENT));

  Serial.print("Charging Voltage: ");
  Serial.println(fetchWord(CHARGING_VOLTAGE));

  Serial.print("Temp: ");
  unsigned int tempk = fetchWord(TEMPERATURE);
  float temp = tempk / 10.0 - 273.15;
  Serial.println(temp);

  Serial.print("Current (mA): ");
  Serial.println(fetchWord(CURRENT));
  Serial.println("");
  
  // Write output to screen
  tft.fillScreen(TFT_BG);
  tft.drawXBitmap((tft.width() - logoWidth)/2, 0, dji_logo, logoWidth, logoHeight, TFT_TXT);

  tft.setTextSize(1);
  tft.setTextColor(TFT_TXT, TFT_BG);

  tft.drawString("SN", 3, 50, 1);
  tft.drawRightString(djiSerial, 126, 50, 1);

  tft.drawString("MFG", 3, 50+14, 1);
  tft.drawRightString(mfg_date, 126, 50+14, 1);

  tft.drawString("CYCLES", 3, 50+28, 1);
  tft.drawNumber(cycles, 45, 50+28, 1);
  tft.drawString("TEMP", 70, 50+28, 1);
  tft.drawFloat(temp, 1, 102, 50+28, 1);

  tft.drawString("VOLTS", 3, 50+42, 1);
  tft.drawRightString(cellsV, 126, 50+42, 1);

  tft.drawString("CAP", 3, 50+56, 1);
  String capacity = String(remainingCapacity) + "/" + String(fullCapacity) + "/" + String(designCapacity);
  tft.drawRightString(capacity, 126, 50+56, 1);

  tft.drawXBitmap((tft.width() - BATTERY_ICON_WIDTH)/2, (tft.height() - BATTERY_ICON_HEIGHT + BATTERY_ICON_H_OFFSET), battery_icon, BATTERY_ICON_WIDTH, BATTERY_ICON_HEIGHT, TFT_TXT);
  #define CHARGE_AREA_START_X     20
  #define CHARGE_AREA_START_Y     tft.height() - BATTERY_ICON_HEIGHT + BATTERY_ICON_H_OFFSET + 18
  #define CHARGE_AREA_WIDTH       83
  #define CHARGE_AREA_HEIGHT      28
  // determine color
  int batteryChargeWidth = (chargePercent * CHARGE_AREA_WIDTH) / 100;
  tft.fillRect(CHARGE_AREA_START_X, CHARGE_AREA_START_Y, batteryChargeWidth, CHARGE_AREA_HEIGHT, TFT_GREY);
  String batteryChargePercent = String(chargePercent) + "%";
  tft.setTextColor(TFT_TXT);
  tft.drawCentreString(batteryChargePercent, tft.width()/2, 50+77, 4);
  
  while(1) yield(); // We must yield() to stop a watchdog timeout.
}
