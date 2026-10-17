import socket
from acc import _core

def register(services):
    svc = services.add_parser("http", help="HTTP operations")
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
        s.settimeout(args.timeout)
        _core.http_ping(s.fileno(), args.host, args.path)
        chunks = []
        while chunk := s.recv(4096):
            chunks.append(chunk)
    print(b"".join(chunks).decode(errors="replace"))
    return 0