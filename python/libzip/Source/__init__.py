import platform
import traceback
import typing
from collections.abc import ByteString
from ctypes import *
from datetime import date, datetime, time
from pathlib import Path

from ..ctypes import functions as f
from ..ctypes._inttypes import *
from ..ctypes.callbacks import *
from ..ctypes.functions import get_error
from ..ctypes.opaque import *
from ..ctypes.structs import *
from ..enums import *
from ..Error import *
from ..Error import _checkArchiveErrorCode
from ..Stat import *
from ..utils.dosTime import *
from .ctors import *
from .Cursor import IReadCursor, IWriteCursor, Openable
from .middleLevel import source_begin_write, source_begin_write_cloning, source_close, source_commit_write, source_get_file_attributes, source_is_deleted, source_open, source_read, source_stat, source_write

__all__ = ("Cursor", "ReadCursor", "WriteCursor", "Source")


class ReadCursor(IReadCursor):
	__slots__ = ()

	TELL_FUNC = f.source_tell
	SEEK_FUNC = f.source_seek
	READ_FUNC = source_read


class WriteCursor(IWriteCursor):
	__slots__ = ()

	TELL_FUNC = f.source_tell_write
	SEEK_FUNC = f.source_seek_write
	WRITE_FUNC = source_write

	def beginWrite(self):
		return source_begin_write(self.parent.ptr)

	def beginWriteCloning(self, offset: c_uint64):
		return source_begin_write_cloning(self.parent.ptr, offset)

	def rollback(self) -> None:
		return f.source_rollback_write(self.parent.ptr)

	def commit(self):
		source_commit_write(self.parent.ptr)


# todo: RawIOBase


class _Source(Openable):
	__slots__ = ("leakSources",)

	READ_CURSOR_TYPE = ReadCursor
	WRITE_CURSOR_TYPE = WriteCursor

	def __init__(self, ptr: zip_source_ptr) -> None:
		super().__init__(ptr)
		self.leakSources = []

	def recordLeak(self) -> None:
		"""Docs prohibits calling `zip_source_free` in certain cases."""
		self.leakSources.append(traceback.extract_stack(f=None, limit=None))

	def incRef(self):
		f.source_keep(self.ptr)

	def decRef(self):
		if self.leakSources:
			raise Exception("Docs prohibits calling `zip_source_free` on the sources used in `zip_open_from_source`, `zip_file_add` and `zip_file_replace` calls.", self.leakSources)

		f.source_free(self.ptr)

	def _open(self):
		source_open(self.ptr)

	def _close(self):
		source_close(self.ptr)

	def stat(self) -> Stat:
		return source_stat(self.ptr)

	def getAttributes(self) -> int:
		return source_get_file_attributes(self.ptr)

	@property
	def attributes(self):
		return source_get_file_attributes(self.ptr)

	@property
	def isDeleted(self) -> bool:
		return source_is_deleted(self.ptr)


class Source:
	__slots__ = ("rs", "parent")

	def __init__(self, parent: typing.Optional["Archive"] = None) -> None:
		self.__class__.rs.__set__(self, None)  # pylint:disable=no-member
		self.__class__.parent.__set__(self, parent)  # pylint:disable=no-member

	def makeRawSource(self, za: typing.Optional[int]) -> _Source:
		raise NotImplementedError

	def __enter__(self) -> _Source:
		# pylint:disable=no-member,attribute-defined-outside-init
		p = self.parent
		self.rs = _Source(self.makeRawSource(p.ptr if p else None))
		return self.rs

	def __exit__(self, ex_type, ex_value, tb) -> None:
		# pylint:disable=attribute-defined-outside-init
		self.rs = None

	@classmethod
	def make(cls, source: typing.Union[Path, bytes, bytearray], slc: slice = slice(0, None), parent: typing.Optional["Archive"] = None) -> "Source":
		if isinstance(source, Path):
			return PathSource(path=source, slc=slc, parent=parent)

		if isinstance(source, ByteString):
			if slc.start != 0 and slc.stop not in (None, len(source) - 1):
				source = source[slc]

			return BufferSource(buff=source, parent=parent)

		raise ValueError("Not yet supported type")


class PathSource(Source):
	__slots__ = ("path", "slc")

	def __init__(self, path: Path, slc: slice = slice(0, None), parent=None):
		super().__init__(parent=parent)
		self.__class__.path.__set__(self, path)  # pylint:disable=no-member
		self.__class__.slc.__set__(self, slc)  # pylint:disable=no-member

	def makeRawSource(self, za: typing.Optional[int]) -> _Source:
		return source_file(fname=self.path, slc=self.slc, za=za)  # pylint:disable=no-member


class BufferSource(Source):
	__slots__ = ("buff",)

	def __init__(self, buff: ByteString, parent: typing.Optional["Archive"] = None) -> None:
		super().__init__(parent=parent)
		self.__class__.buff.__set__(self, buff)  # pylint:disable=no-member

	def makeRawSource(self, za: typing.Optional[int]) -> _Source:
		return source_buffer(data=self.buff, za=za)  # pylint:disable=no-member
