import struct
import numpy as np
from datatype import Datatype

class Serializer:
    def __init__(self):
        self.buffer = bytearray()

    def pack(self, data, tag = None):
        """
        data: 실제 값
        tag: 전송할 타입. EX) Datatype.UINT32
        """
        if tag is None:
            tag = Datatype.get_tag(data)

        if tag is Datatype.STRING:
            # T
            self.buffer.append(Datatype.STRING)
            encoded = data.encode('utf-8')
            # L
            self.buffer.extend(struct.pack("<I",len(encoded)))
            # V
            self.buffer.extend(encoded)
        elif tag in Datatype.FORMAT_MAP:
                # T
                self.buffer.append(tag)
                # V
                fmt = '<' + Datatype.FORMAT_MAP[tag]
                self.buffer.extend(struct.pack(fmt, data))

    def pack_array(self, data, instance_tag):
        #T
        self.buffer.append(Datatype.ARRAY)
        #L
        self.buffer.extend(struct.pack("<I", len(data)))
        #T
        self.buffer.append(instance_tag)
        #V
        fmt = '<' + Datatype.FORMAT_MAP[instance_tag]
        for i in data:
            self.buffer.extend(struct.pack(fmt, i))


class Deserializer:
    def __init__(self, data: bytes):
        self.buffer = data
        self.offset = 0

    def unpack(self):
        if self.offset >= len(self.buffer):
            return None
        tag = self.buffer[self.offset]
        self.offset += 1
        #T
        if tag == Datatype.ARRAY:
            results = []
            #L
            count = struct.unpack_from('<I', self.buffer, self.offset)[0]
            self.offset += 4
            #T
            element_tag = self.buffer[self.offset]
            self.offset += 1
            #V
            for _ in range(count):
                if element_tag in Datatype.FORMAT_MAP:
                    fmt = '<' + Datatype.FORMAT_MAP[element_tag]
                    val = struct.unpack_from(fmt, self.buffer, self.offset)[0]
                    self.offset += struct.calcsize(fmt)
                    results.append(val)
                elif element_tag == Datatype.STRING:
                    str_len = struct.unpack_from('<I', self.buffer, self.offset)[0]
                    self.offset += 4
                    val = self.buffer[self.offset: self.offset + str_len].decode('utf-8')
                    self.offset += str_len
                    results.append(val)
                elif element_tag == Datatype.ARRAY:
                    self.offset -= 1
                    results.append(self.unpack())
            return results
        elif tag == Datatype.STRING:
            str_len = struct.unpack_from('<I', self.buffer, self.offset)[0]
            self.offset += 4
            val = self.buffer[self.offset: self.offset + str_len].decode('utf-8')
            self.offset += str_len
            return val
        elif tag in Datatype.FORMAT_MAP:
            fmt = '<' + Datatype.FORMAT_MAP[tag]
            val = struct.unpack_from(fmt, self.buffer, self.offset)[0]
            self.offset += struct.calcsize(fmt)
            return val
        print("Error during deserialization!")
        return None

    def unpack_all(self):
        result = []
        while self.offset < len(self.buffer):
            item = self.unpack()
            result.append(item)

        self.offset = 0
        self.buffer = bytearray()
        return result

    def add(self, data):
        self.buffer = data
        self. offset = 0
