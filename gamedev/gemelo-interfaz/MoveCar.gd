extends CharacterBody2D

@export var velocidad: float = 200.0
@onready var sprite_animado: AnimatedSprite2D = $AnimatedSprite2D

func _physics_process(_delta: float) -> void:
	var direccion = Vector2.ZERO
	
	if Input.is_key_pressed(KEY_W):
		direccion.y -= 1
		sprite_animado.frame = 26  # Cambia al fotograma 0 para arriba (ajusta el número según tu sprite)
	elif Input.is_key_pressed(KEY_S):
		direccion.y += 1
		sprite_animado.frame = 1  # Cambia al fotograma 1 para abajo
	elif Input.is_key_pressed(KEY_A):
		direccion.x -= 1
		sprite_animado.frame = 2  # Cambia al fotograma 2 para la izquierda
	elif Input.is_key_pressed(KEY_D):
		direccion.x += 1
		sprite_animado.frame = 3  # Cambia al fotograma 3 para la derecha
	
	direccion = direccion.normalized()
	velocity = direccion * velocidad
	move_and_slide()
