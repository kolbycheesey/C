# Logging System

This document explains how to use the logging system implemented in the database engine to help with debugging and monitoring.

## Overview

The logging system provides several key features:
- Multiple log levels (DEBUG, INFO, WARNING, ERROR, NONE)
- Compile-time log filtering for performance optimization
- Runtime log level control
- Optional file output
- Thread-safe logging

## Log Levels

The available log levels in order of increasing severity are:

1. **DEBUG** - Detailed information, useful only during debugging
2. **INFO** - General information showing the normal operation of the application
3. **WARNING** - Indicates potential issues that don't prevent the system from working
4. **ERROR** - Critical issues that prevent operations from functioning correctly
5. **NONE** - Disables all logging

## Build Configuration

You can configure logging at compile time with the following CMake options:

### Setting the Log Level

```bash
cmake -DLOG_LEVEL=INFO ..
```

Available options: DEBUG, INFO, WARNING, ERROR, NONE

### Redirect Logs to File

```bash
cmake -DLOG_TO_FILE=ON ..
```

### Specify Custom Log File Path

```bash
cmake -DLOG_FILE_PATH="/path/to/logfile.log" ..
```

## Usage in Code

### Including the Logger

```cpp
#include "utils/logger.h"
```

### Logging Messages

```cpp
LOG_DEBUG("This is a debug message with details: " + std::to_string(value));
LOG_INFO("Operation completed successfully");
LOG_WARNING("This operation might cause issues");
LOG_ERROR("Failed to complete operation: " + errorMessage);
```

### Performance Considerations

- Log messages below the configured log level are completely eliminated at compile time, resulting in zero runtime overhead.
- For performance-critical sections, consider setting a higher log level during compilation.

## Example CMake Command

```bash
# Debug build with all logging directed to file
cmake -DCMAKE_BUILD_TYPE=Debug -DLOG_LEVEL=DEBUG -DLOG_TO_FILE=ON -DLOG_FILE_PATH="./logs/database.log" ..

# Release build with only warnings and errors to console
cmake -DCMAKE_BUILD_TYPE=Release -DLOG_LEVEL=WARNING ..
```

## Common Issues and Troubleshooting

- If you're experiencing performance issues, consider raising the log level to WARNING or ERROR in production builds.
- For debugging specific components, use DEBUG level but be aware it will generate larger log files.
- If the application crashes before writing to the log file, check if the log directory exists and is writable.