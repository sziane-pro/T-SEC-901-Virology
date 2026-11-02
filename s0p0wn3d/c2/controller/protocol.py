from dataclasses import dataclass

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
    command_id: str
    command: str

@dataclass
class CommandResult:
    command_id: str
    command_response: str
    status: str
