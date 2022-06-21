# pylint:disable=too-few-public-methods

__all__ = ("_variadic_function",)


class _variadic_function:
	def __init__(self, func, restype, argtypes, errcheck):
		self.func = func
		self.func.restype = restype
		self.argtypes = argtypes
		if errcheck:
			self.func.errcheck = errcheck

	def _as_parameter_(self):
		# So we can pass this variadic function as a function pointer
		return self.func

	def __call__(self, *args):
		fixed_args = []
		i = 0
		for argtype in self.argtypes:
			# Type check what we can
			fixed_args.append(argtype.from_param(args[i]))
			i += 1
		return self.func(*fixed_args + list(args[i:]))
