/* Ldrip+ by Marek Macner (c) 2016 */

#include <RFduinoBLE.h>
#include <SPI.h>
#include <Stream.h>


#define PIN_SPI_SCK   4
#define PIN_SPI_MOSI  5
#define PIN_SPI_MISO  3
#define PIN_SPI_SS    6
#define IRQPin        2    

byte RXBuffer[24];
byte NFCReady = 0; 

byte FirstRun = 1;
byte batteryLow;
int batteryPcnt;
long batteryMv;
float lastGlucose;
float trend[16];
String xdripPacket = "";
long vcc = 0;
float lastBG=100;
float currentBG=100;
float deltaBG = 0;
int errBG = 2;

int licznik = 0;
bool debugNFC = true;
bool debugVcc = true;
bool debugBLE = true;
bool debug = true;

bool conn = false;
bool batteryOK = true;

long currentTime=0;
long lastTime = 0;
long delayTime = 1000 * 60 * 5; // 5 minut
long startTime;
long endTime;
long deltaTime;
double temperature;
bool firstrun=true;

void setup() 
{
  Serial.begin(9600);
  Serial.println("=>>> init - start ");
  pinMode(PIN_SPI_SS, OUTPUT);
  
  pinMode(IRQPin, OUTPUT);
  digitalWrite(IRQPin, HIGH); 
   
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setFrequency(1000);            // 125, 250, 500, 1000, 2000, 4000, 8000 (4000 = 4 MHz)
  delay(10);                      
  digitalWrite(IRQPin, LOW);      
  delayMicroseconds(100);         
  digitalWrite(IRQPin, HIGH);     
  RFduinoBLE.txPowerLevel = 4;
  RFduinoBLE.deviceName = "LimiTTer";
  RFduinoBLE.advertisementData = "eLeR";
  RFduinoBLE.customUUID = "c97433f0-be8f-4dc8-b6f0-5343e6100eb4";
//  RFduinoBLE.customUUID = "0000ffe0-0000-1000-8000-00805f9b34fb";
// RFduinoBLE.customUUID = "ffe1";
  RFduinoBLE.advertisementInterval = 100; 
  RFduinoBLE.begin();  
  vcc_measure(); 
  SetProtocol_Command();
  if (debug) Serial.println(" =>>> init - set prot ");
  Inventory_Command();
  if (debug) Serial.println(" =>>> init - invent ");
  NFC_sleep();
  if (debug) Serial.println(" =>>> init - end ");
  delay(1000);


  NRF_WDT->CONFIG = (WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos) | (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos); 
  NRF_WDT->CRV = 32768 * 60 * 11;                          // 11 minut
  NRF_WDT->RREN |= WDT_RREN_RR0_Msk;                      // Enable reload register 0.
  NRF_WDT->TASKS_START = 1;                               // Watchdog start
    
}


void loop() 
{ 
    startTime=millis();
    NFCReady = 0;
    if (debug) Serial.print(licznik);
    if (debug) Serial.print(" start ");     
    if (batteryOK) 
    { 
      NFC_wakeup();
      if (debug) Serial.print(" => wakeup ");
      if (batteryOK) getNFC(); 
      xdripPacket = Build_Packet(currentBG);
      NFC_sleep();  
      if (NFCReady == 2) 
      {       
        for (int i=0; i<9; i++)
        {
          if (conn) 
          {
            if (batteryOK) 
            {
              int dlg = (int) xdripPacket.length();
              RFduinoBLE.send(xdripPacket.c_str(), dlg);          // ,18
              Serial.print("BLE >>");
              Serial.print( xdripPacket.c_str());
              Serial.print("<<");            
            }
            i=15;
            continue;
          }
          delay(10000);
        }       
      }  
    }
    if (debug) Serial.println(" => end");
    licznik++;   
    endTime=millis();
    deltaTime = delayTime - (endTime-startTime) - 5000;    
    if (debug) Serial.println(endTime/1000);
    if (debug) Serial.println(deltaTime/1000);
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    RFduino_ULPDelay(deltaTime); 
    if (debug) Serial.println(millis()/1000);
    delay(5000);
}


