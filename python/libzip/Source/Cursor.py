import typing
from collections.abc import ByteString
from ctypes import c_int64, c_void_p

__all__ = ("Cursor", "IReadCursor", "IWriteCursor")


TELL_FUNC_T = typing.Callable[[c_void_p], int]
SEEK_FUNC_T = typing.Callable[[c_void_p, int, int], None]


class Cursor:
	__slots__ = ("parent",)

	def __init__(self, parent: "Openable") -> None:
		self.parent = parent

	TELL_FUNC = None  # type: TELL_FUNC_T
	SEEK_FUNC = None  # type: SEEK_FUNC_T

	def tell(self):
		return self.__class__.TELL_FUNC(self.parent.ptr)

	def seek(self, offset: int, whence: int):
		# todo: somehow `source_seek_compute_offset` must be used
		return self.__class__.SEEK_FUNC(self.parent.ptr, offset, whence)


READ_FUNC_T = typing.Callable[[c_void_p, int], int]


class IReadCursor(Cursor):
	__slots__ = ()

	READ_FUNC = None  # type: READ_FUNC_T

	def read(self, buff: typing.Union[bytearray, int]) -> ByteString:
		if isinstance(buff, int):
			buff = bytearray(buff)

		read = self.__class__.READ_FUNC(self.parent.ptr, buff)
		return read


class IWriteCursor(Cursor):
	__slots__ = ()

	WRITE_FUNC = None

	def write(self, data: ByteString) -> c_int64:
		return self.__class__.WRITE_FUNC(self.parent.ptr, data)


class Openable:
	__slots__ = ("ptr", "reader", "writer")

	READ_CURSOR_TYPE = None  # type: IReadCursor
	WRITE_CURSOR_TYPE = None  # type: IWriteCursor

	def __repr__(self):
		return self.__class__.__name__ + "(" + hex(self.ptr) + ") <reader=" + repr(self.reader) + ">"

	def _open(self):
		raise NotImplementedError

	def _close(self):
		raise NotImplementedError

	def __init__(self, ptr: c_void_p) -> None:
		self.ptr = ptr
		self.reader = None
		self.writer = None

	def __enter__(self) -> "Openable":
		if not self.reader:
			self._open()
			self.reader = self.__class__.READ_CURSOR_TYPE(self)
			wct = self.__class__.WRITE_CURSOR_TYPE
			if wct:
				self.writer = wct(self)
		else:
			raise RuntimeError("Multiple entering of the same Openable", self)

		return self

	def __exit__(self, ex_type, ex_value, traceback) -> None:
		if self.reader is not None:
			self.reader = None
			self.writer = None
			self._close()

	def write(self, data: ByteString) -> c_int64:
		return self.writer.write(data)

	def read(self, buff: typing.Union[bytearray, int]) -> bytearray:
		return self.reader.read(buff)
