/**
 * @file gbj_tm1637.h
 * @brief Library for 7-segment digital tube displays controlled by TM1637.
 * @details The library controls TM1637 as a state machine with a local screen
 * buffer mirrored to the controller memory. Printing methods modify the screen
 * buffer and the display method transmits the buffer to the controller.
 * @details TM1637 can control up to 6 digital tubes with radix segments. The
 * library can also drive TM1636, which is binary compatible with TM1637 and
 * controls 4 digital tubes.
 * @details Key scan capabilities are not implemented because typical TM1637
 * display modules do not expose keypads.
 *
 * @copyright This program is free software; you can redistribute it and/or
 * modify it under the terms of the license GNU GPL v3
 * http://www.gnu.org/licenses/gpl-3.0.html (related to original code) and MIT
 * License (MIT) for added code.
 *
 * @author Libor Gabaj
 * @see https://github.com/mrkaleArduinoLib/gbj_tm1637.git
 */
#ifndef GBJ_TM1637_H
#define GBJ_TM1637_H

#if defined(__AVR__)
  #include <Arduino.h>
  #include <avr/pgmspace.h>
  #include <inttypes.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <Arduino.h>
#endif

/**
 * @class gbj_tm1637
 * @brief TM1637/TM1636 7-segment display driver.
 */
class gbj_tm1637 : public Print
{
public:
  enum ResultCodes : uint8_t
  {
    SUCCESS = 0,
    ERROR_PINS,
    ERROR_ACK,
  };

  /**
   * @brief Construct a TM1637/TM1636 display driver instance.
   * @details Constructor sanitizes and stores physical display configuration.
   * @param pinClk Microcontroller pin number used as serial clock.
   * @param pinDio Microcontroller pin number used as data input/output.
   * @param digits Number of controlled digital tubes (1 to 6 for TM1637, 1 to 4 for TM1636).
   */
  inline gbj_tm1637(uint8_t pinClk = 2, uint8_t pinDio = 3, uint8_t digits = 4)
  {
    status_.pinClk = pinClk;
    status_.pinDio = pinDio;
    status_.digits = min(digits, (uint8_t)Geometry::DIGITS);
  }

  /**
   * @brief Initialize the display driver and hardware pins.
   * @details Configures communication pins, validates pin mapping, clears the
   * display, and sets default contrast and operation mode.
   * @return Result code of the initialization sequence.
   */
  inline ResultCodes begin()
  {
    setLastResult();
    if (status_.pinClk == status_.pinDio)
      return setLastResult(ResultCodes::ERROR_PINS);
    pinMode(status_.pinClk, OUTPUT);
    pinMode(status_.pinDio, OUTPUT);
    displayClear();
    return setContrast();
  }

  /**
   * @brief Transmit the screen buffer to the controller.
   * @details Sends the current buffer using automatic addressing and optional
   * digit reordering for displays with different physical tube order.
   * @param digitReorder Optional transformation table mapping logical buffer
   * indexes to physical display positions.
   * @return Result code of the transmission.
   */
  inline ResultCodes display(uint8_t *digitReorder = 0)
  {
    if (busSend(Commands::CMD_DATA_INIT | Commands::CMD_DATA_NORMAL |
                Commands::CMD_DATA_WRITE | Commands::CMD_DATA_AUTO))
      return getLastResult();
    if (busSend(
          Commands::CMD_ADDR_INIT, print_.buffer, status_.digits, digitReorder))
      return getLastResult();
    return getLastResult();
  }

  /**
   * @brief Turn the display output on or off.
   * @details These methods change global display state without modifying the
   * screen buffer content.
   * @return Result code of the command sent to the controller.
   */
  inline ResultCodes displayOn()
  {
    if (!busSend(Commands::CMD_DISP_INIT | Commands::CMD_DISP_ON |
                 status_.contrast))
    {
      status_.state = true;
    }
    return getLastResult();
  }
  inline ResultCodes displayOff()
  {
    if (!busSend(Commands::CMD_DISP_INIT | Commands::CMD_DISP_OFF))
    {
      status_.state = false;
    }
    return getLastResult();
  }

  /**
   * @brief Toggle display power state.
   * @return Result code of the issued display command.
   */
  inline ResultCodes displayToggle()
  {
    return status_.state ? displayOff() : displayOn();
  }

  /**
   * @brief Toggle between minimal and maximal contrast.
   * @return Result code of the contrast command.
   */
  ResultCodes displayBreath()
  {
    return status_.contrast > getContrastMin() ? setContrastMin()
                                               : setContrastMax();
  }

  /**
   * @brief Clear all glyph and radix segments and set print position.
   * @param digit Target digit index for subsequent printing.
   */
  inline void displayClear(uint8_t digit = 0)
  {
    printDigitOff();
    printRadixOff();
    placePrint(digit);
  }

  /**
   * @brief Control radix segments independently from glyph segments.
   * @param digit Digit index whose radix segment should be changed.
   */
  inline void printRadixOn(uint8_t digit)
  {
    if (digit < status_.digits)
      print_.buffer[digit] |= 0x80;
  }

  /**
   * @brief Turn radix segments on for all digits.
   */
  inline void printRadixOn()
  {
    for (uint8_t digit = 0; digit < status_.digits; digit++)
      printRadixOn(digit);
  }

  /**
   * @brief Turn radix segment off for one digit.
   * @param digit Target digit index.
   */
  inline void printRadixOff(uint8_t digit)
  {
    if (digit < status_.digits)
      print_.buffer[digit] &= ~0x80;
  }

  /**
   * @brief Turn radix segments off for all digits.
   */
  inline void printRadixOff()
  {
    for (uint8_t digit = 0; digit < status_.digits; digit++)
      printRadixOff(digit);
  }

  /**
   * @brief Toggle radix segment for one digit.
   * @param digit Target digit index.
   */
  inline void printRadixToggle(uint8_t digit)
  {
    if (digit < status_.digits)
      print_.buffer[digit] ^= 0x80;
  }

  /**
   * @brief Toggle radix segments for all digits.
   */
  inline void printRadixToggle()
  {
    for (uint8_t digit = 0; digit < status_.digits; digit++)
      printRadixToggle(digit);
  }

  /**
   * @brief Set glyph segments of one digit in the screen buffer.
   * @param segmentMask Bit mask for segments A-G. Radix bit is ignored.
   * @param digit Target digit index.
   */
  inline void printDigit(uint8_t segmentMask = 0b01111111, uint8_t digit = 0)
  {
    if (digit < status_.digits)
      gridWrite(segmentMask, digit, digit);
  }

  /**
   * @brief Set glyph segments of all digits in the screen buffer.
   * @param segmentMask Bit mask for segments A-G. Radix bit is ignored.
   */
  inline void printDigitAll(uint8_t segmentMask = 0b01111111)
  {
    gridWrite(segmentMask);
  }

  /**
   * @brief Turn complete glyphs on or off.
   * @details These wrappers apply full segment masks to one or all digits
   * without changing radix segments.
   */
  inline void printDigitOn(uint8_t digit) { printDigit(0x7F, digit); }

  /**
   * @brief Turn glyph segments on for all digits.
   */
  inline void printDigitOn() { printDigitAll(0x7F); }

  /**
   * @brief Turn glyph segments off for one digit.
   * @param digit Target digit index.
   */
  inline void printDigitOff(uint8_t digit) { printDigit(0x00, digit); }

  /**
   * @brief Turn glyph segments off for all digits.
   */
  inline void printDigitOff() { printDigitAll(0x00); }

  /**
   * @brief Set current print cursor position.
   * @param digit Digit index where subsequent print operations should start.
   */
  inline void placePrint(uint8_t digit = 0)
  {
    if (digit < status_.digits)
      print_.digit = digit;
  };

  /**
   * @brief Print text after clearing full display content.
   * @param text Text to print.
   * @param digit Start digit index for printing.
   */
  inline void printText(const char *text, uint8_t digit = 0)
  {
    displayClear(digit);
    print(text);
  };

  /**
   * @brief Print String text after clearing full display content.
   * @param text Text to print.
   * @param digit Start digit index for printing.
   */
  inline void printText(String text, uint8_t digit = 0)
  {
    displayClear(digit);
    print(text);
  };

  /**
   * @brief Print text while keeping current radix segments unchanged.
   * @param text Text to print.
   * @param digit Start digit index for printing.
   */
  inline void printGlyphs(const char *text, uint8_t digit = 0)
  {
    printDigitOff();
    placePrint(digit);
    print(text);
  };

  /**
   * @brief Print String text while keeping current radix segments unchanged.
   * @param text Text to print.
   * @param digit Start digit index for printing.
   */
  inline void printGlyphs(String text, uint8_t digit = 0)
  {
    printDigitOff();
    placePrint(digit);
    print(text);
  };

  /**
   * @brief Write one character to the current print position.
   * @details Unknown glyphs are ignored. Characters '.', ',', and ':' are
   * treated as radix markers and applied to the previous printed digit.
   * @param ascii ASCII code of the character to write.
   * @return Number of printed digits.
   */
  inline size_t write(uint8_t ascii)
  {
    if (print_.digit >= status_.digits)
      return 0;
    uint8_t mask = getFontMask(ascii);
    if (mask == Rasters::FONT_MASK_WRONG)
    {
      if (String(".,:").indexOf(ascii) >= 0)
      {
        printRadixOn(print_.digit - 1);
      }
      return 0;
    }
    else
    {
      printDigit(mask, print_.digit);
      return 1;
    }
  }

  /**
   * @brief Write a null-terminated character array.
   * @param text Text buffer to print.
   * @return Number of printed digits.
   */
  inline size_t write(const char *text)
  {
    uint8_t digits = 0;
    uint8_t i = 0;
    while (text[i] != '\0' && print_.digit < status_.digits)
    {
      digits += write(text[i++]);
    }
    return digits;
  }

  /**
   * @brief Write a byte buffer with explicit size.
   * @param buffer Input byte buffer.
   * @param size Number of bytes to process.
   * @return Number of printed digits.
   */
  inline size_t write(const uint8_t *buffer, size_t size)
  {
    uint8_t digits = 0;
    for (uint8_t i = 0; i < size && print_.digit < status_.digits; i++)
    {
      digits += write(buffer[i]);
    }
    return digits;
  }

  /**
   * @brief Store a result code as the latest operation result.
   * @param result Result code to store.
   * @return Stored result code.
   */
  inline ResultCodes setLastResult(ResultCodes result = ResultCodes::SUCCESS)
  {
    return status_.lastResult = result;
  };

  /**
   * @brief Set brightness level and force display on.
   * @param contrast Contrast level in range 0 to 7.
   * @return Result code of the display command.
   */
  inline ResultCodes setContrast(uint8_t contrast = 3)
  {
    status_.contrast = contrast & getContrastMax();
    return displayOn();
  }

  /**
   * @brief Set minimal contrast.
   * @return Result code of the contrast command.
   */
  ResultCodes setContrastMin() { return setContrast(getContrastMin()); };

  /**
   * @brief Set maximal contrast.
   * @return Result code of the contrast command.
   */
  ResultCodes setContrastMax() { return setContrast(getContrastMax()); };

  /**
   * @brief Configure custom font table for printable glyphs.
   * @details Each glyph entry consists of two bytes: ASCII code and segment
   * mask. The table is typically stored in flash memory.
   * @param fontTable Pointer to the font definition table.
   * @param fontTableSize Number of bytes used from the table.
   */
  inline void setFont(const uint8_t *fontTable, uint8_t fontTableSize)
  {
    font_.table = fontTable;
    font_.glyphs = fontTableSize / Rasters::FONT_WIDTH;
  }

  /**
   * @brief Get latest operation result code.
   * @return Last stored result code.
   */
  inline ResultCodes getLastResult() { return status_.lastResult; }

  /**
   * @brief Check whether latest operation result is success.
   * @return True if latest result is SUCCESS.
   */
  inline bool isSuccess() { return status_.lastResult == ResultCodes::SUCCESS; }

  /**
   * @brief Store and evaluate provided result code for success.
   * @param result Result code to store and evaluate.
   * @return True if provided result equals SUCCESS.
   */
  inline bool isSuccess(ResultCodes result)
  {
    setLastResult(result);
    return isSuccess();
  }

  /**
   * @brief Check whether latest operation result indicates an error.
   * @return True if latest result is not SUCCESS.
   */
  inline bool isError() { return !isSuccess(); }

  /**
   * @brief Store and evaluate provided result code for error.
   * @param result Result code to store and evaluate.
   * @return True if provided result is not SUCCESS.
   */
  inline bool isError(ResultCodes result)
  {
    setLastResult(result);
    return isError();
  }

  /**
   * @brief Check whether display state is on.
   * @return True if display output is enabled.
   */
  inline bool isDisplayOn() { return status_.state; }

  /**
   * @brief Check whether display state is off.
   * @return True if display output is disabled.
   */
  inline bool isDisplayOff() { return !isDisplayOn(); }

  /**
   * @brief Get last command value sent to controller.
   * @return Last command byte.
   */
  inline uint8_t getLastCommand() { return status_.lastCommand; }

  /**
   * @brief Get configured count of active digits.
   * @return Number of controlled digits.
   */
  inline uint8_t getDigits()
  {
    return status_.digits;
  }

  /**
   * @brief Get maximal number of supported digits.
   * @return Compile-time maximum supported digits.
   */
  static inline uint8_t getDigitsMax()
  {
    return Geometry::DIGITS;
  }

  /**
   * @brief Get current contrast level.
   * @return Contrast value in range 0 to 7.
   */
  inline uint8_t getContrast() { return status_.contrast; }

  /**
   * @brief Get maximal allowed contrast value.
   * @return Maximal contrast.
   */
  static inline uint8_t getContrastMax() { return 7; }

  /**
   * @brief Get minimal allowed contrast value.
   * @return Minimal contrast.
   */
  static inline uint8_t getContrastMin() { return 0; }

  /**
   * @brief Get current print cursor position.
   * @return Current digit index for subsequent print operations.
   */
  inline uint8_t getPrint() { return print_.digit; }

private:

  /**
   * @brief TM1637 command bit patterns.
   * @details Values are combined and sent over the serial bus to control data
   * mode, addressing mode, and display power/brightness.
   */
  enum Commands : uint8_t
  {
    /** @brief Base value for data command set. */
    CMD_DATA_INIT = 0b01000000,
    /** @brief Data write mode. */
    CMD_DATA_WRITE = 0b00,
    /** @brief Data read mode. */
    CMD_DATA_READ = 0b10,
    /** @brief Auto-increment address mode. */
    CMD_DATA_AUTO = 0b000,
    /** @brief Fixed address mode. */
    CMD_DATA_FIXED = 0b100,
    /** @brief Normal operating mode. */
    CMD_DATA_NORMAL = 0b0000,
    /** @brief Test mode. */
    CMD_DATA_TEST = 0b1000,
    /** @brief Base value for address command set. */
    CMD_ADDR_INIT =
      0b11000000,
    /** @brief Base value for display control command set. */
    CMD_DISP_INIT =
      0b10000000,
    /** @brief Display off flag. */
    CMD_DISP_OFF = 0b0000,
    /** @brief Display on flag. */
    CMD_DISP_ON = 0b1000,
  };

  /**
   * @brief Display geometry limits.
   */
  enum Geometry : uint8_t
  {
    /** @brief Maximum supported digit count. */
    DIGITS = 6,
    /** @brief Maximum addressable bytes in TM1637 display RAM. */
    BYTES_ADDR = 6,
  };

  /**
   * @brief Timing constants for bus and acknowledge handling.
   */
  enum Timing : uint16_t
  {
    /** @brief Relaxation delay in microseconds between signal changes. */
    TIMING_RELAX = 2,
    /** @brief Acknowledge timeout in microseconds. */
    TIMING_ACK = 500,
  };

  /**
   * @brief Font table layout and sentinel values.
   */
  enum Rasters : uint8_t
  {
    /** @brief Number of bytes per glyph entry in font table. */
    FONT_WIDTH = 2,
    /** @brief Index of ASCII code in glyph entry. */
    FONT_INDEX_ASCII = 0,
    /** @brief Index of segment mask in glyph entry. */
    FONT_INDEX_MASK = 1,
    /** @brief Marker for unknown glyph lookup result. */
    FONT_MASK_WRONG = 0xFF,
  };

  /**
   * @brief Runtime print buffer and cursor state.
   */
  struct Print
  {
    /** @brief Segment masks for each display position. */
    uint8_t buffer[Geometry::BYTES_ADDR];
    /** @brief Current print cursor position. */
    uint8_t digit;
  } print_;

  /**
   * @brief Active font table descriptor.
   */
  struct Bitmap
  {
    /** @brief Pointer to font table in memory. */
    const uint8_t *table;
    /** @brief Number of glyph entries available in the table. */
    uint8_t glyphs;
  } font_;

  /**
   * @brief Driver status and hardware configuration.
   */
  struct Status
  {
    /** @brief Result of the most recent operation. */
    ResultCodes lastResult;
    /** @brief Most recently sent command byte. */
    uint8_t lastCommand;
    /** @brief Clock pin number. */
    uint8_t pinClk;
    /** @brief Data pin number. */
    uint8_t pinDio;
    /** @brief Active digit count. */
    uint8_t digits;
    /** @brief Current display contrast level. */
    uint8_t contrast;
    /** @brief Current display power state. */
    bool state = true;
  } status_;

  /**
   * @brief Swap two byte values when first is greater than second.
   * @param a First value.
   * @param b Second value.
   */
  inline void swapByte(uint8_t a, uint8_t b)
  {
    if (a > b)
    {
      uint8_t t = a;
      a = b;
      b = t;
    }
  };

  /**
   * @brief Store command byte as latest command.
   * @param lastCommand Command byte to store.
   * @return Stored command byte.
   */
  inline uint8_t setLastCommand(uint8_t lastCommand)
  {
    return status_.lastCommand = lastCommand;
  };

  /**
   * @brief Wait one timing slot for clock pulse relaxation.
   */
  inline void waitPulseClk()
  {
    delayMicroseconds(Timing::TIMING_RELAX);
  }

  /**
   * @brief Generate TM1637 start condition on the bus.
   */
  inline void beginTransmission()
  {
    digitalWrite(status_.pinClk, LOW);
    digitalWrite(status_.pinDio, HIGH);
    digitalWrite(status_.pinClk, HIGH);
    digitalWrite(status_.pinDio, LOW);
  }

  /**
   * @brief Generate TM1637 stop condition on the bus.
   */
  inline void endTransmission()
  {
    digitalWrite(status_.pinClk, LOW);
    digitalWrite(status_.pinDio, LOW);
    digitalWrite(status_.pinClk, HIGH);
    digitalWrite(status_.pinDio, HIGH);
  }

  /**
   * @brief Write one byte on the TM1637 serial bus.
   * @param data Byte to transmit.
   */
  inline void busWrite(uint8_t data)
  {
    digitalWrite(status_.pinClk, LOW);
    shiftOut(status_.pinDio, status_.pinClk, LSBFIRST, data);
  }

  /**
   * @brief Write a segment mask to one or more digits in local screen buffer.
   * @param segmentMask Segment mask to apply.
   * @param gridStart Start digit index.
   * @param gridStop End digit index.
   */
  inline void gridWrite(uint8_t segmentMask = 0x00,
                        uint8_t gridStart = 0,
                        uint8_t gridStop = DIGITS)
  {
    swapByte(gridStart, gridStop);
    gridStop = min(gridStop, (uint8_t)(status_.digits - 1));
    for (print_.digit = gridStart; print_.digit <= gridStop; print_.digit++)
    {
      segmentMask &= 0x7F;
      print_.buffer[print_.digit] &= 0x80;
      print_.buffer[print_.digit] |=
        segmentMask;
    }
  }

  /**
   * @brief Read acknowledge bit after byte transfer.
   * @return Result code of acknowledge phase.
   */
  inline ResultCodes ackTransmission()
  {
    setLastResult();
    pinMode(status_.pinDio, INPUT_PULLUP);
    digitalWrite(status_.pinClk, HIGH);
    uint32_t tsStart = micros();
    while (digitalRead(status_.pinDio))
    {
      if (millis() - tsStart > Timing::TIMING_ACK)
      {
        setLastResult(ResultCodes::ERROR_ACK);
        break;
      }
    }
    digitalWrite(status_.pinClk, LOW);
    digitalWrite(status_.pinDio, LOW);
    pinMode(status_.pinDio, OUTPUT);
    return getLastResult();
  }

  /**
   * @brief Send one command byte.
   * @param command Command byte.
   * @return Result code of transfer.
   */
  inline ResultCodes busSend(uint8_t command)
  {
    beginTransmission();
    busWrite(setLastCommand(command));
    ackTransmission();
    endTransmission();
    return getLastResult();
  }

  /**
   * @brief Send command byte followed by one data byte.
   * @param command Command byte.
   * @param data Data byte.
   * @return Result code of transfer.
   */
  inline ResultCodes busSend(uint8_t command, uint8_t data)
  {
    beginTransmission();
    busWrite(setLastCommand(command));
    if (ackTransmission())
    {
      endTransmission();
      return getLastResult();
    };
    busWrite(data);
    ackTransmission();
    endTransmission();
    return getLastResult();
  }

  /**
   * @brief Send command and data stream with optional digit reordering.
   * @param command Command byte.
   * @param buffer Pointer to data buffer.
   * @param bufferBytes Number of bytes to send.
   * @param transform Optional transformation table for digit order.
   * @return Result code of transfer.
   */
  inline ResultCodes busSend(uint8_t command,
                             uint8_t *buffer,
                             uint8_t bufferBytes,
                             uint8_t *transform = 0)
  {
    beginTransmission();
    busWrite(setLastCommand(command));
    if (ackTransmission())
    {
      endTransmission();
      return getLastResult();
    };
    for (uint8_t bufferIndex = 0; bufferIndex < bufferBytes; bufferIndex++)
    {
      if (transform)
      {
        busWrite(buffer[transform[bufferIndex]]);
      }
      else
      {
        busWrite(*buffer++);
      }
      if (ackTransmission())
        break;
    }
    endTransmission();
    return getLastResult();
  }

  /**
   * @brief Lookup segment mask in font table by ASCII code.
   * @param ascii ASCII code to search for.
   * @return Segment mask, or FONT_MASK_WRONG if not found.
   */
  inline uint8_t getFontMask(uint8_t ascii)
  {
    uint8_t mask = FONT_MASK_WRONG;
    for (uint8_t glyph = 0; glyph < font_.glyphs; glyph++)
    {
      if (ascii ==
          pgm_read_byte(&font_.table[glyph * FONT_WIDTH + FONT_INDEX_ASCII]))
      {
        mask = pgm_read_byte(&font_.table[glyph * 2 + FONT_INDEX_MASK]);
        mask &= 0x7F;
        break;
      }
    }
    return mask;
  }
};

#endif
