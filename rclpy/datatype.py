import numpy as np

class Datatype:
    NONE     = 255
    BOOL     = 0
    INT8     = 11
    UINT8    = 1
    INT16    = 12
    UINT16   = 2
    INT32    = 14
    UINT32   = 4
    INT64    = 18
    UINT64   = 8
    FLOAT32  = 24
    FLOAT64  = 28
    # FLOAT128 = 29
    STRING   = 30
    ARRAY    = 40
    NODE_INFO = 80
    SOCKET_INFO = 90

    FORMAT_MAP = {
        BOOL: '?',  # 1 byte
        INT8: 'b',  # 1 byte
        UINT8: 'B',  # 1 byte
        INT16: 'h',  # 2 bytes
        UINT16: 'H',  # 2 bytes
        INT32: 'i',  # 4 bytes
        UINT32: 'I',  # 4 bytes
        INT64: 'q',  # 8 bytes
        UINT64: 'Q',  # 8 bytes
        FLOAT32: 'f',  # 4 bytes
        FLOAT64: 'd',  # 8 bytes
    }

    _NP_TYPE_MAP = {
        np.bool_: BOOL,
        np.int8: INT8,
        np.uint8: UINT8,
        np.int16: INT16,
        np.uint16: UINT16,
        np.int32: INT32,
        np.uint32: UINT32,
        np.int64: INT64,
        np.uint64: UINT64,
        np.float32: FLOAT32,
        np.float64: FLOAT64,
        # np.float128: FLOAT128,
    }

    @classmethod
    def get_tag(cls, obj):

        if hasattr(obj, 'dtype'):
            return cls._NP_TYPE_MAP.get(obj.dtype.type, cls.NONE)

        if isinstance(obj, bool): return cls.BOOL
        if isinstance(obj, str): return cls.STRING
        if isinstance(obj, list): return cls.ARRAY

        if isinstance(obj, int):
            if -128 <= obj <= 127: return cls.INT8
            if 0 <= obj <= 255: return cls.UINT8
            if -32768 <= obj <= 32767: return cls.INT16
            if 0 <= obj <= 65535: return cls.INT16
            if -2147483648 <= obj <= 2147483647: return cls.INT32
            if 0 <= obj <= 4294967295: return cls.UINT32
            if 0 <= obj <= 18446744073709551615: return cls.UINT64
            return cls.INT64
            #TODO() if obj > INT64 ?

        if isinstance(obj, float):
            return cls.FLOAT64
            #TODO() double vs float vs float128
        return cls.NONE

class _TypedValue:
    tag = None

    def __init__(self, value, tag):
        self.value = value
        self.tag = tag
    def __repr__(self):
        return f"{self.__class__.__name__}({self.value})"

class Bool(_TypedValue):
    tag = Datatype.BOOL
    def __init__(self, v): super().__init__(bool(v), self.tag)

class Int8(_TypedValue):
    tag = Datatype.INT8
    def __init__(self, v): super().__init__(int(v), self.tag)

class UInt8(_TypedValue):
    tag = Datatype.UINT8
    def __init__(self, v): super().__init__(int(v), self.tag)

class Int16(_TypedValue):
    tag = Datatype.INT16
    def __init__(self, v): super().__init__(int(v), self.tag)

class UInt16(_TypedValue):
    tag = Datatype.UINT16
    def __init__(self, v): super().__init__(int(v), self.tag)

class Int32(_TypedValue):
    tag = Datatype.INT32
    def __init__(self, v): super().__init__(int(v), self.tag)

class UInt32(_TypedValue):
    tag = Datatype.UINT32
    def __init__(self, v): super().__init__(int(v), self.tag)

class Int64(_TypedValue):
    tag = Datatype.INT64
    def __init__(self, v): super().__init__(int(v),self.tag)

class UInt64(_TypedValue):
    tag = Datatype.UINT64
    def __init__(self, v): super().__init__(int(v), self.tag)

class Float32(_TypedValue):
    tag = Datatype.FLOAT32
    def __init__(self, v): super().__init__(float(v), self.tag)

class Float64(_TypedValue):
    tag = Datatype.FLOAT64
    def __init__(self, v): super().__init__(float(v), self.tag)

class String(_TypedValue):
    tag = Datatype.STRING
    def __init__(self, v): super().__init__(str(v), self.tag)

# 배열: 내부 원소 타입을 래퍼 클래스로 지정
class Array(_TypedValue):
    def __init__(self, v: list, inner: type[_TypedValue]):
        self.inner_tag = inner.tag
        super().__init__(v, Datatype.ARRAY)
