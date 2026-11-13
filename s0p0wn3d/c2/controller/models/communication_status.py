from dataclasses import dataclass
from datetime import datetime

@dataclass
class CommunicationStatus:
    client_id: str
    last_result: Optional[str] = None
    last_seen: datetime
    consecutive_errors: int = 0
