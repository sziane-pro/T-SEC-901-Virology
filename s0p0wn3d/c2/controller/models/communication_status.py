from dataclasses import dataclass
from datetime import datetime
from typing import Optional

@dataclass
class CommunicationStatus:
    client_id: str
    last_seen: datetime
    last_result: Optional[str] = None
    consecutive_errors: int = 0

    def is_active(self, threshold_seconds: int = 30) -> bool:
        delta = (datetime.now() - self.last_seen).total_seconds()
        # Si le temps écoulé est inférieur à 30 secondes, c'est actif
        return delta < threshold_seconds
