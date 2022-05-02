#!/usr/bin/env python
'''
parse a MAVLink protocol XML file and generate a Wireshark LUA dissector
'''
from __future__ import print_function

from builtins import range

import os

def generate(basename, xml):
    '''generate complete python implemenation'''
    if basename.endswith('.lua'):
        filename = basename
    else:
        filename = basename + '.lua'

    msgs = []
    enums = []
    filelist = []
    for x in xml:
        msgs.extend(x.message)
        enums.extend(x.enum)
        filelist.append(os.path.basename(x.filename))

    unpack_types = {
        'float'    : 'f',
        'double'   : 'd',
        'char'     : 'c',
        'int8_t'   : 'b',
        'uint8_t'  : 'B',
        'uint8_t_mavlink_version'  : 'B',
        'int16_t'  : 'i2',
        'uint16_t' : 'I2',
        'int32_t'  : 'i4',
        'uint32_t' : 'I4',
        'int64_t'  : 'i8',
        'uint64_t' : 'I8',
        }


    print("Generating %s" % filename)
    outf = open(filename, "w")
    outf.write(
"""function decode_header(message)
  -- build up a map of the result
  local result = {}

  local read_marker = 3

  -- id the MAVLink version
  result.protocol_version, read_marker = string.unpack("<B", message, read_marker)
  if (result.protocol_version == 0xFE) then -- mavlink 1
    result.protocol_version = 1
  elseif (result.protocol_version == 0XFD) then --mavlink 2
    result.protocol_version = 2
  else
    error("Invalid magic byte")
  end

  local payload_len, read_marker = string.unpack("<B", message, read_marker) -- payload is always the second byte

  -- strip the incompat/compat flags
  result.incompat_flags, result.compat_flags, read_marker = string.unpack("<BB", message, read_marker)

  -- fetch seq/sysid/compid
  result.seq, result.sysid, result.compid, read_marker = string.unpack("<BBB", message, read_marker)

  -- fetch the message id
  result.msgid, read_marker = string.unpack("<I3", message, read_marker)

  return result, read_marker
end

function decode(message, messages)
  local result, offset = decode_header(message)
  local message_map = messages[result.msgid]
  if not message_map then
    -- we don't know how to decode this message, bail on it
    return nil
  end

  -- map all the fields out
  for i,v in ipairs(message_map) do
    if v[3] then
      result[v[1]] = {}
      for j=1,v[3] do
        result[v[1]][j], offset = string.unpack(v[2], message, offset)
      end
    else
      result[v[1]], offset = string.unpack(v[2], message, offset)
    end
  end

  -- ignore the idea of a checksum

  return result;
end

function encode(msgid, message, messages)
  local message_map = messages[msgid]   
  if not message_map then                 
    -- we don't know how to encode this message, bail on it
    error("Unknown MAVLink message " .. msgid)
  end

  local packString = "<"
  local packedTable = {}                  
  local packedIndex = 1
  for i,v in ipairs(message_map) do
    if v[3] then
      packString = (packString .. string.rep(string.sub(v[2], 2), v[3]))
      for j = 1, v[3] do
        packedTable[packedIndex] = message[message_map[i][1]][j]
        packedIndex = packedIndex + 1
      end
    else                          
      packString = (packString .. string.sub(v[2], 2))
      packedTable[packedIndex] = message[message_map[i][1]]
      packedIndex = packedIndex + 1
    end
  end

  return string.pack(packString, table.unpack(packedTable))
end

""")

    # dump the actual symbol table
    outf.write("messages = {}\n")
    for m in msgs:
        outf.write("messages[%s] = { -- %s\n" % (m.id, m.name))
        for i in range(0, len(m.ordered_fields)):
            field = m.ordered_fields[i]
            if (field.array_length > 0):
                outf.write("             { \"%s\", \"<%s\", %s },\n" % (field.name, unpack_types.get(field.type), field.array_length))
            else:
                outf.write("             { \"%s\", \"<%s\" },\n" % (field.name, unpack_types.get(field.type)))
        outf.write("             }\n")

    outf.close()
    print("Generated %s OK" % filename)

