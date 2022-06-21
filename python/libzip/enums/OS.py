from enum import IntEnum

__all__ = ("OS", "ZIP_OPSYS")


class OS(IntEnum):
	dos = 0
	amiga = 1
	openvms = 2
	unix = 3
	vm_cms = 4
	atari_st = 5
	os_2 = 6
	macintosh = 7
	z_system = 8
	cpm = 9
	windows_ntfs = 10
	mvs = 11
	vse = 12
	acorn_risc = 13
	vfat = 14
	alternate_mvs = 15
	beos = 16
	tandem = 17
	os_400 = 18
	os_x = 19
	default = unix


ZIP_OPSYS = OS
