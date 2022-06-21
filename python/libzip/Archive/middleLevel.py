import typing
from ctypes import POINTER, byref, c_byte, c_char_p, c_int, c_int32, c_uint16, cast

from ..ctypes import functions as f
from ..ctypes._inttypes import zip_flags
from ..ctypes.opaque import zip_ptr
from ..enums import ZipFlags
from ..Error import _checkArchiveErrorCode
from ..utils import acceptStrOrBytes

__all__ = ("archive_set_tempdir", "close", "discard", "get_archive_comment", "set_archive_comment", "get_archive_flag", "set_archive_flag", "get_num_entries", "set_default_password", "unchange_all", "unchange_archive")


def archive_set_tempdir(za: zip_ptr, tempdir: c_char_p) -> None:
	_checkArchiveErrorCode(za, f.archive_set_tempdir(za, tempdir))


def close(za: zip_ptr) -> None:
	_checkArchiveErrorCode(za, f.close(za))


def discard(za: zip_ptr) -> None:
	return f.discard(za)


def get_archive_comment(za: zip_ptr, flags: ZipFlags) -> typing.Optional[bytes]:
	lenp = c_int32(0)
	resPtr = f.get_archive_comment(za, byref(lenp), zip_flags(flags))

	lenp = lenp.value

	if lenp < 0:
		raise ValueError("lenp returned is < 0", lenp)

	if resPtr:
		bufT = c_byte * lenp
		bufPtrT = POINTER(bufT)
		return bytes(cast(resPtr, bufPtrT)[0])

	return None


def get_archive_flag(za: zip_ptr, flag: ZipFlags, flags: ZipFlags) -> bool:
	return bool(_checkArchiveErrorCode(za, f.get_archive_flag(za, zip_flags(flag), zip_flags(flags))))


def get_num_entries(za: zip_ptr, flags: ZipFlags) -> int:
	return int(f.get_num_entries(za, zip_flags(flags)))


def set_archive_comment(za: zip_ptr, comment: typing.AnyStr) -> None:
	comment = acceptStrOrBytes(comment)

	_checkArchiveErrorCode(za, f.set_archive_comment(za, c_char_p(comment), c_uint16(len(comment))))


def set_archive_flag(za: zip_ptr, flag: ZipFlags, value: c_int) -> None:
	_checkArchiveErrorCode(za, f.set_archive_flag(za, zip_flags(flag), value))


def set_default_password(za: zip_ptr, passwd: typing.AnyStr) -> None:
	_checkArchiveErrorCode(za, f.set_default_password(za, c_char_p(acceptStrOrBytes(passwd))))


def unchange_all(za: zip_ptr) -> None:
	_checkArchiveErrorCode(za, f.unchange_all(za))


def unchange_archive(za: zip_ptr) -> None:
	_checkArchiveErrorCode(za, f.unchange_archive(za))
