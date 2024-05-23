#include <dht.h>
#include <Fuzzy.h>

dht DHT;

#define DHT11_PIN     A2


const int MoistInput = A0; // A0, moist sensor input
const int Lightsensor = A1; // A1
const int waterLevel = 7; //port 7 digital input

//switch for all mosfets
const int TempHumidSwitch = 3; // Digital pwm port 2, IN on pump
const int PumpSwitch = 6; // Digital pwm port 2, IN on pump

const int LightSwitch = 8; // Digital pwm port 2, IN on pump

// Fuzzy Logic
Fuzzy* fuzzy = new Fuzzy();

void setup() {

  Serial.begin(9600);

  pinMode(PumpSwitch, OUTPUT); // Set the pin as an output
  pinMode(TempHumidSwitch, OUTPUT); // Set the pin as an output
  pinMode(LightSwitch, OUTPUT); // Set the pin as an output

  pinMode(Lightsensor, INPUT); // Set the pin as an output
  pinMode(waterLevel, INPUT);

  fuzzyLogic();
}



void TurnOnPump(int time)
{
  // Activate both outputs simultaneously
  digitalWrite(PumpSwitch, HIGH);
  
  for (int i = 0; i < time; i++)
  {
    delay(1000); // Wait for the specified time
  }


  digitalWrite(PumpSwitch, LOW);
}

float MoistSensor()
{

  float MoistureSensor = analogRead(MoistInput);
  return MoistureSensor;
}

float TempSensor()
{
  digitalWrite(TempHumidSwitch, HIGH);
  delay(2000);
  int chk = DHT.read11(DHT11_PIN);
  float temperatureValue = (float)DHT.temperature;
  digitalWrite(TempHumidSwitch, LOW);
  return temperatureValue;
}

float HumidSensor()
{
  digitalWrite(TempHumidSwitch, HIGH);
  delay(2000);
  int chk = DHT.read11(DHT11_PIN);
  float humidityValue = (float)DHT.humidity;
  digitalWrite(TempHumidSwitch, LOW);
  return humidityValue;
}

float LightSensor()
{ 
  //Läs ljussensor
  digitalWrite(LightSwitch, HIGH);
  delay(3000);
  int Lightval = analogRead(Lightsensor);
  float voltage= Lightval * (5.0 / 1023.0);
  digitalWrite(LightSwitch, LOW);
  return voltage;
  //Serial.println(Lightval);
  
}

void refill()
{
  int portValue = digitalRead(waterLevel); // Läs värdet av porten
  
  if (portValue == LOW) {
    //Serial.println("Needs to refill water!");
  }

  //Sätt igång en led eller något
  

}

void fuzzyLogic()
{
  // Define fuzzy inputs
  FuzzyInput* temperature = new FuzzyInput(1);
  FuzzySet* tempCold = new FuzzySet(-10, 0, 10, 13);
  FuzzySet* tempMedium = new FuzzySet(13, 20, 20, 30);
  FuzzySet* tempHot = new FuzzySet(20, 30, 40, 50);
  temperature->addFuzzySet(tempCold);
  temperature->addFuzzySet(tempMedium);
  temperature->addFuzzySet(tempHot);
  fuzzy->addFuzzyInput(temperature);

  FuzzyInput* humidity = new FuzzyInput(2);
  FuzzySet* humidLow = new FuzzySet(0, 10, 20, 30);
  FuzzySet* humidMedium = new FuzzySet(20, 40, 50, 60);
  FuzzySet* humidHigh = new FuzzySet(50, 60, 70, 80);
  humidity->addFuzzySet(humidLow);
  humidity->addFuzzySet(humidMedium);
  humidity->addFuzzySet(humidHigh);
  fuzzy->addFuzzyInput(humidity);

  FuzzyInput* moisture = new FuzzyInput(3);
  FuzzySet* moistWet = new FuzzySet(0, 220, 270, 300);
  FuzzySet* moistMedium = new FuzzySet(270, 300, 320, 330);
  FuzzySet* moistDry = new FuzzySet(320, 350, 390, 400);
  moisture->addFuzzySet(moistDry);
  moisture->addFuzzySet(moistMedium);
  moisture->addFuzzySet(moistWet);
  fuzzy->addFuzzyInput(moisture);

  FuzzyInput* light = new FuzzyInput(4);
  FuzzySet* lightLow = new FuzzySet(0, 1, 2, 3);
  FuzzySet* lightMedium = new FuzzySet(2, 3, 4, 5);
  FuzzySet* lightHigh = new FuzzySet(4, 5, 6, 7);
  light->addFuzzySet(lightLow);
  light->addFuzzySet(lightMedium);
  light->addFuzzySet(lightHigh);
  fuzzy->addFuzzyInput(light);

  // Define fuzzy output
  FuzzyOutput* watering = new FuzzyOutput(1);
  int radie = 11; //cm
  double VolumeWater = radie * radie * 2.5 * 3.14; //Volym per vecka
  double wateringMax = VolumeWater/72; //volym per 20 minut

  // Define fuzzy sets for watering levels
  FuzzySet* waterNone = new FuzzySet(0, 0, wateringMax*0.5, wateringMax*0.1);
  FuzzySet* waterLow = new FuzzySet(wateringMax*0.5, wateringMax*0.15, wateringMax*0.2, wateringMax*0.5);
  FuzzySet* waterMedium = new FuzzySet(wateringMax*0.25, wateringMax*0.3, wateringMax*0.4, wateringMax*0.6);
  FuzzySet* waterHigh = new FuzzySet(wateringMax*0.4, wateringMax*0.7, wateringMax*0.9, wateringMax*1.0);

  // Add fuzzy sets to the output
  watering->addFuzzySet(waterNone); //none
  watering->addFuzzySet(waterLow); //little
  watering->addFuzzySet(waterMedium);//medium
  watering->addFuzzySet(waterHigh); //much
  fuzzy->addFuzzyOutput(watering);

  FuzzyRuleConsequent *thenWaterHigh = new FuzzyRuleConsequent(); 
  thenWaterHigh->addOutput(waterHigh); // Corrected this line
  FuzzyRuleConsequent *thenWaterMedium = new FuzzyRuleConsequent(); 
  thenWaterHigh->addOutput(waterMedium); // Corrected this line
  FuzzyRuleConsequent *thenWaterLow = new FuzzyRuleConsequent(); 
  thenWaterHigh->addOutput(waterLow); // Corrected this line
  FuzzyRuleConsequent *thenWaterNone = new FuzzyRuleConsequent(); 
  thenWaterHigh->addOutput(waterNone); // Corrected this line

    // Regel 1: [1, 'cold', 'low', 'dry', 'low', 'much']
    FuzzyRuleAntecedent *rule1 = new FuzzyRuleAntecedent(); 
    rule1->joinWithAND(tempCold, humidLow); 
    rule1->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule1 = new FuzzyRule(1, rule1, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule1);

    // Regel 2: [2, 'cold', 'low', 'dry', 'medium', 'much']
    FuzzyRuleAntecedent *rule2 = new FuzzyRuleAntecedent(); 
    rule2->joinWithAND(tempCold, humidLow); 
    rule2->joinWithAND(moistDry, lightMedium); 
    FuzzyRule *fuzzyRule2 = new FuzzyRule(2, rule2, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule2);

    // Regel 3: [3, 'cold', 'low', 'dry', 'high', 'medium']
    FuzzyRuleAntecedent *rule3 = new FuzzyRuleAntecedent(); 
    rule3->joinWithAND(tempCold, humidLow); 
    rule3->joinWithAND(moistDry, lightHigh); 
    FuzzyRule *fuzzyRule3 = new FuzzyRule(3, rule3, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule3);

    // Regel 10: [10, 'cold', 'medium', 'dry', 'low', 'much']
    FuzzyRuleAntecedent *rule10 = new FuzzyRuleAntecedent(); 
    rule10->joinWithAND(tempCold, humidMedium); 
    rule10->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule10 = new FuzzyRule(10, rule10, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule10);

    // Regel 11: [11, 'cold', 'medium', 'dry', 'medium', 'much']
    FuzzyRuleAntecedent *rule11 = new FuzzyRuleAntecedent(); 
    rule11->joinWithAND(tempCold, humidMedium); 
    rule11->joinWithAND(moistDry, lightMedium); 
    FuzzyRule *fuzzyRule11 = new FuzzyRule(11, rule11, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule11);

    // Regel 12: [12, 'cold', 'medium', 'dry', 'high', 'medium']
    FuzzyRuleAntecedent *rule12 = new FuzzyRuleAntecedent(); 
    rule12->joinWithAND(tempCold, humidMedium); 
    rule12->joinWithAND(moistDry, lightHigh); 
    FuzzyRule *fuzzyRule12 = new FuzzyRule(12, rule12, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule12);

    // Regel 19: [19, 'cold', 'high', 'dry', 'low', 'much']
    FuzzyRuleAntecedent *rule19 = new FuzzyRuleAntecedent(); 
    rule19->joinWithAND(tempCold, humidHigh); 
    rule19->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule19 = new FuzzyRule(19, rule19, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule19);

    // Regel 20: [20, 'cold', 'high', 'dry', 'medium', 'much']
    FuzzyRuleAntecedent *rule20 = new FuzzyRuleAntecedent(); 
    rule20->joinWithAND(tempCold, humidHigh); 
    rule20->joinWithAND(moistDry, lightMedium); 
    FuzzyRule *fuzzyRule20 = new FuzzyRule(20, rule20, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule20);

    // Regel 21: [21, 'cold', 'high', 'dry', 'high', 'medium']
    FuzzyRuleAntecedent *rule21 = new FuzzyRuleAntecedent(); 
    rule21->joinWithAND(tempCold, humidHigh); 
    rule21->joinWithAND(moistDry, lightHigh); 
    FuzzyRule *fuzzyRule21 = new FuzzyRule(21, rule21, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule21);

    // Regel 28: [28, 'medium', 'low', 'dry', 'low', 'much']
    FuzzyRuleAntecedent *rule28 = new FuzzyRuleAntecedent(); 
    rule28->joinWithAND(tempMedium, humidLow); 
    rule28->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule28 = new FuzzyRule(28, rule28, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule28);

    // Regel 29: [29, 'medium', 'low', 'dry', 'medium', 'much']
    FuzzyRuleAntecedent *rule29 = new FuzzyRuleAntecedent(); 
    rule29->joinWithAND(tempMedium, humidLow); 
    rule29->joinWithAND(moistDry, lightMedium); 
    FuzzyRule *fuzzyRule29 = new FuzzyRule(29, rule29, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule29);

    // Regel 30: [30, 'medium', 'low', 'dry', 'high', 'medium']
    FuzzyRuleAntecedent *rule30 = new FuzzyRuleAntecedent(); 
    rule30->joinWithAND(tempMedium, humidLow); 
    rule30->joinWithAND(moistDry, lightHigh); 
    FuzzyRule *fuzzyRule30 = new FuzzyRule(30, rule30, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule30);

    // Regel 37: [37, 'medium', 'medium', 'dry', 'low', 'much']
    FuzzyRuleAntecedent *rule37 = new FuzzyRuleAntecedent(); 
    rule37->joinWithAND(tempMedium, humidMedium); 
    rule37->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule37 = new FuzzyRule(37, rule37, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule37);

    // Regel 38: [38, 'medium', 'medium', 'dry', 'medium', 'much']
    FuzzyRuleAntecedent *rule38 = new FuzzyRuleAntecedent(); 
    rule38->joinWithAND(tempMedium, humidMedium); 
    rule38->joinWithAND(moistDry, lightMedium); 
    FuzzyRule *fuzzyRule38 = new FuzzyRule(38, rule38, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule38);

    // Regel 39: [39, 'medium', 'medium', 'dry', 'high', 'medium']
    FuzzyRuleAntecedent *rule39 = new FuzzyRuleAntecedent(); 
    rule39->joinWithAND(tempMedium, humidMedium); 
    rule39->joinWithAND(moistDry, lightHigh); 
    FuzzyRule *fuzzyRule39 = new FuzzyRule(39, rule39, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule39);

    // Regel 46: [46, 'medium', 'high', 'dry', 'low', 'much']
    FuzzyRuleAntecedent *rule46 = new FuzzyRuleAntecedent(); 
    rule46->joinWithAND(tempMedium, humidHigh); 
    rule46->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule46 = new FuzzyRule(46, rule46, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule46);

    // Regel 47: [47, 'medium', 'high', 'dry', 'medium', 'much']
    FuzzyRuleAntecedent *rule47 = new FuzzyRuleAntecedent(); 
    rule47->joinWithAND(tempMedium, humidHigh); 
    rule47->joinWithAND(moistDry, lightMedium); 
    FuzzyRule *fuzzyRule47 = new FuzzyRule(47, rule47, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule47);

    // Regel 48: [48, 'medium', 'high', 'dry', 'high', 'medium']
    FuzzyRuleAntecedent *rule48 = new FuzzyRuleAntecedent(); 
    rule48->joinWithAND(tempMedium, humidHigh); 
    rule48->joinWithAND(moistDry, lightHigh); 
    FuzzyRule *fuzzyRule48 = new FuzzyRule(48, rule48, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule48);

    // Regel 55: [55, 'hot', 'low', 'dry', 'low', 'much']
    FuzzyRuleAntecedent *rule55 = new FuzzyRuleAntecedent(); 
    rule55->joinWithAND(tempHot, humidLow); 
    rule55->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule55 = new FuzzyRule(55, rule55, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule55);

    // Regel 64: [64, 'hot', 'medium', 'dry', 'low', 'much']
    FuzzyRuleAntecedent *rule64 = new FuzzyRuleAntecedent(); 
    rule64->joinWithAND(tempHot, humidMedium); 
    rule64->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule64 = new FuzzyRule(64, rule64, thenWaterHigh); 
    fuzzy->addFuzzyRule(fuzzyRule64);

    // Regel 65: [65, 'hot', 'medium', 'dry', 'medium', 'medium']
    FuzzyRuleAntecedent *rule65 = new FuzzyRuleAntecedent(); 
    rule65->joinWithAND(tempHot, humidMedium); 
    rule65->joinWithAND(moistDry, lightMedium); 
    FuzzyRule *fuzzyRule65 = new FuzzyRule(65, rule65, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule65);

    // Regel 66: [66, 'hot', 'medium', 'dry', 'high', 'medium']
    FuzzyRuleAntecedent *rule66 = new FuzzyRuleAntecedent(); 
    rule66->joinWithAND(tempHot, humidMedium); 
    rule66->joinWithAND(moistDry, lightHigh); 
    FuzzyRule *fuzzyRule66 = new FuzzyRule(66, rule66, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule66);

    // Regel 73: [73, 'hot', 'high', 'dry', 'low', 'medium']
    FuzzyRuleAntecedent *rule73 = new FuzzyRuleAntecedent(); 
    rule73->joinWithAND(tempHot, humidHigh); 
    rule73->joinWithAND(moistDry, lightLow); 
    FuzzyRule *fuzzyRule73 = new FuzzyRule(73, rule73, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule73);

    // Regel 74: [74, 'hot', 'high', 'dry', 'medium', 'medium']
    FuzzyRuleAntecedent *rule74 = new FuzzyRuleAntecedent(); 
    rule74->joinWithAND(tempHot, humidHigh); 
    rule74->joinWithAND(moistDry, lightMedium); 
    FuzzyRule *fuzzyRule74 = new FuzzyRule(74, rule74, thenWaterMedium); 
    fuzzy->addFuzzyRule(fuzzyRule74);

    // Regel 75: [75, 'hot', 'high', 'dry', 'high', 'little']
    FuzzyRuleAntecedent *rule75 = new FuzzyRuleAntecedent(); 
    rule75->joinWithAND(tempHot, humidHigh); 
    rule75->joinWithAND(moistDry, lightHigh); 
    FuzzyRule *fuzzyRule75 = new FuzzyRule(75, rule75, thenWaterLow); 
    fuzzy->addFuzzyRule(fuzzyRule75);


}

void TimeDelay(int minutes)
{

  for (int i = 0; i < minutes; i++)
  {
    delay(60000); // Delay for 10 seconds in each iteration
    
  }
}



void loop() {
  float MositVal = 0;
  float TempVal =  0;
  float HumidVal = 0;
  float LightVal = 0;
  float fuzVal = 0;

  MositVal = MoistSensor();
  TempVal =  TempSensor();
  HumidVal = HumidSensor();
  LightVal = LightSensor();

  fuzzy->setInput(1, TempVal);
  fuzzy->setInput(2, HumidVal);
  fuzzy->setInput(3, MositVal);
  fuzzy->setInput(4, LightVal);
  
  fuzzy->fuzzify();

  fuzVal = fuzzy->defuzzify(1);
  fuzVal = fuzVal/5;


  // Use a temporary String to build the concatenated string
  String sensorData = "";
  sensorData += String(HumidVal) + ",";
  sensorData += String(TempVal) + ",";
  sensorData += String(LightVal) + ",";
  sensorData += String(MositVal) + ",";
  sensorData += String(fuzVal);

  // Print the concatenated string
  Serial.println(sensorData);

  TurnOnPump(fuzVal);


  TimeDelay(20);
}
