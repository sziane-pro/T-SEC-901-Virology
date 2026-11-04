# server/main.py
from http.server import ThreadingHTTPServer
from server.http_handler import make_handler

def run_server(client_manager, host="0.0.0.0", port=8080):
    server = ThreadingHTTPServer((host, port), make_handler(client_manager))
    print(f"Serveur sur http://{host}:{port}")
    server.serve_forever()