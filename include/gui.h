#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include "sensors.h"

// Logo paths
#define LOGO_PATH "assets/logo.png"
#define LOGO_ICON_PATH "assets/logo_single_w_offset.png"

// GUI widgets structure
typedef struct {
    GtkWidget *window;
    GtkWidget *notebook;
    
    // Overview tab widgets
    GtkWidget *cpu_temp_label;
    GtkWidget *cpu_usage_label;
    GtkWidget *cpu_clock_label;
    GtkWidget *cpu_voltage_label;
    
    GtkWidget *ram_usage_label;
    GtkWidget *ram_bar;
    
    GtkWidget *disk_usage_label;
    GtkWidget *disk_bar;
    
    GtkWidget *uptime_label;
    GtkWidget *load_label;
    
    GtkWidget *status_label;
    
    // Process tab widgets
    GtkWidget *process_tree;
    GtkWidget *process_refresh_btn;
    GtkWidget *process_status_label;
    GtkWidget *process_filter_combo;
    GtkWidget *process_group_check;
    GtkWidget *process_search_entry;
    GtkListStore *process_store;
    int process_filter_mode; // 0=all, 1=root, 2=user
    int process_group_mode;  // 0=individual, 1=grouped
    
    // Network tab widgets
    GtkWidget *network_tree;
    GtkWidget *network_refresh_btn;
    GtkWidget *network_status_label;
    GtkWidget *network_filter_combo;
    GtkWidget *network_search_entry;
    GtkListStore *network_store;
    int network_filter_mode; // 0=all, 1=tcp, 2=udp
    
    // Stats tab widgets
    GtkWidget *stats_drawing_area;
    GtkWidget *stats_time_range_combo;
    GtkWidget *stats_top_n_combo;
    GtkWidget *stats_sort_combo;
    GtkWidget *stats_refresh_btn;
    GtkWidget *stats_export_btn;
    GtkWidget *stats_show_cpu_check;
    GtkWidget *stats_show_ram_check;
    GtkWidget *stats_list_tree;
    GtkListStore *stats_list_store;
    int stats_time_range;  // 0=instant, 1=1day, 2=1week, 3=1month, 4=1year
    int stats_top_n;       // 5, 10, 20
    int stats_sort_mode;   // 0=CPU, 1=RAM
    gboolean stats_show_cpu;  // Show CPU line
    gboolean stats_show_ram;  // Show RAM line
    ProcessStatsReport stats_report; // Current stats data
    double stats_mouse_x;  // Mouse X position
    double stats_mouse_y;  // Mouse Y position
    int stats_hover_index; // Index of hovered data point (-1 if none)
    
    // Zoom and pan state
    double stats_zoom_level;    // 1.0 = normal, >1.0 = zoomed in
    double stats_pan_offset_x;  // Horizontal pan offset
    gboolean stats_is_panning;  // Currently panning with mouse
    double stats_pan_start_x;   // Pan start position
    
    // CSV file path
    char csv_file_path[512];
    
} AppWidgets;

// Show splash screen with logo
void show_splash_screen();

// Initialize GTK GUI
void init_gui(int *argc, char ***argv, AppWidgets *widgets);

// Update all widgets with current sensor data
void update_overview_tab(AppWidgets *widgets);

// Update process tab
void update_process_tab(AppWidgets *widgets);

// Update network tab
void update_network_tab(AppWidgets *widgets);

// Timer callback for periodic updates
gboolean update_timer_callback(gpointer data);

// Process tab refresh button callback
void on_process_refresh_clicked(GtkButton *button, gpointer data);

// Network tab refresh button callback
void on_network_refresh_clicked(GtkButton *button, gpointer data);

// Network filter changed callback
void on_network_filter_changed(GtkComboBox *combo, gpointer data);

// Process filter changed callback
void on_process_filter_changed(GtkComboBox *combo, gpointer data);

// Process group checkbox callback
void on_process_group_toggled(GtkToggleButton *toggle, gpointer data);

// Search entry callbacks
void on_process_search_changed(GtkSearchEntry *entry, gpointer data);
void on_network_search_changed(GtkSearchEntry *entry, gpointer data);

// Stats tab functions
void update_stats_tab(AppWidgets *widgets);
void on_stats_refresh_clicked(GtkButton *button, gpointer data);
void on_stats_export_clicked(GtkButton *button, gpointer data);
void on_stats_time_range_changed(GtkComboBox *combo, gpointer data);
void on_stats_top_n_changed(GtkComboBox *combo, gpointer data);
void on_stats_sort_changed(GtkComboBox *combo, gpointer data);
void on_stats_metric_toggled(GtkToggleButton *toggle, gpointer data);
gboolean stats_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data);
gboolean stats_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer data);
gboolean stats_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean stats_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

#endif
