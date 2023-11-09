#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//---------------------------------------- LoRa Pin / GPIO configuration.
#define ss 15
#define rst 16
#define dio0 2
//----------------------------------------

//---------------------------------------- LCD Pin.
LiquidCrystal_I2C lcd(0x27,16,2);
//----------------------------------------

//---------------------------------------- Blynk.
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6uz3V8Iyk"
#define BLYNK_TEMPLATE_NAME "Waste Management System"
#define BLYNK_AUTH_TOKEN "tgK7Rz9KY_b36KhVDYpFHF-1IB0pt6Zj"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
char ssid[] = "Minh Tai";
char pass[] = "71120079";
//----------------------------------------

//---------------------------------------- Variable declaration to hold incoming and outgoing data.
String Incoming = "";
String Message = ""; 
//----------------------------------------

//---------------------------------------- LoRa data transmission configuration.
byte LocalAddress = 0x01;                 //--> address of this device (Master Address).
byte Destination_Arduino_Slave_1 = 0x02;    //--> destination to send to Slave 1 (Arduino).
//---------------------------------------- 

//---------------------------------------- Variable declaration for Millis/Timer.
unsigned long previousMillis_SendMSG = 0;
const long interval_SendMSG = 100;
//---------------------------------------- 

// Variable declaration to count slaves.
byte Slv = 0;
int autoOpen;
// Variable rece.
float w;
int ledAuto, ledOpen;

void onReceive(int packetSize) {
  if (packetSize == 0) return;  //--> if there's no packet, return

  //---------------------------------------- read packet header bytes:
  int recipient = LoRa.read();        //--> recipient address
  byte sender = LoRa.read();          //--> sender address
  byte incomingLength = LoRa.read();  //--> incoming msg length
  //---------------------------------------- 

  // Clears Incoming variable data.
  Incoming = "";

  //---------------------------------------- Get all incoming data.
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();
  }
  //---------------------------------------- 

  //---------------------------------------- Check length for error.
  if (incomingLength != Incoming.length()) {
    Serial.println();
    Serial.println("error: message length does not match length");
    return; //--> skip rest of function
  }
  //---------------------------------------- 

  //---------------------------------------- Checks whether the incoming data or message for this device.
  if (recipient != LocalAddress) {
    Serial.println();
    Serial.println("This message is not for me.");
    return; //--> skip rest of function
  }
  //---------------------------------------- 

  //---------------------------------------- if message is for this device, or broadcast, print details:
  Serial.println();
  Serial.println("Received from: 0x" + String(sender, HEX));
  //Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + Incoming);
  //Serial.println("RSSI: " + String(LoRa.packetRssi()));
  //Serial.println("Snr: " + String(LoRa.packetSnr()));
  //---------------------------------------- 
  w = 0.0;
  ledAuto = 0;
  ledOpen = 0;

  int viTriDauPhay1 = Incoming.indexOf(',');
  int viTriDauPhay2 = Incoming.indexOf(',', viTriDauPhay1 + 1);

  String soThuNhatStr = Incoming.substring(viTriDauPhay1 + 1, viTriDauPhay2);
  String soThuHaiStr = Incoming.substring(viTriDauPhay2 + 1, Incoming.indexOf(',', viTriDauPhay2 + 1));
  String soThuBaStr = Incoming.substring(Incoming.lastIndexOf(',') + 1);

  w = soThuNhatStr.toFloat();
  double doubleValue = static_cast<double>(w);
  ledAuto = soThuHaiStr.toInt();
  ledOpen = soThuBaStr.toInt();  
  String tmp;
  if (ledOpen == 1) {
    tmp = "Y";
    WidgetLED LED(V2);
    LED.on();
  } else {
    tmp = "N";
    WidgetLED LED(V2);
    LED.off();
  }
  lcd.setCursor(10,1);
  lcd.print("Over:" + tmp);
  lcd.setCursor(0,0);
  lcd.print("Amount:");
  lcd.setCursor(7,0);
  lcd.print(w);
  Blynk.virtualWrite(V1, doubleValue);
}
//________________________________________________________________________________ 

void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();             //--> start packet
  LoRa.write(Destination);        //--> add destination address
  LoRa.write(LocalAddress);       //--> add sender address
  LoRa.write(Outgoing.length());  //--> add payload length
  LoRa.print(Outgoing);           //--> add payload
  LoRa.endPacket();               //--> finish packet and send it
}

//________________________________________________________________________________ VOID SETUP
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Wire.begin(D3,D4); 
  lcd.init();
  lcd.clear();
  lcd.backlight();
  //---------------------------------------- Settings and start Lora Ra-01.
  LoRa.setPins(ss, rst, dio0);
  Serial.println("Start LoRa init...");
  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 or 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");
  //---------------------------------------- 
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID LOOP
void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  //---------------------------------------- Millis or Timer to send message / command data to slaves every 1 second (see interval_SendCMD variable).
  // Messages are sent every one second is alternately.
  // > Master sends message to Slave 1, delay 1 second.
  // > Master sends message to Slave 2, delay 1 second.
  
  unsigned long currentMillis_SendMSG = millis();
  
  if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
    previousMillis_SendMSG = currentMillis_SendMSG;

    Slv++;
    if (Slv > 2) Slv = 1;
    
    if (autoOpen == 1) {
      lcd.setCursor(0,1);
      lcd.print("Auto:ON ");
    } else {
      lcd.setCursor(0,1);
      lcd.print("Auto:OFF");
    }

    Message = "SDS" + String(Slv) + String(autoOpen);

    //::::::::::::::::: Condition for sending message / command data to Slave 1 (UNO Slave 1).
    if (Slv == 1) {
      Serial.println();
      Serial.print("Send message to UNO Slave " + String(Slv));
      Serial.println(" : " + Message);
      sendMessage(Message, Destination_Arduino_Slave_1);
    }
    //:::::::::::::::::

    //::::::::::::::::: Condition for sending message / command data to Slave 2 (UNO Slave 2).
    // if (Slv == 2) {
    //   Serial.println();
    //   Serial.print("Send message to ESP32 Slave " + String(Slv));
    //   Serial.println(" : " + Message);
    //   sendMessage(Message, Destination_ESP32_Slave_2);
    // }
    //:::::::::::::::::
  }
  //----------------------------------------

  //---------------------------------------- parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  //----------------------------------------
}
//________________________________________________________________________________ 

BLYNK_WRITE(V0) {
  autoOpen = param.asInt();
}