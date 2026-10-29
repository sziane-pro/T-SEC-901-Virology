from dataclasses import dataclass
import datetime

@dataclass
class CommandResult:
    command_id: str
    result: str
    completed_at: datetime
