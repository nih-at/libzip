# Reference Documentation

This section contains reference documentation for libzip. It is meant to be comprehensive and detailed, covering all the functions, features, and components, but may not be the best place to start for new users. 

The concepts and design decisions of libzip are explained in the [Explanations](../explanations/index.md) section. 

To get started, please refer to the [Tutorials](../tutorials/index.md) section. For practical instructions, see the [How-To Guides](../guides/index.md) section.


## Command Line Tools

libzip includes the following command line tools:

[zipcmp](zipcmp.md)
: Compare content of  two zip archives or directories and report differences.

[zipmerge](zipmerge.md)
: Merge multiple zip archives into one.

[ziptool](ziptool.md)
: Perform various operations on zip archives, including getting information about them, extracting or adding files.


## Library

The following sections describe the functions and data types provided by libzip. They are organized by use case, roughly in order of importance or in the expected calling order. Each function is only listed once, so some functions listed in a previous section may also apply to later sections.

If you are looking for a specific function, use the search box in the header, or use your browser's search functionality to find it on this page.

### Reading Zip Archives

This section covers the function used for reading zip archives without modifying them.


#### Opening and Closing Archives

[zip_t](zip.md)
: The main data type representing a zip archive.

[zip_open, zip_open_from_source](zip_open.md)
: Open a zip archive.

[zip_fdopen](zip_fdopen.md)
: Open a zip archive from a file descriptor.

[zip_close](zip_close.md)
: Close a zip archive, writing any changes to disk.


#### Getting Archive Attributes

[zip_get_archive_comment](zip_get_archive_comment.md)
: Get the comment of a zip archive.

[zip_get_archive_flag](zip_get_archive_flag.md)
: Check one of the flags of a zip archive.

[zip_get_num_entries](zip_get_num_entries.md)
: Get the number of entries in a zip archive.


#### Finding Files

[zip_name_locate](zip_name_locate.md)
: Get the index of a file by its name.


#### Getting File Attributes

[zip_stat_init](zip_stat_init.md)
: Initialize a `zip_stat_t` structure.

[zip_file_get_comment](zip_file_get_comment.md)
: Get the comment of a file.

[zip_file_get_external_attributes](zip_file_get_external_attributes.md)
: Get the external attributes of a file.

[zip_get_name](zip_get_name.md)
: Get the name of a file.

[zip_file_extra_field_get, zip_file_extra_field_get_by_id](zip_file_extra_field_get.md)
: Get an extra field from a file.

[zip_file_extra_fields_count, zip_file_extra_fields_count_by_id](zip_file_extra_fields_count.md)
: Get the number of extra fields for a file.


#### Reading File Data

These are convenience functions wrapping a `zip_source_t` for reading file data. You can also get the source directly with [zip_source_zip_file](zip_source_zip_file.md).

[zip_file_t](zip_file.md)
: A file opened for reading.

[zip_fopen, zip_fopen_index](zip_fopen.md)
: Open a file for reading.

[zip_fopen_encrypted, zip_fopen_encrypted_index](zip_fopen_encrypted.md)
: Open an encrypted file for reading with the specified password.

[zip_fread](zip_fread.md)
: Read data from a file.

[zip_fseek, zip_file_is_seekable](zip_fseek.md)
: Seek to a position in a file.

[zip_ftell](zip_ftell.md)
: Get the current position in a file.

[zip_fclose](zip_fclose.md)
: Close a file.


#### Miscellaneous

[zip_set_default_password](zip_set_default_password.md)
: Set the default password for encrypted files in a zip archive.

[zip_compression_method_supported](zip_compression_method_supported.md)
: Check if a compression method is supported.

[zip_encryption_method_supported](zip_encryption_method_supported.md)
: Check if an encryption method is supported.


### Creating and Modifying Zip Archives

This section covers additional functions used for creating and modifying zip archives. For the functions to provide data to be added, see the [Creating Sources](#creating-sources) section below.


#### Changing Archive Attributes

[zip_set_archive_comment](zip_set_archive_comment.md)
: Set the comment of a zip archive.

[zip_set_archive_flag](zip_set_archive_flag.md)
: Set or clear one of the flags of a zip archive.


#### Changing Files and Directories

[zip_file_add, zip_file_replace](zip_file_add.md)
: Add or replace a file in a zip archive.

[zip_file_rename](zip_file_rename.md)
: Rename a file in a zip archive.

[zip_delete](zip_delete.md)
: Delete a file from a zip archive.

[zip_dir_add](zip_dir_add.md)
: Add a directory to a zip archive.

[zip_set_file_compression](zip_set_file_compression.md)
: Set the compression method of a file in a zip archive.

[zip_file_set_encryption](zip_file_set_encryption.md)
: Set the encryption method of a file in a zip archive.

[zip_file_set_comment](zip_file_set_comment.md)
: Set the comment of a file in a zip archive.

[zip_file_set_mtime, zip_file_set_dostime](zip_file_set_mtime.md)
: Set the modification time of a file in a zip archive.

[zip_file_set_external_attributes](zip_file_set_external_attributes.md)
: Set the external attributes of a file in a zip archive.

[zip_file_extra_field_set](zip_file_extra_field_set.md)
: Set an extra field for a file in a zip archive.

[zip_file_extra_field_delete, zip_file_extra_field_delete_by_id](zip_file_extra_field_delete.md)
: Delete an extra field from a file in a zip archive.


#### Reverting Changes

[zip_discard](zip_discard.md)
: Discard all changes to a zip archive and close it.

[zip_unchange_all](zip_unchange_all.md)
: Discard all file data changes to a zip archive.

[zip_unchange_archive](zip_unchange_archive.md)
: Discard all changes to a zip archive.

[zip_unchange](zip_unchange.md)
: Discard changes to a file in a zip archive.


#### Progress Callbacks

[zip_register_cancel_callback_with_state](zip_register_cancel_callback_with_state.md)
: Register a callback to allow cancelling zip_close().

[zip_register_progress_callback_with_state](zip_register_progress_callback_with_state.md)
: Register a callback to report progress during zip_close().


### Error Handling

This section covers the functions and data types used for error handling in libzip.

[zip_error_t](zip_error.md)
: A structure representing an error.

[zip_errors](zip_errors.md)
: List and description of all error codes.

[zip_strerror, zip_file_strerror](zip_file_strerror.md)
: Get a human-readable string describing the error of a `zip_file_t`.

[zip_error_clear, zip_file_error_clear](zip_error_clear.md)
: Clear error state of an archive or file.

[zip_get_error](zip_get_error.md)
: Get the error information of a `zip_t`.

[zip_file_get_error](zip_file_get_error.md)
: Get the error information of a `zip_file_t`.

[zip_error_strerror](zip_error_strerror.md)
: Get a human-readable string describing the error.

[zip_error_system_type](zip_error_system_type.md)
: Get the type of the system error code in a `zip_error_t`.

[zip_error_code_system](zip_error_code_system.md)
: Get the operating system part of a `zip_error_t`.

[zip_error_code_zip](zip_error_code_zip.md)
: Get the libzip part of a `zip_error_t`.

[zip_error_init, zip_error_init_with_code](zip_error_init.md)
: Initialize a `zip_error_t` structure.

[zip_error_fini](zip_error_fini.md)
: Free resources used by a `zip_error_t`.

[zip_error_set](zip_error_set.md)
: Fill in a `zip_error_t` structure.

### Sources

Sources are the abstraction libzip uses to represent file data. They are used for reading data from zip archives and providing data to be added to zip archives. Common sources are provided for files and memory buffers, and custom sources can be implemented by the user.


#### Creating Sources

[zip_source_t](zip_source.md)
: The data structure representing a source.

[zip_source_buffer, zip_source_buffer_create](zip_source_buffer.md)
: Create a source from a memory buffer.

[zip_source_file, zip_source_file_create](zip_source_file.md)
: Create a source from a file.

[zip_source_filep, zip_source_filep_create](zip_source_filep.md)
: Create a source from a `FILE` pointer.

[zip_source_win32a, zip_source_win32a_create](zip_source_win32a.md)
: Create a source from a file with ANSI name. (Windows only)

[zip_source_win32handle, zip_source_win32handle_create](zip_source_win32handle.md)
: Create a source from a Windows file handle. (Windows only)

[zip_source_win32w, zip_source_win32w_create](zip_source_win32w.md)
: Create a source from a file with UTF-16 name. (Windows only)

[zip_source_window_create](zip_source_window_create.md)
: Create a source from a section of another source.

[zip_source_zip_file, zip_source_zip_file_create](zip_source_zip_file.md)
: Create a source from a file in a zip archive.


#### Freeing Sources

[zip_source_free](zip_source_free.md)
: Decrease a source's reference count and free it if it reaches zero.

[zip_source_keep](zip_source_keep.md)
: Keep ownership of a source by increasing its reference count.

[zip_source_is_deleted](zip_source_is_deleted.md)
: Check if a source has been deleted.



#### Reading from a Source

[zip_source_stat](zip_source_stat.md)
: Get information about a source.

[zip_source_open](zip_source_open.md)
: Start reading from a source.

[zip_source_read](zip_source_read.md)
: Read data from a source.

[zip_source_at_eof](zip_source_at_eof.md)
: Check if a source is at end-of-file.

[zip_source_is_seekable](zip_source_is_seekable.md)
: Check if a source is seekable.

[zip_source_seek](zip_source_seek.md)
: Seek to a position in a source.

[zip_source_tell](zip_source_tell.md)
: Get the current position in a source.

[zip_source_close](zip_source_close.md)
: Stop reading from a source.


#### Writing to a Source

[zip_source_begin_write, zip_source_begin_write_cloning](zip_source_begin_write.md)
: Prepare a source for writing.

[zip_source_write](zip_source_write.md)
: Write data to a source.

[zip_source_seek_write](zip_source_seek_write.md)
: Seek to a position in a source for writing.

[zip_source_tell_write](zip_source_tell_write.md)
: Get the current position in a source for writing.

[zip_source_commit_write](zip_source_commit_write.md)
: Commit changes written to a source.

[zip_source_rollback_write](zip_source_rollback_write.md)
: Discard changes written to a source.

#### Implementing a Source

[zip_source_function, zip_source_function_create](zip_source_function.md)
: Create a source from a callback function.

[zip_source_layered, zip_source_layered_create](zip_source_layered.md)
: Create a source based on another source and a callback function.

[ZIP_SOURCE_GET_ARGS](ZIP_SOURCE_GET_ARGS.md)
: Validate and type cast arguments passed to a custom source callback.

[zip_source_make_command_bitmap](zip_source_make_command_bitmap.md)
: Compute bitmap of supported commands from a list of commands.

[zip_error_to_data](zip_error_to_data.md)
: Convert a `zip_error_t` to a value suitable for `ZIP_SOURCE_ERROR`.

[zip_source_seek_compute_offset](zip_source_seek_compute_offset.md)
: Compute the offset to seek to in a source based on seek arguments.

[zip_file_attributes_init](zip_file_attributes_init.md)
: Initialize a `zip_file_attributes_t` structure.

[zip_source_pass_to_lower_layer](zip_source_pass_to_lower_layer.md)
: Pass command to the lower layer of a layered source.

### Getting Library Version

[zip_libzip_version](zip_libzip_version.md)
: Get the version of libzip.


### Deprecated Functions

These functions are deprecated and should not be used in new code. They may be removed in a future major release. They are listed here for reference and to help with migrating old code; replacement functions are suggested for each of them.

[zip_add](zip_add.md)
: Add a file to a zip archive; use [zip_file_add](zip_file_add.md) instead.

[zip_add_dir](zip_add_dir.md)
: Add a directory to a zip archive; use [zip_dir_add](zip_dir_add.md) instead.

[zip_error_get](zip_error_get.md)
: Get the error information of a `zip_t`; use [zip_get_error](zip_get_error.md) instead.

[zip_error_to_str](zip_error_to_str.md)
: Get a human-readable string describing the error; use [zip_error_strerror](zip_error_strerror.md) instead.

[zip_error_get_sys_type](zip_error_get_sys_type.md)
: Get the type of the system error code in a `zip_error_t`; use [zip_error_system_type](zip_error_system_type.md) instead.

[zip_get_file_comment](zip_get_file_comment.md)
: Get the comment of a file in a zip archive; use [zip_file_get_comment](zip_file_get_comment.md) instead.

[zip_get_num_files](zip_get_num_files.md)
: Get the number of files in a zip archive; use [zip_get_num_entries](zip_get_num_entries.md) instead.

[zip_register_progress_callback](zip_register_progress_callback.md)
: Register a callback to report progress during zip_close(); use [zip_register_progress_callback_with_state](zip_register_progress_callback_with_state.md) instead.

[zip_rename](zip_rename.md)
: Rename a file in a zip archive; use [zip_file_rename](zip_file_rename.md) instead.

[zip_set_file_comment](zip_set_file_comment.md)
: Set the comment of a file in a zip archive; use [zip_file_set_comment](zip_file_set_comment.md) instead.

[zip_source_zip](zip_source_zip.md)
: Create a `zip_source_t` from a zip archive; use [zip_source_zip_file](zip_source_zip_file.md) instead.
