#ifndef SENSORS_H
#define SENSORS_H

typedef struct {
    unsigned long total_mb;
    unsigned long used_mb;
    unsigned long free_mb;
    float usage_percent;
} RamInfo;

typedef struct {
    int undervoltage;
    int freq_capped;
    int throttled;
    int soft_temp_limit;
    int undervoltage_occurred;
    int freq_capped_occurred;
    int throttled_occurred;
    int soft_temp_limit_occurred;
    unsigned int raw_value;
} ThrottleStatus;

typedef struct {
    unsigned long days;
    unsigned long hours;
    unsigned long minutes;
    unsigned long seconds;
    double total_seconds;
} Uptime;

typedef struct {
    float load_1min;
    float load_5min;
    float load_15min;
} LoadAvg;

typedef struct {
    char model[128];
    int cores;
    char architecture[32];
} CpuInfo;

typedef struct {
    unsigned long total_gb;
    unsigned long used_gb;
    unsigned long free_gb;
    float usage_percent;
} DiskInfo;

typedef struct {
    int pid;
    char name[256];
    float cpu_percent;
    unsigned long ram_kb;
    float ram_percent;
    char state;
    char user[32];
} ProcessInfo;

typedef struct {
    char name[256];
    int process_count;
    int *pids;
    float total_cpu_percent;
    unsigned long total_ram_kb;
    float total_ram_percent;
    char user[32];
} ProcessGroup;

typedef struct {
    ProcessInfo *processes;
    int count;
    int capacity;
} ProcessList;

typedef struct {
    ProcessGroup *groups;
    int count;
    int capacity;
} ProcessGroupList;

typedef struct {
    char protocol[8];      // TCP or UDP
    unsigned int local_port; // Local port number
    char local_addr[64];   // Local IP:Port
    char remote_addr[64];  // Remote IP:Port
    char state[16];        // Connection state
    int pid;               // Process ID
    char process_name[256]; // Process name
} PortInfo;

typedef struct {
    PortInfo *ports;
    int count;
    int capacity;
} PortList;

typedef struct {
    char process_name[256];
    double total_cpu_percent;    // Total CPU usage
    double total_ram_mb;          // Total RAM usage (MB)
    double avg_cpu_percent;       // Average CPU usage
    double avg_ram_mb;            // Average RAM usage
    int sample_count;             // Number of samples
} ProcessUsageStats;

typedef struct {
    ProcessUsageStats *stats;
    int count;
    time_t start_time;
    time_t end_time;
    int time_range; // 0=instant, 1=1day, 2=1week, 3=1month, 4=1year
} ProcessStatsReport;

float get_cpu_usage();
float get_cpu_temp();
RamInfo get_ram_info();
unsigned long get_cpu_clock();
float get_core_voltage();
unsigned int get_gpu_mem();
ThrottleStatus get_throttle_status();
Uptime get_uptime();
LoadAvg get_load_avg();
CpuInfo get_cpu_info();
DiskInfo get_disk_info();
ProcessList get_process_list(int max_processes);
void free_process_list(ProcessList *list);
ProcessGroupList group_processes(ProcessList *list);
void free_process_group_list(ProcessGroupList *list);
PortList get_port_list();
void free_port_list(PortList *list);
ProcessStatsReport get_process_stats_from_logs(const char *log_file_path, int time_range, int top_n, int sort_mode);
void free_process_stats_report(ProcessStatsReport *report);

#endif
