extends Node
## Escucha los datagramas UDP que publica `middleware_bridge.py` en el puerto
## 4243 y los vuelca en el autoload `Telemetria`.

@export var puerto: int = 4243
@export var imprimir_paquetes: bool = false

var _udp := PacketPeerUDP.new()
var _enlazado := false


func _ready() -> void:
	var error := _udp.bind(puerto)
	_enlazado = error == OK
	if _enlazado:
		print("[UDP] Gemelo a la escucha en 127.0.0.1:%d" % puerto)
	else:
		push_warning("No se pudo abrir el puerto UDP %d (%s)" % [puerto, error_string(error)])


func _process(_delta: float) -> void:
	if not _enlazado:
		return
	while _udp.get_available_packet_count() > 0:
		var texto := _udp.get_packet().get_string_from_utf8().strip_edges()
		if texto.is_empty():
			continue
		if imprimir_paquetes:
			print("[UDP] ", texto)
		Telemetria.aplicar(texto)


func _exit_tree() -> void:
	_udp.close()
