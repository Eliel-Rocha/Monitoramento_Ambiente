
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <dummy.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>


// BLUETOOTH _______________________________________________________________________
#include "BluetoothSerial.h"

//#define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
const char *pin = "1234"; // Change this to more secure PIN.

String device_name = "ESP32-LIEC";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
//_______________________________________________________________________________
// CARACTERES ESPECIAIS NO LCD _____________________________________________________

byte caracter0[] = {
  B00000,
  B01010,
  B01010,
  B01010,
  B00000,
  B10001,
  B01110,
  B00000
};

byte caracter1[] = {
  B00000,
  B01010,
  B01010,
  B01010,
  B00000,
  B01110,
  B10001,
  B00000
};

//_______________________________________________________________________________
//Definiçao dos pinos dos sensores e atuadores___________________________________
#define DHT_PIN 2         // Pino do DHT11
#define LDR_PIN 4         // Pino do LDR (digital)
#define FAN_PIN 15        // Pino "ventilador" (ventoinha do PC)
#define LAMP_PIN 19       // Pino da "lampada"/LED (ou sei la oq vamos ligar)

// Define o tipo do sensor DHT (DHT11)
#define DHT_TYPE DHT11

// Cria um objeto para o sensor DHT
DHT dht(DHT_PIN, DHT_TYPE);

// inicializa o LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// variaveis de ligar e desligar ventilador
bool ventiladorLigado = false;
char ventilador;

// texto pro app inventor
String texto;
 
void setup() {

  // Monitor serial
  Serial.begin(9600);

  // LCD
  lcd.init();
  lcd.backlight();
  delay(100);
 
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.setCursor(0, 1);
  lcd.print("U: ");

  // BLUETOOTH
  SerialBT.begin(device_name); //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  //Serial.printf("The device with name \"%s\" and MAC address %s is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str(), SerialBT.getMacString()); // Use this after the MAC method is implemented
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif

  
  // Inicializa caracteres especiais
  lcd.createChar(0, caracter0);
  lcd.createChar(1, caracter1);

  // Inicializar o sensor DHT
  dht.begin();

  
  // Configure os pinos como entrada ou saída
  pinMode(LDR_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
 
 }
 
void loop() {

  // Leituras do DHT ___________________________________________________
  // Lê a temperatura e umidade do sensor DHT e armazena nas variáveis
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  // Exiba as leituras no display LCD
  lcd.setCursor(3, 0);
  lcd.print(String(temperatura,1));
  lcd.print("C");
  lcd.setCursor(3, 1);
  lcd.print(String(umidade,1));
  lcd.print("%");


  //________________________LDR_____________________________________

  // Verifica a luminosidade (LDR) e liga/desliga a lâmpada
  if (digitalRead(LDR_PIN) == HIGH) {
    digitalWrite(LAMP_PIN, HIGH);    // Liga a lâmpada
    
  } else {
    digitalWrite(LAMP_PIN, LOW);      // Desliga a lâmpada
  }

  // BLUETOOTH _____________________________________________________
  ventilador = SerialBT.read();    // Leitura app inventor
  Serial.print(ventilador);

  // Controle altomatico da FAN _________________________________________
  if(ventilador == 'A' || ventilador == NULL){  
      // Verifica a temperatura e liga/desliga o ventilador e o LED vermelho
      if (temperatura >= 30 ) {         // Ajuste de temperatura 
        digitalWrite(FAN_PIN, HIGH);    // Liga o ventilador
        lcd.setCursor(9, 0);            // Posiciona cursor
        lcd.write(byte(1));             // Desenho triste
        ventiladorLigado = true;
      } else {
        digitalWrite(FAN_PIN, LOW);     // Desliga o ventilador
        lcd.setCursor(9, 0);            // Posiciona cursor
        lcd.write(byte(0));             // Desenho feliz
        ventiladorLigado = false;
      }
      
    // Controle Manual da FAN
    }else {
      if (ventilador == 'L') {
          digitalWrite(FAN_PIN, HIGH); // Liga o ventilador
      } else if (ventilador == 'D'){
          digitalWrite(FAN_PIN, LOW); // Desliga o ventilador
      }
    }
    
  
  // Envia leituras de temperatura e umidade para o app inventor
  texto = String(String(temperatura));
  texto += String("/" + String(umidade));
  SerialBT.println(texto);
  //Serial.print(texto);
  delay(50);
    
 }
