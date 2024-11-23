# funni IIDX
A mess of code that's been kanged from 3 different places to make a barely functional IIDX controller that also works on a PS2 using an Arduino Leonardo.

# Features:
* PS2 Compatibility
* HID Lighting (No reactive LED fallback, it's either reactive or HID lighting. Brain too smol to make it work without re-working the entire thing. Switch using E3+B1/B3)
* WS2812b Turntable Light with on-controller color control (E3 + B2/4/6 + TT Hue/Sat/Val) a'la PWan.
* EEPROM saving (For TT Color)
* Analog and Digital TT (Both are active, I'll make a mode switch when I get to it)
## Wiring

Button 8 should not be used for Infinitas compatibility. E1 - E3 do not have LED pins since all pins are being used already.

| Item                    | Pin |
|-------------------------|-----|
| **Buttons**             |     |
| Btn 1 Switch            |  2  |
| Btn 2 Switch            |  3  |
| Btn 3 Switch            |  4  |
| Btn 4 Switch            |  5  |
| Btn 5 Switch            |  6  |
| Btn 6 Switch            |  7  |
| Btn 7 Switch            |  8  |
| Btn 8 Switch            |  NOT USED  |
| Btn 9 Switch (Start/E1)    |  9  |
| Btn 10 Switch (Select/E2)   | 10  |
| Btn 11 Switch (E3)		| A4/22 |
| **LEDs**                |     |
| Btn 1 LED               |  11 |
| Btn 2 LED               |  12 |
| Btn 3 LED               |  13 |
| Btn 4 LED               |  A0/18 |
| Btn 5 LED               |  A1/19 |
| Btn 6 LED               |  A2/20 |
| Btn 7 LED               |  A3/21 |
| WS2812b Data            |  A5/23  |
| **Encoders**            |     |
| Turntable Enc Channel A |  0  |
| Turntable Enc Channel B |  1  |

## Playstation compatibility (From CrazyRedMachine's Ultimate Pop'n Controller)

### DO NOT USE PINOUT FOR BUTTONS/LEDS, THIS IS FOR THE PS2 CABLE ONLY
Arduino Leonardo version is also compatible with Playstation and Playstation 2 (it can be made to be plugged directly to the controller port, using the following pinout).

![pinout_psx](https://github.com/CrazyRedMachine/UltimatePopnController/blob/master/pinout_leonardo_psx.png?raw=true)

For ACK (TXLED aka PD5) and SS (RXLED aka PB0) you have to solder new headers or cables directly on the leonardo PCB (or you can use an Arduino Micro (not pro micro) which has everything broken out).

LEDs will be dimmer due to 3.3v power. Using the 7V rumble motor line to Vin instead, and using NPN transistors like 2N2222A on MISO and ACK lines to prevent backfeeding voltage into the console will solve the issue (set INVERT_CIPO and INVERT_ACK to 1 in `ps2.c`).

**BEWARE: DO NOT PLUG USB AND PSX AT THE SAME TIME, THIS CAN DAMAGE YOUR CONSOLE**

## Acknowledgments:

[CrazyRedMachine's Ultimate Pop'n Controller](https://github.com/CrazyRedMachine/UltimatePopnController): PS2 Code/Handling

[Hormash's beatmania-IIDX-USKOC-Arduino-Leonardo](https://github.com/Hormash/beatmania-IIDX-USKOC-Arduino-Leonardo): Original Code that this repo was based off of.

[4yn's iivx](https://github.com/4yn/iivx)
