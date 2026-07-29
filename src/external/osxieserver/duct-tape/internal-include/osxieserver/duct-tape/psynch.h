#ifndef _OSXIESERVER_DUCT_TAPE_PSYNCH_H_
#define _OSXIESERVER_DUCT_TAPE_PSYNCH_H_

#include <osxieserver/duct-tape/task.h>
#include <osxieserver/duct-tape/thread.h>

void dtape_psynch_init(void);
void dtape_psynch_task_init(dtape_task_t* task);
void dtape_psynch_task_destroy(dtape_task_t* task);
void dtape_psynch_thread_init(dtape_thread_t* thread);
void dtape_psynch_thread_destroy(dtape_thread_t* thread);

#endif // _OSXIESERVER_DUCT_TAPE_PSYNCH_H_
