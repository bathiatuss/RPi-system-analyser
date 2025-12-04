#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "sensors.h"
#include "logger.h"
#include "gui.h"

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);
    
    // Show splash screen
    show_splash_screen();
    
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
    
    // Initialize CPU usage baseline
    get_cpu_usage();
    sleep(1);
    
    // Initialize GUI
    AppWidgets widgets = {0};
    // Store CSV path in widgets
    strncpy(widgets.csv_file_path, log_path, sizeof(widgets.csv_file_path) - 1);
    init_gui(&argc, &argv, &widgets);
    
    // Initial update
    update_overview_tab(&widgets);
    
    // Setup timer for periodic updates (1000ms = 1 second)
    g_timeout_add(1000, update_timer_callback, &widgets);
    
    // Start GTK main loop
    gtk_main();
    
    // Cleanup
    close_logger();
    
    return 0;
}
