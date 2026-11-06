import threading
from collections import defaultdict, deque

class ServerState:
    def __init__(self):
        self.lock = threading.RLock()
        self.pending_commands = defaultdict(deque)  # client_id -> deque[Command]
        self.command_queue = deque()                # commandes globales (broadcast)
        self.sessions = {}                          # session_token -> Session
        self.comm_status = {}                       # client_id -> CommunicationStatus

SERVER_STATE = ServerState()