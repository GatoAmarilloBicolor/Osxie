#include <duct/duct.h>

#include <duct/duct_pre_xnu.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <sys/proc_internal.h>
#include <sys/user.h>
#include <duct/duct_post_xnu.h>

thread_t osxie_thread_get_current(void);
task_t osxie_task_get_current(void);

uthread_t current_uthread(void) {
	thread_t thread = osxie_thread_get_current();
	return thread ? (uthread_t)thread->uthread : NULL;
};

proc_t current_proc(void) {
	task_t task = osxie_task_get_current();
	return task ? (proc_t)task->bsd_info : NULL;
};
