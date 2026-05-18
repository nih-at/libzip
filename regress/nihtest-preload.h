#ifndef PRELOAD_H
#define PRELOAD_H

#ifdef __APPLE__
/**
 * Get name for replacement function.
 * 
 * @param name name of the function to be replaced
 * @return name of the replacement function
 */
#define PRELOAD_NAME(name) name##_replacement

/**
 * Replace a function with a preloaded version.
 *
 * This must be used after the replacement function is defined.
 *
 * @param name name of the function to be replaced
 */
#define PRELOAD_REPLACE(name) \
    __attribute__((used)) static struct { const void *PRELOAD_NAME(name); const void *name; } \
    _interpose_##name __attribute__((section("__DATA,__interpose"))) = \
    { (const void *)(unsigned long)&PRELOAD_NAME(name), (const void *)(unsigned long)&name }

#else
/**
 * Get name for replacement function.
 * 
 * @param name name of the function to be replaced
 * @return name of the replacement function
 */
#define PRELOAD_NAME(name) name

/**
 * Replace a function with a preloaded version.
 * 
 * This must be used after the replacement function is defined.
 *
 * @param name name of the function to be replaced
 */
#define PRELOAD_REPLACE(name) /* nothing to do */
#endif

#endif /* PRELOAD_H */
