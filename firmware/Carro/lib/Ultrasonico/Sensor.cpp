#include "Sensor.h"

const int Sensor::triggerPin = 18;
const int Sensor::echoPin = 5;
const int Sensor::maxDistance = 15; 
const int Sensor::buzzerPin = 19;

void Sensor::inicializar() {
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
}

float Sensor::medirDistancia() {
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    unsigned long duration = pulseIn(echoPin, HIGH, 30000); 

    if(duration == 0) return -1.0f; 
    
    float distance = duration / 58.0f;

    if(distance <= 0) return -1.0f; 
    return distance;
}

bool Sensor::detectarObstaculo() {
    float distancia = medirDistancia();
    return distancia > 0 && distancia <= Sensor::maxDistance;
}

void Sensor::activarAlarma() {
    digitalWrite(buzzerPin, HIGH);
    delay(1000);
    digitalWrite(buzzerPin, LOW);
}

void Sensor::envioDato(float distancia) {
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
}