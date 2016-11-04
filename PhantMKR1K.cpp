/**
 * PhantMKR1K.cpp
 *
 *This library is made for using Sparkfun's Cloud environment called Phant.
 *The code was leveraged from Sparkfun's Phant library and was modified by ForceTronics to work on the 
 *Arduino MKR1000. It was not tested on other Arduino platfomrs
 */

#include "Arduino.h"
#include "PhantMKR1K.h"

//#ifdef ARDUINO_ARCH_AVR
//#include <avr/pgmspace.h>
//#else
//#include "pgmspace.h"
//#endif

static const char HEADER_POST_URL1[] PROGMEM = "POST /input/";
static const char HEADER_POST_URL2[] PROGMEM = ".txt HTTP/1.1\n";
static const char HEADER_PHANT_PRV_KEY[] PROGMEM = "Phant-Private-Key: ";
static const char HEADER_CONNECTION_CLOSE[] PROGMEM = "Connection: close\n";
static const char HEADER_CONTENT_TYPE[] PROGMEM = "Content-Type: application/x-www-form-urlencoded\n";
static const char HEADER_CONTENT_LENGTH[] PROGMEM = "Content-Length: ";

PhantMKR1K::PhantMKR1K(String host, String publicKey, String privateKey) {
  _host = host;
  _pub = publicKey;
  _prv = privateKey;
  _params = "";
}

void PhantMKR1K::add(String field, String data) {

  _params += "&" + field + "=" + data;

}

void PhantMKR1K::add(String field, char data) {

  _params += "&" + field + "=" + String(data);

}


void PhantMKR1K::add(String field, int data) {

  _params += "&" + field + "=" + String(data);

}


void PhantMKR1K::add(String field, byte data) {

  _params += "&" + field + "=" + String(data);

}

void PhantMKR1K::add(String field, long data) {

  _params += "&" + field + "=" + String(data);

}

void PhantMKR1K::add(String field, unsigned int data) {

  _params += "&" + field + "=" + String(data);

}

void PhantMKR1K::add(String field, unsigned long data) {

  _params += "&" + field + "=" + String(data);

}

String PhantMKR1K::queryString() {
  return String(_params);
}

String PhantMKR1K::url() {

  String result = "http://" + _host + "/input/" + _pub + ".txt";
  result += "?private_key=" + _prv + _params;

  _params = "";

  return result;

}

String PhantMKR1K::get() {

  String result = "GET /output/" + _pub + ".csv HTTP/1.1\n";
  result += "Host: " + _host + "\n";
  result += "Connection: close\n";

  return result;

}

String PhantMKR1K::post() {

  String params = _params.substring(1);
  String result;
  //String result = "POST /input/" + _pub + ".txt HTTP/1.1\n";
  for (int i=0; i<strlen(HEADER_POST_URL1); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_POST_URL1 + i);
  }
  result += _pub;
  for (int i=0; i<strlen(HEADER_POST_URL2); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_POST_URL2 + i);
  }
  result += "Host: " + _host + "\n";
  //result += "Phant-Private-Key: " + _prv + "\n";
  for (int i=0; i<strlen(HEADER_PHANT_PRV_KEY); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_PHANT_PRV_KEY + i);
  }
  result += _prv + '\n';
  //result += "Connection: close\n";
  for (int i=0; i<strlen(HEADER_CONNECTION_CLOSE); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONNECTION_CLOSE + i);
  }
  //result += "Content-Type: application/x-www-form-urlencoded\n";
  for (int i=0; i<strlen(HEADER_CONTENT_TYPE); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONTENT_TYPE + i);
  }	
  //result += "Content-Length: " + String(params.length()) + "\n\n";
  for (int i=0; i<strlen(HEADER_CONTENT_LENGTH); i++)
  {
    result += (char)pgm_read_byte_near(HEADER_CONTENT_LENGTH + i);
  }	
  result += String(params.length()) + "\n\n";
  result += params;

  _params = "";
  return result;

}

String PhantMKR1K::clear() {

  String result = "DELETE /input/" + _pub + ".txt HTTP/1.1\n";
  result += "Host: " + _host + "\n";
  result += "Phant-Private-Key: " + _prv + "\n";
  result += "Connection: close\n";

  return result;

}
