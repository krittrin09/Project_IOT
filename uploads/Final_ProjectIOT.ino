#include <TridentTD_LineNotify.h> //เรียกใช้ Library แจ้งเตือนโดย Line
#include <BlynkSimpleEsp8266.h> //เรียกใช้ Library Blynk แบบง่ายสำหรับ ESP8266 https://github.com/blynkkk/blynk-library/releases/download/v1.1.0/Blynk_Release_v1.1.0.zip
#include <WiFiManager.h> //เรียกใช้ Library WifiManager โดยเป็นการจัดการเชื่อมต่อ Wifi ผ่าน WebManager
#include <ESP8266WiFi.h> //เรียกใช้ Wifi ของ ESP82666
#include <SimpleTimer.h> //กำหนดเวลาการทำงานของแต่ละ Function แบบแยกกันโดยไม่ใช้ Delay
#include <Servo.h> //เรียกใช้ Library Servo เพื่อให้ใช้งาน คำสั่งควบคุม Servo ได้

#define BLYNK_PRINT Serial
#define sw D7 //ประกาศตัวแปร sw รับค่าจากขา D7
#define led D8 //ประกาศตัวแปร led รับค่าจากขา D8
#define LINE_TOKEN "yBK8Z5TBpvbRFqJU8IsZmNqUX1ctHiQKVDAmWPMePKU" //รับ Token จาก line notify
#define BLYNK_TOKEN "H14XPmbsr26dYvZDm66vlRpiH4ICrp5J" //รับ Blynk_Token(auth)

WiFiManager wm; //ประกาศตัวแปร wm รับค่าจาก Function WiFiManager
SimpleTimer timer; //ประกาศตัวแปร timer รับค่าจาก Function SimpleTimer
Servo servo; //ประกาศตัวแปร servo รับค่าจาก Function Servo
WidgetLED detected(V4); ////ประกาศตัวแปร detected รับค่าจากขา V4 จาก application Blynk

const int pingPin = D1; //ประกาศตัวแปร PingPin รับค่าจากขา D1 ที่ต่อกับตัว Ultrasonic
const int inPin = D2; //ประกาศตัวแปร PingPin รับค่าจากขา D2 ที่ต่อกับตัว Ultrasonic
long distance; //ประกาศตัวแปร distance รับค่าระยะทาง
int timeout = 180; //ตัวแปร Timeout กำหนดเวลาสำหรับ Wifimanager

void setup() {
  WiFi.mode(WIFI_STA); //กำหนด Wifi เป็นโหมด Station
  Serial.begin(9600); //กำหนดความเร็วในการสื่อสาร 9600
  servo.attach(D5); //กำหนดให้ Servo รับค่าจากขาสัญญาณ D5
  pinMode(sw, INPUT_PULLUP); //ทำการกำหนด PinMode INPUT_PULLUP เป็นสัญญาณเข้ามา
  pinMode(led, OUTPUT);//ทำการกำหนด PinMode OUTPUT เป็นสัญญาณออก
  Blynk.config(BLYNK_TOKEN,"blynk.iot-cm.com", 8080); //กำหนด Token,Domain , port ของ Blynk
  LINE.setToken(LINE_TOKEN);// กำหนด LineToken รับค่ามาจาก LINE_TOKEN
  servo.write(80);//กำหนดให้ Servo เริ่มที่ 80 องศา
  timer.setInterval(100L,reset_wifi); //กำหนดเวลาในการทำงานของ Function โดยใช้ SimpleTimer
  timer.setInterval(5000L,read_ultraSonic); //กำหนดเวลาในการทำงานของ Function โดยใช้ SimpleTimer
}

void loop() {
  Blynk.run(); //สั่งให้ Blynk ทำงาน
  timer.run(); //สั่งให้ Timer ทำงาน
}

BLYNK_WRITE(V1){
  int pinValue = param.asInt(); // รับค่า pinvalue 0 1 จาก V1
  if (pinValue == 1){
    servo.write(20); //ทำการสั่งให้ Servo หมุน 20 องศาเพื่อเปิดช่องให้อาหารแมว
    LINE.notify("ให้อาหารแมว");  //ทำการส่งการแจ้งเตือนว่าได้ให้อาหารแมวแล้วไปยัง Line Notify
    Serial.print("V1 button value is:");
    Serial.println(pinValue);
  }
  else if(pinValue == 0){
    servo.write(80); //ทำการสั่งให้ Servo หมุน 80 องศาเพื่อปิดช่องให้อาหารแมว
    Serial.print("V1 button value is:");
    Serial.println(pinValue); 
  }
}

void led_blink() {
  //ทำการสั่งให้ LED กระพริบติด/ดับ 7 ครั้ง
  for (int i = 0; i <= 6; i++){
    delay(500);
    digitalWrite(led,!digitalRead(led));
  }
}

void reset_wifi(){
  //Function reset_Wifi ทำงานเมื่อ Sw มีค่าเป็น LOW โดยการกดปุ่มที่ Switch
  if (digitalRead(sw) == LOW) {
    led_blink();//เรียกใช้ Function กระพริบเพื่อบอกว่ามีการ Reset Wifi
    wm.setConfigPortalTimeout(timeout);//ทำการเรียกใช้ WebPortal ในการกำหนด Config(SSID,Password) WifiManager
    //เปลี่ยน Wifi ESP8266 ให้เป็น AP Mode และ กำหนดชื่อให้ Wifi ของ ESP8266
    if (!wm.startConfigPortal("Automatic_Cat_Feeder")) {
      Serial.print("Fail to connect and timeout");
      delay(3000);
      ESP.restart(); //ทำการ Reset ESP8266
      delay(5000);
    }
    digitalWrite(led, LOW); //สั่งให้ไฟดับเมื่อทำการต่อ Wifi สำเร็จ
    LINE.notify("WIFI CONNECTED"); //ส่งข้อความว่า "WIFI CONNECTED" ไปยัง Line
    Serial.print("Wifi Connected");
    Serial.print("IP = ");
    Serial.println(WiFi.localIP());
  }
}

void read_ultraSonic(){
  //ทำการอ่านค่าจาก function ultrasonic
  ultraSonic();
  if (distance <= 35){
    digitalWrite(led,HIGH);
    detected.on();
    LINE.notify("ตรวจพบแมวระยะ  " +String(distance)+ " CM"); // เมื่อเจอวัตถุในระยะ 35 CM จะส่งแจ้งเตือนทางเป็น LED Blynk และข้อความทาง LINE
  }
  else if (distance >= 36){
    digitalWrite(led,LOW);
    detected.off();
  }
  Blynk.virtualWrite(V2,distance);//ส่งค่าระยะทางไปยัง Blynk
}

void ultraSonic() {
  //Function กำหนดค่าต่างๆของตัว ultrasonic โดยแปลงระยะเวลาเป็นระยะทางหน่วย CM
  long duration;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(inPin, INPUT);
  duration = pulseIn(inPin, HIGH);
  distance = (duration/29)/2;
  Serial.print(distance);
  Serial.print(" cm");
  Serial.println();
}
