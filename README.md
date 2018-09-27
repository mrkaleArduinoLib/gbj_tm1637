<a id="library"></a>
# gbjTM1637
Library for utilizing display modules with TM1637 controller. Those modules are available in variants with colon (clock displays) only after the second digit or with decimal point after every digit, usually with 4 digits, although the controller can drive up to 6 digits.
The library controls the controller as a state machine with screen buffer in the microcontroller's operating memory, which is transmitted to the controller for displaying.
- Screen buffer is considered as an image of controller's graphical memory.
- Graphical library methods (prefixed with "**print**") performs all graphical manipulation in the screen buffer, which state reflects the desired image for display.
- Finally the dedicated method [display()](#display) transmits the content of the screen buffer to the controller and it causes to display the image on the attached display (digital tubes).
- The library and controller TM1637 can control up to 6 digital tubes.
- The library can control the TM1636 controller as well, which is binary compatible with TM1637, but controls just 4 tubes.
- The library controls 7-segment glyphs (digits) mutual independently from radix 8th segments of digital tubes.
- The library does not implement key scan capabilities of the controller.
- The library inherits from the system library **Print**, so that all system *print* operations are available.


<a id="dependency"></a>
## Dependency
- **Arduino.h**: Main include file for the Arduino SDK version greater or equal to 100.
- **WProgram.h**: Main include file for the Arduino SDK version less than 100.
- **inttypes.h**: Integer type conversions. This header file includes the exact-width integer definitions and extends them with additional facilities provided by the implementation.
- **Print.h**: System library for printing.


<a id="Fonts"></a>
## Fonts
The font is an assignment of a glyph definition to particular ASCII code.
- A 7-segment display glyph is defined by a segment mask of the controller.
- Every font is defined as one-dimensional array with the same name **gbjFont7segTable**, stored in a *separate include file* with the naming convention **font7seg\_**_variant_**.h** in the subfolder `extras`. Font variants differentiate from each other by length and content of that array.
- The library contains those fonts:
	- **font7seg_basic.h**: Alphanumeric glyphs reasonably recognizable and readable on 7-segment displays.
	- **font7seg_decnums.h**: Decimal digits, space, and minus glyph.
	- **font7seg_hexnums.h**: Hexadecimal digits, space, and minus glyph.
- Despite the font array is one-dimensional one, glyphs are defined by logical group of two bytes in it.
	- The **first byte** of the glyph pair is an **ASCII code** of a glyph. Usually it is a 7-bit code, but it might be 8-bit one as well, if appropriate segment mask is provided, which can be reasonably displayed on the 7-segment display.
	- The **second byte** of the glyph pair is a **segment mask** of a glyph with least significant bit (LSB) corresponding to the segment `A`. The 8th, most significant bit (MSB) corresponding to the decimal point (DP) is ignored if set, because the library controls radix segments separately, not by fonts.
- Involving ASCII codes to the font definition enables to define just recognizable glyphs by the 7-segment displays or needed by a project and not to waste memory by definition contiguous set of ASCII codes with unused glyphs, although not starting from 0.
- After including a font include file into a sketch, the font is stored in the flash memory of a microcontroller in order to save operational SRAM.
- The library can utilize just one font at a time.


<a id="Constants"></a>
## Constants
All constants are embedded into the class as static ones including result and error codes.

- **gbj\_tm1637::VERSION**: Name and semantic version of the library.
- **gbj\_tm1637::SUCCESS**: Result code for successful processing.
### Errors
- **gbj\_tm1637::ERROR\_PINS**: Error code for incorrectly assigned microcontroller's pins to controller's pins, usually some of them are duplicated.
- **gbj\_tm1637::ERROR\_ACK**: Error code for not acknowledged transmission by the controller.


<a id="interface"></a>
## Interface
The methods in bold return [result or error codes](#constants) and communicate with the controller directly. The methods for screen buffer manipulation return nothing, just update the content of the screen buffer as an image of controller registers, and must be followed by [display()](#display) method in order to display the content of the screen buffer.

It is possible to use functions from the parent library [Print](#dependency), which is extended by this library.

- [gbj_tm1637()](#gbj_tm1637)
- [**begin()**](#begin)

#### Display manipulation
- [**display()**](#display)
- [**displayOn()**](#displaySwitch)
- [**displayOff()**](#displaySwitch)

#### Screen buffer manipulation
- [displayClear()](#displayClear)
- [printRadixOn()](#printRadix)
- [printRadixOff()](#printRadix)
- [printRadixToggle()](#printRadix)
- [printDigit()](#printDigit)
- [printDigitOn()](#printDigitSwitch)
- [printDigitOff()](#printDigitSwitch)
- [printText()](#printText)
- [printGlyphs()](#printGlyphs)
- [placePrint()](#placePrint)
- [write()](#write)

#### Setters
- [initLastResult()](#initLastResult)
- [setLastResult()](#setLastResult)
- [**setContrast()**](#setContrast)
- [setFont()](#setFont)

#### Getters
- [getLastResult()](#getLastResult)
- [getLastCommand()](#getLastCommand)
- [getDigits()](#getDigits)
- [getDigitsMax()](#getDigitsMax)
- [getContrast()](#getContrast)
- [getContrastMax()](#getContrastMax)
- [getPrint()](#getPrint)
- [isSuccess()](#isSuccess)
- [isError()](#isError)


<a id="gbj_tm1637"></a>
## gbj_tm1637()
#### Description
The constructor method sanitizes and stores physical features of the display and limited ones for the sake of a sketch to the class instance object.

#### Syntax
    gbj_tm1637(uint8_t pinClk, uint8_t pinDio, uint8_t grids);

#### Parameters
- **pinClk**: Microcontroller pin's number utilized as a serial clock.
	- **Valid values**: non-negative integer b(according to a microcontroller datasheet)
	- **Default value**: 2


- **pinDio**: Microcontroller pin's number utilized as a data input and output.
	- **Valid values**: non-negative integer (according to a microcontroller datasheet)
	- **Default value**: 3


<a id="prm_digits"></a>
- **digits**: Number of 7-segment digital tubes to be controlled. Default value is aimed for clock display with 4 digits as well as for TM1636 controller.
	- **Valid values**: 0 ~ 6 ([getDigitsMax()](#getDigitsMax) according to attached LED display)
	- **Default value**: 4 (for usual clock and numeric LED displays)


#### Returns
The library instance object for display geometry.

[Back to interface](#interface)


<a id="begin"></a>
## begin()
#### Description
The method sets the microcontroller's pins dedicated for the controller and preforms initial sequence recommended by the data sheet for the controller.
- The method clears the display and sets it to the normal operating mode.
- The method checks whether pins set by constructor are not equal.

#### Syntax
	uint8_t begin();

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

[Back to interface](#interface)


<a id="display"></a>
## display()
#### Description
The method transmits current content of the screen buffer to the controller, so that its content is displayed immediately and stays unchanged until another transmission.
- The method utilizes automatic addressing mode of the controller.

#### Syntax
	uint8_t display();

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

#### See also
[displayOn()](#displaySwitch)

[displayOff()](#displaySwitch)

[Back to interface](#interface)


<a id="displaySwitch"></a>
## displayOn(), displayOff()
#### Description
Particular method either turns on or off the entire display module without changing current contrast level.
- Both methods are suitable for making a display module blinking.

#### Syntax
	uint8_t displayOn();
	uint8_t displayOff();

#### Parameters
None

#### Returns
Some of [result or error codes](#constants).

#### See also
[display()](#display)

[Back to interface](#interface)


<a id="displayClear"></a>
## displayClear()
#### Description
The method turns off all segments including for radixes of all digital tubes and then sets the printing position for subsequent printing.
- It is a wrapper method for subsequent calling methods [printDigitOff()](#printDigitSwitch), [printRadixOff()](#printRadix), and [placePrint()](#placePrint).

#### Syntax
	void displayClear(uint8_t digit);

#### Parameters
- **digit**: Number of digital tube counting from 0 where the printing should start after display clearing.
	- **Valid values**: 0 ~ [digits - 1](#prm_digits) (from constructor)
	- **Default value**: 0

#### Returns
None

#### See also
[printDigitOff()](#printDigitSwitch)

[printRadixOff()](#printRadix)

[placePrint()](#placePrint)

[Back to interface](#interface)


<a id="printRadix"></a>
## printRadixOn(), printRadixOff(), printRadixToggle()
#### Description
The particular method performs corresponding manipulation with radix segment (usually 8th one) of particular glyph without influence on its glyph segments (first 7 segments) in the screen buffer.
- Each method is overloaded. If there is no input parameter provided, the method performs appropriate action on all controlled digital tubes.

#### Syntax
	void printRadixOn(uint8_t digit);
	void printRadixOn();
	void printRadixOff(uint8_t digit);
	void printRadixOff();
	void printRadixToggle(uint8_t digit);
	void printRadixToggle();

#### Parameters
- **digit**: controller's digit tube number counting from 0, which radix segment should be manipulated.
	- **Valid values**: 0 ~ [digits - 1](#prm_digits) (from constructor)
	- **Default value**: none

#### Returns
None

#### See also
[printDigitOn()](#printDigitSwitch)

[printDigitOff()](#printDigitSwitch)

[Back to interface](#interface)


<a id="printDigit"></a>
## printDigit()
#### Description
The method sets glyph segments (first 7 ones) of particular digital tube without influence on its radix segment in the screen buffer.
- The method is overloaded. If there is one input parameter provided, it is considered as a segment mask for current print position set at recent printing action.
- The method is useful for writing to the display without any font used.

#### Syntax
	void printDigit(uint8_t digit, uint8_t segmentMask);
	void printDigit(uint8_t segmentMask);

#### Parameters
- **digit**: controller's digital tube number counting from 0, which glyph segments should be manipulated.
	- **Valid values**: 0 ~ [digits - 1](#prm_digits) (from constructor)
	- **Default value**: none


- **segmentMask**: Bit mask defining what segments should be turned on. Segments marks starting from A to G relate to mask bits 0 to 6 counting from the least significant bit. The 7th bit relates to radix segment and therefore it is ignored.
	- **Valid values**: 0 ~ 127
	- **Default value**: none

#### Returns
None

#### See also
[printDigitOn()](#printDigitSwitch)

[printDigitOff()](#printDigitSwitch)

[Back to interface](#interface)


<a id="printDigitSwitch"></a>
## printDigitOn(), printDigitOff()
#### Description
The particular method performs corresponding manipulation turning on or off with all glyph segments at once of the display without changing glyph radix segments.
- Each method is overloaded. If there is no input parameter provided, the method performs appropriate action on all controlled digital tubes.

#### Syntax
	void printDigitOn(uint8_t digit);
	void printDigitOn();
	void printDigitOff(uint8_t digit);
	void printDigitOff();

#### Parameters
- **digit**: controller's digit tube number counting from 0, which glyph segments should be manipulated.
	- **Valid values**: 0 ~ [digits - 1](#prm_digits) (from constructor)
	- **Default value**: none

#### Returns
None

#### See also
[printDigit()](#printDigit)

[Back to interface](#interface)


<a id="printText"></a>
## printText()
#### Description
The method prints text starting from provided or default position on digital tubes.
- The method clears the display right before printing.
- It is a wrapper method for subsequent calling methods [displayClear()](#displayClear) and system method *print()*.

#### Syntax
	void printText(const char* text, uint8_t digit);
	void printText(String text, uint8_t digit);

#### Parameters
- **text**: Pointer to a text that should be printed.
	- **Valid values**: microcontroller's addressing range
	- **Default value**: none


- **digit**: controller's digit tube number counting from 0, where printing should start.
	- **Valid values**: 0 ~ [digits - 1](#prm_digits) (from constructor)
	- **Default value**: 0

#### Returns
None

#### See also
[printGlyphs()](#printGlyphs)

[printDigit()](#printDigit)

[Back to interface](#interface)


<a id="printGlyphs"></a>
## printGlyphs()
#### Description
The method prints text starting from provided or default position on digital tubes without impact on radixes.
- The method clears digits right before printing leaving radixes intact.
- The method is suitable for displaying data, where radixes are independent of them and are used for another purpose.
- It is a wrapper method for subsequent calling methods [printDigitOff()](#printDigitOff), [placePrint()](#placePrint), and system method *print()*.

#### Syntax
	void printGlyphs(const char* text, uint8_t digit);
	void printGlyphs(String text, uint8_t digit);

#### Parameters
- **text**: Pointer to a text that should be printed.
	- **Valid values**: microcontroller's addressing range
	- **Default value**: none


- **digit**: controller's digit tube number counting from 0, where printing should start.
	- **Valid values**: 0 ~ [digits - 1](#prm_digits) (from constructor)
	- **Default value**: 0

#### Returns
None

#### See also
[printText()](#printText)

[Back to interface](#interface)


<a id="placePrint"></a>
## placePrint()
#### Description
The method stores desired position of a digital tube where the subsequent print should start.
- The method should be called right before any printing method, which does not have its input parameter for setting printing position.

#### Syntax
	void placePrint(uint8_t digit);

#### Parameters
- **digit**: Printing position for starting a print action.
	- **Valid values**: 0 ~ [digits - 1](#prm_digits) (from constructor)
	- **Default value**: 0

#### Returns
None

#### See also
[printDigit()](#printDigit)

[Back to interface](#interface)


<a id="write"></a>
## write()
#### Description
The library inherits the system *Print* class, so that all regular print functions can be used.
- Actually all print functions eventually call one of listed write methods, so that all of them should be implemented.
- If some character (ASCII) code is not present in the font table, i.e., it is unknown for the library, that character is ignored and not displayed.
- If unknown character has ASCII code of *comma*, *dot*, or *colon*, the library turns on the radix segments of the recently displayed digit. Thus, the decimal points or colon can be present in displayed string at proper position and does not need to be control separately.

#### Syntax
	size_t write(uint8_t ascii);
	size_t write(const char* text);
	size_t write(const uint8_t* buffer, size_t size);

#### Parameters
- **ascii**: ASCII code of a character that should be displayed at the current print position. The methods is usually utilized internally by system prints.
	- **Valid values**: 0 ~ 255
	- **Default value**: none


- **text**: Pointer to a null terminated string that should be displayed from the current print position.
	- **Valid values**: microcontroller's addressing range
	- **Default value**: none


- **buffer**: Pointer to a string, which part should be displayed from the current print position.
	- **Valid values**: microcontroller's addressing range
	- **Default value**: none


- **size**: Number of characters that should be displayed from the current print position.
	- **Valid values**: microcontroller's addressing range
	- **Default value**: none

#### Returns
None

#### See also
[printText()](#printText)

[placePrint()](#placePrint)

[Back to interface](#interface)


<a id="initLastResult"></a>
## initLastResult()
#### Description
The method sets internal status of recent processing of a controller code to success with value of constant [gbj\_tm1637::SUCCESS](#constants). It is usually called right before any operation with the controller in order to reset the internal status.

#### Syntax
    void initLastResult();

#### Parameters
None

#### Returns
None

#### See also
[getLastResult()](#getLastResult)

[setLastResult()](#setLastResult)

[Back to interface](#interface)


<a id="setLastResult"></a>
## setLastResult()
#### Description
The method sets the internal status of recent processing with controller to input value. Without input parameter it is equivalent to the method [initLastResult()](#initLastResult).

#### Syntax
    uint8_t setLastResult(uint8_t lastResult);

#### Parameters
- **lastResult**: Desired result code that should be set as a last result code.
  - *Valid values*: Some of [result or error codes](#constants).
  - *Default value*: [gbj\_tm1367::SUCCESS](#constants)

#### Returns
New (actual) result code of recent operation.

#### See also
[initLastResult()](#initLastResult)

[getLastResult()](#getLastResult)

[Back to interface](#interface)


<a id="setContrast"></a>
## setContrast()
#### Description
The method sets the level of the display contrast.
- The contrast is perceived as the brightness of the display.
- The brightness is technically implemented with <abbr title="Pulse Width Modulation">PWM</abbr> of segments power supply.

#### Syntax
	uint8_t setContrast(uint8_t contrast);

#### Parameters
- **contrast**: Level of contrast/brightness.
	- *Valid values*: 0 ~ 7 ([getContrastMax()](#getContrastMax))
	- *Default value*: 3

#### Returns
Some of [result or error codes](#constants).

[Back to interface](#interface)


<a id="setFont"></a>
## setFont()
#### Description
The method gathers font parameters for printing characters on 7-segment displays.
- Font definition is usually included to an application sketch from particular include file, while the font table resides in programmatic (flash) memory of a microcontroller in order to save operational memory (SRAM).
- Each glyph of a font consists of the pair of bytes. The first byte determines ASCII code of a glyph and second byte determines segment mask of a glyph. It allows to defined only displayable glyphs on 7-segment displays and suppress need to waste memory for useless characters.

#### Syntax
	void setFont(const uint8_t* fontTable, uint8_t fontTableSize);

#### Parameters
- **fontTable**: Pointer to constant byte array with font characters definitions. Because the font table resides in flash memory, it has to be constant.
	- *Valid values*: microcontroller addressing range
	- *Default value*: none


- **fontTableSize**: The number of bytes that should be utilized from the font table.
	- Because the font table is referenced by a pointer and not as an array, the table size cannot be calculated internally, but has to be defined externally usually by the function `sizeof`.
	- The table size in conjunction with font character pair of bytes determines the number of characters used for printing.
	- The size can be smaller than the real size of the table, however, the size should be a multiple of 2.
		- *Valid values*: 0 ~  255 (maximal 127 different characters)
		- *Default value*: none

#### Returns
None

#### Example
``` cpp
#include "font7seg_basic.h"
gbj_tm1637 Sled = gbj_tm1637();
setup()
{
 Sled.begin();
 Sled.setFont(gbjFont7segTable, sizeof(gbjFont7segTable));
}
```

#### See also
[Fonts](#Fonts)

[Back to interface](#interface)


<a id="getLastResult"></a>
## getLastResult()
#### Description
The method returns a result code of the recent operation with controller. It is usually called for error handling in a sketch.

#### Syntax
    uint8_t getLastResult();

#### Parameters
None

#### Returns
Current result code. It is some of expected [result or error codes](#constants).

#### See also
[setLastResult()](#setLastResult)

[initLastResult()](#initLastResult)

[Back to interface](#interface)


<a id="getLastCommand"></a>
## getLastCommand()
#### Description
The method returns the command code used at recent communication with controller. In conjunction with returned result or error code of particular method it is possible to detect the source or reason of a communication error.

#### Syntax
	uint8_t getLastCommand();

#### Parameters
None

#### Returns
Recently used command code.

#### See also
[Error codes](#constants)

[Back to interface](#interface)


<a id="getDigits"></a>
## getDigits()
#### Description
The method returns number of controlled digital tubes of a display module as it was defined in the corresponding constructor's parameter [digits](#prm_digits).

#### Syntax
	uint8_t getDigits();

#### Parameters
None

#### Returns
Current number of controlled digital tubes by a library instance object.

#### See also
[gbj_tm1637()](#gbj_tm1637)

[Back to interface](#interface)


<a id="getDigitsMax"></a>
## getDigitsMax()
#### Description
The method returns maximal number of digital tubes that the controller supports.

#### Syntax
	uint8_t getDigitsMax();

#### Parameters
None

#### Returns
Maximal supported number of digital tubes by the controller, which is 6.

#### See also
[getDigits()](#getDigits)

[Back to interface](#interface)


<a id="getContrast"></a>
## getContrast()
#### Description
The method returns the current contrast/brightness level store in the library instance object.

#### Syntax
	uint8_t getContrast();

#### Parameters
None

#### Returns
Current contrast level counting up to [getContrastMax()](#getContrastMax).

#### See also
[setContrast()](#setContrast)

[Back to interface](#interface)


<a id="getContrastMax"></a>
## getContrastMax()
#### Description
The method returns the maximal contrast/brightness level supported by the controller.

#### Syntax
	uint8_t getContrastMax();

#### Parameters
None

#### Returns
Maximal contrast level supported by the controller, which is 7.

#### See also
[getContrast()](#getContrast)

[Back to interface](#interface)


<a id="getPrint"></a>
## getPrint()
#### Description
The method returns the current print position set by recent print activity.

#### Syntax
	uint8_t getPrint();

#### Parameters
None

#### Returns
Current printing position counting from 0. It may get beyond the maximal controlled or implemented digital tube.

#### See also
[placePrint()](#placePrint)

[Back to interface](#interface)


<a id="isSuccess"></a>
## isSuccess()
#### Description
The method returns a flag whether the recent operation with controller was successful.

#### Syntax
    bool isSuccess();

#### Parameters
None

#### Returns
Flag about successful recent processing.

#### See also
[getLastResult()](#getLastResult)

[isError()](#isError)

[Back to interface](#interface)


<a id="isError"></a>
## isError()
#### Description
The method returns a flag whether the recent operation with controller failed. The corresponding error code can be obtained by the method [getLastResult()]((#getLastResult), which is one of the error [constants](#constants).

#### Syntax
    bool isError();

#### Parameters
None

#### Returns
Flag about failing of the recent operation.

#### See also
[getLastResult()](#getLastResult)

[isSuccess()](#isSuccess)

[Back to interface](#interface)
