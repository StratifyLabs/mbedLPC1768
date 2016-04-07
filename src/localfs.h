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
#ifndef LOCALFS_H_
#define LOCALFS_H_

#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stratify/sysfs.h>


typedef struct {
	int list_block;
	int serialno_killed;
	int serialno;
} localfs_state_t;

typedef struct {
	const sysfs_t * devfs;
	const void * dev_cfg;
	open_file_t * open_file;
	const char name[NAME_MAX];
	localfs_state_t * state;
} localfs_cfg_t;




int localfs_init(const void * cfg); //initialize the filesystem
int localfs_mkfs(const void * cfg);

int localfs_opendir(const void * cfg, void ** handle, const char * path);
int localfs_readdir_r(const void * cfg, void * handle, int loc, struct dirent * entry);
int localfs_closedir(const void * cfg, void * handle);

int localfs_fstat(const void * cfg, void * handle, struct stat * stat);
int localfs_open(const void * cfg, void ** handle, const char * path, int flags, int mode);
int localfs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte);
int localfs_write(const void * cfg, void * handle, int flags, int loc, const void * buf, int nbyte);
int localfs_close(const void * cfg, void ** handle);
int localfs_remove(const void * cfg, const char * path);
int localfs_rename(const void * cfg, const char * old, const char * new);
int localfs_unlink(const void * cfg, const char * path);

void localfs_unlock(const void * cfg);

int localfs_stat(const void * cfg, const char * path, struct stat * stat);

#define LOCALFS(mount_loc_name, cfgp, access_mode) { \
		.mount_path = mount_loc_name, \
		.access = access_mode, \
		.mount = localfs_init, \
		.unmount = SYSFS_NOTSUP, \
		.ismounted = sysfs_always_mounted, \
		.startup = SYSFS_NOTSUP, \
		.mkfs = localfs_mkfs, \
		.open = localfs_open, \
		.priv_read = NULL, \
		.priv_write = NULL, \
		.read = localfs_read, \
		.write = localfs_write, \
		.close = localfs_close, \
		.priv_ioctl = SYSFS_NOTSUP, \
		.rename = localfs_rename, \
		.unlink = localfs_unlink, \
		.mkdir = SYSFS_NOTSUP, \
		.rmdir = SYSFS_NOTSUP, \
		.remove = localfs_remove, \
		.opendir = SYSFS_NOTSUP, \
		.closedir = SYSFS_NOTSUP, \
		.readdir_r = SYSFS_NOTSUP, \
		.link = SYSFS_NOTSUP, \
		.symlink = SYSFS_NOTSUP, \
		.stat = localfs_stat, \
		.lstat = SYSFS_NOTSUP, \
		.fstat = localfs_fstat, \
		.chmod = SYSFS_NOTSUP, \
		.chown = SYSFS_NOTSUP, \
		.unlock = localfs_unlock, \
		.cfg = cfgp, \
}


#endif /* LOCALFS_H_ */
