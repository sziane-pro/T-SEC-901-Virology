from dataclasses import dataclass
from datetime import datetime

@dataclass
class ClientIdentity:
    client_id: str
    registered_at: datetime
    ip_private: str
