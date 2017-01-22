#include <DHT.h>

int switchPin = 5;
int value = 0;
#define DEFAULT_SKIP_COUNT 20


//#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11 
#define DEFAULT_DELAY 70
DHT dht(5, DHTTYPE);
DHT dhtBatery(6, DHTTYPE);

int currentCount;
int currentCountTemperature = 0;


struct Sensor {
  const char* mSensorName;
  int mPort;
  bool mIsProcessLed = false;
  bool mIsCritical = false;
  int mNormalValue;

  Sensor(const char* sensortName, int port, bool isProcessLed = 0, int normalValue = 0, bool isCritical = false)
    : mSensorName(sensortName)
    , mPort(port)
    , mIsProcessLed(isProcessLed)
    , mNormalValue(normalValue)
    , mIsCritical(isCritical)
  {
  }
};

Sensor sensors[] =
{
  //{"FIRE", 2},
  //{"CO2", 3, true},
  //{"GAS", 4},
  {"PIR1", 11, 1, HIGH, 0},
  {"PIR2", 10, 1, HIGH, 0},
  {"PIR3_test", 3, 1, LOW, 0},
  {"PIR4_test", 2, 1, LOW, 0},
  {"Knock1", 4, 1, LOW, 0}
  
};


//#include <Servo.h> //используем библиотеку для работы с сервоприводом
//Servo servo; //объявляем переменную servo типа Servo

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(switchPin, INPUT);
  Serial.begin(9600);
  //servo.attach(10);
  for (int i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++)
  {
    pinMode(sensors[i].mPort, INPUT);
  }

  dht.begin();
  dhtBatery.begin();
}



void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  bool isForcePrint = currentCount++ == DEFAULT_SKIP_COUNT;
  for (int i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++)
  {
    value = digitalRead(sensors[i].mPort);
    if (sensors[i].mIsProcessLed && sensors[i].mNormalValue != value)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print(sensors[i].mSensorName);
      Serial.print(" ");
      Serial.println(value);
      delay(DEFAULT_DELAY);
    }
    else
    {
      if (isForcePrint)
      {
        Serial.print(sensors[i].mSensorName);
        Serial.print(" ");
        Serial.println(value);
        currentCount = 0;
        delay(DEFAULT_DELAY);
      }
    }
    //value = analogRead(sensors[i].mPort);


    Serial.flush();
    
  }


  if (currentCountTemperature ++ == 50)
  {
    currentCountTemperature = 0;

    {
      float h = dhtBatery.readHumidity();
      float t = dhtBatery.readTemperature();

      // check if returns are valid, if they are NaN (not a number) then something went wrong!
      if (isnan(t) || isnan(h)) {
        Serial.println("Failed to read from DHT");
      } else {
        Serial.print("Humidity_batery ");
        Serial.println(h);
        delay(DEFAULT_DELAY);
        Serial.print("Temperature_batery ");
        Serial.println(t);
      }
    }

    {
      float h = dht.readHumidity();
      float t = dht.readTemperature();

      // check if returns are valid, if they are NaN (not a number) then something went wrong!
      if (isnan(t) || isnan(h)) {
        Serial.println("Failed to read from DHT");
      } else {
        Serial.print("Humidity_home ");
        Serial.println(h);
        delay(DEFAULT_DELAY);
        Serial.print("Temperature_home ");
        Serial.println(t);
      }
    }



  }



  /*
    boolValue = !boolValue;
    if (boolValue)
    {
      servo.write(0); //ставим вал под 0
      Serial.print("10 ");
    }
    else
    {
      servo.write(180); //ставим вал под 180
      Serial.print("180 ");
    }

    delay(2000); //ждем 2 секунды
  */
  delay(100);


}
