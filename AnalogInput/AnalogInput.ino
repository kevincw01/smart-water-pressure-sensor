int sensorPin = A0;    // select the input pin for the sensor
int sensorValue = 0;  // variable to store the value coming from the sensor
float voltage = 0;
int psi = 0;
const int LO_PSI = 0;
const int HI_PSI = 200;
const float LO_VOLTS = 0.5;
const float HI_VOLTS = 4.5;
const float CAL = 1.10;
int readCount = 0;

void setup() {
  Serial.begin(115200);

  Serial1.begin(9600);
}

void writeString(String stringData) { // Used to serially push out a String with Serial.write()

  for (int i = 0; i < stringData.length(); i++)
  {
    Serial1.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }
}

uint16_t calcCRC(String stringData)
{
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
    
  for (int i = 0; i < stringData.length(); i++)
  {
    sum1 = (sum1 + stringData[i]) % 255;
    sum2 = (sum2 + sum1) % 255;
  }
   return (sum2 << 8) | sum1;
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  if(readCount < 2) {
    //Serial.println("discarding first 2 reads upon startup");
    readCount++;
  }else {
    sensorValue *= CAL;
    voltage = sensorValue * (4.5/1023.0);
    psi = (HI_PSI - LO_PSI) * (voltage - LO_VOLTS) / 4 + LO_PSI;

    Serial.print("read pressure as: ");
    Serial.println(psi);
  
    Serial1.flush();
    //Serial1.write(psi); 
    String s = "PSI,";
    s += psi;
  
    uint16_t crc = calcCRC(s);
    Serial.print("calculated crc=");
    Serial.println(crc); 
    s+=",";
    s+=crc;
  
    String f = "<";
    f +=s;
    f +=">";
    
    Serial.print("sending this string: ");
    Serial.println(f);
    writeString(f);
  }
    
  delay(5000);
}
