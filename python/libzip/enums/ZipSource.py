from enum import IntEnum, IntFlag

__all__ = ("ZipSource", "ZIP_SOURCE", "ZipSourceB")


class ZipSource(IntEnum):
	open = 0
	read = open + 1
	close = read + 1
	stat = close + 1
	error = stat + 1
	free = error + 1
	seek = free + 1
	tell = seek + 1
	begin_write = tell + 1
	commit_write = begin_write + 1
	rollback_write = commit_write + 1
	write = rollback_write + 1
	seek_write = write + 1
	tell_write = seek_write + 1
	supports = tell_write + 1
	remove = supports + 1
	reserved_1 = remove + 1
	begin_write_cloning = reserved_1 + 1
	accept_empty = begin_write_cloning + 1
	get_file_attributes = accept_empty + 1


ZIP_SOURCE = ZipSource


class ZipSourceB(IntFlag):
	open = 1 << ZipSource.open
	read = 1 << ZipSource.read
	close = 1 << ZipSource.close
	stat = 1 << ZipSource.stat
	error = 1 << ZipSource.error
	free = 1 << ZipSource.free
	seek = 1 << ZipSource.seek
	tell = 1 << ZipSource.tell
	begin_write = 1 << ZipSource.begin_write
	commit_write = 1 << ZipSource.commit_write
	rollback_write = 1 << ZipSource.rollback_write
	write = 1 << ZipSource.write
	seek_write = 1 << ZipSource.seek_write
	tell_write = 1 << ZipSource.tell_write
	supports = 1 << ZipSource.supports
	remove = 1 << ZipSource.remove
	reserved_1 = 1 << ZipSource.reserved_1
	begin_write_cloning = 1 << ZipSource.begin_write_cloning
	accept_empty = 1 << ZipSource.accept_empty
	get_file_attributes = 1 << ZipSource.get_file_attributes

	supports_readable = open | read | close | stat | error | free
	supports_seekable = supports_readable | seek | tell | supports
	supports_writable = supports_seekable | begin_write | commit_write | rollback_write | write | seek_write | tell_write | remove
