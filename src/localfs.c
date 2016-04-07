/*

Copyright 2011-2016 Tyler Gilbert

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/
#include <sys/stat.h>

#ifndef __SIM__
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stratify/sysfs.h>
#include <mcu/debug.h>

#include "semihost_api.h"

/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define OPEN_R          0
#define OPEN_B          1
#define OPEN_PLUS       2
#define OPEN_W          4
#define OPEN_A          8
#define OPEN_INVALID   -1
int posix_to_semihost_open_flags(int flags) {
    /* POSIX flags -> semihosting open mode */
    int openmode;
    if (flags & O_RDWR) {
        /* a plus mode */
        openmode = OPEN_PLUS;
        if (flags & O_APPEND) {
            openmode |= OPEN_A;
        } else if (flags & O_TRUNC) {
            openmode |= OPEN_W;
        } else {
            openmode |= OPEN_R;
        }
    } else if (flags & O_WRONLY) {
        /* write or append */
        if (flags & O_APPEND) {
            openmode = OPEN_A;
        } else {
            openmode = OPEN_W;
        }
    } else if (flags == O_RDONLY) {
        /* read mode */
        openmode = OPEN_R;
    } else {
        /* invalid flags */
        openmode = OPEN_INVALID;
    }

    return openmode;
}



void localfs_unlock(const void * cfg){ //force unlock when a process exits
	return;
}


int localfs_init(const void * cfg){
	return 0;
}

int localfs_mkfs(const void * cfg){
	int ret = 0; return ret;
}

int localfs_fstat(const void * cfg, void * handle, struct stat * stat){
	int ret = 0;
	int h = (int)handle;

	//needs to be implemented
	memset(stat, 0, sizeof(struct stat));
	stat->st_size = semihost_flen(h);

	return ret;
}

int localfs_stat(const void * cfg, const char * path, struct stat * stat){
	int fd;

	fd = semihost_open(path, 0);
	if( fd < 0 ){
		errno = ENOENT;
		return -1;
	}

	memset(stat, 0, sizeof(struct stat));
	stat->st_size = semihost_flen(fd);
	semihost_close(fd);

	return 0;
}

int localfs_unlink(const void * cfg, const char * path){
	return semihost_remove(path);
}

int localfs_remove(const void * cfg, const char * path){
	return localfs_unlink(cfg, path);
}

int localfs_rename(const void * cfg, const char * old, const char * new){
	return semihost_rename(old, new);
}


int localfs_open(const void * cfg, void ** handle, const char * path, int flags, int mode){
	int fd;
	int openmode = posix_to_semihost_open_flags(flags);

	if( openmode == OPEN_INVALID ){
		errno = EINVAL;
		return -1;
	}

	mcu_debug("Open mode flags 0x%X\n", openmode);

	fd = semihost_open(path, openmode);
	if ( fd >= 0 ){
		*handle = (void*)fd;
		return 0; //success
	}

	errno = EIO;
	return -1;
}

int localfs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte){
	int h = (int)handle;
	semihost_seek(h, loc);
	return semihost_read(h, buf, nbyte, 0666);
}


int localfs_write(const void * cfg, void * handle, int flags, int loc, const void * buf, int nbyte){
	int h = (int)handle;
	semihost_seek(h, loc);
	return semihost_write(h, buf, nbyte, 0666);
}

int localfs_close(const void * cfg, void ** handle){
	int h = (int)handle;
	return semihost_close(h);
}

