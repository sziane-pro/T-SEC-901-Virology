import secrets
import uuid

def generate_session_token():
    return secrets.token_hex(32)

def generate_client_id():
    return str(uuid.uuid4())

def generate_shared_key():
    return secrets.token_hex(16)
