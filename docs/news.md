# News

## Released libzip 1.12.1
<p class="post-date">October 14, 2026</p>

* Protect against denial of service attack (advisory GHSA-m3wq-j48p-jxwh, actually fixed in 1.12).
* Fix compile issues.
* Fix adding and replacing files in torrentzip archives.
* Fix error handling in `zip_source_filep_create()`.

!download 1.12.1


## Released libzip 1.12
<p class="post-date">October 7, 2026</p>

* Keep order of local extra fields, even if an identical extra field is present in the central directory.
* Add `zip_source_at_eof()` and `ZIP_SOURCE_AT_EOF`.
* Consistently report CRC and decryption errors when reading exactly to end-of-file.
* Remove Mbed TLS support - the required functions were made private.
* Make non-random tests work on macOS and Windows by using a special version of libzip.
* Securely clear passwords and key material before freeing it.
* Use SipHash hash function with per-table random keys. This prevents hash flooding attacks.
* Add compile time option to limit LZMA window size.
* Add more comprehensive fuzzers for metadata and write code paths.
* Fix `zip_source_is_seekable()`.
* Fix setting incorrect WinZIP AES version in some cases.
* Further improvements to overall stability and other minor adjustments have been made.

!download 1.12


## Website Overhaul
<p class="post-date">June 11, 2026</p>

The website is now built using Zensical, and the documentation has been split into four sections. This creates a place for documentation that doesn't fit into man pages.

Since Zensical does not support blogs, all news posts are now on a single page. The RSS feed is now generated with a different program, so posts might show up as duplicates in some feed readers.

Redirects have been added for URLs that have changed.


## Released libzip 1.11.4
<p class="post-date">May 23, 2025</p>

* Use separate cmake package files for library and programs.
* Improve documentation.

!download 1.11.4


## Released libzip 1.11.3
<p class="post-date">January 20, 2025</p>

* Report read error for corrupted encrypted file data.
* Avoid unnecessary seeks when writing archive.
* Don't hardcode `_Nullable` support in `zip.h` to allow it to be used with different compilers.
* Improve check for GetSecurityInformation availability on Windows.

!download 1.11.3


## Released libzip 1.11.2
<p class="post-date">October 31, 2024</p>

* Fix performance regression in `zip_stat` introduced in 1.11.

!download 1.11.2


## Released libzip 1.11.1
<p class="post-date">September 19, 2024</p>

* Fix zipconf.h for version number with missing third component.

!download 1.11.1


## Released libzip 1.11
<p class="post-date">September 19, 2024</p>

* Stop searching after finding acceptable central directory, even if it contains inconsistencies.
* Only write Zip64 EOCD if fields don't fit in normal EOCD. Previously libzip also wrote it when any directory entry required Zip64.
* Allow bytes from 0x00-0x1F as UTF-8.
* Add new error code `ZIP_ER_TRUNCATED_ZIP` for files that start with a valid local header signature.
* `zipcmp`: add `-T` option for comparing timestamps.
* `zip_file_replace` now removes the target's extra field information.

!download 1.11


## Released libzip 1.10.1
<p class="post-date">August 23, 2023</p>

* Add `ZIP_LENGTH_TO_END` and `ZIP_LENGTH_UNCHECKED`. Unless `ZIP_LENGTH_UNCHECKED` is used as `length`, it is an error for a file to shrink between the time when the source is created and when its data is read.
* Fix test on Windows.

!download 1.10.1


## Released libzip 1.10.0
<p class="post-date">June 23, 2023</p>

* Make support for layered sources public.
* Add `zip_source_zip_file` and `zip_source_zip_file_create`, deprecate `zip_source_zip` and `zip_source_zip_create`.
* Allow reading changed file data.
* Fix handling of files of size 4294967295.
* `zipmerge`: copy extra fields.
* `zipmerge`: add option to keep files uncompressed.
* Switch test framework to use nihtest instead of Perl.
* Fix reading/writing compressed data with buffers > 4GiB.
* Restore support for torrentzip.
* Add warnings when using deprecated functions.
* Allow keeping files for empty archives.
* Support Mbed TLS>=3.3.0.
* Support OpenSSL 3.
* Use ISO C secure library functions, if available.

!download 1.10.0


## Released libzip 1.9.2
<p class="post-date">June 28, 2022</p>

* Fix version number in header file.

!download 1.9.2


## Released libzip 1.9.1
<p class="post-date">June 28, 2022</p>

* Fix `zip_file_is_seekable()`.

!download 1.9.1


## Released libzip 1.9.0
<p class="post-date">June 13, 2022</p>

* Add `zip_file_is_seekable()`.
* Improve compatibility with WinAES.
* Fix encoding handling in `zip_name_locate()`.
* Add option to `zipcmp` to output summary of changes.
* Various bug fixes and documentation improvements.

!download 1.9.0


## Released libzip 1.8.0
<p class="post-date">June 18, 2021</p>

* Add support for zstd (Zstandard) compression.
* Add support for lzma (ID 14) compression.
* Add `zip_source_window_create()`.
* Add `zip_source_zip_create()` variant to `zip_source_zip()`.
* Allow method specific `comp_flags` in `zip_set_file_compression()`.
* Allow `zip_source_tell()` on sources that don't support seeking and `zip_ftell()` on compressed data.
* Provide more details for consistency check errors.
* Improve output of `zipcmp`.
* In `zipcmp`, don’t ignore empty directories when comparing directory listing.
* Treat empty string as no password given in `zip_file_set_encryption()`, `zip_fopen_encrypted()`, and `zip_set_default_password()`.

!download 1.8.0


## Released libzip 1.7.3
<p class="post-date">July 15, 2020</p>

* Support cmake < 3.17 again.
* Fix pkgconfig file (regression in 1.7.2).

!download 1.7.3


## Released libzip 1.7.2
<p class="post-date">July 11, 2020</p>

* Fixes for the CMake `find_project()` files.
* libzip moved to the CMake `libzip::` `NAMESPACE`.
* CMake usage best practice cleanups.

!download 1.7.2


## Released libzip 1.7.1
<p class="post-date">June 13, 2020</p>

* Restore `LIBZIP_VERSION_{MAJOR,MINOR,MICRO}` symbols.
* Fixes warnings reported by PVS-Studio.
* Add `LIBZIP_DO_INSTALL` build setting to make it easier to use
  libzip as subproject.

!download 1.7.1


## Released libzip 1.7.0
<p class="post-date">June 5, 2020</p>

* Add support for encrypting using traditional PKWare encryption.
* Add `zip_compression_method_supported()`.
* Add `zip_encryption_method_supported()`.
* Add the `ZIP_SOURCE_GET_FILE_ATTRIBUTES` source command.
* Refactor stdio file backend.
* Add CMake find_project() support.

!download 1.7.0


## Released libzip 1.6.1
<p class="post-date">February 3, 2020</p>

* Bugfix for double-free in `zipcmp(1)` during cleanup.

!download 1.6.1


## Released libzip 1.6.0
<p class="post-date">January 24, 2020</p>

* Avoid using `umask()` since it's not thread-safe.
* Set close-on-exec flag when opening files.
* Do not accept empty files as valid zip archives any longer.
* Add support for XZ compressed files (using liblzma).
* Add support for cancelling while closing zip archives.
* Add support for setting the time in the on-disk format.

!download 1.6.0


## Released libzip 1.5.2
<p class="post-date">March 12, 2019</p>

* Fix bug in AES encryption affecting certain file sizes
* Keep file permissions when modifying zip archives
* Support systems with small stack size.
* Support Mbed TLS as crypto backend.
* Add nullability annotations.

!download 1.5.2


## Released libzip 1.5.1
<p class="post-date">April 11, 2018</p>

* Choose format of installed documentation based on available tools.
* Fix visibility of symbols.
* Fix `zipcmp` directory support.
* Don't set RPATH on Linux.
* Use Libs.private for link dependencies in pkg-config file.
* Fix build with LibreSSL.
* Various bugfixes.

!download 1.5.1


## Released libzip 1.5.0
<p class="post-date">March 11, 2018</p>

* Use standard cryptographic library instead of custom AES implementation.
  This also simplifies the license.
* Use `clang-format` to format the source code.
* More Windows improvements.

!download 1.5.0


## Released libzip 1.4.0
<p class="post-date">December 29, 2017</p>

* Improve build with cmake
* Retire autoconf/automake build system
* Add `zip_source_buffer_fragment()`.
* Add support to clone unchanged beginning of archive (instead of rewriting it).
  Supported for buffer sources and on Apple File System.
* Add support for Microsoft Universal Windows Platform.

!download 1.4.0


## Official Repository Moved to GitHub
<p class="post-date">December 8, 2017</p>

We have moved the libzip repository from our own Mercurial server to GitHub: [nih-at/libzip](https://github.com/nih-at/libzip).

This makes it easier to contribute to libzip. Feel free to use the issue tracker to report bugs, or submit fixes via pull requests. For questions or to propose possible enhancements,  continue using the libzip-discuss mailing list.

hg.nih.at/libzip has been retired. If you have a Mercurial checkout, please check out a fresh copy of libzip from github:

```sh
git clone https://github.com/nih-at/libzip.git
```


## Released libzip 1.3.2
<p class="post-date">November 20, 2017</p>

* Fix bug introduced in last: `zip_t` was erroneously freed if `zip_close()` failed.

!download 1.3.2


## Released libzip 1.3.1
<p class="post-date">November 19, 2017</p>

* Install `zipconf.h` into `${PREFIX}/include`
* Add `zip_libzip_version()`
* Fix AES tests on Linux

!download 1.3.1


## Released libzip 1.3.0
<p class="post-date">September 2, 2017</p>

* Support bzip2 compressed zip archives
* Improve file progress callback code
* Fix `zip_fdopen()`
* CVE-2017-12858: Fix double `free()`
* CVE-2017-14107: Improve EOCD64 parsing

!download 1.3.0


## New Website
<p class="post-date">October 19, 2017</p>

libzip has moved to its own site at libzip.org. We've used the opportunity to completely overhaul the layout and structure of the site.

URLs pointing to nih.at/libzip will be redirected to the corresponding page on the new site.


## Released libzip 1.2.0
<p class="post-date">February 19, 2017</p>

* Support for AES encryption (Winzip version), both encryption
  and decryption
* Support legacy zip files with >64k entries
* Fix seeking in `zip_source_file` if `start` > 0
* Add `zip_fseek()` for seeking in uncompressed data
* Add `zip_ftell()` for telling position in uncompressed data
* Add `zip_register_progress_callback()` for UI updates during `zip_close()`

!download 1.2.0


## Released libzip 1.1.3
<p class="post-date">May 28, 2016</p>

* Fix build on Windows when using autoconf

!download 1.1.3


## Released libzip 1.1.2
<p class="post-date">February 19, 2016</p>

* Improve support for 3MF files

!download 1.1.2


## Released libzip 1.1.1
<p class="post-date">February 7, 2016</p>

* Build fixes for Linux
* Fix some warnings reported by PVS-Studio

!download 1.1.1


## Released libzip 1.1.0
<p class="post-date">January 26, 2016</p>

* ziptool(1): command line tool to modify zip archives
* Speedups for archives with many entries
* Coverity fixes
* Better APK support
* Support for running tests on Windows
* More build fixes for Windows
* Portability fixes
* Documentation improvements

!download 1.1.0


## Released libzip 1.0.1
<p class="post-date">May 4, 2015</p>

* Build fixes for Windows

!download 1.0.1


## Released libzip 1.0
<p class="post-date">May 3, 2015</p>

* Implemented an I/O abstraction layer
* Added support for native Windows API for files
* Added support for setting the last modification time for a file
* Added a new type zip_error_t for errors
* Added more typedefs for structs
* Torrentzip support was removed
* CVE-2015-2331 was fixed
* Addressed all Coverity CIDs

!download 1.0


## Released libzip 0.11.2
<p class="post-date">December 19, 2013</p>

* Support querying/setting operating system and external attributes
* For newly added files, set operating system to UNIX, permissions
  to 0666 (0777 for directories)
* Fix bug when writing zip archives containing files bigger than 4GB

!download 0.11.2


## Released libzip 0.11.1
<p class="post-date">April 27, 2013</p>

* Fix bugs in zip_set_file_compression()
* Include Xcode build infrastructure

!download 0.11.1


## Released libzip 0.11
<p class="post-date">March 23, 2013</p>

* Added Zip64 support (large file support)
* Added UTF-8 support for file names, file comments, and archive comments
* Changed API for name and comment related functions for UTF-8 support
* Added zip_discard()
* Added ZIP_TRUNCATE for zip_open()
* Added zip_set_file_compression()
* Added API for accessing and modifying extra fields
* Improved API type consistency
* Use gcc4's visibility __attribute__
* More changes for Windows support
* Additional test cases

!download 0.11


## Released libzip 0.10.1
<p class="post-date">March 20, 2012</p>

* Fixed CVE-2012-1162
* Fixed CVE-2012-1163

!download 0.10.1 bz2 gz


## Released libzip 0.10
<p class="post-date">March 18, 2010</p>

* Added zip_get_num_entries(), deprecated zip_get_num_files()
* Better windows support
* Support for traditional PKWARE encryption added
* Fix opening archives with more than 65535 entries
* Fix some memory leaks
* Fix cmake build and installation
* Fix memory leak in error case in zip_open()
* Fixed CVE-2011-0421 (no security implications though)
* More documentation

!download 0.10 bz2 gz


## Released libzip 0.9.3
<p class="post-date">February 1, 2010</p>

* Include m4/ directory in distribution; some packagers need it

!download 0.9.3 bz2 gz


## Released libzip 0.9.2
<p class="post-date">January 31, 2010</p>

* Avoid passing uninitialized data to deflate()
* Fix memory leak when closing zip archives

!download 0.9.2 bz2 gz


## Mailing list of Commit Messages
<p class="post-date">April 1, 2009</p>

Commits to the libzip repository are now also available via a mailing list. To subscribe, send a mail with the subject `subscribe libzip-changes` to minimalist at nih.at.


## Released libzip 0.9.1
<p class="post-date">January 24, 2010</p>

* Fix infinite loop on reading some broken files
* Optimization in time conversion (don't call localtime())
* Clear data descriptor flag in central directory, fixing Open Office files
* Allow more than 64k entries

!download 0.9.1 bz2 gz


## New libzip Mailing List
<p class="post-date">November 8, 2008</p>

There is now a discussion mailing list for developers using libzip. You can subscribe to it by sending a mail with the subject `subscribe libzip-discuss` to minimalist at nih.at.


## Major Webpage Overhaul
<p class="post-date">August 18, 2008</p>

The web page has been improved and new features have been added:

The repository browser is now integrated into the website; mailing list archives have been added; and news, release announcements and commits are available as RSS feeds.


## Released libzip 0.9
<p class="post-date">July 25, 2008</p>

* on Windows, explicitly set dllimport/dllexport
* remove erroneous references to GPL
* add support for torrentzip
* new functions: zip_get_archive_flag, zip_set_archive_flag
* zip_source_zip: add flag to force recompression
* zip_sorce_file: only keep file open while reading from it

!download 0.9 bz2 gz


## Released libzip 0.8
<p class="post-date">June 6, 2007</p>

* fix for zip archives larger than 2GiB
* fix zip_error_strerror to include libzip error string
* add support for reading streamed zip files
* new functions: zip_add_dir, zip_error_clear, zip_file_error_clear
* add basic support for building with CMake (incomplete)

!download 0.8 gz


## Released libzip 0.7.1
<p class="post-date">May 18, 2006</p>

* bugfix for zip_close

!download 0.7.1 gz

## Released libzip 0.7
<p class="post-date">May 6, 2006</p>

* struct zip_stat increased for future encryption support
* zip_add return value changed (now returns new index of added file)
* shared library major bump because of previous two
* added functions for reading and writing file and archive comments
  New functions: zip_get_archive_comment, zip_get_file_comment,
  zip_set_archive_comment, zip_set_file_comment, zip_unchange_archive

!download 0.7 gz


## Released libzip 0.6.1
<p class="post-date">July 14, 2005</p>

* various bug fixes

!download 0.6.1 gz


## Released libzip 0.6
<p class="post-date">June 9, 2005</p>

* first standalone release
* changed license to three-clause BSD
* overhauled API
* added man pages
* install zipcmp and zipmerge

!download 0.6 gz
