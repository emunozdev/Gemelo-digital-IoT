"""Puente entre el ESP32 del carro y el gemelo digital de Godot.

Recibe por UDP la distancia del sensor ultrasónico (puerto 4242), la convierte en
un paquete de telemetría estructurado y lo reenvía a Godot (puerto 4243), donde
`motor/ReceptorUdp.gd` lo vuelca en el autoload `Telemetria`.

El ESP32 de hoy solo mide distancia, así que la posición del carro en el mapa la
aporta una "fuente de pose". Cuando montes un GPS o un IMU, escribe otra clase
con el mismo `siguiente(paso_s, distancia_cm, hay_senal)` y regístrala en FUENTES.

Contrato JSON que consume Godot:
    {
      "dispositivo": "radar_esp32",
      "fuente": "circuito" | "manual",
      "t": 1730000000.123,
      "pose": {"x": 12.4, "y": 8.1, "rumbo": 90.0},   # metros y grados (0 = norte)
      "velocidad": 1.8,                               # m/s
      "medicion": {"distancia_cm": 34.0, "unidad": "cm"},
      "alerta": true
    }

Uso:
    python middleware_bridge.py                 # óvalo simulado, 20 Hz hacia Godot
    python middleware_bridge.py --fuente manual # la pose se escribe por terminal
    python middleware_bridge.py --verbose       # imprime cada paquete JSON
"""

import argparse
import json
import math
import socket
import sys
import threading
import time
from collections import namedtuple

IP_ESP32 = "0.0.0.0"
PUERTO_ESP32 = 4242
IP_GODOT = "127.0.0.1"
PUERTO_GODOT = 4243
UMBRAL_ALERTA_CM = 20
RETRASO_SIN_SENAL_S = 1.5

Lectura = namedtuple("Lectura", "x y rumbo velocidad distancia_cm simulada",
                     defaults=(None, False))


class FuenteCircuito:
    """Camioneta virtual que recorre el mismo óvalo de 24x16 m que el mapa.

    La velocidad la manda el radar: al acercarse a un obstáculo el gemelo frena.
    Si no llega señal del ESP32 se publica una distancia simulada, para que la
    maqueta siga siendo demostrable sin hardware.
    """

    nombre = "circuito"
    centro = (12.0, 8.0)
    semiejes = (8.0, 5.0)
    velocidad_maxima = 4.0
    demo_min = 18.0
    demo_max = 90.0

    def __init__(self):
        self._angulo = 0.0

    def siguiente(self, paso_s, distancia_cm, hay_senal):
        simulada = distancia_cm is None or not hay_senal
        if simulada:
            distancia_cm = self._distancia_demo()
        frenado = _recortar((distancia_cm - UMBRAL_ALERTA_CM) / 40.0, 0.0, 1.0)
        velocidad = self.velocidad_maxima * frenado
        radio = math.hypot(-self.semiejes[0] * math.sin(self._angulo),
                           self.semiejes[1] * math.cos(self._angulo))
        self._angulo += velocidad * paso_s / radio
        x = self.centro[0] + self.semiejes[0] * math.cos(self._angulo)
        y = self.centro[1] + self.semiejes[1] * math.sin(self._angulo)
        rumbo = math.degrees(math.atan2(self.semiejes[0] * math.cos(self._angulo),
                                        -self.semiejes[1] * math.sin(self._angulo)))
        return Lectura(x, y, rumbo % 360.0, velocidad, distancia_cm, simulada)

    def _distancia_demo(self):
        ciclo = (time.time() % 12.0) / 12.0
        medio = (self.demo_min + self.demo_max) / 2.0
        amplitud = (self.demo_max - self.demo_min) / 2.0
        return medio + amplitud * math.cos(2.0 * math.pi * ciclo)


class FuenteManual:
    """Pose escrita por terminal, línea a línea: x,y,rumbo[,velocidad[,distancia_cm]]."""

    nombre = "manual"

    def __init__(self):
        self._datos = Lectura(12.0, 8.0, 0.0, 0.0)
        self._bloqueo = threading.Lock()
        threading.Thread(target=self._escuchar, daemon=True).start()
        print("Fuente manual: escribe  x,y,rumbo[,velocidad[,distancia_cm]]  y pulsa Enter.")

    def _escuchar(self):
        for linea in sys.stdin:
            partes = [p for p in linea.replace(";", ",").split(",") if p.strip()]
            try:
                valores = [float(p) for p in partes]
            except ValueError:
                print(f"  (descartada) '{linea.strip()}' no tiene la forma x,y,rumbo")
                continue
            velocidad = valores[3] if len(valores) > 3 else 0.0
            distancia = valores[4] if len(valores) > 4 else None
            with self._bloqueo:
                self._datos = Lectura(valores[0], valores[1], valores[2] % 360.0,
                                      velocidad, distancia)

    def siguiente(self, paso_s, distancia_cm, hay_senal):
        with self._bloqueo:
            datos = self._datos
        if datos.distancia_cm is None:
            datos = datos._replace(distancia_cm=distancia_cm)
        return datos._replace(simulada=not hay_senal)


FUENTES = {"circuito": FuenteCircuito, "manual": FuenteManual}


class Estado:
    """Última lectura del ESP32, compartida entre el hilo receptor y el bucle emisor."""

    def __init__(self):
        self._bloqueo = threading.Lock()
        self.distancia_cm = None
        self.ultimo_s = 0.0
        self.paquetes = 0

    def registrar(self, direccion, distancia):
        with self._bloqueo:
            self.distancia_cm = distancia
            self.ultimo_s = time.monotonic()
            self.paquetes += 1
        print(f"ESP32 {direccion}: {distancia} cm")

    def copiar(self):
        with self._bloqueo:
            hay_senal = (time.monotonic() - self.ultimo_s) < RETRASO_SIN_SENAL_S
            return self.distancia_cm, hay_senal, self.paquetes


def recibir_esp32(sock, estado):
    while True:
        try:
            datos, direccion = sock.recvfrom(1024)
        except OSError:
            return
        texto = datos.decode("utf-8", errors="replace").strip()
        distancia = interpretar(texto)
        if distancia is None:
            print(f"Advertencia: dato no reconocido del ESP32 -> {texto}")
            continue
        estado.registrar(direccion[0], distancia)


def interpretar(texto):
    """Acepta el número crudo del firmware actual y también un JSON con distancia."""
    if not texto:
        return None
    try:
        return float(texto)
    except ValueError:
        pass
    try:
        paquete = json.loads(texto)
    except json.JSONDecodeError:
        return None
    if not isinstance(paquete, dict):
        return None
    medicion = paquete.get("medicion")
    if isinstance(medicion, dict) and "distancia_cm" in medicion:
        return float(medicion["distancia_cm"])
    for clave in ("distancia_cm", "distancia"):
        if clave in paquete:
            return float(paquete[clave])
    return None


def construir_paquete(fuente, lectura):
    alerta = lectura.distancia_cm <= UMBRAL_ALERTA_CM
    return {
        "dispositivo": "radar_esp32",
        "fuente": fuente.nombre,
        "t": time.time(),
        "pose": {"x": round(lectura.x, 3), "y": round(lectura.y, 3),
                 "rumbo": round(lectura.rumbo, 1)},
        "velocidad": round(lectura.velocidad, 3),
        "medicion": {"distancia_cm": round(lectura.distancia_cm, 1), "unidad": "cm"},
        "alerta": alerta,
    }


def _recortar(valor, minimo, maximo):
    return max(minimo, min(maximo, valor))


def publicar(argumentos, estado):
    sock_esp32 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_esp32.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock_esp32.bind((IP_ESP32, argumentos.puerto_esp32))
    threading.Thread(target=recibir_esp32, args=(sock_esp32, estado), daemon=True).start()

    sock_godot = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    destino = (argumentos.ip_godot, argumentos.puerto_godot)
    fuente = FUENTES[argumentos.fuente]()
    paso_s = 1.0 / argumentos.hz
    print(f"--- ESP32 en :{argumentos.puerto_esp32}  ->  Godot en "
          f"{destino[0]}:{destino[1]}  (fuente {argumentos.fuente}, {argumentos.hz} Hz) ---")

    proximo = time.monotonic()
    ultimo_reporte = 0.0
    try:
        while True:
            time.sleep(max(0.0, proximo - time.monotonic()))
            proximo += paso_s

            distancia, hay_senal, paquetes = estado.copiar()
            lectura = fuente.siguiente(paso_s, distancia, hay_senal)
            if lectura.distancia_cm is None:
                continue

            if not argumentos.sin_reenvio:
                texto = json.dumps(construir_paquete(fuente, lectura))
                sock_godot.sendto(texto.encode("utf-8"), destino)
                if argumentos.verbose:
                    print(texto)

            ahora = time.monotonic()
            if ahora - ultimo_reporte >= 1.0:
                ultimo_reporte = ahora
                marca = "SIMULADA" if lectura.simulada else "ESP32"
                print(f"  {marca:>8} · x {lectura.x:5.2f} m · y {lectura.y:5.2f} m · "
                      f"rumbo {lectura.rumbo:3.0f}° · v {lectura.velocidad:4.2f} m/s · "
                      f"radar {lectura.distancia_cm:5.1f} cm · paquetes {paquetes}")
    except KeyboardInterrupt:
        print("\nIntermediario detenido por el usuario.")
    finally:
        sock_esp32.close()
        sock_godot.close()


def analizar():
    parser = argparse.ArgumentParser(
        description="Puente ESP32 -> Godot para el gemelo digital.")
    parser.add_argument("--puerto-esp32", type=int, default=PUERTO_ESP32)
    parser.add_argument("--ip-godot", default=IP_GODOT)
    parser.add_argument("--puerto-godot", type=int, default=PUERTO_GODOT)
    parser.add_argument("--hz", type=float, default=20.0, help="frecuencia de publicación")
    parser.add_argument("--fuente", choices=sorted(FUENTES), default="circuito",
                        help="de dónde sale la posición del carro en el mapa")
    parser.add_argument("--sin-reenvio", action="store_true", help="solo imprime, no envía a Godot")
    parser.add_argument("--verbose", action="store_true", help="imprime cada paquete JSON")
    return parser.parse_args()


if __name__ == "__main__":
    publicar(analizar(), Estado())
