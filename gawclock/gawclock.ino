/* ------------------------------------------------------------------------- *
 * Name   : gawclock
 * Author : Gerard Wassink
 * Date   : December 2022
 * Purpose: Show time from RTC module on 7 segment displays
 * Versions:
 *   0.1  : Initial code base
 *   
 * ------------------------------------------------------------------------- */
#define progVersion "0.1"                   // Program version definition
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
#define DATETIME 4

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
  
  pinMode(DATETIME, INPUT);         // Setup date time pin
  digitalWrite(DATETIME, HIGH);     //  to switch date / time
  
  display.clear();                  // Clear LED display
  display.setBrightness( 1 );      // Start with low brightness
  
  myRTC.setClockMode(false);      // set for 24 hour clock

  /* ------------------------------------------------------------------------ *
   * Set time to Epoch for local time, see: https://www.epochconverter.com/ 
   *  after setting the clock, comment out this line!
   * ------------------------------------------------------------------------ */
  //myRTC.setEpoch((time_t)1672479510, true);
}



void loop() {
  
  unsigned long currentMillis = millis();
  
  /*                                       Look for latitude /longtitude */
  if(currentMillis - timeDispPreviousMillis > timeDispInterval) {
    timeDispPreviousMillis = currentMillis;         // save the last time we displayed
    if (digitalRead(DATETIME)) {
      showDate();
    } else {
      showTime();
    }
  }
}


void showDate() {
  display.showNumberDecEx(myRTC.getDate(), 0b00000000, true, 2, 0);
  display.showNumberDec(myRTC.getMonth(century), true, 2, 2);
}


void showTime() {
  showDots = !showDots;
  if (showDots) {
    display.showNumberDecEx(myRTC.getHour(h12Flag, pmFlag), 0b11100000, true, 2, 0);
  } else {
    display.showNumberDecEx(myRTC.getHour(h12Flag, pmFlag), 0b00000000, true, 2, 0);
  }
  display.showNumberDec(myRTC.getMinute(), true, 2, 2);
}
