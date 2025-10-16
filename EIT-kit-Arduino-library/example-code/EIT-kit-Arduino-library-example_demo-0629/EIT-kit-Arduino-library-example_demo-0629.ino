#include "EITKit.h"
#include "Parser.h"
#include <string>
#include <iostream>     // std::cout
#include <iomanip>
#include <sstream>      // std::stringstream

EITKit *eit = nullptr;

#define SET ":SET"
#define GET ":GET"
#define VOLTAGE ":VOLTAGE"
#define CURRENT ":CURRENT"
#define RMS ":RMS"
#define PHASE ":PHASE"
#define GAIN ":GAIN"

#define f_calibrate "/calibrate"
#define f_measurement "/measurement"

auto ident_space = " ";
auto ident_slash = "/";
auto ident_vec_start = "[";
auto ident_vec_end = "]";
auto ident_map_start = "{";
auto ident_map_end = "}";
auto ident_method = ":";

String normalizeSpaces(String s) {
    String out = "";
    bool inSpace = false;
    for (uint i = 0; i < s.length(); ++i) {
        char c = s[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (!inSpace) {
                out += ' ';
                inSpace = true;
            }
        } else {
            out += c;
            inSpace = false;
        }
    }
    return out;
}

// 現状のパース機構は複数文字空白を強制的に正規化するので string ("...") を受け付けない
Parser::request parseInput(String input) {
  input.replace(",", "");
  input.replace("[", "[ ");
  input.replace("]", " ]");
  input.replace("{", "{ ");
  input.replace("}", " }");
  input = normalizeSpaces(input);
  input.trim();
  Serial.println(input);

  int start = input.indexOf(ident_map_start);
  int end = input.indexOf(ident_map_end);
  String req = input.substring(start+1, end);
  req.trim();
  Serial.println(req);

  String method = req.substring(req.indexOf(ident_method), req.indexOf(ident_space));
  req = req.substring(req.indexOf(ident_space) + 1);

  String path = req.substring(0, req.indexOf(ident_space));
  req = req.substring(req.indexOf(ident_space) + 1);

  String args = req.substring(req.indexOf(ident_vec_start), req.indexOf(ident_vec_end) + 1);
  
  return { method, path, args };
}

std::vector<String> parsePath(String path) {
  std::vector<String> result;
  String rest = path.substring(1, path.length());
  String section = "";
  while (0 != rest.indexOf(ident_slash)) {
    String section = rest.substring(0, rest.indexOf(ident_slash));
    result.push_back(section);
    if (-1 == rest.indexOf(ident_slash)) break;
    rest = rest.substring(rest.indexOf(ident_slash) + 1);
  }
  return result;
}

void routePath(std::vector<String> path, String args) {

}

void setup() {
  Serial.begin(115200); // Teensy 4.0 は UART 経由ではなく full-spec USB を使えるので、baudrate は意味がない
  eit = new EITKit();

  eit->calibrate();
  eit->take_measurements(8, AD);
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
    Serial.println("Enter data: ");
    while (Serial.available() == 0) {}
    String input = Serial.readString().c_str();
    
    auto request = parseInput(input);

    Serial.print("method: ");
    Serial.println(request.method);
    
    auto path = parsePath(request.path);
    Serial.print("path: [");
    for (uint i = 0; i < path.size(); i++) {
      Serial.print(path[i]);
      Serial.print(" ");
    }
    Serial.println("]");

    Serial.print("args: ");
    Serial.println(request.args);
  }
  delay(1000);
}
