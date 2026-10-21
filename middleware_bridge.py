import json
import socket

IP_LOCAL = "0.0.0.0"  
PUERTO_ESP32 = 4242  

IP_GODOT = "127.0.0.1"
PUERTO_GODOT = 4243

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((IP_LOCAL, PUERTO_ESP32))

print(f"--- Puerto del ESP32: {PUERTO_ESP32} ---")

try:
  while True:
    data, addr = sock.recvfrom(1024)
    mensaje_crudo = data.decode("utf-8").strip()

    try:
      distancia = int(mensaje_crudo)

      paquete_json = json.dumps({
          "dispositivo": "radar_esp32",
          "medicion": {"distancia_cm": distancia, "unidad": "cm"},
          "alerta": True if distancia <= 20 else False,
      })

      print(f"[{addr[0]}] Distancia: {distancia} cm")
      print(f"JSON estructurado -> {paquete_json}\n")

      
      #sock.sendto(paquete_json.encode('utf-8'), (IP_GODOT, PUERTO_GODOT))

    except ValueError:
      print(f"Advertencia: Dato no numérico recibido -> {mensaje_crudo}")

except KeyboardInterrupt:
  print("\nIntermediario detenido por el usuario.")
  sock.close()