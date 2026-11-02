from controller.models.client_identity import ClientIdentity
from controller.clients import get_client_info
from controller.network import network_check
from datetime import datetime
import questionary

def start_cli():
    while True:
        choices = questionary.select(
            "Select an option:",
            choices=["Afficher les informations client", "Afficher les informations réseau", "Quitter"]
        ).ask()
        if choices is None:
            raise SystemExit(1)
            break
        elif choices == "Afficher les informations client":
            client = get_client_info()
            print(f"Client ID: {client.client_id}")
            print(f"Registered At: {client.registered_at}")
            print(f"Private IP: {client.ip_private}")
            continue
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
        elif choices == "Quitter":
            print("Exiting the program.")
            break
