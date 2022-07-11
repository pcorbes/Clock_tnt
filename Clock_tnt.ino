/*
Clock_tnt 
by Philippe Corbes, 06/07/2022

Clock based on a Digital Terrestrial Receiver (TNT) module,
Brand: SilverCrest
Model: "SL 65 T" sold by Lidl

The front panel is composed of:
 - 4 digits 7 segments plus a point (LIM-5622G + 74HC164).
 - 7 push buttons
 - 1 IR receiver (not used)

 ## Sketch ##
 
  ### digits ###

  (+)
  5V  o---------------------+--+--+--+
                            |  |  |  |
  /DG4                      |  |  | |/
  #9  o-----------------------------|\ QD4
                            |  |  |  |
                            |  |  |  |
  /DG3                      |  | |/  |
  #8  o--------------------------|\  | QD3
                            |  |  |  |
                            |  |  |  |
  /DG2                      | |/  |  |
  #7  o-----------------------|\  |  | QD2
                            |  |  |  |
                            |  |  |  |
  /DG1                     |/  |  |  |
  #6  o--------------------|\  |  |  | QD1
                            |  |  |  |
                         +--+--+--+--+--+
                         | Digit1,2,3,4 |
                         +---++++++++---+
                             ||||||||
  DSA                      +-++++++++-+
  #4  o------------------->+ 74HC164  |
                           +-+--------+
  CP                         |
  #5  o----------------------+

  ### Buttons ###

  /HRM         /HRP         /MNM         /MNP
  #10 o-----+  #11 o-----+  #12 o-----+  #13 o-----+
            |            |            |            |
         [-\          [-\          [-\          [-\
            |            |            |            |
  GND o-----+------------+------------+------------+

This project has been developped on an Arduino Leonardo but it can run on many other boards.
You needs to install the "DS3231" library.
*/
 
#include <Arduino.h>
#include <DS3231.h>

// Module connection pins (Digital Pins)
#define DSA 4   // 74HC164's Data
#define CP  5   // 74HC164's Clock pulse
#define DG1 6   // Digit #.:..
#define DG2 7   // Digit .#:..
#define DG3 8   // Digit ..:#.
#define DG4 9   // Digit ..:.#
#define HRM 10  // Hour -
#define HRP 11  // Hour +
#define MNM 12  // Minute -
#define MNP 13  // Minute +

#define DEFAULT_BIT_TIME     1        // Time in nS to activate Clock pulse
#define DEFAULT_DIGIT_TIME   4000000  // Time in nS before to switch to the next
#define DEFAULT_SCROLL_LOOP  15       // Number of loop before to scroll screen
#define NO_REPEAT_LOOP       50       // Number of loop before repeat a hey

//#define MANAGE_PULL_UP
#ifdef MANAGE_PULL_UP
#define DIGITAL_HI(pin)  pinMode(pin, INPUT_PULLUP)
#define DIGITAL_LO(pin)  pinMode(pin, OUTPUT)
#else
#define DIGITAL_HI(pin)  digitalWrite(pin, HIGH)
#define DIGITAL_LO(pin)  digitalWrite(pin, LOW)
#endif

// Board's digit mapping
#define SEG_A   0b00000010
#define SEG_B   0b00000001
#define SEG_C   0b01000000
#define SEG_D   0b00100000
#define SEG_E   0b00010000
#define SEG_F   0b00001000
#define SEG_G   0b00000100
#define SEG_DOT 0b10000000
//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
const uint8_t digitShape[] = {
  /* 0 */  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  /* 1 */  SEG_B | SEG_C,
  /* 2 */  SEG_A | SEG_B | SEG_G | SEG_E | SEG_D,
  /* 3 */  SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
  /* 4 */  SEG_F | SEG_G | SEG_B | SEG_C,
  /* 5 */  SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,
  /* 6 */  SEG_A | SEG_F | SEG_E | SEG_D | SEG_C | SEG_G,
  /* 7 *//*SEG_F |*/ SEG_A | SEG_B | SEG_C,
  /* 8 */  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  /* 9 */  SEG_G | SEG_F | SEG_A | SEG_B | SEG_C | SEG_D,
  /* A */  SEG_E | SEG_F | SEG_A | SEG_B | SEG_C | SEG_G,
  /* b */  SEG_F | SEG_E | SEG_D | SEG_C | SEG_G,
  /* c */  SEG_G | SEG_E | SEG_D,
  /* d */  SEG_G | SEG_E | SEG_D | SEG_C | SEG_B,
  /* E */  SEG_A | SEG_F | SEG_E | SEG_D | SEG_G,
  /* F */  SEG_A | SEG_F | SEG_E | SEG_G,
  /*   */  0,
  /* ° */  SEG_A | SEG_B | SEG_G | SEG_F,
  /* C */  SEG_A | SEG_F | SEG_E | SEG_D | SEG_C,
  /* ? */  SEG_A | SEG_B | SEG_G | SEG_E,
  };
  
#define DIGIT_BLANK    16
#define DIGIT_DEGRE    17
#define DIGIT_C        18
#define DIGIT_QUESTION 19


// Global vars
RTClib       gRTC;
DS3231       gClock;
uint8_t      gHour;
uint8_t      gMinute;
char         gMsg[9];
uint8_t      gMsgl;
unsigned int gScroll;
unsigned int gNoRepeat;


bool writeByte(uint8_t d, uint8_t b)
{
  uint8_t data = b;

  // Disable all digits during update
  DIGITAL_HI(DG1);
  DIGITAL_HI(DG2);
  DIGITAL_HI(DG3);
  DIGITAL_HI(DG4);
  
  // 8 Data Bits
  for(uint8_t i = 0; i < 8; i++) 
  {
    DIGITAL_LO(CP); // CLK low
    delayMicroseconds(DEFAULT_BIT_TIME);
    
    // Set data bit
    if (data & 0x80)
      DIGITAL_LO(DSA);
    else
      DIGITAL_HI(DSA);

    delayMicroseconds(DEFAULT_BIT_TIME);

    DIGITAL_HI(CP); // CLK high
    delayMicroseconds(DEFAULT_BIT_TIME);
    data = data << 1;
  }
  delayMicroseconds(DEFAULT_BIT_TIME);

  // Activate the right digit
  switch (d) {
    case 0: DIGITAL_LO(DG1); break;
    case 1: DIGITAL_LO(DG2); break;
    case 2: DIGITAL_LO(DG3); break;
    case 3: DIGITAL_LO(DG4); break;
  }
  delayMicroseconds((unsigned int)DEFAULT_DIGIT_TIME);
  return 0;
}

void setup()
{
  // Set the pin direction and default value.
  // Both pins are set as inputs, allowing the pull-up resistors to pull them up
  pinMode(DSA, OUTPUT); // Data
  pinMode(CP,  OUTPUT); // Clock pulse
  pinMode(DG1, OUTPUT); // #.:..
  pinMode(DG2, OUTPUT); // .#:..
  pinMode(DG3, OUTPUT); // ..:#.
  pinMode(DG4, OUTPUT); // ..:.#
  pinMode(HRM, INPUT_PULLUP);  // Hour -
  pinMode(HRP, INPUT_PULLUP);  // Hour +
  pinMode(MNM, INPUT_PULLUP);  // gMinute -
  pinMode(MNP, INPUT_PULLUP);  // gMinute +

  digitalWrite(DSA, LOW);
  digitalWrite(CP,  LOW);
  digitalWrite(DG1, LOW);
  digitalWrite(DG2, LOW);
  digitalWrite(DG3, LOW);
  digitalWrite(DG4, LOW);
  digitalWrite(HRM, LOW);
  digitalWrite(HRP, LOW);
  digitalWrite(MNM, LOW);
  digitalWrite(MNP, LOW);

 
  // Start the I2C interface
  Wire.begin();
  
  DateTime now = gRTC.now();
  gHour = now.hour() % 24;
  gMinute = now.minute() % 60;
  gMsgl = 4;
  // Time string
  gMsg[0] = digitShape[gHour/10%10];
  gMsg[1] = digitShape[gHour%10] | SEG_DOT;
  gMsg[2] = digitShape[gMinute/10%10];
  gMsg[3] = digitShape[gMinute%10];
  gMsg[4] = digitShape[DIGIT_BLANK];
  gMsg[5] = gMsg[0];
  gMsg[6] = gMsg[1];
  gMsg[7] = gMsg[2];
  gMsg[8] = gMsg[3];

  gNoRepeat = 0;
}

void loop()
{
  // Read actual hour/minute values
  DateTime now = gRTC.now();
  uint8_t hr = now.hour();
  uint8_t mn = now.minute();
  
  // Check minute change
  if (mn != gMinute) {
    uint8_t i = 0;
    gHour = hr % 24;
    gMinute = mn % 60;
    if (mn % 15 == 0) {
      // Display new time at the right of the actual time and scroll
      i = 5; 
      gMsgl = 9;
      gScroll = DEFAULT_SCROLL_LOOP;
    }
    if ( gHour/10%10 == 0)
      gMsg[i++] = digitShape[DIGIT_BLANK];
    else
      gMsg[i++] = digitShape[gHour/10%10];
    gMsg[i++] = digitShape[gHour%10] | SEG_DOT;
    gMsg[i++] = digitShape[gMinute/10%10];
    gMsg[i++] = digitShape[gMinute%10];
  }
 
  // Display time
  for (uint8_t i=0; i<4; i++) {
    writeByte(i, gMsg[i]);
  }
  
  // Manage scrolling
  if (gMsgl > 4) {
    if (gScroll > 0) {
      gScroll -= 1;
    } else {
      gScroll = DEFAULT_SCROLL_LOOP;
      gMsgl -= 1;
      for (uint8_t j=0; j<gMsgl; j++) {
        gMsg[j] = gMsg[j+1];
      }
    }
  }

  // Manage buttons
  uint8_t  h_ = hr;
  uint8_t  m_ = mn;
  if (!digitalRead(HRM)) {
    if (!gNoRepeat) {
       gNoRepeat = NO_REPEAT_LOOP;
       h_ = hr - 1;
    }
  } else if (!digitalRead(HRP)) {
    if (!gNoRepeat) {
      gNoRepeat = NO_REPEAT_LOOP;
      h_ = hr + 1;
    }
  } else if (!digitalRead(MNM)) {
    if (!gNoRepeat) {
      gNoRepeat = NO_REPEAT_LOOP;
      m_ = mn - 1;
    }
  } else if (!digitalRead(MNP)) {
    if (!gNoRepeat) {
      gNoRepeat = NO_REPEAT_LOOP;
      m_ = mn + 1;
    }
  } else {
    gNoRepeat = 0;
  }
  
  if (h_ != hr) {
    gClock.setHour(h_);
    gMinute = 255; // force to refresh
  }
  
  if (m_ != mn) {
    gClock.setMinute(m_);
    gMinute = 255; // force to refresh
  }
  
  if (gNoRepeat > 0)
    gNoRepeat--;
}
