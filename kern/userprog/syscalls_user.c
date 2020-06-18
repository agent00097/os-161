#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <kern/unistd.h>
#include <syscall.h>
#include <thread.h>
#include <synch.h>
#include <uio.h>
#include <vfs.h>
#include <vnode.h>
#include <curthread.h>
#include <kern/limits.h>
#include <machine/vm.h>
#include <vm.h>
#include <kern/stat.h>
#include <fs.h>

#include "addrspace.h"

void child_forkentry(struct trapframe *tf, struct addrspace *addr);

#define MAX_NUM_ARGS 15


int sys_getpid(pid_t *anything) {
    *anything = curthread->pid;
    return 0;
}

//Fork function
pid_t sys_fork(struct trapframe* tf, pid_t* ret) {

	//Copying parent's trap frame
	struct trapframe *child_tf = kmalloc(sizeof(struct trapframe));
	memcpy(child_tf, tf, sizeof(struct trapframe));
    
    struct addrspace *new_address_space = NULL;

    /*if(child_tf == NULL) {
        kfree(child_tf);
        return ENOMEM;
    }*/

	//Copying address space to the child
    as_copy(curthread->t_vmspace, &(new_address_space));

    
	struct thread *child_thread = NULL;
    int error = 0;
    error = thread_fork(curthread->t_name, (void*) child_tf, (unsigned long) new_address_space, child_forkentry, &child_thread);

    *ret = child_thread->pid;

	struct process *child_process = get_process(child_thread->pid);
	child_process->parent_pid = curthread->pid;

    //Copying the adress space of the parent process

    //Getting parent's PID

    return error;

}


void
child_forkentry(struct trapframe *child_tf, struct addrspace *addr_space)
{

	struct trapframe my_tf;
	struct addrspace *my_addrspace = (struct addrspace*)addr_space;


	memcpy(&my_tf, child_tf, sizeof(struct trapframe));
	kfree((struct trapframe*)child_tf);

	assert(curthread->t_vmspace == NULL);
	curthread->t_vmspace = my_addrspace;
	as_activate(curthread->t_vmspace);

	my_tf.tf_v0 = 0;
	my_tf.tf_a3 = 0;
	my_tf.tf_epc += 4;

	mips_usermode(&my_tf);
}

int
sys_read(int filehandle, char *buf, size_t size)
{
	(void) filehandle;
	(void) size;
	
	char ch = getch();	
	if(ch == '\r') {
		ch = '\n';
	}
	buf[0] = ch;
	return 1;
}

/*
 *  Implementation of system call write(int, char *, int).
 *  This is not how a system call should be implemented. 
 *  But, for now, it works.
 */
 
int
sys_write(int filehandle, char *buf, size_t size)
{
	(void) filehandle;
	
	size_t i;	
	for (i = 0; i < size; ++i) {
		putch((int)buf[i]);
	}	
	return i;
}

int sys_execv(char *program, char **args) {

	int r = 0;
    size_t len = 0;
    int args_counter = 0;


    // Variable to store program name and copying it
    char *name_copy = kmalloc(sizeof(program));
    if(name_copy == NULL){
        return ENOMEM;
    }

    r = copyinstr(program, name_copy, strlen(program)+1, &len);
    if(r){
        kfree(name_copy);
        name_copy = NULL;
        return r;
    }


    // Counting number of arguments
    int j = 0;
    while(args[j] != NULL){
        j++;
    }
    args_counter = j;

    // MAX_NUM_ARGS checking
    if(args_counter > MAX_NUM_ARGS){
        kfree(name_copy);
        name_copy = NULL;
        return E2BIG;
    }

    // new double pointer initialization to store all the arguments
    char **kargv = kmalloc(sizeof(char*) * (args_counter+1));
    if(kargv == NULL){
        return ENOMEM;
    }

    int i = 0;
    for(i = 0; i < args_counter; ++i){
        kargv[i] = kmalloc(sizeof(char) * 20);
        if(kargv[i] == NULL){
            return ENOMEM;
        }
    }

    // copying all arguments into kernel space
    for(i = 0; i < args_counter; ++i){
        r = copyin(args[i], kargv[i], strlen(args[i]));
        kargv[strlen(kargv[i])] = '\0';
        if(r){
            name_copy = NULL;
            return r;  
        }
    }
    kargv[i] = NULL;


    // destroy old address space and create new one 
    as_destroy(curthread->t_vmspace);
    curthread->t_vmspace = as_create();
    if(curthread->t_vmspace == NULL){
        name_copy = NULL;
        return ENOMEM;
    }

    // Opening the file
    struct vnode *temp_vnode;
    r = vfs_open(name_copy, O_RDONLY, &temp_vnode);
    if (r) {
        as_destroy(curthread->t_vmspace);
        name_copy = NULL;
        return r;
    }

    // activating the new address space
    as_activate(curthread->t_vmspace);

    // load the executable
    vaddr_t entrypoint, stack_ptr;
    r = load_elf(temp_vnode, &entrypoint);
    if(r){
        as_destroy(curthread->t_vmspace);
        name_copy = NULL;
        vfs_close(temp_vnode);
        return r;
    }

    vfs_close(temp_vnode);

    // Defining user stack in address space
    r = as_define_stack(curthread->t_vmspace, &stack_ptr);
    if(r){
        return r;
    }

    //MAIN PART : copy arguments from kernel to user stack
    i = 0;
    char **user_space_ar = kmalloc(sizeof(char*) * (args_counter+1));
    for(i = j - 1; i >=0 ; --i){
        size_t size = strlen(kargv[i]) + 1;
        stack_ptr -= size;
        user_space_ar[i] = stack_ptr;
        bzero(stack_ptr, size);
        r = copyoutstr(kargv[i], stack_ptr, size, NULL);
        if(r){
            as_destroy(curthread->t_vmspace);
            name_copy = NULL;
            return r;
        }

    }

    user_space_ar[strlen(user_space_ar)] = NULL;

    // subtract number of the arguments from the stack pointer to store their addresses and make it divisible by four
    size_t arg_sizes = (strlen(user_space_ar) + 1) * sizeof(char*);
    stack_ptr -= arg_sizes;
    stack_ptr -= (stack_ptr % 4);
    bzero(stack_ptr, arg_sizes);
    r = copyout(user_space_ar, stack_ptr, arg_sizes);
    if(r){
        as_destroy(curthread->t_vmspace);
        name_copy = NULL;
        return r;
    }

    int argc = args_counter;

    md_usermode(argc, (userptr_t) stack_ptr, stack_ptr, entrypoint);

    panic("Shouldn't be here in EXECV!!\n");
    return EINVAL;

	// for(i = args_counter-1; i >= 0; i--) {
	// 	int packed_length = (strlen(kargv[i])) % 4;
	// 	if(packed_length > 0) {
	// 		packed_length = ((int)((strlen(kargv[i]))/4) + 1) * 4;
	// 	}
	// 	else if(packed_length == 0) {
	// 		packed_length = (strlen(kargv[i]));
	// 	}
	// 	stack_ptr = stack_ptr - (packed_length);
	// 	copyoutstr(kargv[i], (userptr_t)stack_ptr, strlen(kargv[i]), &arglen);
	// 	parameter_stack[i] = stack_ptr;
	// }

	// parameter_stack[args_counter] = (int)NULL;
	// for(i = args_counter-1; i >= 0; i--) {
	// 	stack_ptr = stack_ptr - 4;
	// 	copyout(&parameter_stack[i], (userptr_t)stack_ptr, sizeof(parameter_stack[i]));
	// }
	// kprintf("Checkpoint 5\n");
	// kfree(kargv);
	// md_usermode(args_counter, (userptr_t)stack_ptr, stack_ptr, entrypoint);
}

pid_t sys_waitpid(pid_t pid, userptr_t status, int options, int *retval) {
//Getting the index of the process table

	int j, status_two;

	if(options != 0) {
		*retval = -1;
		return EINVAL;
	}

	//Checking for the invalid pointer
	if(status == NULL) {
		*retval = -1;
		return EFAULT;
	}

	//check if the pid is the child of the current thread
	struct thread *child = NULL;
	struct process *here = get_process(pid);
	if(here->parent_pid == curthread->pid) {
		child = here->the_thread;
	}

	//Making sure that the child was found
	if(child == NULL) {
		*retval = -1;
		return EINVAL;
	}

	//Copying exit code into status
	j = copyout(&here->exit_code, status, sizeof(int));
	if(j) {
		*retval = -1;
		return j;
	}

	P(here->sem_th);
	*retval = pid;

	return 0;


}

int sys__exit(int code) {
    pid_t pid = curthread->pid; 
    struct process* p = get_process(pid);
    if(p != NULL)          
    {        
        //acquire the exit lock to see whether the child has exited or not
        // lock_acquire(p->process_lock);                        
        p->exit_status = 1;                                
        //(void)code;
        p->exit_code = code;        
        //sleep on the exitcode variable to be updated by the exiting child
        V(p->sem_th);                       
        // lock_release(p->process_lock);
    }
    
    //kprintf("\n Thread exitted %d\n", curthread->pid);
    thread_exit();

    panic("I shouldn't be here in sys__exit!");

    return 0;
}

