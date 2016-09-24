#include <MapleCoOS.h>
#include <Base64.h>
#include <curve25519-donna.h>
#include "SerialCommand.h"

#define DEBUG_BUILD 1
#define pinLED PC13

OS_STK     taskA_stk[TASK_STK_SIZE];   
OS_STK     btle_stk[TASK_STK_SIZE];   
OS_STK     cnsl_stk[TASK_STK_SIZE];   
OS_STK     aesinit_stk[TASK_STK_SIZE];   

OS_TID TaskMId;
SerialCommand sCmd; 

void initBTStack(void *pdata) {
  Serial.println("Initializing BT stack...");
  // init BTLE stack
  Serial.println("BTLE Init done.");
  
  CoExitTask();
}

void initAESStack(void *pdata) {
  Serial.println("Initializing AES stack...");
  // init BTLE stack
  Serial.println("AES Init done.");
  
  CoExitTask();
}

void initConsole(void *pdata) {
  Serial.println("Initializing console...");
  if (DEBUG_BUILD) { // put sensitive commands here, only internal debug builds may have privilege to this.
     sCmd.addCommand("iamnotapplediagsdammit",    iamnotapplediags); 
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

void resetDevice() {
  Serial.println("Resetting device...");
  
}

void iamnotapplediags() {
  Serial.println("we r of #purpl tools");  
}

void unrecognized(const char *command) {
  Serial.print("Unrecognized command \"");
  Serial.print(command);
  Serial.println("\"");
}

void printVersionHeader() {
  Serial.println("=====================================");
  Serial.println("::Enclave Firmware");
  Serial.println("::Copyright Enclave Security LLC 2016");
  Serial.println("::BUILD_DATE: " __DATE__);
  Serial.println("::BUILD_TAG: DEBUG");
  Serial.println("=====================================");
}

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  Serial.println("Initializing firmware...");
  printVersionHeader();
  
  CoInitOS();
  TaskMId = CoCreateTask(initBTStack,0,1,&btle_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  if (TaskMId == E_CREATE_FAIL) {
    Serial.println("Failed to create BT stack task");
    while (1); // hang
  }

  TaskMId = CoCreateTask(initAESStack,0,2,&aesinit_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  if (TaskMId == E_CREATE_FAIL) {
    Serial.println("Failed to create BT stack task");
    while (1); // hang
  }
  
  TaskMId = CoCreateTask(initConsole,0,3,&cnsl_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  if (TaskMId == E_CREATE_FAIL) {
    Serial.println("Failed to create console stack task");
    while (1); // hang
  }
  CoStartOS();
  while(1);
}

void loop() {
  // not reached with CoOs
}

