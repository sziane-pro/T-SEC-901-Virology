"""
Simule un client qui :
  1. s'enregistre (POST {"action":"register", "client_id", "shared_key"})
  2. beacon régulièrement (POST {"action":"beacon", "client_id"})
     → le serveur renvoie une commande à exécuter (ou rien)
  3. envoie le résultat de la commande exécutée (POST {"action":"command_result", ...})
"""
import json
import time
import urllib.request
import urllib.error
import threading

RESET = "\033[0m"
BLUE  = "\033[94m"
GREEN = "\033[92m"


class FakeClient:
    def __init__(self, name: str, server_url: str, color: str,
                 client_id: str, shared_key: str):
        self.name        = name
        self.server_url  = server_url
        self.color       = color
        self.client_id   = client_id
        self.shared_key  = shared_key
        self.session_token: str | None = None

    # ---------- transport ----------

    def _post(self, payload: dict) -> dict:
        data = json.dumps(payload).encode()
        req = urllib.request.Request(
            self.server_url,
            data=data,
            headers={"Content-Type": "application/json"},
            method="POST",
        )
        try:
            with urllib.request.urlopen(req, timeout=10) as resp:
                return json.loads(resp.read())
        except urllib.error.HTTPError as e:
            body = e.read().decode(errors="replace")
            raise RuntimeError(f"HTTP {e.code} : {body}") from e

    def log(self, msg: str):
        print(f"{self.color}[{self.name}]{RESET} {msg}", flush=True)

    # ---------- étapes ----------
    
    def provision(self):
        resp = self._post({"action": "admin_provision"})
        self.client_id  = resp.get("client_id")
        self.shared_key = resp.get("shared_key")

    def register(self):
        """Premier POST : s'enregistre et récupère un session_token."""
        self.log("→ POST register")
        resp = self._post({
            "action":     "register",
            "client_id":  self.client_id,
            "shared_key": self.shared_key,
        })
        self.session_token = resp["session_token"]
        self.log(f"← session_token={self.session_token[:12]}…")

    def beacon(self) -> dict | None:
        """Poste un beacon, reçoit une commande éventuelle."""
        resp = self._post({
            "action":    "beacon",
            "client_id": self.client_id,
        })
        cmd_id  = resp.get("command_id")
        cmd_str = resp.get("command")

        if not cmd_str:
            self.log("← pas de commande")
            return None

        self.log(f"← commande reçue : {cmd_str!r} (id={cmd_id})")
        return {"command_id": cmd_id, "command": cmd_str}

    def send_result(self, command_id: str, command_response: str, status: str = "ok"):
        """Renvoie le résultat de la commande exécutée."""
        self.log(f"→ POST command_result (id={command_id})")
        self._post({
            "action":           "command_result",
            "command_id":       command_id,
            "command_response": command_response,
            "status":           status,
        })

    def execute(self, cmd: dict):
        """Simule l'exécution de la commande."""
        # Ici, la vraie logique d'exécution.
        return f"{self.name} a exécuté {cmd['command']!r} avec succès"

    # ---------- boucle ----------

    def run(self, interval: float = 3.0, max_iters: int = 10):
        try:
            self.provision()
            self.register()
            for _ in range(max_iters):
                cmd = self.beacon()
                if cmd is not None:
                    result = self.execute(cmd)
                    self.send_result(cmd["command_id"], result, status="ok")
                time.sleep(interval)
        except Exception as e:
            self.log(f"!! erreur : {e}")


# ---------- provisioning out-of-band (à brancher) ----------

def provision(url: str, name: str) -> tuple[str, str]:
    """
    Distribue un couple (client_id, shared_key) au client 'name'.
    À remplacer par votre mécanisme réel (fichier, base, API admin…).
    Le serveur DOIT avoir ces credentials dans sa whitelist avant le register.
    """
    raise NotImplementedError("Brancher ici votre provisioning out-of-band")


if __name__ == "__main__":
    SERVER = "http://127.0.0.1:8080"

    # Credentials pré-distribués hors-bande (côté serveur : whitelist).
    alice_id, alice_key = "alice-preprovisioned-id", "alice-preprovisioned-key"
    bob_id,   bob_key   = "bob-preprovisioned-id",   "bob-preprovisioned-key"

    alice = FakeClient("Alice", SERVER, BLUE,  alice_id, alice_key)
    bob   = FakeClient("Bob",   SERVER, GREEN, bob_id,   bob_key)

    t1 = threading.Thread(target=alice.run, kwargs={"interval": 3.0}, daemon=True)
    t2 = threading.Thread(target=bob.run,   kwargs={"interval": 4.5}, daemon=True)

    print("Démarrage des 2 faux clients (Ctrl+C pour arrêter)\n")
    t1.start()
    t2.start()

    try:
        t1.join()
        t2.join()
    except KeyboardInterrupt:
        print("\nArrêt.")