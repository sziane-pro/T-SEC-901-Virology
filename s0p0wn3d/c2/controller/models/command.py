from dataclasses import dataclass
import datetime
from typing import Optional

@dataclass
class Command:
    command_id: str
    client_id: str
    action_type: str    
    status: str
    created_at: datetime.datetime
    send_at: Optional[datetime.datetime] = None
