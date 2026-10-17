import argparse

def main(argv=None):
    parser = argparse.ArguementParser(prog="acc")
    services = parser.add_subparsers(dest="service", required=True)
    

if __name__ == '__main__':
    raise SystemExit(main())