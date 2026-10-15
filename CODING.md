# Coding Guidelines

These guidelines outline the coding standards and best practices for contributing to the libzip project. Following these guidelines helps maintain code quality, readability, and consistency across the project.

These are guidelines, not hard rules. If they get in the way of achieving their goals, use your best judgment, but don't disregard them without good reason. If you're unsure, ask us first.

## Portability

libzip is written in C89 and aims to be portable across various platforms. When contributing code, please ensure that it adheres to the following portability guidelines:

- Variables must be declared at the beginning of a block, before any executable statements.

- For fixed width integer types, use the `zip_int*_t` and `zip_uint*_t` types defined in `zipint.h`. Avoid using standard types `int*_t`, `uint*_t` directly.

- When using non-standard functions, detect their availability using cmake and provide a fallback implementation if necessary. Localize the use of `#ifdef` to one place (e.g. in `compat.h`) and use functions directly in the rest of the code.

Programs using libzip (i. e. code in the `src` and `examples` directories) should only use the public API and thus include `zip.h` and not `zipint.h`. Test programs should prefer `zip.h` but may include `zipint.h` if they test internal aspects of the library.

## Formatting

Use `clang-format` with the provided `.clang-format` configuration file to format your code. This ensures consistent code style across the project.

## Comments and Readability

If the code is not self-explanatory, add comments to clarify its purpose and logic. Use comments to describe complex algorithms, important decisions, and any non-obvious behavior or assumptions.

Avoid redundant comments that simply restate the code or reference bugs you fixed.

## Robustness

To help ensure the robustness of the code, avoid patterns that lead to errors or inconsistencies.

### Validate Arguments

Always validate function arguments to ensure they meet the expected criteria. For local functions where the arguments are known to be valid, this may be skipped.

### Managing Structures

When creating new structures, provide functions to create and initialize them, as well as functions to free them. This ensures proper memory management and encapsulation of the structure's implementation details:

```c
my_struct_t *my_struct_new(void);
void my_struct_free(my_struct_t *s);
```

If the structures are allocated on the stack or included in other structures, provide functions to initialize and finalize them. This ensures proper initialization and cleanup of the structure's resources:

```c
void my_struct_init(my_struct_t *s);
void my_struct_fini(my_struct_t *s);
```

### Avoid Code Duplication

If the same logic is needed in multiple places, consider creating a helper function instead of duplicating code. This promotes code reuse and maintainability.

## Public API

### Naming Conventions

Prefix all public API symbols with `zip_` or `ZIP_` to avoid name collisions with other libraries or user code.

Keep private symbols in the `zipint.h` header file. 

### Avoid Incompatible Changes

When making changes to the libzip API, maintain backwards compatibility. Avoid breaking changes unless absolutely necessary. If existing code needs to be adapted, document it in `API-CHANGES.md`.

### Private Structures

In most cases, the members of a structure should be kept private. This allows changing them later without breaking the API.

In `zipint.h`:
```c
struct zip_my_struct {
    int member1;
    char *member2;
};
```

In `zip.h`:
```c
typedef struct zip_my_struct my_struct_t;
```

### No Names for Arguments

In the public API, function declarations should not include names for arguments, as these might conflict with preprocessor defines in the calling code.

```c
ZIP_EXTERN int zip_open(const char * _Nonnull, int, int * _Nullable);
ZIP_EXTERN void zip_close(zip_t * _Nonnull);
```

### Extern Functions

All functions that are part of the public API should be declared with `ZIP_EXTERN` to ensure proper symbol visibility across different platforms and compilers:

```c
ZIP_EXTERN int zip_open(const char * _Nonnull path, int flags, int * _Nullable errorp);
ZIP_EXTERN void zip_close(zip_t * _Nonnull archive);
```

### Allowing NULL

Whether a function argument or return value can be `NULL` should be annotated in the function declaration using `_Nonnull` and `_Nullable` macros:

```c
ZIP_EXTERN int zip_open(const char * _Nonnull path, int flags, int * _Nullable errorp);
ZIP_EXTERN void zip_close(zip_t * _Nonnull archive);
ZIP_EXTERN const char *_Nonnull zip_libzip_version(void);
```

## Testing

We use [nihtest](https://nih.at/nihtest/) for testing. It is a framework to test command-line programs, allowing to set up input files and comparing output against expected results.

We prefer data driven tests over one-off programs. For API level tests, use `ziptool_regress`.

We plan to add tools that allow testing of other components, e.g. sources. These will take commands from stdin, one command per line, and be able to run a series of commands in a single test. This allows for more complex testing scenarios and better coverage of the codebase.

We plan to use a framework for C unit tests. This will make writing tests not covered by the methods above easier and more consistent. 

### Test Interface

If a test contains multiple sub-tests, all of them should be run, even if some fail. This ensures that all aspects of the code are tested and any issues are identified.

A test program should exit with status code 0 if all sub-tests succeed, 1 if any sub-test fails, and 77 if the test was skipped. 

If it is run in verbose mode, it should print details of each failure, including status and difference between expected and actual results for each sub-test.

## Documentation

All public functions, types, and macros should be documented in the provided man pages, which also serve as the reference section of the website.
