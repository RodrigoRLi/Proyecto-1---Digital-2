/**********************************************
*Universidad del Valle de Guatemala
*Nombre: Rodrigo Fernando Recinos Lopez
*Fecha: 31/07/2026
*Curso: Electronica digital 2
*Proyecto1: sistema embebido
***********************************************/
#include <Arduino.h>
#include <stdint.h>
#include <driver/ledc.h>

#define alarma 2000
#define canalr 0
#define canalg 1
#define canalb 2
#define canalservo 3
#define freqPWM 50
#define resolutionPWM 16
//pines display
const int segA = 14;
const int segB = 12;
const int segC = 33;
const int segD = 25;
const int segE = 32;
const int segF = 27;
const int segG = 26;
const int punto = 13;
//pines transistores
const int transis1 = 4;
const int transis2 = 5;
const int transis3 = 18;
//pines led rgb
const int ledR = 22;
const int ledG = 21;
const int ledB = 19;
//pin boton de lectura
const int bt1 = 23;
//sensor
const int lm35 = 35;
//motor servo
const int servo = 15;
//arreglo displays
int digitos[3];
float temperatura = 0;
bool nuevalectura = false;
//variable activa por interrupcion
volatile byte displayActual = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

hw_timer_t *Timer1 = NULL;

void initPWM(void);
void initservo(void);
void configTimer(void);
//interrupcion boton
void IRAM_ATTR interr(){
  nuevalectura = true;
  

}

void IRAM_ATTR multiISR()
{
    portENTER_CRITICAL_ISR(&timerMux);

    digitalWrite(transis1, LOW);
    digitalWrite(transis2, LOW);
    digitalWrite(transis3, LOW);

    mostrar(digitos[displayActual]);

    switch(displayActual)
    {
        case 0:

            digitalWrite(transis1,HIGH);
            digitalWrite(punto,HIGH);

            break;

        case 1:

            digitalWrite(transis2,HIGH);
            digitalWrite(punto,LOW);

            break;

        case 2:

            digitalWrite(transis3,HIGH);
            digitalWrite(punto,LOW);

            break;
    }

    displayActual++;

    if(displayActual>2)
        displayActual=0;

    portEXIT_CRITICAL_ISR(&timerMux);
}

//obtenemos valores del sensor
void leertemp()
{
    int lectura = analogRead(lm35);

    float voltaje = lectura * 3.3 / 4095.0;

    temperatura = voltaje * 100.0;
}
//utilizamos el pulso pwm para mover el servo 
void moverservo()
{
    if(temperatura < 23)
    {
        servoPWM(1000);      // Cerrado
    }

    else if(temperatura >=23 && temperatura <27)
    {
        servoPWM(1500);      // Medio
    }

    else
    {
        servoPWM(2000);      // Abierto
    }
}
//utilizamos las lecturas del sensor para mostrar un color
void medirtemp(){
  if(temperatura < 23)
    {
        ledcWrite(canalr,0);
        ledcWrite(canalg,0); //azul
        ledcWrite(canalb,255);
    }

    else if(temperatura >=23 && temperatura <25)
    {
        ledcWrite(canalr,0);
        ledcWrite(canalg,255); //verde
        ledcWrite(canalb,0);
    }

    else if(temperatura >=25 && temperatura <27)
    {
        ledcWrite(canalr,255);
        ledcWrite(canalg,255); //amarillo
        ledcWrite(canalb,0);
    }

    else
    {
        ledcWrite(canalr,255);
        ledcWrite(canalg,0); //rojo
        ledcWrite(canalb,0);
    }
}


//obtenemos valores para mostras en cada display
void cambiosdisplay()
{
    int valor = temperatura * 10;

    digitos[0] = valor / 100;

    digitos[1] = (valor / 10) % 10;

    digitos[2] = valor % 10;
}


void setup() {
  initPWM();
  initservo();
  configTimer();

  pinMode(transis1, OUTPUT);
  pinMode(transis2, OUTPUT);
  pinMode(transis3, OUTPUT);

  pinMode(lm35, INPUT);

  digitalWrite(transis1, LOW);
  digitalWrite(transis2, LOW);
  digitalWrite(transis3, LOW);

  pinMode(segA,OUTPUT);
  pinMode(segB,OUTPUT);
  pinMode(segC,OUTPUT);
  pinMode(segD,OUTPUT);
  pinMode(segE,OUTPUT);
  pinMode(segF,OUTPUT);
  pinMode(segG,OUTPUT);

  attachInterrupt(23, &interr, RISING);

  leertemp();
  cambiosdisplay();
  moverservo();
  medirtemp();
  
}
//encendemos display segun valores recibidos
void mostrar(int data){

  
  int segmentos[7] = {
    segA, segB, segC,
    segD, segE, segF, segG
  };

  
  int numeros[10][7] = {
    {1,1,1,1,1,1,0}, //0
    {0,1,1,0,0,0,0}, //1
    {1,1,0,1,1,0,1}, //2
    {1,1,1,1,0,0,1}, //3
    {0,1,1,0,0,1,1}, //4
    {1,0,1,1,0,1,1}, //5
    {1,0,1,1,1,1,1}, //6
    {1,1,1,0,0,0,0}, //7
    {1,1,1,1,1,1,1}, //8
    {1,1,1,1,0,1,1}  //9
    
  };

  for(int i = 0; i < 7; i++){

    digitalWrite(segmentos[i], numeros[data][i]);
  }
}


void loop() {
  if (nuevalectura){
    nuevalectura = false;
    leertemp();
    medirtemp();
    moverservo();
    cambiosdisplay();


  }
  
}

//generar pulso led rgb
void initPWM(void){
    ledcSetup(canalr,5000,8);
    ledcAttachPin(ledR,canalr);

    ledcSetup(canalg,5000,8);
    ledcAttachPin(ledG,canalg);

    ledcSetup(canalb,5000,8);
    ledcAttachPin(ledB,canalb);

    ledcWrite(canalr,0);
    ledcWrite(canalg,0);
    ledcWrite(canalb,0);
  
}
//generar pulso motor servo
//identificar servo
void initservo(void){
  ledcSetup(canalservo, freqPWM, resolutionPWM);
    ledcAttachPin(servo, canalservo);
}
//mandar pulso
void servoPWM(uint16_t ancho_us)
{
    uint32_t duty = (ancho_us * 65535UL) / 20000UL;
    ledcWrite(canalservo, duty);
}
//timer displays
void configTimer()
{
    Timer1 = timerBegin(0,1000000, true);
    timerAttachInterrupt(Timer1, &multiISR, true);
    timerAlarmWrite(Timer1,alarma,true);
    timerAlarmEnable(Timer1);
}
