// Copyright 2021 Stratify Labs, See LICENSE.md for details

#include <sos/arch.h>

#include <device/appfs.h>
#include <device/mem.h>
#include <mcu/flash.h>
#include <sos/dev/appfs.h>
#include <sos/fs/appfs.h>
#include <sos/fs/assetfs.h>

#include "config.h"
#include "fs_config.h"

// configure the application filesystem to use the extra flash and RAM
// available in the system
#define APPFS_RAM_PAGES 32
u32 ram_usage_table[APPFS_RAM_USAGE_WORDS(APPFS_RAM_PAGES)] MCU_SYS_MEM;

const devfs_device_t flash0 =
    DEVFS_DEVICE("flash0", mcu_flash, 0, 0, 0, 0666, SYSFS_ROOT, S_IFBLK);

// Flash layout 16 x 4KB then 14 x 32KB
// First 4 sectors are 16KB bootloader
// Last 8 sectors are 256KB OS
// 12 x 4KB sectors starting at 0x00004000
// 6 x 32KB sectors starting at 0x00010000
// SRAM is 64KB starting at 0x10000000
// Peripheral SRAM is 32KB starting at 0x2000 0000
// OS uses Peripheral SRAM
const appfs_mem_config_t appfs_mem_config = {
    .usage_size = sizeof(ram_usage_table),
    .usage = ram_usage_table,
    .flash_driver = &flash0,
    .section_count = 3,
    .sections = {{.o_flags = MEM_FLAG_IS_FLASH,
                  .page_count = 12,
                  .page_size = 4 * 1024UL,
                  .address = 0x00004000},
                 {.o_flags = MEM_FLAG_IS_FLASH,
                  .page_count = 6,
                  .page_size = 32 * 1024UL,
                  .address = 0x00010000},
                 {.o_flags = MEM_FLAG_IS_RAM,
                  .page_count = APPFS_RAM_PAGES,
                  .page_size = MCU_RAM_PAGE_SIZE,
                  .address = 0x10000000}}};

const devfs_device_t mem0 = DEVFS_DEVICE(
    "mem0", appfs_mem, 0, &appfs_mem_config, 0, 0666, SYSFS_ROOT, S_IFBLK);


const sysfs_t sysfs_list[] = {
    // the folder for ram/flash applications
    APPFS_MOUNT("/app", &mem0, 0777, SYSFS_ROOT),
    // the list of devices
    DEVFS_MOUNT("/dev", devfs_list, 0777, SYSFS_ROOT),
    // the root filesystem (must be last)
    SYSFS_MOUNT("/", sysfs_list, 0777, SYSFS_ROOT), SYSFS_TERMINATOR};
