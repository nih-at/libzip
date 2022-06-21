from ctypes import POINTER, Structure, c_char_p, c_int, c_int64, c_uint8, c_uint16, c_uint32, c_uint64

from ._inttypes import time_t

# pylint:disable=too-few-public-methods


class zip_source_args_seek(Structure):
	__slots__ = (
		"offset",
		"whence",
	)
	_fields_ = (
		("offset", c_int64),
		("whence", c_int),
	)


class zip_error(Structure):
	__slots__ = (
		"zip_err",
		"sys_err",
		"str",
	)
	_fields_ = (
		("zip_err", c_int),
		("sys_err", c_int),
		("str", c_char_p),
	)


class zip_stat(Structure):
	__slots__ = (
		"valid",
		"name",
		"index",
		"size",
		"comp_size",
		"mtime",
		"crc",
		"comp_method",
		"encryption_method",
		"flags",
	)
	_fields_ = (
		("valid", c_uint64),
		("name", c_char_p),
		("index", c_uint64),
		("size", c_uint64),
		("comp_size", c_uint64),
		("mtime", time_t),
		("crc", c_uint32),
		("comp_method", c_uint16),
		("encryption_method", c_uint16),
		("flags", c_uint32),
	)


class zip_buffer_fragment(Structure):
	__slots__ = (
		"data",
		"length",
	)
	_fields_ = (
		("data", POINTER(c_uint8)),
		("length", c_uint64),
	)


class zip_file_attributes(Structure):
	__slots__ = (
		"valid",
		"version",
		"host_system",
		"ascii",
		"version_needed",
		"external_file_attributes",
		"general_purpose_bit_flags",
		"general_purpose_bit_mask",
	)
	_fields_ = (
		("valid", c_uint64),
		("version", c_uint8),
		("host_system", c_uint8),
		("ascii", c_uint8),
		("version_needed", c_uint8),
		("external_file_attributes", c_uint32),
		("general_purpose_bit_flags", c_uint16),
		("general_purpose_bit_mask", c_uint16),
	)
