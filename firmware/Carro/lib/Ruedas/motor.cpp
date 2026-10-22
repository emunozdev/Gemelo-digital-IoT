#include "motor.h"

const int Motor::MOTOR_IZQ_DIR1 = 25; 
const int Motor::MOTOR_IZQ_DIR2 = 26; 
const int Motor::MOTOR_DER_DIR1 = 27; 
const int Motor::MOTOR_DER_DIR2 = 14; 

void Motor::inicializar() {
  pinMode(MOTOR_IZQ_DIR1, OUTPUT);
  pinMode(MOTOR_IZQ_DIR2, OUTPUT);
  pinMode(MOTOR_DER_DIR1, OUTPUT);
  pinMode(MOTOR_DER_DIR2, OUTPUT);
  
  detenerMotores();
}

void Motor::moverAdelante() {
  digitalWrite(MOTOR_IZQ_DIR1, HIGH);
  digitalWrite(MOTOR_IZQ_DIR2, LOW);
  digitalWrite(MOTOR_DER_DIR1, HIGH);
  digitalWrite(MOTOR_DER_DIR2, LOW);
}

void Motor::moverAtras() {
  digitalWrite(MOTOR_IZQ_DIR1, LOW);
  digitalWrite(MOTOR_IZQ_DIR2, HIGH);
  digitalWrite(MOTOR_DER_DIR1, LOW);
  digitalWrite(MOTOR_DER_DIR2, HIGH);
}


void Motor::girarDerecha() {
  digitalWrite(MOTOR_IZQ_DIR1, HIGH);
  digitalWrite(MOTOR_IZQ_DIR2, LOW);
  digitalWrite(MOTOR_DER_DIR1, LOW);
  digitalWrite(MOTOR_DER_DIR2, HIGH);
}

void Motor::detenerMotores() {
  digitalWrite(MOTOR_IZQ_DIR1, LOW);
  digitalWrite(MOTOR_IZQ_DIR2, LOW);
  digitalWrite(MOTOR_DER_DIR1, LOW);
  digitalWrite(MOTOR_DER_DIR2, LOW);
}