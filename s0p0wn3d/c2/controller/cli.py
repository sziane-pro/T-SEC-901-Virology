from controller.models.client_identity import ClientIdentity
from controller.clients import ClientManager
from controller.commands import CommandManager
from controller.network import network_check
from datetime import datetime
import questionary

def start_cli(manager, command_manager):
    while True:
        choices = questionary.select(
            "Select an option:",
            choices=["Afficher les informations réseau", "Lister les agents", "Générer un agent", "Envoyer une commande", "Quitter"]
        ).ask()
        if choices is None:
            raise SystemExit(1)
            break
        # elif choices == "Afficher les informations client":
        #     client = manager.get_client_info()
        #     print(f"Client ID: {client.client_id}")
        #     print(f"Registered At: {client.registered_at}")
        #     print(f"Private IP: {client.ip_private}")
        #     continue
        elif choices == "Afficher les informations réseau":
            url = questionary.text(
                "url :",
                default="https://google.com",       
                validate=lambda val: "http" in val or "https" in val or "URL invalide", 
                qmark="?",                        
                instruction="(tapez votre url)",
            ).ask()
            timeout = questionary.text(
                "timeout :",
                default="10",       
                validate=lambda val: val.isdigit() or "Timeout invalide", 
                qmark="?",                        
                instruction="(tapez votre timeout)",
            ).ask()
            network_status = network_check(url, timeout=int(timeout))
            if network_status is True:
                print("Network is up and running.")
            elif network_status is False:
                print("Network is down.")
            else:
                print(network_status)
            continue
        elif choices == "Lister les agents":
            clients = manager.get_clients_summary()
            if not clients:
                print("Aucun agent disponible")
            for c in clients:
                statut_icone = "🟢 En ligne" if c["is_active"] else "🔴 Hors ligne"
                print(f"[{c['client_id'][:8]}] | {statut_icone} | Dernière vue: {c['last_seen']}")
            continue
        elif choices == "Générer un agent":
            client = questionary.text("Nom de l'agent : ").ask()
            key = questionary.text("Définissez votre shared_key : ").ask()
            manager.create_client(client, key)
            print(f"[+] Agent `{client}` créer avec succès")
            print(f"[+] Clé `{key}` créer avec succès")
            continue
        elif choices == "Envoyer une commande":
            client_id = questionary.text("ID de l'agent cible : ").ask()
            command = questionary.text("Commande à exécuter (ex: whoami) : ").ask()
            command_id = command_manager.generate_command_id()
            command_manager.create_command(command_id, client_id, command)
            command_manager.queue_command(client_id, command_id, command)
            print(f"[+] Commande '{command}' mise en file pour l'agent {client_id}.")
            print("[*] Elle sera envoyée au prochain beacon.")
            continue
        elif choices == "Quitter":
            print("Exiting the program.")
            break
