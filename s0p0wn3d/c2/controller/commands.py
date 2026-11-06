# server/commands.py
import uuid
from datetime import datetime
from controller.models.command import Command
from controller.models.communication_status import CommunicationStatus
from .state import SERVER_STATE

KNOWN_ACTIONS = {"ping", "list_processes", "screenshot", "exec"}

def make_command(client_id: str, action_type: str) -> Command:
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
    
def enqueue_command(cmd) -> None:
    with SERVER_STATE.lock:
        if cmd.client_id:
            SERVER_STATE.pending_commands[cmd.client_id].append(cmd)
        else:
            SERVER_STATE.command_queue.append(cmd)

def pop_command_for(client_id: str, client_manager):
    """Retourne la commande en attente pour le client"""
    with SERVER_STATE.lock:
        queue = SERVER_STATE.pending_commands.get(client_id)
        if queue:
            return queue.popleft()
        if SERVER_STATE.command_queue:
            return SERVER_STATE.command_queue.popleft()

    # mise à jour du statut de communication
    status = client_manager.statuses.get(client_id)
    if status:
        status.last_seen = datetime.now()
        status.consecutive_errors = 0
    return None

def update_comm_status(client_id: str, error: bool = False) -> None:
    """Met à jour last_seen / consecutive_errors (utile pour la supervision)."""
    with SERVER_STATE.lock:
        status = SERVER_STATE.comm_status.setdefault(
            client_id,
            CommunicationStatus(
                client_id=client_id,
                last_seen=datetime.utcnow(),
            ),
        )
        status.last_seen = datetime.utcnow()
        status.consecutive_errors = status.consecutive_errors + 1 if error else 0