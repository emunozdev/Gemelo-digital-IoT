import json
import socket
import time

LISTEN_ADDR = ("0.0.0.0", 4210)     # puerto donde escucha (el mismo UDP_PORT del ESP32)
GODOT_ADDR = ("127.0.0.1", 4211)    # donde Godot escucha el JSON (PacketPeerUDP)
SAVE_FILE = "latest.json"           # opcional: último JSON en disco (None para desactivar)


def parse_packet(text: str) -> dict:
    """Convierte el texto del ESP32 en un diccionario listo para json.dumps."""
    parts = text.strip().split("|", 3)
    if len(parts) != 4 or parts[0] != "RADAR":
        raise ValueError(f"Paquete inválido: {text[:60]!r}")

    _, seq, n, body = parts
    samples = []
    for item in body.split(";"):
        if not item:
            continue
        angle, dist_mm, t_ms = item.split(",")
        dist_mm = int(dist_mm)
        samples.append({
            "angle": int(angle),                 # grados 0..128
            "distance_cm": dist_mm / 10.0,       # cm
            "detected": dist_mm > 0,             # False = no hubo objeto
            "t_ms": int(t_ms),                   # millis() del ESP32
        })

    return {
        "seq": int(seq),
        "count": len(samples),
        "received_at": time.time(),
        "samples": samples,
    }


def main():
    rx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rx.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    rx.bind(LISTEN_ADDR)

    tx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    print(f"Escuchando en UDP {LISTEN_ADDR[1]}... (Ctrl+C para salir)")
    while True:
        data, addr = rx.recvfrom(2048)
        try:
            payload = parse_packet(data.decode("utf-8", errors="replace"))
        except (ValueError, IndexError) as e:
            print("Descartado:", e)
            continue

        js = json.dumps(payload)
        print(f"[{addr[0]}] seq={payload['seq']} muestras={payload['count']}")

        detectados = [s for s in payload["samples"] if s["detected"]]
        if detectados:
            cercano = min(detectados, key=lambda s: s["distance_cm"])
            print(f"   -> {len(detectados)} detecciones, más cercana: "
                f"{cercano['distance_cm']} cm a {cercano['angle']}°")

        tx.sendto(js.encode("utf-8"), GODOT_ADDR)
        if SAVE_FILE:
            with open(SAVE_FILE, "w", encoding="utf-8") as f:
                f.write(js)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nCerrado.")
