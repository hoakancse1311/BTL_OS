/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * System Library
 * Memory Module Library libmem.c 
 */


#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt->rg_next = rg_node;

  /* Enlist the new region */
  mm->mmap->vm_freerg_list = rg_elmt;

  return 0;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;

  /* TODO: commit the vmaid */
  // rgnode.vmaid
  

  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
 
    *alloc_addr = rgnode.rg_start;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/
 else{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if(cur_vma == NULL){
    return -1;
  }
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);

  cur_vma->vm_end += inc_sz;
  
  struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
  if (newrg == NULL)
    return -1;

  newrg->rg_start = cur_vma->vm_end - inc_sz;
  newrg->rg_end = cur_vma->vm_end; 

  enlist_vm_freerg_list(caller->mm, newrg);
  caller->mm->symrgtbl[rgid].rg_start = newrg->rg_start;
  caller->mm->symrgtbl[rgid].rg_end = newrg->rg_end;
  *alloc_addr = newrg->rg_start;

  pthread_mutex_unlock(&mmvm_lock);
  return 0;

 }

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
  //struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);


  //int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  //int inc_limit_ret;
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  int inc_limit_ret;
  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
  //int old_sbrk = cur_vma->sbrk;
  int old_sbrk = cur_vma->sbrk;


  /* TODO INCREASE THE LIMIT as inovking systemcall 
   * sys_memap with SYSMEM_INC_OP 
   */
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...
  struct sc_regs regs;
  
  regs.a1 = (uint32_t)caller;
  regs.a2 = (uint32_t)old_sbrk;
  regs.a3 = (uint32_t)inc_sz;

  /* SYSCALL 17 sys_memmap */
 if(syscall(caller, 17, &regs) != 0)
    return -1; // Error in increasing limit


  /* TODO: commit the limit increment */
  cur_vma->vm_end  += inc_sz;
  /* TODO: commit the allocation address 
  */

  *alloc_addr = cur_vma->vm_end - inc_sz;

  return 0;

}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  //struct vm_rg_struct rgnode;

  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;

  /* TODO: Manage the collect freed region to freerg_list */
  
  struct vm_rg_struct *rgnode = get_symrg_byid(caller->mm, rgid);
  if (rgnode == NULL)
    return -1;
  if (rgnode->rg_start >= rgnode->rg_end)
    return -1;
  // Tạo một bản sao của vùng nhớ để thêm vào danh sách vùng nhớ tự do
  struct vm_rg_struct *freerg = malloc(sizeof(struct vm_rg_struct));
  if (freerg == NULL)
        return -1; // Lỗi cấp phát bộ nhớ

  freerg->rg_start = rgnode->rg_start;
  freerg->rg_end = rgnode->rg_end;

  // Thêm vùng nhớ đã giải phóng vào danh sách vùng nhớ tự do
  if (enlist_vm_freerg_list(caller->mm, freerg) != 0)
    {
        free(freerg); // Giải phóng nếu không thể thêm vào danh sách
        return -1;
    }


  /*enlist the obsoleted memory region */
  //enlist_vm_freerg_list();
  rgnode->rg_start = -1;
  rgnode->rg_end = -1;

  return 0;
}

/*liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  /* TODO Implement allocation on vm area 0 */
  int addr;


  /* By default using vmaid = 0 */
  return __alloc(proc, 0, reg_index, size, &addr);
}

/*libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  /* TODO Implement free region */

  /* By default using vmaid = 0 */
  return __free(proc, 0, reg_index);
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
void pte_set_present(uint32_t *pte) {
  *pte |= (1U << 31); // Đặt bit 31 lên 1
}

int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
    uint32_t pte = mm->pgd[pgn];

    if (!PAGING_PAGE_PRESENT(pte))
    {
        int vicpgn, swpfpn;

        /* 1. Tìm nạn nhân để thay thế (victim page) */
        find_victim_page(caller->mm, &vicpgn);

        /* 2. Lấy frame trống trong SWAP */
        MEMPHY_get_freefp(caller->active_mswp, &swpfpn);

        /* 3. Hoán đổi: RAM --> SWAP */
        struct sc_regs regs;
        regs.a1 = vicpgn;       // trang nạn nhân trong RAM
        regs.a2 = swpfpn;       // frame trống trong SWAP
        regs.a3 = SYSMEM_SWP_OP;

        if (syscall(caller, 17, &regs) != 0) // Thêm caller vào syscall
            return -1; // lỗi khi ghi trang nạn nhân ra SWAP

        /* 4. Hoán đổi: SWAP --> RAM */
        regs.a1 = pgn;          // trang mục tiêu cần đưa vào RAM
        regs.a2 = vicpgn;       // frame trống trong RAM (do nạn nhân vừa bị đẩy ra)
        regs.a3 = SYSMEM_SWP_OP;

        if (syscall(caller, 17, &regs) != 0) // Thêm caller vào syscall
            return -1; // lỗi khi đọc trang mục tiêu vào RAM

        /* 5. Cập nhật bảng trang */
        pte_set_swap(&mm->pgd[vicpgn], swpfpn, caller);   // Thêm caller vào pte_set_swap
        pte_set_fpn(&mm->pgd[pgn], vicpgn);               // cập nhật physical frame cho page pgn
        pte_set_present(&mm->pgd[pgn]);                   // đánh dấu trang đã nạp

        /* 6. Đưa trang vào hàng FIFO (nếu có dùng) */
        enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
    }

    *fpn = PAGING_FPN(mm->pgd[pgn]);  // Lấy physical frame number cuối cùng
    return 0;
}


/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);       // Trang ảo
  int off = PAGING_OFFST(addr);     // Offset trong trang
  int fpn;                          // Frame vật lý

  /* 1. Đảm bảo trang hiện diện trong RAM */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; // Truy cập sai hoặc lỗi hoán đổi

  /* 2. Tính địa chỉ vật lý */
  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  /* 3. Gọi syscall 17 để đọc dữ liệu */
  struct sc_regs regs;
  regs.a1 = phyaddr;        // Địa chỉ vật lý cần đọc
  regs.a2 = (uint32_t)data; // Con trỏ nơi lưu dữ liệu đọc
  regs.a3 = SYSMEM_IO_READ; // Mã thao tác: đọc dữ liệu

  if (syscall(caller, 17, &regs) != 0)
    return -1; // Lỗi khi thực hiện syscall

  return 0; // Thành công
}


/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);       // Số trang ảo
  int off = PAGING_OFFST(addr);     // Offset trong trang
  int fpn;                          // Frame vật lý tương ứng

  /* 1. Đảm bảo trang được đưa vào RAM (paging in nếu cần) */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; // Lỗi không truy cập được trang

  /* 2. Tính địa chỉ vật lý từ fpn và offset */
  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  /* 3. Gọi syscall 17 để ghi dữ liệu */
  struct sc_regs regs;
  regs.a1 = phyaddr;         // Địa chỉ vật lý đích
  regs.a2 = (uint32_t)&value; // Con trỏ tới dữ liệu cần ghi
  regs.a3 = SYSMEM_IO_WRITE; // Mã thao tác: ghi vào bộ nhớ

  if (syscall(caller, 17, &regs) != 0)
    return -1; // Lỗi syscall khi ghi dữ liệu

  return 0; // Thành công
}


/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*libread - PAGING-based read a region memory */
int libread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t* destination)
{
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  /* TODO update result of reading action*/
  //destination 
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*libwrite - PAGING-based write a region memory */
int libwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return __write(proc, 0, destination, offset, data);
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}


/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  if (pg == NULL)
    return -1;  // Không có trang nào để thay thế

  *retpgn = pg->pgn;         // Lấy số trang nạn nhân từ đầu danh sách FIFO
  mm->fifo_pgn = pg->pg_next; // Loại bỏ trang đầu khỏi FIFO
  free(pg);                  // Giải phóng vùng nhớ

  return 0;
}


/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
    return -1;

  /* Probe uninitialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* Traverse list of free vm region to find a fit space */
  while (rgit != NULL)
  {
    int area_size = rgit->rg_end - rgit->rg_start + 1;
    if (area_size >= size)
    {
      // Copy thông tin vùng trống phù hợp vào newrg
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = newrg->rg_start + size - 1;

      // Cập nhật lại vùng trống hiện tại
      rgit->rg_start += size;
      if (rgit->rg_start > rgit->rg_end)
      {
        // Nếu không còn vùng trống sau khi cấp phát, loại bỏ rgit khỏi danh sách
        // (Tuỳ thuộc vào cách cài đặt danh sách liên kết, bạn có thể cần cập nhật prev->next)
        rgit->rg_start = rgit->rg_end = -1;
      }

      return 0; // Thành công
    }
    rgit = rgit->rg_next;
  }

  return -1; // Không tìm thấy vùng trống phù hợp
}

//#endif
