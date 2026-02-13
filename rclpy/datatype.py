import numpy as np

class Datatype:
    NONE     = 0
    BOOL     = 1
    INT8     = 3
    UINT8    = 4
    INT16    = 6
    UINT16   = 7
    INT32    = 9
    UINT32   = 10
    INT64    = 12
    UINT64   = 13
    FLOAT32  = 20
    FLOAT64  = 22
    FLOAT128 = 24
    STRING   = 30
    ARRAY    = 40

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
        np.float128: FLOAT128,
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