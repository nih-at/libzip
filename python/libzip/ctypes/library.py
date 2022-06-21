import platform
from ctypes import CDLL, c_char_p

from ._funcToCtypesSignatureConvertor import assignTypesFromFunctionSignature as atffs

if platform.system() == "Windows":
	lib = CDLL("libzip.dll")
else:
	lib = CDLL("libzip.so")


def libzip_version() -> c_char_p:
	return _libzip_version()


_libzip_version = atffs(libzip_version, lib)
