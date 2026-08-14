#include <osxieserver/duct-tape/stubs.h>
#include <osxieserver/duct-tape/hooks.internal.h>
#include <osxieserver/duct-tape/task.h>

#include <kern/processor.h>
#include <kern/kalloc.h>
#include <kern/host.h>
#include <kern/machine.h>
#include <kern/task.h>

// Manual libc declarations: the XNU-style build cannot include glibc headers
// (the -D_CLOCK_T/-D_NLINK_T/-Dvolatile=... defines clash with bits/types.h).
typedef struct _IO_FILE FILE;
FILE *fopen(const char *path, const char *mode);
char *fgets(char *s, int size, FILE *stream);
int fclose(FILE *stream);
int sscanf(const char *str, const char *format, ...);
int strncmp(const char *s1, const char *s2, size_t n);

processor_t processor_array[MAX_SCHED_CPUS] = {0};
struct processor_set pset0;

// don't use these in our code; this is only for XNU code
// use get_nprocs() instead.
uint32_t processor_avail_count;
uint32_t processor_avail_count_user;

// ditto
uint32_t primary_processor_avail_count;
uint32_t primary_processor_avail_count_user;

unsigned int processor_count;

simple_lock_data_t processor_list_lock;
processor_t master_processor;

// Linux functions
int get_nprocs(void);
int get_nprocs_conf(void);

int dtape_read_proc_stat_ticks(int cpu_id, uint64_t ticks[4]) {
	FILE* fp = fopen("/proc/stat", "r");
	if (!fp) {
		return -1;
	}

	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		unsigned long long user, nice, system, idle, iowait;

		if (strncmp(line, "cpu", 3) != 0) {
			// the per-CPU lines are contiguous; anything else means we're done
			break;
		}

		if (cpu_id < 0) {
			// aggregate line is "cpu  user nice system idle ..." (a space follows "cpu")
			if (line[3] != ' ') {
				continue;
			}
			if (sscanf(line, "cpu %llu %llu %llu %llu %llu",
			    &user, &nice, &system, &idle, &iowait) != 5) {
				continue;
			}
		} else {
			unsigned int cid;
			if (line[3] < '0' || line[3] > '9') {
				continue;
			}
			if (sscanf(line, "cpu%u %llu %llu %llu %llu %llu",
			    &cid, &user, &nice, &system, &idle, &iowait) != 6) {
				continue;
			}
			if ((int)cid != cpu_id) {
				continue;
			}
		}

		ticks[CPU_STATE_USER] = (uint64_t)user;
		ticks[CPU_STATE_NICE] = (uint64_t)nice;
		ticks[CPU_STATE_SYSTEM] = (uint64_t)system;
		ticks[CPU_STATE_IDLE] = (uint64_t)(idle + iowait);
		fclose(fp);
		return 0;
	}

	fclose(fp);
	return -1;
}

void dtape_processor_init(void) {
	simple_lock_init(&processor_list_lock, 0);

	processor_count = get_nprocs_conf();
	if (processor_count > MAX_SCHED_CPUS)
		processor_count = MAX_SCHED_CPUS;

	processor_avail_count = get_nprocs();
	if (processor_avail_count > MAX_SCHED_CPUS)
		processor_avail_count = MAX_SCHED_CPUS;

	processor_avail_count_user = processor_avail_count;
	primary_processor_avail_count = processor_avail_count;
	primary_processor_avail_count_user = processor_avail_count;

	for (size_t i = 0; i < processor_count; ++i) {
		processor_t processor = NULL;

		processor = kalloc(sizeof(*processor));
		processor_array[i] = processor;

		memset(processor, 0, sizeof(*processor));

		processor->processor_set = &pset0;
		processor->cpu_id = i;
	}

	pset0.online_processor_count = processor_avail_count;

	master_processor = processor_array[0];

	// TODO: there's probably more stuff we should set in these structures
};

kern_return_t processor_assign(processor_t processor, processor_set_t new_pset, boolean_t wait) {
	dtape_stub_safe();
	return KERN_FAILURE;
};

kern_return_t processor_control(processor_t processor, processor_info_t info, mach_msg_type_number_t count) {
	dtape_stub_unsafe();
};

kern_return_t processor_exit_from_user(processor_t processor) {
	dtape_stub_unsafe();
};

kern_return_t processor_get_assignment(processor_t processor, processor_set_t* pset) {
	dtape_stub_unsafe();
};

kern_return_t processor_info(processor_t processor, processor_flavor_t flavor, host_t* host, processor_info_t raw_info, mach_msg_type_number_t* count) {
	if (processor == PROCESSOR_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	switch (flavor) {
		case PROCESSOR_BASIC_INFO: {
			processor_basic_info_t info = (void*)raw_info;

			if (*count < PROCESSOR_BASIC_INFO_COUNT) {
				return KERN_FAILURE;
			}

#if __x86_64__ || __i386__
			info->cpu_type = CPU_TYPE_X86;
#if __x86_64__
			info->cpu_subtype = CPU_SUBTYPE_X86_64_ALL;
#else
			info->cpu_subtype = CPU_SUBTYPE_I386_ALL;
#endif
#else
			#error Unknown CPU type
#endif

			info->is_master = processor == master_processor;
			info->running = TRUE;
			info->slot_num = processor->cpu_id;

			*count = PROCESSOR_BASIC_INFO_COUNT;
			*host = &realhost;

			return KERN_SUCCESS;
		};

		case PROCESSOR_CPU_LOAD_INFO: {
			processor_cpu_load_info_t info = (void*)raw_info;
			uint64_t ticks[4];

			if (*count < PROCESSOR_CPU_LOAD_INFO_COUNT) {
				return KERN_FAILURE;
			}

			// read the real cumulative per-CPU ticks from /proc/stat
			if (dtape_read_proc_stat_ticks(processor->cpu_id, ticks) != 0) {
				// fall back to an all-idle reading rather than inventing load
				ticks[CPU_STATE_USER] = 0;
				ticks[CPU_STATE_NICE] = 0;
				ticks[CPU_STATE_SYSTEM] = 0;
				ticks[CPU_STATE_IDLE] = 1;
			}

			info->cpu_ticks[CPU_STATE_USER] = (integer_t)ticks[CPU_STATE_USER];
			info->cpu_ticks[CPU_STATE_SYSTEM] = (integer_t)ticks[CPU_STATE_SYSTEM];
			info->cpu_ticks[CPU_STATE_IDLE] = (integer_t)ticks[CPU_STATE_IDLE];
			info->cpu_ticks[CPU_STATE_NICE] = (integer_t)ticks[CPU_STATE_NICE];

			*count = PROCESSOR_CPU_LOAD_INFO_COUNT;
			*host = &realhost;

			return KERN_SUCCESS;
		};

		default:
			return KERN_FAILURE;
	}
};

kern_return_t processor_set_create(host_t host, processor_set_t* new_set, processor_set_t* new_name) {
	dtape_stub_unsafe();
};

kern_return_t processor_set_destroy(processor_set_t pset) {
	dtape_stub_unsafe();
};

kern_return_t processor_set_max_priority(processor_set_t pset, int max_priority, boolean_t change_threads) {
	dtape_stub_unsafe();
};

kern_return_t processor_set_policy_control(processor_set_t pset, int flavor, processor_set_info_t policy_info, mach_msg_type_number_t count, boolean_t change) {
	dtape_stub_unsafe();
};

kern_return_t processor_set_policy_disable(processor_set_t pset, int policy, boolean_t change_threads) {
	dtape_stub_unsafe();
};

kern_return_t processor_set_policy_enable(processor_set_t pset, int policy) {
	dtape_stub_unsafe();
};

kern_return_t processor_set_stack_usage(processor_set_t pset, unsigned int* totalp, vm_size_t* spacep, vm_size_t* residentp, vm_size_t* maxusagep, vm_offset_t* maxstackp) {
	dtape_stub_unsafe();
};

kern_return_t processor_set_statistics(processor_set_t pset, int flavor, processor_set_info_t raw_info, mach_msg_type_number_t* count) {
	if (pset == PROCESSOR_SET_NULL || pset != &pset0) {
		return KERN_INVALID_PROCESSOR_SET;
	}

	switch (flavor) {
		case PROCESSOR_SET_LOAD_INFO: {
			processor_set_load_info_t info = (void*)raw_info;
			dtape_load_info_t load_info;

			if (*count < PROCESSOR_SET_LOAD_INFO_COUNT) {
				return KERN_FAILURE;
			}

			info->mach_factor = 0;
			info->load_average = 0;

			dtape_hooks->get_load_info(&load_info);
			info->task_count = load_info.task_count;
			info->thread_count = load_info.thread_count;

			*count = PROCESSOR_SET_LOAD_INFO_COUNT;

			return KERN_SUCCESS;
		};

		default:
			return KERN_INVALID_ARGUMENT;
	}
};

struct dtape_pset_task_collector {
	ipc_port_t* ports;
	unsigned int count;
	unsigned int max;
};

static void dtape_pset_task_count_cb(dtape_task_t* task, void* user_data) {
	unsigned int* total = (unsigned int*)user_data;
	(void)task;
	(*total)++;
};

static void dtape_pset_task_collect_cb(dtape_task_t* task, void* user_data) {
	struct dtape_pset_task_collector* c = (struct dtape_pset_task_collector*)user_data;
	if (c->count >= c->max) {
		return;
	}
	if (task->xnu_task.ipc_active) {
		// the convert functions consume a task reference
		task_reference(&task->xnu_task);
		c->ports[c->count] = convert_task_to_port(&task->xnu_task);
		c->count++;
	}
};

kern_return_t processor_set_tasks_with_flavor(processor_set_t pset, mach_task_flavor_t flavor, task_array_t* task_list, mach_msg_type_number_t* count) {
	if (pset == PROCESSOR_SET_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	if (dtape_hooks == NULL || dtape_hooks->task_for_each == NULL) {
		return KERN_FAILURE;
	}

	unsigned int total = 0;
	dtape_hooks->task_for_each(dtape_pset_task_count_cb, &total);

	if (total == 0) {
		*task_list = NULL;
		*count = 0;
		return KERN_SUCCESS;
	}

	ipc_port_t* ports = (ipc_port_t*)kalloc((vm_size_t)(total * sizeof(ipc_port_t)));
	if (ports == NULL) {
		return KERN_RESOURCE_SHORTAGE;
	}

	struct dtape_pset_task_collector c = { ports, 0, total };
	dtape_hooks->task_for_each(dtape_pset_task_collect_cb, &c);

	// TASK_FLAVOR_READ is what libtop uses; keep the ports we built (they are
	// full task control ports, usable as read ports for all practical purposes).
	(void)flavor;

	*task_list = (task_array_t)ports;
	*count = c.count;

	return KERN_SUCCESS;
};

kern_return_t processor_set_tasks(processor_set_t pset, task_array_t* task_list, mach_msg_type_number_t* count) {
	return processor_set_tasks_with_flavor(pset, TASK_FLAVOR_CONTROL, task_list, count);
};

kern_return_t processor_set_threads(processor_set_t pset, thread_array_t* thread_list, mach_msg_type_number_t* count) {
	dtape_stub_unsafe();
};

kern_return_t processor_start_from_user(processor_t processor) {
	dtape_stub_unsafe();
};

// <copied from="xnu://7195.141.2/osfmk/kern/processor.c" modified>

kern_return_t
processor_set_info(
	processor_set_t         pset,
	int                     flavor,
	host_t                  *host,
	processor_set_info_t    info,
	mach_msg_type_number_t  *count)
{
	if (pset == PROCESSOR_SET_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	if (flavor == PROCESSOR_SET_BASIC_INFO) {
		processor_set_basic_info_t      basic_info;

		if (*count < PROCESSOR_SET_BASIC_INFO_COUNT) {
			return KERN_FAILURE;
		}

		basic_info = (processor_set_basic_info_t) info;
#ifdef __OSXIE__
		basic_info->processor_count = get_nprocs();
#else
#if defined(__x86_64__)
		basic_info->processor_count = processor_avail_count_user;
#else
		basic_info->processor_count = processor_avail_count;
#endif
#endif
		basic_info->default_policy = POLICY_TIMESHARE;

		*count = PROCESSOR_SET_BASIC_INFO_COUNT;
		*host = &realhost;
		return KERN_SUCCESS;
	} else if (flavor == PROCESSOR_SET_TIMESHARE_DEFAULT) {
		policy_timeshare_base_t ts_base;

		if (*count < POLICY_TIMESHARE_BASE_COUNT) {
			return KERN_FAILURE;
		}

		ts_base = (policy_timeshare_base_t) info;
		ts_base->base_priority = BASEPRI_DEFAULT;

		*count = POLICY_TIMESHARE_BASE_COUNT;
		*host = &realhost;
		return KERN_SUCCESS;
	} else if (flavor == PROCESSOR_SET_FIFO_DEFAULT) {
		policy_fifo_base_t              fifo_base;

		if (*count < POLICY_FIFO_BASE_COUNT) {
			return KERN_FAILURE;
		}

		fifo_base = (policy_fifo_base_t) info;
		fifo_base->base_priority = BASEPRI_DEFAULT;

		*count = POLICY_FIFO_BASE_COUNT;
		*host = &realhost;
		return KERN_SUCCESS;
	} else if (flavor == PROCESSOR_SET_RR_DEFAULT) {
		policy_rr_base_t                rr_base;

		if (*count < POLICY_RR_BASE_COUNT) {
			return KERN_FAILURE;
		}

		rr_base = (policy_rr_base_t) info;
		rr_base->base_priority = BASEPRI_DEFAULT;
		rr_base->quantum = 1;

		*count = POLICY_RR_BASE_COUNT;
		*host = &realhost;
		return KERN_SUCCESS;
	} else if (flavor == PROCESSOR_SET_TIMESHARE_LIMITS) {
		policy_timeshare_limit_t        ts_limit;

		if (*count < POLICY_TIMESHARE_LIMIT_COUNT) {
			return KERN_FAILURE;
		}

		ts_limit = (policy_timeshare_limit_t) info;
		ts_limit->max_priority = MAXPRI_KERNEL;

		*count = POLICY_TIMESHARE_LIMIT_COUNT;
		*host = &realhost;
		return KERN_SUCCESS;
	} else if (flavor == PROCESSOR_SET_FIFO_LIMITS) {
		policy_fifo_limit_t             fifo_limit;

		if (*count < POLICY_FIFO_LIMIT_COUNT) {
			return KERN_FAILURE;
		}

		fifo_limit = (policy_fifo_limit_t) info;
		fifo_limit->max_priority = MAXPRI_KERNEL;

		*count = POLICY_FIFO_LIMIT_COUNT;
		*host = &realhost;
		return KERN_SUCCESS;
	} else if (flavor == PROCESSOR_SET_RR_LIMITS) {
		policy_rr_limit_t               rr_limit;

		if (*count < POLICY_RR_LIMIT_COUNT) {
			return KERN_FAILURE;
		}

		rr_limit = (policy_rr_limit_t) info;
		rr_limit->max_priority = MAXPRI_KERNEL;

		*count = POLICY_RR_LIMIT_COUNT;
		*host = &realhost;
		return KERN_SUCCESS;
	} else if (flavor == PROCESSOR_SET_ENABLED_POLICIES) {
		int                             *enabled;

		if (*count < (sizeof(*enabled) / sizeof(int))) {
			return KERN_FAILURE;
		}

		enabled = (int *) info;
		*enabled = POLICY_TIMESHARE | POLICY_RR | POLICY_FIFO;

		*count = sizeof(*enabled) / sizeof(int);
		*host = &realhost;
		return KERN_SUCCESS;
	}


	*host = HOST_NULL;
	return KERN_INVALID_ARGUMENT;
}

kern_return_t
processor_info_count(
	processor_flavor_t              flavor,
	mach_msg_type_number_t  *count)
{
	switch (flavor) {
	case PROCESSOR_BASIC_INFO:
		*count = PROCESSOR_BASIC_INFO_COUNT;
		break;

	case PROCESSOR_CPU_LOAD_INFO:
		*count = PROCESSOR_CPU_LOAD_INFO_COUNT;
		break;

	default:
		return cpu_info_count(flavor, count);
	}

	return KERN_SUCCESS;
}

// </copied>

// <copied from="xnu://7195.141.2/osfmk/i386/cpu.c">

/*ARGSUSED*/
kern_return_t
cpu_info_count(
	__unused processor_flavor_t      flavor,
	unsigned int                    *count)
{
	*count = 0;
	return KERN_FAILURE;
}

// </copied>
