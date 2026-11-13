from dataclasses import dataclass
from datetime import datetime
from typing import Optional

@dataclass
class Command:
    command_id: str
    client_id: str
    action_type: str
    created_at: datetime
    send_at: Optional[datetime] = None
