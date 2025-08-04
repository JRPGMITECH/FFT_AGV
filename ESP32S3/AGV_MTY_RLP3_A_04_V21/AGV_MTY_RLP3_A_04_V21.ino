#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Arduino.h>
#include <ESP32Encoder.h>
#include <Preferences.h>

Preferences prefs;

ESP32Encoder encoder;

#define ENCODER_A_PIN 9  // Canal A del encoder
#define ENCODER_B_PIN 10   // Canal B del encoder

// Configuración de PWM
#define PWM_FREQ 16000    // Frecuencia deseada: 16 kHz
#define PWM_RESOLUTION 8  // Resolución de 8 bits (valores de 0 a 255)
#define PWM_CHANNEL 0     // Canal PWM a usar (0-15)
#define PWM_PIN 33        // Pin de salida (puede ser casi cualquier GPIO)
#define PWM_PIN2 36

#define PWM_PIN3 17
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ForwardSensorPin 42   ////
#define BackwardSensorPin 41  ////

#define NestSensor1Pin 6  ////
#define NestSensor2Pin 5  ////

//#define LimitUpwardPin 18    ////
#define LimitDownwardPin 34  ////

#define ForwardButtonPin 37   ////
#define BackwardButtonPin 39  ////

#define MotorForwardPin 47   ////
#define MotorBackwardPin 21  ////

//#define MotorForwardPin2 35   ////
//#define MotorBackwardPin2 34  ////

#define MotorUpwardPin 18    ////
#define MotorDownwardPin 16  ////

#define Led1Pin 14
#define Led2Pin 11
#define Led3Pin 13

#define Led4Pin 12

#define BuzzerPin 15

#define CurrentSensor2 8  //motor DC
#define CurrentSensor 7   //actuador


int NestSensor1 = 0;
int NestSensor2 = 0;

int ForwardSensor = 0;
int BackwardSensor = 0;

int ForwardButton = 0;
int BackwardButton = 0;

int flagForward = 2;
int flagBackward = 2;

int flagStopForward = 0;
int flagStopBackward = 0;


int flagForwardButton = 1;
int flagBackwardButton = 1;


int cont = 0;
int contBuz = 0;
int contBuzo = 0;


int LimitUpward = 0;
int LimitDownward = 0;
int LimitUpF = 0;
int LimitUpB = 0;
int LimitUp = 0;

int flagLiftDownward = 2;
int flagLiftUpward = 2;

int contLift = 0;

int current = 0;
int currentM1 = 0;
long y = 0;
long yprev = 0;
int conty = 0;
long z = 0;
long zprev = 0;
int contz = 0;

int ContStop = 0;
int ContMagnet = 0;
int flagPWM = 0;
int continterrupt = 0;

int contSec = 0;

float Tao = 0;
float sig = 0;
float sigfren = 0;

int mainflag = 2;
int contmode = 0;
int contlights = 0;
int flagboth = 0;
int continter = 0;

int AluF = 2;
int AluB = 2;

int flagNest = 0;

int flagNestForward = 2;
int flagNestBackward = 2;
int flagNestBackward1 = 2;

int flagelev = 1;

int contboth = 0;
int flagON = 1;

int contpar = 0;

int16_t EncoCount = 0;
int CountR = 0;


int contLift0 = 0;
int contblink = 0;
unsigned long tInicio = 0;
//int alarma = 0;


// --------- WiFi 
#define WIFI_SSID "MitUnifiWifi"
#define WIFI_PASSWORD "85nE753fh" 

// --------- Firebase 
#define FIREBASE_HOST "https://scada-mini-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyCdA9oo-FBzovWneFryQ37N4otzOjW5Isc"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String path = "/datos_esp32/esp1/alarma";


volatile int alarma = 0;
int estado = 1;

TaskHandle_t taskCore0;
TaskHandle_t taskCore1;


void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
//-------------------------------------------------------

  prefs.begin("datos", false);
  prefs.begin("mero", false);


  // Configurar Encoder
  encoder.attachFullQuad(ENCODER_A_PIN, ENCODER_B_PIN);  // Conecta los canales A y B del encoder a GPIO 10 y 9
  encoder.setCount(0);                                   // Inicializa el contador en 0

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode(ForwardSensorPin, INPUT);
  pinMode(BackwardSensorPin, INPUT);

  pinMode(ForwardButtonPin, INPUT);
  pinMode(BackwardButtonPin, INPUT);

  pinMode(MotorForwardPin, OUTPUT);
  pinMode(MotorBackwardPin, OUTPUT);
  //pinMode(PWM_PIN, OUTPUT);

  pinMode(Led1Pin, OUTPUT);
  pinMode(Led2Pin, OUTPUT);
  pinMode(Led3Pin, OUTPUT);
  pinMode(Led4Pin, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);


  pinMode(MotorUpwardPin, OUTPUT);
  pinMode(MotorDownwardPin, OUTPUT);

  //pinMode(LimitUpwardPin, INPUT);
  pinMode(LimitDownwardPin, INPUT);

  pinMode(CurrentSensor, INPUT);
  pinMode(CurrentSensor2, INPUT);

  pinMode(NestSensor2Pin, INPUT);
  pinMode(NestSensor1Pin, INPUT);


  digitalWrite(MotorForwardPin, LOW);
  digitalWrite(MotorBackwardPin, LOW);

  ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
  ledcWrite(PWM_PIN, 0);

  digitalWrite(MotorUpwardPin, LOW);
  digitalWrite(MotorDownwardPin, LOW);

  digitalWrite(Led1Pin, LOW);   //green
  digitalWrite(Led2Pin, LOW);   //yellow
  digitalWrite(Led3Pin, HIGH);  //red

  digitalWrite(Led4Pin, LOW);  //blue

  digitalWrite(BuzzerPin, LOW);
  //digitalWrite(PWM_PIN3, LOW);
  
  ledcAttach(PWM_PIN3, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
  ledcWrite(PWM_PIN3, 0);
  

  //--------------- Firebase Config----------------------
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Firebase.setReadTimeout(fbdo, 1000 * 10); // 10 segundos de timeout
  Firebase.setwriteSizeLimit(fbdo, "small"); // Tamaño de escritura pequeño
  
  xTaskCreatePinnedToCore(TaskCore0, "FirebaseCore", 12000, NULL, 1, &taskCore0, 0);
  xTaskCreatePinnedToCore(TaskCore1, "MachineCore", 10000, NULL, 1, &taskCore1, 1);

  delay(1500);
}

void loop() {
  
}

// --------- CORE 0
void TaskCore0(void *pvParameters) {
  for (;;) {
    if (Firebase.ready()) {
      Firebase.setInt(fbdo, path, alarma);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // cada 1s
  }
}

// --------- CORE 1
void TaskCore1(void *pvParameters) {
  for (;;) {
   ConfigMode();
   FORWARDBUTTON();
   BACKWARDBUTTON();
   LIFTDOWNWARD();
   FORWARD();
   BACKWARD();
   StopForward();
   StopBackward();
   LIFTUPWARD();
   STOPFORWARD();
   STOPBACKWARD();

  vTaskDelay(2);
  }
}


void ConfigMode() {
  contmode = 0;
  ForwardButton = digitalRead(ForwardButtonPin);
  BackwardButton = digitalRead(BackwardButtonPin);

  while (ForwardButton == 0 && BackwardButton == 0 && (flagboth == 1 || flagboth == 2)) {

    unsigned long tInicio = micros();

    ForwardButton = digitalRead(ForwardButtonPin);
    BackwardButton = digitalRead(BackwardButtonPin);

    digitalWrite(Led1Pin, HIGH);  //green
    digitalWrite(Led2Pin, HIGH);  //yellow
    digitalWrite(Led3Pin, HIGH);  //red

    contmode = contmode + 1;

    while (contmode >= 2000 && contmode <= 3000) {

      unsigned long tInicio = micros();

      contmode++;
      contblink++;

      if (contblink <= 400) {
        digitalWrite(Led1Pin, HIGH);  //green
        digitalWrite(Led2Pin, HIGH);  //yellow
        digitalWrite(Led3Pin, HIGH);  //red
        digitalWrite(BuzzerPin, HIGH);
      }

      if (contblink > 400 && contblink <= 800) {
        digitalWrite(Led1Pin, LOW);  //green
        digitalWrite(Led2Pin, LOW);  //yellow
        digitalWrite(Led3Pin, LOW);  //red
        digitalWrite(BuzzerPin, LOW);

        contblink = 0;
      }

      while (micros() - tInicio < 500) {  ////delay 500 us
        delayMicroseconds(10);
      }
    }

    if (contmode > 3000) {

      digitalWrite(Led1Pin, LOW);   //green
      digitalWrite(Led2Pin, LOW);   //yellow
      digitalWrite(Led3Pin, HIGH);  //red

      digitalWrite(BuzzerPin, LOW);

      if (flagboth == 1) {  // primera doble picada para entrar a modo

        mainflag = 3;

        flagLiftDownward = 1;
        LIFTDOWNWARD();

        contmode = 0;
        flagboth = 0;
        flagON = 0;    //condicion de prendido vale 1 inicializando pero aqui la mato para hacer rutina de ajuste de alturas
        flagNest = 1;  //permite grabar aluf y alub
      }

      if (flagboth == 2) {  // segunda doble picada para permitir grabar altura

        mainflag = 2;
        flagelev = 0;
        contmode = 0;
        flagboth = 0;

        flagForwardButton = 1;
        flagBackwardButton = 1;

        contboth = contboth + 1;

        if (contboth == 1) {

          prefs.begin("datos", false);
          prefs.putInt("LimitUpF", 0);
          prefs.putInt("LimitUpB", 0);
          prefs.putInt("AluF", 0);
          prefs.putInt("AluB", 0);
          prefs.putInt("flagNestForward", 0);
          prefs.end();  //grabar
        }

        if (contboth == 2) {  // tercera doble picada para permitir guardar altura
          mainflag = 2;
          flagelev = 1;
          flagON = 1;
          contmode = 0;
          contblink = 0;
          contboth = 0;
          flagForwardButton = 1;
          flagBackwardButton = 1;
          flagboth = 0;
        }
      }
      break;
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }


  while (mainflag == 3 && ForwardButton == 0) {  /////////////////////////////////////////////////Lectura boton subir

    unsigned long tInicio = micros();

    ForwardButton = digitalRead(ForwardButtonPin);
    BackwardButton = digitalRead(BackwardButtonPin);
    EncoCount = encoder.getCount();  //lee encoder

    digitalWrite(MotorUpwardPin, LOW);
    digitalWrite(MotorDownwardPin, HIGH);

    if (ForwardButton == 1) {
      CountR = EncoCount;
      digitalWrite(MotorUpwardPin, LOW);
      digitalWrite(MotorDownwardPin, LOW);
      break;
    }

    continter = 0;

    while (BackwardButton == 0 && continter < 100) {  //detecta que el otro boton se esta aplastando para activar el modo de servicio

      unsigned long tInicio = micros();

      BackwardButton = digitalRead(BackwardButtonPin);

      digitalWrite(MotorUpwardPin, LOW);
      digitalWrite(MotorDownwardPin, LOW);

      continter = continter + 1;

      if (continter >= 100) {
        flagboth = 2;
        mainflag = 1;
        continter = 0;

        break;
      }

      while (micros() - tInicio < 500) {  ////delay 500 us
        delayMicroseconds(10);
      }
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }

  while (mainflag == 3 && BackwardButton == 0) {  /////////////////////////////////Lectura boton bajar

    unsigned long tInicio = micros();

    BackwardButton = digitalRead(BackwardButtonPin);
    ForwardButton = digitalRead(ForwardButtonPin);
    EncoCount = encoder.getCount();  //lee encoder

    digitalWrite(MotorUpwardPin, HIGH);  //baja el elevador
    digitalWrite(MotorDownwardPin, LOW);

    if (BackwardButton == 1) {
      CountR = EncoCount;
      digitalWrite(MotorUpwardPin, LOW);
      digitalWrite(MotorDownwardPin, LOW);
      break;
    }

    continter = 0;

    while (ForwardButton == 0 && continter < 100) {  //detecta que el otro boton se esta alastando para activar el modo de servicio

      unsigned long tInicio = micros();

      ForwardButton = digitalRead(ForwardButtonPin);

      digitalWrite(MotorUpwardPin, LOW);
      digitalWrite(MotorDownwardPin, LOW);

      continter = continter + 1;

      if (continter >= 100) {
        flagboth = 2;
        mainflag = 1;
        continter = 0;
        break;
      }

      while (micros() - tInicio < 500) {  ////delay 500 us
        delayMicroseconds(10);
      }
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void FORWARDBUTTON() {

  ForwardButton = digitalRead(ForwardButtonPin);
  NestSensor2 = digitalRead(NestSensor2Pin);

  cont = 0;

  while ((NestSensor2 == AluF || ForwardButton == 0) && cont <= 2000 && flagForwardButton == 1 && mainflag == 2) {  ///El usuario debe dejar presionado unos milisegundos el boton para arrancar hacie al frent

    unsigned long tInicio = micros();

    BackwardButton = digitalRead(BackwardButtonPin);
    ForwardButton = digitalRead(ForwardButtonPin);
    NestSensor2 = digitalRead(NestSensor2Pin);

    digitalWrite(Led1Pin, HIGH);  //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, LOW);   //red
    digitalWrite(BuzzerPin, LOW);

    cont = cont + 1;

    if (NestSensor2 == AluB || ForwardButton == 1) {
      digitalWrite(Led1Pin, LOW);   //green
      digitalWrite(Led2Pin, LOW);   //yellow
      digitalWrite(Led3Pin, HIGH);  //red
    }

    flagBackwardButton = 0;
    continter = 0;

    while (BackwardButton == 0 && continter < 100) {  //detecta que el otro boton se esta alastando para activar el modo de servicio

      unsigned long tInicio = micros();

      BackwardButton = digitalRead(BackwardButtonPin);
      continter = continter + 1;

      if (continter >= 100) {
        flagboth = 1;
        mainflag = 1;
        cont = 0;
        break;
      }

      while (micros() - tInicio < 500) {  ////delay 500 us
        delayMicroseconds(10);
      }
    }

    if (cont >= 2000) {

      flagForwardButton = 0;  //deshabilita bandera de este loop para  no quiera picarle cuando avanze

      flagForward = 1;      //habilita bandera para avanzar al frente
      flagStopForward = 1;  // habiita bandera para que pueda detenerse con el sensor
      flagLiftDownward = 1;
      cont = 0;
      mainflag = 0;

      if (NestSensor2 == 1 && flagNest == 1) {  /////////////////////////////////////////////////////////////////NO hay nido

        unsigned long tInicio = micros();

        AluF = 1;
        AluB = 0;

        flagNestForward = 1;
        flagNestBackward = 0;

        while (micros() - tInicio < 10000) {  ////delay 10 ms
          delayMicroseconds(10);
        }

        prefs.begin("datos", false);
        prefs.putInt("AluF", AluF);
        prefs.putInt("AluB", AluB);
        prefs.putInt("LimitUpF", CountR);
        prefs.putInt("flagNestForward", flagNestForward);
        prefs.end();  //grabar
      }

      if (NestSensor2 == 0 && flagNest == 1) {  /////////////////////////////////////////////////////////////hay nido

        unsigned long tInicio = micros();

        AluF = 0;
        AluB = 1;

        flagNestForward = 0;
        flagNestBackward = 1;

        while (micros() - tInicio < 20) {  ////delay 10 ms
          delayMicroseconds(10);
        }

        prefs.begin("datos", false);
        prefs.putInt("AluF", AluF);
        prefs.putInt("AluB", AluB);
        prefs.putInt("LimitUpB", CountR);
        prefs.putInt("flagNestForward", flagNestForward);
        prefs.end();  //grabar
      }

      if (flagON == 1) {

        unsigned long tInicio = micros();
        while (micros() - tInicio < 20) {  ////delay 10 ms
          delayMicroseconds(10);
        }

        prefs.begin("datos", true);
        AluF = prefs.getInt("AluF", 0);
        AluB = prefs.getInt("AluB", 0);
        LimitUpF = prefs.getInt("LimitUpF", 0);
        LimitUpB = prefs.getInt("LimitUpB", 0);
        flagNestForward = prefs.getInt("flagNestForward", 0);
        prefs.end();

        flagNest = 0;

        if (flagNestForward == 0) {
          flagNestBackward = 1;
        } else {
          flagNestBackward = 0;
        }
      }

      if (flagON == 1 && flagNestForward == 1 && flagNestBackward == 0) {  // estoy en este boton debo tomar toma   x = #AlturaForward y = #AlturaBackward

        flagON = 3;
      }

      if (flagON == 1 && flagNestForward == 0 && flagNestBackward == 1) {  // estoy en este boton debo tomar toma   x = #AlturaBackward y = #AlturaForward

        flagON = 2;
      }

      break;
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void BACKWARDBUTTON() {

  BackwardButton = digitalRead(BackwardButtonPin);
  NestSensor2 = digitalRead(NestSensor2Pin);

  cont = 0;

  while ((NestSensor2 == AluB || BackwardButton == 0) && cont <= 20 && flagBackwardButton == 1 && mainflag == 2) {  //el usuario debe presionar el boton unos milisegundos antes de arrancar hacia atras

    unsigned long tInicio = micros();

    BackwardButton = digitalRead(BackwardButtonPin);
    ForwardButton = digitalRead(ForwardButtonPin);
    NestSensor2 = digitalRead(NestSensor2Pin);

    digitalWrite(Led1Pin, HIGH);  //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, LOW);   //red
    digitalWrite(BuzzerPin, LOW);

    cont = cont + 1;

    if (NestSensor2 == AluF || BackwardButton == 1) {
      digitalWrite(Led1Pin, LOW);   //green
      digitalWrite(Led2Pin, LOW);   //yellow
      digitalWrite(Led3Pin, HIGH);  //red
    }

    flagForwardButton = 0;  //mantiene deshabilitado el loop del otro boton por si  quiere hacer mamadas mientras presionas este boton
    continter = 0;

    while (ForwardButton == 0 && continter < 100) {  //detecta que el otro boton se esta alastando para activar el modo de servicio

      unsigned long tInicio = micros();

      ForwardButton = digitalRead(ForwardButtonPin);
      continter = continter + 1;

      if (continter >= 100) {
        flagboth = 1;
        mainflag = 1;
        cont = 0;
        break;
      }

      while (micros() - tInicio < 500) {  ////delay 500 us
        delayMicroseconds(10);
      }
    }

    if (cont >= 2000) {

      flagBackwardButton = 0;  //deshabilita bandera de este loop para que  no quiera picarle cuando avanze

      flagBackward = 1;      //activa bandera de ir hacia atras
      flagStopBackward = 1;  //activa bandera de detenerse
      flagLiftDownward = 1;
      cont = 0;
      mainflag = 0;

      if (NestSensor2 == 1 && flagNest == 1) {  ////////////////////////////////////////////////////////////////////////no hay nido

        unsigned long tInicio = micros();

        AluF = 0;
        AluB = 1;

        flagNestForward = 0;
        flagNestBackward = 1;

        while (micros() - tInicio < 10000) {  ////delay 10 ms
          delayMicroseconds(10);
        }

        prefs.begin("datos", false);
        prefs.putInt("AluF", AluF);
        prefs.putInt("AluB", AluB);
        prefs.putInt("LimitUpF", CountR);
        prefs.putInt("flagNestForward", flagNestForward);
        prefs.end();  //grabar
      }

      if (NestSensor2 == 0 && flagNest == 1) {  /////////////////////////////////////////////////////////////////////////////////hay nido

        unsigned long tInicio = micros();

        AluF = 1;
        AluB = 0;

        flagNestForward = 1;
        flagNestBackward = 0;

        while (micros() - tInicio < 10000) {  ////delay 10 ms
          delayMicroseconds(10);
        }

        prefs.begin("datos", false);
        prefs.putInt("AluF", AluF);
        prefs.putInt("AluB", AluB);
        prefs.putInt("LimitUpB", CountR);
        prefs.putInt("flagNestForward", flagNestForward);
        prefs.end();  //grabar
      }

      if (flagON == 1) {

        unsigned long tInicio = micros();

        while (micros() - tInicio < 10000) {  ////delay 10 ms
          delayMicroseconds(10);
        }

        prefs.begin("datos", true);
        AluF = prefs.getInt("AluF", 0);
        AluB = prefs.getInt("AluB", 0);
        LimitUpF = prefs.getInt("LimitUpF", 0);
        LimitUpB = prefs.getInt("LimitUpB", 0);
        flagNestForward = prefs.getInt("flagNestForward", 0);
        prefs.end();

        flagNest = 0;

        if (flagNestForward == 0) {
          flagNestBackward = 1;
        } else {
          flagNestBackward = 0;
        }
      }

      if (flagON == 1 && flagNestBackward == 1 && flagNestForward == 0) {  // estoy en este boton debo tomar toma    x = #AlturaBackward y = #AlturaForward

        flagON = 3;
      }

      if (flagON == 1 && flagNestBackward == 0 && flagNestForward == 1) {  // estoy en este boton debo tomar toma   x = #AlturaForward y = #AlturaBackward

        flagON = 2;
      }
      break;
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void LIFTDOWNWARD() {

  LimitDownward = digitalRead(LimitDownwardPin);
  contLift0 = 0;  //contador para alarma()

  while (LimitDownward == 1 && flagLiftDownward == 1) {

    unsigned long tInicio = micros();

    LimitDownward = digitalRead(LimitDownwardPin);

    digitalWrite(MotorUpwardPin, LOW);  //baja el elevador
    digitalWrite(MotorDownwardPin, HIGH);

    digitalWrite(Led1Pin, LOW);  //green
    //digitalWrite(Led2Pin, LOW);//yellow
    digitalWrite(Led3Pin, LOW);  //red

    contBuzo = contBuzo + 1;

    if (contBuzo <= 200) {  // buzzer
      digitalWrite(BuzzerPin, HIGH);
      digitalWrite(Led2Pin, HIGH);  //yellow
    }

    if (contBuzo > 200) {
      digitalWrite(BuzzerPin, LOW);
      digitalWrite(Led2Pin, LOW);  //yellow

      if (contBuzo >= 2000) {
        contBuzo = 0;
        contLift0++;

        if (contLift0 >= 20) { alarmaLift(); }  //
      }
    }

    contLift = 0;

    while (LimitDownward == 0 && contLift <= 2000) {

      unsigned long tInicio = micros();

      LimitDownward = digitalRead(LimitDownwardPin);

      contBuzo = 0;
      digitalWrite(BuzzerPin, LOW);
      digitalWrite(Led2Pin, HIGH);

      contLift = contLift + 1;

      if (contLift > 2000) {

        unsigned long tInicio = micros();

        digitalWrite(MotorUpwardPin, LOW);  //se detiene el elevador
        digitalWrite(MotorDownwardPin, LOW);

        while (micros() - tInicio < 10000) {  ////delay 10 ms
          delayMicroseconds(10);
        }

        encoder.clearCount();  //reinicia encoder
        EncoCount = 0;
        flagLiftDownward = 0;
        flagPWM = 0;
        break;
      }

      while (micros() - tInicio < 500) {  ////delay 500 us
        delayMicroseconds(10);
      }
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void FORWARD() {

  while (flagForward == 1) {  // avanza al frente

    LIFTDOWNWARD();

    unsigned long tInicio = micros();

    digitalWrite(Led1Pin, HIGH);  //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, LOW);   //red

    digitalWrite(MotorForwardPin, HIGH);  //Direccion de giro en puente H
    digitalWrite(MotorBackwardPin, LOW);

    if (flagPWM == 0) {
      zSigmoid();
      flagPWM = 1;
    }

    if (flagPWM == 1) {
      ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
      ledcWrite(PWM_PIN, 250);
    }

    if (flagPWM == 2) {

      digitalWrite(BuzzerPin, LOW);

      zSigmoid_Inv();

      ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
      ledcWrite(PWM_PIN, 80);

      flagPWM = 3;
      flagStopForward = 1;
    }

    StopForward();

    contBuzo = contBuzo + 1;

    if (contBuzo <= 200) {  // buzzer
      digitalWrite(BuzzerPin, HIGH);
    }

    if (contBuzo > 200) {
      digitalWrite(BuzzerPin, LOW);

      if (contBuzo == 2000) {
        contSec = contSec + 1;
        contBuzo = 0;
      }
    }

    if (contSec == 3) { flagStopForward = 1; }

    if (contSec >= 20) { alarmaFB(); }

    currentM1 = analogRead(CurrentSensor2);
    z = (0.8 * currentM1) + (0.2 * zprev);
    zprev = z;

    contz = 0;

    while (z >= 3500 && contz <= 100) {  // Limite de corriente del actuador

      unsigned long tInicio = micros();

      currentM1 = analogRead(CurrentSensor2);
      z = (0.8 * currentM1) + (0.2 * zprev);
      zprev = z;

      contz = contz + 1;

      if (contz > 100) { alarmaFB(); }

      while (micros() - tInicio < 500) {  ////delay 500 us
        delayMicroseconds(10);
      }
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void BACKWARD() {

  while (flagBackward == 1) {  // avanza hacia atras

    LIFTDOWNWARD();

    unsigned long tInicio = micros();

    digitalWrite(Led1Pin, HIGH);  //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, LOW);   //red

    digitalWrite(MotorForwardPin, LOW);
    digitalWrite(MotorBackwardPin, HIGH);


    if (flagPWM == 0) {
      zSigmoid();
      flagPWM = 1;
    }

    if (flagPWM == 1) {
      ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
      ledcWrite(PWM_PIN, 250);
    }

    if (flagPWM == 2) {

      digitalWrite(BuzzerPin, LOW);

      digitalWrite(MotorForwardPin, LOW);
      digitalWrite(MotorBackwardPin, HIGH);
      zSigmoid_Inv();

      ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
      ledcWrite(PWM_PIN, 80);

      flagPWM = 3;
      flagStopBackward = 1;
    }

    StopBackward();

    contBuzo = contBuzo + 1;

    if (contBuzo <= 200) {  // buzzer
      digitalWrite(BuzzerPin, HIGH);
    }

    if (contBuzo > 200) {
      digitalWrite(BuzzerPin, LOW);

      if (contBuzo == 2000) {
        contSec = contSec + 1;
        contBuzo = 0;
      }
    }

    if (contSec == 3) { flagStopBackward = 1; }
    if (contSec >= 20) { alarmaFB(); }

    currentM1 = analogRead(CurrentSensor2);
    z = (0.8 * currentM1) + (0.2 * zprev);
    zprev = z;

    contz = 0;

    while (z >= 3500 && contz <= 100) {  // Limite de corriente del actuador

      unsigned long tInicio = micros();

      currentM1 = analogRead(CurrentSensor2);
      z = (0.8 * currentM1) + (0.2 * zprev);
      zprev = z;

      contz = contz + 1;

      if (contz > 100) { alarmaFB(); }

      while (micros() - tInicio < 500) {  ////delay 500 us
        delayMicroseconds(10);
      }
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void StopForward() {  // interrupcion de sensor magnetico

  ForwardSensor = digitalRead(ForwardSensorPin);
  ContStop = 0;

  while (ForwardSensor == 0 && flagStopForward == 1 && ContStop < 3) {

    unsigned long tInicio = micros();

    ForwardSensor = digitalRead(ForwardSensorPin);

    ContStop = ContStop + 1;

    if (ContStop == 3) {

      ContMagnet = ContMagnet + 1;

      if (ContMagnet == 1) {

        flagStopForward = 0;
        break;
      }

      if (ContMagnet == 2) {

        flagPWM = 2;
        flagStopForward = 0;
        break;
      }

      if (ContMagnet == 3) {

        flagForward = 0;  //desabilita rutina FORWARD
        flagLiftUpward = 1;
        flagStopForward = 2;  //habilita rutina STOPFORWARD
        ContMagnet = 0;
        ContStop = 0;
        contSec = 0;
        digitalWrite(Led4Pin, HIGH);  //blue
        break;
      }
    }

    while (micros() - tInicio < 500) {  ////delay 500 us
      delayMicroseconds(10);
    }
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void StopBackward() {  // interrupcion de sensor magnetico

  BackwardSensor = digitalRead(BackwardSensorPin);
  ContStop = 0;

  while (BackwardSensor == 0 && flagStopBackward == 1 && ContStop < 3) {

    unsigned long tInicio = micros();

    BackwardSensor = digitalRead(BackwardSensorPin);

    ContStop = ContStop + 1;

    if (ContStop == 3) {

      ContMagnet = ContMagnet + 1;

      if (ContMagnet == 1) {

        flagStopBackward = 0;
        break;
      }

      if (ContMagnet == 2) {

        flagPWM = 2;
        flagStopBackward = 0;
        break;
      }

      if (ContMagnet == 3) {

        flagBackward = 0;  //desabilita rutina
        flagLiftUpward = 1;
        flagStopBackward = 2;  //habilita rutina
        ContMagnet = 0;
        ContStop = 0;
        contSec = 0;
        digitalWrite(Led4Pin, HIGH);  //blue
        break;
      }
    }

    while (micros() - tInicio < 500) {  ////////////////////////////////////////////////////////////delay 500 us
      delayMicroseconds(10);
    }
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void LIFTUPWARD() {

  current = analogRead(CurrentSensor);
  contLift0 = 0;

  while (EncoCount <= LimitUp && flagLiftUpward == 1) {

    unsigned long tInicio = micros();

    digitalWrite(MotorUpwardPin, HIGH);  //sube el elevador
    digitalWrite(MotorDownwardPin, LOW);

    digitalWrite(Led1Pin, LOW);  //green
    //digitalWrite(Led2Pin, LOW);//yellow
    digitalWrite(Led3Pin, LOW);  //red

    contBuzo = contBuzo + 1;

    if (contBuzo <= 200) {  // buzzer
      digitalWrite(BuzzerPin, HIGH);
      digitalWrite(Led2Pin, HIGH);  //yellow
    }

    if (contBuzo > 200) {
      digitalWrite(BuzzerPin, LOW);
      digitalWrite(Led2Pin, LOW);  //yellow

      if (contBuzo >= 1000) {
        contBuzo = 0;
        contLift0++;

        if (contLift0 >= 20) { alarmaLift(); }
      }
    }

    current = analogRead(CurrentSensor);
    y = (0.8 * current) + (0.2 * yprev);
    yprev = y;

    conty = 0;

    while (y >= 4000 && conty <= 100) {  // Limite de corriente del actuador

      unsigned long tInicio = micros();

      current = analogRead(CurrentSensor);
      y = (0.8 * current) + (0.2 * yprev);
      yprev = y;

      conty = conty + 1;

      if (conty > 100) { alarmaLiftUpward(); }

      while (micros() - tInicio < 500) {  ////////////////////////////////////////////////////////delay 500 us
        delayMicroseconds(10);
      }
    }

    EncoCount = encoder.getCount();

    if (EncoCount >= LimitUp) {

      digitalWrite(MotorUpwardPin, LOW);  //se detiene el elevador
      digitalWrite(MotorDownwardPin, LOW);

      contBuzo = 0;
      digitalWrite(BuzzerPin, LOW);
      digitalWrite(Led2Pin, HIGH);

      flagLiftUpward = 0;
      flagPWM = 0;
      break;
    }

    while (micros() - tInicio < 500) {  ////////////////////////////////////////////////////////delay 500 us
      delayMicroseconds(10);
    }
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void STOPFORWARD() {

  if (flagStopForward == 2) {

    unsigned long tInicio = micros();

    digitalWrite(BuzzerPin, LOW);

    digitalWrite(MotorForwardPin, LOW);
    digitalWrite(MotorBackwardPin, LOW);

    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
    ledcWrite(PWM_PIN, 0);

    while (micros() - tInicio < 500000) {  ////////////////////////////////////////////////////////delay 500 ms
      delayMicroseconds(10);
    }

    if (flagelev == 0) {
      mainflag = 3;
    }

    if (flagelev == 1) {

      mainflag = 2;

      if (flagON == 2) {
        contpar = 1;
        flagON = 0;
      }

      if (flagON == 3) {
        contpar = 0;
        flagON = 0;
      }

      contpar = contpar + 1;

      if (contpar % 2 == 0) {  //es par

        LimitUp = LimitUpF;  //entra set para el final

      } else {               //es impar
        LimitUp = LimitUpB;  //entra set para el inicio
      }

      LIFTUPWARD();  // sube el elevador  //LimitUp es el setup dela altura
    }

    digitalWrite(Led1Pin, LOW);   //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, HIGH);  //red

    flagStopForward = 0;     //desabilita rutina STOPFORWARD
    flagBackwardButton = 1;  //habilita el boton para backward
    flagPWM = 0;
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void STOPBACKWARD() {

  if (flagStopBackward == 2) {

    unsigned long tInicio = micros();

    digitalWrite(BuzzerPin, LOW);

    digitalWrite(MotorForwardPin, LOW);
    digitalWrite(MotorBackwardPin, LOW);

    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
    ledcWrite(PWM_PIN, 0);


    while (micros() - tInicio < 500000) {  ////////////////////////////////////////////////////////delay 500 ms
      delayMicroseconds(10);
    }

    if (flagelev == 0) {
      mainflag = 3;
    }

    if (flagelev == 1) {

      mainflag = 2;

      if (flagON == 2) {
        contpar = 1;
        flagON = 0;
      }

      if (flagON == 3) {
        contpar = 0;
        flagON = 0;
      }

      contpar = contpar + 1;

      if (contpar % 2 == 0) {  //es par

        LimitUp = LimitUpF;  ///entra altura para el final
      } else {               //es impar
        LimitUp = LimitUpB;  //entra altura para el inicio
      }

      LIFTUPWARD();  // sube el elevador //LimitUp es el setup d ela altura
    }

    digitalWrite(Led1Pin, LOW);   //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, HIGH);  //red

    flagStopBackward = 0;   //desabilita rutina STOPBACKWARD
    flagForwardButton = 1;  //habilita el boton para forward
    flagPWM = 0;
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void zSigmoid() {
  //sigmoid de 0 a 80
  while (Tao <= .5) {

    unsigned long tInicio = micros();

    sig = 80 * (1 / (1 + exp(-(Tao - 0.25) * 25)));
    Tao = Tao + 0.01;

    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
    ledcWrite(PWM_PIN, int(sig));

    while (micros() - tInicio < 10000) {  ////////////////////////////////////////////////////////delay 10 ms
      delayMicroseconds(10);
    }
  }

  Tao = 0;
  sig = 0;

  //sigmoid de 80 a 250
  while (Tao <= 1) {

    unsigned long tInicio = micros();

    sig = 80 + (170 * (1 / (1 + exp(-(Tao - .5) * 12))));
    Tao = Tao + 0.01;

    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
    ledcWrite(PWM_PIN, int(sig));

    while (micros() - tInicio < 10000) {  ////////////////////////////////////////////////////////delay 10 ms
      delayMicroseconds(10);
    }
  }

  Tao = 0;
  sig = 0;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void zSigmoid_Inv() {  //sigmoid de 250 a 80

  while (Tao <= .5) {

    unsigned long tInicio = micros();

    sig = 250 - (170 * (1 / (1 + exp(-(Tao - .25) * 20))));
    Tao = Tao + 0.01;

    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
    ledcWrite(PWM_PIN, int(sig));

    /// sigmoid para la resistencia
    if (Tao <= .35) {
      sigfren = 10 + (100 * (1 / (1 + exp(-(Tao - .12) * 45))));

      ledcAttach(PWM_PIN3, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
      ledcWrite(PWM_PIN3, int(sigfren));
    } else {
      ledcAttach(PWM_PIN3, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
      ledcWrite(PWM_PIN3, 0);
    }

    while (micros() - tInicio < 10000) {  ////////////////////////////////////////////////////////delay 10 ms
      delayMicroseconds(10);
    }
  }

  Tao = 0;
  sig = 0;
  sigfren = 0;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void alarmaLift() {

  alarma = 1;  //se manda alarma en firebase en el core 0

  while (micros() - tInicio < 500000) {  ////////////////////////////////////////////////////////delay 500 ms
    delayMicroseconds(10);
  }

  while (1) {                     //aqui rompo todo para obligar a que reinicien
    digitalWrite(Led1Pin, LOW);   //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, HIGH);  //red

    digitalWrite(BuzzerPin, HIGH);

    digitalWrite(MotorUpwardPin, LOW);
    digitalWrite(MotorDownwardPin, LOW);
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void alarmaLiftUpward() {

  alarma = 2;  //se manda alarma en firebase en el core 0

  digitalWrite(MotorUpwardPin, LOW);  //baja el elevador por 500 ms por el delay que sigue
  digitalWrite(MotorDownwardPin, HIGH);

  while (micros() - tInicio < 500000) {  ////////////////////////////////////////////////////////delay 500 ms
    delayMicroseconds(10);
  }

  while (1) {                     //aqui rompo todo para obligar a que reinicien
    digitalWrite(Led1Pin, LOW);   //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, HIGH);  //red

    digitalWrite(BuzzerPin, HIGH);

    digitalWrite(MotorUpwardPin, LOW);
    digitalWrite(MotorDownwardPin, LOW);
  }
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void alarmaFB() {

  alarma = 3;  //se manda alarma en firebase en el core 0

  while (micros() - tInicio < 500000) {  ////////////////////////////////////////////////////////delay 500 ms
    delayMicroseconds(10);
  }

  while (1) {

    zSigmoid_Inv();

    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RESOLUTION);  // GPIO 33 16kHz, resolución de 8 bits
    ledcWrite(PWM_PIN, 0);

    digitalWrite(MotorForwardPin, LOW);
    digitalWrite(MotorBackwardPin, LOW);

    digitalWrite(BuzzerPin, HIGH);

    digitalWrite(Led1Pin, LOW);   //green
    digitalWrite(Led2Pin, LOW);   //yellow
    digitalWrite(Led3Pin, HIGH);  //red
  }
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

