from enum import IntFlag

__all__ = ("ZipFlags", "ZIP_FL")


class ZipFlags(IntFlag):
	nocase = 1
	nodir = 1 << 1
	compressed = 1 << 2
	unchanged = 1 << 3
	recompress = 1 << 4
	encrypted = 1 << 5
	enc_guess = 0
	enc_raw = 1 << 6
	enc_strict = 1 << 7
	local = 1 << 8
	central = 1 << 9
	enc_utf_8 = 1 << 11
	enc_cp437 = 1 << 12
	overwrite = 1 << 13


ZIP_FL = ZipFlags
