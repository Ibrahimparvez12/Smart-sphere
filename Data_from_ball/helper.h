#ifndef _HELPER_H
#define _HELPER_H

// header for 12C
#include <Wire.h> 
// header for SD card
#include <SPI.h>
#include <SdFat.h>
// header for BNO055
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
// header for BLE
#include "ble_helper.h"

#define CSV_DELIM ','
#define CSV_BASE_NAME "Data"
#define CAL_NAME "Calibration"

const uint8_t CS = 0;

SdFat SD;
// Error messages stored in flash.
#define error(msg) SD.errorHalt(F(msg))

File csvFile, calFile;
const uint8_t BASE_NAME_SIZE = sizeof(CSV_BASE_NAME) - 1;
char csvName[] = CSV_BASE_NAME "00.csv";
char calName[] = CAL_NAME ".csv";

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);


void init_SD_card() {
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(CS, SD_SCK_MHZ(50))) {
    bleuart.println("Card failed, or not present");
    SD.initErrorHalt();
  }
  bleuart.print("SD initialized.");
  Serial.println("SD initialized.");
}


void init_BNO() {
  Serial.println("Initializing BNO055... \n");
  /* HELPER the sensor */
  if (!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    bleuart.print("BNO error");
    while (1);
  }
  Serial.print("BNO initialized.");
  bleuart.println("BNO initialized.");
}


void createCSV() {
  while (SD.exists(csvName)) {
    if (csvName[BASE_NAME_SIZE + 1] != '9') {
      csvName[BASE_NAME_SIZE + 1]++;
    } else if (csvName[BASE_NAME_SIZE] != '9') {
      csvName[BASE_NAME_SIZE + 1] = '0';
      csvName[BASE_NAME_SIZE]++;
    } else {
      bleuart.print(F("Can't create file name"));
      error(F("Can't create file name"));
    }
  }
  //open csv file
  csvFile = SD.open(csvName, FILE_WRITE);
  if (!csvFile) {
    bleuart.print(F("CSV open failed"));
    error(F("CSV open failed"));
  }
  bleuart.print(F("opened: "));
  bleuart.println(csvName);
}


unsigned int csvAvailableForWrite() {
  return csvFile.availableForWrite();
}


void writeCSVheader() {
  csvFile.println("Time,Delay,Qw,Qx,Qy,Qz,Ex,Ey,Ez,Ax,Ay,Az,Mx,My,Mz,Gx,Gy,Gz,LAx,LAy,LAz,GRVx,GRVy,GRVz,Temp");
}


void writeCSV(String* dataString) {
  //csvFile.write(dataString->c_str(), *chunkSize);
  csvFile.println(*dataString);
}


void syncCSV() {
  // Force data to SD and update the directory entry to avoid data loss.
  if (!csvFile.sync() || csvFile.getWriteError()) {
    bleuart.print(F("CSV file write error"));
    error("CSV file write error");
  }
}

void syncCAL() {
  // Force data to SD and update the directory entry to avoid data loss.
  if (!calFile.sync() || calFile.getWriteError()) {
    bleuart.print(F("CAL file write error"));
    error("CAL file write error");
  }
  bleuart.print(F("Calbiration data stored."));
  delay(3000);
}


void closeCSV() {
  csvFile.close();
}


void displaySensorOffsets(const adafruit_bno055_offsets_t &calibData) {
  String data = "";
  data += "Acc offset: \n";
  data += "X: ";
  data += String(calibData.accel_offset_x);
  data += ", ";
  data += "Y: ";
  data += String(calibData.accel_offset_y);
  data += ", ";
  data += "Z: ";
  data += String(calibData.accel_offset_z);
  bleuart.print(data);
  
  data = "";
  data += "Mag offset: \n";
  data += "X: ";
  data += String(calibData.gyro_offset_x);
  data += ", ";
  data += "Y: ";
  data += String(calibData.gyro_offset_y);
  data += ", ";
  data += "Z: ";
  data += String(calibData.gyro_offset_z);
  bleuart.print(data);

  data = "";
  data += "Gyro offset: \n";
  data += "X: ";
  data += String(calibData.mag_offset_x);
  data += ", ";
  data += "Y: ";
  data += String(calibData.mag_offset_y);
  data += ", ";
  data += "Z: ";
  data += String(calibData.mag_offset_z);
  bleuart.print(data);

  data = "";
  data += "Acc radius: \n";
  data += String(calibData.accel_radius);
  bleuart.print(data);

  data = "";
  data += "Mag radius: \n";
  data += String(calibData.mag_radius);
  bleuart.println(data);
}


void displayCalStatus(void) {
  /* Get the four calibration values (0..3) */
  /* Any sensor data reporting 0 should be ignored, */
  /* 3 means 'fully calibrated" */
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);
  String data = "";
  /* Display the individual values */
  data += "Sys: ";
  data += String(system);
  data += ", ";
  data += "G: ";
  data += String(gyro);
  data += ", ";
  data += "A: ";
  data += String(accel);
  data += ", ";
  data += "M: ";
  data += String(mag);

  bleuart.print(data);
}


void Calibrate() {
  // Use external cystal for better accuracy
  bno.setExtCrystalUse(true);
  while (!bno.isFullyCalibrated()){
    sensors_event_t event;
    bno.getEvent(&event);

    /* Optional: Display calibration status */
    displayCalStatus();

    /* New line for the next sample */
    bleuart.println("");

    /* Wait the specified delay before requesting new data */
    delay(1000);
  }
}


void MagCalibrate() {
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;

  while (mag != 3){
    bno.getCalibration(&system, &gyro, &accel, &mag);

    /* Optional: Display calibration status */
    displayCalStatus();

    /* New line for the next sample */
    bleuart.println("");

    /* Wait the specified delay before requesting new data */
    delay(1000);
  }
}

void SysCalibrate() {
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;

  while (system != 3){
    bno.getCalibration(&system, &gyro, &accel, &mag);

    /* Optional: Display calibration status */
    displayCalStatus();

    /* New line for the next sample */
    bleuart.println("");

    /* Wait the specified delay before requesting new data */
    delay(1000);
  }
}

/*
 * Read a file one field at a time.
 *
 * file - File to read.
 *
 * str - Character array for the field.
 *
 * size - Size of str array.
 *
 * delim - csv delimiter.
 *
 * return - negative value for failure.
 *          delimiter, '\n' or zero(EOF) for success.
 */
int csvReadText(File* file, char* str, size_t size, char delim) {
  char ch;
  int rtn;
  size_t n = 0;
  while (true) {
    // check for EOF
    if (!file->available()) {
      rtn = 0;
      break;
    }
    if (file->read(&ch, 1) != 1) {
      // read error
      rtn = -1;
      break;
    }
    // Delete CR.
    if (ch == '\r') {
      continue;
    }
    if (ch == delim || ch == '\n') {
      rtn = ch;
      break;
    }
    if ((n + 1) >= size) {
      // string too long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;
  }
  str[n] = '\0';
  return rtn;
}


int csvReadInt32(File* file, int32_t* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtol(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}


int csvReadInt16(File* file, int16_t* num, char delim) {
  int32_t tmp;
  int rtn = csvReadInt32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp < INT_MIN || tmp > INT_MAX) return -5;
  *num = tmp;
  return rtn;
}


void readCalibration() {

  adafruit_bno055_offsets_t calibrationData;

  if(SD.exists(calName)) {
    calFile = SD.open(calName);
    if (!calFile) {
      bleuart.print(F("Unable to open CSV file"));
      error(F("Unable to open CSV file"));
    }
    bleuart.print(F("Found calibration data : "));
    bleuart.println(calName);
    delay(2000);

    // Read calibration values from csv file 
    calFile.seek(0);

    int16_t accel_offset_x; /**< x acceleration offset */
    int16_t accel_offset_y; /**< y acceleration offset */
    int16_t accel_offset_z; /**< z acceleration offset */

    int16_t mag_offset_x; /**< x magnetometer offset */
    int16_t mag_offset_y; /**< y magnetometer offset */
    int16_t mag_offset_z; /**< z magnetometer offset */

    int16_t gyro_offset_x; /**< x gyroscrope offset */
    int16_t gyro_offset_y; /**< y gyroscrope offset */
    int16_t gyro_offset_z; /**< z gyroscrope offset */

    int16_t accel_radius; /**< acceleration radius */

    int16_t mag_radius; /**< magnetometer radius */

    while (calFile.available()) {
      if ( csvReadInt16(&calFile, &accel_offset_x, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &accel_offset_y, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &accel_offset_z, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &gyro_offset_x, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &gyro_offset_y, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &gyro_offset_z, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &mag_offset_x, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &mag_offset_y, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &mag_offset_z, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &accel_radius, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&calFile, &mag_radius, CSV_DELIM) != CSV_DELIM != '\n') {
        bleuart.println("read error");
        int ch;
        int nr = 0;
        // print part of file after error.
        while ((ch = calFile.read()) > 0 && nr++ < 100) {
          bleuart.write(ch);
        }
        break;
      }
    }

    calibrationData.accel_offset_x = accel_offset_x;
    calibrationData.accel_offset_y = accel_offset_y;
    calibrationData.accel_offset_z = accel_offset_z;

    calibrationData.mag_offset_x = mag_offset_x;
    calibrationData.mag_offset_y = mag_offset_y;
    calibrationData.mag_offset_z = mag_offset_z;

    calibrationData.gyro_offset_x = gyro_offset_x;
    calibrationData.gyro_offset_y = gyro_offset_y;
    calibrationData.gyro_offset_z = gyro_offset_z;

    calibrationData.accel_radius = accel_radius;
    calibrationData.mag_radius = mag_radius;

    //and write to calibration registers
    displaySensorOffsets(calibrationData);
    delay(3000);
    bleuart.print("Restoring Calibration data to the BNO055...");
    Serial.println("\n\nRestoring Calibration data to the BNO055...");
    bno.setSensorOffsets(calibrationData);

    /* Crystal must be configured AFTER loading calibration data into BNO055. */
    bno.setExtCrystalUse(true);

    delay(2000);
    bleuart.println("Move ball around to calibrate magnetometers");
    Serial.println("Move ball around to calibrate magnetometers");
    delay(2000);
    MagCalibrate();
    bleuart.print("Calibrate the System");
    Serial.println("Calibrate the System");
    delay(3000);
    SysCalibrate();
  }
  else{
    bleuart.println(F("Calibration data doesn't exist!"));
    bleuart.println("Please Calibrate Sensor: ");
    Serial.println(F("Calibration data doesn't exist!"));
    Serial.println("Please Calibrate Sensor: ");
    delay(3000);
    Calibrate();
  }  

  //close file
  calFile.close();
}


void storeCalibration() {
  // Remove existing file.
  SD.remove(calName);

  calFile = SD.open(calName, FILE_WRITE);
  if (!calFile) {
    bleuart.print(F("Unable to open Calibration file"));
    error(F("Unable to open Calibration file"));
  }
  bleuart.print(F("opened: "));
  bleuart.println(calName);

  // Get new calibration data
  adafruit_bno055_offsets_t newCalib;
  bno.getSensorOffsets(newCalib);
  displaySensorOffsets(newCalib);

  String calibOffsets = "";
  calibOffsets += String(newCalib.accel_offset_x);
  calibOffsets += String(",");
  calibOffsets += String(newCalib.accel_offset_y);
  calibOffsets += String(",");
  calibOffsets += String(newCalib.accel_offset_z);
  calibOffsets += String(",");

  calibOffsets += String(newCalib.gyro_offset_x);
  calibOffsets += String(",");
  calibOffsets += String(newCalib.gyro_offset_y);
  calibOffsets += String(",");
  calibOffsets += String(newCalib.gyro_offset_z);
  calibOffsets += String(",");

  calibOffsets += String(newCalib.mag_offset_x);
  calibOffsets += String(",");
  calibOffsets += String(newCalib.mag_offset_y);
  calibOffsets += String(",");
  calibOffsets += String(newCalib.mag_offset_z);
  calibOffsets += String(",");

  calibOffsets += String(newCalib.accel_radius);
  calibOffsets += String(",");
  calibOffsets += String(newCalib.mag_radius);
  calibOffsets += "\r\n";
  

  // Save calibration data to csv file
  calFile.print(calibOffsets);

  //close file
  syncCAL();
  bleuart.println();
  calFile.close();
}

#endif  // _HELPER_H