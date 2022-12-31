/* ------------------------------------------------------------------------- *
 * Name   : gawclock
 * Author : Gerard Wassink
 * Date   : December 2022
 * Purpose: Show time from RTC module on 7 segment displays
 * Versions:
 *   0.1  : Initial code base
 *   0.2  : Built in setting the clock
 *   
 * ------------------------------------------------------------------------- */
#define progVersion "0.2"                   // Program version definition
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


#include <Arduino.h>
#include <TM1637Display.h>

#include <DS3231.h>
#include <Wire.h>

#define DEBUG 1

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

bool century = false;               //
bool h12Flag;                       //
bool pmFlag;                        //
bool showDots = true;               //

/* -------------------------------------------------------------------------- *
 * Definitions for display module
 * -------------------------------------------------------------------------- */
#define CLK 2                       // Module connection pins
#define DIO 3                       //  (Digital Pins)

TM1637Display display(CLK, DIO);    // display object


/* -------------------------------------------------------------------------- *
 * General global definitions
 * -------------------------------------------------------------------------- */
#define DATETIME 4                  // Switch between date & time display
#define CLOCKSET 5                  // Button to set clock values
#define CLOCKUP  6                  // move value up
#define CLOCKDWN 7                  // move value down

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
  };

#define timeDispInterval 1000

long timeDispPreviousMillis = 1000; // Make timeouts work first time


void setup()
{
  debugstart(115200);
  debugln("Program start");
  
  Wire.begin();                     // Start the I2C interface
  
  /* ------------------------------------------------------------------------ *
   * Set pins for input, make them default high
   * ------------------------------------------------------------------------ */
  for (int PIN=DATETIME; PIN<=CLOCKDWN; PIN++) {
    pinMode(PIN, INPUT);         // Setup date time pin
    digitalWrite(PIN, HIGH);     //  to switch date / time
  }
  
  display.clear();                  // Clear LED display
  display.setBrightness( 1 );      // Start with low brightness
  
  /* ------------------------------------------------------------------------ *
   * Set time to Epoch for local time, see: https://www.epochconverter.com/ 
   *  after setting the clock, comment out this line!
   * ------------------------------------------------------------------------ */
  //myRTC.setEpoch((time_t)1672479510, true);

  /* ------------------------------------------------------------------------ *
   * Set time to individual values for time and date
   *  after setting the clock, comment out these lines!
   * ------------------------------------------------------------------------ */
  /* ------------------------------------------------------------------------ *
  myRTC.setYear((byte)22);
  myRTC.setMonth((byte)12);
  myRTC.setDate((byte)31);
  myRTC.setHour((byte)13);
  myRTC.setMinute((byte)30);
  myRTC.setSecond((byte)30);
   * ------------------------------------------------------------------------ */

  /* ------------------------------------------------------------------------ *
   * Set for 24 hour clock
   * ------------------------------------------------------------------------ */
  myRTC.setClockMode(false);
}



void loop() {
  
  unsigned long currentMillis = millis();
  
  /*                                   Depending on switch, show date or time */
  if(currentMillis - timeDispPreviousMillis > timeDispInterval) {
    timeDispPreviousMillis = currentMillis;  // save the last time we displayed
    if (digitalRead(DATETIME)) {
      showDate();
    } else {
      showTime();
    }
  }

  /*                        does the switch indicate we have to set the clock? */
  if (digitalRead(CLOCKSET) == 0) {
    delay(300);
    setClock();
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
  showDots = !showDots;
  if (showDots) {
    display.showNumberDecEx(myRTC.getHour(h12Flag, pmFlag), 0b11100000, true, 2, 0);
  } else {
    display.showNumberDecEx(myRTC.getHour(h12Flag, pmFlag), 0b00000000, true, 2, 0);
  }
  display.showNumberDec(myRTC.getMinute(), true, 2, 2);
}


/* -------------------------------------------------------------------------- *
 * Routine to set six values in the clock, indicated by the left display::
 *  1. year   2. month    3. date
 *  4. hour   5. minute   6. second
 * -------------------------------------------------------------------------- */
void setClock() {
  display.clear();                  // Clear LED display
  
  int setVal = 1;
  byte valueToSet = 0;
  for (setVal=1; setVal<=6; setVal++) {
    display.showNumberDec(setVal, true, 1, 0);

    switch (setVal) {
      case 1:
        valueToSet = myRTC.getYear();
        valueToSet = setValue(valueToSet);
        myRTC.setYear((byte)valueToSet);
        break;

      case 2:
        valueToSet = myRTC.getMonth(century);
        valueToSet = setValue(valueToSet);
        myRTC.setMonth((byte)valueToSet);
        break;
        
      case 3:
        valueToSet = myRTC.getDate();
        valueToSet = setValue(valueToSet);
        myRTC.setDate((byte)valueToSet);
        break;

      case 4:
        valueToSet = myRTC.getHour(h12Flag, pmFlag);
        valueToSet = setValue(valueToSet);
        myRTC.setHour((byte)valueToSet);
        break;

      case 5:
        valueToSet = myRTC.getMinute();
        valueToSet = setValue(valueToSet);
        myRTC.setMinute((byte)valueToSet);
        break;

      case 6:
        valueToSet = myRTC.getSecond();
        valueToSet = setValue(valueToSet);
        myRTC.setSecond((byte)valueToSet);
        break;
        
      default:
        break;        
    }    
  }
}


/* -------------------------------------------------------------------------- *
 * Routine to change a values from the clock
 *  range of value is not checked
 * -------------------------------------------------------------------------- */
byte setValue(byte valueToSet) {
  while (1) {
    display.showNumberDec(valueToSet, true, 2, 2);
        
    if (digitalRead(CLOCKUP) == 0) {
      delay(300);
      valueToSet++;
    }

    if (digitalRead(CLOCKDWN) == 0) {
      delay(300);
      valueToSet--;
    }
    
    if (digitalRead(CLOCKSET) == 0) {
      delay(300);
      break;
    }
  }
  return valueToSet;
}
