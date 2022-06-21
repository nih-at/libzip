import typing
from ctypes import POINTER, byref, c_byte, c_char_p, c_int32, c_uint8, c_uint16, c_uint32, cast
from datetime import date, datetime, time

from ..ctypes import functions as f
from ..ctypes._inttypes import zip_index
from ..ctypes.opaque import zip_ptr, zip_source_ptr
from ..ctypes.structs import time_t
from ..enums import OS, CompressionMethod, EncryptionMethod, ZipFlags
from ..Error import _checkArchiveErrorCode, _checkArchiveErrorCodeResult
from ..Source import Source, _Source
from ..Stat import zip_flags
from ..utils import PathT, acceptPathOrStrOrBytes, acceptStrOrBytes, toPurePathOrStrOrBytes
from ..utils.dosTime import dateToDosDateInt, timeToDosTimeInt

__all__ = ("file_get_comment", "ExternalAttrs", "file_get_external_attributes", "file_set_external_attributes", "file_rename", "file_add", "file_replace", "file_set_comment", "file_set_dostime", "file_set_encryption", "file_set_mtime", "dir_add", "name_locate", "get_name", "set_file_compression", "unchange", "file_idx")

# pylint:disable=too-many-function-args


class file_idx(int):
	pass


class ExternalAttrs:
	__slots__ = ("_os", "_attrs")

	def __init__(self, os: OS, attrs: int):
		self._os = OS(os)
		self._attrs = attrs

	@property
	def os(self):
		return self._os

	@property
	def attrs(self):
		return self._attrs

	def toTuple(self):
		return tuple(self)

	def __iter__(self):
		yield self.os
		yield self.attrs

	def __eq__(self, other):
		return self.toTuple() == other.toTuple()

	def __hash__(self):
		return hash(self.toTuple())

	def __repr__(self):
		return self.__class__.__name__ + repr(self.toTuple())


EXTERNAL_ATTRS_FLAGS = ZipFlags(0)


def file_get_external_attributes(za: zip_ptr, idx: file_idx, flags: ZipFlags) -> ExternalAttrs:
	os = c_uint8(0)
	attrs = c_uint32(0)

	_checkArchiveErrorCode(za, f.file_get_external_attributes(za, zip_index(idx), zip_flags(flags), byref(os), byref(attrs)))

	return ExternalAttrs(os.value, attrs.value)


def file_set_external_attributes(za: zip_ptr, idx: file_idx, flags: ZipFlags, attrs: ExternalAttrs) -> None:
	_checkArchiveErrorCode(za, f.file_set_external_attributes(za, zip_index(idx), flags, c_uint8(attrs.os), attributes=c_uint32(attrs.attrs)))


def file_rename(za: zip_ptr, idx: file_idx, name: typing.AnyStr, flags: ZipFlags) -> None:
	_checkArchiveErrorCode(za, f.file_rename(za, zip_index(idx), acceptPathOrStrOrBytes(name), zip_flags(flags)))


ADD_REPLACE_ARGS = ZipFlags.enc_utf_8


def file_add(za: zip_ptr, name: PathT, source: typing.Union[Source, zip_source_ptr], flags: ZipFlags = ADD_REPLACE_ARGS) -> zip_index:
	if isinstance(source, Source):
		if source.rs is None:
			with source as enteredSource:
				return file_add(za, name, enteredSource, flags)
		else:
			source = source.rs

	if isinstance(source, _Source):
		source.recordLeak()
		source = source.ptr

	return _checkArchiveErrorCodeResult(za, f.file_add(za, acceptPathOrStrOrBytes(name), source, zip_flags(flags)))


def file_replace(za: zip_ptr, idx: file_idx, source: typing.Union[Source, zip_source_ptr], flags: ZipFlags = ADD_REPLACE_ARGS) -> None:
	if isinstance(source, Source):
		if source.rs is None:
			with source as enteredSource:
				return file_replace(za, idx, enteredSource, flags)
		else:
			source = source.rs

	if isinstance(source, _Source):
		source.recordLeak()
		source = source.ptr

	_checkArchiveErrorCode(za, f.file_replace(za, zip_index(idx), source, zip_flags(flags)))
	return None


COMMENT_FLAGS = ZipFlags.enc_raw


def file_get_comment(za: zip_ptr, idx: file_idx, flags: ZipFlags = COMMENT_FLAGS) -> typing.Optional[bytes]:
	size = c_uint32(0)
	resPtr = f.file_get_comment(za, zip_index(idx), byref(size), zip_flags(int(flags)))

	if resPtr:
		bufT = c_byte * size.value
		bufPtrT = POINTER(bufT)
		return bytes(cast(resPtr, bufPtrT)[0])

	return None


def file_set_comment(za: zip_ptr, idx: file_idx, comment: typing.AnyStr, flags: ZipFlags = COMMENT_FLAGS) -> None:
	comment = acceptStrOrBytes(comment)
	_checkArchiveErrorCode(za, f.file_set_comment(za, zip_index(idx), c_char_p(comment), c_uint16(len(comment)), zip_flags(int(flags))))


def file_set_dostime(za: zip_ptr, idx: file_idx, dosTime: typing.Optional[typing.Union[time, int]] = None, dosDate: typing.Optional[typing.Union[date, int]] = None, flags: ZipFlags = 0) -> None:
	if bool(int(flags)):
		raise NotImplementedError("Flags are not implemented yet in the underlying lib. If they got implemented, notify us.")

	if not isinstance(dosTime, int):
		dosTime = timeToDosTimeInt(dosTime)

	if not isinstance(dosDate, int):
		dosDate = dateToDosDateInt(dosDate)

	_checkArchiveErrorCode(za, f.file_set_dostime(za, zip_index(idx), c_uint16(dosTime), c_uint16(dosDate), zip_flags(flags)))


def file_set_encryption(za: zip_ptr, idx: file_idx, method: EncryptionMethod, password: typing.AnyStr) -> None:
	_checkArchiveErrorCode(za, f.file_set_encryption(za, zip_index(idx), c_uint16(method), c_char_p(acceptStrOrBytes(password))))


MOD_TIME_FLAGS = ZipFlags(0)


def file_set_mtime(za: zip_ptr, idx: file_idx, mtime: typing.Union[int, datetime], flags: ZipFlags = MOD_TIME_FLAGS) -> None:
	if flags:
		raise NotImplementedError("Flags are not implemented.")

	if not isinstance(mtime, int):
		mtime = int(mtime.timestamp())

	_checkArchiveErrorCode(za, f.file_set_mtime(za, zip_index(idx), time_t(mtime), flags))


def dir_add(za: zip_ptr, name: PathT, flags: ZipFlags) -> zip_index:
	return _checkArchiveErrorCode(za, f.dir_add(za, acceptPathOrStrOrBytes(name), zip_flags(flags)))


def delete(za: zip_ptr, idx: file_idx) -> None:
	_checkArchiveErrorCode(za, f.delete(za, zip_index(idx)))


NAME_FLAGS = ZipFlags(0)
LOCATE_FLAGS = ZipFlags(0)


def name_locate(za: zip_ptr, fname: PathT, flags: ZipFlags = LOCATE_FLAGS) -> typing.Optional[zip_index]:
	res = f.name_locate(za, acceptPathOrStrOrBytes(fname), zip_flags(flags))
	if res < 0:
		res = None
	return res


def get_name(za: zip_ptr, idx: file_idx, flags: ZipFlags) -> PathT:
	return toPurePathOrStrOrBytes(_checkArchiveErrorCode(za, f.get_name(za, zip_index(idx), zip_flags(flags))))


_ALLOWED_LEVEL_RANGE = range(0, 10)  # 10 is excluded


def set_file_compression(za: zip_ptr, idx: file_idx, method: CompressionMethod = CompressionMethod.lzma2, level: int = 9) -> None:
	if level not in _ALLOWED_LEVEL_RANGE:
		raise ValueError("Incorrect compression level", level, _ALLOWED_LEVEL_RANGE)

	_checkArchiveErrorCode(za, f.set_file_compression(za, zip_index(idx), c_int32(method), c_uint32(level)))


def unchange(za: zip_ptr, idx: file_idx) -> None:
	_checkArchiveErrorCode(za, f.unchange(za, zip_index(idx)))
