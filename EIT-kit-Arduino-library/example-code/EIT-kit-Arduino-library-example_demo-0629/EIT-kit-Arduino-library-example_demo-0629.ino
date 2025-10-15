#include "EITKitArduino.h"
#include <string>
#include <iostream>     // std::cout
#include <iomanip>
#include <sstream>      // std::stringstream

EITKitArduino *eit = nullptr;

void setup() {
  eit = new EITKitArduino(4,1,4, AD, AD);
  eit->take_measurements(AD, AD);
  // Example for checking attributes of EITKitArduino
  double* rms_array = eit->get_magnitude_array();

  std::string result = "GET/RMS ";

  std::ostringstream streamObj;
  streamObj << std::fixed;
  // Set precision to 4 digits
  streamObj << std::setprecision(4);
  streamObj << "GET/RMS";
  streamObj << " ";
  streamObj << "[";
  for (int i = 0; i < NUM_ELECTRODES; i++) {
    streamObj << "[ ";
    for (int j = 0; j < NUM_ELECTRODES; j++) {
      streamObj << rms_array[i * NUM_ELECTRODES + j];
      streamObj << " ";
    }
    streamObj << "]\n";
  }
  streamObj << "]\n";
  Serial.println(streamObj.str().c_str());
}

void loop() {
  if (!eit) {
    Serial.println("EIT not set!");
  } else {
  }
  delay(1000);
}
