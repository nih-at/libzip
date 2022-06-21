import typing
from datetime import date, time


def timeToDosTimeInt(dosTime: typing.Optional[time]) -> int:
	if dosTime:
		return dosTime.hour << 11 | dosTime.minute << 5 | (dosTime.second // 2)

	return 0


def dateToDosDateInt(dosDate: typing.Optional[date]) -> int:
	if dosDate:
		return (dosDate.year - 1980) << 9 | dosDate.month << 5 | dosDate.day

	return 0
