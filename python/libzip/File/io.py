import typing
from ctypes import c_char_p, c_int, c_int8, c_int64

from ..ctypes import functions as f
from ..ctypes.opaque import zip_file_ptr
from ..Error import _checkErrorCodeGeneric
from ..Source.abstractIO import ioWrapper

__all__ = ("file_strerror", "fclose", "fread", "fseek", "ftell")


def _checkFileErrorCode(zf: zip_file_ptr, isFailure: typing.Callable, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkErrorCodeGeneric(zf, f.file_get_error, f.file_error_clear, isFailure, res, *args)


def _checkFileErrorCodeResult(zf: zip_file_ptr, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkFileErrorCode(zf, lambda res: res != 0, res, *args)


def _checkFileErrorCodeIO(zf: zip_file_ptr, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkFileErrorCode(zf, lambda res: res < 0, res, *args)


def file_strerror(zf: zip_file_ptr) -> c_char_p:
	return f.file_strerror(zf)


def fclose(zf: zip_file_ptr) -> None:
	return _checkFileErrorCodeResult(zf, f.fclose(zf))


fread = ioWrapper(f.fread, _checkFileErrorCodeIO, zip_file_ptr)


def fseek(zf: zip_file_ptr, offset: c_int64, whence: c_int) -> c_int8:
	return _checkFileErrorCodeIO(zf, f.fseek(zf, offset, whence))


def file_is_seekable(zf: zip_file_ptr) -> bool:
	return bool(int(f.file_is_seekable(zf)))


def ftell(zf: zip_file_ptr) -> c_int64:
	return _checkFileErrorCodeIO(zf, f.ftell(zf))
