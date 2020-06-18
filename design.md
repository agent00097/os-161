
# Rostawal-Pfonash-Asst2 Design Document

## Code Walkthrough Answers

1. What are the ELF magic numbers?

The ELF magic numbers are 0x7f, 'E', 'L', and 'F'. The ELF magic numbers are used in the ELF file header. The magic numbers go from index EI_MAG0 to 3 in e_ident[]. These magic numbers identify a file as an ELF file.

2. What is the difference between UIO_USERISPACE and UIO_USERSPACE? When should one use UIO_SYSSPACE instead?

Based on line 63 from loadelf.c,

`u.uio_segflg = is_executable ? UIO_USERISPACE : UIO_USERSPACE`

A UIO_USERISPACE segment is executable and a UIO_USERSPACE segment is not. Therefore, UIO_USERISPACE is user process code and UIO_USERSPACE is user process data.

We know that from the comment in uio.h that uio is based on BSD uio. Per BSD documentation, UIO_SYSSPACE points to kernel address space. (http://nixdoc.net/man-pages/FreeBSD/man9/uio.9.html) Given that, it seems we'd want to use UIO_SYSSPACE when we're handling kernel data. We see in uio.h, on line 22, that UIO_SYSSPACE is indeed kernel address space.

3. Why can the struct uio that is used to read in a segment be allocated on the stack in load_segment() (i.e., where does the memory read actually go)?

Per uio.h, the data is read into "a data region defined by a uio struct." We see in load_segment(), that we load the data from a virtual address into a iovec struct that is a member of a uio struct. Specifically, we set the user base of the uio struct to the virtual address. Thus, the memory read goes somewhere in a user address space.

4. In runprogram(), why is it important to call vfs_close() before going to usermode?

Because, according to kern/include/vnode.h, we should use vfs_close() to close vnodes opened with vfs_open(). The user program won't have access to the file's location, and so it won't be able to close it. This would result in false reference numbers for the file, which vfs_open() and vfs_close() increment and decrement, respectively.

5. What function forces the processor to switch into usermode? Is this function machine dependent?

md_usermode calls mips_usermode which calls asm_usernmode, which appears to actually force the processor to switch into usermode.

Yes, the function is machine dependent because asm_usermode is in kern/arch/mips/mips/exception.S, which are the source files containing the machine-dependent code that the kernel needs to run.

It also appears the md_ prefix before a function means "machine dependent.""

6. In what file are copyin and copyout defined? memmove? Why can’t copyin and copyout be implemented as simply as memmove?

copyin and copyout are defined in copyinout.c. memmove is defined in memmove.c.

copyin copies bytes from a user-space address to a
kernel-space address. copyout conversely copies bytes from a kernel-space address to a user-space address. For that reason, copyin and copyout need to make sure a user process is not passing a kernel address. copyin and copyout ensure this via copycheck().

7. What (briefly) is the purpose of userptr_t?

Defines that a pointer belongs to a userspace. For example,

 `copyin(const_userptr_t usersrc, void *dest, size_t len)`

 establishes that the address we're copying in must come from the userspace.

8. What is the numerical value of the exception code for a MIPS system call?

The numerical value is 8.

9. Why does mips_trap() set curspl to SPL_HIGH “manually”, instead of using splhigh()?

Because interrupts should already be off. Therefore, we only need to updated the global value of curspl, which the line `curspl = SPL_HIGH;` does.

10. How many bytes is an instruction in MIPS? (Answer this by reading mips_syscall() carefully, not by looking somewhere else.)

The comments state that this line,

`tf->tf_epc += 4;` is increasing the program counter. Because we're increasing it by 4, that means an instruction is 4 bytes.

11. Why do you “probably want to change” the implementation of kill_curthread()?

Because kill_curthread(), as given, doesn't actually kill the current thread. Instead, we just panic. That's not the behavior we want. We should instead deallocate the thread's resources.

12. What would be required to implement a system call that took more than 4 arguments?

In exception.S, we see that comments that state that the assembly code `addi sp, sp, -164` "[a]llocate[s] stack space for 37 words to hold the trap frame **plus four more words for a minimal argument block.**" To implement a system call that took more than 4 arguments, then, we would need to allocate stack space for a word for each additional argument we wanted to pass in.

We'd also have to ensure that the order we allocated matched trapframe.h. and vice versa.

13. What is the purpose of the SYSCALL macro?

We know that callno-parse.sh parses callno.h to generate assembly language system call stubs. syscalls-mips.S in turn uses the SYSCALL MACRO as a signal to turn each SYSCALL(syscallname, syscallnumber) line in syscalls-mips.S into SYS_syscallname and loads the syscallnumber into the v0 register.  

14. What is the MIPS instruction that actually triggers a system call? (Answer this by reading the source in this directory, not looking somewhere else.)

syscall.

## Approach

**System Calls.**

**Scheduler.** we are using a simple 2-level feedback queue scheduler. The 2 queues are called runqueue 1 (higher priority) and rq2 (runqueue2, which is lower priority). The runqueue queue has an adjustable quantum and the qunatum of rq2 is twice the quantum of runqueue queue. Both queues use Round Robin scheduling.


## Implementation Description

**System Calls.**

**Scheduler.**




## Pros and Cons of Approach

We considered that hardclock.c may be edited to change the quantum time, defined by HZ macro.


## Alternatives Considered
