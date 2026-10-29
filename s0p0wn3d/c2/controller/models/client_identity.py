from dataclasses import dataclass
import datetime

@dataclass
class ClientIdentity:
    client_id: str
    registered_at: datetime.datetime
    ip_private: str
