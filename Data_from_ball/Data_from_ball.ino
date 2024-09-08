/*
  Code to send data from the IMU of the ball to be used over Serial, 
  BLE and save data to the SD Card.
  The data logging only starts after the calibration sequence is completed or 
  if already existing calibration data is available.

  The SD card holds the calibration data and the Log files with data from the BNO055 IMU.
*/

#include "helper.h"

const int calibration_time = 180;
/* Set the delay between fresh samples */
uint16_t BNO055_SAMPLERATE_DELAY_MS = 10;

// make a string for assembling the data to log:
String dataString = "";
unsigned long lastMillis = 0;

// Sensor events
sensors_event_t orientationData, angVelocityData , linearAccelData, magnetometerData, accelerometerData, gravityData;

void printQuat(imu::Quaternion* quat) {
  double w = -1000000, x = -1000000, y = -1000000 , z = -1000000; //dumb values, easy to spot problem

  w = quat->w();
  x = quat->x();
  y = quat->y();
  z = quat->z();

  String data = "";
  /* Display the individual values */
  data += "Quaternion: ";
  data += "W: ";
  data += String(w);
  data += ", ";
  data += "X: ";
  data += String(x);
  data += ", ";
  data += "Y: ";
  data += String(y);
  data += ", ";
  data += "Z: ";
  data += String(z);

  bleuart.println(data);

  Serial.print(F("Quaternion: "));
  Serial.print((float)w, 4);
  Serial.print(F(", "));
  Serial.print((float)x, 4);
  Serial.print(F(", "));
  Serial.print((float)y, 4);
  Serial.print(F(", "));
  Serial.print((float)z, 4);
  Serial.println(F(""));

  dataString += String(w);
  dataString += ",";
  dataString += String(x);
  dataString += ",";
  dataString += String(y);
  dataString += ",";
  dataString += String(z);
  dataString += ",";
}


void printEvent(sensors_event_t* event) {
  String data = "";
  double x = -1000000, y = -1000000 , z = -1000000; //dumb values, easy to spot problem
  
  if (event->type == SENSOR_TYPE_ORIENTATION) {
    data +="Euler Angles: ";
    x = event->orientation.x;
    y = event->orientation.y;
    z = event->orientation.z;
  }
  else if (event->type == SENSOR_TYPE_ACCELEROMETER) {
    data +="Accl: ";
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD) {
    data +="Mag: ";
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  }
  else if (event->type == SENSOR_TYPE_GYROSCOPE) {
    data +="Gyro: ";
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_LINEAR_ACCELERATION) {
    data +="Linear Accl: ";
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_GRAVITY) {
    data +="Gravity: ";
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else {
    data +="Invalid sensor event\n";
  }

  /* Display the individual values */
  data += "X: ";
  data += String(x);
  data += ", ";
  data += "Y: ";
  data += String(y);
  data += ", ";
  data += "Z: ";
  data += String(z);
  
  bleuart.println(data);

  Serial.print("\tx= ");
  Serial.print(x);
  Serial.print(" |\ty= ");
  Serial.print(y);
  Serial.print(" |\tz= ");
  Serial.println(z);

  dataString += String(x);
  dataString += ",";
  dataString += String(y);
  dataString += ",";
  dataString += String(z);
  dataString += ",";
}


//Initialise everything & Wait for calibration to be done.
void setup() {

  // Initialise communications:
  Serial.begin(115200);
  #if CFG_DEBUG
    // Blocking wait for connection when debug mode is enabled via IDE
    while ( !Serial ) yield();
  #endif
  init_BLE();

  //wait for connecting to ble
  while(!BLE_connected) yield();
  //wait 8 seconds to open UART
  delay(5000);
  
  // // reserve 1kB for String used as a buffer
  // dataString.reserve(1024);

  // Initialise serial connection, SD card and BLE/Bluetooth. 
  init_SD_card();
  init_BNO();

  // TODO:
  // Get input if ball needs to be calibrated?
  //If yes, run calibration sequence ultil ball is calibrated
  
  bleuart.print("Perform Calibration Sequence? (y/n)");
  while (!bleuart.available()) yield();
  char ch = bleuart.read();

  if(ch == 'Y' || ch == 'y') {
    Calibrate();
  }
  else {
    // read calibration values from csv
    readCalibration();
  }

  bleuart.print("Fully calibrated!");
  displayCalStatus();
  bleuart.println("");
  delay(1000);
  
  bleuart.print("Storing calibration data ...");
  storeCalibration();
  

  //wait till L is pressed to start logging
  bleuart.println(F("Type L to start logging:"));
  while (!(bleuart.available() && (bleuart.read() == 'l' || bleuart.read() == 'L'))) {
    delay(100);
  }

  // Start logging data after device is calibrated and calibration data is saved to csv
  // create and open csv file to log data when calibration is done
  createCSV();
  bleuart.print(F("Logging to: "));
  bleuart.println(csvName);
  bleuart.print(F("!!! Type S to stop logging !!!"));
  delay(5000);

  // write header of csv file
  writeCSVheader();
}


// Get sensor data every sample rate (10ms)
void loop() {
  unsigned long now = millis();
  dataString = "";
  if ((now - lastMillis) >= BNO055_SAMPLERATE_DELAY_MS) {
    // add a new line to the dataString
    dataString += now;
    dataString += ",";

    bleuart.print("Time : ");
    
    bleuart.println(now);

    // Serial.print("Time delay: ");
    // Serial.println(now - lastMillis);
    bleuart.print("Time delay: ");
    bleuart.println(now - lastMillis);

    dataString += now - lastMillis;
    dataString += ",";
    
    // Read quaternion from IMU
    imu::Quaternion quat = bno.getQuat();

    /* Get a new sensor event */
    bno.getEvent(&orientationData, Adafruit_BNO055::Adafruit_BNO055::VECTOR_EULER);
    bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
    bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
    bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
    bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
    bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);

    //print quaternion
    printQuat(&quat);

    // print events
    printEvent(&orientationData);
    printEvent(&accelerometerData);
    printEvent(&magnetometerData);
    printEvent(&angVelocityData);
    printEvent(&linearAccelData);
    printEvent(&gravityData);
    int8_t boardTemp = bno.getTemp();
    Serial.println(boardTemp);
    bleuart.print("Temp: ");
    bleuart.println(boardTemp);
    //dataString += "\r\n";

    // Get temperature data
    dataString += boardTemp;

    lastMillis = now;
  }

  writeCSV(&dataString);

  // Close file and stop logging when S is sent
  if(bleuart.available() && (bleuart.read() == 's' || bleuart.read() == 'S')) {
    syncCSV();
    closeCSV();
    bleuart.println(F("Data logging ended!"));
    delay(500);
    while (true) {}
  }
}
