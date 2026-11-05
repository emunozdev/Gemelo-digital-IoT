extends Node

@export var puerto_escucha: int = 4243

var servidor := PacketPeerUDP.new()

func _ready() -> void:
	var error = servidor.bind(puerto_escucha)
	if error == OK:
		print("Receptor UDP escuchando en el puerto: ", puerto_escucha)
	else:
		printerr("No se pudo iniciar el servidor UDP. Código de error: ", error)

func _process(_delta: float) -> void:
	while servidor.get_available_packet_count() > 0:
		var paquete: PackedByteArray = servidor.get_packet()
		var texto_json: String = paquete.get_string_from_utf8()
		
		if Telemetria:
			Telemetria.aplicar(texto_json)
		else:
			printerr("El Autoload Telemetria no está disponible.")
