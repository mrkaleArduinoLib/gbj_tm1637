#include "gbj_tm1637.h"
const String gbj_tm1637::VERSION = "GBJ_TM1637 1.1.0";

gbj_tm1637::ResultCodes gbj_tm1637::begin()
{
  setLastResult();
  // Check pin duplicity
  if (status_.pinClk == status_.pinDio)
    return setLastResult(ResultCodes::ERROR_PINS);
  // Setup pins
  pinMode(status_.pinClk, OUTPUT);
  pinMode(status_.pinDio, OUTPUT);
  // Initialize controller
  displayClear();
  return setContrast();
}

// Print one character determined by a byte of ASCII code
size_t gbj_tm1637::write(uint8_t ascii)
{
  if (print_.digit >= status_.digits)
    return 0;
  uint8_t mask = getFontMask(ascii);
  if (mask == Rasters::FONT_MASK_WRONG)
  {
    if (String(".,:").indexOf(ascii) >= 0) // Detect radix
    {
      printRadixOn(print_.digit - 1); // Set radix to the previous digit
    }
    return 0;
  }
  else
  {
    printDigit(mask, print_.digit);
    return 1;
  }
}

// Print null terminated character array
size_t gbj_tm1637::write(const char *text)
{
  uint8_t digits = 0;
  uint8_t i = 0;
  while (text[i] != '\0' && print_.digit < status_.digits)
  {
    digits += write(text[i++]);
  }
  return digits;
}

// Print byte array with length
size_t gbj_tm1637::write(const uint8_t *buffer, size_t size)
{
  uint8_t digits = 0;
  for (uint8_t i = 0; i < size && print_.digit < status_.digits; i++)
  {
    digits += write(buffer[i]);
  }
  return digits;
}

gbj_tm1637::ResultCodes gbj_tm1637::display(uint8_t *digitReorder)
{
  // Automatic addressing
  if (busSend(Commands::CMD_DATA_INIT | Commands::CMD_DATA_NORMAL |
              Commands::CMD_DATA_WRITE | Commands::CMD_DATA_AUTO))
    return getLastResult();
  if (busSend(
        Commands::CMD_ADDR_INIT, print_.buffer, status_.digits, digitReorder))
    return getLastResult();
  return getLastResult();
}

gbj_tm1637::ResultCodes gbj_tm1637::displayOn()
{
  if (!busSend(Commands::CMD_DISP_INIT | Commands::CMD_DISP_ON |
               status_.contrast))
  {
    status_.state = true;
  }
  return getLastResult();
}

gbj_tm1637::ResultCodes gbj_tm1637::displayOff()
{
  if (!busSend(Commands::CMD_DISP_INIT | Commands::CMD_DISP_OFF))
  {
    status_.state = false;
  }
  return getLastResult();
}

gbj_tm1637::ResultCodes gbj_tm1637::setContrast(uint8_t contrast)
{
  status_.contrast = contrast & getContrastMax();
  return displayOn();
}

void gbj_tm1637::setFont(const uint8_t *fontTable, uint8_t fontTableSize)
{
  font_.table = fontTable;
  font_.glyphs = fontTableSize / Rasters::FONT_WIDTH;
}

// Wait for delay period expiry
void gbj_tm1637::waitPulseClk()
{
  delayMicroseconds(Timing::TIMING_RELAX);
}

// Start condition - pull down DIO from HIGH to LOW while CLK is HIGH
void gbj_tm1637::beginTransmission()
{
  digitalWrite(status_.pinClk, LOW);
  digitalWrite(status_.pinDio, HIGH);
  digitalWrite(status_.pinClk, HIGH);
  digitalWrite(status_.pinDio, LOW);
}

// Stop condition - pull up DIO from LOW to HIGH while CLK is HIGH
void gbj_tm1637::endTransmission()
{
  digitalWrite(status_.pinClk, LOW);
  digitalWrite(status_.pinDio, LOW);
  digitalWrite(status_.pinClk, HIGH);
  digitalWrite(status_.pinDio, HIGH);
}

// Acknowledgment - wait for DIO to be pull LOW from HIGH while CLK is HIGH
gbj_tm1637::ResultCodes gbj_tm1637::ackTransmission()
{
  setLastResult();
  pinMode(status_.pinDio, INPUT_PULLUP);
  digitalWrite(status_.pinClk, HIGH);
  // Wait for acknowledge
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

void gbj_tm1637::busWrite(uint8_t data)
{
  digitalWrite(status_.pinClk, LOW); // For active rising edge of clock pulse
  shiftOut(status_.pinDio, status_.pinClk, LSBFIRST, data);
}

gbj_tm1637::ResultCodes gbj_tm1637::busSend(uint8_t command)
{
  beginTransmission();
  busWrite(setLastCommand(command));
  ackTransmission();
  endTransmission();
  return getLastResult();
}

gbj_tm1637::ResultCodes gbj_tm1637::busSend(uint8_t command, uint8_t data)
{
  beginTransmission();
  // Send one command byte
  busWrite(setLastCommand(command));
  if (ackTransmission())
  {
    endTransmission();
    return getLastResult();
  };
  // Send one data byte
  busWrite(data);
  ackTransmission();
  endTransmission();
  return getLastResult();
}

gbj_tm1637::ResultCodes gbj_tm1637::busSend(uint8_t command,
                                            uint8_t *buffer,
                                            uint8_t bufferItems,
                                            uint8_t *digitReorder)
{
  beginTransmission();
  // Send one command byte
  busWrite(setLastCommand(command));
  if (ackTransmission())
  {
    endTransmission();
    return getLastResult();
  };
  // Send data byte stream
  for (uint8_t bufferIndex = 0; bufferIndex < bufferItems; bufferIndex++)
  {
    if (digitReorder)
    {
      busWrite(buffer[digitReorder[bufferIndex]]);
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

// The method leaves digit cursor after last print digit
void gbj_tm1637::gridWrite(uint8_t segmentMask,
                           uint8_t gridStart,
                           uint8_t gridStop)
{
  swapByte(gridStart, gridStop);
  gridStop = min(gridStop, (uint8_t)(status_.digits - 1));
  for (print_.digit = gridStart; print_.digit <= gridStop; print_.digit++)
  {
    segmentMask &= 0x7F; // Clear radix bit in segment mask
    print_.buffer[print_.digit] &= 0x80; // Clear digit bits in screen buffer
    print_.buffer[print_.digit] |=
      segmentMask; // Set digit bits but leave radix bit intact
  }
}

uint8_t gbj_tm1637::getFontMask(uint8_t ascii)
{
  uint8_t mask = FONT_MASK_WRONG;
  for (uint8_t glyph = 0; glyph < font_.glyphs; glyph++)
  {
    if (ascii ==
        pgm_read_byte(&font_.table[glyph * FONT_WIDTH + FONT_INDEX_ASCII]))
    {
      mask = pgm_read_byte(&font_.table[glyph * 2 + FONT_INDEX_MASK]);
      mask &= 0x7F; // Clear radix bit not to mess with wrong mask
      break;
    }
  }
  return mask;
}
