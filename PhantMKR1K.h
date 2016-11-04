/**
 * PhantMKR1K.h
 *
 *This library is made for using Sparkfun's Cloud environment called Phant.
 *The code was leveraged from Sparkfun's Phant library and was modified by ForceTronics to work on the 
 *Arduino MKR1000. It was not tested on other Arduino platfomrs
 */

#ifndef PhantMKR1K_h
#define PhantMKR1K_h

#include "Arduino.h"

class PhantMKR1K {

  public:
    PhantMKR1K(String host, String publicKey, String privateKey);
    void add(String field, String data);
    void add(String field, char data);
    void add(String field, int data);
    void add(String field, byte data);
    void add(String field, long data);
    void add(String field, unsigned int data);
    void add(String field, unsigned long data);

    String queryString();
    String url();
    String get();
    String post();
    String clear();

  private:
    String _pub;
    String _prv;
    String _host;
    String _params;
};

#endif
