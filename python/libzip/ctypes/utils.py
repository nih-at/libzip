import typing
from collections.abc import ByteString
from ctypes import c_byte, c_void_p, cast


def byteStringToPointer(data: ByteString) -> typing.Tuple[c_void_p, int]:
	size = len(data)
	if isinstance(data, bytes):
		buf = cast(data, c_void_p)
	else:
		bufT = c_byte * size
		buf = bufT.from_buffer(data)
		buf = cast(buf, c_void_p)

	return buf, size
