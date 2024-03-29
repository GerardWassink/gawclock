/* ------------------------------------------------------------------------- *
 * Name   : gawclock
 * Author : Gerard Wassink
 * Date   : December 2022
 * Purpose: Show time from RTC module on 7 segment displays
 * Versions:
 *   0.1  : Initial code base
 *   0.2  : Built in setting the clock
 *   0.3  : Credits for used libraries
 *          Code cleanup and more comments
 *   0.4  : Built in brightness adjusting
 *   1.0  : Some code cleanup
 *   1.1  : Updated pin numbers for new clock
 *          Default to show time when datetime button not implemented   
 *   
 * ------------------------------------------------------------------------- */
#define progVersion "1.1"                   // Program version definition
/* ------------------------------------------------------------------------- *
 *             GNU LICENSE CONDITIONS
 * ------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ------------------------------------------------------------------------- *
 *       Copyright (C) December 2022 Gerard Wassink
 * ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- *
 *             Credits to whom it is due                              DS3231
 * ------------------------------------------------------------------------- *
 * For the DS3231 clock module library:
 * 
 *   authors:
 *   - family-names: "Ayars"
 *     given-names: "Eric"
 *     orcid: "https://orcid.org/0000-0003-2150-6935"
 *   - family-names: "Wickert"
 *     given-names: "Andrew D."
 *     orcid: "https://orcid.org/0000-0000-0000-0000"
 *   - family-names: "Community"
 *     given-names: "Open Source Hardware"
 *   title: "DS3231"
 *   version: 1.1.0
 *   doi: 10.5281/zenodo.2008621
 *   date-released: 2021-12-06
 *   url: "https://github.com/NorthernWidget/DS3231"
 * ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- *
 *             Credits to whom it is due                              TM1637
 * ------------------------------------------------------------------------- *
 * For the TM1637 display module library:
 * 
 * author:
 * - avishorp
 * - url: https://github.com/avishorp/TM1637
 * ------------------------------------------------------------------------- */

#include <Arduino.h>
#include <TM1637Display.h>

#include <DS3231.h>
#include <Wire.h>

#define DEBUG 0

#if DEBUG == 1
  #define debugstart(x) Serial.begin(x)
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debugstart(x)
  #define debug(x)
  #define debugln(x)
#endif

/* -------------------------------------------------------------------------- *
 * Definitions for clock module
 * -------------------------------------------------------------------------- */
DS3231 myRTC;                       // --- clock object

bool century = false;               // century roll-over bit
bool h12Flag;                       // 12/ 24 flag returned using getHour()
bool pmFlag;                        // AM / PM flag returned using getHour()
bool showDots = true;               // dots blinker boolean

/* -------------------------------------------------------------------------- *
 * Definitions for display module
 * -------------------------------------------------------------------------- */
#define CLK 2                       // Module connection pins
#define DIO 3                       //  (Digital Pins)

TM1637Display display(CLK, DIO);    // display object


/* -------------------------------------------------------------------------- *
 * Global definitions for pins used
 * -------------------------------------------------------------------------- */
#define DATETIME 4                 // Switch between date & time display
#define CLOCKSET 5                 // Button to set clock values
#define CLOCKUP  6                 // move value up
#define CLOCKDWN 7                 // move value down

/* -------------------------------------------------------------------------- *
 * Global definitions
 * -------------------------------------------------------------------------- */
#define timeDispInterval 1000
long timeDispPreviousMillis = 1000; // Make timeouts work first time

int brightness = 1;

/* -------------------------------------------------------------------------- *
 * setup() initialization routine
 * -------------------------------------------------------------------------- */
void setup()
{
  debugstart(9600);                // only when 
  debugln("Program start");        //  debugging is on
  
  Wire.begin();                    // Start the I2C interface
  
  /* ------------------------------------------------------------------------ *
   * Set these pins for input, make them default high
   * ------------------------------------------------------------------------ */
  for (int PIN=DATETIME; PIN<=CLOCKDWN; PIN++) {
    pinMode(PIN, INPUT);         // Setup pin for input
    digitalWrite(PIN, HIGH);     //  and engage pull-up to make in high
  }

  /* ------------------------------------------------------------------------ *
   * Initialize the display
   * ------------------------------------------------------------------------ */
  display.clear();                  // Clear LED display
  display.setBrightness(brightness);// Start with low brightness
  
  /* ------------------------------------------------------------------------ *
   * Set time to Epoch for local time, see: https://www.epochconverter.com/ 
   *  after setting the clock, comment out only this line!
   * ------------------------------------------------------------------------ */
  //myRTC.setEpoch((time_t)1672479510, true);

  /* ------------------------------------------------------------------------ *
   * Set time to individual values for time and date
   *  after setting the clock, comment out these lines!
   * ------------------------------------------------------------------------ */
  //myRTC.setYear((byte)23);
  //myRTC.setMonth((byte)07);
  //myRTC.setDate((byte)14);
  //myRTC.setHour((byte)15);
  //myRTC.setMinute((byte)42);
  //myRTC.setSecond((byte)00);

  /* ------------------------------------------------------------------------ *
   * Set for 24 hour clock
   * ------------------------------------------------------------------------ */
  myRTC.setClockMode(false);

}



/* -------------------------------------------------------------------------- *
 * Loop this after setup
 * -------------------------------------------------------------------------- */
void loop() {
  
  unsigned long currentMillis = millis();
  
                                    // Depending on switch, show date or time
  if(currentMillis - timeDispPreviousMillis > timeDispInterval) {
    timeDispPreviousMillis = currentMillis;  // save the last time we displayed
    if (!digitalRead(DATETIME)) {
      showDate();
    } else {
      showTime();
    }
  }

  if (digitalRead(CLOCKSET) == 0) { // do we have to set the clock?
    delay(300);                     // Bouce delay
    setClock();                     // Perform set routine
  }
}


/* -------------------------------------------------------------------------- *
 * Routine to show the date
 * -------------------------------------------------------------------------- */
void showDate() {
  display.showNumberDec(myRTC.getDate(), true, 2, 0);
  display.showNumberDec(myRTC.getMonth(century), true, 2, 2);
}


/* -------------------------------------------------------------------------- *
 * Routine to show the time
 * -------------------------------------------------------------------------- */
void showTime() {
  showDots = !showDots;             // flipped every second to make dots blink
  if (showDots) {
    display.showNumberDecEx(myRTC.getHour(h12Flag, pmFlag), 0b11100000, true, 2, 0);
  } else {
    display.showNumberDecEx(myRTC.getHour(h12Flag, pmFlag), 0b00000000, true, 2, 0);
  }
  display.showNumberDec(myRTC.getMinute(), true, 2, 2);
}


/* -------------------------------------------------------------------------- *
 * Routine to set six values in the clock, indicated by the left display:
 *  1. year   2. month    3. date
 *  4. hour   5. minute   6. second
 *  7. brightness
 * -------------------------------------------------------------------------- */
void setClock() {
  display.clear();                  // Clear LED display
  
  int setVal;
  byte valueToSet;
  for (setVal=1; setVal<=7; setVal++) {
    display.showNumberDec(setVal, true, 1, 0);

    switch (setVal) {
      case 1:                       // retrieve, alter and restore Year value
        valueToSet = myRTC.getYear();
        valueToSet = setValue(valueToSet, setVal, 00, 99);
        myRTC.setYear((byte)valueToSet);
        break;

      case 2:                       // retrieve, alter and restore Month value
        valueToSet = myRTC.getMonth(century);
        valueToSet = setValue(valueToSet, setVal, 01, 12);
        myRTC.setMonth((byte)valueToSet);
        break;
        
      case 3:                       // retrieve, alter and restore Date value
        valueToSet = myRTC.getDate();
        valueToSet = setValue(valueToSet, setVal, 01, 31);
        myRTC.setDate((byte)valueToSet);
        break;

      case 4:                       // retrieve, alter and restore Hour value
        valueToSet = myRTC.getHour(h12Flag, pmFlag);
        valueToSet = setValue(valueToSet, setVal, 01, 23);
        myRTC.setHour((byte)valueToSet);
        break;

      case 5:                       // retrieve, alter and restore Minute value
        valueToSet = myRTC.getMinute();
        valueToSet = setValue(valueToSet, setVal, 00, 59);
        myRTC.setMinute((byte)valueToSet);
        break;

      case 6:                       // retrieve, alter and restore Second value
        valueToSet = myRTC.getSecond();
        valueToSet = setValue(valueToSet, setVal, 00, 59);
        myRTC.setSecond((byte)valueToSet);
        break;
        
      case 7:                       // retrieve, alter and restore Brightness value
        valueToSet = brightness;
        valueToSet = setValue(valueToSet, setVal, 1, 7);
        brightness = valueToSet;
        
      default:
        break;        
    }    
  }
}


/* -------------------------------------------------------------------------- *
 * Routine to change a values from the clock
 * -------------------------------------------------------------------------- */
byte setValue(byte valueToSet, int num, int low, int high) {
  while (1) {
    display.showNumberDec(valueToSet, true, 2, 2);
        
    if (digitalRead(CLOCKUP) == 0) {
      delay(300);                   // Bouce delay
      valueToSet++;                 // Increment value
    }

    if (digitalRead(CLOCKDWN) == 0) {
      delay(300);                   // Bouce delay
      valueToSet--;                 // Decrement value
    }
    
    if (valueToSet <  low) valueToSet = low;
    if (valueToSet > high) valueToSet = high;
    
    if (num == 7) {
      display.setBrightness(valueToSet); // New value for display brightness
    }

    if (digitalRead(CLOCKSET) == 0) {
      delay(300);                   // Bouce delay
      break;                        // Return
    }
  }
  return valueToSet;
}
