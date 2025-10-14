#include "EITKitArduino.h"

EITKitArduino *eit = nullptr;

void setup() {
  eit = new EITKitArduino(4,1,4, AD, AD);
}

void loop() {
  if (!eit) {
    Serial.println("EIT not set!");
  } else {
    eit->take_measurements(AD, AD);
    // Example for checking attributes of EITKitArduino
    double* rms_array = eit->get_magnitude_array();
    for (int i = 0; i < NUM_MEAS; i++) {
     Serial.print(i);
     Serial.print(": ");
     Serial.println(rms_array[i]);
    }
  }
  delay(5);

}
