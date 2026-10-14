# Thread Safety

<!-- --8<-- [start:summary] -->
This page explains how to use libzip in a multi-threaded environment and which access to libzip objects needs to be synchronized.
<!-- --8<-- [end:summary] -->

## No Internal Synchronization

To remain portable and small, libzip does not use any threading library and does not perform any locking. It is therefore the responsibility of the caller to synchronize access to libzip objects if they are shared between threads.

Also, the caller knows which structures are shared between threads and which aren't, and can therefore synchronize access to them more efficiently than libzip could.


## Independent Archives

In general, different zip archives opened by libzip are independent of each other. You can use different zip archives in different threads without locking, as long as no one archive is accessed by multiple threads.

If you want to use the same archive from multiple threads, you have to synchronize access to it yourself.  All uses of an archive have to be synchronized, since even read-only operations can modify the archive's internal data structures or read from the underlying source.

Access to any sources obtained from an archive has to be synchronized with each other and the archive itself, since they share the same underlying data structures and source.


## Dependent Archives

If you use an archive as a source for [`zip_file_add`](../documentation/zip_file_add) or [`zip_file_replace`](../documentation/zip_file_replace), the target archive will access the source archive. In this case, you have to synchronize access to both archives as one.
