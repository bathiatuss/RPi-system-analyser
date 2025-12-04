#ifndef LOGGER_H
#define LOGGER_H

// Initialize logger with file path
void init_logger(const char *log_file_path);

// Close logger and cleanup
void close_logger();

// Generic function to log any metric
void log_metric(const char *metric_name, double value, const char *unit);

// Log all system metrics at once (with same timestamp)
void log_system_snapshot();

// Optional: Rotate log file if size exceeds limit
void rotate_log_if_needed(unsigned long max_size_mb);

#endif