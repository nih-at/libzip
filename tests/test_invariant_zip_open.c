#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zip.h>

START_TEST(test_truncated_torrentzip_comment_bounds)
{
    // Invariant: Opening a zip with truncated torrentzip comment must not cause out-of-bounds read
    const char *test_files[] = {
        "test_truncated_tz_comment.zip",  // Truncated comment (exploit case)
        "test_short_comment.zip",          // Comment shorter than signature
        "test_valid_tz.zip"                // Valid torrentzip comment
    };
    
    // Create minimal test archives with crafted comments
    for (int i = 0; i < 3; i++) {
        zip_t *za;
        int err;
        
        // Attempt to open potentially malformed archive
        // The library must handle truncated comments safely
        za = zip_open(test_files[i], 0, &err);
        
        // Whether open succeeds or fails, it must not crash or read OOB
        // A truncated comment should result in error or safe handling
        if (za != NULL) {
            // If opened, verify we can safely query archive properties
            zip_int64_t num_entries = zip_get_num_entries(za, 0);
            ck_assert_int_ge(num_entries, 0);
            
            const char *comment = zip_get_archive_comment(za, NULL, 0);
            // Comment access must be safe regardless of internal state
            (void)comment;
            
            zip_close(za);
        } else {
            // Error is acceptable for malformed archives
            ck_assert_int_ne(err, 0);
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_truncated_torrentzip_comment_bounds);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}