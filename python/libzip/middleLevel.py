from ctypes import c_char_p, c_double, c_void_p

from .ctypes import functions as f
from .ctypes.callbacks import zip_cancel_callback, zip_progress_callback
from .ctypes.opaque import zip_ptr
from .enums import CompressionMethod, EncryptionMethod
from .version import libzip_version  # pylint:disable=unused-import


def discard(za: zip_ptr) -> None:
	return f.discard(za)


def register_progress_callback_with_state(za: zip_ptr, precision: c_double, callback: zip_progress_callback, ud_free: f.userDataFreer_p, ud: c_void_p) -> None:
	return f.register_progress_callback_with_state(za, precision, callback, ud_free, ud)


def register_cancel_callback_with_state(za: zip_ptr, callback: zip_cancel_callback, ud_free: f.userDataFreer_p, ud: c_void_p) -> None:
	return f.register_cancel_callback_with_state(za, callback, ud_free, ud)


def strerror(za: zip_ptr) -> c_char_p:
	return f.strerror(za)


def compression_method_supported(method: CompressionMethod, compress: bool) -> bool:
	return bool(f.compression_method_supported(int(method), int(compress)))


def encryption_method_supported(method: EncryptionMethod, encrypt: bool) -> bool:
	return bool(f.encryption_method_supported(int(method), int(encrypt)))
