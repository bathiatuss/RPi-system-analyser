#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "logger.h"
#include "sensors.h"

static FILE *log_file = NULL;
static char log_file_path[256] = {0};

void init_logger(const char *file_path) {
    if (!file_path) return;
    
    strncpy(log_file_path, file_path, sizeof(log_file_path) - 1);
    
    // Check if file exists
    struct stat st;
    int file_exists = (stat(log_file_path, &st) == 0);
    
    // Open file in append mode
    log_file = fopen(log_file_path, "a");
    if (!log_file) {
        fprintf(stderr, "Error: Cannot open log file: %s\n", log_file_path);
        return;
    }
    
    // Write CSV header if file is new
    if (!file_exists || st.st_size == 0) {
        fprintf(log_file, "timestamp,metric_name,value,unit\n");
        fflush(log_file);
    }
}

void close_logger() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_metric(const char *metric_name, double value, const char *unit) {
    if (!log_file || !metric_name || !unit) return;
    
    // Get current timestamp
    time_t now = time(NULL);
    
    // Write to CSV: timestamp,metric_name,value,unit
    fprintf(log_file, "%ld,%s,%.2f,%s\n", now, metric_name, value, unit);
    fflush(log_file);
}

void log_system_snapshot() {
    if (!log_file) return;
    
    // Get single timestamp for all metrics
    time_t now = time(NULL);
    
    // Gather all sensor data
    float cpu_temp = get_cpu_temp();
    float cpu_usage = get_cpu_usage();
    unsigned long cpu_clock = get_cpu_clock();
    float core_volt = get_core_voltage();
    
    RamInfo ram = get_ram_info();
    DiskInfo disk = get_disk_info();
    
    unsigned int gpu_mem = get_gpu_mem();
    
    LoadAvg load = get_load_avg();
    Uptime uptime = get_uptime();
    
    ThrottleStatus throttle = get_throttle_status();
    
    // Log CPU metrics
    if (cpu_temp >= 0) {
        fprintf(log_file, "%ld,cpu_temp,%.2f,celsius\n", now, cpu_temp);
    }
    if (cpu_usage >= 0) {
        fprintf(log_file, "%ld,cpu_usage,%.2f,percent\n", now, cpu_usage);
    }
    if (cpu_clock > 0) {
        fprintf(log_file, "%ld,cpu_clock,%lu,mhz\n", now, cpu_clock);
    }
    if (core_volt > 0) {
        fprintf(log_file, "%ld,cpu_voltage,%.4f,volts\n", now, core_volt);
    }
    
    // Log RAM metrics
    if (ram.usage_percent >= 0) {
        fprintf(log_file, "%ld,ram_total,%lu,mb\n", now, ram.total_mb);
        fprintf(log_file, "%ld,ram_used,%lu,mb\n", now, ram.used_mb);
        fprintf(log_file, "%ld,ram_free,%lu,mb\n", now, ram.free_mb);
        fprintf(log_file, "%ld,ram_usage,%.2f,percent\n", now, ram.usage_percent);
    }
    
    // Log Disk metrics
    if (disk.usage_percent >= 0) {
        fprintf(log_file, "%ld,disk_total,%lu,gb\n", now, disk.total_gb);
        fprintf(log_file, "%ld,disk_used,%lu,gb\n", now, disk.used_gb);
        fprintf(log_file, "%ld,disk_free,%lu,gb\n", now, disk.free_gb);
        fprintf(log_file, "%ld,disk_usage,%.2f,percent\n", now, disk.usage_percent);
    }
    
    // Log GPU metrics
    if (gpu_mem > 0) {
        fprintf(log_file, "%ld,gpu_mem,%u,mb\n", now, gpu_mem);
    }
    
    // Log Load Average
    fprintf(log_file, "%ld,load_1min,%.2f,load\n", now, load.load_1min);
    fprintf(log_file, "%ld,load_5min,%.2f,load\n", now, load.load_5min);
    fprintf(log_file, "%ld,load_15min,%.2f,load\n", now, load.load_15min);
    
    // Log Uptime
    fprintf(log_file, "%ld,uptime,%.0f,seconds\n", now, uptime.total_seconds);
    
    // Log Throttle Status
    if (throttle.raw_value != 0xFFFFFFFF) {
        fprintf(log_file, "%ld,throttle_status,0x%X,hex\n", now, throttle.raw_value);
    }
    
    // Log Process Statistics (top 20 by CPU or RAM usage)
    ProcessList plist = get_process_list(0);
    
    // Sort by CPU
    for (int i = 0; i < plist.count - 1; i++) {
        for (int j = 0; j < plist.count - i - 1; j++) {
            if (plist.processes[j].cpu_percent < plist.processes[j + 1].cpu_percent) {
                ProcessInfo temp = plist.processes[j];
                plist.processes[j] = plist.processes[j + 1];
                plist.processes[j + 1] = temp;
            }
        }
    }
    
    // Log top 20 processes
    int log_limit = plist.count < 20 ? plist.count : 20;
    for (int i = 0; i < log_limit; i++) {
        if (plist.processes[i].cpu_percent > 0.01 || plist.processes[i].ram_percent > 0.01) {
            // Use process name as metric identifier
            char metric_cpu[300], metric_ram[300];
            snprintf(metric_cpu, sizeof(metric_cpu), "process_%s_cpu", plist.processes[i].name);
            snprintf(metric_ram, sizeof(metric_ram), "process_%s_ram", plist.processes[i].name);
            
            fprintf(log_file, "%ld,%s,%.2f,percent\n", now, metric_cpu, plist.processes[i].cpu_percent);
            fprintf(log_file, "%ld,%s,%.2f,mb\n", now, metric_ram, (float)plist.processes[i].ram_kb / 1024.0);
        }
    }
    
    // Log total process count
    fprintf(log_file, "%ld,process_count,%d,number\n", now, plist.count);
    
    free_process_list(&plist);
    
    fflush(log_file);
}

void rotate_log_if_needed(unsigned long max_size_mb) {
    if (!log_file || !log_file_path[0]) return;
    
    struct stat st;
    if (stat(log_file_path, &st) != 0) return;
    
    unsigned long size_mb = st.st_size / (1024 * 1024);
    
    if (size_mb >= max_size_mb) {
        // Close current file
        fclose(log_file);
        
        // Rename current log to backup with timestamp
        time_t now = time(NULL);
        char backup_path[512];
        snprintf(backup_path, sizeof(backup_path), "%s.%ld.bak", log_file_path, now);
        rename(log_file_path, backup_path);
        
        // Reopen new log file
        log_file = fopen(log_file_path, "a");
        if (log_file) {
            fprintf(log_file, "timestamp,metric_name,value,unit\n");
            fflush(log_file);
        }
    }
}