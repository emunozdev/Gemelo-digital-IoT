extends Node2D
## Mapa 2D del gemelo digital: cuadrícula métrica, zona de maniobras y muros.
## Todo se dibuja por código para no depender de texturas de suelo.

@export var pixeles_por_metro: float = 50.0
@export var tamano_m: Vector2 = Vector2(24.0, 16.0)
@export var cerrar_perimetro := true
@export var color_suelo := Color(0.10, 0.12, 0.15)
@export var color_cuadricula := Color(1, 1, 1, 0.05)
@export var color_cuadricula_mayor := Color(1, 1, 1, 0.13)
@export var color_borde := Color(1, 1, 1, 0.30)
@export var color_muro := Color(0.32, 0.26, 0.17)

## Obstáculos en metros; posición = esquina superior izquierda, tamaño = ancho/alto.
## Todos quedan fuera del óvalo de 8x5 m que recorre el gemelo: un muro entre el
## centro y la pista bloquearía al gemelo persiguiendo su propia telemetría.
@export var obstaculos_m: Array[Rect2] = [
	Rect2(Vector2(21.2, 2.0), Vector2(1.6, 3.0)),
	Rect2(Vector2(1.2, 1.2), Vector2(2.0, 1.8)),
	Rect2(Vector2(1.5, 12.6), Vector2(2.4, 2.0)),
	Rect2(Vector2(19.0, 12.8), Vector2(3.2, 1.8)),
]


func _ready() -> void:
	_construir_muros()
	_configurar_camara()
	queue_redraw()


func _draw() -> void:
	var limite := tamano_m * pixeles_por_metro
	draw_rect(Rect2(Vector2.ZERO, limite), color_suelo, true)

	for x in range(int(tamano_m.x) + 1):
		var px := x * pixeles_por_metro
		var tono_x := color_cuadricula_mayor if x % 5 == 0 else color_cuadricula
		draw_line(Vector2(px, 0.0), Vector2(px, limite.y), tono_x)
	for y in range(int(tamano_m.y) + 1):
		var py := y * pixeles_por_metro
		var tono_y := color_cuadricula_mayor if y % 5 == 0 else color_cuadricula
		draw_line(Vector2(0.0, py), Vector2(limite.x, py), tono_y)

	draw_rect(Rect2(Vector2.ZERO, limite), color_borde, false, 2.0)


func _construir_muros() -> void:
	for rect in obstaculos_m:
		_anadir_muro(rect)
	if not cerrar_perimetro:
		return
	var grosor := 0.4
	_anadir_muro(Rect2(Vector2(-grosor, -grosor), Vector2(tamano_m.x + grosor * 2.0, grosor)))
	_anadir_muro(Rect2(Vector2(-grosor, tamano_m.y), Vector2(tamano_m.x + grosor * 2.0, grosor)))
	_anadir_muro(Rect2(Vector2(-grosor, 0.0), Vector2(grosor, tamano_m.y)))
	_anadir_muro(Rect2(Vector2(tamano_m.x, 0.0), Vector2(grosor, tamano_m.y)))


func _anadir_muro(rect: Rect2) -> void:
	var medio := rect.size * 0.5 * pixeles_por_metro
	var cuerpo := StaticBody2D.new()
	cuerpo.name = "Muro"
	cuerpo.z_index = 1
	cuerpo.position = (rect.position + rect.size * 0.5) * pixeles_por_metro

	var colision := CollisionShape2D.new()
	var forma := RectangleShape2D.new()
	forma.size = rect.size * pixeles_por_metro
	colision.shape = forma
	cuerpo.add_child(colision)

	var visual := Polygon2D.new()
	visual.polygon = PackedVector2Array([
		Vector2(-medio.x, -medio.y), Vector2(medio.x, -medio.y),
		Vector2(medio.x, medio.y), Vector2(-medio.x, medio.y),
	])
	visual.color = color_muro
	cuerpo.add_child(visual)

	add_child(cuerpo)


func _configurar_camara() -> void:
	var carro := get_tree().get_first_node_in_group("carro") as MotorCarro2D
	if carro == null:
		push_warning("El mapa no encontró un nodo en el grupo 'carro'.")
		return
	var camara := carro.get_node_or_null("Camera2D") as Camera2D
	if camara == null:
		return
	camara.limit_left = 0
	camara.limit_top = 0
	camara.limit_right = int(tamano_m.x * pixeles_por_metro)
	camara.limit_bottom = int(tamano_m.y * pixeles_por_metro)
	camara.limit_smoothed = true
	camara.position_smoothing_enabled = true
	camara.position_smoothing_speed = 6.0
	camara.make_current()
