# Gemelo-digital-IoT
Construcción de un gemelo virtual renderizando objetos de la vida real en Godot Engine con ESP32.

## Mapa 2D del carro

El gemelo se ve en un mapa 2D de 24 × 16 m donde el carro se mueve con la
telemetría que envía el ESP32. Los datos viajan así:

```
ESP32 (radar) --UDP :4242--> middleware_bridge.py --UDP :4243--> Godot (Telemetria) --> Carro
```

### Ejecutar

1. `cd firmware/Carro && pio run -t upload` y alimenta el ESP32. Crea la red
   `ESP32_Radar_AP` (clave `12345678`).
2. Conecta el PC a esa red para que el ESP32 pueda escribir en `192.168.4.2:4242`.
3. `python middleware_bridge.py`
4. Abre `gamedev/gemelo-interfaz/project.godot` en Godot 4 y pulsa **F5**.

Sin ESP32 también funciona: el puente publica una distancia simulada y el carro
recorre el óvalo, así que la maqueta se puede enseñar sin hardware. Con la
ventana del mapa abierta, `W`/`A`/`S`/`D` (o las flechas) mueven el carro en
ocho direcciones y `F1` fuerza el modo simulación.

### Contrato de telemetría

Un datagrama UDP por lectura, JSON UTF-8:

```json
{
  "dispositivo": "radar_esp32",
  "fuente": "circuito",
  "t": 1730000000.123,
  "pose": { "x": 12.4, "y": 8.1, "rumbo": 90.0 },
  "velocidad": 1.8,
  "medicion": { "distancia_cm": 34.0, "unidad": "cm" },
  "alerta": true
}
```

`x` e `y` son metros del mapa (x crece hacia el este, y hacia el sur) y `rumbo`
son grados con 0 = norte. `middleware_bridge.py` acepta también el número crudo
que envía el firmware actual.

### Código

| Archivo | Qué hace |
| --- | --- |
| `motor/Telemetria.gd` | Autoload con el estado compartido y la señal de conexión. |
| `motor/ReceptorUdp.gd` | Escucha el puerto 4243 y vuelca el JSON en `Telemetria`. |
| `motor/MotorCarro2D.gd` | El motor: sigue la pose real o conduce con teclado. |
| `motor/AnimadorCarro.gd` | Mapea rumbo y velocidad a la hoja de sprites. |
| `motor/Mapa.gd` | Cuadrícula métrica, muros y límites de la cámara. |
| `motor/RenderizarObjetos.gd` | Estela, alcance del radar y punto reportado por el ESP32. |
| `motor/Hud.gd` | Panel de velocidad, posición, alerta y paquetes. |

### Falta la posición real

El ESP32 de hoy solo tiene el radar ultrasónico, así que **la posición del carro
en el mapa es una simulación** (`FuenteCircuito` en `middleware_bridge.py`) y el
desfase que ves en el HUD es el de esa simulación, no el del mundo real. Para que
el gemelo muestre la posición verdadera hace falta una fuente de pose: un GPS
(NEO-6M por UART), o un encoder en cada rueda más una IMU para el rumbo. Cuando
la tengas, escribe una clase con la firma de `FuenteCircuito.siguiente()`,
regístrala en `FUENTES` y selecciona `--fuente gps`; Godot no cambia.
