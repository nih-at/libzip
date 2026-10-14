# libzip

<p class="subtitle"> A C library for reading, creating, and modifying zip archives.</p>


## Why Use libzip?

libzip has been continuously developed since 2005. It is efficient, small, and flexible. It is usable on Linux, macOS, Windows, and many other operating systems.

The main design criteria are:

- Maintain a stable API without breaking backwards compatibility.
- Do not create corrupt files, even in case of errors.
- Do not lose data.
- Be efficient.

It supports the following features:

- reading archives and file data from files or memory buffers
- reverting unsaved changes
- support for Zip64 large archives
- deflate, bzip2, LZMA, and zstd compression
- Winzip AES and legacy PKWARE encryption

The [BSD license](license/) used for libzip allows its use in commercial products. 

## Who Uses libzip?

libzip is used in major open source projects like [KDE](https://kde.org/), [Chromium](https://www.chromium.org/Home), [ImageMagick](https://github.com/ImageMagick/ImageMagick/), and [VeraCrypt](https://www.veracrypt.fr/).

Commercial products using libzip include [Lightroom from Adobe](https://lightroom.adobe.com/) and the [Kobo eReader](http://www.kobo.com/desktop).

There are also bindings for other programming languages: [Python](https://github.com/KOLANICH-libs/libzip.py), [Ruby](http://rubygems.org/gems/zipruby/), [Lua](https://github.com/brimworks/lua-zip), [PHP](http://pecl.php.net/package/zip), and others.

There is a more complete [list of projects](/users/) using libzip.


## Getting Started

Most Linux and other Unix distributions include libzip in their package distributions, it is usually called `libzip` or `libzip-dev`. 

On macOS, it is included in both Homebrew and Mac Ports.

On Windows, it is in vcpkg.

A list of available packages can be found on [Repology](https://repology.org/project/libzip/versions). 

For building and installing libzip from source, see the [build instructions](guides/building.md).


## Using libzip

libzip is fully documented via man pages. HTML versions of the man
pages are in the [Reference](documentation/) section. Example source code is in the [examples](http://github.com/nih-at/libzip/tree/main/examples) and
[src](http://github.com/nih-at/libzip/tree/main/src) subdirectories.

If you have developed an application using libzip, you can find out
about API changes and how to adapt your code for them in [API Changes](explanations/api-changes.md).


## Staying in Touch

The official repository is at [GitHub](https://github.com/nih-at/libzip/). You can submit bug reports and feature requests via the [issue tracker](https://github.com/nih-at/libzip/issues), or submit [pull requests](https://github.com/nih-at/libzip/pulls) as well.

If you have questions about using libzip, you can use the [discussions](https://github.com/nih-at/libzip/discussions).

If you want to reach the authors in private, use <info@libzip.org>.

Current version is {{config.extra.version}}, released on {{ config.extra.release_date }}.

[![Packaging status](https://repology.org/badge/tiny-repos/libzip.svg)](https://repology.org/project/libzip/versions)

[![Github Actions Build Status](https://github.com/nih-at/libzip/workflows/build/badge.svg)](https://github.com/nih-at/libzip/actions?query=workflow%3Abuild)
[![Appveyor Build status](https://ci.appveyor.com/api/projects/status/f1bqqt9djvf22f5g?svg=true)](https://ci.appveyor.com/project/nih-at/libzip)
{#- [![Coverity Status](https://scan.coverity.com/projects/127/badge.svg)](https://scan.coverity.com/projects/libzip) -#}&nbsp;
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/libzip.svg)](https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&can=1&q=proj:libzip)
