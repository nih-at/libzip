import typing
from ctypes import c_int64, c_uint64, c_void_p

from ..ctypes.utils import byteStringToPointer

__all__ = ("ioWrapper",)


def ioWrapper(readWriteFunc: typing.Callable, errorCodeFunc: typing.Callable, descriptorType: typing.Type[c_void_p]) -> typing.Callable:
	def wrapper(src: descriptorType, data: bytearray) -> c_int64:
		buf, size = byteStringToPointer(data)
		return errorCodeFunc(src, readWriteFunc(src, buf, c_uint64(size)))

	wrapper.__name__ = readWriteFunc.__name__

	return wrapper
