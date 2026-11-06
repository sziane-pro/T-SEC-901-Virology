import json
from dataclasses import dataclass
from typing import Optional

class MessageError(Exception):
    pass

@dataclass
class IncomingMessage:
    session_token: Optional[str]
    client_id: Optional[str]
    output: Optional[str]
    raw: dict

def decode_message(raw_bytes: bytes) -> IncomingMessage:
    """Décode et valide le corps d'une requête POST."""
    try:
        data = json.loads(raw_bytes)
    except json.JSONDecodeError as e:
        raise MessageError(f"JSON invalide: {e}")

    if not isinstance(data, dict):
        raise MessageError("Le payload doit être un objet JSON")

    if data.get("action") == "init":
        return IncomingMessage(
            session_token=None,
            client_id=None,
            output=None,
            raw=data,
        )
        
    session_token = data.get("sid")
    client_id = data.get("client_id")
    output = data.get("output")
    
    if not session_token and not client_id:
        raise MessageError("No credentials")

    if output is not None and not isinstance(output, str):
        raise MessageError("'output' must be a string")

    return IncomingMessage(
        session_token=session_token,
        client_id=client_id,
        output=output,
        raw=data,
    )