extends CharacterBody2D
class_name MotorCarro2D
## Motor 2D del carro del gemelo digital.
##
## Dos fuentes de control sobre el mismo cuerpo:
##  - TELEMETRÍA: sigue la pose que publica el ESP32 a través del middleware,
##    corrigiendo la posición de forma proporcional para no teletransportar.
##  - TECLADO: dirección directa, el carro va hacia donde se pulsa y mira hacia
##    ahí. Sirve para probar el mapa cuando todavía no llega señal del carro.

enum Modo { AUTO, TELEMETRIA, TECLADO }

@export_group("Mapa")
## Escala del mundo: 1 metro real son N pixeles del mapa.
@export var pixeles_por_metro: float = 50.0

@export_group("Conduccion")
@export var velocidad_maxima: float = 6.0
## Suavizado del control por teclado, en 1/s. Más alto, más inmediato.
@export var respuesta_teclado: float = 10.0

@export_group("Gemelo")
@export var modo: Modo = Modo.AUTO
## Ganancia del corrector proporcional que acerca el gemelo a la pose real.
@export var ganancia_pose: float = 6.0
@export var ganancia_rumbo: float = 8.0
## Por debajo de este error (metros) el gemelo se da por alineado.
@export var tolerancia_pose_m: float = 0.05

@onready var sprite: Sprite2D = $Sprite2D

var rumbo := 0.0
var velocidad := 0.0
var modo_activo := Modo.TECLADO
var animador := AnimadorCarro.new()

signal modo_cambiado(nuevo_modo: Modo)


func _ready() -> void:
	add_to_group("carro")
	sprite.hframes = AnimadorCarro.COLUMNAS
	sprite.vframes = AnimadorCarro.FILAS
	sprite.offset = AnimadorCarro.DESPLAZAMIENTO_VISUAL
	rumbo = deg_to_rad(Telemetria.rumbo_grados)
	modo_activo = _resolver_modo()


func _physics_process(delta: float) -> void:
	var nuevo_modo := _resolver_modo()
	if nuevo_modo != modo_activo:
		modo_activo = nuevo_modo
		modo_cambiado.emit(modo_activo)

	var posicion_previa := global_position
	if modo_activo == Modo.TELEMETRIA:
		_seguir_telemetria(delta)
	else:
		_conducir_direccional(delta)

	move_and_slide()

	if modo_activo == Modo.TELEMETRIA:
		velocidad = (global_position - posicion_previa).length() / pixeles_por_metro / maxf(delta, 0.0001)

	_animar(delta)


func _unhandled_key_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and event.physical_keycode == KEY_F1:
		modo = Modo.TECLADO if modo != Modo.TECLADO else Modo.AUTO
		modo_cambiado.emit(_resolver_modo())


func _resolver_modo() -> Modo:
	if modo != Modo.AUTO:
		return modo
	return Modo.TELEMETRIA if Telemetria.esta_conectado() else Modo.TECLADO


func _seguir_telemetria(delta: float) -> void:
	var objetivo_px := Telemetria.pose_m * pixeles_por_metro
	var error := objetivo_px - global_position
	if error.length() > tolerancia_pose_m * pixeles_por_metro:
		velocity = error * ganancia_pose
		var tope := velocidad_maxima * pixeles_por_metro * 2.0
		if velocity.length() > tope:
			velocity = velocity.normalized() * tope
	else:
		velocity = Vector2.ZERO
	rumbo = _acercar_angulo(rumbo, deg_to_rad(Telemetria.rumbo_grados), 1.0 - exp(-ganancia_rumbo * delta))


func _conducir_direccional(delta: float) -> void:
	var direccion := Input.get_vector("mover_izquierda", "mover_derecha",
			"mover_arriba", "mover_abajo")
	var objetivo := direccion * velocidad_maxima * pixeles_por_metro
	velocity = velocity.lerp(objetivo, 1.0 - exp(-respuesta_teclado * delta))
	velocidad = velocity.length() / pixeles_por_metro
	if direccion != Vector2.ZERO:
		rumbo = atan2(direccion.x, -direccion.y)


func _animar(delta: float) -> void:
	var fotograma := animador.actualizar(rumbo, velocidad / velocidad_maxima, delta)
	sprite.frame = animador.indice(fotograma.x, fotograma.y)


func _acercar_angulo(desde: float, hacia: float, peso: float) -> float:
	var diferencia := wrapf(hacia - desde + PI, 0.0, TAU) - PI
	return desde + diferencia * clampf(peso, 0.0, 1.0)


func pose_m() -> Vector2:
	return global_position / pixeles_por_metro


func rumbo_grados() -> float:
	return wrapf(rad_to_deg(rumbo), 0.0, 360.0)


func velocidad_kmh() -> float:
	return velocidad * 3.6


func error_gemelo_m() -> float:
	return Telemetria.pose_m.distance_to(pose_m())


func nombre_modo() -> String:
	return "TELEMETRÍA" if modo_activo == Modo.TELEMETRIA else "SIMULACIÓN"
