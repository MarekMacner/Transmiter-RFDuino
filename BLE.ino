String Build_Packet(float glucose) 
{
  String packet = ""; 
  if (glucose==0) glucose=100.0;
  unsigned long raw = glucose*1000; // raw_value 
  packet = String(raw);
  packet += ' ';
  packet += String(batteryMv);
  packet += ' ';
  packet += String(batteryPcnt);
  packet += ' ';  
  packet += String((int)(temperature*10.0));
  if (debugBLE)
  {
        Serial.println();
        Serial.print("Glucose level: ");
        Serial.print(glucose);
        Serial.println();
        Serial.print("15 minutes-trend: ");
        Serial.println();
        for (int i=0; i<16; i++)
        {
          Serial.print(trend[i]);
          Serial.println();
        }
        Serial.print("Battery level: ");
        Serial.print(batteryPcnt);
        Serial.print("%");
        Serial.println();
        Serial.print("Battery mVolts: ");
        Serial.print(batteryMv);
        Serial.print("mV");
        Serial.println();
  }      
  return packet;
}

void RFduinoBLE_onConnect()
{
  conn = true;
  if (debugBLE) Serial.println("BLE Connected ");            
}

void RFduinoBLE_onDisconnect()
{
  conn = false;
  if (debugBLE) Serial.println("BLE Disconnected ");
}

