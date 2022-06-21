import typing
from collections.abc import ByteString
from datetime import date, datetime, time
from pathlib import PurePath

from ..enums import OS, CompressionMethod, EncryptionMethod
from ..enums.Flags import ZipFlags
from ..File.middleLevel import file_idx
from ..Source import Source
from ..Source.Cursor import IReadCursor, Openable
from ..Stat import Stat, ZipStat, stat
from ..utils import PathT, toPurePathOrStrOrBytes
from .ctors import fopen_index, fopen_index_encrypted
from .extra import ExtraFields
from .io import fclose, file_is_seekable, fread, fseek, ftell
from .middleLevel import ADD_REPLACE_ARGS, COMMENT_FLAGS, EXTERNAL_ATTRS_FLAGS, LOCATE_FLAGS, MOD_TIME_FLAGS, NAME_FLAGS, ExternalAttrs, delete, file_add, file_get_comment, file_get_external_attributes, file_idx, file_rename, file_replace, file_set_comment, file_set_dostime, file_set_encryption, file_set_external_attributes, file_set_mtime, get_name, name_locate, set_file_compression, unchange

__all__ = ("MutableExternalAttrs", "ExistingFile")

# pylint:disable=too-many-instance-attributes,too-many-arguments,too-many-public-methods


class ReadCursor(IReadCursor):
	__slots__ = ()

	TELL_FUNC = ftell
	SEEK_FUNC = fseek
	READ_FUNC = fread

	@property
	def isSeekable(self) -> bool:
		file_is_seekable(self.parent.ptr)


class MutableExternalAttrs(ExternalAttrs):
	__slots__ = ("parent",)

	def __init__(self, parent, s: OS, attrs: int):
		self.parent = parent
		super().__init__(s, attrs)

	@ExternalAttrs.os.setter
	def os(self, v):
		self._os = v
		self.parent.externalAttributes = self

	@ExternalAttrs.attrs.setter
	def attrs(self, v):
		self._attrs = v
		self.parent.externalAttributes = self


class ExistingFile(Openable):
	__slots__ = ("parent", "idx", "_path", "_stat", "flags", "password", "extras")

	NAME_FLAGS = NAME_FLAGS
	LOCATE_FLAGS = LOCATE_FLAGS
	READ_CURSOR_TYPE = ReadCursor

	def get_name(self, flags: ZipFlags = NAME_FLAGS) -> PathT:
		return get_name(self.parent.ptr, self.idx, flags=flags)

	def rename(self, name: PathT, flags: ZipFlags = NAME_FLAGS) -> None:
		return file_rename(self.parent.ptr, self.idx, name=name, flags=flags)

	def __repr__(self):
		return self.__class__.__name__ + "(" + ", ".join((repr(self.parent), repr(self.idx), repr(self.pathName), repr(self.flags), repr(self.password))) + ")"

	@property
	def pathName(self) -> PathT:
		if self._path is None:
			self._path = self.get_name(self.__class__.NAME_FLAGS)
		return self._path

	@pathName.setter
	def pathName(self, v: PathT):
		self.rename(v, flags=self.__class__.NAME_FLAGS)
		self._path = PurePath(self.get_name(self.__class__.NAME_FLAGS))
		return self._path

	def __init__(self, parent: "Archive", idx: file_idx, path: PathT, flags: ZipFlags, password: typing.Optional[typing.Union[ByteString, str]] = None) -> None:
		super().__init__(None)
		self.parent = parent
		self.flags = flags
		self.password = password
		self._stat = None
		self._path = path

		if idx is None and path is None:
			raise ValueError("You must provide either an index or a path")

		if idx is None:
			idx = name_locate(parent.ptr, path, self.__class__.LOCATE_FLAGS)
			if idx is None:
				raise KeyError("File not found in the archive", path)

		self.idx = idx

		if path is None:
			self._path = self.pathName
		else:
			self._path = toPurePathOrStrOrBytes(self._path)

		self.extras = ExtraFields(self)

	def _open(self) -> None:
		if self.password is not None:
			self.ptr = fopen_index_encrypted(self.parent.ptr, self.idx, self.flags, password=self.password)
		else:
			self.ptr = fopen_index(self.parent.ptr, self.idx, self.flags)

	def _close(self) -> None:
		fclose(self.ptr)
		self.ptr = None

	COMMENT_FLAGS = COMMENT_FLAGS

	def getComment(self, flags: ZipFlags = COMMENT_FLAGS) -> typing.Optional[bytes]:
		return file_get_comment(self.parent.ptr, self.idx, flags)

	def setComment(self, comment: typing.Union[str, ByteString], flags: ZipFlags = COMMENT_FLAGS) -> None:
		return file_set_comment(self.parent.ptr, self.idx, comment=comment, flags=flags)

	@property
	def comment(self) -> typing.Optional[bytes]:
		return self.getComment()

	@comment.setter
	def comment(self, v: typing.Union[str, ByteString]):
		return self.setComment(v)

	EXTERNAL_ATTRS_FLAGS = EXTERNAL_ATTRS_FLAGS

	def getExternalAttributes(self, flags: ZipFlags = EXTERNAL_ATTRS_FLAGS) -> ExternalAttrs:
		return MutableExternalAttrs(self, *file_get_external_attributes(self.parent.ptr, self.idx, flags=flags))

	def setExternalAttributes(self, attrs: ExternalAttrs, flags: ZipFlags = EXTERNAL_ATTRS_FLAGS) -> None:
		return file_set_external_attributes(self.parent.ptr, self.idx, flags=flags, attrs=attrs)

	@property
	def externalAttributes(self, flags: ZipFlags = EXTERNAL_ATTRS_FLAGS) -> ExternalAttrs:
		return self.getExternalAttributes(self.__class__.EXTERNAL_ATTRS_FLAGS)

	@externalAttributes.setter
	def externalAttributes(self, v: ExternalAttrs):
		return self.setExternalAttributes(v, self.__class__.EXTERNAL_ATTRS_FLAGS)

	# `stat`-related
	@property
	def stat(self) -> Stat:
		if self._stat is None:
			self._stat = stat(self.parent.ptr, self.idx, what=ZipStat.everything)
		return self._stat

	@property
	def compressionMethod(self) -> CompressionMethod:
		return self.stat.compressionMethod

	@compressionMethod.setter
	def compressionMethod(self, method: CompressionMethod):
		return self.setCompression(method)

	def setCompression(self, method: CompressionMethod = CompressionMethod.deflate, level: int = 9) -> None:
		set_file_compression(self.parent.ptr, self.idx, method=method, level=level)
		self.stat.compressionMethod = method

	@property
	def encryptionMethod(self) -> EncryptionMethod:
		return self.stat.encryptionMethod

	@encryptionMethod.setter
	def encryptionMethod(self, method: EncryptionMethod):
		return self.setEncryption(method, self.password)

	def setEncryption(self, method: EncryptionMethod, password: str) -> None:
		file_set_encryption(self.parent.ptr, self.idx, method=method, password=password)
		self.stat.encryptionMethod = method

	@property
	def modTime(self) -> datetime:
		return self.stat.modificationTime

	@modTime.setter
	def modTime(self, modTime: typing.Union[int, datetime]):
		return self.setModTime(modTime)

	MOD_TIME_FLAGS = MOD_TIME_FLAGS

	def setModTime(self, modTime: typing.Union[int, datetime], flags: ZipFlags = MOD_TIME_FLAGS) -> None:
		return file_set_mtime(self.parent.ptr, self.idx, mtime=modTime, flags=flags)

	def setDosTime(self, dosTime: typing.Optional[typing.Union[time, int]] = None, dosDate: typing.Optional[typing.Union[date, int]] = None, flags: ZipFlags = 0) -> None:
		return file_set_dostime(self.parent.ptr, self.idx, dosTime=dosTime, dosDate=dosDate, flags=flags)

	@property
	def externaAttributes(self) -> ExternalAttrs:
		return self.getExternalAttributes(self.__class__.EXTERNAL_ATTRS_FLAGS)

	@externaAttributes.setter
	def externaAttributes(self, attrs: ExternalAttrs):
		return self.setExternalAttributes(flags=self.__class__.EXTERNAL_ATTRS_FLAGS, attrs=attrs)

	def delete(self) -> None:
		return delete(self.parent.ptr, self.idx)

	def unchange(self) -> None:
		return unchange(self.parent.ptr, self.idx)

	def replace(self, source: Source, flags: ZipFlags = ADD_REPLACE_ARGS) -> None:
		file_replace(self.parent.ptr, self.idx, source=source, flags=flags)
