#include "os.h"
#include <stdio.h>

uint64_t search(uint64_t pt, uint64_t vpn);
void get_keys(uint64_t vpn, uint64_t* keys);

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    uint64_t keys[5];
    get_keys(vpn, keys);

    uint64_t* curr_lvl_virt;
    uint64_t* next_lvl_virt = phys_to_virt(pt << 12);
    uint64_t next_lvl_phys;
    for (int i = 0; i < 4; i++) {
        curr_lvl_virt = next_lvl_virt;
        next_lvl_phys = curr_lvl_virt[keys[i]];
        if ((next_lvl_phys & 1) == 0) { /* if not valid */
            if (ppn == NO_MAPPING) { /* if need to destroy */
                return;
            } else { /* need to add lvl */
                uint64_t new_lvl = (alloc_page_frame() << 12) | 1;
                curr_lvl_virt[keys[i]] = new_lvl;
            }
        }
        next_lvl_phys = curr_lvl_virt[keys[i]];
        next_lvl_virt = phys_to_virt(next_lvl_phys & (~0xfff));
    }

    uint64_t* last_lvl_virt = next_lvl_virt;
    if (ppn != NO_MAPPING) {
        last_lvl_virt[keys[4]] = (ppn << 12) | 1;
    } else {
        last_lvl_virt[keys[4]] = 0;
    }
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    uint64_t keys[5];
    get_keys(vpn, keys);

    uint64_t next_lvl_phys;
    uint64_t* next_lvl_virt = (uint64_t*) phys_to_virt(pt << 12);
    for (int i = 0; i < 5; i++) {
        next_lvl_phys = next_lvl_virt[keys[i]];
        if ((next_lvl_phys & 1) == 0) {
            return NO_MAPPING;
        }
        if (i != 4) {
            next_lvl_virt = phys_to_virt(next_lvl_phys & (~(0xfff)));
        }
    }

    return next_lvl_virt[keys[4]] >> 12;
}

void get_keys(uint64_t vpn, uint64_t* keys)
{
    for (int i = 0; i < 5; i++) {
        keys[4-i] = vpn & 0x1ff;
        vpn = vpn >> 9;
    }
}