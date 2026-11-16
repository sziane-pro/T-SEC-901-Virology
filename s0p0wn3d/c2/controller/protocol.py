from dataclasses import dataclass, asdict

@dataclass
class RegisterRequest:
    client_id: str
    shared_key: str
    ip_private: str

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class RegisterResponse:
    status: str
    session_token: str

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class BeaconRequest:
    client_id: str

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class BeaconResponse:
    command_id: str
    command: str

    def to_dict(self) -> dict:
        return asdict(self)

@dataclass
class CommandResult:
    command_id: str
    command_response: str
    status: str

    def to_dict(self) -> dict:
        return asdict(self)
