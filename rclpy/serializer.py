import struct
import numpy as np
from datatype import Datatype

class Serializer:
    def __init__(self):
        self.buffer = bytearray()

    def pack(self, data):
        tag = Datatype.get_tag(data)
        if isinstance(data, (list, np.ndarray)):
            # T
            self.buffer.append(Datatype.ARRAY)
            # L
            count = len(data) if isinstance(data, list) else data.size
            self.buffer.extend(struct.pack('<I', count))
            # V (recursive)
            items = data.flat if isinstance(data, np.ndarray) else data
            for item in items:
                self.pack(item)

        elif isinstance(data, str):
            # T
            self.buffer.append(Datatype.STRING)
            encoded = data.encode('utf-8')
            # L
            self.buffer.extend(struct.pack('<I', len(encoded)))
            # V
            self.buffer.extend(encoded)

        else:
            if tag in Datatype.FORMAT_MAP:
                # T
                self.buffer.append(tag)
                fmt = '<' + Datatype.FORMAT_MAP[tag]
                # V
                self.buffer.extend(struct.pack(fmt, data))


class Deserializer:
    def __init__(self, data: bytes):
        self.buffer = data
        self.offset = 0

    def unpack(self):
        if self.offset >= len(self.buffer):
            return None
        tag = self.buffer[self.offset]
        self.offset += 1

        if tag == Datatype.ARRAY:
            count = struct.unpack_from('<I', self.buffer, self.offset)[0]
            self.offset += 4
            return [self.unpack() for _ in range(count)]

        elif tag == Datatype.STRING:
            length = struct.unpack_from('<I', self.buffer, self.offset)[0]
            self.offset += 4
            val = self.buffer[self.offset:self.offset + length].decode('utf-8')
            self.offset += length
            return val

        elif tag in Datatype.FORMAT_MAP:
            fmt = '<' + Datatype.FORMAT_MAP[tag]
            val = struct.unpack_from(fmt, self.buffer, self.offset)[0]
            self.offset += struct.calcsize(fmt)
            return val

        return None

# Serialize = Serializer()
#
# Serialize.pack(123)
# Serialize.pack("dasda")
# Serialize.pack([123, 424])
# Deserialize = Deserializer(Serialize.buffer)
# print(Deserialize.unpack())
# print(Deserialize.unpack())
# print(Deserialize.unpack())
