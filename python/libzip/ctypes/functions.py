from ctypes import CFUNCTYPE, POINTER, c_char_p, c_double, c_int, c_int8, c_int16, c_int32, c_int64, c_uint8, c_uint16, c_uint32, c_uint64, c_void_p

from deprecation import deprecated

from ..version import LIB_VERSION
from ._ctypesgen_preamble import _variadic_function
from ._funcToCtypesSignatureConvertor import assignTypesFromFunctionSignature as atffs
from ._inttypes import zip_flags, zip_index, zip_source_cmd
from .callbacks import zip_cancel_callback, zip_progress_callback, zip_source_callback
from .library import lib
from .opaque import FILE_P, zip_file_ptr, zip_ptr, zip_source_ptr
from .structs import time_t, zip_buffer_fragment, zip_error, zip_file_attributes, zip_stat

# pylint:disable=too-many-arguments


def archive_set_tempdir(za: zip_ptr, tempdir: c_char_p) -> c_int:
	return _archive_set_tempdir(za, tempdir)


_archive_set_tempdir = atffs(archive_set_tempdir, lib)


def close(za: zip_ptr) -> c_int:
	return _close(za)


_close = atffs(close, lib)


def delete(za: zip_ptr, idx: zip_index) -> c_int:
	return _delete(za, idx)


_delete = atffs(delete, lib)


def dir_add(za: zip_ptr, name: c_char_p, flags: zip_flags) -> c_int64:
	return _dir_add(za, name, flags)


_dir_add = atffs(dir_add, lib)


def discard(za: zip_ptr) -> None:
	return _discard(za)


_discard = atffs(discard, lib)


def get_error(za: zip_ptr) -> POINTER(zip_error):
	return _get_error(za)


_get_error = atffs(get_error, lib)


def error_clear(za: zip_ptr) -> None:
	return _error_clear(za)


_error_clear = atffs(error_clear, lib)


def error_code_zip(error: POINTER(zip_error)) -> c_int:
	return _error_code_zip(error)


_error_code_zip = atffs(error_code_zip, lib)


def error_code_system(error: POINTER(zip_error)) -> c_int:
	return _error_code_system(error)


_error_code_system = atffs(error_code_system, lib)


def error_fini(err: POINTER(zip_error)) -> None:
	return _error_fini(err)


_error_fini = atffs(error_fini, lib)


def error_init(err: POINTER(zip_error)) -> None:
	return _error_init(err)


_error_init = atffs(error_init, lib)


def error_init_with_code(error: POINTER(zip_error), ze: c_int) -> None:
	return _error_init_with_code(error, ze)


_error_init_with_code = atffs(error_init_with_code, lib)


def error_set(err: POINTER(zip_error), ze: c_int, se: c_int) -> None:
	return _error_set(err, ze, se)


_error_set = atffs(error_set, lib)


def error_strerror(err: POINTER(zip_error)) -> c_char_p:
	return _error_strerror(err)


_error_strerror = atffs(error_strerror, lib)


def error_system_type(error: POINTER(zip_error)) -> c_int:
	return _error_system_type(error)


_error_system_type = atffs(error_system_type, lib)


def error_to_data(error: POINTER(zip_error), data: c_void_p, length: c_uint64) -> c_int64:
	return _error_to_data(error, data, length)


_error_to_data = atffs(error_to_data, lib)


def fclose(zf: zip_file_ptr) -> c_int:
	return _fclose(zf)


_fclose = atffs(fclose, lib)


def fdopen(fd_orig: c_int, _flags: c_int, errorp: POINTER(c_int)) -> zip_ptr:
	return _fdopen(fd_orig, _flags, errorp)


_fdopen = atffs(fdopen, lib)


def file_add(za: zip_ptr, name: c_char_p, source: zip_source_ptr, flags: zip_flags) -> c_int64:
	return _file_add(za, name, source, flags)


_file_add = atffs(file_add, lib)


def file_attributes_init(attributes: POINTER(zip_file_attributes)) -> None:
	return _file_attributes_init(attributes)


_file_attributes_init = atffs(file_attributes_init, lib)


def file_error_clear(zf: zip_file_ptr) -> None:
	return _file_error_clear(zf)


_file_error_clear = atffs(file_error_clear, lib)


def file_extra_field_delete(za: zip_ptr, idx: zip_index, ef_idx: c_uint16, flags: zip_flags) -> c_int:
	return _file_extra_field_delete(za, idx, ef_idx, flags)


_file_extra_field_delete = atffs(file_extra_field_delete, lib)


def file_extra_field_delete_by_id(za: zip_ptr, idx: zip_index, ef_id: c_uint16, ef_idx: c_uint16, flags: zip_flags) -> c_int:
	return _file_extra_field_delete_by_id(za, idx, ef_id, ef_idx, flags)


_file_extra_field_delete_by_id = atffs(file_extra_field_delete_by_id, lib)


def file_extra_field_set(za: zip_ptr, idx: zip_index, ef_id: c_uint16, ef_idx: c_uint16, data: POINTER(c_uint8), length: c_uint16, flags: zip_flags) -> c_int:
	return _file_extra_field_set(za, idx, ef_id, ef_idx, data, length, flags)


_file_extra_field_set = atffs(file_extra_field_set, lib)


def file_extra_fields_count(za: zip_ptr, idx: zip_index, flags: zip_flags) -> c_int16:
	return _file_extra_fields_count(za, idx, flags)


_file_extra_fields_count = atffs(file_extra_fields_count, lib)


def file_extra_fields_count_by_id(za: zip_ptr, idx: zip_index, ef_id: c_uint16, flags: zip_flags) -> c_int16:
	return _file_extra_fields_count_by_id(za, idx, ef_id, flags)


_file_extra_fields_count_by_id = atffs(file_extra_fields_count_by_id, lib)


def file_extra_field_get(za: zip_ptr, idx: zip_index, ef_idx: c_uint16, idp: POINTER(c_uint16), lenp: POINTER(c_uint16), flags: zip_flags) -> POINTER(c_uint8):
	return _file_extra_field_get(za, idx, ef_idx, idp, lenp, flags)


_file_extra_field_get = atffs(file_extra_field_get, lib)


def file_extra_field_get_by_id(za: zip_ptr, idx: zip_index, ef_id: c_uint16, ef_idx: c_uint16, lenp: POINTER(c_uint16), flags: zip_flags) -> POINTER(c_uint8):
	return _file_extra_field_get_by_id(za, idx, ef_id, ef_idx, lenp, flags)


_file_extra_field_get_by_id = atffs(file_extra_field_get_by_id, lib)


def file_get_comment(za: zip_ptr, idx: zip_index, lenp: POINTER(c_uint32), flags: zip_flags) -> c_char_p:
	return _file_get_comment(za, idx, lenp, flags)


_file_get_comment = atffs(file_get_comment, lib)


def file_get_error(f: zip_file_ptr) -> POINTER(zip_error):
	return _file_get_error(f)


_file_get_error = atffs(file_get_error, lib)


def file_get_external_attributes(za: zip_ptr, idx: zip_index, flags: zip_flags, opsys: POINTER(c_uint8), attributes: POINTER(c_uint32)) -> c_int:
	return _file_get_external_attributes(za, idx, flags, opsys, attributes)


_file_get_external_attributes = atffs(file_get_external_attributes, lib)


def file_rename(za: zip_ptr, idx: zip_index, name: c_char_p, flags: zip_flags) -> c_int:
	return _file_rename(za, idx, name, flags)


_file_rename = atffs(file_rename, lib)


def file_replace(za: zip_ptr, idx: zip_index, source: zip_source_ptr, flags: zip_flags) -> c_int:
	return _file_replace(za, idx, source, flags)


_file_replace = atffs(file_replace, lib)


def file_set_comment(za: zip_ptr, idx: zip_index, comment: c_char_p, length: c_uint16, flags: zip_flags) -> c_int:
	return _file_set_comment(za, idx, comment, length, flags)


_file_set_comment = atffs(file_set_comment, lib)


def file_set_dostime(za: zip_ptr, idx: zip_index, dtime: c_uint16, ddate: c_uint16, flags: zip_flags) -> c_int:
	return _file_set_dostime(za, idx, dtime, ddate, flags)


_file_set_dostime = atffs(file_set_dostime, lib)


def file_set_encryption(za: zip_ptr, idx: zip_index, method: c_uint16, password: c_char_p) -> c_int:
	return _file_set_encryption(za, idx, method, password)


_file_set_encryption = atffs(file_set_encryption, lib)


def file_set_external_attributes(za: zip_ptr, idx: zip_index, flags: zip_flags, opsys: c_uint8, attributes: c_uint32) -> c_int:
	return _file_set_external_attributes(za, idx, flags, opsys, attributes)


_file_set_external_attributes = atffs(file_set_external_attributes, lib)


def file_set_mtime(za: zip_ptr, idx: zip_index, mtime: time_t, flags: zip_flags) -> c_int:
	return _file_set_mtime(za, idx, mtime, flags)


_file_set_mtime = atffs(file_set_mtime, lib)


def file_strerror(zf: zip_file_ptr) -> c_char_p:
	return _file_strerror(zf)


_file_strerror = atffs(file_strerror, lib)


def fopen(za: zip_ptr, fname: c_char_p, flags: zip_flags) -> zip_file_ptr:
	return _fopen(za, fname, flags)


_fopen = atffs(fopen, lib)
fopen = deprecated(deprecated_in=None, removed_in=None, current_version=None, details="fopen_index")(fopen)


def fopen_encrypted(za: zip_ptr, fname: c_char_p, flags: zip_flags, password: c_char_p) -> zip_file_ptr:
	return _fopen_encrypted(za, fname, flags, password)


_fopen_encrypted = atffs(fopen_encrypted, lib)
fopen_encrypted = deprecated(deprecated_in=None, removed_in=None, current_version=None, details="fopen_index_encrypted")(fopen_encrypted)


def fopen_index(za: zip_ptr, index: c_uint64, flags: zip_flags) -> zip_file_ptr:
	return _fopen_index(za, index, flags)


_fopen_index = atffs(fopen_index, lib)


def fopen_index_encrypted(za: zip_ptr, index: c_uint64, flags: zip_flags, password: c_char_p) -> zip_file_ptr:
	return _fopen_index_encrypted(za, index, flags, password)


_fopen_index_encrypted = atffs(fopen_index_encrypted, lib)


def fread(zf: zip_file_ptr, outbuf: c_void_p, toread: c_uint64) -> c_int64:
	return _fread(zf, outbuf, toread)


_fread = atffs(fread, lib)


def fseek(zf: zip_file_ptr, offset: c_int64, whence: c_int) -> c_int8:
	return _fseek(zf, offset, whence)


_fseek = atffs(fseek, lib)


if LIB_VERSION >= (1, 9, 0):

	def file_is_seekable(zf: zip_file_ptr) -> c_int:
		return _file_is_seekable(zf)

	_file_is_seekable = atffs(file_is_seekable, lib)


def ftell(zf: zip_file_ptr) -> c_int64:
	return _ftell(zf)


_ftell = atffs(ftell, lib)


def get_archive_comment(za: zip_ptr, lenp: POINTER(c_int), flags: zip_flags) -> c_char_p:
	return _get_archive_comment(za, lenp, flags)


_get_archive_comment = atffs(get_archive_comment, lib)


def get_archive_flag(za: zip_ptr, flag: zip_flags, flags: zip_flags) -> c_int:
	return _get_archive_flag(za, flag, flags)


_get_archive_flag = atffs(get_archive_flag, lib)


def get_name(za: zip_ptr, idx: zip_index, flags: zip_flags) -> c_char_p:
	return _get_name(za, idx, flags)


_get_name = atffs(get_name, lib)


def get_num_entries(za: zip_ptr, flags: zip_flags) -> c_int64:
	return _get_num_entries(za, flags)


_get_num_entries = atffs(get_num_entries, lib)


def name_locate(za: zip_ptr, fname: c_char_p, flags: zip_flags) -> c_int64:
	return _name_locate(za, fname, flags)


_name_locate = atffs(name_locate, lib)


def open(src: c_char_p, flags: c_int, error: POINTER(c_int)) -> zip_ptr:  # pylint:disable=redefined-builtin
	return _open(src, flags, error)


_open = atffs(open, lib)


def open_from_source(src: zip_source_ptr, _flags: c_int, error: POINTER(zip_error)) -> zip_ptr:
	return _open_from_source(src, _flags, error)


_open_from_source = atffs(open_from_source, lib)
userDataFreer = CFUNCTYPE(None, c_void_p)
userDataFreer_p = POINTER(userDataFreer)


def register_progress_callback_with_state(za: zip_ptr, precision: c_double, callback: zip_progress_callback, ud_free: userDataFreer_p, ud: c_void_p) -> None:
	return _register_progress_callback_with_state(za, precision, callback, ud_free, ud)


_register_progress_callback_with_state = atffs(register_progress_callback_with_state, lib)


def register_cancel_callback_with_state(za: zip_ptr, callback: zip_cancel_callback, ud_free: userDataFreer_p, ud: c_void_p) -> None:
	return _register_cancel_callback_with_state(za, callback, ud_free, ud)


_register_cancel_callback_with_state = atffs(register_cancel_callback_with_state, lib)


def set_archive_comment(za: zip_ptr, comment: c_char_p, length: c_uint16) -> c_int:
	return _set_archive_comment(za, comment, length)


_set_archive_comment = atffs(set_archive_comment, lib)


def set_archive_flag(za: zip_ptr, flag: zip_flags, value: c_int) -> c_int:
	return _set_archive_flag(za, flag, value)


_set_archive_flag = atffs(set_archive_flag, lib)


def set_default_password(za: zip_ptr, passwd: c_char_p) -> c_int:
	return _set_default_password(za, passwd)


_set_default_password = atffs(set_default_password, lib)


def set_file_compression(za: zip_ptr, idx: zip_index, method: c_int32, flags: c_uint32) -> c_int:
	return _set_file_compression(za, idx, method, flags)


_set_file_compression = atffs(set_file_compression, lib)


def source_begin_write(src: zip_source_ptr) -> c_int:
	return _source_begin_write(src)


_source_begin_write = atffs(source_begin_write, lib)


def source_begin_write_cloning(src: zip_source_ptr, offset: c_uint64) -> c_int:
	return _source_begin_write_cloning(src, offset)


_source_begin_write_cloning = atffs(source_begin_write_cloning, lib)


def source_buffer(za: zip_ptr, data: c_void_p, length: c_uint64, freep: c_int) -> zip_source_ptr:
	return _source_buffer(za, data, length, freep)


_source_buffer = atffs(source_buffer, lib)
source_buffer = deprecated(deprecated_in=None, removed_in=None, current_version=None, details="source_buffer_create")(source_buffer)


def source_buffer_create(data: c_void_p, length: c_uint64, freep: c_int, error: POINTER(zip_error)) -> zip_source_ptr:
	return _source_buffer_create(data, length, freep, error)


_source_buffer_create = atffs(source_buffer_create, lib)


def source_buffer_fragment(za: zip_ptr, fragments: POINTER(zip_buffer_fragment), nfragments: c_uint64, freep: c_int) -> zip_source_ptr:
	return _source_buffer_fragment(za, fragments, nfragments, freep)


_source_buffer_fragment = atffs(source_buffer_fragment, lib)
source_buffer_fragment = deprecated(deprecated_in=None, removed_in=None, current_version=None, details="source_buffer_fragment_create")(source_buffer_fragment)


def source_buffer_fragment_create(fragments: POINTER(zip_buffer_fragment), nfragments: c_uint64, freep: c_int, error: POINTER(zip_error)) -> zip_source_ptr:
	return _source_buffer_fragment_create(fragments, nfragments, freep, error)


_source_buffer_fragment_create = atffs(source_buffer_fragment_create, lib)


def source_close(src: zip_source_ptr) -> c_int:
	return _source_close(src)


_source_close = atffs(source_close, lib)


def source_commit_write(src: zip_source_ptr) -> c_int:
	return _source_commit_write(src)


_source_commit_write = atffs(source_commit_write, lib)


def source_error(src: zip_source_ptr) -> POINTER(zip_error):
	return _source_error(src)


_source_error = atffs(source_error, lib)


def source_file(za: zip_ptr, fname: c_char_p, start: c_uint64, length: c_int64) -> zip_source_ptr:
	return _source_file(za, fname, start, length)


_source_file = atffs(source_file, lib)
source_file = deprecated(deprecated_in=None, removed_in=None, current_version=None, details="source_file_create")(source_file)


def source_file_create(fname: c_char_p, start: c_uint64, length: c_int64, error: POINTER(zip_error)) -> zip_source_ptr:
	return _source_file_create(fname, start, length, error)


_source_file_create = atffs(source_file_create, lib)


def source_filep(za: zip_ptr, file: FILE_P, start: c_uint64, length: c_int64) -> zip_source_ptr:
	return _source_filep(za, file, start, length)


_source_filep = atffs(source_filep, lib)
source_filep = deprecated(deprecated_in=None, removed_in=None, current_version=None, details="source_filep_create")(source_filep)


def source_filep_create(file: FILE_P, start: c_uint64, length: c_int64, error: POINTER(zip_error)) -> zip_source_ptr:
	return _source_filep_create(file, start, length, error)


_source_filep_create = atffs(source_filep_create, lib)


def source_free(src: zip_source_ptr) -> None:
	return _source_free(src)


_source_free = atffs(source_free, lib)


def source_function(za: zip_ptr, zcb: zip_source_callback, ud: c_void_p) -> zip_source_ptr:
	return _source_function(za, zcb, ud)


_source_function = atffs(source_function, lib)
source_function = deprecated(deprecated_in=None, removed_in=None, current_version=None, details="source_function_create")(source_function)


def source_function_create(zcb: zip_source_callback, ud: c_void_p, error: POINTER(zip_error)) -> zip_source_ptr:
	return _source_function_create(zcb, ud, error)


_source_function_create = atffs(source_function_create, lib)


def source_get_file_attributes(src: zip_source_ptr, attributes: POINTER(zip_file_attributes)) -> c_int:
	return _source_get_file_attributes(src, attributes)


_source_get_file_attributes = atffs(source_get_file_attributes, lib)


def source_is_deleted(src: zip_source_ptr) -> c_int:
	return _source_is_deleted(src)


_source_is_deleted = atffs(source_is_deleted, lib)


def source_keep(src: zip_source_ptr) -> None:
	return _source_keep(src)


_source_keep = atffs(source_keep, lib)


_source_make_command_bitmap = _variadic_function(lib.zip_source_make_command_bitmap, c_int64, [zip_source_cmd], None)


def source_open(src: zip_source_ptr) -> c_int:
	return _source_open(src)


_source_open = atffs(source_open, lib)


def source_read(src: zip_source_ptr, data: c_void_p, length: c_uint64) -> c_int64:
	return _source_read(src, data, length)


_source_read = atffs(source_read, lib)


def source_rollback_write(src: zip_source_ptr) -> None:
	return _source_rollback_write(src)


_source_rollback_write = atffs(source_rollback_write, lib)


def source_seek(src: zip_source_ptr, offset: c_int64, whence: c_int) -> c_int:
	return _source_seek(src, offset, whence)


_source_seek = atffs(source_seek, lib)


def source_seek_compute_offset(offset: c_uint64, length: c_uint64, data: c_void_p, data_length: c_uint64, error: POINTER(zip_error)) -> c_int64:
	return _source_seek_compute_offset(offset, length, data, data_length, error)


_source_seek_compute_offset = atffs(source_seek_compute_offset, lib)


def source_seek_write(src: zip_source_ptr, offset: c_int64, whence: c_int) -> c_int:
	return _source_seek_write(src, offset, whence)


_source_seek_write = atffs(source_seek_write, lib)


def source_stat(src: zip_source_ptr, st: POINTER(zip_stat)) -> c_int:
	return _source_stat(src, st)


_source_stat = atffs(source_stat, lib)


def source_tell(src: zip_source_ptr) -> c_int64:
	return _source_tell(src)


_source_tell = atffs(source_tell, lib)


def source_tell_write(src: zip_source_ptr) -> c_int64:
	return _source_tell_write(src)


_source_tell_write = atffs(source_tell_write, lib)


def source_write(src: zip_source_ptr, data: c_void_p, length: c_uint64) -> c_int64:
	return _source_write(src, data, length)


_source_write = atffs(source_write, lib)


def source_zip(za: zip_ptr, srcza: zip_ptr, srcidx: c_uint64, flags: zip_flags, start: c_uint64, length: c_int64) -> zip_source_ptr:
	return _source_zip(za, srcza, srcidx, flags, start, length)


_source_zip = atffs(source_zip, lib)


def stat(za: zip_ptr, fname: c_char_p, flags: zip_flags, st: POINTER(zip_stat)) -> c_int:
	return _stat(za, fname, flags, st)


_stat = atffs(stat, lib)


def stat_index(za: zip_ptr, index: c_uint64, flags: zip_flags, st: POINTER(zip_stat)) -> c_int:
	return _stat_index(za, index, flags, st)


_stat_index = atffs(stat_index, lib)


def stat_init(st: POINTER(zip_stat)) -> None:
	return _stat_init(st)


_stat_init = atffs(stat_init, lib)


def strerror(za: zip_ptr) -> c_char_p:
	return _strerror(za)


_strerror = atffs(strerror, lib)


def unchange(za: zip_ptr, idx: zip_index) -> c_int:
	return _unchange(za, idx)


_unchange = atffs(unchange, lib)


def unchange_all(za: zip_ptr) -> c_int:
	return _unchange_all(za)


_unchange_all = atffs(unchange_all, lib)


def unchange_archive(za: zip_ptr) -> c_int:
	return _unchange_archive(za)


_unchange_archive = atffs(unchange_archive, lib)


def compression_method_supported(method: c_int32, compress: c_int) -> c_int:
	return _compression_method_supported(method, compress)


_compression_method_supported = atffs(compression_method_supported, lib)


def encryption_method_supported(method: c_uint16, encode: c_int) -> c_int:
	return _encryption_method_supported(method, encode)


_encryption_method_supported = atffs(encryption_method_supported, lib)


# Just to ignore them
_DEPRECATED_API = {
	"register_progress_callback": (register_progress_callback_with_state,),
	"add": (file_add, file_replace),
	"add_dir": (dir_add,),
	"get_file_comment": (file_get_comment,),
	"set_file_comment": (file_set_comment,),
	"get_num_files": (get_num_entries,),
	"rename": (file_rename,),
	"replace": (file_replace,),
	"error_get_sys_type": (error_system_type,),
	"error_get": (get_error, error_code_zip, error_code_system),
	"error_to_str": (error_init_with_code, error_strerror),
	"file_error_get": (file_get_error, error_code_zip, error_code_system),
}
