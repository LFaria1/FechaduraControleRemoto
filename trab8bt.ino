#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
//#define CHIP_PIN 4
SoftwareSerial bt(A5, A4);

Adafruit_NeoPixel leds = Adafruit_NeoPixel(5,A0, NEO_GRB + NEO_KHZ800);    
Servo servo;
int state = 0;
int buff;
boolean fade=true;
boolean fadea=true;
unsigned long timer=0;
unsigned long timer2=0;
int manual = 0; //1-trancado 0-aberto
boolean aux2 = false;
MFRC522 nfc(SS_PIN, RST_PIN);
String uid;
//fechado = 255 0 0
//aberto = 0 255 0
//esperando = 255 245 23
void setup() {

  Serial.begin(9600);
  SPI.begin();
  bt.begin(38400);
  leds.begin();
  //color(255,255,255);
//  leds.show();
  servo.attach(5);
  servo.write(0);
  pinMode(SS_PIN, OUTPUT);
  //pinMode(CHIP_PIN,OUTPUT);
  
  nfc.PCD_Init();
  Serial.println("Hello, Planet");
  //Serial.write("AT")
}

void loop() {
  //ler o buffer pra setar o state;
  //se n houver comando state=0;
  if(timer+7000 >millis() && (state==0 || state==4)){
  if(manual==0){
    if(fade){
     fadeG();
      }
      fade=false;   
    }
     
    else if(manual==1)
    {
      if(fade){
        fadeR();
        }
        fade=false;
      }
  }else if(state==2 || state == 1){
    color(255,255,0);
    }else{
      color(0,0,0);}
  /*if(bt.available()){
    Serial.write(bt.read());
    }*/

  if (Serial.available()) {
    bt.write(Serial.read());
    }
  //*****//
  //executa de acordo com o valor da var buff
  if (bt.available()) {
    buff = "";
    while (bt.available()) {

      buff = bt.readString().toInt();
    }
    if (buff == 1) {
      Serial.println("Cadastro de tag");
      Serial.println("Aproxime a tag do leitor");
      bt.write("Cadastro \n Aproxime o cartão do leitor \n");
      fadea=true;
      if(fadea){
      fadeY();
      fadea=false;
      }
      state = 1;
    } else if (buff == 2) {
      Serial.println("Remover tag");
      Serial.println("Aproxime a tag do leitor");
      bt.write("Remover tag \n Aproxime o cartão do leitor \n");
      state = 2;
         if(fadea){
      fadeY();
      fadea=false;
      }
    } else if (buff == 3) {
      Serial.println("modo 3");
      for (int i = 0; i < EEPROM.length(); i += 4) {
        Serial.print((i + 4) / 4);
        Serial.print(".");

        for (int j = 0; j < 4; j++) {
          Serial.print(EEPROM.read(i + j));
          Serial.print(" ");
        }
        Serial.println();
      }
    } else if (buff == 4) {
     
      state = 4;
    } else {
      Serial.println("digite um modo válido");
      bt.write("Digite um modo válido \n");
    }
  }
  /*if (timer + 3000 <= millis()  && aux2) {
    manual = 0;
    servo.write(180 * manual);
    Serial.println("deuaq");
    aux2 = false;
  }*/
  if (state == 4 && timer+2500<millis()) {
    
    if (manual == 0) {
        manual = 1;
    } else {
      manual = 0;
    }
    servo.write(180 * manual);
    /*if(manual==1){
    color(0,255,0);
    }else if(manual==0)
    {
      color(255,0,0);
      }*/
    state = 0;
    //delay(1000);
    timer=millis();
    fade=true;
}

//espera cartao
if (!nfc.PICC_IsNewCardPresent()) {
  return;
}
if (!nfc.PICC_ReadCardSerial()) {
  return;
}
//cadastro da tag
if (state == 1) {
  digitalWrite(SS_PIN, LOW);

  for (byte i = 0; i < nfc.uid.size; i++) {
    //MEE
    uid += String(nfc.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //
    uid += String(nfc.uid.uidByte[i], HEX);
  }
  Serial.println(uid);

  //verifica se já tem e o prox espaço vazio
  boolean aux = false; //aux true = já está ocupado
  int empty = EEPROM.read(1023);
  for (int i = 0; i < EEPROM.length() - 4; i += 4) {
    if (EEPROM.read(i) == nfc.uid.uidByte[0] && EEPROM.read(i + 1) == nfc.uid.uidByte[1] && EEPROM.read(i + 2) == nfc.uid.uidByte[2] && EEPROM.read(i + 3) == nfc.uid.uidByte[3] ) {
      aux = true;
    }
  }
  if (!aux) {
    for (int i = 0; i < 4; i++) {
      EEPROM.write((empty + i), nfc.uid.uidByte[i]);
      aux = true; //só pra ter ctz
    }
    int nextEmpty = EEPROM.read(1023);
    EEPROM.write(1023, nextEmpty + 4);
    Serial.println("tag cadastrada com sucesso");
    bt.write("Tag cadastrada com sucesso \n");
    delay(1000);
  } else {
    Serial.println("tag já cadastrada");
    bt.write("Tag já cadastrada \n");
    
  }
  uid = "";
  state = 0;
  nfc.PICC_HaltA();
  timer2=millis();
}
//--
if (state == 2) {
  boolean aux = false;
  //verifica se já tá cadastrado
  for (int i = 0; i < EEPROM.length() - 4; i += 4) {
    if (EEPROM.read(i) == nfc.uid.uidByte[0] && EEPROM.read(i + 1) == nfc.uid.uidByte[1] && EEPROM.read(i + 2) == nfc.uid.uidByte[2] && EEPROM.read(i + 3) == nfc.uid.uidByte[3] ) {
      aux = true;
      EEPROM.write(i, 255);
      EEPROM.write((i + 1), 255);
      EEPROM.write((i + 2), 255);
      EEPROM.write((i + 3), 255);
    }
  }
  if (aux) {
    Serial.println("tag removida com  sucesso");
   bt.write("Tag removida com sucesso \n");
   
  } else {
    Serial.println("tag não cadastrada");
   bt.write("Tag não cadastrada \n");
   
  }
  state = 0;
  nfc.PICC_HaltA();
  timer2=millis();
}
//--
if (state == 0 && timer2+3000<millis()) {
  for (int i = 0; i < EEPROM.length() - 4; i += 4) {
    if (EEPROM.read(i) == nfc.uid.uidByte[0] && EEPROM.read(i + 1) == nfc.uid.uidByte[1] && EEPROM.read(i + 2) == nfc.uid.uidByte[2] && EEPROM.read(i + 3) == nfc.uid.uidByte[3]) {
      state=4;
      Serial.println("foi");
      }
    }
  }
}

void color(int r, int g, int b){
  for(int i=0;i<=4;i++){
    leds.setPixelColor(i,r,g,b);    
    }
    leds.show();
  }

void fadeG(){
  for(int i=0;i<=255;i++){
    color(0,i,0);
    delay(2);
    }
  }

void fadeR(){
    for(int i=0;i<=255;i++){
    color(i,0,0);
    delay(2);
    }
}

void fadeY(){
  
  for(int i=0;i<=255;i++){
    color(i,i,0);
    delay(2);
    }
  
  }
