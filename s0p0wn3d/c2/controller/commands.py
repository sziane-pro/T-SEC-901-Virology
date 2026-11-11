import uuid
from datetime import datetime
from controller.models.command import Command
from controller.clients import ClientManager
from .command_catalog import KNOWN_ACTIONS
from . import audit


def make_command(client_id: str, action_type: str) -> Command:
    """Crée une nouvelle commande pour le client spécifié ou pour tous les clients (broadcast)."""
    if action_type not in KNOWN_ACTIONS:
        raise ValueError(f"Action inconnue: {action_type}")
    return Command(
        command_id=str(uuid.uuid4()),
        client_id=client_id,
        action_type=action_type,
        status="pending",
        created_at=datetime.utcnow(),
        send_at=None,
    )

def enqueue_command(cmd: Command, client_manager: ClientManager) -> None:
    """Ajoute une commande à la file d'attente du client ou à la file globale (broadcast)."""
    with client_manager._lock:
        if cmd.client_id:
            client_manager.pending_commands[cmd.client_id].append(cmd)
        else:
            client_manager.command_queue.append(cmd)
    audit.command_enqueued(cmd)


def pop_command_for(client_id: str, client_manager: ClientManager) -> Command | None:
    """Retourne la prochaine commande en attente pour le client, ou None.
    Priorité : file du client, puis file globale (broadcast).
    La commande retournée passe en statut "in_progress".
    """
    with client_manager._lock:
        cmd = None
        queue = client_manager.pending_commands.get(client_id)
        if queue:
            cmd = queue.popleft()
        elif client_manager.command_queue:
            cmd = client_manager.command_queue.popleft()

        if cmd is not None:
            cmd.status = "in_progress"
            cmd.send_at = datetime.utcnow()

    if cmd is not None:
        audit.command_sent(cmd)
    return cmd