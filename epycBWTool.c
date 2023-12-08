#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include "epycBWTool.hpp"

int read_msr(int cpuid, off_t MSR_REGISTER_address, uint64_t *MSR_REGISTER_bits)
{
	char msr_file_name[64];
	sprintf(msr_file_name, "/dev/cpu/%d/msr_safe", cpuid);
	int fd = open(msr_file_name, O_RDONLY);
	if (fd < 0)
	{
		fprintf(stderr, "read msr error [%d]\n", cpuid);
		return -1;
	}

	if (pread(fd, MSR_REGISTER_bits, sizeof MSR_REGISTER_bits, (uint32_t) MSR_REGISTER_address) != sizeof MSR_REGISTER_bits)
	{
		fprintf(stderr, "read msr error - cannot read register %X \n", MSR_REGISTER_address);
		return -1;
	}
	close(fd);
	return 0;
}

int write_msr(int cpuid, off_t MSR_REGISTER_address, uint64_t MSR_REGISTER_bits)
{
	char msr_file_name[64];
	sprintf(msr_file_name, "/dev/cpu/%d/msr_safe", cpuid);
	int fd = open(msr_file_name, O_WRONLY);
	if (fd < 0)
	{
		fprintf(stderr, "read msr error [%d]\n", cpuid);
		return -1;
	}
	
	if (pwrite(fd, &MSR_REGISTER_bits, sizeof MSR_REGISTER_bits, MSR_REGISTER_address) != sizeof MSR_REGISTER_bits)
	{
		fprintf(stderr, "write msr error - cannot read register %X \n", MSR_REGISTER_address);
		return -1;
	}
	return 0;
}

double timespec2double(timespec time) 
{
	return time.tv_sec + (double)time.tv_nsec/1e9;
}


int socket_cpuids[2]   = {0, 64};
int socket_channels[2] = {0, 0 };

timespec spectime1, spectime2;

uint64_t socket_df_pcm_0[2] = {DF_PCM_DRAM_0, DF_PCM_DRAM_0};
uint64_t socket_df_pcm_1[2] = {DF_PCM_DRAM_1, DF_PCM_DRAM_1};
uint64_t socket_df_pcm_2[2] = {DF_PCM_DRAM_2, DF_PCM_DRAM_2};
uint64_t socket_df_pcm_3[2] = {DF_PCM_DRAM_3, DF_PCM_DRAM_3};

void stop_measuring()
{
	clock_gettime(CLOCK_REALTIME, &spectime2);

	for (int socket = 0; socket < 2; socket++) {
		// Stop measuring
		write_msr(socket_cpuids[socket], DF_PERF_CTL_0, 0x0);
		write_msr(socket_cpuids[socket], DF_PERF_CTL_1, 0x0);
		write_msr(socket_cpuids[socket], DF_PERF_CTL_2, 0x0);
		write_msr(socket_cpuids[socket], DF_PERF_CTL_3, 0x0);
	}
}

void start_measuring()
{
	for (int socket = 0; socket < 2; socket++) {
		write_msr(socket_cpuids[socket], DF_PERF_CTL_0, socket_df_pcm_0[socket]);
		write_msr(socket_cpuids[socket], DF_PERF_CTL_1, socket_df_pcm_1[socket]);
		write_msr(socket_cpuids[socket], DF_PERF_CTL_2, socket_df_pcm_2[socket]);
		write_msr(socket_cpuids[socket], DF_PERF_CTL_3, socket_df_pcm_3[socket]);
	}

	clock_gettime(CLOCK_REALTIME, &spectime1);
}

void reset_counters()
{
	for (int socket = 0; socket < 2; socket++) {
		// Reset counters to 0
		write_msr(socket_cpuids[socket], DF_PERF_CTR_0, 0x0);
		write_msr(socket_cpuids[socket], DF_PERF_CTR_1, 0x0);
		write_msr(socket_cpuids[socket], DF_PERF_CTR_2, 0x0);
		write_msr(socket_cpuids[socket], DF_PERF_CTR_3, 0x0);
	}
}
	// Barrier needed here in case of multithreading or multiprocessing inside a socket
void read_counters(uint64_t* data)
{
	for (int socket = 0; socket < 2; socket++) {
		// Read counters
		for (int i=0; i<4; i++)	{
			uint64_t DRAM_PMC_bits = 0x0;
			read_msr(socket_cpuids[socket], DF_PERF_CTR_0 + i*2, &DRAM_PMC_bits);
			data[socket*4 + i] = (DRAM_PMC_bits & DF_PERF_CTR_mask)*64;
		}
	}
}

void select_sets (int sets[2])
{
	for (int socket = 0; socket < 2; socket++) {
		// Select DRAM channels
		if(sets[socket]) {
			socket_channels[socket] = 1;
			socket_df_pcm_0[socket] = DF_PCM_DRAM_4; // DRAM channel 4
			socket_df_pcm_1[socket] = DF_PCM_DRAM_5; // DRAM channel 5
			socket_df_pcm_2[socket] = DF_PCM_DRAM_6; // DRAM channel 6
			socket_df_pcm_3[socket] = DF_PCM_DRAM_7; // DRAM channel 7
		} else {
			socket_channels[socket] = 0;
			socket_df_pcm_0[socket] = DF_PCM_DRAM_0; // DRAM channel 0
			socket_df_pcm_1[socket] = DF_PCM_DRAM_1; // DRAM channel 1
			socket_df_pcm_2[socket] = DF_PCM_DRAM_2; // DRAM channel 2
			socket_df_pcm_3[socket] = DF_PCM_DRAM_3; // DRAM channel 3
		}
	}
}

void print_bandwidth ()
{
	double elapsed_time = timespec2double(spectime2) - timespec2double(spectime1);
	for (int socket = 0; socket < 2; socket++) {
		// Read counters
		for (int i=0; i<4; i++)	{
			uint64_t DRAM_PMC_bits = 0x0;
			read_msr(socket_cpuids[socket], DF_PERF_CTR_0 + i*2, &DRAM_PMC_bits);
			uint64_t ctr_val = DRAM_PMC_bits & DF_PERF_CTR_mask;
        	double volume_gb = ctr_val * 64 / (1024.0*1024.0*1024.0);
			// std::cout << "Socket " << socket << " DRAM "<< i+socket_channels[socket]*4 << ": " << ctr_val << " \t\tGB: " << volume_gb << " \ts: " << elapsed_time << " \tGB/s: " << volume_gb / elapsed_time << "\n";
			printf("Socket %d DRAM %d: %d \t\tGB: %f \ts: %f \tGB/s: %f\n", socket, i+socket_channels[socket]*4, ctr_val, volume_gb, elapsed_time, volume_gb / elapsed_time);
		}
	}
}