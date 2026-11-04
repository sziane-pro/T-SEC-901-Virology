# main.py
import threading
from server.main import run_server
from cli import start_cli
from controller.clients import ClientManager

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