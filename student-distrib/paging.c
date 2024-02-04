#include "paging.h"
#include "types.h"

 /* init_paging
 *   DESCIRPTION: Initialize page table and page directory
 *   INPUT:  none
 *   OUTPUT: none
 */
void init_paging(){
    int index;

    for (index = 0; index < PDE_SIZE; index++){
        if(index == 0){
            set_pde_kb(index, 1);
        }

        else if(index == 1){
            set_pde_mb(index, 1);
        }

         else if(index == _132MB >> 22){
            set_pde_vidmap_kb(index, 1);
        }

        else{
            set_pde_mb_unused(index, 0);  //for others mark not present
        }
    }

    for(index = 0; index < PTE_SIZE; index++){
        set_pte_video_mem(index, 0);
        if(index == (VIDEO_ADDR >> 12)){
            set_pte(index, 1);   //set video memory page
        }

        else{
            set_pte(index, 0);  //mark not present for others
        }
    }

    //enable paging
    asm volatile(
    "movl %0, %%eax    ;"
    "movl %%eax, %%cr3 ;"  
 
    "movl %%cr4, %%eax ;"
    "orl $0x00000010, %%eax ;"
    "movl %%eax, %%cr4 ;"  

    "movl %%cr0, %%eax ;"
    "orl $0x80000001, %%eax ;"
    "movl %%eax, %%cr0 ;" 
    : 
    : "r"(page_directory)
    : "eax"  
    );
}

 /* set_pde_kb
 *   DESCIRPTION: set the first pde with 4kb pages
 *   INPUT: index: index in the PDE
 *          present: whther this entry is present in pythical memory
 *   OUTPUT: none
 */
void set_pde_kb(int index, int present){
    page_directory[index].KB.present = present;
    page_directory[index].KB.read_write = 1;
    page_directory[index].KB.user_supervisor = 0;
    page_directory[index].KB.write_through = 0;
    page_directory[index].KB.cache_disabled = 0;
    page_directory[index].KB.accessed = 0;
    page_directory[index].KB.reserved = 0;
    page_directory[index].KB.page_size = 0;
    page_directory[index].KB.global = 0;
    page_directory[index].KB.available = 0;
    page_directory[index].KB.table_address = ((uint32_t) page_table >> 12);  //get the address of 4kb page table
}

 /* set_pde_vidmap_kb
 *   DESCIRPTION: set the pde for video map
 *   INPUT: index: index in the PDE
 *          present: whther this entry is present in pythical memory
 *   OUTPUT: none
 */
void set_pde_vidmap_kb(int index, int present){
    page_directory[index].KB.present = present;
    page_directory[index].KB.read_write = 1;
    page_directory[index].KB.user_supervisor = 1;
    page_directory[index].KB.write_through = 0;
    page_directory[index].KB.cache_disabled = 0;
    page_directory[index].KB.accessed = 0;
    page_directory[index].KB.reserved = 0;
    page_directory[index].KB.page_size = 0;
    page_directory[index].KB.global = 0;
    page_directory[index].KB.available = 0;
    page_directory[index].KB.table_address = ((uint32_t) page_table_vidmap >> 12);  //get the address of 4kb page table
}

 /* set_pde_mb
 *   DESCIRPTION: set the first pde with 4mb pages
 *   INPUT: index: index in the PDE
 *          present: whther this entry is present in pythical memory
 *   OUTPUT: none
 */
void set_pde_mb(int index, int present){
    page_directory[index].MB.present = present;
    page_directory[index].MB.read_write = present;  //set read only accordingly for kernel code
    page_directory[index].MB.user_supervisor = 0;
    page_directory[index].MB.write_through = 0;
    page_directory[index].MB.cache_disabled = 0;
    page_directory[index].MB.accessed = 0;
    page_directory[index].MB.dirty = 0;
    page_directory[index].MB.page_size = 1; //set to 1 for 4mb aligned
    page_directory[index].MB.global = 1;    //for kernel code, keep tlb even after context switch
    page_directory[index].MB.available = 0;
    page_directory[index].MB.attribute_index = 0;
    page_directory[index].MB.reserved = 0;
    page_directory[index].MB.table_address = 1;
}

 /* set_pde_mb_unused
 *   DESCIRPTION: set the other pde with 4mb pages and mark not present
 *   INPUT: index: index in the PDE
 *          present: whther this entry is present in pythical memory
 *   OUTPUT: none
 */
void set_pde_mb_unused(int index, int present){
    page_directory[index].MB.present = 0; //presemt set to 0 for unused pages
    page_directory[index].MB.read_write = 0;
    page_directory[index].MB.user_supervisor = 0;
    page_directory[index].MB.write_through = 0;
    page_directory[index].MB.cache_disabled = 0;
    page_directory[index].MB.accessed = 0;
    page_directory[index].MB.dirty = 0;
    page_directory[index].MB.page_size = 1; //set to 1 for 4mb aligned
    page_directory[index].MB.global = 0;
    page_directory[index].MB.available = 0;
    page_directory[index].MB.attribute_index = 0;
    page_directory[index].MB.reserved = 0;
    page_directory[index].MB.table_address = index;
}

 /* set_pte
 *   DESCIRPTION: set the 4kb page table entry
 *   INPUT: index: index in the PTE
 *          present: whther this entry is present in pythical memory
 *   OUTPUT: none
 */
void set_pte(int index, int present){
    page_table[index].present = present;
    page_table[index].read_write = 1;
    page_table[index].user_supervisor = 0;
    page_table[index].write_through = 0;
    page_table[index].cache_disabled = 0;
    page_table[index].accessed = 0;
    page_table[index].dirty = 0;
    page_table[index].attribute_index = 0;
    page_table[index].global = 0;
    page_table[index].available = 0;
    page_table[index].page_address = index;
}

 /* set_pte_video_mem
 *   DESCIRPTION: set the 4kb page table entry for video memory
 *   INPUT: index: index in the PTE
 *          present: whther this entry is present in pythical memory 
 *   OUTPUT: none
 */
void set_pte_video_mem(int index, int present){
    page_table_vidmap[index].present = present;
    page_table_vidmap[index].read_write = 1;
    page_table_vidmap[index].user_supervisor = 1;
    page_table_vidmap[index].write_through = 0;
    page_table_vidmap[index].cache_disabled = 0;
    page_table_vidmap[index].accessed = 0;
    page_table_vidmap[index].dirty = 0;
    page_table_vidmap[index].attribute_index = 0;
    page_table_vidmap[index].global = present;
    page_table_vidmap[index].available = 0;
    page_table_vidmap[index].page_address = page_table[index].page_address;
}
