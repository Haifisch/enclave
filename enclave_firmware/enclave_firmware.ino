// Arduino Libraries
#include <MapleCoOS.h>
#include <curve25519-donna.h>
#include <stdint.h>
#include <EEPROM.h>

// Local libraries
#include "SerialCommand.h"
#include "eeprom_func.h"

extern "C"{
#include "aes.h"
#include "tests.h"
};

#define DEBUG_BUILD 1
#define pinLED PC13
#define resetPin PC15

OS_STK    btle_stk[TASK_STK_SIZE];   
OS_STK    cnsl_stk[TASK_STK_SIZE];   
OS_STK    aesinit_stk[TASK_STK_SIZE];   
OS_STK    eepprom_stk[TASK_STK_SIZE];   

OS_TID    TaskMId;
SerialCommand   sCmd; 

uint16 promStatus;
uint16 promSerialData = 0x414141;
uint16 promSerialAddress = 8000000;


void initEEPROM(void *pdata) {
  Serial.println("Initializing EEPROM");
  promStatus = EEPROM.init();
  if (promStatus != 0)
  {
    panic("EEPROM.init() failed!", "initEEPROM");
  }
  EEPROM.PageBase0 = 0x801F000;
  EEPROM.PageBase1 = 0x801F800;
  EEPROM.PageSize  = 0x800;
  Serial.println("EEPROM Init done.");
  CoExitTask();
}

void initBTStack(void *pdata) {
  Serial.println("Initializing BT stack...");
  // init BTLE stack
  Serial.println("BTLE Init done.");
  
  CoExitTask();
}

void initAESStack(void *pdata) {
  Serial.println("Initializing AES stack...");
  Serial.println("Testing AES Enc/Dec...");

  doAESTest();

  Serial.println("AES Init done.");
  
  CoExitTask();
}

void initConsole(void *pdata) {
  Serial.println("Initializing console...");
  if (DEBUG_BUILD) { // put sensitive commands here, only internal debug builds may have privilege to this.
    sCmd.addCommand("iamnotapplediagsdammit",    iamnotapplediags); 
    sCmd.addCommand("aestest",                   doAESTest);
    sCmd.addCommand("wserial",                   writeSerial);
    sCmd.addCommand("rserial",                   readSerial);
  }
  sCmd.addCommand("reset",    resetDevice); 
  sCmd.addCommand("version",  printVersionHeader); 
  sCmd.setDefaultHandler(unrecognized);  
  
  Serial.println(":-)");
  
  while(1) {
    sCmd.readSerial();
  }
  CoExitTask();
}

void writeSerial() {
  char *arg;
  arg = sCmd.next();

  if (!arg)
  {
    Serial.println("Missing serial in command argument.");
  }  else {
    Serial.print("wserial 0x");
    Serial.print(promSerialAddress, HEX);
    Serial.print(", ");
    Serial.println(arg);
    if (!eeprom_write_string(0x8000000, arg)) {
      Serial.println("Could not write serial to EEPROM!");
    } else {
      Serial.println("Wrote serial to EEPROM!");
    }
  }
}
void readSerial() {
  uint16 ReadSerial;
  Serial.print("rserial 0x");
  Serial.print(promSerialAddress, HEX);
  Serial.write(10); // newline
  for (int i = 0; i < 24; ++i)
  {
      promStatus = EEPROM.read(0x8000000+i, &ReadSerial);
      Serial.write((int)ReadSerial);
  }
  Serial.write(10); // newline
}

void resetDevice() {
  Serial.println("Resetting device...");
  pinMode(resetPin, OUTPUT);
}

void iamnotapplediags() {
  Serial.println("we r of #purpl tools");  
}

void doAESTest() {
  if (!do_cbc_tests())
  {
    panic("AES CBC Enc/Dec test failed, hanging...", "doAESTest");
    while(1); // hang if tests don't pass. 
  }
  Serial.println("AES CBC Enc/Dec test passed.");
}

void panic(String reason, String caller) {
  Serial.println("Panicing!!!");
  Serial.print("Panic reason; ");
  Serial.println(reason);
  Serial.print("Panic caller; ");
  Serial.println(caller);

  if (DEBUG_BUILD)
  {
    Serial.println("DEBUG tag enabled, no reset.");
    Serial.println("We're hanging here...");
    while(1);
  } else  {
    resetDevice();
  }
  while (1); // we shouldn't get here
}

void unrecognized(const char *command) {
  Serial.print("Unrecognized command \"");
  Serial.print(command);
  Serial.println("\"");
}

void printVersionHeader() {
  Serial.println("=====================================");
  Serial.println("::Enclave Firmware");
  Serial.println("::Copyright ST Security LLC 2016");
  Serial.println("::BUILD_DATE: " __DATE__);
  if (DEBUG_BUILD) {
    Serial.println("::BUILD_TAG: DEBUG");
  } else {  
    Serial.println("::BUILD_TAG: PRODUCTION");
  }
  Serial.println("=====================================");
}

void setup() {
  Serial.begin(115200);
  Serial.flush();
  pinMode(pinLED, OUTPUT);
  Serial.println("Initializing firmware...");
  printVersionHeader();

  CoInitOS();
  TaskMId = CoCreateTask(initBTStack,0,1,&btle_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  if (TaskMId == E_CREATE_FAIL) {
    panic("Failed to create BT task", "setup");
    while (1); // hang
  }

  TaskMId = CoCreateTask(initAESStack,0,2,&aesinit_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  if (TaskMId == E_CREATE_FAIL) {
    panic("Failed to create AES task", "setup");
    while (1); // hang
  }

  TaskMId = CoCreateTask(initEEPROM,0,3,&eepprom_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  if (TaskMId == E_CREATE_FAIL) {
    panic("Failed to create EEPROM init task", "setup");
    while (1); // hang
  }

  TaskMId = CoCreateTask(initConsole,0,4,&cnsl_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  if (TaskMId == E_CREATE_FAIL) {
    panic("Failed to create console task", "setup");
    while (1); // hang
  }

  CoStartOS();
  while(1);
}

void loop() {
  // not reached with CoOs
}

