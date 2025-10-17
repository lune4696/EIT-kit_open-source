#include "EITKit.h"
#include "Parser.h"
#include "magic_enum.hpp"

#include <algorithm>    // std::find
#include <iostream>     // std::cout
#include <iomanip>
#include <string>
#include <sstream>      // std::stringstream

EITKit *eit = nullptr;

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

void stringifyVec2(std::ostringstream& streamObj, std::vector<std::vector<double>> v) {
  streamObj << std::fixed;
  // Set precision to 4 digits
  streamObj << std::setprecision(4);

  streamObj << "[";
  for (uint i = 0; i < v.size(); i++) {
    streamObj << "[ ";
    for (uint j = 0; j < v[i].size(); j++) {
      streamObj << v[i][j];
      streamObj << " ";
    }
    streamObj << "]";
  }
  streamObj << "]";
}

std::vector<std::vector<double>> pick_rms(std::vector<std::vector<EITKit::measured>> result) {
  std::vector<std::vector<double>> rms(result.size());
  for (uint i = 0; i < result.size(); i++) {
    rms[i].resize(result[i].size());
  }
  for (uint i = 0; i < result.size(); i++) {
    for (uint j = 0; j < result[i].size(); j++) {
      rms[i][j] = result[i][j].rms;
    }
  }
  return rms;
}

std::vector<std::vector<double>> pick_mag(std::vector<std::vector<EITKit::measured>> result) {
  std::vector<std::vector<double>> mag(result.size());
  for (uint i = 0; i < result.size(); i++) {
    mag[i].resize(result[i].size());
  }
  for (uint i = 0; i < result.size(); i++) {
    for (uint j = 0; j < result[i].size(); j++) {
      mag[i][j] = result[i][j].mag;
    }
  }
  return mag;
}

std::vector<std::vector<double>> pick_phase(std::vector<std::vector<EITKit::measured>> result) {
  std::vector<std::vector<double>> phase(result.size());
  for (uint i = 0; i < result.size(); i++) {
    phase[i].resize(result[i].size());
  }
  for (uint i = 0; i < result.size(); i++) {
    for (uint j = 0; j < result[i].size(); j++) {
      phase[i][j] = result[i][j].phase;
    }
  }
  return phase;
}

// 現状のパース機構は複数文字空白を強制的に正規化するので string ("...") を受け付けない
Request parseInput(String input) {
  input.replace(",", "");
  input.replace("[", "[ ");
  input.replace("]", " ]");
  input.replace("{", "{ ");
  input.replace("}", " }");
  input = normalizeSpaces(input);
  input.trim();

  int start = input.indexOf(ident_map_start);
  int end = input.indexOf(ident_map_end);
  String req = input.substring(start+1, end);
  req.trim();

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

std::vector<String> parseArgs(String args) {
  std::vector<String> result;
  String rest = args.substring(2, args.length() - 2);
  String section = "";
  while (true) {
    String section = rest.substring(0, rest.indexOf(ident_space));
    result.push_back(section);
    if (-1 == rest.indexOf(ident_space)) break;
    rest = rest.substring(rest.indexOf(ident_space) + 1);
  }
  return result;
}

void routing(std::ostringstream& streamObj, std::vector<String> path, String args) {
  switch (magic_enum::enum_cast<Func>(path[0].c_str()).value()) {
    case Func::CALI: {
        eit->calibrate();
      }
      break;

    case Func::MEAS: {
        //auto t = micros();
        auto result = eit->take_measurements(8, AD); // 10.9 ms (8, AD)
        //Serial.print("time.take_measurements: ");
        //Serial.println(micros() - t);
        auto argvec = parseArgs(args);
        String RMS(magic_enum::enum_name(Property::RMS).data());
        String MAG(magic_enum::enum_name(Property::MAG).data());
        String PHASE(magic_enum::enum_name(Property::PHASE).data());

        streamObj << "{ :GET { ";
        if (argvec.end() != std::find(argvec.begin(), argvec.end(), RMS)) {
          streamObj << ":RMS ";
          auto v = pick_rms(result);
          stringifyVec2(streamObj, v);
          streamObj << " ";
        }
        if (argvec.end() != std::find(argvec.begin(), argvec.end(), MAG)) {
          streamObj << ":MAG ";
          auto v = pick_mag(result);
          stringifyVec2(streamObj, v);
          streamObj << " ";
        }
        if (argvec.end() != std::find(argvec.begin(), argvec.end(), PHASE)) {
          streamObj << ":PHASE ";
          auto v = pick_phase(result);
          stringifyVec2(streamObj, v);
          streamObj << " ";
        }
        streamObj << "} }";
      }
      break;

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200); // Teensy 4.0 は UART 経由ではなく full-spec USB を使えるので、baudrate は意味がない
  Serial.setTimeout(1);
  eit = new EITKit();
  eit->calibrate();
}

void loop() {
  std::ostringstream streamObj;
  while (Serial.available() == 0) {}

  String input = Serial.readStringUntil('\n').c_str();
  auto request = parseInput(input);
  auto path = parsePath(request.path);

  routing(streamObj, path, request.args);
  Serial.println(streamObj.str().c_str());
}
