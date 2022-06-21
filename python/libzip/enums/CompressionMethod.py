import zipfile
from enum import IntEnum

__all__ = ("CompressionMethod", "ZIP_CM")


class CompressionMethod(IntEnum):
	"""https://libzip.org/documentation/zip_set_file_compression.html"""

	default = -1
	store = ZIP_STORED = zipfile.ZIP_STORED
	shrink = 1
	reduce_1 = 2
	reduce_2 = 3
	reduce_3 = 4
	reduce_4 = 5
	implode = 6
	deflate = ZIP_DEFLATED = zipfile.ZIP_DEFLATED
	deflate64 = 9
	pkware_implode = 10
	bzip2 = ZIP_BZIP2 = zipfile.ZIP_BZIP2
	LZMA = ZIP_LZMA = zipfile.ZIP_LZMA
	terse = 18
	lz77 = 19
	lzma2 = 33
	zstd = 93
	xz = 95
	jpeg = 96
	wavpack = 97
	ppmd = 98


ZIP_CM = CompressionMethod
