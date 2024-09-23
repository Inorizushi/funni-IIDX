#include <FastLED.h>

#include "iivx_leo.h"
#include <EEPROM.h>

iivxReport_t report;

#define REPORT_DELAY 995
// Number of microseconds between HID reports
// 2000 = 500hz
#define ENCODER_Z_A 50
#define ENCODER_Z_B 51
#define ENCODER_Z_B_REGISTER 12
#define ENCODER_Z_MILLIS_TOLERANCE 100  // Amount of miliseconds to wait and change state of turntable buttons
#define ENC_L_A 0
#define ENC_L_B 1
#define ENC_L_B_ADDR 3
#define ENCODER_SENSITIVITY (double)0.2000
#define ENCODER_PORT PIND

#define LED_PIN 23
#define NUM_LEDS 57

#define LIGHTING_MODE 0

uint8_t rHue = 0;
uint8_t Hue = EEPROM.read(0), Sat = EEPROM.read(1), Val = EEPROM.read(2);

#define TT_COLOR CHSV(Hue, Sat, Val);

#define LED_STRIP_VOLTAGE 5
#define LED_STRIP_MILLIAMPS 1200  // you can increase/decrease this to get brighter/dimmer LEDs, or if you add/remove more LEDs

// encoder sensitivity = number of positions per rotation (400) / number of positions for HID report (256)
/*
 * connect encoders
 * Scratch to pins 0 and 1
 */
volatile int32_t encX = 0, encY = 0, encZ = 0, encZlast = 0, encZmillis = 0;  // Storage for encoder states


int tmp;
uint8_t buttonCount = 11;
uint8_t lightMode = 0;
// 0 = reactive lighting, 1 = HID lighting
uint8_t ledPins[] = { 11, 12, 13, 18, 19, 20, 21, 24, 25, 26, 32, 32 };
uint8_t buttonPins[] = { 2, 3, 4, 5, 6, 7, 8, 31, 9, 10, 22, 31 };
int32_t encL = 0;
/* current pin layout
 *  pins 1 to 9 = LED 1 to 9
 *    connect pin to + termnial of LED
 *    connect ground to resistor and then - terminal of LED
 *  pins 11 to 13, A0 to A5 = Button input 1 to 9
 *    connect button pin to ground to trigger button press
 *  pins 0 = system pin
 *    connect system pin to ground 
 *      together with other buttons to change lighting scheme
 *    system button + button 1 = reactive lighting
 *    system button + button 3 = HID lighting
 */

void doEncL() {
  if ((ENCODER_PORT >> ENC_L_B_ADDR) & 1) {
    encL++;
  } else {
    encL--;
  }
}


void lights(uint8_t lightDesc) {
  for (int i = 0; i < buttonCount - 4; i++) {
    if ((lightDesc >> i) & 1) {
      digitalWrite(ledPins[i], HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }
}

#define WITH_PSX 1
#if WITH_PSX == 1
#include "ps2.h"
#endif

CRGB leds[NUM_LEDS];

/* Display animation on the cab according to a bitfield array */
void animate(uint16_t* tab, uint8_t n, int mswait) {
  for (int i = 0; i < n; i++) {
    lights(tab[i]);
    delay(mswait);
  }
}

void setup() {
  delay(1000);
  // Setup I/O for pins
  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
  }

  //setup interrupts
  pinMode(ENC_L_A, INPUT_PULLUP);
  pinMode(ENC_L_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), doEncL, RISING);

  FastLED.setMaxPowerInVoltsAndMilliamps(LED_STRIP_VOLTAGE, LED_STRIP_MILLIAMPS);
  FastLED.setCorrection(TypicalSMD5050);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(calculate_max_brightness_for_power_vmA(leds, NUM_LEDS, 255, LED_STRIP_VOLTAGE, LED_STRIP_MILLIAMPS));

#if WITH_PSX == 1
  PS2_MapInput(&report.buttons, (1 << 0), PS2_SQUARE);
  PS2_MapInput(&report.buttons, (1 << 1), PS2_L1);
  PS2_MapInput(&report.buttons, (1 << 2), PS2_CROSS);
  PS2_MapInput(&report.buttons, (1 << 3), PS2_R1);
  PS2_MapInput(&report.buttons, (1 << 4), PS2_CIRCLE);
  PS2_MapInput(&report.buttons, (1 << 5), PS2_L2);
  PS2_MapInput(&report.buttons, (1 << 6), PS2_LEFT);
  PS2_MapInput(&report.buttons, (1 << 8), PS2_START);
  PS2_MapInput(&report.buttons, (1 << 9), PS2_SELECT);
  PS2_MapInput(&report.buttons, (1 << 12), PS2_DOWN);
  PS2_MapInput(&report.buttons, (1 << 13), PS2_UP);

  PS2_Init();

#endif
  //Turn off TT LEDs
  for (int i = 0; i <= NUM_LEDS - 1; i++) {
    if (LIGHTING_MODE == 0) {
      leds[i] = CRGB(0, 0, 0);
    }
    FastLED.show();
  }

  //boot animation
  uint16_t anim[] = { 1, 4, 16, 64, 256, 128, 32, 8, 2 };
  animate(anim, 9, 100);
  animate(anim, 9, 100);
  uint16_t anim2[] = { 1 + 4 + 16 + 64 + 256, 2 + 8 + 32 + 128 };
  animate(anim2, 2, 500);
  animate(anim2, 2, 500);

  //Now Show the TT LEDs
  for (int i = 0; i <= NUM_LEDS - 1; i++) {
    if (LIGHTING_MODE == 0) {
      leds[i] = TT_COLOR
    } else if (LIGHTING_MODE == 1) {
      leds[i] = CHSV(rHue + (i * 10),255,255);
    }
    FastLED.show();
    FastLED.delay(10);
  }
}

void loop() {
  // Read buttons
  for (int i = 0; i < buttonCount; i++) {
    if (digitalRead(buttonPins[i]) != HIGH) {
      report.buttons |= (uint16_t)1 << i;
    } else {
      report.buttons &= ~((uint16_t)1 << i);
    }
  }
  report.buttons &= ~((uint16_t)1 << 11);
  // Read Encoders
  report.xAxis = (uint8_t)((int32_t)(encL / ENCODER_SENSITIVITY) % 256);

  if (LIGHTING_MODE == 1) {
      for (int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = CHSV(rHue + (i * 10),255,255);
      }
      EVERY_N_MILLISECONDS(15){
        rHue++;
      }
      FastLED.show();
    }

  // Read turntable buttons
  if ((int32_t)(encL / ENCODER_SENSITIVITY) - encZlast > 5) {
    if (millis() - encZmillis > ENCODER_Z_MILLIS_TOLERANCE || bitRead(report.buttons, 12)) {
      // Set button 12 active and de-activate 13
      report.buttons |= (uint16_t)1 << 12;
      report.buttons &= ~((uint16_t)1 << 13);
      encZlast = (encL / ENCODER_SENSITIVITY);
      encZmillis = millis();
      if (digitalRead(buttonPins[10]) != HIGH) {
        if (digitalRead(buttonPins[1]) != HIGH) {
          if (Hue <= 255) {
            Hue += 4;
          } else if (Hue = 255) {
            Hue = 0;
          }
          updateTT(1, 0);
        } else if (digitalRead(buttonPins[3]) != HIGH) {
          if (Sat <= 255) {
            Sat += 4;
          }
          updateTT(1, 1);
        } else if (digitalRead(buttonPins[5]) != HIGH) {
          if (Val <= 255) {
            Val += 4;
          }
          updateTT(1, 2);
        }
      }
    }
  } else if ((int32_t)(encL / ENCODER_SENSITIVITY) - encZlast < -5) {
    if (millis() - encZmillis > ENCODER_Z_MILLIS_TOLERANCE || bitRead(report.buttons, 13)) {
      // Set button 13 active and de-activate 12
      report.buttons |= (uint16_t)1 << 13;
      report.buttons &= ~((uint16_t)1 << 12);
      encZlast = (encL / ENCODER_SENSITIVITY);
      encZmillis = millis();
      if (digitalRead(buttonPins[10]) != HIGH) {
        if (digitalRead(buttonPins[1]) != HIGH) {
          if (Hue >= 0) {
            Hue -= 4;
          } else if (Hue = 0) {
            Hue = 255;
          }
          updateTT(1, 0);
        } else if (digitalRead(buttonPins[3]) != HIGH) {
          if (Sat >= 0) {
            Sat -= 4;
          }
          updateTT(1, 1);
        } else if (digitalRead(buttonPins[5]) != HIGH) {
          if (Val >= 0) {
            Val -= 4;
          }
          updateTT(1, 2);
        }
      }
    }
  } else {
    if (millis() - encZmillis > ENCODER_Z_MILLIS_TOLERANCE) {
      // Reset the turntable buttons
      report.buttons &= ~((uint16_t)1 << 12);
      report.buttons &= ~((uint16_t)1 << 13);
    }
  }

#if WITH_PSX == 1
  PS2_Task();
#endif
  if (lightMode == 0) {
    lights(report.buttons);
  } else if (lightMode == 1) {
    lights(iivx_led);
  } else if (lightMode == 2) {
    lights(iivx_led);
  }
  // Detect lighting changes
  if (digitalRead(buttonPins[10]) != HIGH) {
    if (digitalRead(buttonPins[0]) != HIGH) {
      lightMode = 0;
    } else if (digitalRead(buttonPins[2]) != HIGH) {
      lightMode = 1;
    } else if (digitalRead(buttonPins[4]) != HIGH) {
      lightMode = 2;
    }
  }
  if (digitalRead(buttonPins[10]) != HIGH && digitalRead(buttonPins[8]) != HIGH) {
    report.buttons &= ~((uint16_t)1 << 10);
    report.buttons &= ~((uint16_t)1 << 9);
    report.buttons |= (uint16_t)1 << 11;
  }
  // Send report and delay
  iivx.setState(&report);
  delayMicroseconds(REPORT_DELAY);
}

void updateTT(int write, int type) {
  for (int i = 0; i <= NUM_LEDS - 1; i++) {
    if (LIGHTING_MODE == 0) {
      leds[i] = TT_COLOR
    }
    FastLED.show();
    if (write = 1) {
      switch (type) {
        case 0:
          Serial.println(Hue);
          EEPROM.update(0, Hue);
          break;
        case 1:
          Serial.println(Sat);
          EEPROM.update(1, Sat);
          break;
        case 2:
          Serial.println(Val);
          EEPROM.update(2, Val);
          break;
      }
    }
  }
}
