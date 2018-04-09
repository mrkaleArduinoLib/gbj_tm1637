#include "gbj_tm1637.h"


gbj_tm1637::gbj_tm1637(uint8_t pinClk, uint8_t pinDio, uint8_t digits)
{
  _status.pinClk = pinClk;
  _status.pinDio = pinDio;
  _status.digits = min(digits, DIGITS);
}


uint8_t gbj_tm1637::begin()
{
  initLastResult();
  // Check pin duplicity
  if (_status.pinClk == _status.pinDio) return setLastResult(GBJ_TM1637_ERR_PINS);
  // Setup pins
  pinMode(_status.pinClk, OUTPUT);
  pinMode(_status.pinDio, OUTPUT);
  // Initialize controller
  displayClear();
  if (setContrast()) return getLastResult();
  return display();
}


//------------------------------------------------------------------------------
// Software manipulation - updating screen buffer
//------------------------------------------------------------------------------
// Print one character determined by a byte of ASCII code
size_t gbj_tm1637::write(uint8_t ascii)
{
  if (_print.digit >= _status.digits) return 0;
  uint8_t mask = getFontMask(ascii);
  if (mask == FONT_MASK_WRONG)
  {
    if (String(".,:").indexOf(ascii) >= 0)  // Detect radix
    {
      printRadixOn(_print.digit - 1); // Set radix to the previous digit
    }
    return 0;
  }
  else
  {
    printDigit(_print.digit, mask);
    return 1;
  }
}


// Print null terminated character array
size_t  gbj_tm1637::write(const char* text)
{
  uint8_t digits = 0;
  uint8_t i = 0;
  while (text[i] != '\0' && _print.digit < _status.digits)
  {
    digits += write(text[i++]);
  }
  return digits;
}


// Print byte array with length
size_t  gbj_tm1637::write(const uint8_t* buffer, size_t size)
{
  uint8_t digits = 0;
  for (uint8_t i = 0; i < size && _print.digit < _status.digits; i++)
  {
    digits += write(buffer[i]);
  }
  return digits;
}


//------------------------------------------------------------------------------
// Hardware manipulation - communication with the controller
//------------------------------------------------------------------------------
uint8_t gbj_tm1637::display()
{
  // Automatic addressing
  if (busSend(CMD_DATA_INIT | CMD_DATA_NORMAL | CMD_DATA_WRITE | CMD_DATA_AUTO)) return getLastResult();
  if (busSend(CMD_ADDR_INIT, _print.buffer, sizeof(_print.buffer) / sizeof(_print.buffer[0]))) return getLastResult();
  return getLastResult();
}


uint8_t gbj_tm1637::displayOn()
{
  return setContrast(_status.contrast);
}


uint8_t gbj_tm1637::displayOff()
{
  return busSend(CMD_DISP_INIT | CMD_DISP_OFF);
}


//------------------------------------------------------------------------------
// Setters
//------------------------------------------------------------------------------
uint8_t gbj_tm1637::setContrast(uint8_t contrast)
{
  _status.contrast = contrast & 0x07;
  return busSend(CMD_DISP_INIT | CMD_DISP_ON | _status.contrast);
}


void gbj_tm1637::setFont(const uint8_t* fontTable, uint8_t fontTableSize)
{
  _font.table = fontTable;
  _font.glyphs = fontTableSize / FONT_WIDTH;
}


//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
// Wait for delay period expiry
void gbj_tm1637::waitPulseClk()
{
  delayMicroseconds(TIMING_RELAX);
}


// Start condition - pull dow DIO from HIGH to LOW while CLK is HIGH
void gbj_tm1637::beginTransmission()
{
  digitalWrite(_status.pinDio, HIGH);
  digitalWrite(_status.pinClk, HIGH);
  waitPulseClk();
  digitalWrite(_status.pinDio, LOW);
}


// Stop condition - pull up DIO from LOW to HIGH while CLK is HIGH
void gbj_tm1637::endTransmission()
{
  digitalWrite(_status.pinClk, LOW);
  waitPulseClk();
  digitalWrite(_status.pinDio, LOW);
  waitPulseClk();
  digitalWrite(_status.pinClk, HIGH);
  waitPulseClk();
  digitalWrite(_status.pinDio, HIGH);
}


// Acknowledgment - wait for DIO to be pull LOW from HIGH while CLK is HIGH
uint8_t gbj_tm1637::ackTransmission()
{
  initLastResult();
  digitalWrite(_status.pinClk, LOW);
  digitalWrite(_status.pinDio, HIGH);
  pinMode(_status.pinDio, INPUT_PULLUP);
  waitPulseClk();
  digitalWrite(_status.pinClk, HIGH);
  waitPulseClk();
  digitalWrite(_status.pinDio, HIGH);
  // Wait for acknowledge
  uint32_t tsStart = micros();
  while (digitalRead(_status.pinDio))
  {
    if (millis() - tsStart > TIMING_ACK)
    {
      setLastResult(GBJ_TM1637_ERR_ACK);
      break;
    }
  }
  pinMode(_status.pinDio, OUTPUT);
  digitalWrite(_status.pinDio, LOW);
  return getLastResult();
}


void gbj_tm1637::busWrite(uint8_t data)
{
  digitalWrite(_status.pinClk, LOW); // For active rising edge of clock pulse
  shiftOut(_status.pinDio, _status.pinClk, LSBFIRST, data);
}


uint8_t gbj_tm1637::busSend(uint8_t command)
{
  beginTransmission();
  busWrite(setLastCommand(command));
  ackTransmission();
  endTransmission();
  return getLastResult();
}


uint8_t gbj_tm1637::busSend(uint8_t command, uint8_t data)
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


uint8_t gbj_tm1637::busSend(uint8_t command, uint8_t* buffer, uint8_t bufferItems)
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
    busWrite(*buffer++);
    if (ackTransmission()) break;
  }
  endTransmission();
  return getLastResult();
}


// The method leaves digit cursor after last print digit
void gbj_tm1637::gridWrite(uint8_t segmentMask, uint8_t gridStart, uint8_t gridStop)
{
  swapByte(gridStart, gridStop);
  gridStop = min(gridStop, _status.digits - 1);
  for (_print.digit = gridStart; _print.digit <= gridStop; _print.digit++)
  {
    segmentMask &= 0x7F; // Clear radix bit in segment mask
    _print.buffer[addrGrid(_print.digit)] &= 0x80;  // Clear digit bits in screen buffer
    _print.buffer[addrGrid(_print.digit)] |= segmentMask;  // Set digit bits but leave radix bit intact
  }
}


uint8_t gbj_tm1637::getFontMask(uint8_t ascii)
{
  uint8_t mask = FONT_MASK_WRONG;
  for (uint8_t glyph = 0; glyph < _font.glyphs; glyph++)
  {
    if (ascii == pgm_read_byte(&_font.table[glyph * FONT_WIDTH + FONT_INDEX_ASCII]))
    {
      mask = pgm_read_byte(&_font.table[glyph*2 + FONT_INDEX_MASK]);
      mask &= 0x7F; // Clear radix bit not to mess with wrong mask
      break;
    }
  }
  return mask;
}
