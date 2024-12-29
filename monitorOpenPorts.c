#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LOG_FILE "/var/log/port_monitor.log"
#define MAX_PORTS 1024
#define MAX_LINE_LENGTH 1035

typedef struct {
    char protocol[10];
    char local_address_port[256];
} PortEntry;

// Array to store the previous state of ports
PortEntry previous_ports[MAX_PORTS];
int previous_port_count = 0;

// Log message to file
void log_message(const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        time_t now = time(NULL);
        char time_buffer[20];

        struct tm* local_time = localtime(&now);
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time);

        fprintf(log, "[%s]\n%s\n", time_buffer, message);
        fclose(log);
    }
}

// Helper function to check if a port is in the previous state
int port_exists(PortEntry *ports, int count, const char *protocol, const char *local_address_port) {
    for (int i = 0; i < count; i++) {
        if (strcmp(ports[i].protocol, protocol) == 0 && strcmp(ports[i].local_address_port, local_address_port ) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to check ports and log only if there are changes
void check_ports() {
    FILE *fp;
    char path[MAX_LINE_LENGTH];
    PortEntry current_ports[MAX_PORTS];
    int current_port_count = 0;
    char changes[MAX_LINE_LENGTH * MAX_PORTS] = "";

    // Run the command to get open ports
    fp = popen("ss -tuln", "r");
    if (fp == NULL) {
        log_message("Failed to run command.");
        exit(1);
    }

    // Skip the header line
    fgets(path, sizeof(path), fp);

    // Read the output line by line
    while (fgets(path, sizeof(path), fp) != NULL) {
        char protocol[10], local_address_port[256];

        // Extract protocol and port
        sscanf(path, "%s %*s %*s %*s %s", protocol, local_address_port);

        // Store current port entry
        strcpy(current_ports[current_port_count].protocol, protocol);
        strcpy(current_ports[current_port_count].local_address_port, local_address_port);
        current_port_count++;
    }
    pclose(fp);

    // Compare previous and current states
    int has_changes = 0;

    // Check for new ports
    for (int i = 0; i < current_port_count; i++) {
        if (!port_exists(previous_ports, previous_port_count, current_ports[i].protocol, current_ports[i].local_address_port)) {
            char temp[300];
            snprintf(temp, sizeof(temp), "New open port: %s %s\n", current_ports[i].protocol, current_ports[i].local_address_port);
            strcat(changes, temp);
            has_changes = 1;
        }
    }

    // Check for closed ports
    for (int i = 0; i < previous_port_count; i++) {
        if (!port_exists(current_ports, current_port_count, previous_ports[i].protocol, previous_ports[i].local_address_port)) {
            char temp[300];
            snprintf(temp, sizeof(temp), "Closed port: %s %s\n", previous_ports[i].protocol, previous_ports[i].local_address_port);
            strcat(changes, temp);
            has_changes = 1;
        }
    }

    // If there are changes, log them
    if (has_changes) {
        log_message(changes);
    }

    // Update previous ports to current ports
    previous_port_count = current_port_count;
    memcpy(previous_ports, current_ports, sizeof(PortEntry) * current_port_count);
}

// Signal handler to stop the daemon
void signal_handler(int sig) {
    if (sig == SIGTERM) {
        log_message("Stopping port monitoring daemon.");
        exit(0);
    }
}

int main() {

    pid_t pid, sid;

    // Fork off the parent process
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);

    log_message("Starting port monitoring daemon.");

    sid = setsid();
    if (sid < 0) exit(EXIT_FAILURE);

    if ((chdir("/")) < 0) exit(EXIT_FAILURE);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGTERM, signal_handler);

    while (1) {
        check_ports();
        sleep(60);
    }

    exit(EXIT_SUCCESS);
}

