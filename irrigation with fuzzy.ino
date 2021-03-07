#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "";
char ssid[] = "";
char pass[] = "";

const int relay1 = D5;
const int relay1 = D6;
int relayON = LOW;
int relayOFF = HIGH;

int Humidity;
int temperature;
float output;
unsigned long timeNow;
unsigned long timeLast;

Fuzzy* fuzzy = new Fuzzy();

// Creating fuzzification of Temperature  
FuzzySet* dingin = new FuzzySet(0, 0, 20, 30);
FuzzySet* normal = new FuzzySet(25, 30, 30, 35); 
FuzzySet* panas = new FuzzySet(30, 35, 40, 40); 

// creating fuzzification of Humidity

FuzzySet* kering = new FuzzySet(0, 0, 237, 380);
FuzzySet* lembab = new FuzzySet(237, 380, 380, 712);
FuzzySet* basah = new FuzzySet(570, 712, 950, 950);

// Fuzzy output watering duration 

FuzzySet* cepat = new FuzzySet(0, 3, 3, 5);
FuzzySet* lumayan = new FuzzySet(3, 5, 5, 7);
FuzzySet* lama = new FuzzySet(5, 7, 7, 10);


void setup() 
{
  //Blynk debug console
  Blynk.begin(auth, ssid, pass); 
  timeNow = millis();
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  //AFMS.begin();
  sensor.begin();
  Serial.begin(115200);
//  dht.begin();
//  oledStart();
  digitalWrite (soilMoisterVcc, LOW);
  delay(500);

  FuzzyInput* suhu = new FuzzyInput(1);

    suhu->addFuzzySet(dingin);
    suhu->addFuzzySet(normal); 
    suhu->addFuzzySet(panas);

    fuzzy->addFuzzyInput(suhu); 


    FuzzyInput* kelembapan = new FuzzyInput(2);

    kelembapan->addFuzzySet(kering);
    kelembapan->addFuzzySet(lembab);
    kelembapan->addFuzzySet(basah);

    fuzzy->addFuzzyInput(kelembapan);

    FuzzyOutput* WaktuPengairan = new FuzzyOutput(1);

    WaktuPengairan->addFuzzySet(cepat); 
    WaktuPengairan->addFuzzySet(lumayan); 
    WaktuPengairan->addFuzzySet(lama);
  
    fuzzy->addFuzzyOutput(WaktuPengairan);


    //fuzzy rule 
    
    FuzzyRuleConsequent* thenCepat = new FuzzyRuleConsequent();
    thenCepat->addOutput(cepat);
    FuzzyRuleConsequent* thenLumayan = new FuzzyRuleConsequent(); 
    thenLumayan->addOutput(lumayan);  
    FuzzyRuleConsequent* thenLama = new FuzzyRuleConsequent(); 
    thenLama->addOutput(lama);

    FuzzyRuleAntecedent* ifDinginDanBasah = new FuzzyRuleAntecedent();
    ifDinginDanBasah->joinWithAND(dingin, basah); 
    
    FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ifDinginDanBasah, thenCepat);
    fuzzy->addFuzzyRule(fuzzyRule01); 


    FuzzyRuleAntecedent* ifDinginLembab = new FuzzyRuleAntecedent(); 
    ifDinginLembab->joinWithAND(dingin, lembab);
      
    FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ifDinginLembab, thenCepat);
    fuzzy->addFuzzyRule(fuzzyRule02);

    FuzzyRuleAntecedent* ifDinginKering = new FuzzyRuleAntecedent(); 
    ifDinginKering->joinWithAND(dingin, kering);

  
    FuzzyRule* fuzzyRule03 = new FuzzyRule(3, ifDinginKering, thenLumayan);
    fuzzy->addFuzzyRule(fuzzyRule03); 

    FuzzyRuleAntecedent* ifNormalBasah = new FuzzyRuleAntecedent(); 
    ifNormalBasah->joinWithAND(normal, basah);

    FuzzyRule* fuzzyRule04 = new FuzzyRule(4, ifNormalBasah, thenCepat);
    fuzzy->addFuzzyRule(fuzzyRule04); 

    FuzzyRuleAntecedent* ifNormalLembab = new FuzzyRuleAntecedent(); 
    ifNormalLembab->joinWithAND(normal, lembab); 

    FuzzyRule* fuzzyRule05 = new FuzzyRule(5, ifNormalLembab, thenLumayan); 
    fuzzy->addFuzzyRule(fuzzyRule05); 

    FuzzyRuleAntecedent* ifNormalKering = new FuzzyRuleAntecedent(); 
    ifNormalKering->joinWithAND(normal, kering); 
 
    FuzzyRule* fuzzyRule06 = new FuzzyRule(6, ifNormalKering, thenLama); 
    fuzzy->addFuzzyRule(fuzzyRule06); 

    FuzzyRuleAntecedent* ifPanasBasah = new FuzzyRuleAntecedent(); 
    ifPanasBasah->joinWithAND(panas, basah); 

    FuzzyRule* fuzzyRule07 = new FuzzyRule(7, ifPanasBasah, thenLumayan); 
    fuzzy->addFuzzyRule(fuzzyRule07); 

    FuzzyRuleAntecedent* ifPanasLembab = new FuzzyRuleAntecedent(); 
    ifPanasLembab->joinWithAND(panas, lembab); 
      
    FuzzyRule* fuzzyRule08 = new FuzzyRule(8, ifPanasLembab, thenLumayan); 
    fuzzy->addFuzzyRule(fuzzyRule08);
  
    FuzzyRuleAntecedent* ifPanasKering = new FuzzyRuleAntecedent(); 
    ifPanasKering->joinWithAND(panas, kering); 

    FuzzyRule* fuzzyRule09 = new FuzzyRule(9, ifPanasKering, thenLama); 
    fuzzy->addFuzzyRule(fuzzyRule09);

}

void loop() 
{
  Blynk.run();
  timer.run();
//  getDhtData();
  getSoilMoisterData();
  getTempData();
  displayData();
  delay(2000); // delay for getting DHT22 data
 // Humidity = soilMoister.getCapacitance();

  
  fuzzy->setInput(1, temperature);
  fuzzy->setInput(2, soilMoister);

  fuzzy->fuzzify();
  output = fuzzy->defuzzify(1);

  //relayInput->run(FORWARD);
  if (sensorValue < 40 ){
  digitalWrite(relay1, relayON);
  Serial.print("pompa nyala selama : ");
  Serial.print(output);
  Serial.println(" ");
  delay(output*1000);
  digitalWrite(relay1, relayOFF);
  Serial.println("pump mati");
 // Blynk.virtualWrite(V5, relayInput); //v5
  }
  if (sensorValue > 40 ){
  
  //relayInput->run(RELEASE);
  digitalWrite(relay1, relayOFF);
  Serial.println("pump mati");
  //Blynk.virtualWrite(V5, relayInput); //v5
  }
  timeLast = timeNow;
}


// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_BUTTON_1) {
  relay1State = param.asInt();
  digitalWrite(relay2, relay1State);
   Serial.println("pump diaktifkan manual");
    Blynk.notify("pompa nyala {relay2} manual");
}

void checkPhysicalButton()
{
  if (digitalRead(PUSH_BUTTON_1) == LOW) {
    // pushButton1State is used to avoid sequential toggles
    if (pushButton1State != LOW) {

      // Toggle Relay state
      relay1State = !relay1State;
      digitalWrite(relay2, relay1State);
      Serial.println("pump diaktifkan manual");
      // Update Button Widget
      Blynk.virtualWrite(VPIN_BUTTON_1, relay1State);
      Serial.println("pump diaktifkan manual");
      Blynk.notify("pompa nyala {relay2} manual");
    }
    pushButton1State = LOW;
  } else {
    pushButton1State = HIGH;
  }
}
/***************************************************
 * Start OLED
 **************************************************/
void oledStart(void)
{
  Wire.begin();  
}

void getTempData(void)
{
  //temprature
  sensor.requestTemperatures();                // Send the command to get temperatures  
  temperature = sensor.getTempCByIndex(0);
  delay(500);
  Blynk.virtualWrite(V11, temperature); //v11
  //
}

/***************************************************
 * Get DHT data
 **************************************************/
}

/***************************************************
 * Display data at Serial Monitora & OLED Display
 **************************************************/
void displayData(void)
{
  Serial.println("==========================");
  //soil moister
  Serial.print("Kelembaban tanah :  ");
  Serial.print(sensorValue);
  Serial.println("%");
 
 
  //temp
  Serial.print("Temperatur:  ");
  Serial.print(sensor.getTempCByIndex(0));   // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  Serial.println(" ºC");
  Serial.println("==========================");
  delay(1000);
}

