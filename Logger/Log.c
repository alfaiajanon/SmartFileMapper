#include <stdio.h>

typedef struct {
    const char *logFilePath;
} LogSettings;


void writeLog(LogSettings *logSettings, const char *message) {
    FILE *logFile = fopen(logSettings->logFilePath, "a");
    if (logFile) {
        fprintf(logFile, "%s\n", message);
        fclose(logFile);
    } else {
        perror("Failed to open log file");
    }
}