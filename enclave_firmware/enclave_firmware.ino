#include <Base64.h>
#include <curve25519-donna.h>
#include <MapleCoOS116.h>

#define DEBUG_BUILD 1
#define pinLED PC13

OS_STK     taskA_stk[TASK_STK_SIZE];   
OS_STK     btle_stk[TASK_STK_SIZE];   
OS_TID TaskMId;

void  initEnclaveOS(void* pdata) {
  char *BUILD_TAG = "::BUILD_TAG: PRODUCTION";
  if (DEBUG_BUILD) {
    BUILD_TAG = "::BUILD_TAG: DEBUG";
  }
  
  Serial.println("=====================================");
  Serial.println("::Enclave Firmware");
  Serial.println("::Copyright Enclave Security LLC 2016");
  Serial.println("::BUILD_DATE: " __DATE__);
  //Serial.println(BUILD_TAG);
  Serial.println("=====================================");
  Serial.println("Testing crypto...");
  
  static const uint8_t basepoint[32] = {9};
  uint8_t mypublic[32];
  uint8_t mysecret[32];
  for (int i = 0; i < sizeof(mysecret); i++){
    mysecret[i] = rand();
  }
  curve25519_donna(mypublic, mysecret, basepoint);

  int inputLen = sizeof(mypublic);
  
  int encodedLen = base64_enc_len(inputLen);
  char encoded[encodedLen];
  
  base64_encode(encoded, (char*)mypublic, inputLen); 
  Serial.println(encoded);
  
  TaskMId = CoCreateTask(initBTStack,0,1,&btle_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  if (TaskMId == E_CREATE_FAIL) {
    Serial.println("Failed to create BT stack task");
    while (1); // hang
  }
  CoExitTask();
}

void initBTStack(void *pdata) {
  Serial.println("Initializing BT stack...");
  // init BTLE stack
  CoExitTask();
}

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  Serial.println("Initializing firmware...");
  CoInitOS();
  CoCreateTask(initEnclaveOS,0,0,&taskA_stk[TASK_STK_SIZE-1],TASK_STK_SIZE);
  CoStartOS();
  while(1);
}

void loop() {
  // not reached with CoOs
}

