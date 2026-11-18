from controller.models.command_result import CommandResult
from controller.models.command import Command
from datetime import datetime
import queue
import uuid

class CommandManager:
    def __init__(self, client_manager):
        self.client_manager = client_manager  # référence vers le ClientManager
        self.commands = {}                    # command_id -> Command (fiche complète)
        self.command_results = {}             # client_id -> {command_id: CommandResult}
        self.command_queues = {}              # client_id -> queue.Queue[Command]

    def generate_command_id(self):
        return str(uuid.uuid4())

    def queue_command(self, client_id, command_id, command):
        """Crée la commande et la met en file pour l'agent cible.

        Une seule opération : la fiche et la file ne peuvent pas diverger.
        """

        if not client_id or not command_id or not command:
            return "One or more required arguments are missing."

        cmd = Command(
            command_id=command_id,
            client_id=client_id,
            action_type=command,
            created_at=datetime.now(),
        )

        self.commands[command_id] = cmd
        self.command_queues.setdefault(client_id, queue.Queue()).put(cmd)
        return cmd

    def wait_for_command(self, client_id, timeout):
        """Long-poll : attend une commande jusqu'à `timeout` secondes.

        Retourne None si aucune commande n'arrive avant l'expiration de la tenue.
        """

        if not client_id:
            return None

        channel = self.command_queues.setdefault(client_id, queue.Queue())
        try:
            cmd = channel.get(timeout=timeout)
        except queue.Empty:
            return None

        cmd.send_at = datetime.now()  # horodatage posé au moment de la remise
        return cmd

    def save_command_result(self, client_id, command_id, result, status):

        if not client_id or not command_id or not result or not status:
            return "One or more required arguments are missing."

        if client_id not in self.command_results:
            self.command_results[client_id] = {}

        result_model = CommandResult(
            client_id=client_id,
            command_id=command_id,
            result=result,
            status=status,
            completed_at=datetime.now()
        )

        self.command_results[client_id][command_id] = result_model

        self.client_manager.update_last_seen(client_id)
        self.client_manager.update_last_result(client_id, result)

        return result_model

    def get_command_result(self, client_id, command_id):
        return self.command_results.get(client_id, {}).get(command_id, None)

    def get_all_command_results_for_client(self, client_id):
        return self.command_results.get(client_id, {})
