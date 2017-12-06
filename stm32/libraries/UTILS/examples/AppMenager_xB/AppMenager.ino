/* AppMenager.ino cmdline app menage demo
   ---------------------------------------------------------------------------------------
   cmd usage:
   type help or h or ? for Display list of commands.
   ---------------------------------------------------------------------------------------
   for F103C8  by huaweiwx@sina.com 2017.10
   ---------------------------------------------------------------------------------------
*/

#include <cmdline.h>
 char cmdline[128];
int ptr;

#include <utils.h>
#include <LED.h>

void setup() {
  Serial.begin(115200);
//  SPIFLASH.begin(); //extern SPIFLASH SpiFlash; need while usb un connected
  Led.Init();

  //for  use SerialUSB if selected from menu
#if (MENU_USB_SERIAL || MENU_USB_IAD)
  while (!Serial.available()) {
    Led.flash(10, 990, 1);
  }
#endif
  ptr = 0;
  Serial.print(F(">"));
//  fat.begin();
}

boolean stringComplete = false;  // whether the string is complete
void loop() {
  int nStatus;
  if (stringComplete) {
    if (cmdline[0] > ' ') {
      nStatus = CmdLineProcess(cmdline);
      if (nStatus == CMDLINE_BAD_CMD)
      {
        Serial.println(F("Bad command!"));
      }
      else if (nStatus == CMDLINE_TOO_MANY_ARGS)
      {
        Serial.println(F("Too many arguments for command processor!"));
      }
      else if (nStatus != 0)
      {
        Serial.print(F("Command returned error code"));
        Serial.println(nStatus);
      }
    }
    ptr = 0;
    stringComplete = false;
    Serial.print(F("\r\n>"));
  }
#if defined(STM32GENERIC)
  serialEvent();
#endif
  Led.flash(10, 90, 1);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    cmdline[ptr++] = inChar;
    cmdline[ptr] = '\0';
    if ((inChar == '\n') || (inChar == '\r')) {
      stringComplete = true;
    }
  }
}

//*****************************************************************************
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//*****************************************************************************

int Cmd_help(int argc, char *argv[])
{
  volatile uint32_t useradr;
  tCmdLineEntry *pEntry;
  pEntry = &g_sCmdTable[0];
  Serial.println(F("\r\nAvailable commands:\r\n------------------\n"));
  while (pEntry->pcCmd)
  {
    Serial.print( pEntry->pcCmd);
    Serial.println(pEntry->pcHelp);
    pEntry++;
  }
    Serial << "\ncodes on slot:\n";

  for (volatile uint32_t i = 0; i < CODESEG_NUM; i++) {
    if (i > 0)
      useradr = FLASH_BASE + codeSegAddr[i - 1] * 1024;
    else {
      useradr = USER_CODE_RAM;
    }
    Serial << i << "# addr:0x";
    if(useradr < 0x10000000) Serial << "0";
    Serial << _HEX(useradr);
    if (UTIL_checkUserCode(useradr))
      Serial << " ok\n";
    else
      Serial << " is unavailed!\n";
  }
  return (0);
}

int Cmd_go(int argc, char *argv[])//exp:go 1/2/...
{
  if (!(argc == 2)) return 0;
  volatile uint32_t useradr;
  Serial << "go " << argv[1] << "\n";
  uint32_t i = atoi(argv[1]);
  //  if ((i < CODESEG_NUM) && (i >  0)) {
  if (i < CODESEG_NUM) {
    if (i == 0) {
      if (UTIL_checkUserCode(SERIAL_LOAD_RAM))
        useradr = SERIAL_LOAD_RAM;
      else
        useradr =  USER_CODE_RAM;

    } else {
      useradr = FLASH_BASE + codeSegAddr[i - 1] * 1024;
    }

    if (UTIL_checkUserCode(useradr))
    {
      Serial << "check ok and goto " << _HEX(useradr) << ".....\n" << _endl;
      USBDeviceFS.end();
      delay(100); /*wait  serial complated */

      //    Serial1.end();  /*if open close it*/
      //    Serial2.end();  /*if open close it*/
      UTIL_jumpToUser(useradr);
    } else {
      Serial << "slot: " << argv[1] << " is unavailed!\n";
    }
  }
  return 0;
}

tCmdLineEntry g_sCmdTable[] =
{
  { "?",      Cmd_help,   "    : Display list of commands & get mcu info..."} ,

  //go flash addr
  { "go",     Cmd_go,      "   : goto n slot addr running(slot#)"},

  {  0, 0, 0 }
};

