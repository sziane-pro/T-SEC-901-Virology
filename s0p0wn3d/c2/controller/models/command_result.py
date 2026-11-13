from dataclasses import dataclass
import datetime

@dataclass
class CommandResult:
    client_id: str
    command_id: str
    result: str
    status: str
    completed_at: datetime.datetime
