from enum import IntEnum

__all__ = ("ZipError", "ZIP_ER", "SystemErrorType", "ZIP_ET")


class ZipError(IntEnum):
	ok = no_error = 0
	multidisk_not_supported = multidisk = 1
	renaming_tempfile_failed = rename = 2
	close = 3
	seek = 4
	read = 5
	write = 6
	crc = 7
	zip_closed = zipclosed = 8
	no_such_file = ENOENT = noent = 9
	already_exists = exists = 10
	open = 11
	tmp_open = tmpopen = 12
	zlib = 13
	bad_alloc = memory = 14
	entry_changed = changed = 15
	compression_not_supported = compnotsupp = 16
	eof = 17
	invalid_argument = inval = 18
	not_zip = no_zip = nozip = 19
	internal_error = internal = 20
	inconsistent = incons = 21
	remove = 22
	deleted = 23
	encryption_not_supported = encrnotsupp = 24
	read_only = rdonly = 25
	password_required = no_passwd = nopasswd = 26
	wrong_password = wrong_passwd = wrongpasswd = 27
	operation_not_supported = opnotsupp = 28
	resource_in_use = in_use = inuse = 29
	tell = 30
	compressed_data_invalid = compressed_data = 31
	cancelled = 32


ZIP_ER = ZipError


class SystemErrorType(IntEnum):
	"""https://libzip.org/documentation/zip_error_system_type.html"""

	none = 0
	sys = 1
	zlib = 2
	libzip = 3


ZIP_ET = SystemErrorType
