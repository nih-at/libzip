import typing
from collections.abc import ByteString
from ctypes import c_uint64

from ..ctypes import functions as f
from ..ctypes.opaque import zip_file_ptr, zip_ptr
from ..enums import ZipFlags
from ..utils import AnyStr, PathT, acceptPathOrStrOrBytes, acceptStrOrBytes

__all__ = ("fopen", "fopen_encrypted", "fopen_index", "fopen_index_encrypted")


def fopen(za: zip_ptr, fname: PathT, flags: ZipFlags) -> zip_file_ptr:
	return f.fopen(za, acceptPathOrStrOrBytes(fname), flags)


def fopen_encrypted(za: zip_ptr, fname: PathT, flags: ZipFlags, password: AnyStr) -> zip_file_ptr:
	return f.fopen_encrypted(za, acceptPathOrStrOrBytes(fname), flags, acceptStrOrBytes(password))


def fopen_index(za: zip_ptr, index: int, flags: ZipFlags) -> zip_file_ptr:
	return f.fopen_index(za, c_uint64(index), flags)


def fopen_index_encrypted(za: zip_ptr, index: int, flags: ZipFlags, password: typing.Union[ByteString, str]) -> zip_file_ptr:
	return f.fopen_index_encrypted(za, c_uint64(index), flags, acceptStrOrBytes(password))
