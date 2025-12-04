#include <gtk/gtk.h>
#include <stdio.h>
#include <math.h>
#include "gui.h"
#include "sensors.h"
#include "logger.h"

static CpuInfo cpu_info;

// Forward declarations
void cpu_cell_data_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                       GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void ram_percent_cell_data_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                                GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void ram_mb_cell_data_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                           GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
GtkWidget* create_process_tab(AppWidgets *widgets);
GtkWidget* create_network_tab(AppWidgets *widgets);
GtkWidget* create_stats_tab(AppWidgets *widgets);
void on_process_filter_changed(GtkComboBox *combo, gpointer data);
gboolean stats_drawing_area_draw(GtkWidget *widget, cairo_t *cr, gpointer data);

GtkWidget* create_label_with_markup(const char *text) {
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(label, 10);
    gtk_widget_set_margin_end(label, 10);
    gtk_widget_set_margin_top(label, 5);
    gtk_widget_set_margin_bottom(label, 5);
    return label;
}

GtkWidget* create_progress_bar() {
    GtkWidget *bar = gtk_progress_bar_new();
    gtk_widget_set_margin_start(bar, 10);
    gtk_widget_set_margin_end(bar, 10);
    gtk_widget_set_margin_top(bar, 5);
    gtk_widget_set_margin_bottom(bar, 5);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(bar), TRUE);
    return bar;
}

GtkWidget* create_overview_tab(AppWidgets *widgets) {
    // Create scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    
    // System Info Header
    GtkWidget *header = create_label_with_markup("<b><big>System Overview</big></b>");
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 5);
    
    // CPU Info
    GtkWidget *cpu_info_label = create_label_with_markup(
        g_strdup_printf("<b>CPU:</b> %s | %s | %d cores", 
        cpu_info.model, cpu_info.architecture, cpu_info.cores));
    gtk_box_pack_start(GTK_BOX(main_box), cpu_info_label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // CPU Metrics Box
    GtkWidget *cpu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *cpu_label = create_label_with_markup("<b>CPU Metrics</b>");
    gtk_box_pack_start(GTK_BOX(cpu_box), cpu_label, FALSE, FALSE, 0);
    
    widgets->cpu_temp_label = create_label_with_markup("Temperature: --");
    widgets->cpu_usage_label = create_label_with_markup("Usage: --");
    widgets->cpu_clock_label = create_label_with_markup("Clock: --");
    widgets->cpu_voltage_label = create_label_with_markup("Voltage: --");
    
    gtk_box_pack_start(GTK_BOX(cpu_box), widgets->cpu_temp_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cpu_box), widgets->cpu_usage_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cpu_box), widgets->cpu_clock_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cpu_box), widgets->cpu_voltage_label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), cpu_box, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // RAM Section
    GtkWidget *ram_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *ram_header = create_label_with_markup("<b>RAM</b>");
    gtk_box_pack_start(GTK_BOX(ram_box), ram_header, FALSE, FALSE, 0);
    
    widgets->ram_usage_label = create_label_with_markup("Usage: --");
    gtk_box_pack_start(GTK_BOX(ram_box), widgets->ram_usage_label, FALSE, FALSE, 0);
    
    widgets->ram_bar = create_progress_bar();
    gtk_box_pack_start(GTK_BOX(ram_box), widgets->ram_bar, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), ram_box, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // Disk Section
    GtkWidget *disk_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *disk_header = create_label_with_markup("<b>Disk (/)</b>");
    gtk_box_pack_start(GTK_BOX(disk_box), disk_header, FALSE, FALSE, 0);
    
    widgets->disk_usage_label = create_label_with_markup("Usage: --");
    gtk_box_pack_start(GTK_BOX(disk_box), widgets->disk_usage_label, FALSE, FALSE, 0);
    
    widgets->disk_bar = create_progress_bar();
    gtk_box_pack_start(GTK_BOX(disk_box), widgets->disk_bar, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), disk_box, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // System Info
    GtkWidget *system_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *system_header = create_label_with_markup("<b>System</b>");
    gtk_box_pack_start(GTK_BOX(system_box), system_header, FALSE, FALSE, 0);
    
    widgets->uptime_label = create_label_with_markup("Uptime: --");
    widgets->load_label = create_label_with_markup("Load Average: --");
    
    gtk_box_pack_start(GTK_BOX(system_box), widgets->uptime_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(system_box), widgets->load_label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), system_box, FALSE, FALSE, 5);
    
    // Add main_box to scrolled window
    gtk_container_add(GTK_CONTAINER(scrolled), main_box);
    
    return scrolled;
}

GtkWidget* create_process_tab(AppWidgets *widgets) {
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    
    // Header with filter and refresh button
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *header = create_label_with_markup("<b><big>Process Monitor</big></b>");
    gtk_box_pack_start(GTK_BOX(header_box), header, FALSE, FALSE, 0);
    
    // Filter combo box
    GtkWidget *filter_label = gtk_label_new("Filter:");
    gtk_box_pack_start(GTK_BOX(header_box), filter_label, FALSE, FALSE, 10);
    
    widgets->process_filter_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->process_filter_combo), "All Processes");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->process_filter_combo), "Root Processes");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->process_filter_combo), "User Processes");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->process_filter_combo), 0);
    g_signal_connect(widgets->process_filter_combo, "changed",
                    G_CALLBACK(on_process_filter_changed), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->process_filter_combo, FALSE, FALSE, 0);
    
    // Group by name checkbox
    widgets->process_group_check = gtk_check_button_new_with_label("Group by Name");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->process_group_check), FALSE);
    g_signal_connect(widgets->process_group_check, "toggled",
                    G_CALLBACK(on_process_group_toggled), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->process_group_check, FALSE, FALSE, 10);
    
    // Search entry
    GtkWidget *search_label = gtk_label_new("Search:");
    gtk_box_pack_start(GTK_BOX(header_box), search_label, FALSE, FALSE, 10);
    
    widgets->process_search_entry = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->process_search_entry), "Filter by name or PID...");
    gtk_widget_set_size_request(widgets->process_search_entry, 200, -1);
    g_signal_connect(widgets->process_search_entry, "search-changed",
                    G_CALLBACK(on_process_search_changed), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->process_search_entry, FALSE, FALSE, 0);
    
    // Spacer
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(header_box), spacer, TRUE, TRUE, 0);
    
    widgets->process_refresh_btn = gtk_button_new_with_label("Refresh");
    g_signal_connect(widgets->process_refresh_btn, "clicked", 
                    G_CALLBACK(on_process_refresh_clicked), widgets);
    gtk_box_pack_end(GTK_BOX(header_box), widgets->process_refresh_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // Create scrolled window for tree view
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    // Create list store: PID, Name, CPU%, RAM%, RAM(MB)
    widgets->process_store = gtk_list_store_new(5, 
        G_TYPE_INT,      // PID
        G_TYPE_STRING,   // Name
        G_TYPE_FLOAT,    // CPU%
        G_TYPE_FLOAT,    // RAM%
        G_TYPE_FLOAT);   // RAM(MB)
    
    // Create tree view
    widgets->process_tree = gtk_tree_view_new_with_model(
        GTK_TREE_MODEL(widgets->process_store));
    
    // Create columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // PID column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_fixed_width(column, 70);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->process_tree), column);
    
    // Name column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    column = gtk_tree_view_column_new_with_attributes("Process Name", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->process_tree), column);
    
    // CPU% column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    column = gtk_tree_view_column_new_with_attributes("CPU %", renderer, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 2);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_fixed_width(column, 80);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_cell_data_func(column, renderer, 
        (GtkTreeCellDataFunc)cpu_cell_data_func, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->process_tree), column);
    
    // RAM% column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    column = gtk_tree_view_column_new_with_attributes("RAM %", renderer, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 3);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_fixed_width(column, 80);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_cell_data_func(column, renderer, 
        (GtkTreeCellDataFunc)ram_percent_cell_data_func, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->process_tree), column);
    
    // RAM(MB) column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    column = gtk_tree_view_column_new_with_attributes("RAM (MB)", renderer, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 4);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_fixed_width(column, 100);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_cell_data_func(column, renderer, 
        (GtkTreeCellDataFunc)ram_mb_cell_data_func, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->process_tree), column);
    
    // Enable sorting
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(widgets->process_store), 
                                         2, GTK_SORT_DESCENDING);
    
    gtk_container_add(GTK_CONTAINER(scrolled), widgets->process_tree);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled, TRUE, TRUE, 0);
    
    // Status bar
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    widgets->process_status_label = create_label_with_markup("Loading processes...");
    gtk_box_pack_start(GTK_BOX(main_box), widgets->process_status_label, FALSE, FALSE, 0);
    
    return main_box;
}

GtkWidget* create_network_tab(AppWidgets *widgets) {
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    
    // Header with filter and refresh button
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *header = create_label_with_markup("<b><big>Network Connections</big></b>");
    gtk_box_pack_start(GTK_BOX(header_box), header, FALSE, FALSE, 0);
    
    // Filter combo box
    GtkWidget *filter_label = gtk_label_new("Protocol:");
    gtk_box_pack_start(GTK_BOX(header_box), filter_label, FALSE, FALSE, 10);
    
    widgets->network_filter_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->network_filter_combo), "All");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->network_filter_combo), "TCP");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->network_filter_combo), "UDP");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->network_filter_combo), 0);
    g_signal_connect(widgets->network_filter_combo, "changed",
                    G_CALLBACK(on_network_filter_changed), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->network_filter_combo, FALSE, FALSE, 0);
    
    // Search entry
    GtkWidget *search_label = gtk_label_new("Search:");
    gtk_box_pack_start(GTK_BOX(header_box), search_label, FALSE, FALSE, 10);
    
    widgets->network_search_entry = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->network_search_entry), "Filter by address, port or process...");
    gtk_widget_set_size_request(widgets->network_search_entry, 250, -1);
    g_signal_connect(widgets->network_search_entry, "search-changed",
                    G_CALLBACK(on_network_search_changed), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->network_search_entry, FALSE, FALSE, 0);
    
    // Spacer
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(header_box), spacer, TRUE, TRUE, 0);
    
    widgets->network_refresh_btn = gtk_button_new_with_label("Refresh");
    g_signal_connect(widgets->network_refresh_btn, "clicked", 
                    G_CALLBACK(on_network_refresh_clicked), widgets);
    gtk_box_pack_end(GTK_BOX(header_box), widgets->network_refresh_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // Create scrolled window for tree view
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    // Create list store: Protocol, Port, Local Address, Remote Address, State, PID, Process
    widgets->network_store = gtk_list_store_new(7, 
        G_TYPE_STRING,   // Protocol
        G_TYPE_INT,      // Port
        G_TYPE_STRING,   // Local Address
        G_TYPE_STRING,   // Remote Address
        G_TYPE_STRING,   // State
        G_TYPE_INT,      // PID
        G_TYPE_STRING);  // Process Name
    
    // Create tree view
    widgets->network_tree = gtk_tree_view_new_with_model(
        GTK_TREE_MODEL(widgets->network_store));
    
    // Create columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Protocol column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Protocol", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_fixed_width(column, 80);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->network_tree), column);
    
    // Port column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    column = gtk_tree_view_column_new_with_attributes("Port", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_fixed_width(column, 70);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->network_tree), column);
    
    // Local Address column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Local Address", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 2);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->network_tree), column);
    
    // Remote Address column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Remote Address", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 3);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->network_tree), column);
    
    // State column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("State", renderer, "text", 4, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 4);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_fixed_width(column, 120);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->network_tree), column);
    
    // PID column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 5, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 5);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_fixed_width(column, 70);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->network_tree), column);
    
    // Process Name column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    column = gtk_tree_view_column_new_with_attributes("Process", renderer, "text", 6, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 6);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_min_width(column, 150);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->network_tree), column);
    
    gtk_container_add(GTK_CONTAINER(scrolled), widgets->network_tree);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled, TRUE, TRUE, 0);
    
    // Status bar
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    widgets->network_status_label = create_label_with_markup("Loading connections...");
    gtk_box_pack_start(GTK_BOX(main_box), widgets->network_status_label, FALSE, FALSE, 0);
    
    return main_box;
}

// Splash screen with logo
void show_splash_screen() {
    // Create splash window
    GtkWidget *splash_window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_position(GTK_WINDOW(splash_window), GTK_WIN_POS_CENTER);
    gtk_window_set_decorated(GTK_WINDOW(splash_window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(splash_window), TRUE);
    
    // Set white background using CSS
    GtkCssProvider *css_provider = gtk_css_provider_new();
    const gchar *css = "window { background-color: white; }";
    gtk_css_provider_load_from_data(css_provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider);
    
    // Create container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 30);
    gtk_widget_set_margin_end(vbox, 30);
    gtk_widget_set_margin_top(vbox, 30);
    gtk_widget_set_margin_bottom(vbox, 30);
    
    // Load and scale logo
    GError *error = NULL;
    GdkPixbuf *logo = gdk_pixbuf_new_from_file(LOGO_PATH, &error);
    
    if (logo) {
        // Scale logo to 200x200 for splash
        GdkPixbuf *scaled_logo = gdk_pixbuf_scale_simple(logo, 200, 200, GDK_INTERP_BILINEAR);
        GtkWidget *logo_image = gtk_image_new_from_pixbuf(scaled_logo);
        gtk_box_pack_start(GTK_BOX(vbox), logo_image, FALSE, FALSE, 0);
        g_object_unref(logo);
        g_object_unref(scaled_logo);
    }
    
    // Application name
    GtkWidget *app_name = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(app_name), 
        "<span size='16000' weight='bold'>System Analyser</span>");
    gtk_box_pack_start(GTK_BOX(vbox), app_name, FALSE, FALSE, 0);
    
    // Loading text
    GtkWidget *loading_label = gtk_label_new("Loading...");
    gtk_box_pack_start(GTK_BOX(vbox), loading_label, FALSE, FALSE, 0);
    
    // Add to window
    gtk_container_add(GTK_CONTAINER(splash_window), vbox);
    
    // Show splash
    gtk_widget_show_all(splash_window);
    
    // Process events to display splash
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
    
    // Wait 2 seconds
    g_usleep(2000000);
    
    // Close splash
    gtk_widget_destroy(splash_window);
    
    // Process remaining events
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
}

// Cell data functions for formatting
void cpu_cell_data_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                       GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
    gfloat cpu;
    gtk_tree_model_get(model, iter, 2, &cpu, -1);
    gchar *text = g_strdup_printf("%.1f%%", cpu);
    
    // Color code based on CPU usage
    const char *color = cpu > 80 ? "red" : (cpu > 50 ? "orange" : "black");
    g_object_set(renderer, "text", text, "foreground", color, NULL);
    g_free(text);
}

void ram_percent_cell_data_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                                GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
    gfloat ram;
    gtk_tree_model_get(model, iter, 3, &ram, -1);
    gchar *text = g_strdup_printf("%.1f%%", ram);
    g_object_set(renderer, "text", text, NULL);
    g_free(text);
}

void ram_mb_cell_data_func(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                           GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
    gfloat ram_mb;
    gtk_tree_model_get(model, iter, 4, &ram_mb, -1);
    gchar *text = g_strdup_printf("%.1f", ram_mb);
    g_object_set(renderer, "text", text, NULL);
    g_free(text);
}

void init_gui(int *argc, char ***argv, AppWidgets *widgets) {
    gtk_init(argc, argv);
    
    // Get CPU info once
    cpu_info = get_cpu_info();
    
    // Get screen dimensions
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);
    
    // Calculate window size (60% of screen width, 70% of screen height)
    int window_width = (int)(geometry.width * 0.3);
    int window_height = (int)(geometry.height * 0.7);
    
    // Set minimum size (30% of screen dimensions)
    int min_width = (int)(geometry.width * 0.3);
    int min_height = (int)(geometry.height * 0.4);
    
    // Ensure reasonable bounds
    if (window_width < 400) window_width = 400;
    if (window_height < 500) window_height = 500;
    if (min_width < 300) min_width = 300;
    if (min_height < 400) min_height = 400;
    
    // Initialize filter mode
    widgets->process_filter_mode = 0; // All processes by default
    
    // Create main window
    widgets->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "Raspberry Pi System Analyser");
    
    // Set WM_CLASS for proper taskbar icon support
    gtk_window_set_wmclass(GTK_WINDOW(widgets->window), "system-analyser", "System Analyser");
    
    // Set window icon (taskbar logo) - must be done before showing window
    GError *error = NULL;
    g_print("Loading icon from: %s\n", LOGO_ICON_PATH);
    GdkPixbuf *icon_logo = gdk_pixbuf_new_from_file(LOGO_ICON_PATH, &error);
    if (icon_logo) {
        g_print("Icon loaded successfully: %dx%d\n", 
                gdk_pixbuf_get_width(icon_logo), 
                gdk_pixbuf_get_height(icon_logo));
        
        // Set as default icon for all windows
        gtk_window_set_default_icon(icon_logo);
        
        // Also set for this specific window
        gtk_window_set_icon(GTK_WINDOW(widgets->window), icon_logo);
        
        // Create multiple sizes for better compatibility
        GdkPixbuf *icon_128 = gdk_pixbuf_scale_simple(icon_logo, 128, 128, GDK_INTERP_BILINEAR);
        GdkPixbuf *icon_64 = gdk_pixbuf_scale_simple(icon_logo, 64, 64, GDK_INTERP_BILINEAR);
        GdkPixbuf *icon_48 = gdk_pixbuf_scale_simple(icon_logo, 48, 48, GDK_INTERP_BILINEAR);
        GdkPixbuf *icon_32 = gdk_pixbuf_scale_simple(icon_logo, 32, 32, GDK_INTERP_BILINEAR);
        GdkPixbuf *icon_24 = gdk_pixbuf_scale_simple(icon_logo, 24, 24, GDK_INTERP_BILINEAR);
        GdkPixbuf *icon_16 = gdk_pixbuf_scale_simple(icon_logo, 16, 16, GDK_INTERP_BILINEAR);
        
        GList *icon_list = NULL;
        icon_list = g_list_append(icon_list, icon_128);
        icon_list = g_list_append(icon_list, icon_64);
        icon_list = g_list_append(icon_list, icon_48);
        icon_list = g_list_append(icon_list, icon_32);
        icon_list = g_list_append(icon_list, icon_24);
        icon_list = g_list_append(icon_list, icon_16);
        
        gtk_window_set_icon_list(GTK_WINDOW(widgets->window), icon_list);
        gtk_window_set_default_icon_list(icon_list);
        
        g_list_free(icon_list);
        g_object_unref(icon_logo);
        g_object_unref(icon_128);
        g_object_unref(icon_64);
        g_object_unref(icon_48);
        g_object_unref(icon_32);
        g_object_unref(icon_24);
        g_object_unref(icon_16);
    } else {
        if (error) {
            g_warning("Failed to load icon logo: %s", error->message);
            g_error_free(error);
        }
    }
    
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), window_width, window_height);
    gtk_widget_set_size_request(widgets->window, min_width, min_height);
    gtk_window_set_resizable(GTK_WINDOW(widgets->window), TRUE);
    gtk_window_set_position(GTK_WINDOW(widgets->window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(widgets->window), 10);
    g_signal_connect(widgets->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Add CSS for responsive font scaling
    GtkCssProvider *provider = gtk_css_provider_new();
    const gchar *css = 
        "window { font-size: 10pt; }"
        "label { font-size: 0.9em; }"
        "progressbar { min-height: 20px; font-size: 0.85em; }";
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    
    // Create notebook for tabs
    widgets->notebook = gtk_notebook_new();
    
    // Create Overview tab
    GtkWidget *overview_tab = create_overview_tab(widgets);
    GtkWidget *overview_label = gtk_label_new("Overview");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets->notebook), overview_tab, overview_label);
    
    // Create Process tab
    GtkWidget *process_tab = create_process_tab(widgets);
    GtkWidget *process_label = gtk_label_new("Processes");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets->notebook), process_tab, process_label);
    
    // Create Network tab
    GtkWidget *network_tab = create_network_tab(widgets);
    GtkWidget *network_label = gtk_label_new("Network");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets->notebook), network_tab, network_label);
    
    // Create Stats tab
    GtkWidget *stats_tab = create_stats_tab(widgets);
    GtkWidget *stats_label = gtk_label_new("Usage Statistics");
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets->notebook), stats_tab, stats_label);
    
    // Add notebook to window
    gtk_container_add(GTK_CONTAINER(widgets->window), widgets->notebook);
    
    // Show all widgets
    gtk_widget_show_all(widgets->window);
}

void update_overview_tab(AppWidgets *widgets) {
    // Gather sensor data
    float cpu_temp = get_cpu_temp();
    float cpu_usage = get_cpu_usage();
    unsigned long cpu_clock = get_cpu_clock();
    float core_volt = get_core_voltage();
    
    RamInfo ram = get_ram_info();
    DiskInfo disk = get_disk_info();
    
    LoadAvg load = get_load_avg();
    Uptime uptime = get_uptime();
    
    // Update CPU labels
    gchar *temp_text = g_strdup_printf("Temperature: <span color='%s'><b>%.1f°C</b></span>",
        cpu_temp > 70 ? "red" : (cpu_temp > 60 ? "orange" : "green"), cpu_temp);
    gtk_label_set_markup(GTK_LABEL(widgets->cpu_temp_label), temp_text);
    g_free(temp_text);
    
    gchar *usage_text = g_strdup_printf("Usage: <b>%.1f%%</b>", cpu_usage);
    gtk_label_set_markup(GTK_LABEL(widgets->cpu_usage_label), usage_text);
    g_free(usage_text);
    
    if (cpu_clock > 0) {
        gchar *clock_text = g_strdup_printf("Clock Speed: <b>%lu MHz</b>", cpu_clock);
        gtk_label_set_markup(GTK_LABEL(widgets->cpu_clock_label), clock_text);
        g_free(clock_text);
    }
    
    if (core_volt > 0) {
        gchar *volt_text = g_strdup_printf("Core Voltage: <b>%.4fV</b>", core_volt);
        gtk_label_set_markup(GTK_LABEL(widgets->cpu_voltage_label), volt_text);
        g_free(volt_text);
    }
    
    // Update RAM
    gchar *ram_text = g_strdup_printf("Usage: <b>%lu MB / %lu MB</b> (%.1f%%)", 
        ram.used_mb, ram.total_mb, ram.usage_percent);
    gtk_label_set_markup(GTK_LABEL(widgets->ram_usage_label), ram_text);
    g_free(ram_text);
    
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->ram_bar), ram.usage_percent / 100.0);
    gchar *ram_bar_text = g_strdup_printf("%.1f%%", ram.usage_percent);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets->ram_bar), ram_bar_text);
    g_free(ram_bar_text);
    
    // Update Disk
    gchar *disk_text = g_strdup_printf("Usage: <b>%lu GB / %lu GB</b> (%.1f%%)", 
        disk.used_gb, disk.total_gb, disk.usage_percent);
    gtk_label_set_markup(GTK_LABEL(widgets->disk_usage_label), disk_text);
    g_free(disk_text);
    
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->disk_bar), disk.usage_percent / 100.0);
    gchar *disk_bar_text = g_strdup_printf("%.1f%%", disk.usage_percent);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets->disk_bar), disk_bar_text);
    g_free(disk_bar_text);
    
    // Update System info
    gchar *uptime_text = g_strdup_printf("Uptime: <b>%lud %02luh %02lum %02lus</b>", 
        uptime.days, uptime.hours, uptime.minutes, uptime.seconds);
    gtk_label_set_markup(GTK_LABEL(widgets->uptime_label), uptime_text);
    g_free(uptime_text);
    
    gchar *load_text = g_strdup_printf("Load Average: <b>%.2f, %.2f, %.2f</b> (1m, 5m, 15m)", 
        load.load_1min, load.load_5min, load.load_15min);
    gtk_label_set_markup(GTK_LABEL(widgets->load_label), load_text);
    g_free(load_text);
}

void update_process_tab(AppWidgets *widgets) {
    // Clear existing data
    gtk_list_store_clear(widgets->process_store);
    
    // Get all processes
    ProcessList list = get_process_list(0); // 0 means get all
    
    // Get search text
    const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(widgets->process_search_entry));
    gchar *search_lower = g_utf8_strdown(search_text, -1);
    
    int displayed = 0;
    int total = list.count;
    
    GtkTreeIter iter;
    
    // Check if we should group processes
    if (widgets->process_group_mode == 1) {
        // Group processes by name
        ProcessGroupList group_list = group_processes(&list);
        
        // Populate tree view with groups
        for (int i = 0; i < group_list.count; i++) {
            ProcessGroup *group = &group_list.groups[i];
            
            // Apply filter - check first PID's user
            int uid = 0;
            for (int j = 0; j < list.count; j++) {
                if (list.processes[j].pid == group->pids[0]) {
                    uid = atoi(list.processes[j].user);
                    break;
                }
            }
            if (widgets->process_filter_mode == 1 && uid != 0) continue; // Root only
            if (widgets->process_filter_mode == 2 && uid == 0) continue; // User only
            
            // Apply search filter
            if (strlen(search_lower) > 0) {
                gchar *name_lower = g_utf8_strdown(group->name, -1);
                gchar *pid_str = g_strdup_printf("%d", group->pids[0]);
                
                gboolean match = strstr(name_lower, search_lower) != NULL ||
                               strstr(pid_str, search_lower) != NULL;
                
                g_free(name_lower);
                g_free(pid_str);
                
                if (!match) continue;
            }
            
            // Format name with count
            char display_name[300];
            if (group->process_count > 1) {
                snprintf(display_name, sizeof(display_name), "%s (%d)", 
                        group->name, group->process_count);
            } else {
                snprintf(display_name, sizeof(display_name), "%s", group->name);
            }
            
            // Calculate RAM percentage
            RamInfo ram = get_ram_info();
            float ram_percent = ram.total_mb > 0 ? 
                ((group->total_ram_kb / 1024.0) * 100.0) / ram.total_mb : 0.0;
            
            gtk_list_store_append(widgets->process_store, &iter);
            gtk_list_store_set(widgets->process_store, &iter,
                0, group->pids[0], // Show first PID
                1, display_name,
                2, group->total_cpu_percent,
                3, ram_percent,
                4, (float)group->total_ram_kb / 1024.0, // Convert to MB
                -1);
            displayed++;
        }
        
        free_process_group_list(&group_list);
    } else {
        // Show individual processes
        for (int i = 0; i < list.count; i++) {
            ProcessInfo *proc = &list.processes[i];
            
            // Apply filter
            int uid = atoi(proc->user);
            if (widgets->process_filter_mode == 1 && uid != 0) continue; // Root only
            if (widgets->process_filter_mode == 2 && uid == 0) continue; // User only
            
            // Apply search filter
            if (strlen(search_lower) > 0) {
                gchar *name_lower = g_utf8_strdown(proc->name, -1);
                gchar *pid_str = g_strdup_printf("%d", proc->pid);
                
                gboolean match = strstr(name_lower, search_lower) != NULL ||
                               strstr(pid_str, search_lower) != NULL;
                
                g_free(name_lower);
                g_free(pid_str);
                
                if (!match) continue;
            }
            
            gtk_list_store_append(widgets->process_store, &iter);
            gtk_list_store_set(widgets->process_store, &iter,
                0, proc->pid,
                1, proc->name,
                2, proc->cpu_percent,
                3, proc->ram_percent,
                4, (float)proc->ram_kb / 1024.0, // Convert to MB
                -1);
            displayed++;
        }
    }
    
    // Update status label
    const char *filter_name[] = {"All", "Root", "User"};
    const char *mode_name = widgets->process_group_mode == 1 ? "Grouped" : "Individual";
    gchar *status_text = g_strdup_printf(
        "Displaying: <b>%d</b> / <b>%d</b> total processes | Filter: <b>%s</b> | Mode: <b>%s</b>", 
        displayed, total, filter_name[widgets->process_filter_mode], mode_name);
    gtk_label_set_markup(GTK_LABEL(widgets->process_status_label), status_text);
    g_free(status_text);
    g_free(search_lower);
    
    // Free process list
    free_process_list(&list);
}

void on_process_filter_changed(GtkComboBox *combo, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    widgets->process_filter_mode = gtk_combo_box_get_active(combo);
    update_process_tab(widgets);
}

void on_process_refresh_clicked(GtkButton *button, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    update_process_tab(widgets);
}

void on_process_group_toggled(GtkToggleButton *toggle, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    widgets->process_group_mode = gtk_toggle_button_get_active(toggle) ? 1 : 0;
    update_process_tab(widgets);
}

void update_network_tab(AppWidgets *widgets) {
    // Clear existing data
    gtk_list_store_clear(widgets->network_store);
    
    // Get all network connections
    PortList list = get_port_list();
    
    // Get search text
    const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(widgets->network_search_entry));
    gchar *search_lower = g_utf8_strdown(search_text, -1);
    
    int displayed = 0;
    int total = list.count;
    
    // Populate tree view with filtering
    GtkTreeIter iter;
    for (int i = 0; i < list.count; i++) {
        PortInfo *port = &list.ports[i];
        
        // Apply protocol filter
        if (widgets->network_filter_mode == 1 && strcmp(port->protocol, "TCP") != 0) continue;
        if (widgets->network_filter_mode == 2 && strcmp(port->protocol, "UDP") != 0) continue;
        
        // Apply search filter
        if (strlen(search_lower) > 0) {
            gchar *local_lower = g_utf8_strdown(port->local_addr, -1);
            gchar *remote_lower = g_utf8_strdown(port->remote_addr, -1);
            gchar *process_lower = g_utf8_strdown(port->process_name, -1);
            gchar *port_str = g_strdup_printf("%u", port->local_port);
            
            gboolean match = strstr(local_lower, search_lower) != NULL ||
                           strstr(remote_lower, search_lower) != NULL ||
                           strstr(process_lower, search_lower) != NULL ||
                           strstr(port_str, search_lower) != NULL;
            
            g_free(local_lower);
            g_free(remote_lower);
            g_free(process_lower);
            g_free(port_str);
            
            if (!match) continue;
        }
        
        gtk_list_store_append(widgets->network_store, &iter);
        gtk_list_store_set(widgets->network_store, &iter,
            0, port->protocol,
            1, port->local_port,
            2, port->local_addr,
            3, port->remote_addr,
            4, port->state,
            5, port->pid,
            6, port->process_name,
            -1);
        displayed++;
    }
    
    g_free(search_lower);
    
    // Update status label
    const char *filter_name[] = {"All", "TCP", "UDP"};
    gchar *status_text = g_strdup_printf(
        "Displaying: <b>%d</b> / <b>%d</b> total connections | Filter: <b>%s</b>", 
        displayed, total, filter_name[widgets->network_filter_mode]);
    gtk_label_set_markup(GTK_LABEL(widgets->network_status_label), status_text);
    g_free(status_text);
    
    // Free port list
    free_port_list(&list);
}

void on_network_filter_changed(GtkComboBox *combo, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    widgets->network_filter_mode = gtk_combo_box_get_active(combo);
    update_network_tab(widgets);
}

void on_network_refresh_clicked(GtkButton *button, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    update_network_tab(widgets);
}

void on_process_search_changed(GtkSearchEntry *entry, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    update_process_tab(widgets);
}

void on_network_search_changed(GtkSearchEntry *entry, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    update_network_tab(widgets);
}

gboolean update_timer_callback(gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    static int counter = 0;
    
    // Update overview tab every second
    update_overview_tab(widgets);
    
    // Update process tab every 2 seconds
    if (counter % 2 == 0) {
        update_process_tab(widgets);
    }
    
    // Update network tab every 3 seconds
    if (counter % 3 == 0) {
        update_network_tab(widgets);
    }
    
    // Update stats tab every 10 seconds
    if (counter % 10 == 0) {
        update_stats_tab(widgets);
    }
    
    // Log data
    log_system_snapshot();
    
    counter++;
    
    // Continue timer
    return TRUE;
}

// Stats tab drawing callback - Line Graph with tooltips
gboolean stats_drawing_area_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    
    if (!widgets || widgets->stats_report.count == 0) {
        // Draw "No data" message
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 14);
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_move_to(cr, 20, 30);
        cairo_show_text(cr, "No data available. Waiting for log data...");
        return TRUE;
    }
    
    ProcessStatsReport *report = &widgets->stats_report;
    
    // Get drawing area size
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    
    // Margins
    int margin_left = 70;
    int margin_right = 30;
    int margin_top = 50;
    int margin_bottom = 60;
    
    int chart_width = width - margin_left - margin_right;
    int chart_height = height - margin_top - margin_bottom;
    
    if (chart_width < 100 || chart_height < 100) return TRUE;
    
    // Background
    cairo_set_source_rgb(cr, 0.98, 0.98, 0.98);
    cairo_paint(cr);
    
    // Find max value for scaling
    double max_cpu = 0, max_ram = 0;
    for (int i = 0; i < report->count; i++) {
        if (report->stats[i].avg_cpu_percent > max_cpu)
            max_cpu = report->stats[i].avg_cpu_percent;
        if (report->stats[i].avg_ram_mb > max_ram)
            max_ram = report->stats[i].avg_ram_mb;
    }
    
    if (max_cpu < 10) max_cpu = 10;
    if (max_ram < 10) max_ram = 10;
    max_cpu *= 1.15; // Add 15% padding
    max_ram *= 1.15;
    
    // Draw chart background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, margin_left, margin_top, chart_width, chart_height);
    cairo_fill(cr);
    
    // Enable anti-aliasing for smoother lines
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
    
    // Draw grid with better styling
    cairo_set_line_width(cr, 0.8);
    cairo_set_source_rgba(cr, 0.85, 0.85, 0.85, 0.6);
    
    // Horizontal grid lines
    for (int i = 0; i <= 5; i++) {
        int y = margin_top + (chart_height * i / 5);
        cairo_move_to(cr, margin_left, y);
        cairo_line_to(cr, margin_left + chart_width, y);
        cairo_stroke(cr);
    }
    
    // Vertical grid lines (fewer lines for cleaner look)
    int v_lines = (report->count > 10) ? 10 : report->count;
    for (int i = 0; i <= v_lines; i++) {
        int x = margin_left + (chart_width * i / v_lines);
        cairo_move_to(cr, x, margin_top);
        cairo_line_to(cr, x, margin_top + chart_height);
        cairo_stroke(cr);
    }
    
    // Draw axes
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_set_line_width(cr, 2);
    
    // Y axis
    cairo_move_to(cr, margin_left, margin_top);
    cairo_line_to(cr, margin_left, margin_top + chart_height);
    cairo_stroke(cr);
    
    // X axis
    cairo_move_to(cr, margin_left, margin_top + chart_height);
    cairo_line_to(cr, margin_left + chart_width, margin_top + chart_height);
    cairo_stroke(cr);
    
    // Y axis labels (CPU - left side) with better formatting
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10);
    cairo_set_source_rgb(cr, 0.2, 0.6, 0.95);
    
    for (int i = 0; i <= 5; i++) {
        double value = (max_cpu / 5.0) * (5 - i);
        int y = margin_top + (chart_height * i / 5);
        char label[32];
        
        // Format with appropriate precision
        if (value >= 10) {
            snprintf(label, sizeof(label), "%.0f%%", value);
        } else {
            snprintf(label, sizeof(label), "%.1f%%", value);
        }
        
        // Draw with background for better readability
        cairo_text_extents_t extents;
        cairo_text_extents(cr, label, &extents);
        
        // Background box
        cairo_set_source_rgba(cr, 1, 1, 1, 0.8);
        cairo_rectangle(cr, 3, y - extents.height - 1, extents.width + 4, extents.height + 4);
        cairo_fill(cr);
        
        // Text
        cairo_set_source_rgb(cr, 0.2, 0.6, 0.95);
        cairo_move_to(cr, 5, y + 3);
        cairo_show_text(cr, label);
    }
    
    // Y axis labels (RAM - right side) with smart units
    cairo_set_source_rgb(cr, 0.3, 0.85, 0.45);
    for (int i = 0; i <= 5; i++) {
        double value = (max_ram / 5.0) * (5 - i);
        int y = margin_top + (chart_height * i / 5);
        char label[32];
        
        // Smart unit conversion (MB to GB if large)
        if (value >= 1024) {
            snprintf(label, sizeof(label), "%.1f GB", value / 1024.0);
        } else if (value >= 10) {
            snprintf(label, sizeof(label), "%.0f MB", value);
        } else {
            snprintf(label, sizeof(label), "%.1f MB", value);
        }
        
        // Draw with background
        cairo_text_extents_t extents;
        cairo_text_extents(cr, label, &extents);
        
        cairo_set_source_rgba(cr, 1, 1, 1, 0.8);
        cairo_rectangle(cr, margin_left + chart_width + 3, y - extents.height - 1, 
                       extents.width + 4, extents.height + 4);
        cairo_fill(cr);
        
        cairo_set_source_rgb(cr, 0.3, 0.85, 0.45);
        cairo_move_to(cr, margin_left + chart_width + 5, y + 3);
        cairo_show_text(cr, label);
    }
    
    // Draw CPU area fill with gradient (if enabled)
    if (widgets->stats_show_cpu) {
        cairo_pattern_t *cpu_gradient = cairo_pattern_create_linear(
            margin_left, margin_top, 
            margin_left, margin_top + chart_height);
        cairo_pattern_add_color_stop_rgba(cpu_gradient, 0, 0.3, 0.7, 1.0, 0.3);
        cairo_pattern_add_color_stop_rgba(cpu_gradient, 1, 0.3, 0.7, 1.0, 0.05);
        
        cairo_move_to(cr, margin_left, margin_top + chart_height);
        for (int i = 0; i < report->count; i++) {
            double cpu = report->stats[i].avg_cpu_percent;
            int x = margin_left + (chart_width * i / (report->count - 1));
            int y = margin_top + chart_height - (int)((cpu / max_cpu) * chart_height);
            
            if (i == 0) {
                cairo_line_to(cr, x, y);
            } else {
                // Use smooth curve interpolation
                cairo_line_to(cr, x, y);
            }
        }
        cairo_line_to(cr, margin_left + chart_width, margin_top + chart_height);
        cairo_close_path(cr);
        cairo_set_source(cr, cpu_gradient);
        cairo_fill_preserve(cr);
        cairo_pattern_destroy(cpu_gradient);
        
        // Draw CPU line on top
        cairo_set_line_width(cr, 3.0);
        cairo_set_source_rgb(cr, 0.2, 0.6, 0.95);
        cairo_new_path(cr);
        for (int i = 0; i < report->count; i++) {
            double cpu = report->stats[i].avg_cpu_percent;
            int x = margin_left + (chart_width * i / (report->count - 1));
            int y = margin_top + chart_height - (int)((cpu / max_cpu) * chart_height);
            
            if (i == 0) {
                cairo_move_to(cr, x, y);
            } else {
                cairo_line_to(cr, x, y);
            }
        }
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        cairo_stroke(cr);
    }
    
    // Draw RAM area fill with gradient (if enabled)
    if (widgets->stats_show_ram) {
        cairo_pattern_t *ram_gradient = cairo_pattern_create_linear(
            margin_left, margin_top, 
            margin_left, margin_top + chart_height);
        cairo_pattern_add_color_stop_rgba(ram_gradient, 0, 0.4, 0.9, 0.5, 0.3);
        cairo_pattern_add_color_stop_rgba(ram_gradient, 1, 0.4, 0.9, 0.5, 0.05);
        
        cairo_move_to(cr, margin_left, margin_top + chart_height);
        for (int i = 0; i < report->count; i++) {
            double ram = report->stats[i].avg_ram_mb;
            int x = margin_left + (chart_width * i / (report->count - 1));
            int y = margin_top + chart_height - (int)((ram / max_ram) * chart_height);
            
            if (i == 0) {
                cairo_line_to(cr, x, y);
            } else {
                cairo_line_to(cr, x, y);
            }
        }
        cairo_line_to(cr, margin_left + chart_width, margin_top + chart_height);
        cairo_close_path(cr);
        cairo_set_source(cr, ram_gradient);
        cairo_fill_preserve(cr);
        cairo_pattern_destroy(ram_gradient);
        
        // Draw RAM line on top
        cairo_set_line_width(cr, 3.0);
        cairo_set_source_rgb(cr, 0.3, 0.85, 0.45);
        cairo_new_path(cr);
        for (int i = 0; i < report->count; i++) {
            double ram = report->stats[i].avg_ram_mb;
            int x = margin_left + (chart_width * i / (report->count - 1));
            int y = margin_top + chart_height - (int)((ram / max_ram) * chart_height);
            
            if (i == 0) {
                cairo_move_to(cr, x, y);
            } else {
                cairo_line_to(cr, x, y);
            }
        }
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        cairo_stroke(cr);
    }
    
    // Draw data points and check for hover
    int hover_idx = -1;
    double hover_dist = 15.0; // Hover radius
    
    for (int i = 0; i < report->count; i++) {
        double cpu = report->stats[i].avg_cpu_percent;
        double ram = report->stats[i].avg_ram_mb;
        
        int x = margin_left + (chart_width * i / (report->count - 1));
        int y_cpu = margin_top + chart_height - (int)((cpu / max_cpu) * chart_height);
        int y_ram = margin_top + chart_height - (int)((ram / max_ram) * chart_height);
        
        // Check if mouse is near this point
        double dx_cpu = widgets->stats_mouse_x - x;
        double dy_cpu = widgets->stats_mouse_y - y_cpu;
        double dx_ram = widgets->stats_mouse_x - x;
        double dy_ram = widgets->stats_mouse_y - y_ram;
        
        double dist_cpu = sqrt(dx_cpu*dx_cpu + dy_cpu*dy_cpu);
        double dist_ram = sqrt(dx_ram*dx_ram + dy_ram*dy_ram);
        
        if (dist_cpu < hover_dist || dist_ram < hover_dist) {
            hover_idx = i;
        }
        
        // Draw CPU point with glow effect
        gboolean is_hovered = (i == hover_idx);
        double point_size = is_hovered ? 6 : 4;
        
        // Outer glow for CPU point
        cairo_set_source_rgba(cr, 0.2, 0.6, 0.95, 0.3);
        cairo_arc(cr, x, y_cpu, point_size + 2, 0, 2 * M_PI);
        cairo_fill(cr);
        
        // Main CPU point
        cairo_set_source_rgb(cr, 0.2, 0.6, 0.95);
        cairo_arc(cr, x, y_cpu, point_size, 0, 2 * M_PI);
        cairo_fill(cr);
        
        // White center
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_arc(cr, x, y_cpu, point_size - 2, 0, 2 * M_PI);
        cairo_fill(cr);
        
        // Outer glow for RAM point
        cairo_set_source_rgba(cr, 0.3, 0.85, 0.45, 0.3);
        cairo_arc(cr, x, y_ram, point_size + 2, 0, 2 * M_PI);
        cairo_fill(cr);
        
        // Main RAM point
        cairo_set_source_rgb(cr, 0.3, 0.85, 0.45);
        cairo_arc(cr, x, y_ram, point_size, 0, 2 * M_PI);
        cairo_fill(cr);
        
        // White center
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_arc(cr, x, y_ram, point_size - 2, 0, 2 * M_PI);
        cairo_fill(cr);
    }
    
    // Draw tooltip if hovering
    if (hover_idx >= 0) {
        ProcessUsageStats *stat = &report->stats[hover_idx];
        
        int x = margin_left + (chart_width * hover_idx / (report->count - 1));
        int y_cpu = margin_top + chart_height - (int)((stat->avg_cpu_percent / max_cpu) * chart_height);
        int y_ram = margin_top + chart_height - (int)((stat->avg_ram_mb / max_ram) * chart_height);
        
        // Draw vertical highlight line
        cairo_set_source_rgba(cr, 0.9, 0.6, 0.2, 0.4);
        cairo_set_line_width(cr, 1.5);
        cairo_move_to(cr, x, margin_top);
        cairo_line_to(cr, x, margin_top + chart_height);
        cairo_stroke(cr);
        
        // Highlight hovered points with glow
        cairo_set_source_rgba(cr, 1, 0.8, 0, 0.4);
        cairo_arc(cr, x, y_cpu, 10, 0, 2 * M_PI);
        cairo_fill(cr);
        cairo_arc(cr, x, y_ram, 10, 0, 2 * M_PI);
        cairo_fill(cr);
        
        // Calculate trend if we have previous data
        double cpu_trend = 0, ram_trend = 0;
        if (hover_idx > 0) {
            cpu_trend = stat->avg_cpu_percent - report->stats[hover_idx - 1].avg_cpu_percent;
            ram_trend = stat->avg_ram_mb - report->stats[hover_idx - 1].avg_ram_mb;
        }
        
        // Modern tooltip with rounded corners
        int tooltip_w = 220;
        int tooltip_h = 115;
        int tooltip_x = x + 20;
        int tooltip_y = y_cpu - tooltip_h / 2;
        int corner_radius = 8;
        
        // Keep tooltip in bounds
        if (tooltip_x + tooltip_w > width - 10) tooltip_x = x - tooltip_w - 20;
        if (tooltip_y < 10) tooltip_y = 10;
        if (tooltip_y + tooltip_h > height - 10) tooltip_y = height - tooltip_h - 10;
        
        // Draw shadow with blur effect (multiple layers)
        for (int blur = 4; blur > 0; blur--) {
            cairo_set_source_rgba(cr, 0, 0, 0, 0.05);
            cairo_new_path(cr);
            cairo_arc(cr, tooltip_x + corner_radius + blur, tooltip_y + blur, corner_radius, M_PI, 3 * M_PI / 2);
            cairo_arc(cr, tooltip_x + tooltip_w - corner_radius + blur, tooltip_y + blur, corner_radius, 3 * M_PI / 2, 2 * M_PI);
            cairo_arc(cr, tooltip_x + tooltip_w - corner_radius + blur, tooltip_y + tooltip_h - corner_radius + blur, corner_radius, 0, M_PI / 2);
            cairo_arc(cr, tooltip_x + corner_radius + blur, tooltip_y + tooltip_h - corner_radius + blur, corner_radius, M_PI / 2, M_PI);
            cairo_close_path(cr);
            cairo_fill(cr);
        }
        
        // Draw tooltip box with rounded corners
        cairo_new_path(cr);
        cairo_arc(cr, tooltip_x + corner_radius, tooltip_y, corner_radius, M_PI, 3 * M_PI / 2);
        cairo_arc(cr, tooltip_x + tooltip_w - corner_radius, tooltip_y, corner_radius, 3 * M_PI / 2, 2 * M_PI);
        cairo_arc(cr, tooltip_x + tooltip_w - corner_radius, tooltip_y + tooltip_h - corner_radius, corner_radius, 0, M_PI / 2);
        cairo_arc(cr, tooltip_x + corner_radius, tooltip_y + tooltip_h - corner_radius, corner_radius, M_PI / 2, M_PI);
        cairo_close_path(cr);
        
        // Gradient background
        cairo_pattern_t *tooltip_gradient = cairo_pattern_create_linear(
            tooltip_x, tooltip_y, tooltip_x, tooltip_y + tooltip_h);
        cairo_pattern_add_color_stop_rgb(tooltip_gradient, 0, 1.0, 1.0, 1.0);
        cairo_pattern_add_color_stop_rgb(tooltip_gradient, 1, 0.96, 0.96, 0.98);
        cairo_set_source(cr, tooltip_gradient);
        cairo_fill_preserve(cr);
        cairo_pattern_destroy(tooltip_gradient);
        
        // Border
        cairo_set_source_rgba(cr, 0.5, 0.5, 0.6, 0.8);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        
        // Draw tooltip text
        int text_y = tooltip_y + 22;
        
        // Process name with icon
        cairo_set_source_rgb(cr, 0.15, 0.15, 0.2);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 11);
        cairo_move_to(cr, tooltip_x + 12, text_y);
        char name[30];
        strncpy(name, stat->process_name, 25);
        name[25] = '\0';
        cairo_show_text(cr, name);
        
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 9);
        
        // Separator line
        text_y += 10;
        cairo_set_source_rgba(cr, 0.7, 0.7, 0.75, 0.5);
        cairo_set_line_width(cr, 1);
        cairo_move_to(cr, tooltip_x + 12, text_y);
        cairo_line_to(cr, tooltip_x + tooltip_w - 12, text_y);
        cairo_stroke(cr);
        
        text_y += 18;
        
        // CPU info with trend indicator
        cairo_set_source_rgb(cr, 0.2, 0.6, 0.95);
        cairo_move_to(cr, tooltip_x + 12, text_y);
        char cpu_text[80];
        const char *cpu_arrow = (cpu_trend > 0.1) ? "↑" : (cpu_trend < -0.1) ? "↓" : "→";
        snprintf(cpu_text, sizeof(cpu_text), "● CPU: %.2f%% %s", stat->avg_cpu_percent, cpu_arrow);
        cairo_show_text(cr, cpu_text);
        
        // Trend percentage
        if (fabs(cpu_trend) > 0.01) {
            text_y += 14;
            cairo_set_source_rgba(cr, 0.2, 0.6, 0.95, 0.7);
            cairo_set_font_size(cr, 8);
            cairo_move_to(cr, tooltip_x + 22, text_y);
            char trend_text[40];
            snprintf(trend_text, sizeof(trend_text), "(%+.2f%% from prev)", cpu_trend);
            cairo_show_text(cr, trend_text);
            cairo_set_font_size(cr, 9);
        } else {
            text_y += 6;
        }
        
        text_y += 14;
        
        // RAM info with trend indicator
        cairo_set_source_rgb(cr, 0.3, 0.85, 0.45);
        cairo_move_to(cr, tooltip_x + 12, text_y);
        char ram_text[80];
        const char *ram_arrow = (ram_trend > 1) ? "↑" : (ram_trend < -1) ? "↓" : "→";
        snprintf(ram_text, sizeof(ram_text), "● RAM: %.2f MB %s", stat->avg_ram_mb, ram_arrow);
        cairo_show_text(cr, ram_text);
        
        // Trend info
        if (fabs(ram_trend) > 0.1) {
            text_y += 14;
            cairo_set_source_rgba(cr, 0.3, 0.85, 0.45, 0.7);
            cairo_set_font_size(cr, 8);
            cairo_move_to(cr, tooltip_x + 22, text_y);
            char ram_trend_text[40];
            snprintf(ram_trend_text, sizeof(ram_trend_text), "(%+.2f MB from prev)", ram_trend);
            cairo_show_text(cr, ram_trend_text);
        }
    }
    
    // Title with zoom indicator
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 15);
    cairo_move_to(cr, margin_left, 25);
    
    const char *time_names[] = {"Last Minute", "Last 24 Hours", "Last Week", "Last Month", "Last Year"};
    char title[128];
    snprintf(title, sizeof(title), "Process Resource Usage - %s", time_names[report->time_range]);
    cairo_show_text(cr, title);
    
    // Zoom indicator
    if (widgets->stats_zoom_level > 1.01) {
        cairo_set_font_size(cr, 10);
        cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.8);
        cairo_move_to(cr, margin_left, 40);
        char zoom_text[64];
        snprintf(zoom_text, sizeof(zoom_text), "🔍 Zoom: %.1fx (Scroll to zoom, drag to pan)", widgets->stats_zoom_level);
        cairo_show_text(cr, zoom_text);
    }
    
    // Enhanced Legend with box
    int legend_x = width - 170;
    int legend_y = 10;
    int legend_w = 160;
    int legend_h = 55;
    
    // Legend background box
    cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
    cairo_rectangle(cr, legend_x - 5, legend_y, legend_w, legend_h);
    cairo_fill(cr);
    
    cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.5);
    cairo_set_line_width(cr, 1);
    cairo_rectangle(cr, legend_x - 5, legend_y, legend_w, legend_h);
    cairo_stroke(cr);
    
    legend_x += 5;
    legend_y += 18;
    
    // CPU legend
    cairo_set_source_rgb(cr, 0.2, 0.6, 0.95);
    cairo_set_line_width(cr, 3);
    cairo_move_to(cr, legend_x, legend_y);
    cairo_line_to(cr, legend_x + 25, legend_y);
    cairo_stroke(cr);
    cairo_arc(cr, legend_x + 12, legend_y, 4, 0, 2 * M_PI);
    cairo_fill(cr);
    
    cairo_set_font_size(cr, 11);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, legend_x + 32, legend_y + 5);
    cairo_show_text(cr, "CPU Usage (%)");
    
    // RAM legend
    legend_y += 20;
    cairo_set_source_rgb(cr, 0.3, 0.85, 0.45);
    cairo_set_line_width(cr, 3);
    cairo_move_to(cr, legend_x, legend_y);
    cairo_line_to(cr, legend_x + 25, legend_y);
    cairo_stroke(cr);
    cairo_arc(cr, legend_x + 12, legend_y, 4, 0, 2 * M_PI);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, legend_x + 32, legend_y + 5);
    cairo_show_text(cr, "RAM Usage (MB)");
    
    // Instructions at bottom
    if (widgets->stats_zoom_level <= 1.01) {
        cairo_set_font_size(cr, 9);
        cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.7);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
        int inst_y = margin_top + chart_height + 45;
        cairo_move_to(cr, margin_left, inst_y);
        cairo_show_text(cr, "💡 Tip: Use mouse wheel to zoom in/out • Hover over points for details");
    }
    
    return TRUE;
}

// Mouse motion handler for stats drawing area
gboolean stats_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    
    // Handle panning
    if (widgets->stats_is_panning) {
        double delta_x = event->x - widgets->stats_pan_start_x;
        widgets->stats_pan_offset_x += delta_x / widgets->stats_zoom_level;
        widgets->stats_pan_start_x = event->x;
        
        // Clamp pan offset
        int width = gtk_widget_get_allocated_width(widget);
        double max_offset = (widgets->stats_zoom_level - 1.0) * width / 2.0;
        if (widgets->stats_pan_offset_x > max_offset) widgets->stats_pan_offset_x = max_offset;
        if (widgets->stats_pan_offset_x < -max_offset) widgets->stats_pan_offset_x = -max_offset;
    }
    
    widgets->stats_mouse_x = event->x;
    widgets->stats_mouse_y = event->y;
    
    // Trigger redraw
    gtk_widget_queue_draw(widget);
    
    return TRUE;
}

// Mouse scroll handler for zoom
gboolean stats_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    
    double zoom_factor = 1.1;
    
    if (event->direction == GDK_SCROLL_UP) {
        // Zoom in
        widgets->stats_zoom_level *= zoom_factor;
        if (widgets->stats_zoom_level > 5.0) widgets->stats_zoom_level = 5.0; // Max 5x zoom
    } else if (event->direction == GDK_SCROLL_DOWN) {
        // Zoom out
        widgets->stats_zoom_level /= zoom_factor;
        if (widgets->stats_zoom_level < 1.0) {
            widgets->stats_zoom_level = 1.0;
            widgets->stats_pan_offset_x = 0; // Reset pan when fully zoomed out
        }
    }
    
    gtk_widget_queue_draw(widget);
    return TRUE;
}

// Mouse button press handler for panning
gboolean stats_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    
    if (event->button == 1 && widgets->stats_zoom_level > 1.0) {  // Left click when zoomed
        widgets->stats_is_panning = TRUE;
        widgets->stats_pan_start_x = event->x;
        return TRUE;
    }
    
    return FALSE;
}

// Mouse button release handler
gboolean stats_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    
    if (event->button == 1) {
        widgets->stats_is_panning = FALSE;
        return TRUE;
    }
    
    return FALSE;
}

GtkWidget* create_stats_tab(AppWidgets *widgets) {
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    
    // Header with controls
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *header = create_label_with_markup("<b><big>Process Usage Statistics</big></b>");
    gtk_box_pack_start(GTK_BOX(header_box), header, FALSE, FALSE, 0);
    
    // Time range combo
    GtkWidget *time_label = gtk_label_new("Time Range:");
    gtk_box_pack_start(GTK_BOX(header_box), time_label, FALSE, FALSE, 10);
    
    widgets->stats_time_range_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_time_range_combo), "Last Minute");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_time_range_combo), "Last 24 Hours");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_time_range_combo), "Last Week");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_time_range_combo), "Last Month");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_time_range_combo), "Last Year");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->stats_time_range_combo), 1); // Default: 24 hours
    g_signal_connect(widgets->stats_time_range_combo, "changed",
                    G_CALLBACK(on_stats_time_range_changed), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->stats_time_range_combo, FALSE, FALSE, 0);
    
    // Top N combo
    GtkWidget *top_label = gtk_label_new("Top:");
    gtk_box_pack_start(GTK_BOX(header_box), top_label, FALSE, FALSE, 10);
    
    widgets->stats_top_n_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_top_n_combo), "5");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_top_n_combo), "10");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_top_n_combo), "20");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->stats_top_n_combo), 1); // Default: 10
    g_signal_connect(widgets->stats_top_n_combo, "changed",
                    G_CALLBACK(on_stats_top_n_changed), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->stats_top_n_combo, FALSE, FALSE, 0);
    
    // Sort combo
    GtkWidget *sort_label = gtk_label_new("Sort by:");
    gtk_box_pack_start(GTK_BOX(header_box), sort_label, FALSE, FALSE, 10);
    
    widgets->stats_sort_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_sort_combo), "CPU");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->stats_sort_combo), "RAM");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->stats_sort_combo), 0); // Default: CPU
    g_signal_connect(widgets->stats_sort_combo, "changed",
                    G_CALLBACK(on_stats_sort_changed), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->stats_sort_combo, FALSE, FALSE, 0);
    
    // Separator
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(header_box), separator, FALSE, FALSE, 10);
    
    // Metric toggles
    GtkWidget *show_label = gtk_label_new("Show:");
    gtk_box_pack_start(GTK_BOX(header_box), show_label, FALSE, FALSE, 5);
    
    widgets->stats_show_cpu_check = gtk_check_button_new_with_label("CPU");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->stats_show_cpu_check), TRUE);
    g_signal_connect(widgets->stats_show_cpu_check, "toggled",
                    G_CALLBACK(on_stats_metric_toggled), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->stats_show_cpu_check, FALSE, FALSE, 0);
    
    widgets->stats_show_ram_check = gtk_check_button_new_with_label("RAM");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->stats_show_ram_check), TRUE);
    g_signal_connect(widgets->stats_show_ram_check, "toggled",
                    G_CALLBACK(on_stats_metric_toggled), widgets);
    gtk_box_pack_start(GTK_BOX(header_box), widgets->stats_show_ram_check, FALSE, FALSE, 0);
    
    // Spacer
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(header_box), spacer, TRUE, TRUE, 0);
    
    // Export button
    widgets->stats_export_btn = gtk_button_new_with_label("📊 Export");
    g_signal_connect(widgets->stats_export_btn, "clicked", 
                    G_CALLBACK(on_stats_export_clicked), widgets);
    gtk_box_pack_end(GTK_BOX(header_box), widgets->stats_export_btn, FALSE, FALSE, 0);
    
    // Refresh button
    widgets->stats_refresh_btn = gtk_button_new_with_label("🔄 Refresh");
    g_signal_connect(widgets->stats_refresh_btn, "clicked", 
                    G_CALLBACK(on_stats_refresh_clicked), widgets);
    gtk_box_pack_end(GTK_BOX(header_box), widgets->stats_refresh_btn, FALSE, FALSE, 5);
    
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    
    // Content area with paned widget (splitter)
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(paned), 250);
    
    // Left side: Process list
    GtkWidget *list_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(list_scrolled, 200, -1);
    
    // Create list store: Process, Avg CPU%, Avg RAM (MB)
    widgets->stats_list_store = gtk_list_store_new(3,
        G_TYPE_STRING,   // Process name
        G_TYPE_DOUBLE,   // Avg CPU %
        G_TYPE_DOUBLE);  // Avg RAM MB
    
    widgets->stats_list_tree = gtk_tree_view_new_with_model(
        GTK_TREE_MODEL(widgets->stats_list_store));
    
    // Create columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Process name column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    column = gtk_tree_view_column_new_with_attributes("Process", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->stats_list_tree), column);
    
    // CPU column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    column = gtk_tree_view_column_new_with_attributes("Avg CPU %", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->stats_list_tree), column);
    
    // RAM column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    column = gtk_tree_view_column_new_with_attributes("Avg RAM (MB)", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->stats_list_tree), column);
    
    gtk_container_add(GTK_CONTAINER(list_scrolled), widgets->stats_list_tree);
    gtk_paned_pack1(GTK_PANED(paned), list_scrolled, FALSE, TRUE);
    
    // Right side: Drawing area
    widgets->stats_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(widgets->stats_drawing_area, 400, 300);
    
    // Enable mouse events for zoom and pan
    gtk_widget_add_events(widgets->stats_drawing_area, 
                         GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | 
                         GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK);
    
    g_signal_connect(widgets->stats_drawing_area, "draw",
                    G_CALLBACK(stats_drawing_area_draw), widgets);
    g_signal_connect(widgets->stats_drawing_area, "motion-notify-event",
                    G_CALLBACK(stats_motion_notify_event), widgets);
    g_signal_connect(widgets->stats_drawing_area, "scroll-event",
                    G_CALLBACK(stats_scroll_event), widgets);
    g_signal_connect(widgets->stats_drawing_area, "button-press-event",
                    G_CALLBACK(stats_button_press_event), widgets);
    g_signal_connect(widgets->stats_drawing_area, "button-release-event",
                    G_CALLBACK(stats_button_release_event), widgets);
    
    GtkWidget *chart_frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(chart_frame), widgets->stats_drawing_area);
    gtk_paned_pack2(GTK_PANED(paned), chart_frame, TRUE, TRUE);
    
    gtk_box_pack_start(GTK_BOX(main_box), paned, TRUE, TRUE, 0);
    
    // Initialize defaults
    widgets->stats_time_range = 1; // 24 hours
    widgets->stats_top_n = 10;
    widgets->stats_sort_mode = 0; // CPU
    widgets->stats_show_cpu = TRUE;
    widgets->stats_show_ram = TRUE;
    widgets->stats_report.stats = NULL;
    widgets->stats_report.count = 0;
    widgets->stats_mouse_x = -1;
    widgets->stats_mouse_y = -1;
    widgets->stats_hover_index = -1;
    
    // Initialize zoom and pan
    widgets->stats_zoom_level = 1.0;
    widgets->stats_pan_offset_x = 0;
    widgets->stats_is_panning = FALSE;
    widgets->stats_pan_start_x = 0;
    
    return main_box;
}

void update_stats_tab(AppWidgets *widgets) {
    // Free previous report
    if (widgets->stats_report.stats) {
        free_process_stats_report(&widgets->stats_report);
    }
    
    // Get new stats from logs
    widgets->stats_report = get_process_stats_from_logs(
        widgets->csv_file_path,
        widgets->stats_time_range,
        widgets->stats_top_n,
        widgets->stats_sort_mode
    );
    
    // Update list view
    gtk_list_store_clear(widgets->stats_list_store);
    
    GtkTreeIter iter;
    for (int i = 0; i < widgets->stats_report.count; i++) {
        ProcessUsageStats *stat = &widgets->stats_report.stats[i];
        
        gtk_list_store_append(widgets->stats_list_store, &iter);
        gtk_list_store_set(widgets->stats_list_store, &iter,
            0, stat->process_name,
            1, stat->avg_cpu_percent,
            2, stat->avg_ram_mb,
            -1);
    }
    
    // Redraw chart
    gtk_widget_queue_draw(widgets->stats_drawing_area);
}

void on_stats_refresh_clicked(GtkButton *button, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    update_stats_tab(widgets);
}

void on_stats_time_range_changed(GtkComboBox *combo, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    widgets->stats_time_range = gtk_combo_box_get_active(combo);
    update_stats_tab(widgets);
}

void on_stats_top_n_changed(GtkComboBox *combo, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    int selection = gtk_combo_box_get_active(combo);
    int values[] = {5, 10, 20};
    widgets->stats_top_n = values[selection];
    update_stats_tab(widgets);
}

void on_stats_sort_changed(GtkComboBox *combo, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    widgets->stats_sort_mode = gtk_combo_box_get_active(combo);
    update_stats_tab(widgets);
}

void on_stats_metric_toggled(GtkToggleButton *toggle, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    widgets->stats_show_cpu = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(widgets->stats_show_cpu_check));
    widgets->stats_show_ram = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(widgets->stats_show_ram_check));
    
    // At least one metric must be shown
    if (!widgets->stats_show_cpu && !widgets->stats_show_ram) {
        gtk_toggle_button_set_active(toggle, TRUE);
        return;
    }
    
    gtk_widget_queue_draw(widgets->stats_drawing_area);
}

void on_stats_export_clicked(GtkButton *button, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    
    if (widgets->stats_report.count == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(widgets->window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "No data available to export.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Create export dialog with options
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Export Statistics",
        GTK_WINDOW(widgets->window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Export", GTK_RESPONSE_ACCEPT,
        NULL);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content), 15);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(content), vbox);
    
    // Format selection
    GtkWidget *format_label = gtk_label_new("Export Format:");
    gtk_widget_set_halign(format_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), format_label, FALSE, FALSE, 0);
    
    GtkWidget *csv_radio = gtk_radio_button_new_with_label(NULL, "CSV (Data only)");
    GtkWidget *png_radio = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(csv_radio), "PNG (Chart image)");
    
    gtk_box_pack_start(GTK_BOX(vbox), csv_radio, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), png_radio, FALSE, FALSE, 0);
    
    gtk_widget_show_all(dialog);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        gboolean export_csv = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(csv_radio));
        
        // File chooser
        GtkWidget *file_dialog = gtk_file_chooser_dialog_new(
            "Save Export File",
            GTK_WINDOW(widgets->window),
            GTK_FILE_CHOOSER_ACTION_SAVE,
            "Cancel", GTK_RESPONSE_CANCEL,
            "Save", GTK_RESPONSE_ACCEPT,
            NULL);
        
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(file_dialog), TRUE);
        
        if (export_csv) {
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(file_dialog), "stats_export.csv");
        } else {
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(file_dialog), "stats_chart.png");
        }
        
        if (gtk_dialog_run(GTK_DIALOG(file_dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_dialog));
            
            gboolean success = FALSE;
            if (export_csv) {
                // Export as CSV
                FILE *fp = fopen(filename, "w");
                if (fp) {
                    fprintf(fp, "Process Name,Average CPU (%%),Average RAM (MB)\n");
                    for (int i = 0; i < widgets->stats_report.count; i++) {
                        ProcessUsageStats *stat = &widgets->stats_report.stats[i];
                        fprintf(fp, "%s,%.2f,%.2f\n", 
                               stat->process_name, 
                               stat->avg_cpu_percent, 
                               stat->avg_ram_mb);
                    }
                    fclose(fp);
                    success = TRUE;
                }
            } else {
                // Export as PNG
                int width = 1200;
                int height = 700;
                cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
                cairo_t *cr = cairo_create(surface);
                
                // Render the chart
                GtkAllocation alloc = {0, 0, width, height};
                gtk_widget_set_allocation(widgets->stats_drawing_area, &alloc);
                stats_drawing_area_draw(widgets->stats_drawing_area, cr, widgets);
                
                // Save to PNG
                cairo_status_t status = cairo_surface_write_to_png(surface, filename);
                success = (status == CAIRO_STATUS_SUCCESS);
                
                cairo_destroy(cr);
                cairo_surface_destroy(surface);
            }
            
            // Show result dialog
            GtkWidget *result_dialog = gtk_message_dialog_new(
                GTK_WINDOW(widgets->window),
                GTK_DIALOG_MODAL,
                success ? GTK_MESSAGE_INFO : GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                success ? "Export completed successfully!" : "Export failed!");
            gtk_dialog_run(GTK_DIALOG(result_dialog));
            gtk_widget_destroy(result_dialog);
            
            g_free(filename);
        }
        
        gtk_widget_destroy(file_dialog);
    }
    
    gtk_widget_destroy(dialog);
}
