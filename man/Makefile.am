MANFMT=@MANFMT@
man1_MANS=${MAN1:.mdoc=.${MANFMT}}
man3_MANS=${MAN3:.mdoc=.${MANFMT}}

DISTCLEANFILES=${HTML}
EXTRA_DIST=${MANDOC} ${MAN} CMakeLists.txt handle_links links

MAN1=	zipcmp.mdoc zipmerge.mdoc ziptool.mdoc
MAN3=	\
	ZIP_SOURCE_GET_ARGS.mdoc \
	libzip.mdoc \
	zip_add.mdoc \
	zip_add_dir.mdoc \
	zip_close.mdoc \
	zip_delete.mdoc \
	zip_dir_add.mdoc \
	zip_discard.mdoc \
	zip_error_clear.mdoc \
	zip_error_code_system.mdoc \
	zip_error_code_zip.mdoc \
	zip_error_fini.mdoc \
	zip_error_get.mdoc \
	zip_error_get_sys_type.mdoc \
	zip_error_init.mdoc \
	zip_error_set.mdoc \
	zip_error_strerror.mdoc \
	zip_error_system_type.mdoc \
	zip_error_to_data.mdoc \
	zip_error_to_str.mdoc \
	zip_errors.mdoc \
	zip_fclose.mdoc \
	zip_fdopen.mdoc \
	zip_file_add.mdoc \
	zip_file_extra_field_delete.mdoc \
	zip_file_extra_field_get.mdoc \
	zip_file_extra_field_set.mdoc \
	zip_file_extra_fields_count.mdoc \
	zip_file_get_comment.mdoc \
	zip_file_get_error.mdoc \
	zip_file_get_external_attributes.mdoc \
	zip_file_rename.mdoc \
	zip_file_set_comment.mdoc \
	zip_file_set_external_attributes.mdoc \
	zip_file_set_mtime.mdoc \
	zip_file_strerror.mdoc \
	zip_fopen.mdoc \
	zip_fopen_encrypted.mdoc \
	zip_fread.mdoc \
	zip_get_archive_comment.mdoc \
	zip_get_archive_flag.mdoc \
	zip_get_error.mdoc \
	zip_get_file_comment.mdoc \
	zip_get_name.mdoc \
	zip_get_num_entries.mdoc \
	zip_get_num_files.mdoc \
	zip_name_locate.mdoc \
	zip_open.mdoc \
	zip_rename.mdoc \
	zip_set_archive_comment.mdoc \
	zip_set_archive_flag.mdoc \
	zip_set_default_password.mdoc \
	zip_set_file_comment.mdoc \
	zip_set_file_compression.mdoc \
	zip_source.mdoc \
	zip_source_begin_write.mdoc \
	zip_source_buffer.mdoc \
	zip_source_close.mdoc \
	zip_source_commit_write.mdoc \
	zip_source_error.mdoc \
	zip_source_file.mdoc \
	zip_source_filep.mdoc \
	zip_source_free.mdoc \
	zip_source_function.mdoc \
	zip_source_is_deleted.mdoc \
	zip_source_keep.mdoc \
	zip_source_make_command_bitmap.mdoc \
	zip_source_open.mdoc \
	zip_source_read.mdoc \
	zip_source_rollback_write.mdoc \
	zip_source_seek.mdoc \
	zip_source_seek_compute_offset.mdoc \
	zip_source_seek_write.mdoc \
	zip_source_stat.mdoc \
	zip_source_tell.mdoc \
	zip_source_tell_write.mdoc \
	zip_source_win32a.mdoc \
	zip_source_win32handle.mdoc \
	zip_source_win32w.mdoc \
	zip_source_write.mdoc \
	zip_source_zip.mdoc \
	zip_stat.mdoc \
	zip_stat_init.mdoc \
	zip_unchange.mdoc \
	zip_unchange_all.mdoc \
	zip_unchange_archive.mdoc

install-data-hook: link-man3

LN=ln -f

link-man3: install-man3 uninstall-hook
	@${srcdir}/handle_links install directory=${DESTDIR}${man3dir} extension=3 command="${LN}" file=${srcdir}/links

uninstall-hook:
	@${srcdir}/handle_links uninstall directory=${DESTDIR}${man3dir} extension=3 command="rm -f" file=${srcdir}/links

link-html: ${HTML}
	@${srcdir}/handle_links uninstall directory=. extension=html command="rm -f" file=${srcdir}/links
	@${srcdir}/handle_links install directory=. extension=html command="${LN}" file=${srcdir}/links

MANDOC=${MAN1} ${MAN3}
HTML=${MANDOC:.mdoc=.html}
MAN=${MANDOC:.mdoc=.man}

SUFFIXES=.man .mdoc .html

.PHONY: mkman update-errors

mkman: ${MAN}
mkhtml: ${HTML} link-html

mkdocset: mkdocset.pl ${HTML}
	SRCDIR=${srcdir} perl ${srcdir}/mkdocset.pl ${HTML}

update-errors:
	sh $(srcdir)/make_zip_errors.sh $(srcdir)/../lib/zip.h zip_errors.mdoc

.mdoc.man:
	mandoc -T man $< | sed '/TH/s,"NetBSD [^"]*","NiH",' > $@.$$$$ && mv $@.$$$$ $@

.mdoc.html:
	mandoc -Thtml -Oman="%N.html",style=../nih-man.css $< | ${srcdir}/fix-man-links.sh > $@.$$$$ && mv $@.$$$$ $@
