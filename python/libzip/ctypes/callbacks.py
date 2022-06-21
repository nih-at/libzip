from ctypes import CFUNCTYPE, c_double, c_int, c_int64, c_uint64, c_void_p

from ._inttypes import zip_source_cmd
from .opaque import zip_ptr

zip_source_callback = CFUNCTYPE(c_int64, c_void_p, c_void_p, c_uint64, zip_source_cmd)
zip_progress_callback = CFUNCTYPE(None, zip_ptr, c_double, c_void_p)
zip_cancel_callback = CFUNCTYPE(c_int, zip_ptr, c_void_p)
zip_progress_callback_t = CFUNCTYPE(None, c_double)
