import argparse
import json
import sys

parser = argparse.ArgumentParser(
    description='Check if a json represents a succeeding test')
parser.add_argument('filename')
args = parser.parse_args()

with open(args.filename) as f:
    j = json.load(f)

sys.exit(j['exitcode'])