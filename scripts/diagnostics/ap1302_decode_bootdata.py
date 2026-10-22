"""Read-only structural analysis of AP1302 record-stream bootdata.

The record and register-list encodings are inferred from supplied binaries.
This does not produce flashable firmware or interpret opaque type-3 data.
"""
import argparse
import hashlib
import json
import struct
import zlib
from pathlib import Path


def u32(data, offset):
    return struct.unpack_from('>I', data, offset)[0]


def decode(data):
    records = []
    offset = 0
    while offset + 4 <= len(data):
        header = u32(data, offset)
        if header == 0:
            break
        kind, size = header >> 24, header & 0xffffff
        if size < 4 or offset + size > len(data):
            raise ValueError(f'invalid record at {offset:#x}: {header:#010x}')
        record = {'offset': offset, 'kind': kind, 'bytes': size,
                  'sha256': hashlib.sha256(data[offset:offset+size]).hexdigest()}
        if kind == 5:
            if size < 12 or u32(data, offset + 8) != size - 12:
                raise ValueError(f'invalid register payload length at {offset:#x}')
            cursor, end = offset + 12, offset + size
            writes = []
            while cursor + 4 <= end:
                token, value = struct.unpack_from('>HH', data, cursor)
                cursor += 4
                if token & 1:
                    count = value
                    if not count or cursor + count * 2 > end:
                        raise ValueError(f'invalid register block at {cursor-4:#x}')
                    values = struct.unpack_from('>' + 'H' * count, data, cursor)
                    cursor += count * 2
                else:
                    values = (value,)
                address = token & ~1
                writes.append({'address': f'0x{address:04x}',
                               'words': [f'0x{v:04x}' for v in values]})
            record.update(writes=writes, padding_hex=data[cursor:end].hex())
        elif kind == 3:
            if size < 12:
                raise ValueError('short type-3 record')
            record['destination_or_descriptor'] = f'0x{u32(data,offset+8):08x}'
            record['opaque_bytes'] = size - 12
        elif kind == 6:
            record['payload_hex'] = data[offset+4:offset+size].hex()
        records.append(record)
        offset += size
    tail = data[offset:]
    return {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest(),
            'records': records, 'record_stream_bytes': offset,
            'tail_hex': tail.hex(),
            'zlib_crc32_before_tail': f'{zlib.crc32(data[:offset]):08x}',
            'zlib_crc32_before_last_word': f'{zlib.crc32(data[:-4]):08x}'}


def final_words(result):
    words = {}
    for record in result['records']:
        for write in record.get('writes', []):
            address = int(write['address'], 16)
            for index, value in enumerate(write['words']):
                words[f'0x{address + index * 2:04x}'] = value
    return words


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--compare', type=Path, nargs='*', default=[])
    args = parser.parse_args()
    result = decode(args.input.read_bytes())
    current = final_words(result)
    result['final_register_words'] = current
    result['comparisons'] = []
    for reference in args.compare:
        other = decode(reference.read_bytes())
        reference_words = final_words(other)
        result['comparisons'].append({
            'path': str(reference), 'sha256': other['sha256'],
            'records': other['records'],
            'register_differences': {
                address: {'installed': current.get(address),
                          'reference': reference_words.get(address)}
                for address in sorted(current.keys() | reference_words.keys())
                if current.get(address) != reference_words.get(address)}})
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'bytes': result['bytes'],
                      'record_count': len(result['records']),
                      'register_words': len(current), 'tail_hex': result['tail_hex'],
                      'crc32': result['zlib_crc32_before_last_word'],
                      'comparisons': len(result['comparisons'])}))
