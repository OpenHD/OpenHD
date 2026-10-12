#!/usr/bin/env python3
"""Create a reviewable string-metadata schema proposal from an upstream checkout."""
import argparse
import copy
import difflib
import json
import pathlib

def extend(source):
    schema=copy.deepcopy(source)
    schema['properties']['version']['minimum']=4
    parameter=schema['properties']['parameters']['items']
    parameter['required']=['name','type']
    props=parameter['properties']
    props['type']['enum'].append('String')
    props['default']['type']=['number','string']
    props['values']['items']['properties']['value']['type']=['number','string']
    props['maxLength']={'type':'integer','minimum':0,'maximum':128,
        'description':'Maximum UTF-8 string length in bytes, excluding an optional NUL terminator.'}
    parameter['allOf']=[{
        'if':{'properties':{'type':{'const':'String'}},'required':['type']},
        'then':{'properties':{
            'default':{'type':'string'},
            'values':{'items':{'properties':{'value':{'type':'string'}}}},
            'min':False,'max':False,'increment':False,'decimalPlaces':False,'bitmask':False}},
        'else':{'properties':{
            'default':{'type':'number'},
            'values':{'items':{'properties':{'value':{'type':'number'}}}},
            'maxLength':False}}
    }]
    return schema

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('upstream_schema',type=pathlib.Path)
    args=parser.parse_args()
    source=json.loads(args.upstream_schema.read_text())
    target=pathlib.Path(__file__).resolve().parents[1]/'upstream'
    before=json.dumps(source,indent=2)+'\n'
    after=json.dumps(extend(source),indent=2)+'\n'
    (target/'parameter-v4.schema.json').write_text(after)
    patch=''.join(difflib.unified_diff(before.splitlines(True),after.splitlines(True),
        fromfile='a/component_metadata/parameter.schema.json',tofile='b/component_metadata/parameter.schema.json'))
    (target/'parameter-string-metadata.patch').write_text(patch)
    print('Generated unsubmitted upstream schema proposal')

if __name__=='__main__': main()
