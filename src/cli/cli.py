import argparse
from cli import cmds

def main(argv=None):
    parser = argparse.ArgumentParser(prog="acc")
    services = parser.add_subparsers(dest="service", required=True)
    cmds.register(services)
    args = parser.parse_args(argv)
    try:
        return args.func(args)
    except (OSError, RuntimeError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    

if __name__ == '__main__':
    raise SystemExit(main())