from controller.models.client_identity import ClientIdentity
from controller.auth import generate_session_token, generate_client_id, generate_shared_key
from controller.models.session import Session
from controller.models.communication_status import CommunicationStatus
from datetime import datetime

class ClientManager:
    def __init__(self):
        self.clients = {}
        self.whitelist = {}
        self.session = {}
        self.statuses = {}
        self.sessions_by_token = {}

    def generate_client_credentials(self):
        client_id = generate_client_id()
        shared_key = generate_shared_key()
        
        self.whitelist[client_id] = shared_key
        return client_id, shared_key
    
    def register_client(self, client_id, shared_key, ip_private):

        if not client_id or not shared_key or not ip_private:
            return False, "Argument(s) is missing"
        
        if self.whitelist.get(client_id) != shared_key:
            return False, "Not Authorized"
        
        if client_id not in self.clients:
            self.clients[client_id] = ClientIdentity(
                client_id=client_id,
                registered_at=datetime.now(),
                ip_private=ip_private
            )

        session_token = generate_session_token()
        if session_token not in self.session:
            self.session[client_id] = Session(
                client_id=client_id,
                session_token=session_token,
                created_at=datetime.now(),
                revoked=False
            )
            
        self.sessions_by_token[session_token] = client_id
        
        if client_id not in self.statuses:
            self.statuses[client_id] = CommunicationStatus(
                client_id=client_id,
                last_seen=datetime.now()
            )
        
        return session_token

    def get_client_id_from_token(self, session_token: str) -> str | None:
        return self.sessions_by_token.get(session_token)
    
    def get_client_info(self, client_id: str) -> ClientIdentity | None:
        """Retourne l'identité du client, ou None s'il n'est pas enregistré."""
        return self.clients.get(client_id)
    
    # def get_client_info(self):
    #     client = ClientIdentity(
    #         client_id="client_1",
    #         registered_at=datetime.now(),
    #         ip_private="192.168.1.100"
    #     )
    #     return client

