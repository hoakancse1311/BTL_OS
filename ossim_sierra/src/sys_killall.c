/*
* Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
*/

/* Sierra release
* Source Code License Grant: The authors hereby grant to Licensee
* personal permission to use and modify the Licensed Source Code
* for the sole purpose of studying while attending the course CO2018.
*/

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"

// Additional includes
#include "string.h"
#include "queue.h"
#include "sched.h"

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    //hardcode for demo only
    uint32_t memrg = regs->a1; // ID of mem region that store proc name need to kill
    
    /* TODO: Get name of the target proc */
    //proc_name = libread..
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
    
    /* TODO: Traverse proclist to terminate the proc
    *       stcmp to check the process match proc_name
    */
    struct pcb_t *kill_process[MAX_PRIO];
    int index = 0;

    if (caller->running_list != NULL) {
        struct queue_t *run_list = caller->running_list;
        for(int i = 0; i < run_list->size; i++) {
            struct pcb_t *proc = run_list->proc[i];
            if(proc != NULL && strcmp(proc->path, proc_name) == 0) {
                #ifdef MM_PAGING
                if (proc->mm != NULL) {
                    free_pcb_memph(proc);
                }
                #endif
                kill_process[index++] = run_list->proc[i];
                run_list->proc[i]->pc = run_list->proc[i]->code->size; // Set program counter = size to force the process to end
                for(int j = i; j < run_list->size - 1; j++) {
                    run_list->proc[j] = run_list->proc[j + 1];  // Remove process form queue by shifting array
                }
                run_list->size--;
                i--; // To check the newly shifted process, avoid skipping it
            }
        }
    }

    #ifdef MLQ_SCHED
    if (caller->mlq_ready_queue != NULL) {
        for(int prio = 0; prio < MAX_PRIO; prio++) {
            struct queue_t *queue = &caller->mlq_ready_queue[prio];
            for(int i = 0; i < queue->size; i++) {
                struct pcb_t *proc = queue->proc[i];
                if(proc != NULL && strcmp(proc->path, proc_name) == 0) {
                    #ifdef MM_PAGING
                    if (proc->mm != NULL) {
                        free_pcb_memph(proc);
                    }
                    #endif
                    kill_process[index++] = queue->proc[i];
                    queue->proc[i]->pc = queue->proc[i]->code->size; // Set program counter = size to force the process to end
                    for(int j = i; j < queue->size - 1; j++) {
                        queue->proc[j] = queue->proc[j + 1];  // Remove process form queue by shifting array
                    }
                    queue->size--;
                    i--; // To check the newly shifted process, avoid skipping it
                }
            }
        }
    }
    #else
    if (caller->ready_queue != NULL) {
        struct queue_t *ready_q = caller->ready_queue;
        for(int i = 0; i < ready_q->size; i++) {
            struct pcb_t *proc = ready_q->proc[i];
            if(proc != NULL && strcmp(proc->path, proc_name) == 0) {
                #ifdef MM_PAGING
                if (proc->mm != NULL) {
                    free_pcb_memph(proc);
                }
                #endif
                kill_process[index++] = ready_q->proc[i];
                ready_q->proc[i]->pc = ready_q->proc[i]->code->size; // Set program counter = size to force the process to end
                for(int j = i; j < ready_q->size - 1; j++) {
                    ready_q->proc[j] = ready_q->proc[j + 1];  // Remove process form queue by shifting array
                }
                ready_q->size--;
                i--; // To check the newly shifted process, avoid skipping it
            }
        }
    }
    #endif

    /* TODO Maching and terminating 
    *       all processes with given
    *        name in var proc_name
    */
    
    if(index > 0) {
        for(int i = 0; i < index; i++) {
            for(int j = 0; j < kill_process[i]->code->size; j++)
            if(kill_process[i]->code->text[j].opcode == ALLOC) {  // Free all allocated region of killed process
                __free(kill_process[i], kill_process[i]->mm->mmap->vm_id, kill_process[i]->code->text[j].arg_0);
            }
        }
    // } else {
    //     printf("Process with name %s does not exist\n",proc_name);
    }

    return 0; 
}