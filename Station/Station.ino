#include <SPI.h>
#include <LoRa.h>
#include <Servo.h>
#include <HX711_ADC.h>

#define infrared  7
const int ledAuto = 4;
const int ledOver = 8;
const int button = A2;
const unsigned int TRIG_PIN = 6;
const unsigned int ECHO_PIN = 5;
HX711_ADC LoadCell(A1,A0);
Servo myservo;

float w = 0.0;
int pos = 0; 
byte ledStateAuto;
byte ledStateOver;

int state = 0;
String Incoming = "";
String Message = "";  

//---------------------------------------- LoRa data transmission configuration.
////////////////////////////////////////////////////////////////////////////
// PLEASE UNCOMMENT AND SELECT ONE OF THE "LocalAddress" VARIABLES BELOW. //
////////////////////////////////////////////////////////////////////////////

byte LocalAddress = 0x02;       //--> address of this device (Slave 1).
//byte LocalAddress = 0x03;       //--> address of this device (Slave 2).

byte Destination_Master = 0x01; //--> destination to send to Master (ESP32).
//----------------------------------------

//---------------------------------------- Millis / Timer to update weight values from sensor.
unsigned long previousMillis_UpdateDHT11 = 0;
const long interval_UpdateDHT11 = 1000;
//---------------------------------------- 

//________________________________________________________________________________ Subroutines for sending data (LoRa Ra-02).
void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();             //--> start packet
  LoRa.write(Destination);        //--> add destination address
  LoRa.write(LocalAddress);       //--> add sender address
  LoRa.write(Outgoing.length());  //--> add payload length
  LoRa.print(Outgoing);           //--> add payload
  LoRa.endPacket();               //--> finish packet and send it
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutines for receiving data (LoRa Ra-02).
void onReceive(int packetSize) {
  if (packetSize == 0) {
    Serial.println("There's no packet");
    return;
  }   //--> if there's no packet, return
  
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

  // Calls the Processing_incoming_data() subroutine.
  Processing_incoming_data();
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutines to process data from incoming messages, then send messages to the Master.
void Processing_incoming_data() {
  //---------------------------------------- Conditions for sending messages to Master.
/////////////////////////////////////////////////////////////////////////////////////////
// PLEASE UNCOMMENT THE LINE OF CODE BELOW IF THIS CODE OR THIS DEVICE IS FOR SLAVE 1. //
/////////////////////////////////////////////////////////////////////////////////////////

  if (Incoming == "SDS11" && (digitalRead (infrared) == HIGH)) {
    digitalWrite(ledAuto, HIGH);
    digitalWrite(ledOver, LOW);

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    const unsigned long duration = pulseIn(ECHO_PIN, HIGH);
    int distance = duration/29/2;
    if(duration==0){
      Serial.println("Warning: no pulse from sensor");
    } 
    else{
      Serial.print("Distance to nearest object: ");
      Serial.print(distance);
      Serial.println(" cm");
    }
    if (distance > 10 ) {
      myservo.write(180);  
    } 
    else if (distance <= 10 && distance >0) {     
      myservo.write(60); 
      delay(4000);
    }   

    ledStateAuto = digitalRead(ledAuto);
    ledStateOver = digitalRead(ledOver);
    Message = "";
    Message = "SL1," + String(w) + "," + String(ledStateAuto) + "," + String(ledStateOver);

    Serial.println();
    Serial.println("Send message to Master");
    Serial.print("Message: ");
    Serial.println(Message);
   
    sendMessage(Message, Destination_Master);
  }
  if (Incoming == "SDS11" && (digitalRead (infrared) == LOW)) {
    digitalWrite(ledAuto, HIGH);
    digitalWrite(ledOver, HIGH); 

    ledStateAuto = digitalRead(ledAuto);
    ledStateOver = digitalRead(ledOver);
    Message = "";
    Message = "SL1," + String(w) + "," + String(ledStateAuto) + "," + String(ledStateOver);

    Serial.println();
    Serial.println("Send message to Master");
    Serial.print("Message: ");
    Serial.println(Message);
   
    sendMessage(Message, Destination_Master);
  }
  if (Incoming == "SDS10" && (digitalRead (infrared) == LOW)) {
    digitalWrite(ledAuto, LOW);
    digitalWrite(ledOver, HIGH); 

    ledStateAuto = digitalRead(ledAuto);
    ledStateOver = digitalRead(ledOver);
    Message = "";
    Message = "SL1," + String(w) + "," + String(ledStateAuto) + "," + String(ledStateOver);

    Serial.println();
    Serial.println("Send message to Master");
    Serial.print("Message: ");
    Serial.println(Message);
   
    sendMessage(Message, Destination_Master);
  }
  if (Incoming == "SDS10" && (digitalRead (infrared) == HIGH)) {
    digitalWrite(ledAuto, LOW);
    digitalWrite(ledOver, LOW);

    ledStateAuto = digitalRead(ledAuto);
    ledStateOver = digitalRead(ledOver);
    Message = "";
    Message = "SL1," + String(w) + "," + String(ledStateAuto) + "," + String(ledStateOver);

    Serial.println();
    Serial.println("Send message to Master");
    Serial.print("Message: ");
    Serial.println(Message);
   
    sendMessage(Message, Destination_Master);
  }
  //---------------------------------------- 

  //---------------------------------------- Conditions for sending messages to Master.
/////////////////////////////////////////////////////////////////////////////////////////
// PLEASE UNCOMMENT THE LINE OF CODE BELOW IF THIS CODE OR THIS DEVICE IS FOR SLAVE 2. //
/////////////////////////////////////////////////////////////////////////////////////////

//  if (Incoming == "SDS2") {
//    digitalWrite(LED_1_Pin, !digitalRead(LED_1_Pin));
//    digitalWrite(LED_2_Pin, !digitalRead(LED_2_Pin));
//    
//    LED_1_State = digitalRead(LED_1_Pin);
//    LED_2_State = digitalRead(LED_2_Pin);
//
//    Message = "";
//    Message = "SL2," + String(h) + "," + String(t) + "," + String(LED_1_State) + "," + String(LED_2_State);
//
//    Serial.println();
//    Serial.println("Send message to Master");
//    Serial.print("Message: ");
//    Serial.println(Message);
//    
//    sendMessage(Message, Destination_Master);
//  }
  //---------------------------------------- 
}
//________________________________________________________________________________ 


void setup() {
  Serial.begin(9600);
  
  pinMode (ledAuto, OUTPUT);
  pinMode (ledOver, OUTPUT);
  myservo.attach(3); 
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(infrared, INPUT);
  pinMode(button, INPUT);

  for (pos = 180; pos >= 60; pos -= 1) { 
      myservo.write(pos);            
  }
  for (pos = 60; pos <= 180; pos += 1) { 
      myservo.write(pos);            
  }

  LoadCell.begin();
  LoadCell.start(2000);
  LoadCell.setCalFactor(1000.0);

  //---------------------------------------- Settings and start Lora Ra-01.
  Serial.println();
  Serial.println("Start LoRa init...");
  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 or 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");
  //---------------------------------------- 
}



//________________________________________________________________________________ VOID LOOP
void loop() {
  // put your main code here, to run repeatedly:

  //---------------------------------------- Millis / Timer to update the temperature and humidity values ​​from the DHT11 sensor every 2 seconds (see the variable interval_UpdateDHT11).
  unsigned long currentMillis_UpdateDHT11 = millis();
  
  if (currentMillis_UpdateDHT11 - previousMillis_UpdateDHT11 >= interval_UpdateDHT11) {
    previousMillis_UpdateDHT11 = currentMillis_UpdateDHT11;
    // Reading weight takes about 250 milliseconds!
    LoadCell.update();
    w = LoadCell.getData();
    
    // Check if any reads failed and exit early (to try again).
    if (isnan(w)) {
      Serial.println(F("Failed to read from LoadCell sensor!"));
      w = 0.0;
    }
  }
  //---------------------------------------- 
  state = digitalRead(button);
  if (state == 1) {
    Serial.println("Open");
    myservo.write(60); 
    delay(3000);
    return;
  } 
  //---------------------------------------- parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  //----------------------------------------
}
//________________________________________________________________________________ 
