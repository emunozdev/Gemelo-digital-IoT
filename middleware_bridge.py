import argparse
import json
import math
import socket
import struct
import sys
import threading
import time
from collections import namedtuple

IP_ESP32 = "0.0.0.0"
PUERTO_ESP32 = 4242
IP_GODOT = "127.0.0.1"
PUERTO_GODOT = 4243
UMBRAL_ALERTA_CM = 20          # umbral del esp32
RETRASO_SIN_SENAL_S = 1.5


FORMATO_TRAMA = "<BHIBHBB"
LONGITUD_TRAMA = struct.calcsize(FORMATO_TRAMA)      
TRAMA_MAGIC = 0xA5
FLAG_ALERTA = 0x01
DIST_SIN_ECO = 0xFFFF
DIST_SIN_ECO_CM = 400.0                              
ESTADOS = {0: "parado", 1: "adelante", 2: "atras", 3: "derecha", 4: "izquierda"}

Lectura = namedtuple("Lectura", "x y rumbo velocidad distancia_cm simulada", defaults=(None, False))


class FuenteCircuito:
    nombre = "circuito"
    centro = (12.0, 8.0)
    semiejes = (8.0, 5.0)
    velocidad_maxima = 4.0
    demo_min = 18.0
    demo_max = 90.0

    def __init__(self):
        self._angulo = 0.0

    def siguiente(self, paso_s, distancia_cm, hay_senal, estado=None):
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


class FuenteCarro:
    nombre = "carro"
    limites = (24.0, 16.0)

    def __init__(self, velocidad=2.0, giro=90.0):
        self.velocidad = velocidad     
        self.giro = giro                
        self._x, self._y, self._rumbo = 12.0, 8.0, 0.0

    def siguiente(self, paso_s, distancia_cm, hay_senal, estado=None):
        v = 0.0
        if hay_senal:                   
            if estado == 1:
                v = self.velocidad
            elif estado == 2:
                v = -self.velocidad
            elif estado == 3:
                self._rumbo += self.giro * paso_s
            elif estado == 4:
                self._rumbo -= self.giro * paso_s

        self._rumbo %= 360.0
        rad = math.radians(self._rumbo)
        self._x = _recortar(self._x + v * paso_s * math.sin(rad), 0.0, self.limites[0])
        self._y = _recortar(self._y + v * paso_s * math.cos(rad), 0.0, self.limites[1])
        return Lectura(self._x, self._y, self._rumbo, v, distancia_cm, not hay_senal)


class FuenteManual:
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

    def siguiente(self, paso_s, distancia_cm, hay_senal, estado=None):
        with self._bloqueo:
            datos = self._datos
        if datos.distancia_cm is None:
            datos = datos._replace(distancia_cm=distancia_cm)
        return datos._replace(simulada=not hay_senal)


FUENTES = {"circuito": FuenteCircuito, "manual": FuenteManual, "carro": FuenteCarro}


class Estado:
    def __init__(self):
        self._bloqueo = threading.Lock()
        self.medida = None
        self.ultimo_s = 0.0
        self.paquetes = 0
        self.perdidas = 0
        self.descartadas = 0
        self._ultima_seq = None
        self._ultimo_ms = None

    def registrar(self, medida):
        with self._bloqueo:
            seq, ms = medida["seq"], medida["ms"]
            if seq is not None:
                if self._ultimo_ms is not None and ms < self._ultimo_ms:
                    self._ultima_seq = None         
                if self._ultima_seq is not None:
                    salto = (seq - self._ultima_seq) & 0xFFFF
                    if salto == 0 or salto >= 0x8000:
                        return                       
                    self.perdidas += salto - 1
                self._ultima_seq = seq
                self._ultimo_ms = ms
            self.medida = medida
            self.ultimo_s = time.monotonic()
            self.paquetes += 1

    def descartar(self):
        with self._bloqueo:
            self.descartadas += 1

    def copiar(self):
        with self._bloqueo:
            hay_senal = (time.monotonic() - self.ultimo_s) < RETRASO_SIN_SENAL_S
            return self.medida, hay_senal, self.paquetes, self.perdidas


def decodificar_trama(datos):
    if len(datos) != LONGITUD_TRAMA or datos[0] != TRAMA_MAGIC:
        return None

    _, seq, ms, angulo, dist, estado, flags = struct.unpack(FORMATO_TRAMA, datos)
    sin_eco = dist == DIST_SIN_ECO
    return {"seq": seq, "ms": ms, "angulo": angulo,
            "distancia_cm": DIST_SIN_ECO_CM if sin_eco else float(dist),
            "sin_eco": sin_eco, "estado": estado, "alerta": bool(flags & FLAG_ALERTA)}


def medida_desde_texto(datos):
    distancia = interpretar(datos.decode("utf-8", errors="replace").strip())

    if distancia is None or not math.isfinite(distancia):
        return None
    return {"seq": None, "ms": None, "angulo": 90, "distancia_cm": distancia,
            "sin_eco": False, "estado": None, "alerta": distancia <= UMBRAL_ALERTA_CM}


def recibir_esp32(sock, estado):
    while True:
        try:
            datos, _ = sock.recvfrom(1024)
        except OSError:
            return
        medida = decodificar_trama(datos) or medida_desde_texto(datos)
        if medida is None:
            estado.descartar()
            continue
        estado.registrar(medida)      


def interpretar(texto):
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


def construir_paquete(fuente, lectura, medida, perdidas):
    medida = medida or {}
    return {
        "dispositivo": "radar_esp32",
        "fuente": fuente.nombre,
        "t": time.time(),
        "pose": {"x": round(lectura.x, 3), "y": round(lectura.y, 3),
                 "rumbo": round(lectura.rumbo, 1)},
        "velocidad": round(lectura.velocidad, 3),
        "medicion": {"distancia_cm": round(lectura.distancia_cm, 1), "unidad": "cm",
                     "angulo": medida.get("angulo", 90),
                     "sin_eco": medida.get("sin_eco", False)},
        "alerta": lectura.distancia_cm <= UMBRAL_ALERTA_CM,
        "estado_carro": ESTADOS.get(medida.get("estado"), "desconocido"),
        "enlace": {"seq": medida.get("seq"), "perdidas": perdidas},
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
    if argumentos.fuente == "carro":
        fuente = FuenteCarro(argumentos.vel, argumentos.giro)
    else:
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

            medida, hay_senal, paquetes, perdidas = estado.copiar()
            distancia = medida["distancia_cm"] if medida else None
            codigo = medida["estado"] if medida else None
            lectura = fuente.siguiente(paso_s, distancia, hay_senal, codigo)
            if lectura.distancia_cm is None:
                continue

            medida_vigente = medida if hay_senal else None
            if not argumentos.sin_reenvio:
                texto = json.dumps(construir_paquete(fuente, lectura, medida_vigente, perdidas))
                sock_godot.sendto(texto.encode("utf-8"), destino)
                if argumentos.verbose:
                    print(texto)

            ahora = time.monotonic()
            if ahora - ultimo_reporte >= 1.0:
                ultimo_reporte = ahora
                marca = "SIMULADA" if lectura.simulada else "ESP32"
                angulo = medida_vigente["angulo"] if medida_vigente else 90
                nombre_estado = ESTADOS.get(codigo, "-") if medida_vigente else "-"
                print(f"  {marca:>8} · x {lectura.x:5.2f} m · y {lectura.y:5.2f} m · "
                      f"rumbo {lectura.rumbo:3.0f}° · v {lectura.velocidad:4.2f} m/s · "
                      f"radar {lectura.distancia_cm:5.1f} cm @ {angulo:3d}° · {nombre_estado:<9} · "
                      f"paquetes {paquetes} · perdidas {perdidas} · descartadas {estado.descartadas}")
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
    parser.add_argument("--vel", type=float, default=2.0,
                        help="fuente 'carro': velocidad en el mapa al ir adelante (m/s)")
    parser.add_argument("--giro", type=float, default=90.0,
                        help="fuente 'carro': velocidad de giro (grados/s)")
    parser.add_argument("--sin-reenvio", action="store_true", help="solo imprime, no envía a Godot")
    parser.add_argument("--verbose", action="store_true", help="imprime cada paquete JSON")
    return parser.parse_args()


if __name__ == "__main__":
    publicar(analizar(), Estado())