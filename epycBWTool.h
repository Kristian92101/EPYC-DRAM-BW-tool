#include <sys/types.h>
#include <stdint.h>

#ifndef BANDWIDTH_MEASUREMENT_HPP
#define BANDWIDTH_MEASUREMENT_HPP

int read_msr(int cpuid, off_t MSR_REGISTER_address, uint64_t *MSR_REGISTER_bits);
int write_msr(int cpuid, off_t MSR_REGISTER_address, uint64_t MSR_REGISTER_bits);

double timespec2double(timespec time);
void stop_measuring();
void start_measuring();
void reset_counters();
void read_counters(uint64_t* data);
void select_sets(int sets[2]);
void print_bandwidth();

#define DF_PERF_CTL_0 0xC0010240
#define DF_PERF_CTL_1 0xC0010242
#define DF_PERF_CTL_2 0xC0010244
#define DF_PERF_CTL_3 0xC0010246

#define DF_PERF_CTR_0 0xC0010241
#define DF_PERF_CTR_1 0xC0010243
#define DF_PERF_CTR_2 0xC0010245
#define DF_PERF_CTR_3 0xC0010247

#define DF_PERF_CTR_mask 0xFFFFFFFFFFFF

#define DF_PCM_DRAM_0 0x403807
#define DF_PCM_DRAM_1 0x403847
#define DF_PCM_DRAM_2 0x403887
#define DF_PCM_DRAM_3 0x4038C7
#define DF_PCM_DRAM_4 0x100403807
#define DF_PCM_DRAM_5 0x100403847
#define DF_PCM_DRAM_6 0x100403887
#define DF_PCM_DRAM_7 0x1004038C7

extern int socket_cpuids[2]  ;
extern int socket_channels[2];

extern uint64_t socket_df_pcm_0[2];
extern uint64_t socket_df_pcm_1[2];
extern uint64_t socket_df_pcm_2[2];
extern uint64_t socket_df_pcm_3[2];

extern timespec spectime1, spectime2;

#endif