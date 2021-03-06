// Copyright (c) 2015-2016 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <wasi/core.h>
#include <errno.h>
#include <unistd.h>

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset) {
  if (offset < 0) {
    errno = EINVAL;
    return -1;
  }
  __wasi_iovec_t iov = {.buf = buf, .buf_len = nbyte};
  size_t bytes_read;
  __wasi_errno_t error =
      __wasi_fd_pread(fildes, &iov, 1, offset, &bytes_read);
  if (error != 0) {
    __wasi_fdstat_t fds;
#ifdef __wasilibc_unmodified_upstream
    if (error == ENOTCAPABLE && __wasi_fd_stat_get(fildes, &fds) == 0) {
#else
    if (error == ENOTCAPABLE && __wasi_fd_fdstat_get(fildes, &fds) == 0) {
#endif
      // Determine why we got ENOTCAPABLE.
      if ((fds.fs_rights_base & __WASI_RIGHT_FD_READ) == 0)
        error = EBADF;
      else
        error = ESPIPE;
    }
    errno = error;
    return -1;
  }
  return bytes_read;
}
