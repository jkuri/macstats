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

#define SMC_CMD_READ_BYTES 5
#define SMC_CMD_WRITE_BYTES 6
#define SMC_CMD_READ_INDEX 8
#define SMC_CMD_READ_KEYINFO 9
#define SMC_CMD_READ_PLIMIT 11
#define SMC_CMD_READ_VERS 12

#define DATATYPE_FPE2 "fpe2"
#define DATATYPE_UINT8 "ui8 "
#define DATATYPE_UINT16 "ui16"
#define DATATYPE_UINT32 "ui32"
#define DATATYPE_SP78 "sp78"

// Apple Silicon Model Detection
enum AppleSiliconModel {
  MODEL_UNKNOWN = 0,
  MODEL_INTEL,
  MODEL_M1,
  MODEL_M1_PRO,
  MODEL_M1_MAX,
  MODEL_M1_ULTRA,
  MODEL_M2,
  MODEL_M2_PRO,
  MODEL_M2_MAX,
  MODEL_M2_ULTRA,
  MODEL_M3,
  MODEL_M3_PRO,
  MODEL_M3_MAX,
  MODEL_M3_ULTRA,
  MODEL_M4,
  MODEL_M4_PRO,
  MODEL_M4_MAX,
  MODEL_M4_ULTRA,
  MODEL_M5,
  MODEL_M5_PRO,
  MODEL_M5_MAX,
  MODEL_M5_ULTRA
};

// SMC Key definitions - Intel Macs
#define SMC_KEY_CPU_TEMP_INTEL "TC0P"
#define SMC_KEY_CPU_DIE_INTEL "TC0D"
#define SMC_KEY_GPU_TEMP_INTEL "TG0D"

// SMC Key definitions - M1 Generation (M1, M1 Pro, M1 Max, M1 Ultra)
#define SMC_KEY_CPU_PCORE1_M1 "Tp01"
#define SMC_KEY_CPU_PCORE2_M1 "Tp05"
#define SMC_KEY_CPU_PCORE3_M1 "Tp0D"
#define SMC_KEY_CPU_PCORE4_M1 "Tp0H"
#define SMC_KEY_CPU_PCORE5_M1 "Tp0L"
#define SMC_KEY_CPU_PCORE6_M1 "Tp0P"
#define SMC_KEY_CPU_PCORE7_M1 "Tp0X"
#define SMC_KEY_CPU_PCORE8_M1 "Tp0b"
#define SMC_KEY_CPU_ECORE1_M1 "Tp09"
#define SMC_KEY_CPU_ECORE2_M1 "Tp0T"
#define SMC_KEY_GPU1_M1 "Tg05"
#define SMC_KEY_GPU2_M1 "Tg0D"
#define SMC_KEY_GPU3_M1 "Tg0L"
#define SMC_KEY_GPU4_M1 "Tg0T"

// SMC Key definitions - M2 Generation (M2, M2 Pro, M2 Max, M2 Ultra)
#define SMC_KEY_CPU_PCORE1_M2 "Tp01"
#define SMC_KEY_CPU_PCORE2_M2 "Tp05"
#define SMC_KEY_CPU_PCORE3_M2 "Tp09"
#define SMC_KEY_CPU_PCORE4_M2 "Tp0D"
#define SMC_KEY_CPU_PCORE5_M2 "Tp0X"
#define SMC_KEY_CPU_PCORE6_M2 "Tp0b"
#define SMC_KEY_CPU_PCORE7_M2 "Tp0f"
#define SMC_KEY_CPU_PCORE8_M2 "Tp0j"
#define SMC_KEY_CPU_ECORE1_M2 "Tp1h"
#define SMC_KEY_CPU_ECORE2_M2 "Tp1t"
#define SMC_KEY_CPU_ECORE3_M2 "Tp1p"
#define SMC_KEY_CPU_ECORE4_M2 "Tp1l"
#define SMC_KEY_GPU1_M2 "Tg0f"
#define SMC_KEY_GPU2_M2 "Tg0j"

// SMC Key definitions - M3 Generation (M3, M3 Pro, M3 Max, M3 Ultra)
#define SMC_KEY_CPU_PCORE1_M3 "Tf04"
#define SMC_KEY_CPU_PCORE2_M3 "Tf09"
#define SMC_KEY_CPU_PCORE3_M3 "Tf0A"
#define SMC_KEY_CPU_PCORE4_M3 "Tf0B"
#define SMC_KEY_CPU_PCORE5_M3 "Tf0D"
#define SMC_KEY_CPU_PCORE6_M3 "Tf0E"
#define SMC_KEY_CPU_PCORE7_M3 "Tf44"
#define SMC_KEY_CPU_PCORE8_M3 "Tf49"
#define SMC_KEY_CPU_PCORE9_M3 "Tf4A"
#define SMC_KEY_CPU_PCORE10_M3 "Tf4B"
#define SMC_KEY_CPU_PCORE11_M3 "Tf4D"
#define SMC_KEY_CPU_PCORE12_M3 "Tf4E"
#define SMC_KEY_CPU_ECORE1_M3 "Te05"
#define SMC_KEY_CPU_ECORE2_M3 "Te0L"
#define SMC_KEY_CPU_ECORE3_M3 "Te0P"
#define SMC_KEY_CPU_ECORE4_M3 "Te0S"
#define SMC_KEY_GPU1_M3 "Tf14"
#define SMC_KEY_GPU2_M3 "Tf18"
#define SMC_KEY_GPU3_M3 "Tf19"
#define SMC_KEY_GPU4_M3 "Tf1A"
#define SMC_KEY_GPU5_M3 "Tf24"
#define SMC_KEY_GPU6_M3 "Tf28"
#define SMC_KEY_GPU7_M3 "Tf29"
#define SMC_KEY_GPU8_M3 "Tf2A"

// SMC Key definitions - M4 Generation (M4, M4 Pro, M4 Max, M4 Ultra)
#define SMC_KEY_CPU_PCORE1_M4 "Tp01"
#define SMC_KEY_CPU_PCORE2_M4 "Tp05"
#define SMC_KEY_CPU_PCORE3_M4 "Tp09"
#define SMC_KEY_CPU_PCORE4_M4 "Tp0D"
#define SMC_KEY_CPU_PCORE5_M4 "Tp0V"
#define SMC_KEY_CPU_PCORE6_M4 "Tp0Y"
#define SMC_KEY_CPU_PCORE7_M4 "Tp0b"
#define SMC_KEY_CPU_PCORE8_M4 "Tp0e"
#define SMC_KEY_CPU_ECORE1_M4 "Te05"
#define SMC_KEY_CPU_ECORE2_M4 "Te0S"
#define SMC_KEY_CPU_ECORE3_M4 "Te09"
#define SMC_KEY_CPU_ECORE4_M4 "Te0H"
#define SMC_KEY_GPU1_M4 "Tg0G"
#define SMC_KEY_GPU2_M4 "Tg0H"
#define SMC_KEY_GPU1_M4PRO "Tg1U"
#define SMC_KEY_GPU2_M4PRO "Tg1k"

// SMC Key definitions - M5 Generation (M5, M5 Pro, M5 Max, M5 Ultra)
// Note: These are placeholder keys - actual keys will be determined when M5 hardware is released
#define SMC_KEY_GPU1_M5 "Tg0G"  // Placeholder - use M4 key as fallback
#define SMC_KEY_GPU2_M5 "Tg0H"  // Placeholder - use M4 key as fallback

// Common SMC keys (work across all platforms)
#define SMC_KEY_FAN_NUMBER "FNum"
#define SMC_PKEY_FAN_RPM "F%dAc"
#define SMC_PKEY_FAN_MIN "F%dMn"
#define SMC_PKEY_FAN_MAX "F%dMx"

// Power keys (may not work on all Apple Silicon models)
#define SMC_KEY_CPU_POWER "PCTR"
#define SMC_KEY_GPU_POWER "PGTR"
#define SMC_KEY_SYSTEM_POWER "PSTR"

// Voltage keys (may not work on all Apple Silicon models)
#define SMC_KEY_CPU_VOLTAGE "VC0C"
#define SMC_KEY_GPU_VOLTAGE "VG0C"
#define SMC_KEY_MEMORY_VOLTAGE "VM0R"

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

// prototypes
AppleSiliconModel GetAppleSiliconModel();
const char* GetModelName(AppleSiliconModel model);
double SMCGetTemperature();
double SMCGetTemperatureKey(const char *key);
kern_return_t SMCSetFanRpm(char *key, int rpm);
int SMCGetFanRpm(char *key);

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
