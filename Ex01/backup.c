#include "os.h"
#include <stdio.h>

uint64_t search(uint64_t pt, uint64_t vpn);
void get_keys(uint64_t vpn, uint64_t* keys);

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    uint64_t keys[5];
    get_keys(vpn, keys);

    uint64_t next_lvl_phys;
    uint64_t* next_lvl_virt = (uint64_t*) phys_to_virt(pt << 12);
    for (int i = 0; i < 4; i++) {
        next_lvl_phys = next_lvl_virt[keys[i]];
        if (((next_lvl_phys & 0b1)==0) && (ppn != NO_MAPPING)) {
            next_lvl_virt[keys[i]] = (alloc_page_frame() << 12) + 1;
        }
        if (!(next_lvl_phys & 0b1) && (ppn == NO_MAPPING)){
            return;
        }
        next_lvl_phys = next_lvl_virt[keys[i]] - 1;
    }
    next_lvl_virt = phys_to_virt(next_lvl_phys << 12);
    if (ppn == NO_MAPPING) {
        next_lvl_virt[keys[4]] = 1;
    } else {
        next_lvl_virt[keys[4]] = ppn;
    }
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    uint64_t keys[] = {0,0,0,0,0};
    get_keys(vpn, keys);

    uint64_t next_lvl_phys;
    uint64_t* next_lvl_virt = (uint64_t*) phys_to_virt(pt << 12);
    for (int i = 0; i < 4; i++) {
        next_lvl_phys = next_lvl_virt[keys[i]];
        if ((next_lvl_phys & 0b1) == 0) {
            return NO_MAPPING;
        }
        next_lvl_virt = phys_to_virt(next_lvl_phys << 12);
    }
    return next_lvl_virt[keys[4]] >> 12;
}

void get_keys(uint64_t vpn, uint64_t* keys)
{
    for (int i = 0; i < 5; i++) {
        keys[4-i] = vpn & 0x1ff;
        printf("iteration %d, vpn with mask: %llx\n",i, vpn & 0x1ff);
        vpn = vpn >> 9;
    }
}