void vcc_measure()
{
    analogReference(VBG);           
    analogSelection(VDD_1_3_PS); 
    delay(10); 
    long sensorValue = analogRead(1);    
    batteryMv = sensorValue * (360 / 1023.0) * 10;  
    batteryPcnt = map(batteryMv, 3000, 3600, 0, 100);
    if (batteryPcnt > 99) batteryPcnt = 99; 
    if (batteryPcnt <1) batteryPcnt = 10;   
    if (batteryPcnt < 3) batteryOK = false;
    else batteryOK = true;
    temperature = (double) RFduino_temperature(CELSIUS);   
    if (debugVcc)
    {
      Serial.print("T=");
      Serial.print(temperature);
      Serial.print(" Vcc=");
      Serial.print(batteryMv);
      Serial.print(" mV ");    
      Serial.print(batteryPcnt);
      Serial.print(" %");
      if (batteryOK) Serial.println(" OK");
      else Serial.println(" LOW");
    }
    
}
