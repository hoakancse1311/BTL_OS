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
 #include "queue.h" //Added for queue (dont know if allowed)
 #include <string.h> //Added for strcmp (maybe not needed)
 
 int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
 {
     char proc_name[100];
     uint32_t data;
 
     //hardcode for demo only
     uint32_t memrg = regs->a1;
     
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
      *       strcmp to check the process match proc_name
      */
     //caller->running_list
     //caller->mlq_ready_queu
    
     struct queue_t *run = caller->running_list;
     struct queue_t *ready = caller->mlq_ready_queue;
 
     /* TODO Maching and terminating 
      *       all processes with given
      *        name in var proc_name
      */
 
     int kill_count = 0;
 
     for(int i = 0; i < run->size; i++){
         if(strcmp(run->proc[i]->path, proc_name) == 0){
             // printf("Killing process %s\n", run->proc[i]->path);
             free(run->proc[i]);
             run->proc[i] = NULL;
             kill_count++;
         }
     }
 
     for(int i = 0; i < ready->size; i++){
         if(strcmp(ready->proc[i]->path, proc_name) == 0){
             // printf("Killing process %s\n", ready->proc[i]->path);
             free(ready->proc[i]);
             ready->proc[i] = NULL;
             kill_count++;
         }
     }
     // printf("Total killed process: %d\n", kill_count);
 
     return kill_count;
 }
 