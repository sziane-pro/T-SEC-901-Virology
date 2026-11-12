import logging

def command_enqueued(cmd):
    logging.info("Command enqueued id=%s client=%s action=%s",
                 cmd.command_id, cmd.client_id, cmd.action_type)

def command_sent(cmd):
    logging.info("Command sent id=%s client=%s", cmd.command_id, cmd.client_id)

def command_result(cmd, result):
    logging.info("Command result id=%s status=%s", cmd.command_id, cmd.status)

def client_error(client_id, exc):
    logging.warning("Client error id=%s: %s", client_id, exc)