/*
  acl_preserve.c -- verify that replacing an archive preserves its POSIX access ACL
  SPDX-License-Identifier: BSD-3-Clause
*/

#include "config.h"

#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "zip.h"

#define ACL_EA_ACCESS "system.posix_acl_access"
#define ACL_EA_DEFAULT "system.posix_acl_default"
#define ACL_EA_VERSION 0x0002
#define ACL_UNDEFINED_ID UINT32_MAX

#define ACL_USER_OBJ 0x01
#define ACL_USER 0x02
#define ACL_GROUP_OBJ 0x04
#define ACL_MASK 0x10
#define ACL_OTHER 0x20

#define ACL_READ 0x04
#define ACL_WRITE 0x02

struct acl_header {
    uint32_t version;
};

struct acl_entry {
    uint16_t tag;
    uint16_t permissions;
    uint32_t id;
};

struct test_acl {
    struct acl_header header;
    struct acl_entry entries[5];
};

static void set_entry(struct acl_entry *entry, uint16_t tag, uint16_t permissions, uint32_t id) {
    entry->tag = htole16(tag);
    entry->permissions = htole16(permissions);
    entry->id = htole32(id);
}

int main(void) {
    static const char archive_name[] = "acl-preserve.zip";
    static const char default_dir[] = "acl-preserve-default";
    static const char inherited_name[] = "acl-preserve-default/acl-preserve.zip";
    static const char contents[] = "data";
    unsigned char before[sizeof(struct test_acl)];
    unsigned char after[sizeof(struct test_acl)];
    struct test_acl acl;
    struct stat st;
    FILE *file = NULL;
    zip_source_t *source = NULL;
    zip_t *archive = NULL;
    zip_error_t error;
    ssize_t before_length;
    ssize_t after_length;
    int error_code;
    int default_dir_created = 0;
    int result = 1;

    (void)remove(archive_name);

    archive = zip_open(archive_name, ZIP_CREATE | ZIP_TRUNCATE, &error_code);
    if (archive == NULL) {
        fprintf(stderr, "cannot create archive: %d\n", error_code);
        goto done;
    }
    source = zip_source_buffer(archive, contents, sizeof(contents) - 1, 0);
    if (source == NULL || zip_file_add(archive, "data", source, 0) < 0) {
        fprintf(stderr, "cannot add archive entry\n");
        if (source != NULL) {
            zip_source_free(source);
            source = NULL;
        }
        zip_discard(archive);
        archive = NULL;
        goto done;
    }
    source = NULL; /* owned by archive */
    if (zip_close(archive) < 0) {
        fprintf(stderr, "cannot finish initial archive\n");
        zip_discard(archive);
        archive = NULL;
        goto done;
    }
    archive = NULL;

    acl.header.version = htole32(ACL_EA_VERSION);
    set_entry(&acl.entries[0], ACL_USER_OBJ, ACL_READ | ACL_WRITE, ACL_UNDEFINED_ID);
    set_entry(&acl.entries[1], ACL_USER, ACL_READ | ACL_WRITE, 65534);
    set_entry(&acl.entries[2], ACL_GROUP_OBJ, 0, ACL_UNDEFINED_ID);
    set_entry(&acl.entries[3], ACL_MASK, ACL_READ | ACL_WRITE, ACL_UNDEFINED_ID);
    set_entry(&acl.entries[4], ACL_OTHER, 0, ACL_UNDEFINED_ID);

    if (setxattr(archive_name, ACL_EA_ACCESS, &acl, sizeof(acl), 0) < 0) {
        if (errno == ENOTSUP || errno == EOPNOTSUPP || errno == EPERM) {
            result = 77;
            goto done;
        }
        perror("setxattr");
        goto done;
    }
    before_length = getxattr(archive_name, ACL_EA_ACCESS, before, sizeof(before));
    if (before_length < 0) {
        perror("getxattr before");
        goto done;
    }

    archive = zip_open(archive_name, 0, &error_code);
    if (archive == NULL) {
        fprintf(stderr, "cannot reopen archive: %d\n", error_code);
        goto done;
    }
    if (zip_set_archive_comment(archive, "changed", 7) < 0 || zip_close(archive) < 0) {
        fprintf(stderr, "cannot update archive\n");
        zip_discard(archive);
        archive = NULL;
        goto done;
    }
    archive = NULL;

    after_length = getxattr(archive_name, ACL_EA_ACCESS, after, sizeof(after));
    if (after_length != before_length || memcmp(before, after, (size_t)before_length) != 0) {
        fprintf(stderr, "POSIX access ACL changed during archive replacement\n");
        goto done;
    }

    zip_error_init(&error);
    source = zip_source_file_create(archive_name, 0, -1, &error);
    if (source == NULL) {
        fprintf(stderr, "cannot create named file source: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        goto done;
    }
    if (zip_source_begin_write(source) < 0 || zip_source_write(source, contents, sizeof(contents) - 1) != sizeof(contents) - 1 || zip_source_commit_write(source) < 0) {
        fprintf(stderr, "cannot replace named file source: %s\n", zip_error_strerror(zip_source_error(source)));
        zip_source_free(source);
        source = NULL;
        zip_error_fini(&error);
        goto done;
    }
    zip_source_free(source);
    source = NULL;
    zip_error_fini(&error);

    after_length = getxattr(archive_name, ACL_EA_ACCESS, after, sizeof(after));
    if (after_length != before_length || memcmp(before, after, (size_t)before_length) != 0) {
        fprintf(stderr, "POSIX access ACL changed during direct source replacement\n");
        goto done;
    }

    if (mkdir(default_dir, 0700) < 0) {
        perror("mkdir");
        goto done;
    }
    default_dir_created = 1;
    if (setxattr(default_dir, ACL_EA_DEFAULT, &acl, sizeof(acl), 0) < 0) {
        if (errno == ENOTSUP || errno == EOPNOTSUPP || errno == EPERM) {
            result = 77;
            goto done;
        }
        perror("setxattr default ACL");
        goto done;
    }
    file = fopen(inherited_name, "wb");
    if (file == NULL || fwrite(contents, 1, sizeof(contents) - 1, file) != sizeof(contents) - 1) {
        perror("create inherited file");
        goto done;
    }
    if (fclose(file) < 0) {
        file = NULL;
        perror("close inherited file");
        goto done;
    }
    file = NULL;
    if (getxattr(inherited_name, ACL_EA_ACCESS, NULL, 0) <= 0) {
        fprintf(stderr, "source did not inherit the directory default ACL\n");
        goto done;
    }
    if (removexattr(inherited_name, ACL_EA_ACCESS) < 0 || chmod(inherited_name, 0640) < 0) {
        perror("clear inherited ACL and set source mode");
        goto done;
    }
    if (getxattr(inherited_name, ACL_EA_ACCESS, NULL, 0) != -1 || errno != ENODATA) {
        fprintf(stderr, "source still has an access ACL before replacement\n");
        goto done;
    }

    zip_error_init(&error);
    source = zip_source_file_create(inherited_name, 0, -1, &error);
    if (source == NULL) {
        fprintf(stderr, "cannot create ACL-free file source: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        goto done;
    }
    if (zip_source_begin_write(source) < 0 || zip_source_write(source, contents, sizeof(contents) - 1) != sizeof(contents) - 1 || zip_source_commit_write(source) < 0) {
        fprintf(stderr, "cannot replace ACL-free file source: %s\n", zip_error_strerror(zip_source_error(source)));
        zip_source_free(source);
        source = NULL;
        zip_error_fini(&error);
        goto done;
    }
    zip_source_free(source);
    source = NULL;
    zip_error_fini(&error);
    if (getxattr(inherited_name, ACL_EA_ACCESS, NULL, 0) != -1 || errno != ENODATA) {
        fprintf(stderr, "replacement inherited a directory access ACL\n");
        goto done;
    }
    if (stat(inherited_name, &st) < 0 || (st.st_mode & 0777) != 0640) {
        fprintf(stderr, "replacement did not retain the source mode\n");
        goto done;
    }

    puts("POSIX access ACL preserved");
    result = 0;

done:
    if (archive != NULL) {
        zip_discard(archive);
    }
    if (source != NULL) {
        zip_source_free(source);
    }
    if (file != NULL) {
        fclose(file);
    }
    if (default_dir_created) {
        (void)remove(inherited_name);
        (void)rmdir(default_dir);
    }
    (void)remove(archive_name);
    return result;
}
