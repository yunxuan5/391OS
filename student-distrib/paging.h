#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define PDE_SIZE    1024
#define PTE_SIZE    1024
#define VIDEO_ADDR  0xB8000
#define _132MB      0x8400000

typedef union PDE_4MB_t {
    uint32_t val;
    struct {
        uint32_t present            : 1;
        uint32_t read_write         : 1;
        uint32_t user_supervisor    : 1;
        uint32_t write_through      : 1;
        uint32_t cache_disabled     : 1;
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t page_size          : 1;
        uint32_t global             : 1;
        uint32_t available          : 3;
        uint32_t attribute_index    : 1;
        uint32_t reserved           : 9;
        uint32_t table_address      : 10;
    } __attribute__ ((packed));
} PDE_4MB_t;

typedef union PDE_4KB_t {
    uint32_t val;
    struct {
        uint32_t present            : 1;
        uint32_t read_write         : 1;
        uint32_t user_supervisor    : 1;
        uint32_t write_through      : 1;
        uint32_t cache_disabled     : 1;
        uint32_t accessed           : 1;
        uint32_t reserved           : 1;
        uint32_t page_size          : 1;
        uint32_t global             : 1;
        uint32_t available          : 3;
        uint32_t table_address      : 20;
    } __attribute__ ((packed));
} PDE_4KB_t;

typedef union PDE_t
{
    PDE_4MB_t MB;
    PDE_4KB_t KB;
} PDE_t;

typedef union PTE_t {
    uint32_t val;
    struct {
        uint32_t present            : 1;
        uint32_t read_write         : 1;
        uint32_t user_supervisor    : 1;
        uint32_t write_through      : 1;
        uint32_t cache_disabled     : 1;
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t attribute_index    : 1;
        uint32_t global             : 1;
        uint32_t available          : 3;
        uint32_t page_address       : 20;
    } __attribute__ ((packed));
} PTE_t;

PDE_t page_directory[PDE_SIZE] __attribute__((aligned (4096)));
PTE_t page_table[PTE_SIZE] __attribute__((aligned (4096)));
PTE_t page_table_vidmap[PTE_SIZE] __attribute__((aligned (4096)));

extern void init_paging();
void set_pde_kb(int index, int present);
void set_pde_vidmap_kb(int index, int present);
void set_pde_mb(int index, int present);
void set_pde_mb_unused(int index, int present);
void set_pte_video_mem(int index, int present);
void set_pte(int index, int present);

#endif
