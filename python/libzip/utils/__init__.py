import typing
from collections.abc import ByteString
from pathlib import PurePath

AnyStr = typing.Union[ByteString, str]
PathT = typing.Union[AnyStr, PurePath]


def acceptStrOrBytes(s: AnyStr) -> ByteString:
	if not isinstance(s, ByteString):
		s = s.encode("utf-8")
	assert isinstance(s, ByteString), "Must be bytes, a string or convertible to them, but " + repr(s) + " was the result of the possible conversions"
	return s


def acceptPathOrStrOrBytes(p: PathT) -> ByteString:
	if not isinstance(p, (PurePath, str, ByteString)):
		p = p.__path__()

	if isinstance(p, PurePath):
		p = str(p)

	assert isinstance(p, (PurePath, str, ByteString)), "Must be bytes, a string or a [Pure]Path or convertible to them, but " + repr(p) + " was the result of the possible conversions"

	return acceptStrOrBytes(p)


def toPurePathOrStrOrBytes(v: ByteString) -> PathT:
	if isinstance(v, ByteString):
		try:
			v = str(v, "utf-8")
		except UnicodeDecodeError:
			pass

	if isinstance(v, str):
		v = PurePath(v)

	return v
