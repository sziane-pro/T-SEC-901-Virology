from controller.models.client_identity import ClientIdentity
from controller.auth import generate_session_token, generate_client_id, generate_shared_key
from controller.models.session import Session
from controller.models.communication_status import CommunicationStatus
import hmac
from datetime import datetime, timedelta

SESSION_TTL = timedelta(hours=1)

class ClientManager:
    def __init__(self):
        self.clients = {}
        self.whitelist = {}
        self.session = {}
        self.statuses = {}

    def generate_client_credentials(self):
        client_id = generate_client_id()
        shared_key = generate_shared_key()
        
        self.whitelist[client_id] = shared_key
        return client_id, shared_key
    
    def register_client(self, client_id, shared_key, ip_private):

        if not client_id or not shared_key or not ip_private:
            return False, "Argument(s) is missing"
        
        if client_id not in self.whitelist or self.whitelist[client_id] != shared_key:
            return None
        
        if client_id not in self.clients:
            self.clients[client_id] = ClientIdentity(
                client_id=client_id,
                registered_at=datetime.now(),
                ip_private=ip_private,
            )

        session_token = generate_session_token()
        if session_token not in self.session:
            self.session[client_id] = Session(
                client_id=client_id,
                session_token=session_token,
                created_at=datetime.now(),
                revoked=False,
            )
        
        if client_id not in self.statuses:
            self.statuses[client_id] = CommunicationStatus(
                client_id=client_id,
                last_seen=datetime.now(),
            )
        
        #print(session_token)
        return session_token

    def create_client(self, client_id, shared_key):
        if not client_id or not shared_key:
            return False
        
        if client_id not in self.whitelist:
            self.whitelist[client_id] = shared_key

    def verify_session(self, client_id, session_token):
        
        if not client_id or not session_token:
            print("Missing one or many arguments")
        
        session = self.session.get(client_id)
        if session is None:
            return False
        
        if session.revoked:
            return False
        
        if datetime.now() - session.created_at > SESSION_TTL:
            del self.session[client_id]
            return False
        
        return hmac.compare_digest(session.session_token, session_token)

    def update_last_seen(self, client_id):
       
        if not client_id:
            return "client id argument is missing"
        
        if client_id in self.statuses:
            self.statuses[client_id].last_seen = datetime.now()

    def update_last_result(self, client_id, result):
       
        if not client_id or not result:
            return "Missing one or many arguments"
        
        if client_id in self.statuses:
            self.statuses[client_id].last_result = result
    
    def get_client_id_by_token(self, session_token):
        
        if not session_token:
            return None
        
        session = self.session.get(session_token)
        if session is None:
            return None
        
        if session.client_id:
            return session.client_id
        
    def get_clients_summary(self) -> list:
        summary = []
        for client_id, identity in self.clients.items():
            
            status = self.statuses.get(client_id)
            
            is_active = status.is_active() if status else False
            
            summary.append({
                "client_id": client_id,
                "is_active": is_active,
                "registered_at": identity.registered_at.strftime("%Y-%m-%d %H:%M:%S"),
                "last_seen": status.last_seen.strftime("%H:%M:%S") if status else "Jamais"
            })

        return summary
