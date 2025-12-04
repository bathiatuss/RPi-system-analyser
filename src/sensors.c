#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <time.h>
#include "sensors.h"

float get_cpu_temp() {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) return -1.0;

    int temp_milli;
    fscanf(fp, "%d", &temp_milli);
    fclose(fp);

    return temp_milli / 1000.0;
}

float get_cpu_usage() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return -1.0;

    unsigned long user, nice, system, idle;
    fscanf(fp, "cpu  %lu %lu %lu %lu", &user, &nice, &system, &idle);
    fclose(fp);

    static unsigned long prev_total = 0, prev_idle = 0;

    unsigned long total = user + nice + system + idle;

    unsigned long total_diff = total - prev_total;
    unsigned long idle_diff = idle - prev_idle;

    prev_total = total;
    prev_idle = idle;

    if (total_diff == 0) return 0;

    return (float)(total_diff - idle_diff) * 100.0f / total_diff;
}

RamInfo get_ram_info() {
    RamInfo ram = {0};
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        ram.usage_percent = -1.0;
        return ram;
    }

    unsigned long mem_total = 0, mem_free = 0, mem_available = 0;
    unsigned long buffers = 0, cached = 0;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %lu kB", &mem_total) == 1) continue;
        if (sscanf(line, "MemFree: %lu kB", &mem_free) == 1) continue;
        if (sscanf(line, "MemAvailable: %lu kB", &mem_available) == 1) continue;
        if (sscanf(line, "Buffers: %lu kB", &buffers) == 1) continue;
        if (sscanf(line, "Cached: %lu kB", &cached) == 1) continue;
    }
    fclose(fp);

    // Convert from KB to MB
    ram.total_mb = mem_total / 1024;
    ram.free_mb = mem_available / 1024;
    ram.used_mb = ram.total_mb - ram.free_mb;
    
    if (mem_total > 0) {
        ram.usage_percent = (float)(mem_total - mem_available) * 100.0f / mem_total;
    } else {
        ram.usage_percent = 0.0;
    }

    return ram;
}

unsigned long get_cpu_clock() {
    // Try reading from /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq first
    FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
    if (fp) {
        unsigned long freq_khz = 0;
        if (fscanf(fp, "%lu", &freq_khz) == 1) {
            fclose(fp);
            return freq_khz / 1000; // Convert KHz to MHz
        }
        fclose(fp);
    }
    
    // Fallback to vcgencmd (requires root)
    fp = popen("vcgencmd measure_clock arm 2>/dev/null", "r");
    if (!fp) return 0;

    char output[128];
    unsigned long clock_hz = 0;
    
    if (fgets(output, sizeof(output), fp)) {
        // Output format: frequency(45)=600000000
        sscanf(output, "frequency(%*d)=%lu", &clock_hz);
    }
    pclose(fp);
    
    return clock_hz / 1000000; // Convert Hz to MHz
}

float get_core_voltage() {
    FILE *fp = popen("vcgencmd measure_volts core 2>/dev/null", "r");
    if (!fp) return -1.0;

    char output[128];
    float voltage = 0.0;
    
    if (fgets(output, sizeof(output), fp)) {
        // Output format: volt=1.2000V
        if (sscanf(output, "volt=%fV", &voltage) != 1) {
            voltage = -1.0;
        }
    }
    pclose(fp);
    
    return voltage;
}

unsigned int get_gpu_mem() {
    FILE *fp = popen("vcgencmd get_mem gpu 2>/dev/null", "r");
    if (!fp) return 0;

    char output[128];
    unsigned int gpu_mem = 0;
    
    if (fgets(output, sizeof(output), fp)) {
        // Output format: gpu=76M
        if (sscanf(output, "gpu=%uM", &gpu_mem) != 1) {
            gpu_mem = 0;
        }
    }
    pclose(fp);
    
    return gpu_mem;
}

ThrottleStatus get_throttle_status() {
    ThrottleStatus status = {0};
    status.raw_value = 0xFFFFFFFF; // Use special value to indicate unavailable
    FILE *fp = popen("vcgencmd get_throttled 2>/dev/null", "r");
    if (!fp) return status;

    char output[128];
    
    if (fgets(output, sizeof(output), fp)) {
        // Output format: throttled=0x0
        if (sscanf(output, "throttled=%x", &status.raw_value) == 1) {
            // Parse individual bits
            status.undervoltage = (status.raw_value & 0x1) ? 1 : 0;
            status.freq_capped = (status.raw_value & 0x2) ? 1 : 0;
            status.throttled = (status.raw_value & 0x4) ? 1 : 0;
            status.soft_temp_limit = (status.raw_value & 0x8) ? 1 : 0;
            status.undervoltage_occurred = (status.raw_value & 0x10000) ? 1 : 0;
            status.freq_capped_occurred = (status.raw_value & 0x20000) ? 1 : 0;
            status.throttled_occurred = (status.raw_value & 0x40000) ? 1 : 0;
            status.soft_temp_limit_occurred = (status.raw_value & 0x80000) ? 1 : 0;
        }
    }
    pclose(fp);
    
    return status;
}

Uptime get_uptime() {
    Uptime uptime = {0};
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp) return uptime;

    fscanf(fp, "%lf", &uptime.total_seconds);
    fclose(fp);

    unsigned long total = (unsigned long)uptime.total_seconds;
    uptime.days = total / 86400;
    uptime.hours = (total % 86400) / 3600;
    uptime.minutes = (total % 3600) / 60;
    uptime.seconds = total % 60;

    return uptime;
}

LoadAvg get_load_avg() {
    LoadAvg load = {0};
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) return load;

    fscanf(fp, "%f %f %f", &load.load_1min, &load.load_5min, &load.load_15min);
    fclose(fp);

    return load;
}

CpuInfo get_cpu_info() {
    CpuInfo cpu = {0};
    strcpy(cpu.model, "Unknown");
    strcpy(cpu.architecture, "Unknown");
    
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return cpu;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                colon += 2; // Skip ": "
                // Remove newline
                char *newline = strchr(colon, '\n');
                if (newline) *newline = '\0';
                strncpy(cpu.model, colon, sizeof(cpu.model) - 1);
            }
        } else if (strncmp(line, "Model", 5) == 0) {
            // For ARM processors (Raspberry Pi)
            char *colon = strchr(line, ':');
            if (colon) {
                colon += 2;
                char *newline = strchr(colon, '\n');
                if (newline) *newline = '\0';
                strncpy(cpu.model, colon, sizeof(cpu.model) - 1);
            }
        } else if (strncmp(line, "processor", 9) == 0) {
            cpu.cores++;
        }
    }
    fclose(fp);

    // Get architecture
    fp = popen("uname -m 2>/dev/null", "r");
    if (fp) {
        if (fgets(cpu.architecture, sizeof(cpu.architecture), fp)) {
            char *newline = strchr(cpu.architecture, '\n');
            if (newline) *newline = '\0';
        }
        pclose(fp);
    }

    return cpu;
}

DiskInfo get_disk_info() {
    DiskInfo disk = {0};
    struct statvfs stat;

    if (statvfs("/", &stat) != 0) {
        disk.usage_percent = -1.0;
        return disk;
    }

    // Calculate in GB
    unsigned long block_size = stat.f_frsize;
    disk.total_gb = (stat.f_blocks * block_size) / (1024 * 1024 * 1024);
    disk.free_gb = (stat.f_bavail * block_size) / (1024 * 1024 * 1024);
    disk.used_gb = disk.total_gb - disk.free_gb;
    
    if (disk.total_gb > 0) {
        disk.usage_percent = (float)disk.used_gb * 100.0f / disk.total_gb;
    }

    return disk;
}

ProcessList get_process_list(int max_processes) {
    ProcessList list = {0};
    list.capacity = 1024; // Allocate for up to 1024 processes
    list.processes = malloc(sizeof(ProcessInfo) * list.capacity);
    list.count = 0;
    
    if (!list.processes) return list;
    
    // Get total RAM for percentage calculation
    RamInfo ram = get_ram_info();
    unsigned long total_ram_kb = ram.total_mb * 1024;
    
    // Get system tick count for CPU calculation
    static unsigned long prev_total_time[4096] = {0};
    static double prev_uptime = 0;
    
    FILE *uptime_fp = fopen("/proc/uptime", "r");
    double current_uptime = 0;
    if (uptime_fp) {
        fscanf(uptime_fp, "%lf", &current_uptime);
        fclose(uptime_fp);
    }
    double uptime_delta = current_uptime - prev_uptime;
    if (uptime_delta < 0.01) uptime_delta = 1.0; // Prevent division by zero
    
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) return list;
    
    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL && list.count < list.capacity) {
        // Check if directory name is a number (PID)
        if (entry->d_type != DT_DIR) continue;
        
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        
        ProcessInfo proc = {0};
        proc.pid = pid;
        
        // Read /proc/[pid]/stat
        // Format: pid (comm) state ppid pgrp session tty_nr tpgid flags minflt cminflt majflt cmajflt utime stime cutime cstime ...
        char stat_path[256];
        snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);
        FILE *fp = fopen(stat_path, "r");
        if (!fp) continue;
        
        unsigned long utime = 0, stime = 0, vsize = 0;
        long rss = 0;
        int ret = fscanf(fp, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %lu %ld",
               &proc.state, &utime, &stime, &vsize, &rss);
        fclose(fp);
        
        if (ret < 5) continue; // Skip if we couldn't read all fields
        
        // Calculate CPU percentage
        unsigned long total_time = utime + stime;
        unsigned long time_delta = total_time - prev_total_time[pid % 4096];
        
        // CPU % calculation:
        // time_delta is in clock ticks (jiffies)
        // uptime_delta is in seconds
        // sysconf(_SC_CLK_TCK) is typically 100 (100 ticks per second)
        // Formula: (time_delta / (uptime_delta * CLK_TCK)) * 100
        // For system-wide compatibility, divide by number of cores
        if (prev_total_time[pid % 4096] > 0 && uptime_delta > 0.01 && total_time >= prev_total_time[pid % 4096]) {
            long clk_tck = sysconf(_SC_CLK_TCK);
            if (clk_tck <= 0) clk_tck = 100;
            
            // Get number of CPU cores
            static int num_cores = 0;
            if (num_cores == 0) {
                num_cores = sysconf(_SC_NPROCESSORS_ONLN);
                if (num_cores <= 0) num_cores = 1;
            }
            
            // Sanity check: time_delta should be reasonable
            // Maximum: uptime_delta * CLK_TCK * num_cores (all cores at 100%)
            unsigned long max_delta = (unsigned long)(uptime_delta * clk_tck * num_cores * 1.1); // 10% margin
            if (time_delta > max_delta) {
                // Process restarted or hash collision - reset
                proc.cpu_percent = 0.0;
                prev_total_time[pid % 4096] = total_time;
                goto skip_cpu;
            }
            
            // Calculate raw CPU percentage
            float raw_cpu = (float)time_delta / (uptime_delta * clk_tck) * 100.0;
            // Normalize by number of cores so it matches system-wide CPU%
            proc.cpu_percent = raw_cpu / num_cores;
            
            // Cap at reasonable value (100% means using all cores)
            if (proc.cpu_percent > 100.0) proc.cpu_percent = 100.0;
            if (proc.cpu_percent < 0.0) proc.cpu_percent = 0.0;
        } else {
            proc.cpu_percent = 0.0;
        }
        
        skip_cpu:
        prev_total_time[pid % 4096] = total_time;
        
        // RAM in KB (RSS * page_size)
        proc.ram_kb = rss * 4; // Assuming 4KB pages
        proc.ram_percent = (total_ram_kb > 0) ? 
                          ((float)proc.ram_kb * 100.0 / total_ram_kb) : 0.0;
        
        // Get process name from /proc/[pid]/comm
        char comm_path[256];
        snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);
        fp = fopen(comm_path, "r");
        if (fp) {
            if (fgets(proc.name, sizeof(proc.name), fp)) {
                char *newline = strchr(proc.name, '\n');
                if (newline) *newline = '\0';
            }
            fclose(fp);
        }
        
        // Get user from /proc/[pid]/status
        char status_path[256];
        snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);
        fp = fopen(status_path, "r");
        if (fp) {
            char line[256];
            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "Uid:", 4) == 0) {
                    int uid;
                    sscanf(line, "Uid:\t%d", &uid);
                    snprintf(proc.user, sizeof(proc.user), "%d", uid);
                    break;
                }
            }
            fclose(fp);
        }
        
        list.processes[list.count++] = proc;
    }
    
    closedir(proc_dir);
    
    // Update prev_uptime for next call
    if (prev_uptime == 0) prev_uptime = current_uptime;
    prev_uptime = current_uptime;
    
    // Sort by CPU usage (descending)
    for (int i = 0; i < list.count - 1; i++) {
        for (int j = 0; j < list.count - i - 1; j++) {
            if (list.processes[j].cpu_percent < list.processes[j + 1].cpu_percent) {
                ProcessInfo temp = list.processes[j];
                list.processes[j] = list.processes[j + 1];
                list.processes[j + 1] = temp;
            }
        }
    }
    
    return list;
}

void free_process_list(ProcessList *list) {
    if (list && list->processes) {
        free(list->processes);
        list->processes = NULL;
        list->count = 0;
        list->capacity = 0;
    }
}

ProcessGroupList group_processes(ProcessList *list) {
    ProcessGroupList grouped = {0};
    grouped.capacity = 256;
    grouped.groups = malloc(sizeof(ProcessGroup) * grouped.capacity);
    grouped.count = 0;
    
    if (!grouped.groups || !list) return grouped;
    
    // Group processes by name
    for (int i = 0; i < list->count; i++) {
        ProcessInfo *proc = &list->processes[i];
        
        // Find existing group
        int group_idx = -1;
        for (int j = 0; j < grouped.count; j++) {
            if (strcmp(grouped.groups[j].name, proc->name) == 0) {
                group_idx = j;
                break;
            }
        }
        
        if (group_idx == -1) {
            // Create new group
            if (grouped.count >= grouped.capacity) break;
            
            ProcessGroup *group = &grouped.groups[grouped.count];
            strncpy(group->name, proc->name, sizeof(group->name) - 1);
            strncpy(group->user, proc->user, sizeof(group->user) - 1);
            group->process_count = 1;
            group->pids = malloc(sizeof(int) * 64); // Max 64 PIDs per group
            group->pids[0] = proc->pid;
            group->total_cpu_percent = proc->cpu_percent;
            group->total_ram_kb = proc->ram_kb;
            group->total_ram_percent = proc->ram_percent;
            
            grouped.count++;
        } else {
            // Add to existing group
            ProcessGroup *group = &grouped.groups[group_idx];
            if (group->process_count < 64) {
                group->pids[group->process_count] = proc->pid;
                group->process_count++;
                group->total_cpu_percent += proc->cpu_percent;
                group->total_ram_kb += proc->ram_kb;
                group->total_ram_percent += proc->ram_percent;
            }
        }
    }
    
    // Sort groups by CPU usage
    for (int i = 0; i < grouped.count - 1; i++) {
        for (int j = 0; j < grouped.count - i - 1; j++) {
            if (grouped.groups[j].total_cpu_percent < grouped.groups[j + 1].total_cpu_percent) {
                ProcessGroup temp = grouped.groups[j];
                grouped.groups[j] = grouped.groups[j + 1];
                grouped.groups[j + 1] = temp;
            }
        }
    }
    
    return grouped;
}

void free_process_group_list(ProcessGroupList *list) {
    if (list && list->groups) {
        for (int i = 0; i < list->count; i++) {
            if (list->groups[i].pids) {
                free(list->groups[i].pids);
            }
        }
        free(list->groups);
        list->groups = NULL;
        list->count = 0;
        list->capacity = 0;
    }
}

// Helper function to convert hex string to IP address
static void hex_to_ip(const char *hex_str, char *ip_str) {
    unsigned int ip;
    sscanf(hex_str, "%X", &ip);
    sprintf(ip_str, "%u.%u.%u.%u", 
            ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
}

// Helper function to get process name by inode
static void get_process_by_inode(unsigned long inode, char *process_name, int *pid) {
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type != DT_DIR) continue;
        
        int proc_pid = atoi(entry->d_name);
        if (proc_pid <= 0) continue;
        
        char fd_path[512];
        snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd", proc_pid);
        
        DIR *fd_dir = opendir(fd_path);
        if (!fd_dir) continue;
        
        struct dirent *fd_entry;
        while ((fd_entry = readdir(fd_dir)) != NULL) {
            char link_path[512], link_target[512];
            snprintf(link_path, sizeof(link_path), "/proc/%d/fd/%s", proc_pid, fd_entry->d_name);
            
            ssize_t len = readlink(link_path, link_target, sizeof(link_target) - 1);
            if (len > 0) {
                link_target[len] = '\0';
                char expected[64];
                snprintf(expected, sizeof(expected), "socket:[%lu]", inode);
                if (strcmp(link_target, expected) == 0) {
                    // Found the process, get its name
                    char comm_path[512];
                    snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", proc_pid);
                    FILE *comm_fp = fopen(comm_path, "r");
                    if (comm_fp) {
                        if (fgets(process_name, 256, comm_fp)) {
                            // Remove newline
                            process_name[strcspn(process_name, "\n")] = 0;
                        }
                        fclose(comm_fp);
                    }
                    *pid = proc_pid;
                    closedir(fd_dir);
                    closedir(proc_dir);
                    return;
                }
            }
        }
        closedir(fd_dir);
    }
    closedir(proc_dir);
    strcpy(process_name, "-");
    *pid = 0;
}

// Parse /proc/net/tcp or /proc/net/udp
static void parse_net_file(const char *filename, const char *protocol, PortList *list) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return;
    
    char line[512];
    fgets(line, sizeof(line), fp); // Skip header
    
    while (fgets(line, sizeof(line), fp)) {
        unsigned int local_ip, local_port, remote_ip, remote_port, state;
        unsigned long inode;
        
        // Parse the line
        if (sscanf(line, "%*d: %X:%X %X:%X %X %*X:%*X %*X:%*X %*X %*d %*d %lu",
                   &local_ip, &local_port, &remote_ip, &remote_port, &state, &inode) == 6) {
            
            // Expand capacity if needed
            if (list->count >= list->capacity) {
                list->capacity = list->capacity == 0 ? 64 : list->capacity * 2;
                list->ports = realloc(list->ports, list->capacity * sizeof(PortInfo));
            }
            
            PortInfo *port = &list->ports[list->count];
            
            // Fill protocol
            strcpy(port->protocol, protocol);
            
            // Store local port
            port->local_port = local_port;
            
            // Convert IPs
            char local_ip_str[16], remote_ip_str[16];
            hex_to_ip((char[]){(local_ip >> 0) & 0xFF, (local_ip >> 8) & 0xFF, 
                               (local_ip >> 16) & 0xFF, (local_ip >> 24) & 0xFF, 0}, local_ip_str);
            hex_to_ip((char[]){(remote_ip >> 0) & 0xFF, (remote_ip >> 8) & 0xFF, 
                               (remote_ip >> 16) & 0xFF, (remote_ip >> 24) & 0xFF, 0}, remote_ip_str);
            
            sprintf(local_ip_str, "%u.%u.%u.%u", 
                    local_ip & 0xFF, (local_ip >> 8) & 0xFF, 
                    (local_ip >> 16) & 0xFF, (local_ip >> 24) & 0xFF);
            sprintf(remote_ip_str, "%u.%u.%u.%u", 
                    remote_ip & 0xFF, (remote_ip >> 8) & 0xFF, 
                    (remote_ip >> 16) & 0xFF, (remote_ip >> 24) & 0xFF);
            
            snprintf(port->local_addr, sizeof(port->local_addr), "%s:%u", local_ip_str, local_port);
            snprintf(port->remote_addr, sizeof(port->remote_addr), "%s:%u", remote_ip_str, remote_port);
            
            // Set state
            if (strcmp(protocol, "TCP") == 0) {
                const char *states[] = {"", "ESTABLISHED", "SYN_SENT", "SYN_RECV", 
                                       "FIN_WAIT1", "FIN_WAIT2", "TIME_WAIT", "CLOSE",
                                       "CLOSE_WAIT", "LAST_ACK", "LISTEN", "CLOSING"};
                if (state < 12) {
                    strcpy(port->state, states[state]);
                } else {
                    strcpy(port->state, "UNKNOWN");
                }
            } else {
                strcpy(port->state, "");
            }
            
            // Get process info
            get_process_by_inode(inode, port->process_name, &port->pid);
            
            list->count++;
        }
    }
    
    fclose(fp);
}

PortList get_port_list() {
    PortList list = {0};
    list.capacity = 64;
    list.ports = malloc(list.capacity * sizeof(PortInfo));
    list.count = 0;
    
    // Parse TCP connections
    parse_net_file("/proc/net/tcp", "TCP", &list);
    
    // Parse UDP connections
    parse_net_file("/proc/net/udp", "UDP", &list);
    
    return list;
}

void free_port_list(PortList *list) {
    if (list && list->ports) {
        free(list->ports);
        list->ports = NULL;
        list->count = 0;
        list->capacity = 0;
    }
}

ProcessStatsReport get_process_stats_from_logs(const char *log_file_path, int time_range, int top_n, int sort_mode) {
    ProcessStatsReport report = {0};
    report.time_range = time_range;
    report.end_time = time(NULL);
    
    // Calculate start time based on time range
    switch (time_range) {
        case 0: // Instant (last 1 minute)
            report.start_time = report.end_time - 60;
            break;
        case 1: // 1 Day
            report.start_time = report.end_time - (24 * 3600);
            break;
        case 2: // 1 Week
            report.start_time = report.end_time - (7 * 24 * 3600);
            break;
        case 3: // 1 Month
            report.start_time = report.end_time - (30 * 24 * 3600);
            break;
        case 4: // 1 Year
            report.start_time = report.end_time - (365 * 24 * 3600);
            break;
        default:
            report.start_time = report.end_time - 60;
    }
    
    FILE *fp = fopen(log_file_path, "r");
    if (!fp) {
        report.stats = NULL;
        report.count = 0;
        return report;
    }
    
    // Temporary storage for process stats (use hash-like array)
    #define MAX_PROCESSES 200
    ProcessUsageStats temp_stats[MAX_PROCESSES];
    int temp_count = 0;
    
    char line[512];
    int first_line = 1;
    
    // Read CSV file
    while (fgets(line, sizeof(line), fp)) {
        if (first_line) {
            first_line = 0;
            continue; // Skip header
        }
        
        time_t timestamp;
        char metric_name[300];
        double value;
        char unit[32];
        
        // Parse CSV line: timestamp,metric_name,value,unit
        if (sscanf(line, "%ld,%299[^,],%lf,%31s", &timestamp, metric_name, &value, unit) != 4) {
            continue;
        }
        
        // Filter by time range
        if (timestamp < report.start_time || timestamp > report.end_time) {
            continue;
        }
        
        // Check if this is a process metric
        if (strncmp(metric_name, "process_", 8) != 0) {
            continue;
        }
        
        // Extract process name and metric type
        char process_name[256];
        char metric_type[16];
        
        // Format: process_<name>_cpu or process_<name>_ram
        char *last_underscore = strrchr(metric_name, '_');
        if (!last_underscore || last_underscore == metric_name + 7) continue;
        
        // Copy metric type safely
        strncpy(metric_type, last_underscore + 1, sizeof(metric_type) - 1);
        metric_type[sizeof(metric_type) - 1] = '\0';
        
        // Calculate and validate name length
        int name_len = last_underscore - (metric_name + 8);
        if (name_len <= 0 || name_len >= 256) continue;
        
        // Copy process name safely
        memcpy(process_name, metric_name + 8, name_len);
        process_name[name_len] = '\0';
        
        // Find or create process stats entry
        int idx = -1;
        for (int i = 0; i < temp_count; i++) {
            if (strcmp(temp_stats[i].process_name, process_name) == 0) {
                idx = i;
                break;
            }
        }
        
        if (idx == -1) {
            if (temp_count >= MAX_PROCESSES) continue;
            idx = temp_count;
            temp_count++;
            strcpy(temp_stats[idx].process_name, process_name);
            temp_stats[idx].total_cpu_percent = 0;
            temp_stats[idx].total_ram_mb = 0;
            temp_stats[idx].avg_cpu_percent = 0;
            temp_stats[idx].avg_ram_mb = 0;
            temp_stats[idx].sample_count = 0;
        }
        
        // Accumulate values
        if (strcmp(metric_type, "cpu") == 0) {
            temp_stats[idx].total_cpu_percent += value;
        } else if (strcmp(metric_type, "ram") == 0) {
            temp_stats[idx].total_ram_mb += value;
        }
        
        temp_stats[idx].sample_count++;
    }
    
    fclose(fp);
    
    // Calculate averages
    for (int i = 0; i < temp_count; i++) {
        if (temp_stats[i].sample_count > 0) {
            // Each process has 2 samples per snapshot (cpu and ram)
            int actual_samples = temp_stats[i].sample_count / 2;
            if (actual_samples == 0) actual_samples = 1;
            
            temp_stats[i].avg_cpu_percent = temp_stats[i].total_cpu_percent / actual_samples;
            temp_stats[i].avg_ram_mb = temp_stats[i].total_ram_mb / actual_samples;
        }
    }
    
    // Sort by CPU or RAM
    for (int i = 0; i < temp_count - 1; i++) {
        for (int j = 0; j < temp_count - i - 1; j++) {
            int should_swap = 0;
            if (sort_mode == 0) { // Sort by CPU
                should_swap = temp_stats[j].total_cpu_percent < temp_stats[j + 1].total_cpu_percent;
            } else { // Sort by RAM
                should_swap = temp_stats[j].total_ram_mb < temp_stats[j + 1].total_ram_mb;
            }
            
            if (should_swap) {
                ProcessUsageStats temp = temp_stats[j];
                temp_stats[j] = temp_stats[j + 1];
                temp_stats[j + 1] = temp;
            }
        }
    }
    
    // Select top N
    int count = temp_count < top_n ? temp_count : top_n;
    report.count = count;
    report.stats = malloc(count * sizeof(ProcessUsageStats));
    
    for (int i = 0; i < count; i++) {
        report.stats[i] = temp_stats[i];
    }
    
    return report;
}

void free_process_stats_report(ProcessStatsReport *report) {
    if (report && report->stats) {
        free(report->stats);
        report->stats = NULL;
        report->count = 0;
    }
}
