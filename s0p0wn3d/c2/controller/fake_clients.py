# fake_client.py
"""
Simule un client qui :
  1. s'initialise auprès du serveur (POST {"action":"init"})
  2. poll régulièrement (POST {"sid": ..., "output": ...})
  3. exécute la commande reçue (simulée) et renvoie un résultat au poll suivant
"""
import json
import time
import urllib.request
import urllib.error
import threading


class FakeClient:
    def __init__(self, name: str, server_url: str, color: str):
        self.name = name
        self.server_url = server_url
        self.color = color
        self.client_id: str | None = None
        self.session_token: str | None = None
        self.shared_key: str | None = None
        self._pending_output: str | None = None

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

    # ---------- étapes ----------

    def log(self, msg: str):
        print(f"{self.color}[{self.name}]{RESET} {msg}", flush=True)

    def init(self):
        self.log("→ POST init")
        resp = self._post({"action": "init"})
        self.client_id = resp["client_id"]
        self.session_token = resp["session_token"]
        self.shared_key = resp["shared_key"]
        self.log(f"← client_id={self.client_id}")
        self.log(f"← session_token={self.session_token[:12]}…")
        self.log(f"← shared_key={self.shared_key[:12]}…")

    def poll_once(self, output: str | None = None):
        payload = {"sid": self.session_token}
        if output is not None:
            payload["output"] = output
        resp = self._post(payload)
        cmd = resp.get("cmd", "")
        if cmd:
            self.log(f"← commande reçue : {cmd!r} (id={resp.get('command_id')})")
            # simule l'exécution
            self._pending_output = f"{self.name} a exécuté {cmd!r} avec succès"
        else:
            self.log("← pas de commande")
        return cmd

    def run(self, interval: float = 3.0, max_iters: int = 10):
        try:
            self.init()
            for i in range(max_iters):
                output = self._pending_output
                self._pending_output = None
                self.poll_once(output=output)
                time.sleep(interval)
        except Exception as e:
            self.log(f"!! erreur : {e}")


# couleurs ANSI
RESET = "\033[0m"
BLUE = "\033[94m"
GREEN = "\033[92m"


if __name__ == "__main__":
    SERVER = "http://127.0.0.1:8080"

    alice = FakeClient("Alice", SERVER, BLUE)
    bob = FakeClient("Bob", SERVER, GREEN)

    t1 = threading.Thread(target=alice.run, kwargs={"interval": 3.0}, daemon=True)
    t2 = threading.Thread(target=bob.run, kwargs={"interval": 4.5}, daemon=True)

    print("Démarrage des 2 faux clients (Ctrl+C pour arrêter)\n")
    t1.start()
    t2.start()

    try:
        t1.join()
        t2.join()
    except KeyboardInterrupt:
        print("\nArrêt.")