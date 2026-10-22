extends CanvasLayer
## Panel de estado del gemelo digital: lo que reporta el ESP32 y lo que el
## motor está renderizando sobre el mapa.

@export var carro_ruta: NodePath = ^"../Carro"

var _carro: MotorCarro2D
var _texto: Label
var _velo_alerta: ColorRect


func _ready() -> void:
	layer = 10
	_carro = get_node_or_null(carro_ruta) as MotorCarro2D

	_velo_alerta = ColorRect.new()
	_velo_alerta.name = "VeloAlerta"
	_velo_alerta.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_velo_alerta.set_anchors_preset(Control.PRESET_FULL_RECT)
	_velo_alerta.color = Color(0, 0, 0, 0)
	add_child(_velo_alerta)

	_texto = Label.new()
	_texto.name = "Estado"
	_texto.position = Vector2(16, 12)
	_texto.add_theme_font_size_override("font_size", 14)
	_texto.add_theme_color_override("font_outline_color", Color(0, 0, 0, 0.85))
	_texto.add_theme_constant_override("outline_size", 4)
	add_child(_texto)


func _process(_delta: float) -> void:
	_texto.text = _informe()
	_velo_alerta.color = Color(1.0, 0.2, 0.2, 0.16) if Telemetria.alerta else Color(0, 0, 0, 0)


func _informe() -> String:
	var lineas := PackedStringArray()
	lineas.append("GEMELO DIGITAL · ESP32 RADAR")
	lineas.append("Modo        %s" % (_carro.nombre_modo() if _carro != null else "—"))
	lineas.append("Señal       %s" % _estado_senal())
	lineas.append("Paquetes    %d recibidos · %d descartados" % [
		Telemetria.paquetes_recibidos, Telemetria.paquetes_rechazados])
	lineas.append("Radar       %.1f cm%s" % [
		Telemetria.distancia_cm, "   ¡ALERTA!" if Telemetria.alerta else ""])
	lineas.append("")

	if _carro != null:
		var pose := _carro.pose_m()
		lineas.append("Carro en el mapa")
		lineas.append("Posición    x %6.2f m · y %6.2f m" % [pose.x, pose.y])
		lineas.append("Rumbo       %3.0f°" % _carro.rumbo_grados())
		lineas.append("Velocidad   %5.1f km/h" % _carro.velocidad_kmh())
		lineas.append("Desfase     %5.2f m" % _carro.error_gemelo_m())
		lineas.append("")

	lineas.append("WASD o flechas: moverse · F1 forzar simulación")
	return "\n".join(lineas)


func _estado_senal() -> String:
	if not Telemetria.esta_conectado():
		return "sin datos (último hace %.1f s)" % (Telemetria.retardo_ms() / 1000.0)
	return "%s · último hace %d ms" % [Telemetria.fuente, Telemetria.retardo_ms()]
