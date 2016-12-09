void SetProtocol_Command() 
{
  digitalWrite(IRQPin, LOW);      
  delayMicroseconds(100);         
  digitalWrite(IRQPin, HIGH);  
  delay(10);
  if (debug) Serial.println(" =>>> sp1");
  digitalWrite(PIN_SPI_SS, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x02);  // Set protocol command
  SPI.transfer(0x02);  // length of data to follow
  SPI.transfer(0x01);  // code for ISO/IEC 15693
  SPI.transfer(0x0D);  // Wait for SOF, 10% modulation, append CRC
  digitalWrite(PIN_SPI_SS, HIGH);
  delay(1);
  if (debug) Serial.println(" =>>> sp2");
  digitalWrite(PIN_SPI_SS, LOW);
  while(RXBuffer[0] != 8)
    {
      RXBuffer[0] = SPI.transfer(0x03);  // Write 3 until
      RXBuffer[0] = RXBuffer[0] & 0x08;  // bit 3 is set
    }
  digitalWrite(PIN_SPI_SS, HIGH);
  delay(1);
  digitalWrite(PIN_SPI_SS, LOW);
  if (debug) Serial.println(" =>>> sp3");
  SPI.transfer(0x02);   // SPI control byte for read         
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  digitalWrite(PIN_SPI_SS, HIGH);
  if (debug) Serial.println(" =>>> sp4");
  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 0))  // is response code good?
    {
      if (debugNFC) Serial.print("Command OK => ");
      NFCReady = 1; // NFC is ready
    }
    else
    {
      if (debugNFC) Serial.print("Command FAIL => ");
      NFCReady = 0; // NFC not ready
    }
}

void Inventory_Command() 
{

  digitalWrite(PIN_SPI_SS, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x04);  // Send Receive CR95HF command
  SPI.transfer(0x03);  // length of data that follows is 0
  SPI.transfer(0x26);  // request Flags byte
  SPI.transfer(0x01);  // Inventory Command for ISO/IEC 15693
  SPI.transfer(0x00);  // mask length for inventory command
  digitalWrite(PIN_SPI_SS, HIGH);
  delay(1);
 
  digitalWrite(PIN_SPI_SS, LOW);
  while(RXBuffer[0] != 8)
    {
    RXBuffer[0] = SPI.transfer(0x03);  // Write 3 until
    RXBuffer[0] = RXBuffer[0] & 0x08;  // bit 3 is set
    }
  digitalWrite(PIN_SPI_SS, HIGH);
  delay(1);

  digitalWrite(PIN_SPI_SS, LOW);
  SPI.transfer(0x02);   // SPI control byte for read         
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  for (byte i=0;i<RXBuffer[1];i++)  RXBuffer[i+2]=SPI.transfer(0);  // data
  digitalWrite(PIN_SPI_SS, HIGH);
  delay(1);

  if (RXBuffer[0] == 128)  // is response code good?
    {
      if (debugNFC) Serial.println(" Sensor OK ");
      NFCReady = 2;
    }
  else
    {
      if (debugNFC) Serial.println(" Sensor OUT ");
      NFCReady = 1;
    }
 }


float Read_Memory() 
{
  byte oneBlock[8];
  String hexPointer = "";
  String trendValues = "";
  float currentGlucose;
  float shownGlucose;
  float trendGlucose;
  int glucosePointer;
  
  for ( int b = 3; b < 19; b++) 
  {
    digitalWrite(PIN_SPI_SS, LOW);
    SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
    SPI.transfer(0x04);  // Send Receive CR95HF command
    SPI.transfer(0x03);  // length of data that follows
    SPI.transfer(0x02);  // request Flags byte
    SPI.transfer(0x20);  // Read Single Block command for ISO/IEC 15693
    SPI.transfer(b);  // memory block address
    digitalWrite(PIN_SPI_SS, HIGH);
    delay(1);
    digitalWrite(PIN_SPI_SS, LOW);
    while(RXBuffer[0] != 8)
    {
      RXBuffer[0] = SPI.transfer(0x03);  // Write 3 until
      RXBuffer[0] = RXBuffer[0] & 0x08;  // bit 3 is set
    }
    digitalWrite(PIN_SPI_SS, HIGH);
    delay(1);

    digitalWrite(PIN_SPI_SS, LOW);
    SPI.transfer(0x02);   // SPI control byte for read         
    RXBuffer[0] = SPI.transfer(0);  // response code
    RXBuffer[1] = SPI.transfer(0);  // length of data
    for (byte i=0;i<RXBuffer[1];i++) RXBuffer[i+2]=SPI.transfer(0);  // data
    digitalWrite(PIN_SPI_SS, HIGH);
    delay(1);
    for (int i = 0; i < 8; i++) oneBlock[i] = RXBuffer[i+3];
    
    char str[24];
    unsigned char * pin = oneBlock;
    const char * hex = "0123456789ABCDEF";
    char * pout = str;
    for(; pin < oneBlock+8; pout+=2, pin++) 
    {
      pout[0] = hex[(*pin>>4) & 0xF];
      pout[1] = hex[ *pin     & 0xF];
    }
    pout[0] = 0;
    if (debugNFC) Serial.println(str);
    trendValues += str;  
  }   
  if (RXBuffer[0] == 128) // is response code good?
  {
    hexPointer = trendValues.substring(4,6);
    glucosePointer = strtoul(hexPointer.c_str(), NULL, 16);
    if (debugNFC) Serial.println();
    if (debugNFC) Serial.print("Glucose pointer: ");
    if (debugNFC) Serial.print(glucosePointer);
    if (debugNFC) Serial.println();
    int ii = 0;
    for (int i=8; i<=200; i+=12) 
    {
      if (glucosePointer == ii)
      {
        if (glucosePointer == 0)
        {
          String g = trendValues.substring(190,192) + trendValues.substring(188,190);
          String h = trendValues.substring(178,180) + trendValues.substring(176,178);
          currentGlucose = Glucose_Reading(strtoul(g.c_str(), NULL ,16));
          if ((FirstRun != 1) && ((lastGlucose - currentGlucose) > 50)) currentGlucose = Glucose_Reading(strtoul(h.c_str(), NULL ,16)); // invalid trend check
        }
        else if (glucosePointer == 1)
        {
          String g = trendValues.substring(i-10,i-8) + trendValues.substring(i-12,i-10);
          String h = trendValues.substring(190,192) + trendValues.substring(188,190);
          currentGlucose = Glucose_Reading(strtoul(g.c_str(), NULL ,16));
          trendGlucose = Glucose_Reading(strtoul(h.c_str(), NULL ,16));
          if ((FirstRun != 1) && ((lastGlucose - currentGlucose) > 50)) currentGlucose = Glucose_Reading(strtoul(h.c_str(), NULL ,16)); // invalid trend check
        }
        else
        {
          String g = trendValues.substring(i-10,i-8) + trendValues.substring(i-12,i-10);
          String h = trendValues.substring(i-22,i-20) + trendValues.substring(i-24,i-22);
          currentGlucose = Glucose_Reading(strtoul(g.c_str(), NULL ,16));
          trendGlucose = Glucose_Reading(strtoul(h.c_str(), NULL ,16));
          if ((FirstRun != 1) && ((lastGlucose - currentGlucose) > 50)) currentGlucose = Glucose_Reading(strtoul(h.c_str(), NULL ,16));  // invalid trend check
        }         
      }  
      ii++;
    }
    lastGlucose = currentGlucose;
    for (int i=8, j=0; i<200; i+=12,j++) 
    {
      String t = trendValues.substring(i+2,i+4) + trendValues.substring(i,i+2);
      trend[j] = Glucose_Reading(strtoul(t.c_str(), NULL ,16));
    }
    if (FirstRun == 1) lastGlucose = currentGlucose;     
    shownGlucose = trend[0];
    if ((lastGlucose < currentGlucose) && (currentGlucose < trendGlucose)) // apex from RISE -> FALL
    {
      for (int i=0; i<15; i++)                                             // taking the lowest value
      {
        if (((shownGlucose - trend[i+1]) > 50) || (trend[i+1] - shownGlucose) > 50) continue; // invalid trend check
        else if (trend[i+1] < shownGlucose) shownGlucose = trend[i+1];   
      }
    }
    else if (lastGlucose < currentGlucose) // RISE - taking the highest value
    {
      for (int i=0; i<15; i++)
      {
        if (((shownGlucose - trend[i+1]) > 50) || (trend[i+1] - shownGlucose) > 50) continue; // invalid trend check
        else if (trend[i+1] > shownGlucose) shownGlucose = trend[i+1];   
      }
    }
    else if ((lastGlucose > currentGlucose) && (currentGlucose > trendGlucose)) // apex from FALL -> RISE
    {
      for (int i=0; i<15; i++)                                             // taking the highest value
      {
        if (((shownGlucose - trend[i+1]) > 50) || (trend[i+1] - shownGlucose) > 50) continue; // invalid trend check
        else if (trend[i+1] > shownGlucose) shownGlucose = trend[i+1];
      }
    }
    else if (lastGlucose > currentGlucose) // FALLS - taking the lowest value
    {
      for (int i=0; i<15; i++)
      {
        if (((shownGlucose - trend[i+1]) > 50) || (trend[i+1] - shownGlucose) > 50) continue; // invalid trend check
        else if (trend[i+1] < shownGlucose) shownGlucose = trend[i+1];
      }
    }
    else shownGlucose = currentGlucose;  
       
    lastGlucose = currentGlucose;   
    NFCReady = 2;
    FirstRun = 0;   
    return shownGlucose;   
  }
  else
  {
    if (debugNFC) Serial.println("Read Memory Block Command FAIL");
    NFCReady = 0;
  }
}

float Glucose_Reading(unsigned int val) 
{
  int bitmask = 0x0FFF;
  return ((val & bitmask) / 8.5);
}

void NFC_sleep()
{
  digitalWrite(PIN_SPI_SS, LOW);
  SPI.transfer(0x00);
  SPI.transfer(0x07);  
  SPI.transfer(0x0E);  
  SPI.transfer(0x08);  
  SPI.transfer(0x04);  
  SPI.transfer(0x00);  
  SPI.transfer(0x04);
  SPI.transfer(0x00);  
  SPI.transfer(0x18);
  SPI.transfer(0x00); 
  SPI.transfer(0x00); 
  SPI.transfer(0x00); 
  SPI.transfer(0x00); 
  SPI.transfer(0x00); 
  SPI.transfer(0x00); 
  SPI.transfer(0x00); 
  SPI.transfer(0x00);   
  digitalWrite(PIN_SPI_SS, HIGH);
}

void NFC_wakeup()
{ 
  digitalWrite(IRQPin, HIGH);
  delay(100);                      
  digitalWrite(IRQPin, LOW);      
  delayMicroseconds(100);         
  digitalWrite(IRQPin, HIGH);     
  delay(1000);
  NFCReady = 0;
}

bool getNFC()
{    
    vcc_measure();
    delay(10);
    int proba=0;
    while (proba<9)
    {
      SetProtocol_Command(); 
      delay(5);
      Inventory_Command();  
      delay(5);
      if (NFCReady==2) 
      {
        currentBG = Read_Memory(); 
        return true;                 
      }   
      proba++;
      delay(1000);
      Serial.println(proba);
    }
    return false;
}

