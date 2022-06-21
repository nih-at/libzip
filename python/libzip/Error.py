import typing
from ctypes import POINTER, byref, c_uint64, c_void_p

from .ctypes import functions as f
from .ctypes._inttypes import c_int, c_int64
from .ctypes.opaque import zip_file_ptr, zip_ptr, zip_source_ptr
from .ctypes.structs import zip_error
from .enums.ZipError import SystemErrorType, ZipError

zip_error_ptr = POINTER(zip_error)


def error_to_data(error: zip_error_ptr, data: c_void_p, length: c_uint64) -> c_int64:
	return f.error_to_data(error, data, length)


def error_fini(err: zip_error_ptr) -> None:
	return f.error_fini(err)


def error_init(err: zip_error_ptr) -> None:
	return f.error_init(err)


def error_init_with_code(error: zip_error_ptr, ze: c_int) -> None:
	return f.error_init_with_code(error, ze)


def error_set(err: zip_error_ptr, ze: c_int, se: c_int) -> None:
	return f.error_set(err, ze, se)


class IDecodedError:
	__slots__ = ("_zip", "_sysType", "_sysCode", "_descr")
	# pylint:disable=attribute-defined-outside-init

	def __init__(self) -> None:
		self.resetCache()

	def resetCache(self) -> None:
		self._zip = None
		self._sysType = None
		self._sysCode = None
		self._descr = None

	@property
	def zip(self) -> ZipError:
		if self._zip is None:
			self._zip = ZipError(self._getZipError())
		return self._zip

	def _getZipError(self) -> int:
		raise NotImplementedError

	@property
	def sysType(self) -> SystemErrorType:
		if self._sysType is None:
			self._sysType = SystemErrorType(self._getSystemErrorType())
		return self._sysType

	def _getSystemErrorType(self) -> int:
		raise NotImplementedError

	@property
	def sysCode(self) -> int:
		if self._sysCode is None:
			self._sysCode = self._getSystemCode()
		return self._sysCode

	def _getSystemCode(self) -> int:
		raise NotImplementedError

	@property
	def descr(self) -> str:
		if self._descr is None:
			self._descr = self._getDescr().decode("utf-8")
		return self._descr

	def _getDescr(self) -> bytes:
		raise NotImplementedError

	def __repr__(self):
		return self.__class__.__name__ + repr(tuple(self))

	def __iter__(self):
		for k in __class__.__slots__:
			yield getattr(self, k[1:])

	def __bool__(self) -> bool:
		return bool(self.zip) or bool(self.sysType) or bool(self.sysCode)


class DecodedErrorPtr(IDecodedError):
	__slots__ = ("ptr",)

	def __init__(self, sPtr):
		super().__init__()
		self.ptr = sPtr

	def _getZipError(self) -> int:
		return f.error_code_zip(self.ptr)

	def _getSystemErrorType(self) -> int:
		return f.error_system_type(self.ptr)

	def _getSystemCode(self) -> int:
		return f.error_code_system(self.ptr)

	def _getDescr(self) -> bytes:
		return f.error_strerror(self.ptr)


class DecodedErrorStruct(DecodedErrorPtr):
	__slots__ = ("err",)

	def __init__(self) -> None:
		self.err = zip_error(0)
		super().__init__(byref(self.err))

	def _getZipError(self) -> int:
		return self.err.zip_err

	def _getSystemErrorType(self) -> int:
		return self.err.sys_err


class LibZipError(Exception):
	pass


def _checkErrorCodeGeneric(handle: typing.Union[zip_ptr, zip_source_ptr, zip_file_ptr], errorStructGetter: typing.Callable, resourceErrorCleaner: typing.Callable, isFailure: typing.Callable, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	if res is None or (isinstance(res, int) and isFailure(res)):
		error = errorStructGetter(handle)  # type: zip_error_ptr
		if error:
			dec = DecodedErrorPtr(error)
			errorObj = LibZipError(*dec, *args)
			error_fini(error)
			if resourceErrorCleaner is not None:
				resourceErrorCleaner(handle)
			raise errorObj

		raise Exception("Operation haven't suceed but no error code have been set", *args)

	return res


def _checkArchiveErrorCodeGeneric(za: zip_ptr, isFailure: typing.Callable, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkErrorCodeGeneric(za, f.get_error, f.error_clear, isFailure, res, *args)


def _checkArchiveErrorCode(za: zip_ptr, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkArchiveErrorCodeGeneric(za, lambda res: res != 0, res, *args)


def _checkArchiveErrorCodeResult(za: zip_ptr, res: typing.Union[int, typing.Any], *args) -> typing.Any:
	return _checkArchiveErrorCodeGeneric(za, lambda res: res == 0, res, *args)
