TRACE_NATIVE_CALLS = False

if TRACE_NATIVE_CALLS:
	import sys

	def _traceArgs(name, argNames, args, kwargs, rawFunc):
		identifiedArgNames = argNames[: len(args)]
		positionalKwargs = dict(zip(identifiedArgNames, args))
		fullKwargs = {}
		fullKwargs.update(positionalKwargs)
		fullKwargs.update(kwargs)
		argsStr = "(" + ", ".join(k + "=" + repr(v) for k, v in fullKwargs.items()) + ")"
		callString = name + argsStr
		sys.stderr.flush()
		sys.stderr.write(callString)
		sys.stderr.flush()
		res = rawFunc(*args, **kwargs)
		sys.stderr.write("\33[2K\r")
		sys.stderr.write(callString + " -> " + repr(res) + "\n")
		sys.stderr.flush()
		return res


def assignTypesFromFunctionSignature(func, lib):
	rawFunc = getattr(lib, "zip_" + func.__name__)
	argNames = func.__code__.co_varnames[: func.__code__.co_argcount]
	rawFunc.argtypes = [func.__annotations__[argName] for argName in argNames]
	rawFunc.restype = func.__annotations__["return"]

	if TRACE_NATIVE_CALLS:

		def tracerFunc(*args, **kwargs):
			return _traceArgs(func.__name__, argNames, args, kwargs, rawFunc)

		return tracerFunc
	return rawFunc
