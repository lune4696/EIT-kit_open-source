struct Request {
  String method;
  String path;
  String args;
};

enum Method { SET, GET, };
enum Func { CALI, MEAS, };
enum Property { RMS, MAG, PHASE, GAIN, };