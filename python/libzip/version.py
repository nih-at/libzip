import typing

from .ctypes import library as lib

__all__ = ("libzip_version", "LIB_VERSION")


def libzip_version() -> typing.Tuple[int]:
	return tuple(int(el) for el in lib.libzip_version().split(b"."))


LIB_VERSION = libzip_version()
