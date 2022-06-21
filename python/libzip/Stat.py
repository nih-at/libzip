import typing
from collections.abc import ByteString
from ctypes import POINTER, byref, c_int, c_uint64
from datetime import datetime
from enum import _decompose
from pathlib import PurePath

from .ctypes import functions as f
from .ctypes._inttypes import zip_flags
from .ctypes.opaque import zip_ptr
from .ctypes.structs import zip_stat
from .enums import CompressionMethod, EncryptionMethod, ZipStat
from .Error import _checkArchiveErrorCode
from .utils import PathT, acceptPathOrStrOrBytes


def stat(za: zip_ptr, fileNameOrIndex: typing.Union[PathT, int], what: ZipStat = ZipStat.everything) -> c_int:
	res = zip_stat()

	if isinstance(fileNameOrIndex, int):
		err = f.stat_index(za, c_uint64(fileNameOrIndex), zip_flags(int(what)), byref(res))
	elif isinstance(fileNameOrIndex, (PurePath, ByteString, str)):
		err = f.stat(za, acceptPathOrStrOrBytes(fileNameOrIndex), zip_flags(int(what)), byref(res))
	else:
		raise TypeError("fileNameOrIndex", "Must be either a file name or its index")

	_checkArchiveErrorCode(za, err, fileNameOrIndex)

	return Stat(res)


def stat_init(st: POINTER(zip_stat)) -> None:
	return f.stat_init(st)


class SubscriptableDateTime(datetime):
	__slots__ = ()

	INT2ATTRMAP = ("year", "month", "day", "hour", "minute", "second")

	def __getitem__(self, k) -> typing.Union[int, typing.Tuple[int, ...]]:
		k = self.__class__.INT2ATTRMAP[k]
		if isinstance(k, int):
			return getattr(self, k)

		return tuple(getattr(self, el) for el in k)


class Stat:
	__slots__ = ("res",)

	@property
	def name(self):
		if self.what & ZipStat.name:
			return self.res.name
		return None

	@property
	def index(self):
		if self.what & ZipStat.index:
			return self.res.index
		return None

	@index.setter
	def index(self, v):
		self.res.index = v
		self.what |= ZipStat.index

	@property
	def originalSize(self):
		if self.what & ZipStat.originalSize:
			return self.res.size
		return None

	@originalSize.setter
	def originalSize(self, v):
		self.res.size = v
		self.what |= ZipStat.originalSize

	@property
	def compressedSize(self):
		if self.what & ZipStat.compressedSize:
			return self.res.comp_size
		return None

	@compressedSize.setter
	def compressedSize(self, v):
		self.res.comp_size = v
		self.what |= ZipStat.compressedSize

	@property
	def modificationTime(self):
		if self.what & ZipStat.modificationTime:
			return SubscriptableDateTime.fromtimestamp(self.res.mtime)
		return None

	@modificationTime.setter
	def modificationTime(self, v):
		self.res.mtime = v
		self.what |= ZipStat.modificationTime

	@property
	def crc(self):
		if self.what & ZipStat.crc:
			return self.res.crc
		return None

	@crc.setter
	def crc(self, v):
		self.res.crc = v
		self.what |= ZipStat.crc

	@property
	def compressionMethod(self):
		if self.what & ZipStat.compressionMethod:
			return CompressionMethod(self.res.comp_method)
		return None

	@compressionMethod.setter
	def compressionMethod(self, v):
		self.res.comp_method = v
		self.what |= ZipStat.compressionMethod

	@property
	def encryptionMethod(self):
		if self.what & ZipStat.encryptionMethod:
			return EncryptionMethod(self.res.encryption_method)
		return None

	@encryptionMethod.setter
	def encryptionMethod(self, v):
		self.res.encryption_method = v
		self.what |= ZipStat.encryptionMethod

	@property
	def flags(self):
		if self.what & ZipStat.flags:
			return self.res.flags
		return None

	@flags.setter
	def flags(self, v):
		self.res.flags = v
		self.what |= ZipStat.flags

	@property
	def what(self) -> ZipStat:
		return ZipStat(self.res.valid)

	def __init__(self, res: zip_stat) -> None:
		self.res = res

	def __repr__(self):
		members, _uncovered = _decompose(self.what.__class__, self.what)
		return self.__class__.__name__ + "<" + ", ".join((el.name + "=" + repr(getattr(self, el.name))) for el in sorted(members)) + ">"
