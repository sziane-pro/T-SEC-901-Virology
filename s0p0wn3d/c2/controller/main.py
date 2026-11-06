# main.py
import threading
# from server.main import run_server
from controller.cli import start_cli
from controller.clients import ClientManager
from http.server import ThreadingHTTPServer
from controller.http_handler import make_handler


def run_server(client_manager, host="0.0.0.0", port=8080):
    server = ThreadingHTTPServer((host, port), make_handler(client_manager))
    print(f"Serveur sur http://{host}:{port}")
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