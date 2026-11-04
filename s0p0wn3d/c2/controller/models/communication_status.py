from dataclasses import dataclass
from datetime import datetime
from typing import Optional

@dataclass
class CommunicationStatus:
    client_id: str
    last_seen: datetime
    last_result: Optional[str] = None
    consecutive_errors: int = 0
