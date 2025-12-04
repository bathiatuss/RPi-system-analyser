#include <stdio.h>
#include <unistd.h> //sleep() func
#include <stdlib.h>
#include <sys/stat.h>
#include "sensors.h"
#include "logger.h"

int main(){

	// Create data directory if it doesn't exist
	char data_dir[512];
	const char *home = getenv("HOME");
	if (!home) home = "/tmp";
	
	// Create .local directory
	char local_dir[512];
	snprintf(local_dir, sizeof(local_dir), "%s/.local", home);
	mkdir(local_dir, 0755);
	
	// Create .local/share directory
	char share_dir[512];
	snprintf(share_dir, sizeof(share_dir), "%s/.local/share", home);
	mkdir(share_dir, 0755);
	
	// Create .local/share/system-analyser directory
	snprintf(data_dir, sizeof(data_dir), "%s/.local/share/system-analyser", home);
	mkdir(data_dir, 0755);
	
	// Initialize logger with full path
	char log_path[512];
	snprintf(log_path, sizeof(log_path), "%s/system_metrics.csv", data_dir);
	init_logger(log_path);
	
	get_cpu_usage();
	sleep(1);
	
	// Get CPU info once (doesn't change)
	CpuInfo cpu_info = get_cpu_info();

	while(1){
	system("clear");
	
	// Gather all sensor data
	Uptime uptime = get_uptime();
	LoadAvg load = get_load_avg();
	float temp = get_cpu_temp();
	float cpu_usage = get_cpu_usage();
	unsigned long cpu_clock = get_cpu_clock();
	float core_volt = get_core_voltage();
	RamInfo ram = get_ram_info();
	DiskInfo disk = get_disk_info();
	unsigned int gpu_mem = get_gpu_mem();
	ThrottleStatus throttle = get_throttle_status();

	printf("╔════════════════════════════════════════════════════════════╗\n");
	printf("║       Raspberry Pi System Analyser                        ║\n");
	printf("╚════════════════════════════════════════════════════════════╝\n\n");
	
	// System Info
	printf("SYSTEM:\n");
	printf("  Uptime: %lud %02luh %02lum %02lus\n", 
	       uptime.days, uptime.hours, uptime.minutes, uptime.seconds);
	printf("  Load Average: %.2f, %.2f, %.2f (1m, 5m, 15m)\n", 
	       load.load_1min, load.load_5min, load.load_15min);
	
	// CPU Info
	printf("\nCPU:\n");
	printf("  Model: %s\n", cpu_info.model);
	printf("  Architecture: %s | Cores: %d\n", cpu_info.architecture, cpu_info.cores);
	printf("  Temperature: %.2f°C\n", temp);
	printf("  Usage: %.2f%%\n", cpu_usage);
	if (cpu_clock > 0) {
		printf("  Clock Speed: %lu MHz\n", cpu_clock);
	} else {
		printf("  Clock Speed: N/A\n");
	}
	if (core_volt > 0) {
		printf("  Core Voltage: %.4fV\n", core_volt);
	} else {
		printf("  Core Voltage: N/A\n");
	}
	
	// RAM Info
	printf("\nRAM:\n");
	printf("  Total: %lu MB | Used: %lu MB | Free: %lu MB\n", 
	       ram.total_mb, ram.used_mb, ram.free_mb);
	printf("  Usage: %.2f%%\n", ram.usage_percent);
	
	// Disk Info
	printf("\nDISK (/):\n");
	printf("  Total: %lu GB | Used: %lu GB | Free: %lu GB\n", 
	       disk.total_gb, disk.used_gb, disk.free_gb);
	printf("  Usage: %.2f%%\n", disk.usage_percent);
	
	// GPU Info
	printf("\nGPU:\n");
	if (gpu_mem > 0) {
		printf("  Memory: %u MB\n", gpu_mem);
	} else {
		printf("  Memory: N/A (needs root)\n");
	}
	
	// Throttle Status
	printf("\nTHROTTLE STATUS:\n");
	if (throttle.raw_value == 0xFFFFFFFF) {
		printf("  N/A (needs root for vcgencmd access)\n");
	} else {
		printf("  Status Code: 0x%X\n", throttle.raw_value);
		if (throttle.undervoltage) printf("  ⚠️  Undervoltage detected\n");
		if (throttle.freq_capped) printf("  ⚠️  Frequency capped\n");
		if (throttle.throttled) printf("  ⚠️  Currently throttled\n");
		if (throttle.soft_temp_limit) printf("  ⚠️  Soft temp limit active\n");
		if (!throttle.raw_value) printf("  ✓ No throttling issues\n");
	}
	
	printf("\n");
	
	// Log all metrics to CSV
	log_system_snapshot();
	
	// Optional: Rotate log if it exceeds 10MB
	rotate_log_if_needed(10);
	
	sleep(1);
	}
	
	// Cleanup
	close_logger();

	return 0;
}

