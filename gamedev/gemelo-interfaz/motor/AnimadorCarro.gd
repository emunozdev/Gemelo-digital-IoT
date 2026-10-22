class_name AnimadorCarro
extends RefCounted
## Convierte el rumbo y la velocidad del carro en un fotograma de la hoja de
## sprites `resources/car.png`.
##
## La hoja mide 7296x608 px: 12 columnas x 8 filas de celdas de 608x76. Dentro
## de cada celda el camión (90x76 px) está anclado a la izquierda, por eso hay
## que compensar la posición del sprite con DESPLAZAMIENTO_VISUAL.
##
## Filas usadas: 0 y 1 = vistas laterales; 2 y 3 = vista trasera, luces rojas,
## el carro se aleja de la cámara y por eso corresponde al rumbo norte; 4, 5 y 6
## = vista frontal, luces blancas, el carro viene hacia la cámara = rumbo sur.
## La fila 7 está vacía.

const COLUMNAS := 12
const FILAS := 8
const ANCHO_CELDA := 608.0
const ANCHO_VISUAL := 90.0

## Centro de la celda (304) menos centro del camión dentro de la celda (49.5).
const DESPLAZAMIENTO_VISUAL := Vector2(ANCHO_CELDA * 0.5 - ANCHO_VISUAL * 0.5 - 5.0, -2.0)

## Las dos filas laterales solo tienen 10 columnas útiles.
const FOTOGRAMAS := 10

## Fila según el sector de rumbo: round(grados / 45) % 8.
const FILAS_POR_SECTOR := [3, 2, 1, 6, 5, 4, 0, 2]

## Fotogramas por segundo de animación a velocidad máxima.
const VELOCIDAD_ANIMACION := 12.0

var _acumulado := 0.0


func actualizar(rumbo_rad: float, velocidad_normalizada: float, delta: float) -> Vector2i:
	var magnitud := clampf(absf(velocidad_normalizada), 0.0, 1.0)
	_acumulado = 0.0 if magnitud < 0.02 else _acumulado + delta * magnitud * VELOCIDAD_ANIMACION
	var columna := int(_acumulado) % FOTOGRAMAS
	var grados := fmod(rad_to_deg(rumbo_rad) + 360.0, 360.0)
	var sector := int(round(grados / 45.0)) % 8
	return Vector2i(columna, FILAS_POR_SECTOR[sector])


func indice(columna: int, fila: int) -> int:
	return fila * COLUMNAS + columna
