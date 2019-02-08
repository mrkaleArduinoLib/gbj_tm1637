/*
  NAME:
  gbj_tm1637

  DESCRIPTION:
  Library for 7-segment digital tubes displays controlled by the driver TM1637.
  - The library controls the driver as a state machine with screen buffer in the
    microcontroller's operating memory, which is transmitted to the controller
    for displaying.
    - Screen buffer is considered as an image of controller's graphical memory.
    - Graphical library methods (prefixed with "print") performes all graphical
      manipulation in the screen buffer, which state reflects the desired image
      for display.
    - Finally the dedicated method transmitts the content of the screen buffer
      to the driver and it causes to display the image on the attached display.
  - The driver TM1637 can control up to 6 digital tubes each with radix (decimal
    dot or colon).
  - The library controls 7-segment glyphs (digits) independently from radix 8th
    segments of glyphs.
  - The library can control the TM1636 driver as well, which is binary compatible
    with TM1637 but controls just 4 digital tubes.
  - The library does not implement key scan capabalities of the driver, because
    display modules with TM1637 controller do not implement a keypad.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the license GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
  (related to original code) and MIT License (MIT) for added code.

  CREDENTIALS:
  Author: Libor Gabaj
  GitHub: https://github.com/mrkaleArduinoLib/gbj_tm1637.git
 */
#ifndef GBJ_TM1637_H
#define GBJ_TM1637_H

#if defined(__AVR__)
  #if ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif
  #include <inttypes.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <Arduino.h>
#elif defined(PARTICLE)
  #include <Particle.h>
#endif


class gbj_tm1637 : public Print
{
public:
//------------------------------------------------------------------------------
// Public constants
//------------------------------------------------------------------------------
static const String VERSION;
enum ResultCodes
{
  SUCCESS = 0,
  ERROR_PINS = 255, // Error defining pins, usually both are the same
  ERROR_ACK = 254, // Error at acknowledging a command
};


//------------------------------------------------------------------------------
// Public methods
//------------------------------------------------------------------------------
/*
  Initialize display geometry.

  DESCRIPTION:
  The constructor methodsanitizes and stores physical features of the display
  to the class instance object.

  PARAMETERS:
  pinClk - Microcontroller pin's number utilized as a serial clock.
           - Data type: non-negative integer
           - Default value: 2
           - Limited range: 0 ~ 255 (by microcontroller datasheet)

  pinDio - Microcontroller pin's number utilized as a data input and output.
           - Data type: non-negative integer
           - Default value: 3
           - Limited range: 0 ~ 255 (by microcontroller datasheet)

  grids - Number of 7-segment LED digits to be controlled. Default value is aimed
          for clock LED display with 4 LEDs as well as for TM1636 driver.
          - Data type: non-negative integer
          - Default value: 4
          - Limited range: 1 ~ 6

  RETURN:
  Result code.
*/
gbj_tm1637(uint8_t pinClk = 2, uint8_t pinDio = 3, uint8_t digits = 4);


/*
  Initialize display.

  DESCRIPTION:
  The method sets the microcontroller's pins dedicated for the driver and perfoms
  initial sequence recommended by the data sheet for the controller.
  - The method clears the display and sets it to the normal operating mode.
  - The method checks whether pins set by constructor are not equal.

  PARAMETERS: none

  RETURN:
  Result code.
*/
uint8_t begin();


/*
  Transmit screen buffer to driver.

  DESCRIPTION:
  The method transmits current content of the screen buffer to the driver, so that
  it content is displayed immediatelly and stays unchanged until another transmission.
  - The method utilizes automatic addressing mode of the driver.

  PARAMETERS: none

  RETURN:
  Result code.
*/
uint8_t display();


/*
  Turn display off or on.

  DESCRIPTION:
  Particular method either turns on or off the entired display module without
  changing current contrast level.
  - Both methods are suitable for making a display module blink.

  PARAMETERS: none

  RETURN:
  Result code.
*/
uint8_t displayOn();
uint8_t displayOff();


/*
  Clear entire digital tubes including radixes and set printing position.

  DESCRIPTION:
  The method turns off all segments including for radixes of all digital tubes
  and then sets the printing position for subsequent printing.

  PARAMETERS:
  digit - Number of digital tube counting from 0 where the printing should start
          after display clearing.
          - Data type: non-negative integer
          - Default value: 0
          - Limited range: 0 ~ 5 (constructor's parameter digits - 1)

  RETURN: none
*/
inline void displayClear(uint8_t digit = 0) { printDigitOff(); printRadixOff(); placePrint(digit); }


/*
  Manipulate digital tubes' radixes of a display module.

  DESCRIPTION:
  The particular method performs corresponding manipulation with radix segment
  (usually 8th one) of particular glyph without influence on its glyph segments
  (first 7 segments) in the screen buffer.

  PARAMETERS:
  digit - Driver's digit tube number counting from 0, which radix segment
          should be manipulated.
          - Data type: non-negative integer
          - Default value: none
          - Limited range: 0 ~ 5

  RETURN: none
*/
inline void printRadixOn(uint8_t digit) { if (digit < _status.digits) _print.buffer[digit] |= 0x80; }
inline void printRadixOn() { for (uint8_t digit = 0; digit < _status.digits; digit++) printRadixOn(digit); }
inline void printRadixOff(uint8_t digit) { if (digit < _status.digits) _print.buffer[digit] &= ~0x80; }
inline void printRadixOff() { for (uint8_t digit = 0; digit < _status.digits; digit++) printRadixOff(digit); }
inline void printRadixToggle(uint8_t digit) { if (digit < _status.digits) _print.buffer[digit] ^= 0x80; }
inline void printRadixToggle() { for (uint8_t digit = 0; digit < _status.digits; digit++) printRadixToggle(digit); }


/*
  Manipulate digit segments.

  DESCRIPTION:
  The particular method sets glyph segments (first 7 ones) of particular digit
  (digital tube) without influence on its radix segment in the screen buffer.

  PARAMETERS:
  digit - Driver's digit (digital tube) number counting from 0, which glyph
          segments should be manipulated.
          - Data type: non-negative integer
          - Default value: none
          - Limited range: 0 ~ 5

  segmentMask - Bit mask defining what segments should be turned on. Segments
                are marked starting from A to G and relate to mask bits 0 to 6
                counting from least significant bit. The 7th bit relates to radix
                segment and therefore it is ignored.
                - Data type: non-negative integer
                - Default value: none
                - Limited range: 0 ~ 127

  RETURN: none
*/
inline void printDigit(uint8_t digit, uint8_t segmentMask) { if (digit < _status.digits) gridWrite(segmentMask, digit, digit); }
inline void printDigit(uint8_t segmentMask) { gridWrite(segmentMask); }
inline void printDigitOn(uint8_t digit) { printDigit(digit, 0x7F); }
inline void printDigitOn() { printDigit(0x7F); }
inline void printDigitOff(uint8_t digit) { printDigit(digit, 0x00); }
inline void printDigitOff() { printDigit(0x00); }


/*
  Set printing position within digital tubes.

  DESCRIPTION:
  The method stores desired position of a digital tube where the subsequent
  print should start.

  PARAMETERS:
  digit - Number of digital tube counting from 0 where the printing should start.
          - Data type: non-negative integer
          - Default value: 0
          - Limited range: 0 ~ 5 (constructor's parameter digits - 1)

  RETURN: none
*/
inline void placePrint(uint8_t digit = 0) { if (digit < _status.digits) _print.digit = digit; };


/*
  Print text at desired printing position.

  DESCRIPTION:
  The method prints text starting from provided or default position on digital tubes.
  - The method clears the display right before printing.

  PARAMETERS:
  text - Pointer to a text that should be printed.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: microcontroller's addressing range

  digit - Printing position for starting the printing.
          - Data type: non-negative integer
          - Default value: 0
          - Limited range: 0 ~ 5 (constructor's parameter digits - 1)

  RETURN: none
*/
inline void printText(const char* text, uint8_t digit = 0) { displayClear(digit); print(text); };
inline void printText(String text, uint8_t digit = 0) { displayClear(digit); print(text); };


/*
  Print text at desired printing position without impact on radixes.

  DESCRIPTION:
  The method prints text starting from provided or default position on digital
  tubes and leaves radixes intact.
  - The method clears only digit without radixes right before printing.

  PARAMETERS:
  text - Pointer to a text that should be printed.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: microcontroller's addressing range

  digit - Printing position for starting the printing.
          - Data type: non-negative integer
          - Default value: 0
          - Limited range: 0 ~ 5 (constructor's parameter digits - 1)

  RETURN: none
*/
inline void printGlyphs(const char* text, uint8_t digit = 0) { printDigitOff(); placePrint(digit); print(text); };
inline void printGlyphs(String text, uint8_t digit = 0) { printDigitOff(); placePrint(digit); print(text); };


/*
  Print class inheritance.

  DESCRIPTION:
  The library inherits the system Print class, so that all regular print
  functions can be used.
  - Actually all print functions eventually call one of listed write methods,
    so that all of them should be implemented.
  - If some character (ASCII) code is not present in the font table, i.e., it is
    unknown for the library, that character is ignored and not displayed.
  - If unknown character has ASCII code of comma, dot, or colon, the library
    turns on the radix segments of the recently displayed digit (lastly manipulated
    grid). Thus, the decimal points or colon can be present in displayed string
    at proper position and does not need to be control separately.

  PARAMETERS:
  ascii - ASCII code of a character that should be displayed at the current grid
          position. The methods is usually utilized internally by system prints.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: 0 ~ 255

  text - Pointer to a nul terminated string that should be displayed from the
         very beginnng of the display, i.e., from the first digit.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: microcontroller's addressing range

  buffer - Pointer to a string, which part should be displayed from the very
           beginnng of the display, i.e., from the first digit.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: microcontroller's addressing range

  size - Number of characters that should be displayed from the very beginnng of
         the display, i.e., from the first digit.
         - Data type: non-negative integer
         - Default value: none
         - Limited range: microcontroller's addressing range
*/
size_t write(uint8_t ascii);
size_t write(const char* text);
size_t write(const uint8_t* buffer, size_t size);


//------------------------------------------------------------------------------
// Public setters - they usually return result code.
//------------------------------------------------------------------------------
inline void initLastResult() { _status.lastResult = SUCCESS; };
inline uint8_t setLastResult(uint8_t lastResult = SUCCESS) { return _status.lastResult = lastResult; };


/*
  Set contrast of the digital tubes.

  DESCRIPTION:
  The method set constrast level of all digital tubes and simultaniously
  turns display on.

  PARAMETERS:
  contrast - Level of constrast/brightness.
             - Data type: non-negative integer
             - Default value: 3
             - Limited range: 0 ~ 7

  RETURN: none
*/
uint8_t setContrast(uint8_t contrast = 3);


/*
  Define font parameters for printing.

  DESCRIPTION:
  The method gathers font parameters for printing characters on 7-segment displays.
  - Font definition is usually included to an application sketch from particular
    include file, while the font table resides in programmatic (flash) memory of
    a microcontroller in order to save operational memory (SRAM).
  - Each glyph of a font consists of the pair of bytes. The first byte determines
    ASCII code of a glyph and second byte determines segment mask of a glyph. It
    allows to defined only displayable glyphs on 7-segment displays and suppress
    need to waste memory for useless characters.

  PARAMETERS:
  fontTable - Pointer to a font definition table.
              - Data type: non-negative integer
              - Default value: none
              - Limited range: microcontroller's addressing range

  fontTableSize - The number of bytes that should be utilized from the font
                  table.
                  - The table size in conjunction with font character pair of bytes
                    determines the number of characters used for printing.
                  - The size can be smaller than the real size of the table,
                    however, the size should be a multiple of 2.
                  - Data type: non-negative integer
                  - Default value: none
                  - Limited range: 0 ~ 255 (maximal 127 7-segments characters)

  RETURN: none
*/
void setFont(const uint8_t* fontTable, uint8_t fontTableSize);


//------------------------------------------------------------------------------
// Public getters
//------------------------------------------------------------------------------
inline uint8_t getLastResult() { return _status.lastResult; }; // Result of a recent operation
inline uint8_t getLastCommand() { return _status.lastCommand; }; // Command code of a recent operation
inline uint8_t getDigits() { return _status.digits; } // Current digital tubes for displaying
inline uint8_t getDigitsMax() { return DIGITS; } // Maximal supported digital tubes
inline uint8_t getContrast() { return _status.contrast; } // Current contrast
inline uint8_t getContrastMax() { return 7; } // Maximal contrast
inline uint8_t getPrint() { return _print.digit; }; // Current display position
inline bool isSuccess() { return _status.lastResult == SUCCESS; } // Flag about successful recent operation
inline bool isError() { return !isSuccess(); } // Flag about erroneous recent operation


private:
//------------------------------------------------------------------------------
// Private constants
//------------------------------------------------------------------------------
enum Commands
{
  // Data command setting (0x40)
  CMD_DATA_INIT   = 0b01000000, // 0x40, Command set, ORed by proper next ones
  CMD_DATA_WRITE  = 0b00, // 0x00, Write data to display register
  CMD_DATA_READ   = 0b10, // 0x02, Read key scanning data
  CMD_DATA_AUTO   = 0b000, // 0x00, Automatic address adding
  CMD_DATA_FIXED  = 0b100, // 0x04, Fixed address
  CMD_DATA_NORMAL = 0b0000, // 0x00, Normal mode
  CMD_DATA_TEST   = 0b1000, // 0x08, Test mode
  // Address command setting (0xC0)
  CMD_ADDR_INIT = 0b11000000, // 0xC0, Address set, OR-ed by display address 0x00 ~ 0x05 in lower nibble
  // Display control (0x80)
  CMD_DISP_INIT = 0b10000000, // 0x80, Display control, OR-ed by proper next ones
  CMD_DISP_OFF  = 0b0000, // 0x00, Display is off
  CMD_DISP_ON   = 0b1000, // 0x08, Display is on, OR-ed by contrast 0x00 ~ 0x07 in lower 3 bits
};
enum Geometry // Controller TM1637
{
  DIGITS = 6, // Usable and maximal implemented digital tubes
  BYTES_ADDR = 6, // By datasheet maximal addressable register position
};
enum Timing
{
  TIMING_RELAX = 2, // MCU relaxing delay in microseconds after pin change
  TIMING_ACK = 500, // Timeout in microseconds for acknowledgment
};
enum Rasters
{
  FONT_WIDTH = 2,
  FONT_INDEX_ASCII = 0,
  FONT_INDEX_MASK = 1,
  FONT_MASK_WRONG = 0xFF,  // Byte value for unknown font glyph
};


//------------------------------------------------------------------------------
// Private attributes
//------------------------------------------------------------------------------
struct
{
  uint8_t buffer[BYTES_ADDR];  // Screen buffer
  uint8_t digit; // Current grid for next printing
} _print; // Display hardware parameters for printing
struct Bitmap
{
  const uint8_t* table; // Pointer to a font table
  uint8_t glyphs; // Number of glyphs in the font table
} _font;  // Font parameters
struct
{
  uint8_t lastResult; // Result of a recent operation
  uint8_t lastCommand;  // Command code recently sent to controller
  uint8_t pinClk; // Number of serial clock pin
  uint8_t pinDio; // Number of data input/output pin
  uint8_t digits; // Amount of controlled digital tubes
  uint8_t contrast; // Current contrast level
} _status;  // Microcontroller status features


//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
inline void swapByte(uint8_t a, uint8_t b) { if (a > b) {uint8_t t = a; a = b; b = t;} };
inline uint8_t setLastCommand(uint8_t lastCommand) { return _status.lastCommand = lastCommand; };
void waitPulseClk();  // Delay for clock pulse duration
void beginTransmission(); // Start condition
void endTransmission(); // Stop condition
void busWrite(uint8_t data);  // Write byte to the bus
void gridWrite(uint8_t segmentMask = 0x00, uint8_t gridStart = 0, uint8_t gridStop = DIGITS); // Fill screen buffer with digit masks
uint8_t ackTransmission();  // Acknowledgment of transmission
uint8_t busSend(uint8_t command); // Send sole command
uint8_t busSend(uint8_t command, uint8_t data); // Send data at fixed address
uint8_t busSend(uint8_t command, uint8_t* buffer, uint8_t bufferBytes); // Send data at auto-increment addressing
uint8_t getFontMask(uint8_t ascii); // Lookup font mask in font table by ASCII code
};

#endif
