import typing
from collections.abc import ByteString
from ctypes import POINTER, byref, c_byte, c_int16, c_uint16, cast

from ..ctypes import functions as f
from ..ctypes._inttypes import zip_flags, zip_index
from ..ctypes.opaque import zip_ptr
from ..enums.Flags import ZipFlags
from ..Error import _checkArchiveErrorCode

__all__ = ("file_extra_field_delete", "file_extra_field_delete_by_id", "file_extra_field_set", "file_extra_fields_count", "file_extra_fields_count_by_id", "file_extra_field_get", "file_extra_field_get_by_id", "ExtraField")

# pylint:disable=too-many-arguments


def file_extra_field_delete(za: zip_ptr, idx: int, ef_idx: c_uint16, flags: ZipFlags) -> None:
	_checkArchiveErrorCode(za, f.file_extra_field_delete(za, zip_index(idx), ef_idx, zip_flags(flags)))


def file_extra_field_delete_by_id(za: zip_ptr, idx: int, ef_id: c_uint16, ef_idx: c_uint16, flags: ZipFlags) -> None:
	_checkArchiveErrorCode(za, f.file_extra_field_delete_by_id(za, zip_index(idx), ef_id, ef_idx, zip_flags(flags)))


def file_extra_field_set(za: zip_ptr, idx: int, ef_id: int, ef_idx: int, data: ByteString, flags: ZipFlags) -> None:
	_checkArchiveErrorCode(za, f.file_extra_field_set(za, zip_index(idx), c_uint16(ef_id), c_uint16(ef_idx), data, c_uint16(len(data)), zip_flags(flags)))


def file_extra_fields_count(za: zip_ptr, idx: int, flags: ZipFlags) -> c_int16:
	return _checkArchiveErrorCode(za, f.file_extra_fields_count(za, zip_index(idx), zip_flags(flags)))


def file_extra_fields_count_by_id(za: zip_ptr, idx: int, ef_id: int, flags: ZipFlags) -> c_int16:
	return _checkArchiveErrorCode(za, f.file_extra_fields_count_by_id(za, zip_index(idx), c_uint16(ef_id), zip_flags(flags)))


def file_extra_field_get(za: zip_ptr, idx: int, ef_idx: int, flags: ZipFlags) -> (bytes, int):
	size = c_uint16(0)
	ef_id = c_uint16(0)
	resPtr = _checkArchiveErrorCode(za, f.file_extra_field_get(za, zip_index(idx), c_uint16(ef_idx), byref(ef_id), byref(size), zip_flags(flags)))

	bufT = c_byte * size.value
	bufPtrT = POINTER(bufT)
	return (bytes(cast(resPtr, bufPtrT)[0]), ef_id.value)


def file_extra_field_get_by_id(za: zip_ptr, idx: int, ef_id: int, ef_idx: int, flags: ZipFlags) -> bytes:
	size = c_uint16(0)
	resPtr = _checkArchiveErrorCode(za, f.file_extra_field_get_by_id(za, zip_index(idx), c_uint16(ef_id), c_uint16(ef_idx), byref(size), zip_flags(flags)))

	bufT = c_byte * size.value
	bufPtrT = POINTER(bufT)
	return bytes(cast(resPtr, bufPtrT)[0])


EXTRA_FIELDS_FLAGS = ZipFlags.central | ZipFlags.local  # | ZipFlags.unchanged


class ExtraField:
	__slots__ = ("idx", "parent", "ef_idx")

	def __init__(self, parent: "ExtraFields", ef_idx: int):
		self.parent = parent
		self.ef_idx = ef_idx

	def flags(self):
		return self.parent.flags

	def ef_id(self):
		return self.parent.ef_id

	def __bytes__(self):
		if self.ef_id is not None:
			return file_extra_field_get_by_id(self.parent.archive.ptr, self.parent.parent.idx, self.ef_id, self.ef_idx, self.flags)

		return file_extra_field_get(self.parent.archive.ptr, self.parent.parent.idx, self.ef_idx, self.flags)

	def set(self, data: ByteString):
		if self.ef_id is not None:
			return file_extra_field_set(self.parent.archive.ptr, self.parent.parent.idx, self.ef_id, self.ef_idx, data, self.flags)

		raise ValueError("`ef_id` must be set")

	def delete(self):
		if self.ef_id is not None:
			return file_extra_field_delete_by_id(self.parent.archive.ptr, self.parent.parent.idx, self.ef_id, self.ef_idx, self.flags)

		return file_extra_field_delete(self.parent.archive.ptr, self.parent.parent.idx, self.ef_idx, self.flags)


class ExtraFields:
	__slots__ = ("parent", "ef_id", "flags")

	def __init__(self, parent: "ExistingFile", ef_id: int = None, flags: ZipFlags = EXTRA_FIELDS_FLAGS) -> None:
		self.parent = parent
		self.ef_id = ef_id
		self.flags = flags

	@property
	def archive(self):
		return self.parent.parent

	def __len__(self):
		if self.ef_id is not None:
			return file_extra_fields_count_by_id(self.archive.ptr, self.parent.idx, self.ef_id, self.flags)

		return file_extra_fields_count(self.archive.ptr, self.parent.idx, self.flags)

	def range(self) -> range:
		return range(len(self))

	def __iter__(self) -> typing.Iterator[ExtraField]:
		for i in self.range:  # pylint:disable=not-an-iterable
			yield ExtraField(self, i)

	def __getitem__(self, k: int):
		if k not in self.range:  # pylint:disable=unsupported-membership-test
			raise KeyError(k)

		return bytes(ExtraField(self, k))

	def __setitem__(self, k: int, v: ByteString):
		if k not in self.range:  # pylint:disable=unsupported-membership-test
			raise KeyError(k)

		return ExtraField(self, k).set(v)

	def __delitem__(self, k: int):
		if k not in self.range:  # pylint:disable=unsupported-membership-test
			raise KeyError(k)

		return ExtraField(self, k).delete()
