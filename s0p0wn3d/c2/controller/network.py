import requests
import json
import logging
from http.server import BaseHTTPRequestHandler

def network_check(url, timeout):
    try:
        r = requests.get(url, timeout=timeout)
        if r.status_code == requests.codes.ok:
            return True, "Network is up and running."
        else:
            return False, f"HTTP Error: {r.status_code}"
    except requests.exceptions.RequestException as e:
        return False, f"Network check failed: {e}"

class Handler(BaseHTTPRequestHandler):
    """Couche transport"""
    dispatcher = None
    
    def log_message(self, format, *args):
        """Override the default logging to use the logging module."""
        logging.info("%s - %s", self.address_string(), format % args)

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length)
        try:
            code, payload = self.dispatcher(raw, self.client_address[0])
        except Exception:
            logging.exception("Erreur dans le dispatcher")
            code, payload = 500, {"error": "internal error"}
        self._reply(code, payload)

    def _reply(self, code, payload):
        body = json.dumps(payload).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

def make_handler(dispatcher):
    class BoundHandler(Handler):
        pass
    BoundHandler.dispatcher = dispatcher
    return BoundHandler