from flask import Flask, request, jsonify
from datetime import datetime
import requests
from controller.clients import ClientManager
from controller.commands import CommandManager
from controller.protocol import RegisterRequest, RegisterResponse, BeaconResponse, BeaconRequest, CommandResult

app = Flask(__name__)
manager = None
command_manager = None

def setup_network(client_manager_instance, command_manager_instance):
    global manager, command_manager
    manager = client_manager_instance
    command_manager = command_manager_instance

@app.route("/register", methods=["POST"])
def register():
    data = request.get_json()

    req = RegisterRequest(
        client_id=data.get("client_id"),
        shared_key=data.get("shared_key"),
        ip_private=data.get("ip_private"),
    )

    session_token = manager.register_client(
        client_id=req.client_id,
        shared_key=req.shared_key,
        ip_private=req.ip_private
    )

    if session_token:
        response=RegisterResponse(status="OK", session_token=session_token)
        return jsonify(response.to_dict()), 200
    else:
        response=RegisterResponse(status="UNAUTHORIZED", session_token="")
        return jsonify(response.to_dict()), 401

@app.route("/beacon", methods=["POST"])
def beacon():
    data = request.get_json()

    if not data or "client_id" not in data:
        return jsonify({"error": "Missing client_id"}), 400

    req = BeaconRequest(client_id=data.get("client_id"))
    
    session_token = request.headers.get("X-Session-Token")
    if not session_token:
        return jsonify({"error": "Missing session token"}), 401

    is_valid_session = manager.verify_session(client_id=req.client_id, session_token=session_token)
    if not is_valid_session:
        return jsonify({"error": "Session token is invalid"}), 401

    manager.update_last_seen(client_id=req.client_id)

    commande = command_manager.get_next_command(req.client_id)
    
    if commande is not None:
        response = BeaconResponse(
            command_id=commande["command_id"],
            command=commande["action_type"]
        )

        command_manager.commands[commande["command_id"]].send_at = datetime.now()
    else:
        response = BeaconResponse(command_id="", command="")

    return jsonify(response.to_dict()), 200

@app.route("/result", methods=["POST"])
def result():

    session_token = request.headers.get("X-Session-Token")
    if not session_token:
        return jsonify({"error": "Missing session token"}), 401

    client_id = manager.get_client_id_by_token(session_token)
    if not client_id:
        return jsonify({"error": "Unauthorized"}), 401

    data = request.get_json()

    REQUIRED_FIELDS = ("command_id", "command_response", "status")

    if not data or not all(field in data for field in REQUIRED_FIELDS):
        return jsonify({"One or more required fields are missing."}), 400

    req = CommandResult(
        command_id=data.get("command_id"),
        command_response=data.get("command_response"),
        status=data.get("status")
    )

    manager.update_last_seen(client_id=client_id)

    success = command_manager.save_command_result(
        client_id=client_id,
        command_id=data["command_id"],
        result=data["command_response"],
        status=data["status"]
    )

    if success:
        return jsonify({"status": "ACK"}), 200
    else:
        return jsonify({"error": "Unknown command"}), 404

def network_check(url, timeout):
    try:
        r = requests.get(url, timeout=timeout)
        if r.status_code == requests.codes.ok:
            return True, "Network is up and running."
        else:
            return False, f"HTTP Error: {r.status_code}"
    except requests.exceptions.RequestException as e:
        return False, f"Network check failed: {e}"
