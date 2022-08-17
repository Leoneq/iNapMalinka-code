#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>

#define SEND_DATA_PERIOD    (1000) //one second
#define PIN_BTN_STOP        D6
#define PIN_SENSOR          D8
#define PIN_TEMP            D0
#define PIN_SERVO_A         D1
#define PIN_SERVO_B         D9
#define PIN_SERVO_C         D10


RF24 radio(D4, D2); // CN, CSN
OneWire oneWire(PIN_TEMP);
DallasTemperature sensors(&oneWire);
const byte address[6] = "00001";

Servo servoA;
Servo servoB;
Servo servoC;

struct receivedData
{
    uint8_t servoA_angle = 0;
    uint8_t servoB_angle = 0;
    uint8_t servoC_angle = 0;
} rData;

struct sentData
{
    float temperature = 0;
    uint8_t sensor = 0;
    uint8_t stop = 0;
} sData;

unsigned long start_time = 0;

void setup()
{
    while(1)
    {
        digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    }
    //initialize the radio
    radio.begin();
    radio.openReadingPipe(0, address);
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();

    //initialize sensors
    sensors.begin();

    //initialize servos
    servoA.attach(PIN_SERVO_A);
    servoB.attach(PIN_SERVO_B);
    servoC.attach(PIN_SERVO_C);

    //initialize GPIO
    pinMode(PIN_SENSOR, INPUT);
    pinMode(PIN_BTN_STOP, INPUT_PULLUP);
}

void loop()
{
    //receive new data if available
    if(radio.available())
    {
        radio.read(&rData, sizeof(rData));
    }

    //send new data every second
    if((millis() - start_time) >= SEND_DATA_PERIOD)
    {
        //read data
        sensors.requestTemperatures();
        sData.temperature = sensors.getTempCByIndex(0);
        sData.sensor = digitalRead(PIN_SENSOR);
        sData.stop = digitalRead(PIN_BTN_STOP);

        //send data
        radio.stopListening();
        radio.write(&sData, sizeof(sData));
        delay(10); //tactical delay
        radio.startListening();

        //restart the timer
        start_time = millis();
    }

    //use the received data
    servoA.write(rData.servoA_angle);
    servoB.write(rData.servoB_angle);
    servoC.write(rData.servoC_angle);
}
