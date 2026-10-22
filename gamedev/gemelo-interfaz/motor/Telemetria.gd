extends Node
## Autoload `Telemetria`: estado compartido que llega del ESP32 a través del
## middleware. Un solo lugar donde el carro, el mapa y el HUD leen el gemelo.

signal telemetria_actualizada
signal conexion_actualizada(conectado: bool)

## Retraso máximo para seguir considerando que hay señal del carro real.
const RETARDO_MAXIMO_MS := 1500

var pose_m: Vector2 = Vector2.ZERO
var rumbo_grados: float = 0.0
var velocidad_ms: float = 0.0
var distancia_cm: float = 0.0
var alerta: bool = false
var fuente: String = "sin datos"
var paquetes_recibidos: int = 0
var paquetes_rechazados: int = 0

var _ultimo_ms: int = -RETARDO_MAXIMO_MS
var _senal_notificada := false


func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS


func esta_conectado() -> bool:
	return retardo_ms() < RETARDO_MAXIMO_MS


func retardo_ms() -> int:
	return Time.get_ticks_msec() - _ultimo_ms


func aplicar(texto: String) -> void:
	var dato: Variant = JSON.parse_string(texto)
	if typeof(dato) != TYPE_DICTIONARY:
		paquetes_rechazados += 1
		return
	_aplicar_diccionario(dato)


func _aplicar_diccionario(paquete: Dictionary) -> void:
	if paquete.get("pose") is Dictionary:
		var pose: Dictionary = paquete["pose"]
		pose_m = Vector2(float(pose.get("x", pose_m.x)), float(pose.get("y", pose_m.y)))
		rumbo_grados = float(pose.get("rumbo", rumbo_grados))
	velocidad_ms = float(paquete.get("velocidad", velocidad_ms))
	if paquete.get("medicion") is Dictionary:
		var medicion: Dictionary = paquete["medicion"]
		distancia_cm = float(medicion.get("distancia_cm", distancia_cm))
	alerta = bool(paquete.get("alerta", alerta))
	fuente = str(paquete.get("fuente", fuente))
	paquetes_recibidos += 1
	_ultimo_ms = Time.get_ticks_msec()
	_notificar_conexion()
	telemetria_actualizada.emit()


func _process(_delta: float) -> void:
	_notificar_conexion()


func _notificar_conexion() -> void:
	var hay_senal := esta_conectado()
	if hay_senal == _senal_notificada:
		return
	_senal_notificada = hay_senal
	conexion_actualizada.emit(hay_senal)
