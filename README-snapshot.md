To build libzip from the repository using autotools, you need to
install autoconf, automake, and libtool; then run
```shell
autoreconf -fi
```

Afterwards you can run configure as usual.

Alternatively, use cmake.

```shell
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS:BOOL=ON
```

If you prefer building a static library, leave define `BUILD_SHARED_LIBS` to `OFF`
