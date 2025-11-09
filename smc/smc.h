/*
 * Apple System Management Control (SMC) Tool
 * Copyright (C) 2006 devnull
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __SMC_H__
#define __SMC_H__
#endif

#define VERSION "0.01"

#define KERNEL_INDEX_SMC 2

// SMC Commands
#define SMC_CMD_READ_BYTES 5
#define SMC_CMD_READ_KEYINFO 9

// SMC Data Types
#define DATATYPE_FPE2 "fpe2"
#define DATATYPE_UINT8 "ui8 "
#define DATATYPE_SP78 "sp78"
#define DATATYPE_FLT "flt "

// Chip generations for SMC key selection
enum ChipGeneration {
  CHIP_INTEL = 0,
  CHIP_M1 = 1,
  CHIP_M2 = 2,
  CHIP_M3 = 3,
  CHIP_M4 = 4,
  CHIP_M5 = 5,
  CHIP_UNKNOWN = 99
};

// Common SMC keys (work across all platforms)
#define SMC_PKEY_FAN_RPM "F%dAc"
#define SMC_PKEY_FAN_MIN "F%dMn"
#define SMC_PKEY_FAN_MAX "F%dMx"

typedef struct
{
  char major;
  char minor;
  char build;
  char reserved[1];
  UInt16 release;
} SMCKeyData_vers_t;

typedef struct
{
  UInt16 version;
  UInt16 length;
  UInt32 cpuPLimit;
  UInt32 gpuPLimit;
  UInt32 memPLimit;
} SMCKeyData_pLimitData_t;

typedef struct
{
  UInt32 dataSize;
  UInt32 dataType;
  char dataAttributes;
} SMCKeyData_keyInfo_t;

typedef char SMCBytes_t[32];

typedef struct
{
  UInt32 key;
  SMCKeyData_vers_t vers;
  SMCKeyData_pLimitData_t pLimitData;
  SMCKeyData_keyInfo_t keyInfo;
  char result;
  char status;
  char data8;
  UInt32 data32;
  SMCBytes_t bytes;
} SMCKeyData_t;

typedef char UInt32Char_t[5];

typedef struct
{
  UInt32Char_t key;
  UInt32 dataSize;
  UInt32Char_t dataType;
  SMCBytes_t bytes;
} SMCVal_t;

// IOKit HID Sensor structures and functions
typedef struct {
  char name[128];
  double value;
} IOKitSensor;

typedef struct {
  IOKitSensor* sensors;
  int count;
} IOKitSensorList;

// Mac Model Database Structure
typedef struct {
  const char* hw_model;          // Hardware model identifier (e.g., "Mac14,6")
  const char* model_description;  // Model description (e.g., "MacBook Pro")
  const char* screen_size;       // Screen size (e.g., "14-inch" or "")
  const char* release_date;      // Release date (e.g., "Jan 2023")
  const char* chip_name;         // Expected chip name (e.g., "M2 Pro", "M2 Max")
} MacModelInfo;

// Function prototypes
ChipGeneration GetChipGeneration();
bool IsAppleSilicon();
const MacModelInfo* LookupMacModel(const char* hw_model);
double SMCGetTemperature();
double SMCGetTemperatureKey(const char *key);

// IOKit HID sensor functions (for Apple Silicon)
IOKitSensorList GetIOKitTemperatureSensors();
IOKitSensorList GetIOKitVoltageSensors();
IOKitSensorList GetIOKitCurrentSensors();
void FreeIOKitSensorList(IOKitSensorList list);

// Battery structure
typedef struct {
  bool external_connected;
  bool battery_installed;
  bool is_charging;
  bool fully_charged;
  int voltage;           // in mV
  int cycle_count;
  int design_capacity;   // in mAh
  int max_capacity;      // in mAh
  int current_capacity;  // in mAh
  int design_cycle_count;
  int time_remaining;    // in seconds
  int temperature;       // in 0.01°C
  int amperage;          // in mA
  int charge_percent;    // Current charge percentage (0-100)
  int health_percent;    // Battery health percentage (0-100)
} BatteryInfo;

// System information structure
typedef struct {
  uint64_t total_memory;     // Total RAM in bytes
  uint64_t uptime;           // System uptime in seconds
  char os_version[64];       // macOS version string
  char os_codename[64];      // macOS codename (e.g., "Tahoe")
  char model_name[128];      // Model name (e.g., "Apple M3 Pro")
  char hardware_model[128];  // Hardware model (e.g., "Mac14,6")
  char serial_number[64];    // Serial number
  char screen_size[32];      // Screen size (e.g., "14-inch")
  char release_year[32];     // Release year and month (e.g., "Nov 2023")
} SystemInfo;

// RAM usage structure
typedef struct {
  uint64_t total;            // Total RAM in bytes
  uint64_t used;             // Used RAM in bytes
  uint64_t free;             // Free RAM in bytes
  uint64_t active;           // Active RAM in bytes
  uint64_t inactive;         // Inactive RAM in bytes
  uint64_t wired;            // Wired RAM in bytes
  uint64_t compressed;       // Compressed RAM in bytes
  uint64_t app;              // App memory in bytes
  uint64_t cache;            // Cache memory in bytes
  uint64_t swap_total;       // Total swap in bytes
  uint64_t swap_used;        // Used swap in bytes
  uint64_t swap_free;        // Free swap in bytes
  int pressure_level;        // Memory pressure level (1=normal, 2=warning, 4=critical)
} RAMUsage;

// CPU usage structure
typedef struct {
  double user_load;          // User CPU load (0.0 - 1.0)
  double system_load;        // System CPU load (0.0 - 1.0)
  double idle_load;          // Idle CPU load (0.0 - 1.0)
  double total_usage;        // Total CPU usage (0.0 - 1.0)
  double load_avg_1;         // 1 minute load average
  double load_avg_5;         // 5 minute load average
  double load_avg_15;        // 15 minute load average
} CPUUsage;

// Disk information structure
typedef struct {
  char name[256];            // Disk name
  char mount_point[1024];    // Mount point path
  char file_system[64];      // File system type
  uint64_t total_size;       // Total size in bytes
  uint64_t free_size;        // Free size in bytes
  uint64_t used_size;        // Used size in bytes
  char bsd_name[64];         // BSD name (e.g., disk0s1)
  bool is_removable;         // Is removable media
} DiskInfo;

// Disk list structure
typedef struct {
  DiskInfo* disks;           // Array of disks
  int count;                 // Number of disks
} DiskList;

// Power metrics structure
typedef struct {
  double cpu;                // CPU power in Watts
  double gpu;                // GPU power in Watts
  double ane;                // ANE (Apple Neural Engine) power in Watts
  double ram;                // RAM power in Watts
  double gpu_ram;            // GPU RAM power in Watts
  double total;              // Total measured power (cpu + gpu + ane + ram + gpu_ram)
} PowerMetrics;
