from enum import IntEnum, IntFlag

from .CompressionMethod import *
from .Flags import *
from .OS import *
from .ZipError import *
from .ZipSource import *


class OpenFlags(IntFlag):
	"""https://libzip.org/documentation/zip_open.html"""

	read_write = create = ZIP_CREATE = 1
	dont_create = excl = ZIP_EXCL = 2
	check = check_consistency = checkcons = ZIP_CHECKCONS = 4
	overwrite = truncate = ZIP_TRUNCATE = 8
	read_only = rdonly = ZIP_RDONLY = 16


modesRemap = {
	"r": OpenFlags.read_only | OpenFlags.check,
	"w": OpenFlags.overwrite | OpenFlags.check,
	"x": OpenFlags.read_write | OpenFlags.dont_create | OpenFlags.check,
	"a": OpenFlags.read_write | OpenFlags.check,
}


class ArchiveFlags(IntFlag):
	read_only = rdonly = 2


ZIP_AFL = ArchiveFlags


class EncryptionMethod(IntEnum):
	"""https://libzip.org/documentation/zip_file_set_encryption.html"""

	none = 0
	trad_pkware = 1
	aes_128 = 257
	aes_192 = 258
	aes_256 = 259
	unknown = 65535


ZIP_EM = EncryptionMethod


class ZipStat(IntFlag):
	"""https://libzip.org/documentation/zip_stat.html"""

	name = 1
	index = 1 << 1
	originalSize = size = 1 << 2
	compressedSize = comp_size = 1 << 3
	modificationTime = mtime = 1 << 4
	crc = 1 << 5
	compressionMethod = comp_method = 1 << 6
	encryptionMethod = 1 << 7
	flags = 1 << 8

	everything = 0xFFFFFFFF


ZIP_STAT = ZipStat


class FileAttrs(IntFlag):
	host_system = 1
	ascii = 1 << 1
	version_needed = 1 << 2
	external_file_attributes = 1 << 3
	general_purpose_bit_flags = 1 << 4


ZIP_FILE_ATTRIBUTES = FileAttrs
