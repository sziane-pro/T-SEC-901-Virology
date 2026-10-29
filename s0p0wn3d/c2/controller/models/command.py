from dataclasses import dataclass
import datetime

@dataclass
class Commmand:
    command_id: str
    client_id: str
    action_type: str    
    status: str
    created_at: datetime
    send_at: datetime
