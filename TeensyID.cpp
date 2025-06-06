/* TeensyMAC library code is placed under the MIT license
 * Copyright (c) 2017 Stefan Staub
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "TeensyID.h"

// UNIQUE_ID
#if defined ARDUINO_TEENSY40 || defined ARDUINO_TEENSY41
static uint32_t getTeensySerial(void)
{
  uint32_t num;
  num = HW_OCOTP_MAC0 & 0xFFFFFF;
  return num;
}

#else
static uint32_t getTeensySerial(void)
{
  uint32_t num = 0;
  __disable_irq();
#if defined(HAS_KINETIS_FLASH_FTFA) || defined(HAS_KINETIS_FLASH_FTFL)
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  FTFL_FCCOB0 = 0x41;
  FTFL_FCCOB1 = 15;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF))
    ; // wait
  num = *(uint32_t *)&FTFL_FCCOB7;
#elif defined(HAS_KINETIS_FLASH_FTFE)
  kinetis_hsrun_disable();
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  *(uint32_t *)&FTFL_FCCOB3 = 0x41070000;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF))
    ; // wait
  num = *(uint32_t *)&FTFL_FCCOBB;
  kinetis_hsrun_enable();
#endif
  __enable_irq();
  return num;
}
#endif

uint32_t teensyUsbSN()
{
  uint32_t num = getTeensySerial();
  if (num < 10000000)
    num = num * 10;
  return num;
}

void teensySN(uint8_t *sn)
{
  uint32_t num = getTeensySerial();
  sn[0] = num >> 24;
  sn[1] = num >> 16;
  sn[2] = num >> 8;
  sn[3] = num;
}

const char *teensySN(void)
{
  uint8_t serial[4];
  static char teensySerial[12];
  teensySN(serial);
  sprintf(teensySerial, "%02x-%02x-%02x-%02x", serial[0], serial[1], serial[2], serial[3]);
  return teensySerial;
}

#if defined ARDUINO_TEENSY40 || defined ARDUINO_TEENSY41

void teensyMAC(uint8_t *mac)
{ // there are 2 MAC addresses each 48bit
  uint32_t m1 = HW_OCOTP_MAC1;
  uint32_t m2 = HW_OCOTP_MAC0;
  mac[0] = m1 >> 8;
  mac[1] = m1 >> 0;
  mac[2] = m2 >> 24;
  mac[3] = m2 >> 16;
  mac[4] = m2 >> 8;
  mac[5] = m2 >> 0;
}

#else

void teensyMAC(uint8_t *mac)
{
  uint8_t serial[4];
  teensySN(serial);
  mac[0] = 0x04;
  mac[1] = 0xE9;
  mac[2] = 0xE5;
  mac[3] = serial[1];
  mac[4] = serial[2];
  mac[5] = serial[3];
}

#endif

const char *teensyMAC(void)
{
  uint8_t mac[6];
  static char teensyMac[18];
  teensyMAC(mac);
  sprintf(teensyMac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return teensyMac;
}

#if defined ARDUINO_TEENSY40 || defined ARDUINO_TEENSY41
void kinetisUID(uint32_t *uid)
{
  (void)uid; // suppress warn unused parameter 'uid'
}

const char *kinetisUID(void)
{
  return "no UID";
}

#elif ARDUINO_TEENSYLC // 80bit UID Teensy LC

void kinetisUID(uint32_t *uid)
{
  uid[0] = SIM_UIDMH;
  uid[1] = SIM_UIDML;
  uid[2] = SIM_UIDL;
}

const char *kinetisUID(void)
{
  uint32_t uid[3];
  static char uidString[27];
  kinetisUID(uid);
  sprintf(uidString, "%08lx-%08lx-%08lx", uid[0], uid[1], uid[2]);
  return uidString;
}

#else // 128bit UIDTeensy 3.x

void kinetisUID(uint32_t *uid)
{
  uid[0] = SIM_UIDH;
  uid[1] = SIM_UIDMH;
  uid[2] = SIM_UIDML;
  uid[3] = SIM_UIDL;
}

const char *kinetisUID(void)
{
  uint32_t uid[4];
  static char uidString[36];
  kinetisUID(uid);
  sprintf(uidString, "%08lx-%08lx-%08lx-%08lx", uid[0], uid[1], uid[2], uid[3]);
  return uidString;
}

#endif

#if defined ARDUINO_TEENSY40 || defined ARDUINO_TEENSY41

void teensyUUID(uint8_t *uuid)
{
  (void)uuid; // suppress warn unused parameter 'uuid'
}

const char *teensyUUID(void)
{
  return "no UID";
}

void teensyUID64(uint8_t *uid64)
{
  uint32_t uid0 = HW_OCOTP_CFG0;
  uint32_t uid1 = HW_OCOTP_CFG1;
  uid64[0] = uid0 >> 24;
  uid64[1] = uid0 >> 16;
  uid64[2] = uid0 >> 8;
  uid64[3] = uid0;
  uid64[4] = uid1 >> 24;
  uid64[5] = uid1 >> 16;
  uid64[6] = uid1 >> 8;
  uid64[7] = uid1;
}

const char *teensyUID64(void)
{
  uint8_t uid64[8];
  static char uid64String[24];
  teensyUID64(uid64);
  sprintf(uid64String, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", uid64[0], uid64[1], uid64[2], uid64[3], uid64[4], uid64[5], uid64[6], uid64[7]);
  return uid64String;
}

#else

void teensyUUID(uint8_t *uuid)
{
  uint8_t mac[6];
  teensyMAC(mac);
  uuid[0] = SIM_UIDML >> 24;
  uuid[1] = SIM_UIDML >> 16;
  uuid[2] = SIM_UIDML >> 8;
  uuid[3] = SIM_UIDML;
  uuid[4] = SIM_UIDL >> 24;
  uuid[5] = SIM_UIDL >> 16;
  uuid[6] = 0x40; // marked as version v4, but this uuid is not random based !!!
  uuid[7] = SIM_UIDL >> 8;
  uuid[8] = 0x80; // variant
  uuid[9] = SIM_UIDL;
  uuid[10] = mac[0];
  uuid[11] = mac[1];
  uuid[12] = mac[2];
  uuid[13] = mac[3];
  uuid[14] = mac[4];
  uuid[15] = mac[5];
}

const char *teensyUUID(void)
{
  uint8_t uuid[16];
  static char uuidString[37];
  teensyUUID(uuid);
  sprintf(uuidString, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
  return uuidString;
}

#endif

void teensyBoardVersion(void)
{
  const char *boardVersion =
#if defined(ARDUINO_TEENSY41)
      "Teensy 4.1";
#elif defined(ARDUINO_TEENSY40)
      "Teensy 4.0";
#elif defined(ARDUINO_TEENSYLC)
      "Teensy LC";
#elif defined(ARDUINO_TEENSY36)
      "Teensy 3.6";
#elif defined(ARDUINO_TEENSY35)
      "Teensy 3.5";
#elif defined(ARDUINO_TEENSY32)
      "Teensy 3.2";
#elif defined(ARDUINO_TEENSY31)
      "Teensy 3.1";
#elif defined(ARDUINO_TEENSY30)
      "Teensy 3.0";
#else
      "Unknown Teensy";
#endif

  return boardVersion;
}
