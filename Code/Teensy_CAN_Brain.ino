/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2022 Bolder Flight Systems Inc
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the “Software”), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#include "FlexCAN_T4.h"

/* Loopback test on Teensy 3.6 can0 transmitting to can1 */

FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_256> can1;
FlexCAN_T4<CAN2, RX_SIZE_16, TX_SIZE_256> can2;

CAN_message_t tx_msg, rx_msg;


void irq(const CAN_message_t &ref) {
  Serial.print("Received ID: ");
  Serial.println(ref.id);

  //Serial.print("MB "); Serial.print(ref.mb);
  //Serial.print("  OVERRUN: "); Serial.print(ref.flags.overrun);
  //Serial.print("  LEN: "); Serial.print(ref.len);
  //Serial.print(" EXT: "); Serial.print(ref.flags.extended);
  //Serial.print(" TS: "); Serial.print(ref.timestamp);
  //Serial.print(" ID: "); Serial.print(ref.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < ref.len; i++ ) {
    Serial.print(ref.buf[i], HEX); Serial.print(" "); 
  } Serial.println(); Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("STARTING TEST");
  /* Enable the CAN transceivers */
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  digitalWriteFast(26, LOW);
  digitalWriteFast(27, LOW);
  /* Start the CAN bus and set the baud */
  can1.begin();
  can1.setBaudRate(1000000);
  can2.begin();
  can2.setBaudRate(1000000);
  can2.setRFFN(RFFN_32); // 32 filters
  can2.enableFIFO();  // enable FIFO
  can2.setMRP(0);  // prioritize FIFO
  can2.enableFIFOInterrupt();
  can2.onReceive(FIFO, irq);
  /* Set some random filters */
  can1.setFIFOFilter(REJECT_ALL);
  can1.setFIFOFilter(0, 1, STD);
  can1.setFIFOFilter(1, 2, STD);
  can1.setFIFOFilter(2, 3, STD);
  can1.setFIFOFilter(3, 4, STD);
  can1.setFIFOFilter(4, 5, STD);
  /* Transmit data sequentially */
  for (int i = 0; i < 24; i++) {
    //Serial.print("For Loop Iteration: ");
    //Serial.println(i);
    tx_msg.id = i;
    tx_msg.seq = 1;
    can1.write(tx_msg);
  }
}

/*https://startingelectronics.org/software/arduino/learn-to-program-course/19-serial-input/ */
char rx_byte = 0;
String rx_str = "";
int rx_int = 0;


void loop() {
  if (Serial.available() > 0) {    // is a character available?
    rx_byte = Serial.read();       // get the character
    
    if (rx_byte != '\n') {
      // a character of the string was received
      rx_str += rx_byte;
    }
    else {
      // end of string
      rx_int = rx_str.toInt(); //convert string to integer
      Serial.print("RPM Sent: ");
      Serial.println(rx_int);
      rx_msg.buf[0] = rx_int;  //sets duty cycle: 255 max
      rx_msg.id = 1; //sets id of message
      rx_msg.seq = rx_int;
      Serial.println(rx_msg.buf[0]);
      can1.write(rx_msg);
      rx_str = "";                // clear the string for reuse
      
    }
  } // end: if (Serial.available() > 0)
}
