#include "Radar.h"

Adafruit_SSD1306 Radar::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const uint8_t Radar::distanciaUmbral = 20;
unsigned long Radar::lastMotionTime = 0;
const unsigned long Radar::motionTimeout = 5000;
bool Radar::alertaActiva = false;
Sensor Radar::sensor; 

void Radar::inicializar() {
  Serial.begin(115200);

  Radar::sensor.inicializar();

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Fallo al asignar memoria para SSD1306"));
    while(true); 
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando sistema...");
  display.display();
  delay(1000);
}

void Radar::update() {
  float distancia = Radar::sensor.medirDistancia();

  if (distancia > 0 && distancia < 400) { 

    /*Enviar datos por telemetria propia */
      //Network::enviar_distancia_udp((uint16_t)distancia);
      Network::enviar_telemetria_udp((uint16_t)distancia, alertaActiva);

      if (distancia <= 15) { 
        if (!alertaActiva) {
          Serial.println("¡Objeto a menos de 15cm detectado!");
          Radar::sensor.activarAlarma(); 
          alertaActiva = true;
        }
        lastMotionTime = millis(); 

        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 20); 
        display.println("ALERTA!");
        display.setTextSize(1);
        display.setCursor(10, 45);
        display.print("Dist: ");
        display.print(distancia, 1);
        display.print(" cm");
        display.display();

      } else { 
        if (alertaActiva && (millis() - lastMotionTime > motionTimeout)) {
          Serial.println("Volviendo a escanear...");
          Radar::sensor.activarAlarma();
          alertaActiva = false;
        }
        if (!alertaActiva) {
          drawScanningRadar((uint8_t)distancia);
        }
      }
  } else {
      drawScanningRadar(0);
  }

  delay(50); 
}

void Radar::drawScanningRadar(const uint8_t distanciaActual) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("ESCANEANDO...");
  display.setCursor(0, 10);
  display.print("Actual: ");
  display.print(distanciaActual);
  display.print(" cm");

  uint8_t centerX = 96, centerY = 36, radius = 24;

  display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);
  display.drawCircle(centerX, centerY, radius / 2, SSD1306_WHITE);
  display.drawLine(centerX - radius, centerY, centerX + radius, centerY, SSD1306_WHITE);
  display.drawLine(centerX, centerY - radius, centerX, centerY + radius, SSD1306_WHITE);

  static uint16_t angleStep = 0;
  float rad = radians(angleStep);
  int8_t xEnd = centerX + (int8_t)(radius * cos(rad));
  int8_t yEnd = centerY + (int8_t)(radius * sin(rad));
  display.drawLine(centerX, centerY, xEnd, yEnd, SSD1306_WHITE);

  angleStep = (angleStep + 30) % 360;

  display.display();
}