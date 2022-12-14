/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID           "TMPLeWXGKa8C"
#define BLYNK_DEVICE_NAME           "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "GCR7jFAIv36InWj1XNW4u365ZSQfdZb3"
#define DHTTYPE DHT22   
#define sensorApi 32
#define DHTPIN 33 
#define sensorGas 35 
#define sensorPir 27 
#define led 26
#define buzzer 25
#define waktuCekAdaOrang 30
#define BOTtoken "5928085070:AAG-z6qotjALbmB2Ne83aEbqHQQgBZPvFMQ"  // your Bot Token (Get from Botfather)
#include <LiquidCrystal_I2C.h>

#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <MQ2.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
bool startTimer = false;
double readLpgValue;
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
unsigned long startMillis;
unsigned long currentMillis;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Redmi";
char pass[] = "12345678";

BlynkTimer timer;
BlynkTimer timer1;
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
MQ2 mq2(sensorGas);
double humidityValue;  //Stores humidity value
double temperatureValue; 
int fireSensorValue;
int pirSensorValue;
String cekOrang = "Tidak Ada Orang";
String cekApi = "KOMPOR MATI";
String cekGas = "GAS AMAN";
String chat_id;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);


// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  digitalWrite(led, HIGH);
  cekOrang = "Ada Orang";
  startTimer = true;
  lastTrigger = millis();
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  humidityValue = dht.readHumidity();
  temperatureValue = dht.readTemperature();
  fireSensorValue = digitalRead(sensorApi);
  pirSensorValue = digitalRead(sensorPir);
  
  readLpgValue = analogRead(sensorGas);
  Serial.println(readLpgValue);
  if(readLpgValue > 1000){
      cekGas = "GAS BOCOR";
  }else{
      cekGas = "Gas Aman";
  }

  if(fireSensorValue == 0){
    cekApi = "Kompor Menyala";    
  }else{
    cekApi = "Kompor Mati";
  }
  lcd.setCursor(0, 0);
  lcd.print("Suhu : ");
  lcd.print(temperatureValue);
  lcd.setCursor(0, 1); 
  lcd.print("Humidity : ");
  lcd.print(humidityValue);

  Blynk.virtualWrite(V32, humidityValue);
  Blynk.virtualWrite(V34, temperatureValue);
  Blynk.virtualWrite(V33, cekApi);
  Blynk.virtualWrite(V27, cekOrang);
  Blynk.virtualWrite(V26, cekGas);
  Blynk.virtualWrite(V35, readLpgValue);
}

void handleNewMessages(int numNewMessages) {
  if(numNewMessages == 0){
  Serial.print("handle new message ");
  Serial.println(numNewMessages);
      
     for (int i=0; i<numNewMessages+1; i++) {
       String chat_id = String(bot.messages[i].chat_id);
       Serial.println(chat_id);
       bot.sendMessage(chat_id, "GAS BOCOR MATIKAN SEGERA MATIKAN SEMUA ALAT LISTRIK DAN JANGAN PANIK !!!!!");
     }   
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(sensorApi, INPUT);
  pinMode(sensorPir, INPUT);
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPir), detectsMovement, RISING);
  dht.begin();
  mq2.begin();
  lcd.init();
  lcd.backlight();
  float lpgValue = analogRead(sensorGas);
  Serial.print("mulai baca ");
  Serial.println(lpgValue);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  #ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
   delay(1000);
   Serial.println("Connecting to WiFi..");
  }
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  timer.setInterval(3000L, myTimerEvent);
}

void loop()
{

  Blynk.run();
  timer.run();
  
  now = millis();
  if(startTimer && (now - lastTrigger > (waktuCekAdaOrang*1000))) {
    Serial.println("Motion stopped...");
    digitalWrite(led, LOW);
    startTimer = false;
  }

  currentMillis = millis();  
  if(millis() > lastTimeBotRan + botRequestDelay){
    if(currentMillis - startMillis >= 10000){
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      Serial.print("dapet pesan ");
      Serial.println(numNewMessages);
      if(cekGas == "GAS BOCOR"){
        tone(buzzer, 293);
        Serial.println("masuk fungsi");
        handleNewMessages(numNewMessages);    
        startMillis = currentMillis;  
      }else{
        noTone(buzzer);
      }
    }    
  }

}
