from dataclasses import dataclass
from datetime import datetime

@dataclass
class Session:
    client_id: str
    session_token: str
    created_at: datetime
    revoked: bool = False
