# Rewrite the title strings of an SMDH (all 16 languages) as proper UTF-16.
import sys
path, short, long_, pub = sys.argv[1:5]
d = bytearray(open(path, 'rb').read())
assert d[:4] == b'SMDH'
def put(off, size, s):
    b = s.encode('utf-16le'); assert len(b) < size
    d[off:off+size] = b + b'\0' * (size - len(b))
for i in range(16):
    base = 8 + i * 0x200
    put(base, 0x80, short); put(base + 0x80, 0x100, long_); put(base + 0x180, 0x80, pub)
open(path, 'wb').write(d)
