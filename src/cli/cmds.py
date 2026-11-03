import argparse
import os
import socket
import sys
from cli import _core

DEFAULT_CONTROL_SOCKET = "/run/acc.sock"

def control_socket_path():
    return os.environ.get("ACC_SOCKET", DEFAULT_CONTROL_SOCKET)

def port_number(text):
    try:
        port = int(text)
    except ValueError:
        raise argparse.ArgumentTypeError(f"invalid port: {text!r}")
    if not 1 <= port <= 65535:
        raise argparse.ArgumentTypeError("port must be from 1 to 65535")
    return port

def send_command(*parts, timeout=30.0):
    """Send one command line to the daemon's control socket.

    Returns (ok, lines): on success the payload lines, on failure the error text.
    """
    # The protocol is one space-separated line, so arguments cannot contain whitespace.
    for part in parts:
        if not part or any(c.isspace() or not c.isprintable() for c in part):
            raise RuntimeError(f"invalid argument: {part!r}")

    path = control_socket_path()
    try:
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
            s.settimeout(timeout)
            s.connect(path)
            s.sendall((" ".join(parts) + "\n").encode())
            chunks = []
            while chunk := s.recv(4096):
                chunks.append(chunk)
    except (FileNotFoundError, ConnectionRefusedError):
        raise RuntimeError(f"cannot reach the ACC daemon at {path} (is accd running?)")

    lines = b"".join(chunks).decode(errors="replace").splitlines()
    if not lines:
        raise RuntimeError("the daemon closed the connection without replying")
    if lines[0] == "OK":
        return True, lines[1:]
    return False, [lines[0].removeprefix("ERR ")]

def run_app_command(args, *parts, done=None):
    ok, lines = send_command(*parts)
    if not ok:
        print(f"error: {lines[0]}", file=sys.stderr)
        return 1
    if done:
        print(done)
    for line in lines:
        print(line)
    return 0

def run_add(args):
    return run_app_command(args, "add", args.name, args.image, args.hostname,
                           str(args.port), done=f"{args.name}: registered")

def run_up(args):
    return run_app_command(args, "up", args.name, done=f"{args.name}: started")

def run_down(args):
    return run_app_command(args, "down", args.name, done=f"{args.name}: stopped")

def run_rm(args):
    return run_app_command(args, "rm", args.name, done=f"{args.name}: removed")

def run_status(args):
    return run_app_command(args, "status", args.name)

def run_list(args):
    return run_app_command(args, "list")

def register_app(services):
    svc = services.add_parser("app", help="Manage hosted apps")
    cmds = svc.add_subparsers(dest="command", required=True)

    add = cmds.add_parser("add", help="Register an app (does not start it)")
    add.add_argument("name")
    add.add_argument("image", help="image that already exists on the server, e.g. nginx:latest")
    add.add_argument("hostname", help="Host header that routes to this app")
    add.add_argument("port", type=port_number, help="port the app listens on inside its container")
    add.set_defaults(func=run_add)

    up = cmds.add_parser("up", help="Start an app's container")
    up.add_argument("name")
    up.set_defaults(func=run_up)

    down = cmds.add_parser("down", help="Stop an app's container")
    down.add_argument("name")
    down.set_defaults(func=run_down)

    rm = cmds.add_parser("rm", help="Remove an app's container and unregister it")
    rm.add_argument("name")
    rm.set_defaults(func=run_rm)

    status = cmds.add_parser("status", help="Show an app's registration and container state")
    status.add_argument("name")
    status.set_defaults(func=run_status)

    ls = cmds.add_parser("list", help="List registered apps")
    ls.set_defaults(func=run_list)

def register(services):
    register_app(services)
    svc = services.add_parser("debug", help="HTTP operations")
    cmds = svc.add_subparsers(dest="command", required=True)

    ping = cmds.add_parser("ping", help="Send a GET request and print the reposne")
    ping.add_argument("--host", default="localhost")
    ping.add_argument("--port", type=int, default=8080)
    ping.add_argument("--path", default="/")
    ping.add_argument("--timeout", type=float, default=5.0)
    ping.set_defaults(func=run_ping)

def run_ping(args):
    with socket.create_connection((args.host, args.port), timeout=args.timeout) as s:
        s.setblocking(True)
        _core.http_ping(s.fileno(), args.host, args.path)
        s.settimeout(args.timeout)
        chunks = []
        while chunk := s.recv(4096):
            chunks.append(chunk)
    print(b"".join(chunks).decode(errors="replace"))
    return 0