from controller.models.client_identity import ClientIdentity
import datetime
import questionary

choices = questionary.select(
    "Select an option:",
    choices=["Afficher les informations client", "Quitter"]
).ask()

client = ClientIdentity(
    client_id="client_1",
    registered_at=datetime.datetime.now(),
    ip_private="192.168.1.100"
)

if choices == "Afficher les informations client":
    print(f"Client ID: {client.client_id}")
    print(f"Registered At: {client.registered_at}")
    print(f"Private IP: {client.ip_private}")
if choices == "Quitter":
    print("Exiting the program.")
if choices is None:   # Ctrl+C
    raise SystemExit(1)


