#include <osxieserver/duct-tape/stubs.h>

#include <kern/host.h>
#include <mach_debug/mach_debug.h>
#include <osxieserver/duct-tape/processor.h>

#include <libsimple/lock.h>

// Linux sysinfo (from the sysinfo man page)
struct sysinfo {
	long uptime;
	unsigned long loads[3];
	unsigned long totalram;
	unsigned long freeram;
	unsigned long sharedram;
	unsigned long bufferram;
	unsigned long totalswap;
	unsigned long freeswap;
	unsigned short procs;
	unsigned long totalhigh;
	unsigned long freehigh;
	unsigned int mem_unit;
	char _f[20 - 2 * sizeof(long) - sizeof(int)];
};
int sysinfo(struct sysinfo *info);

// Linux sysconf
long sysconf(int name);
#define _SC_NPROCESSORS_CONF 83
#define _SC_NPROCESSORS_ONLN 84

static void cache_sysinfo(void* context) {
	struct sysinfo* cached_sysinfo = context;

	if (sysinfo(cached_sysinfo) < 0) {
		panic("Failed to retrieve sysinfo");
	}
};

kern_return_t host_info(host_t host, host_flavor_t flavor, host_info_t info, mach_msg_type_number_t* count) {
	static libsimple_once_t once_token = LIBSIMPLE_ONCE_INITIALIZER;
	static struct sysinfo cached_sysinfo;

	if (host == HOST_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	switch (flavor) {
		case HOST_BASIC_INFO: {
			host_basic_info_t basic_info = (host_basic_info_t)info;

			// need at least enough space for the legacy structure
			if (*count < HOST_BASIC_INFO_OLD_COUNT) {
				return KERN_FAILURE;
			}

			libsimple_once(&once_token, cache_sysinfo, &cached_sysinfo);

			uint64_t memsize = cached_sysinfo.totalram * cached_sysinfo.mem_unit;

			basic_info->memory_size = memsize;
#if __x86_64__ || __i386__
			basic_info->cpu_type = CPU_TYPE_X86;
#if __x86_64__
			basic_info->cpu_subtype = CPU_SUBTYPE_X86_64_ALL;
#else
			basic_info->cpu_subtype = CPU_SUBTYPE_I386_ALL;
#endif
#else
			#error Unknown CPU type
#endif
			basic_info->max_cpus = sysconf(_SC_NPROCESSORS_CONF);
			basic_info->avail_cpus = sysconf(_SC_NPROCESSORS_ONLN);

			// if there's room for the modern structure, fill in some additional info
			if (*count >= HOST_BASIC_INFO_COUNT) {
				// TODO: properly differentiate physical vs. logical cores
				dtape_stub_safe("modern HOST_BASIC_INFO");
				basic_info->cpu_threadtype = CPU_THREADTYPE_NONE;
				basic_info->physical_cpu = basic_info->avail_cpus;
				basic_info->physical_cpu_max = basic_info->max_cpus;
				basic_info->logical_cpu = basic_info->avail_cpus;
				basic_info->logical_cpu_max = basic_info->max_cpus;

				basic_info->max_mem = memsize;

				*count = HOST_BASIC_INFO_COUNT;
			} else {
				*count = HOST_BASIC_INFO_OLD_COUNT;
			}

			return KERN_SUCCESS;
		}

		case HOST_PRIORITY_INFO: {
			// <copied from="xnu://7195.141.2/osfmk/kern/host.c">
			host_priority_info_t priority_info;

			if (*count < HOST_PRIORITY_INFO_COUNT) {
				return KERN_FAILURE;
			}

			priority_info = (host_priority_info_t)info;

			priority_info->kernel_priority = MINPRI_KERNEL;
			priority_info->system_priority = MINPRI_KERNEL;
			priority_info->server_priority = MINPRI_RESERVED;
			priority_info->user_priority = BASEPRI_DEFAULT;
			priority_info->depress_priority = DEPRESSPRI;
			priority_info->idle_priority = IDLEPRI;
			priority_info->minimum_priority = MINPRI_USER;
			priority_info->maximum_priority = MAXPRI_RESERVED;

			*count = HOST_PRIORITY_INFO_COUNT;

			return KERN_SUCCESS;
			// </copied>
		}

		case HOST_DEBUG_INFO_INTERNAL:
			return KERN_NOT_SUPPORTED;

		case HOST_PREFERRED_USER_ARCH: {
			host_preferred_user_arch_t user_arch_info;

			if (*count < HOST_PREFERRED_USER_ARCH_COUNT) {
				return KERN_FAILURE;
			}

			*count = HOST_PREFERRED_USER_ARCH_COUNT;

			user_arch_info = (void*)info;

#if __x86_64__ || __i386__
			user_arch_info->cpu_type = CPU_TYPE_X86;
#if __x86_64__
			user_arch_info->cpu_subtype = CPU_SUBTYPE_X86_64_ALL;
#else
			user_arch_info->cpu_subtype = CPU_SUBTYPE_I386_ALL;
#endif
#elif __aarch64__
			// TODO: check whether this is actually what ARM64 macOS returns
			user_arch_info->cpu_type = CPU_TYPE_ARM64;
			user_arch_info->cpu_subtype = CPU_SUBTYPE_ARM64_ALL;
#else
			#error HOST_PREFERRED_USER_ARCH not implemented for this architecture
#endif

			return KERN_SUCCESS;
		};

		case HOST_SCHED_INFO:
			dtape_stub_unsafe("HOST_SCHED_INFO");
		case HOST_RESOURCE_SIZES:
			dtape_stub_unsafe("HOST_RESOURCE_SIZES");
		case HOST_CAN_HAS_DEBUGGER:
			dtape_stub_unsafe("HOST_CAN_HAS_DEBUGGER");
		case HOST_VM_PURGABLE:
			dtape_stub_unsafe("HOST_VM_PURGABLE");

		case HOST_MACH_MSG_TRAP:
		case HOST_SEMAPHORE_TRAPS:
			*count = 0;
			return KERN_SUCCESS;

		default:
			return KERN_INVALID_ARGUMENT;
	}
};

kern_return_t host_default_memory_manager(host_priv_t host_priv, memory_object_default_t* default_manager, memory_object_cluster_size_t cluster_size) {
	dtape_stub_unsafe();
};

kern_return_t host_get_boot_info(host_priv_t host_priv, kernel_boot_info_t boot_info) {
	dtape_stub_unsafe();
};

kern_return_t host_get_UNDServer(host_priv_t host_priv, UNDServerRef* serverp) {
	dtape_stub_unsafe();
};

kern_return_t host_set_UNDServer(host_priv_t host_priv, UNDServerRef server) {
	dtape_stub_unsafe();
};

kern_return_t host_lockgroup_info(host_t host, lockgroup_info_array_t* lockgroup_infop, mach_msg_type_number_t* lockgroup_infoCntp) {
	dtape_stub_unsafe();
};

kern_return_t host_reboot(host_priv_t host_priv, int options) {
	dtape_stub_unsafe();
};

kern_return_t host_security_create_task_token(host_security_t host_security, task_t parent_task, security_token_t sec_token, audit_token_t audit_token, host_priv_t host_priv, ledger_port_array_t ledger_ports, mach_msg_type_number_t num_ledger_ports, boolean_t inherit_memory, task_t* child_task) {
	dtape_stub_safe();
	return KERN_NOT_SUPPORTED;
};

kern_return_t host_security_set_task_token(host_security_t host_security, task_t task, security_token_t sec_token, audit_token_t audit_token, host_priv_t host_priv) {
	dtape_stub_unsafe();
};

kern_return_t host_virtual_physical_table_info(host_t host, hash_info_bucket_array_t* infop, mach_msg_type_number_t* countp) {
	dtape_stub_unsafe();
};

kern_return_t host_statistics(host_t host, host_flavor_t flavor, host_info_t info, mach_msg_type_number_t* count) {
	switch (flavor) {
		case HOST_LOAD_INFO: {
			host_load_info_t load_info = (host_load_info_t)info;

			if (*count < HOST_LOAD_INFO_COUNT) {
				return KERN_FAILURE;
			}

			static struct sysinfo cached_sysinfo;
			if (sysinfo(&cached_sysinfo) == 0) {
				// Linux /proc/loadavg values are scaled by 1<<16 (SI_LOAD_SHIFT);
				// Mach LOAD_SCALE is 1000 in this tree.
				for (unsigned int i = 0; i < 3; i++) {
					load_info->avenrun[i] = (integer_t)((cached_sysinfo.loads[i] * 1000) >> 16);
					load_info->mach_factor[i] = load_info->avenrun[i];
				}
			} else {
				memset(load_info, 0, sizeof(*load_info));
			}

			*count = HOST_LOAD_INFO_COUNT;
			return KERN_SUCCESS;
		}

		case HOST_VM_INFO: {
			vm_statistics_t stat32 = (vm_statistics_t)info;
			static struct sysinfo cached_sysinfo;

			if (*count < HOST_VM_INFO_REV0_COUNT) {
				return KERN_FAILURE;
			}

			if (sysinfo(&cached_sysinfo) == 0) {
				uint64_t page_size = 4096;
				uint64_t total_pages = (cached_sysinfo.totalram * cached_sysinfo.mem_unit) / page_size;
				uint64_t free_pages = (cached_sysinfo.freeram * cached_sysinfo.mem_unit) / page_size;
				uint64_t buffer_pages = (cached_sysinfo.bufferram * cached_sysinfo.mem_unit) / page_size;

				memset(stat32, 0, (*count) * sizeof(integer_t));
				stat32->free_count = free_pages;
				stat32->active_count = total_pages > free_pages ? (total_pages - free_pages) / 2 : 0;
				stat32->inactive_count = total_pages > free_pages ? (total_pages - free_pages) / 2 : 0;
				stat32->wire_count = buffer_pages;
			} else {
				memset(stat32, 0, (*count) * sizeof(integer_t));
			}

			return KERN_SUCCESS;
		}

		case HOST_CPU_LOAD_INFO: {
			host_cpu_load_info_t cpu_info = (host_cpu_load_info_t)info;
			uint64_t ticks[4];

			if (*count < HOST_CPU_LOAD_INFO_COUNT) {
				return KERN_FAILURE;
			}

			// real aggregate CPU ticks from /proc/stat
			if (dtape_read_proc_stat_ticks(-1, ticks) != 0) {
				memset(cpu_info, 0, sizeof(*cpu_info));
			} else {
				cpu_info->cpu_ticks[CPU_STATE_USER] = (integer_t)ticks[CPU_STATE_USER];
				cpu_info->cpu_ticks[CPU_STATE_SYSTEM] = (integer_t)ticks[CPU_STATE_SYSTEM];
				cpu_info->cpu_ticks[CPU_STATE_IDLE] = (integer_t)ticks[CPU_STATE_IDLE];
				cpu_info->cpu_ticks[CPU_STATE_NICE] = (integer_t)ticks[CPU_STATE_NICE];
			}
			*count = HOST_CPU_LOAD_INFO_COUNT;

			return KERN_SUCCESS;
		}

		default:
			dtape_stub_unsafe();
	}
};

kern_return_t vm_stats(void* info, unsigned int* count) {
	vm_statistics64_t stat = (vm_statistics64_t)info;
	static struct sysinfo cached_sysinfo;

	if (*count < HOST_VM_INFO64_COUNT)
		return (KERN_FAILURE);

	if (sysinfo(&cached_sysinfo) < 0) {
		memset(stat, 0, sizeof(*stat));
		*count = HOST_VM_INFO64_COUNT;
		return KERN_SUCCESS;
	}

	memset(stat, 0, sizeof(*stat));

	uint64_t page_size = 4096;
	uint64_t total_pages = (cached_sysinfo.totalram * cached_sysinfo.mem_unit) / page_size;
	uint64_t free_pages = (cached_sysinfo.freeram * cached_sysinfo.mem_unit) / page_size;
	uint64_t buffer_pages = (cached_sysinfo.bufferram * cached_sysinfo.mem_unit) / page_size;

	stat->free_count = free_pages;
	stat->active_count = total_pages > free_pages ? (total_pages - free_pages) / 2 : 0;
	stat->inactive_count = total_pages > free_pages ? (total_pages - free_pages) / 2 : 0;
	stat->wire_count = buffer_pages;

	*count = HOST_VM_INFO64_COUNT;

	return KERN_SUCCESS;
};
