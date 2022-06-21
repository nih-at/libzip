import typing
from ctypes import byref, c_char_p, c_int

from ..ctypes import functions as f
from ..Source import OpenFlags, Source, ZipError, zip_ptr, zip_source_ptr
from ..utils import acceptPathOrStrOrBytes

__all__ = ("open", "open_from_source", "fdopen")


def open(src: str, flags: OpenFlags) -> zip_ptr:  # pylint:disable=redefined-builtin
	err = c_int(0)
	res = f.open(c_char_p(acceptPathOrStrOrBytes(src)), c_int(flags), byref(err))
	err = ZipError(err.value)
	if err:
		raise Exception(err)
	return res


def open_from_source(src: typing.Union[zip_source_ptr, Source], flags: OpenFlags) -> zip_ptr:
	if isinstance(src, Source):
		src.recordLeak()
		src = src.ptr

	err = c_int(0)
	res = f.open_from_source(src, c_int(flags), byref(err))
	err = ZipError(err.value)
	if err:
		raise Exception(err)
	return res


def fdopen(fd_orig: c_int, _flags: OpenFlags) -> zip_ptr:
	err = c_int(0)
	res = f.fdopen(fd_orig, c_int(_flags), byref(err))
	err = ZipError(err.value)
	if err:
		raise Exception(err)
	return res
