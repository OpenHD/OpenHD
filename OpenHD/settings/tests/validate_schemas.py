#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import generate
import jsonschema

parser=argparse.ArgumentParser()
parser.add_argument('fixtures',type=Path)
parser.add_argument('upstream',type=Path)
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
load=lambda path:json.loads(path.read_text())
catalog=load(root/'catalog.json');generate.validate(catalog)
jsonschema.validate(catalog,load(root/'schema.json'))
jsonschema.validate(load(args.fixtures/'registry.json'),load(root/'schema.json'))
jsonschema.validate(load(args.fixtures/'general.json'),load(args.upstream/'general.schema.json'))
jsonschema.validate(load(args.fixtures/'parameters.json'),load(args.upstream/'parameter.schema.json'))
draft=load(root/'upstream/parameter-v4.schema.json')
jsonschema.Draft7Validator.check_schema(draft)
jsonschema.validate({'version':4,'parameters':[{'name':'STRING_TEST','type':'String','maxLength':128,'default':'auto',
    'values':[{'value':'auto','description':'Automatic'}]}]},draft)
for p in [{'name':'BAD_STRING','type':'String','default':1},
          {'name':'BAD_NUMERIC','type':'Int32','default':'text'},
          {'name':'BAD_LENGTH','type':'String','maxLength':129}]:
    try: jsonschema.validate({'version':4,'parameters':[p]},draft)
    except jsonschema.ValidationError: pass
    else: raise AssertionError('Invalid draft fixture accepted')
print('CATALOG, CURRENT MAVLINK SCHEMAS, AND UPSTREAM DRAFT PASSED')
