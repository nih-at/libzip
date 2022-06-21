import typing
from collections.abc import ByteString
from ctypes import POINTER, byref, c_char_p, c_int, c_int64, c_uint64, c_void_p

from ..ctypes import functions as f
from ..ctypes.callbacks import zip_source_callback
from ..ctypes.opaque import FILE_P, zip_ptr, zip_source_ptr
from ..ctypes.structs import zip_buffer_fragment
from ..ctypes.utils import byteStringToPointer
from ..Error import DecodedErrorStruct, LibZipError, _checkArchiveErrorCodeResult
from ..utils import PathT, acceptPathOrStrOrBytes

__all__ = ("source_file", "source_buffer_fragment", "source_buffer", "source_filep", "source_function", "source_zip")


def _source_file(za: zip_ptr, fname: PathT, start: int, length: int) -> zip_source_ptr:
	return _checkArchiveErrorCodeResult(za, f.source_file(za, c_char_p(acceptPathOrStrOrBytes(fname)), c_uint64(start), c_int64(length)))


def _source_file_create(fname: PathT, start: int, length: int) -> zip_source_ptr:
	err = DecodedErrorStruct()
	res = f.source_file_create(c_char_p(acceptPathOrStrOrBytes(fname)), c_uint64(start), c_int64(length), byref(err.err))
	if err:
		raise LibZipError(*err)
	return res


def source_file(fname: PathT, slc: slice = slice(0, None), za: typing.Optional[zip_ptr] = None) -> zip_source_ptr:
	start = slc.start
	if slc.stop is not None:
		length = slc.stop - slc.start
	else:
		length = -1

	return _source_file(za, fname, start, length) if za is not None else _source_file_create(fname, start, length)


def _source_buffer_fragment(za: zip_ptr, fragments: POINTER(zip_buffer_fragment), nfragments: c_uint64, freep: c_int) -> zip_source_ptr:
	return _checkArchiveErrorCodeResult(za, f.source_buffer_fragment(za, fragments, nfragments, freep))


def _source_buffer_fragment_create(fragments: POINTER(zip_buffer_fragment), nfragments: c_uint64, freep: c_int) -> zip_source_ptr:
	err = DecodedErrorStruct()
	res = f.source_buffer_fragment_create(fragments, nfragments, freep, byref(err.err))
	if err:
		raise LibZipError(*err)
	return res


def source_buffer_fragment(fragments: POINTER(zip_buffer_fragment), nfragments: c_uint64, freep: c_int, za: typing.Optional[zip_ptr] = None) -> zip_source_ptr:
	return _source_buffer_fragment(za, fragments, nfragments, freep) if za is not None else _source_buffer_fragment_create(fragments, nfragments, freep)


def _source_buffer(za: zip_ptr, data: ByteString, freep: bool = False) -> zip_source_ptr:
	buf, size = byteStringToPointer(data)
	return _checkArchiveErrorCodeResult(za, f.source_buffer(za, buf, c_uint64(size), c_int(int(bool(freep)))))


def _source_buffer_create(data: ByteString, freep: bool = False) -> zip_source_ptr:
	err = DecodedErrorStruct()
	buf, size = byteStringToPointer(data)
	res = f.source_buffer_create(buf, c_uint64(size), c_int(int(bool(freep))), byref(err.err))
	if err:
		raise LibZipError(*err)
	return res


def source_buffer(data: ByteString, za: typing.Optional[zip_ptr] = None) -> zip_source_ptr:
	"""`freep` parameter was removed intentionally, the lib frees the buffer when it is True, but in our case it is Python that manages the memory. Use the specialized lower-level functions yourself if you need to use that parameter, for example, to free memory allocated using malloc"""

	return _source_buffer(za, data) if za is not None else _source_buffer_create(data)


def _source_filep(za: zip_ptr, file: FILE_P, start: c_uint64, length: c_int64) -> zip_source_ptr:
	return _checkArchiveErrorCodeResult(za, f.source_filep(za, file, start, length))


def _source_filep_create(file: FILE_P, start: c_uint64, length: c_int64) -> zip_source_ptr:
	err = DecodedErrorStruct()
	res = f.source_filep_create(file, start, length, byref(err.err))
	if err:
		raise LibZipError(*err)
	return res


def source_filep(file: FILE_P, start: c_uint64, length: c_int64, za: typing.Optional[zip_ptr] = None) -> zip_source_ptr:
	return _source_filep(za, file, start, length) if za is not None else _source_filep_create(file, start, length)


def _source_function(za: zip_ptr, zcb: zip_source_callback, ud: c_void_p) -> zip_source_ptr:
	return _checkArchiveErrorCodeResult(za, f.source_function(za, zcb, ud))


def _source_function_create(zcb: zip_source_callback, ud: c_void_p) -> zip_source_ptr:
	err = DecodedErrorStruct()
	res = f.source_function_create(zcb, ud, byref(err.err))
	if err:
		raise LibZipError(*err)
	return res


def source_function(zcb: zip_source_callback, ud: c_void_p, za: typing.Optional[zip_ptr] = None) -> zip_source_ptr:
	return _source_function(za, zcb, ud) if za is not None else _source_function_create(zcb, ud)


source_zip = f.source_zip
