#include "EITKitArduino.h"
#include <string>
#include <iostream>     // std::cout
#include <iomanip>
#include <sstream>      // std::stringstream

EITKitArduino *eit = nullptr;

#define SET ":SET"
#define GET ":GET"
#define VOLTAGE ":VOLTAGE"
#define CURRENT ":CURRENT"
#define RMS ":RMS"
#define PHASE ":PHASE"
#define GAIN ":GAIN"

void setup() {
  eit = new EITKitArduino(8,1,4, AD, AD);
  eit->calibrate();
  eit->take_measurements(AD, AD);
  std::vector<std::vector<double>> rms = eit->get_rms();

  std::ostringstream streamObj;
  streamObj << std::fixed;
  // Set precision to 4 digits
  streamObj << std::setprecision(4);

  streamObj << GET;
  streamObj << " ";
  streamObj << RMS;
  streamObj << " ";

  streamObj << "[";
  for (uint i = 0; i < rms.size(); i++) {
    streamObj << "[ ";
    for (uint j = 0; j < rms[i].size(); j++) {
      streamObj << rms[i][j];
      streamObj << " ";
    }
    streamObj << "]";
  }
  streamObj << "]";
  Serial.println(streamObj.str().c_str());
}

void loop() {
  if (!eit) {
    Serial.println("EIT not set!");
  } else {
  }
  delay(1000);
}
