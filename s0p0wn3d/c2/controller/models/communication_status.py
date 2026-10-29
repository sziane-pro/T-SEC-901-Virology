from dataclasses import dataclass
import datetime

@dataclass
class CommunicationStatus:
    last_result: str
    last_seen: datetime
    consecutive_errors: int
