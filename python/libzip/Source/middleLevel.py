import typing
from ctypes import byref, c_int, c_int64, c_uint64, c_void_p

from ..ctypes import functions as f
from ..ctypes.opaque import zip_source_ptr
from ..ctypes.structs import zip_file_attributes
from ..Error import LibZipError, _checkErrorCodeGeneric, zip_error
from ..Stat import Stat, zip_stat
from .abstractIO import ioWrapper

__all__ = ("source_begin_write", "source_begin_write_cloning", "source_close", "source_commit_write", "file_attributes_init", "source_get_file_attributes", "source_is_deleted", "source_open", "source_seek_compute_offset", "source_stat", "source_read", "source_write")


def _checkSourceErrorCode(src: zip_source_ptr, isFailure, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkErrorCodeGeneric(src, f.source_error, None, isFailure, res=res, *args)


def _checkFileErrorCodeResult(src: zip_source_ptr, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkSourceErrorCode(src, lambda res: res != 0, res=res, *args)


def _checkFileErrorCodeIO(src: zip_source_ptr, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkSourceErrorCode(src, lambda res: res < 0, res=res, *args)


source_read = ioWrapper(f.source_read, _checkFileErrorCodeIO, zip_source_ptr)
source_write = ioWrapper(f.source_write, _checkFileErrorCodeIO, zip_source_ptr)


def source_begin_write(src: zip_source_ptr) -> None:
	_checkFileErrorCodeResult(src, f.source_begin_write(src))


def source_begin_write_cloning(src: zip_source_ptr, offset: c_uint64) -> None:
	_checkFileErrorCodeResult(src, f.source_begin_write_cloning(src, offset))


def source_close(src: zip_source_ptr) -> None:
	_checkFileErrorCodeResult(src, f.source_close(src))


def source_commit_write(src: zip_source_ptr) -> None:
	_checkFileErrorCodeResult(src, f.source_commit_write(src))


def file_attributes_init() -> zip_file_attributes:
	res = zip_file_attributes()
	f.file_attributes_init(byref(res))
	return res


def source_get_file_attributes(src: zip_source_ptr) -> c_int:
	attrs = zip_file_attributes()
	_checkFileErrorCodeResult(src, f.source_get_file_attributes(src, byref(attrs)))
	return attrs


def source_is_deleted(src: zip_source_ptr) -> bool:
	return bool(f.source_is_deleted(src))


# def source_make_command_bitmap():
# 	l.zip_source_make_command_bitmap()
# 	= _variadic_function(, c_int64, [zip_source_cmd], None)


def source_open(src: zip_source_ptr) -> None:
	_checkFileErrorCodeResult(src, f.source_open(src))


def source_seek_compute_offset(offset: c_uint64, length: c_uint64, data: c_void_p, data_length: c_uint64) -> c_int64:
	err = zip_error(0)
	res = f.source_seek_compute_offset(offset, length, data, data_length, byref(err))
	if err.value:
		raise LibZipError(err.value)
	return res


def source_stat(src: zip_source_ptr) -> Stat:
	res = zip_stat()
	_checkFileErrorCodeResult(src, f.source_stat(src, byref(res)))
	return Stat(res)
