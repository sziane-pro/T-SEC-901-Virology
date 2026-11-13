import threading
from controller.cli import start_cli
from controller.clients import ClientManager
from controller.commands import CommandManager
from controller.network import app, setup_network

def main():
    client_manager = ClientManager()
    commands_manager = CommandManager(client_manager)

    setup_network(client_manager, commands_manager)

    server_thread = threading.Thread(
        target=lambda: app.run(
            host="127.0.0.1", 
            port=8080, 
            #ssl_context=("config/cert.pem", "config/key.pem")
            debug=False
        )
    )

    server_thread.daemon = True
    server_thread.start()
    print("[*] Serveur C2 démarré sur http://127.0.0.1:8080")

    try:
        start_cli(client_manager, commands_manager)
    except KeyboardInterrupt:
        print("\n[*] Arrêt du C2.")

if __name__ == "__main__":
    main()
