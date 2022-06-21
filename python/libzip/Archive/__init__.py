import typing
from pathlib import Path, PurePath

from .. import Source
from ..ctypes.opaque import zip_source_ptr
from ..enums import EncryptionMethod, OpenFlags, ZipFlags, ZipStat
from ..File import ADD_REPLACE_ARGS, COMMENT_FLAGS, ExistingFile, file_add
from ..Stat import stat
from .ctors import open  # pylint:disable=redefined-builtin
from .middleLevel import close, get_archive_comment, get_num_entries, set_archive_comment, set_default_password, zip_ptr

__all__ = ("Archive",)


class Archive:
	__slots__ = ("ptr", "path", "mode")

	FILE_FLAGS = ZipFlags(0)

	def __init__(self, path: Path, mode: OpenFlags) -> None:
		self.path = path
		self.mode = mode
		self.ptr = None

	def __enter__(self) -> "Archive":
		self.ptr = open(str(self.path), self.mode)
		return self

	def __exit__(self, ex_type, ex_value, traceback) -> None:
		self.close()

	def __len__(self) -> int:
		return get_num_entries(self.ptr, 0)

	@property
	def range(self) -> range:
		return range(len(self))

	def __getitem__(self, k: typing.Union[str, int]) -> ExistingFile:
		if isinstance(k, int):
			return ExistingFile(self, idx=k, path=None, flags=self.__class__.FILE_FLAGS, password=None)

		if isinstance(k, (str, bytes, PurePath)):
			return ExistingFile(self, idx=None, path=k, flags=self.__class__.FILE_FLAGS, password=None)

		raise ValueError("Incorrect type of key", k)

	def __iter__(self):
		for idx in self.range:
			yield ExistingFile(self, idx=idx, path=None, flags=self.__class__.FILE_FLAGS, password=None)

	@property
	def password(self):
		raise NotImplementedError

	@password.setter
	def password(self, pwd):
		set_default_password(self.ptr, pwd)

	@property
	def comment(self) -> typing.Optional[bytes]:
		return get_archive_comment(self.ptr, COMMENT_FLAGS)

	@comment.setter
	def comment(self, v):
		set_archive_comment(self.ptr, v)

	def add(self, path: PurePath, source: typing.Union[zip_source_ptr, Source.Source], flags: ZipFlags = FILE_FLAGS, addFlags: ZipFlags = ADD_REPLACE_ARGS) -> ExistingFile:
		idx = file_add(self.ptr, name=path, source=source, flags=addFlags)
		return ExistingFile(self, idx=idx, path=path, flags=flags, password=None)

	def close(self) -> None:
		if self.ptr is not None:
			close(self.ptr)
			self.ptr = None
