#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_execv(char *program, char **args);
int sys_reboot(int code);
int sys_getpid(pid_t *retval);
pid_t sys_fork(struct trapframe* tf, pid_t* ret);
pid_t sys_waitpid(pid_t pid, userptr_t status, int options, int *retval);
sys_write(int filehandle, char *buf, size_t size);
sys_read(int filehandle, char *buf, size_t size);
int sys__exit(int code);
#endif /* _SYSCALL_H_ */
