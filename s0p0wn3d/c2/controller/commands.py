from controller.models.command_result import CommandResult
from controller.models.command import Command
from datetime import datetime
import queue
import uuid

class CommandManager:
    def __init__(self, client_manager):
        self.client_manager = client_manager # référence vers le ClientManager
        self.commands = {}
        self.command_results = {}
        self.command_queues = {}
    
    def generate_command_id(self):
        return str(uuid.uuid4())

    def queue_command(self, client_id, command_id, command):

        if not client_id or not command_id or not command:
            return "One or more required arguments are missing."
        
        if client_id not in self.command_queues:
            self.command_queues[client_id] = queue.Queue()
        
        self.command_queues[client_id].put({
            "command_id": command_id,
            "action_type": command
        })

    def get_next_command(self, client_id):
        
        if not client_id:
            return None

        if client_id in self.command_queues:
            try:
                return self.command_queues[client_id].get_nowait()
            except queue.Empty:
                return None
        return None

    def create_command(self, command_id, client_id, action_type):
        if not command_id or not client_id or not action_type:
            return None
        
        if command_id not in self.commands:
            self.commands[command_id] = Command(
                command_id=command_id,
                client_id=client_id,
                action_type=action_type,
                created_at=datetime.now(),
            )
    
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

    def get_command_result(self, client_id, command_id):
        return self.command_results.get(client_id, {}).get(command_id, None)

    def get_all_command_results_for_client(self, client_id):
        return self.command_results.get(client_id, {})
