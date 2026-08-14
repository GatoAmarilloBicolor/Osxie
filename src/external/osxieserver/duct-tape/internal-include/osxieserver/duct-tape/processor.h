#ifndef _OSXIESERVER_DUCT_TAPE_PROCESOR_H_
#define _OSXIESERVER_DUCT_TAPE_PROCESOR_H_

void dtape_processor_init(void);

// Read cumulative CPU ticks from Linux /proc/stat.
// cpu_id < 0 reads the aggregate "cpu" line, otherwise the matching "cpuN" line.
// ticks is filled as [CPU_STATE_USER, CPU_STATE_NICE, CPU_STATE_SYSTEM, CPU_STATE_IDLE]
// (iowait is folded into idle so the four states sum to the full accounting).
// Returns 0 on success, -1 on failure.
int dtape_read_proc_stat_ticks(int cpu_id, uint64_t ticks[4]);

#endif // _OSXIESERVER_DUCT_TAPE_PROCESOR_H_
