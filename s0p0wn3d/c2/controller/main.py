import threading
from controller.cli import start_cli
from controller.clients import ClientManager
from http.server import ThreadingHTTPServer
import logging
from functools import partial
from controller.network import make_handler
from controller.protocol import handle_request

logging.basicConfig(
    filename="c2_server.log",
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    encoding="utf-8",
)

# Faire la commande suivante dans PowerShell pour suivre le fichier de log en temps réel :
# Get-Content .\c2_server.log -Wait -Tail 20

def run_server(client_manager, host="0.0.0.0", port=8080):
    dispatcher = partial(handle_request, client_manager=client_manager)
    server = ThreadingHTTPServer((host, port), make_handler(dispatcher))
    logging.info("Serveur sur http://%s:%s", host, port)
    server.serve_forever()

if __name__ == "__main__":
    
    #import manager
    manager = ClientManager()
    
    # http server
    server_thread = threading.Thread(target=run_server, kwargs={"client_manager": manager}, daemon=True)
    server_thread.start()

    # CLI
    try:
        start_cli(client_manager=manager)
    except KeyboardInterrupt:
        print("\nArrêt demandé.")
    finally:
        print("Fermeture du serveur…")