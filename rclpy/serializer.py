from datatype import Datatype, _TypedValue, Array
import struct



class Serializer:
    def __init__(self):
        self.buffer = bytearray()

    def pack(self, data, tag=None):
        # 래퍼 클래스 인스턴스면 자동 추출
        if isinstance(data, _TypedValue):
            if isinstance(data, Array):
                self._pack_array(data.value, data.inner_tag)
                return
            tag = data.tag
            data = data.value

        if tag is None:
            tag = Datatype.get_tag(data)

        if tag is Datatype.STRING:
            self.buffer.append(Datatype.STRING)
            encoded = data.encode('utf-8')
            self.buffer.extend(struct.pack('<I', len(encoded)))
            self.buffer.extend(encoded)
        elif tag in Datatype.FORMAT_MAP:
            self.buffer.append(tag)
            fmt = '<' + Datatype.FORMAT_MAP[tag]
            self.buffer.extend(struct.pack(fmt, data))
        else:
            raise ValueError(f"Unknown tag: {tag}")

    def _pack_array(self, data, instance_tag):
        """내부용. 외부에서는 Array 래퍼 클래스로 접근."""
        if instance_tag not in Datatype.FORMAT_MAP:
            raise ValueError(f"Array inner tag not supported: {instance_tag}")
        self.buffer.append(Datatype.ARRAY)
        self.buffer.extend(struct.pack('<I', len(data)))
        self.buffer.append(instance_tag)
        fmt = '<' + Datatype.FORMAT_MAP[instance_tag]
        for i in data:
            self.buffer.extend(struct.pack(fmt, i))

    # 하위호환: 기존 코드가 pack_array(data, tag) 로 직접 호출하던 부분용
    def pack_array(self, data, instance_tag):
        self._pack_array(data, instance_tag)


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
