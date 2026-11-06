# server/sessions.py
import secrets
from datetime import datetime, timezone
from controller.models.session import Session
from controller.models.client_identity import ClientIdentity
from .state import SERVER_STATE
from controller.models.session import Session

SESSION_TTL_SECONDS = 3600

def init_session(ip_private: str, client_manager) -> dict:
    """
    Crée un nouveau client + session. Retourne un dict prêt pour la réponse HTTP.
    """
    client_id, shared_key = client_manager.generate_client_credentials()
    session_token = client_manager.register_client(client_id, shared_key, ip_private)

    return {
        "client_id": client_id,
        "shared_key": shared_key,
        "session_token": session_token,
        "registered_at": client_manager.clients[client_id].registered_at.isoformat(),
    }
    
    
def identify_client(session_token: str | None,
                    client_id: str | None,
                    client_manager) -> str:
    """
    Résout client_id depuis session_token (prioritaire) ou client_id direct.
    Lève ValueError si non identifiable / expiré / révoqué.
    """
    if session_token:
        cid = client_manager.get_client_id_from_token(session_token)
        if cid is None:
            raise ValueError("Session inconnue")
        session: Session = client_manager.session[cid]
        if session.revoked:
            raise ValueError("Session révoquée")
        if (datetime.now() - session.created_at).total_seconds() > SESSION_TTL_SECONDS:
            session.revoked = True
            raise ValueError("Session expirée")
        return cid

    if client_id and client_manager.is_registered(client_id):
        return client_id

    raise ValueError("Impossible d'identifier le client")