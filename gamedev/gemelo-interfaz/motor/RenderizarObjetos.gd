extends Node2D
## Capa visual del gemelo sobre el mapa: estela recorrida, alcance del radar y
## el punto exacto que reportó el ESP32, para comparar realidad y render.

@export var pixeles_por_metro: float = 50.0
@export var maximo_puntos_estela: int = 400
@export var intervalo_muestreo_ms: int = 60
@export var color_estela := Color(0.35, 0.75, 1.0, 0.35)
@export var color_radar := Color(0.45, 1.0, 0.65, 0.55)
@export var color_alerta := Color(1.0, 0.35, 0.35, 0.9)
@export var color_pose_real := Color(1.0, 0.85, 0.35, 0.9)

var _estela := PackedVector2Array()
var _ultima_muestra_ms := 0
var _carro: MotorCarro2D = null


func _ready() -> void:
	Telemetria.telemetria_actualizada.connect(queue_redraw)


func _process(_delta: float) -> void:
	if _carro == null:
		_carro = get_tree().get_first_node_in_group("carro") as MotorCarro2D
		return
	var ahora := Time.get_ticks_msec()
	if ahora - _ultima_muestra_ms >= intervalo_muestreo_ms:
		_ultima_muestra_ms = ahora
		_estela.append(_carro.global_position)
		if _estela.size() > maximo_puntos_estela:
			_estela.remove_at(0)
	queue_redraw()


func _draw() -> void:
	if _estela.size() > 1:
		draw_polyline(_estela, color_estela, 3.0)
	if _carro == null:
		return

	var origen := to_local(_carro.global_position)
	var alcance_px := Telemetria.distancia_cm * 0.01 * pixeles_por_metro
	var color := color_alerta if Telemetria.alerta else color_radar

	if alcance_px > 1.0:
		var apertura := deg_to_rad(60.0)
		var frente := _angulo_pantalla(_carro.rumbo)
		draw_arc(origen, alcance_px, frente - apertura, frente + apertura, 24, color, 2.0)
		draw_circle(origen + Vector2(sin(_carro.rumbo), -cos(_carro.rumbo)) * alcance_px, 5.0, color)

	if Telemetria.esta_conectado():
		var pose_px := to_local(Telemetria.pose_m * pixeles_por_metro)
		draw_line(origen, pose_px, color_pose_real, 1.5, true)
		draw_set_transform(pose_px, 0.0, Vector2.ONE)
		draw_line(Vector2(-8, 0), Vector2(8, 0), color_pose_real, 2.0)
		draw_line(Vector2(0, -8), Vector2(0, 8), color_pose_real, 2.0)
		draw_set_transform(Vector2.ZERO, 0.0, Vector2.ONE)


func _angulo_pantalla(rumbo_rad: float) -> float:
	return rumbo_rad - PI * 0.5
