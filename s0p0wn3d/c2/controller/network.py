import requests

def network_check(url, timeout):
    try:
        r = requests.get(url, timeout=timeout)
        if r.status_code == requests.codes.ok:
            return True, "Network is up and running."
        else:
            return False, f"HTTP Error: {r.status_code}"
    except requests.exceptions.RequestException as e:
        return False, f"Network check failed: {e}"
