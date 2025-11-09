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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 USA.
 */

#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDEventSystemClient.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#include <CoreFoundation/CoreFoundation.h>
#include <nan.h>
#include <node.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <v8.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/vm_statistics.h>
#include <DiskArbitration/DiskArbitration.h>

#include "smc.h"

// IOKit HID declarations (private APIs)
extern "C" {
  typedef struct __IOHIDEvent* IOHIDEventRef;
  typedef struct __IOHIDServiceClient* IOHIDServiceClientRef;
  typedef double IOHIDFloat;

  IOHIDEventSystemClientRef IOHIDEventSystemClientCreate(CFAllocatorRef allocator);
  CFArrayRef IOHIDEventSystemClientCopyServices(IOHIDEventSystemClientRef client);
  int IOHIDEventSystemClientSetMatching(IOHIDEventSystemClientRef client, CFDictionaryRef match);
  IOHIDEventRef IOHIDServiceClientCopyEvent(IOHIDServiceClientRef service, int64_t type, int32_t options, int64_t depth);
  CFStringRef IOHIDServiceClientCopyProperty(IOHIDServiceClientRef service, CFStringRef property);
  IOHIDFloat IOHIDEventGetFloatValue(IOHIDEventRef event, int32_t field);
}

// IOReport framework declarations (private APIs)
extern "C" {
  typedef struct __IOReportSubscriptionCF* IOReportSubscriptionRef;

  CFDictionaryRef IOReportCopyChannelsInGroup(CFStringRef group, CFStringRef subgroup, uint64_t v0, uint64_t v1, uint64_t v2);
  CFDictionaryRef IOReportCreateSamples(IOReportSubscriptionRef subscription, CFMutableDictionaryRef channels, CFTypeRef a);
  CFDictionaryRef IOReportCreateSamplesDelta(CFDictionaryRef prev, CFDictionaryRef current, CFTypeRef a);
  IOReportSubscriptionRef IOReportCreateSubscription(void* a, CFMutableDictionaryRef desiredChannels, CFMutableDictionaryRef* subbedChannels, uint64_t channel_id, CFTypeRef b);
  int64_t IOReportSimpleGetIntegerValue(CFDictionaryRef sample, int entry);
  CFStringRef IOReportChannelGetChannelName(CFDictionaryRef channel);
  CFStringRef IOReportChannelGetGroup(CFDictionaryRef channel);
  CFStringRef IOReportChannelGetSubGroup(CFDictionaryRef channel);
  CFStringRef IOReportChannelGetUnitLabel(CFDictionaryRef channel);
  void IOReportMergeChannels(CFDictionaryRef a, CFDictionaryRef b, CFTypeRef c);
}

// IOKit HID constants
#define kIOHIDEventTypeTemperature 15
#define kIOHIDEventTypePower 25
#define IOHIDEventFieldBase(type) (type << 16)

using namespace v8;

static io_connect_t conn;

// Get chip generation from CPU brand string for SMC key selection
ChipGeneration GetChipGeneration() {
  static ChipGeneration cached_gen = CHIP_UNKNOWN;

  if (cached_gen != CHIP_UNKNOWN) {
    return cached_gen;
  }

  char cpu_brand[128];
  size_t size = sizeof(cpu_brand);

  if (sysctlbyname("machdep.cpu.brand_string", &cpu_brand, &size, NULL, 0) != 0) {
    cached_gen = CHIP_INTEL;
    return cached_gen;
  }

  // Check if Intel
  if (strstr(cpu_brand, "Intel") != NULL) {
    cached_gen = CHIP_INTEL;
    return cached_gen;
  }

  // Check if Apple Silicon
  if (strstr(cpu_brand, "Apple") == NULL) {
    cached_gen = CHIP_INTEL;
    return cached_gen;
  }

  // Detect specific Apple Silicon generation (M1, M2, M3, M4, M5)
  if (strstr(cpu_brand, "M5") != NULL) {
    cached_gen = CHIP_M5;
  } else if (strstr(cpu_brand, "M4") != NULL) {
    cached_gen = CHIP_M4;
  } else if (strstr(cpu_brand, "M3") != NULL) {
    cached_gen = CHIP_M3;
  } else if (strstr(cpu_brand, "M2") != NULL) {
    cached_gen = CHIP_M2;
  } else if (strstr(cpu_brand, "M1") != NULL) {
    cached_gen = CHIP_M1;
  } else {
    // Unknown Apple Silicon model - default to M1 keys as fallback
    cached_gen = CHIP_M1;
  }

  return cached_gen;
}

// Check if running on Apple Silicon (backward compatibility)
bool IsAppleSilicon() {
  return GetChipGeneration() != CHIP_INTEL;
}

// Comprehensive Mac Model Database
// Based on public Mac model identifiers and release information
// This database maps hardware model identifiers (e.g., "Mac15,6") to:
// - Model description (e.g., "MacBook Pro")
// - Screen size (e.g., "14-inch")
// - Release date (e.g., "Nov 2023")
// - Expected chip (e.g., "M3 Pro")
//
// This approach is inspired by macmon and eliminates hardcoded model checks,
// making it easier to add support for new Mac models by simply adding entries
// to this database rather than modifying conditional logic throughout the code.
//
// To add support for new Mac models:
// 1. Find the hardware model identifier using: sysctl hw.model
// 2. Add a new entry to the MAC_MODELS array with the relevant information
// 3. Rebuild the module
static const MacModelInfo MAC_MODELS[] = {
  // M4 Generation (2024)
  {"Mac16,1", "MacBook Pro", "14-inch", "Nov 2024", "M4"},
  {"Mac16,2", "MacBook Pro", "14-inch", "Nov 2024", "M4"},
  {"Mac16,3", "MacBook Pro", "14-inch", "Nov 2024", "M4"},
  {"Mac16,5", "MacBook Pro", "16-inch", "Nov 2024", "M4 Pro"},
  {"Mac16,6", "MacBook Pro", "14-inch", "Nov 2024", "M4 Pro"},
  {"Mac16,7", "MacBook Pro", "16-inch", "Nov 2024", "M4 Pro"},
  {"Mac16,8", "MacBook Pro", "14-inch", "Nov 2024", "M4 Max"},
  {"Mac16,9", "MacBook Pro", "14-inch", "Nov 2024", "M4 Max"},
  {"Mac16,10", "MacBook Pro", "16-inch", "Nov 2024", "M4 Max"},
  {"Mac16,11", "MacBook Pro", "16-inch", "Nov 2024", "M4 Max"},
  {"Mac16,12", "Mac mini", "", "Nov 2024", "M4"},
  {"Mac16,13", "Mac mini", "", "Nov 2024", "M4 Pro"},
  {"Mac17,1", "iMac", "24-inch", "Nov 2024", "M4"},

  // M3 Generation (2023-2024)
  {"Mac15,3", "MacBook Pro", "14-inch", "Nov 2023", "M3"},
  {"Mac15,4", "iMac", "24-inch", "Nov 2023", "M3"},
  {"Mac15,5", "iMac", "24-inch", "Nov 2023", "M3"},
  {"Mac15,6", "MacBook Pro", "14-inch", "Nov 2023", "M3 Pro"},
  {"Mac15,7", "MacBook Pro", "16-inch", "Nov 2023", "M3 Pro"},
  {"Mac15,8", "MacBook Pro", "14-inch", "Nov 2023", "M3 Max"},
  {"Mac15,9", "MacBook Pro", "14-inch", "Nov 2023", "M3 Max"},
  {"Mac15,10", "MacBook Pro", "16-inch", "Nov 2023", "M3 Max"},
  {"Mac15,11", "MacBook Pro", "16-inch", "Nov 2023", "M3 Max"},
  {"Mac15,12", "MacBook Air", "13-inch", "Mar 2024", "M3"},
  {"Mac15,13", "MacBook Air", "15-inch", "Mar 2024", "M3"},

  // M2 Generation (2022-2023)
  {"Mac14,2", "MacBook Air", "13-inch", "Jun 2022", "M2"},
  {"Mac14,3", "Mac mini", "", "Jan 2023", "M2"},
  {"Mac14,5", "MacBook Pro", "14-inch", "Jan 2023", "M2 Pro"},
  {"Mac14,6", "MacBook Pro", "16-inch", "Jan 2023", "M2 Pro"},
  {"Mac14,7", "MacBook Pro", "13-inch", "Jun 2022", "M2"},
  {"Mac14,8", "Mac Studio", "", "Jun 2023", "M2 Max"},
  {"Mac14,9", "MacBook Pro", "14-inch", "Jan 2023", "M2 Max"},
  {"Mac14,10", "MacBook Pro", "16-inch", "Jan 2023", "M2 Max"},
  {"Mac14,12", "Mac mini", "", "Jan 2023", "M2 Pro"},
  {"Mac14,13", "Mac Studio", "", "Jun 2023", "M2 Ultra"},
  {"Mac14,14", "Mac Studio", "", "Jun 2023", "M2 Ultra"},
  {"Mac14,15", "MacBook Air", "15-inch", "Jun 2023", "M2"},

  // M1 Generation (2020-2021)
  {"MacBookAir10,1", "MacBook Air", "13-inch", "Nov 2020", "M1"},
  {"MacBookPro17,1", "MacBook Pro", "13-inch", "Nov 2020", "M1"},
  {"MacBookPro18,1", "MacBook Pro", "16-inch", "Oct 2021", "M1 Pro"},
  {"MacBookPro18,2", "MacBook Pro", "16-inch", "Oct 2021", "M1 Max"},
  {"MacBookPro18,3", "MacBook Pro", "14-inch", "Oct 2021", "M1 Pro"},
  {"MacBookPro18,4", "MacBook Pro", "14-inch", "Oct 2021", "M1 Max"},
  {"Macmini9,1", "Mac mini", "", "Nov 2020", "M1"},
  {"iMac21,1", "iMac", "24-inch", "Apr 2021", "M1"},
  {"iMac21,2", "iMac", "24-inch", "Apr 2021", "M1"},
  {"Mac13,1", "Mac Studio", "", "Mar 2022", "M1 Max"},
  {"Mac13,2", "Mac Studio", "", "Mar 2022", "M1 Ultra"},

  // Intel Generation - Major Models (2017-2020)
  {"MacBookPro14,1", "MacBook Pro", "13-inch", "Jun 2017", "Intel"},
  {"MacBookPro14,2", "MacBook Pro", "13-inch", "Jun 2017", "Intel"},
  {"MacBookPro14,3", "MacBook Pro", "15-inch", "Jun 2017", "Intel"},
  {"MacBookPro15,1", "MacBook Pro", "15-inch", "Jul 2018", "Intel"},
  {"MacBookPro15,2", "MacBook Pro", "13-inch", "Jul 2018", "Intel"},
  {"MacBookPro15,3", "MacBook Pro", "15-inch", "May 2019", "Intel"},
  {"MacBookPro15,4", "MacBook Pro", "13-inch", "Jul 2019", "Intel"},
  {"MacBookPro16,1", "MacBook Pro", "16-inch", "Nov 2019", "Intel"},
  {"MacBookPro16,2", "MacBook Pro", "13-inch", "May 2020", "Intel"},
  {"MacBookPro16,3", "MacBook Pro", "13-inch", "May 2020", "Intel"},
  {"MacBookPro16,4", "MacBook Pro", "16-inch", "Nov 2019", "Intel"},
  {"MacBookAir8,1", "MacBook Air", "13-inch", "Oct 2018", "Intel"},
  {"MacBookAir8,2", "MacBook Air", "13-inch", "Jul 2019", "Intel"},
  {"MacBookAir9,1", "MacBook Air", "13-inch", "Mar 2020", "Intel"},
  {"iMac19,1", "iMac", "27-inch", "Mar 2019", "Intel"},
  {"iMac19,2", "iMac", "21.5-inch", "Mar 2019", "Intel"},
  {"iMac20,1", "iMac", "27-inch", "Aug 2020", "Intel"},
  {"iMac20,2", "iMac", "27-inch", "Aug 2020", "Intel"},
  {"iMacPro1,1", "iMac Pro", "27-inch", "Dec 2017", "Intel"},
  {"Macmini8,1", "Mac mini", "", "Oct 2018", "Intel"},
  {"MacPro7,1", "Mac Pro", "", "Dec 2019", "Intel"},

  // Terminator
  {NULL, NULL, NULL, NULL, NULL}
};

// Lookup Mac model information from hardware identifier
const MacModelInfo* LookupMacModel(const char* hw_model) {
  if (!hw_model || hw_model[0] == '\0') {
    return NULL;
  }

  for (int i = 0; MAC_MODELS[i].hw_model != NULL; i++) {
    if (strcmp(MAC_MODELS[i].hw_model, hw_model) == 0) {
      return &MAC_MODELS[i];
    }
  }

  return NULL;
}

// IOKit HID sensor functions for Apple Silicon
IOKitSensorList GetIOKitSensors(int page, int usage, int eventType) {
  IOKitSensorList result;
  result.sensors = NULL;
  result.count = 0;

  // Create matching dictionary
  CFMutableDictionaryRef matchingDict = CFDictionaryCreateMutable(
    kCFAllocatorDefault, 0,
    &kCFTypeDictionaryKeyCallBacks,
    &kCFTypeDictionaryValueCallBacks
  );

  if (!matchingDict) {
    return result;
  }

  CFNumberRef pageNum = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
  CFNumberRef usageNum = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);

  CFDictionarySetValue(matchingDict, CFSTR("PrimaryUsagePage"), pageNum);
  CFDictionarySetValue(matchingDict, CFSTR("PrimaryUsage"), usageNum);

  CFRelease(pageNum);
  CFRelease(usageNum);

  // Create HID event system client
  IOHIDEventSystemClientRef system = IOHIDEventSystemClientCreate(kCFAllocatorDefault);
  if (!system) {
    CFRelease(matchingDict);
    return result;
  }

  IOHIDEventSystemClientSetMatching(system, matchingDict);
  CFArrayRef services = IOHIDEventSystemClientCopyServices(system);

  CFRelease(matchingDict);

  if (!services) {
    CFRelease(system);
    return result;
  }

  CFIndex count = CFArrayGetCount(services);
  if (count == 0) {
    CFRelease(services);
    CFRelease(system);
    return result;
  }

  // Allocate sensor array
  result.sensors = (IOKitSensor*)malloc(sizeof(IOKitSensor) * count);
  result.count = 0;

  // Read sensor values
  for (CFIndex i = 0; i < count; i++) {
    IOHIDServiceClientRef service = (IOHIDServiceClientRef)CFArrayGetValueAtIndex(services, i);

    // Get sensor name
    CFStringRef nameRef = IOHIDServiceClientCopyProperty(service, CFSTR("Product"));
    const char* name = "Unknown";
    if (nameRef) {
      name = CFStringGetCStringPtr(nameRef, kCFStringEncodingUTF8);
      if (!name) {
        char buffer[128];
        if (CFStringGetCString(nameRef, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
          name = buffer;
        }
      }
      strncpy(result.sensors[result.count].name, name, sizeof(result.sensors[result.count].name) - 1);
      result.sensors[result.count].name[sizeof(result.sensors[result.count].name) - 1] = '\0';
      CFRelease(nameRef);
    } else {
      strncpy(result.sensors[result.count].name, "Unknown", sizeof(result.sensors[result.count].name) - 1);
    }

    // Get sensor value
    IOHIDEventRef event = IOHIDServiceClientCopyEvent(service, eventType, 0, 0);
    double value = 0.0;

    if (event) {
      value = IOHIDEventGetFloatValue(event, IOHIDEventFieldBase(eventType));
      if (eventType == kIOHIDEventTypePower) {
        value = value / 1000.0; // Convert mW to W
      }
      CFRelease(event);
    }

    result.sensors[result.count].value = value;
    result.count++;
  }

  CFRelease(services);
  CFRelease(system);

  return result;
}

IOKitSensorList GetIOKitTemperatureSensors() {
  // kHIDPage_AppleVendor = 0xff00
  // kHIDUsage_AppleVendor_TemperatureSensor = 0x0005
  return GetIOKitSensors(0xff00, 5, kIOHIDEventTypeTemperature);
}

IOKitSensorList GetIOKitVoltageSensors() {
  // kHIDPage_AppleVendorPowerSensor = 0xff08
  // kHIDUsage_AppleVendorPowerSensor_Voltage = 0x0003
  return GetIOKitSensors(0xff08, 3, kIOHIDEventTypePower);
}

IOKitSensorList GetIOKitCurrentSensors() {
  // kHIDPage_AppleVendorPowerSensor = 0xff08
  // kHIDUsage_AppleVendorPowerSensor_Current = 0x0002
  return GetIOKitSensors(0xff08, 2, kIOHIDEventTypePower);
}

void FreeIOKitSensorList(IOKitSensorList list) {
  if (list.sensors) {
    free(list.sensors);
  }
}

UInt32 _strtoul(char *str, int size, int base) {
  UInt32 total = 0;
  int i;

  for (i = 0; i < size; i++) {
    if (base == 16)
      total += str[i] << (size - 1 - i) * 8;
    else
      total += (unsigned char)(str[i] << (size - 1 - i) * 8);
  }
  return total;
}

float _strtof(char *str, int size, int e) {
  float total = 0;
  int i;

  for (i = 0; i < size; i++) {
    if (i == (size - 1))
      total += (str[i] & 0xff) >> e;
    else
      total += str[i] << (size - 1 - i) * (8 - e);
  }

  return total;
}

void _ultostr(char *str, UInt32 val) {
  str[0] = '\0';
  snprintf(str, 5, "%c%c%c%c", (unsigned int)val >> 24, (unsigned int)val >> 16,
           (unsigned int)val >> 8, (unsigned int)val);
}

kern_return_t SMCOpen(void) {
  kern_return_t result;
  io_iterator_t iterator;
  io_object_t device;

  // Use AppleSMC to enumerate all services
  CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
  result = IOServiceGetMatchingServices(kIOMainPortDefault,
                                        matchingDictionary, &iterator);
  if (result != kIOReturnSuccess) {
    printf("Error: IOServiceGetMatchingServices() = %08x\n", result);
    return 1;
  }

  // Match macmon: look for "AppleSMCKeysEndpoint" specifically for key enumeration
  io_object_t found_device = 0;
  while ((device = IOIteratorNext(iterator)) != 0) {
    io_name_t name;
    if (IORegistryEntryGetName(device, name) == kIOReturnSuccess) {
      if (strcmp(name, "AppleSMCKeysEndpoint") == 0) {
        found_device = device;
        break;
      }
    }
    IOObjectRelease(device);
  }
  IOObjectRelease(iterator);

  if (found_device == 0) {
    // Fallback to first AppleSMC device if AppleSMCKeysEndpoint not found
    matchingDictionary = IOServiceMatching("AppleSMC");
    result = IOServiceGetMatchingServices(kIOMainPortDefault,
                                          matchingDictionary, &iterator);
    if (result != kIOReturnSuccess) {
      printf("Error: IOServiceGetMatchingServices() = %08x\n", result);
      return 1;
    }
    found_device = IOIteratorNext(iterator);
    IOObjectRelease(iterator);
  }

  if (found_device == 0) {
    printf("Error: no SMC found\n");
    return 1;
  }

  result = IOServiceOpen(found_device, mach_task_self(), 0, &conn);
  IOObjectRelease(found_device);
  if (result != kIOReturnSuccess) {
    printf("Error: IOServiceOpen() = %08x\n", result);
    return 1;
  }

  return kIOReturnSuccess;
}

kern_return_t SMCClose() { return IOServiceClose(conn); }

kern_return_t SMCCall(int index, SMCKeyData_t *inputStructure,
                      SMCKeyData_t *outputStructure) {
  size_t structureInputSize;
  size_t structureOutputSize;

  structureInputSize = sizeof(SMCKeyData_t);
  structureOutputSize = sizeof(SMCKeyData_t);

#if MAC_OS_X_VERSION_10_5
  return IOConnectCallStructMethod(conn, index,
                                   // inputStructure
                                   inputStructure, structureInputSize,
                                   // ouputStructure
                                   outputStructure, &structureOutputSize);
#else
  return IOConnectMethodStructureIStructureO(
      conn, index, structureInputSize, /* structureInputSize */
      &structureOutputSize,            /* structureOutputSize */
      inputStructure,                  /* inputStructure */
      outputStructure);                /* ouputStructure */
#endif
}

kern_return_t SMCReadKey(UInt32Char_t key, SMCVal_t *val) {
  kern_return_t result;
  SMCKeyData_t inputStructure;
  SMCKeyData_t outputStructure;

  memset(&inputStructure, 0, sizeof(SMCKeyData_t));
  memset(&outputStructure, 0, sizeof(SMCKeyData_t));
  memset(val, 0, sizeof(SMCVal_t));

  inputStructure.key = _strtoul(key, 4, 16);
  inputStructure.data8 = SMC_CMD_READ_KEYINFO;

  result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
  if (result != kIOReturnSuccess)
    return result;

  val->dataSize = outputStructure.keyInfo.dataSize;
  _ultostr(val->dataType, outputStructure.keyInfo.dataType);
  inputStructure.keyInfo.dataSize = val->dataSize;
  inputStructure.data8 = SMC_CMD_READ_BYTES;

  result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
  if (result != kIOReturnSuccess)
    return result;

  memcpy(val->bytes, outputStructure.bytes, sizeof(outputStructure.bytes));

  return kIOReturnSuccess;
}

kern_return_t SMCReadKeyInfo(UInt32Char_t key, SMCKeyData_keyInfo_t *keyInfo) {
  kern_return_t result;
  SMCKeyData_t inputStructure;
  SMCKeyData_t outputStructure;

  memset(&inputStructure, 0, sizeof(SMCKeyData_t));
  memset(&outputStructure, 0, sizeof(SMCKeyData_t));

  inputStructure.key = _strtoul(key, 4, 16);
  inputStructure.data8 = SMC_CMD_READ_KEYINFO;

  result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
  if (result == kIOReturnSuccess) {
    *keyInfo = outputStructure.keyInfo;
  }

  return result;
}

// Cache for discovered CPU temperature keys
static std::vector<std::string> cpu_temp_keys_cache;
static bool cpu_temp_keys_initialized = false;

// Cache for discovered GPU temperature keys
static std::vector<std::string> gpu_temp_keys_cache;
static bool gpu_temp_keys_initialized = false;

// Get CPU temperature - reads SMC temperature sensors directly
double SMCGetTemperature() {
  ChipGeneration gen = GetChipGeneration();

  if (gen == CHIP_INTEL) {
    // Intel: use traditional SMC key
    return SMCGetTemperatureKey("TC0P");
  }

  // Initialize cache on first call - optimized scanning
  if (!cpu_temp_keys_initialized) {
    const char* prefixes[] = {"Tp", "Te"};

    // Strategy: Check common patterns first, stop if we find enough
    // Most Apple Silicon temp keys follow patterns like Tp01, Tp05, Te05, etc.

    // Pattern 1: Numeric keys (0-9)(0-9) - covers 90% of cases
    for (int p = 0; p < 2; p++) {
      for (int i = 0; i <= 9; i++) {
        for (int j = 0; j <= 9; j++) {
          char key[5];
          snprintf(key, sizeof(key), "%s%d%d", prefixes[p], i, j);

          SMCVal_t val;
          kern_return_t result = SMCReadKey(key, &val);

          if (result == kIOReturnSuccess && val.dataSize == 4 && strcmp(val.dataType, DATATYPE_FLT) == 0) {
            cpu_temp_keys_cache.push_back(key);
          }
        }
      }
    }

    // Pattern 2: Only check hex patterns (0-9)(a-f) if we need more coverage
    // This adds keys like Tp0a, Tp0f, etc.
    if (cpu_temp_keys_cache.size() < 20) {
      for (int p = 0; p < 2; p++) {
        for (int i = 0; i <= 9; i++) {
          for (char j = 'a'; j <= 'f'; j++) {
            char key[5];
            snprintf(key, sizeof(key), "%s%d%c", prefixes[p], i, j);

            SMCVal_t val;
            kern_return_t result = SMCReadKey(key, &val);

            if (result == kIOReturnSuccess && val.dataSize == 4 && strcmp(val.dataType, DATATYPE_FLT) == 0) {
              cpu_temp_keys_cache.push_back(key);
            }
          }
        }
      }
    }

    cpu_temp_keys_initialized = true;
  }

  // Read from cached keys
  double total = 0.0;
  int valid_count = 0;

  for (const auto& key_str : cpu_temp_keys_cache) {
    SMCVal_t val;
    kern_return_t result = SMCReadKey((char*)key_str.c_str(), &val);

    if (result == kIOReturnSuccess && val.dataSize == 4 && strcmp(val.dataType, DATATYPE_FLT) == 0) {
      float temp;
      memcpy(&temp, val.bytes, sizeof(float));

      if (temp > 0.0 && temp < 110.0) {
        total += temp;
        valid_count++;
      }
    }
  }

  if (valid_count > 0) {
    return total / valid_count;
  }

  // Fallback to IOKit HID sensors (for older M1 or systems without SMC access)
  IOKitSensorList sensors = GetIOKitTemperatureSensors();
  if (sensors.count == 0) {
    FreeIOKitSensorList(sensors);
    return 0.0;
  }

  total = 0.0;
  valid_count = 0;

  for (int i = 0; i < sensors.count; i++) {
    const char* name = sensors.sensors[i].name;

    // Match macmon HID logic: pACC/eACC MTR Temp Sensor
    bool is_cpu_sensor = (strstr(name, "pACC MTR Temp Sensor") != NULL) ||
                         (strstr(name, "eACC MTR Temp Sensor") != NULL);

    if (is_cpu_sensor && sensors.sensors[i].value > 0.0) {
      total += sensors.sensors[i].value;
      valid_count++;
    }
  }

  FreeIOKitSensorList(sensors);
  return valid_count > 0 ? total / valid_count : 0.0;
}

double SMCGetTemperatureKey(const char *key) {
  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey((char *)key, &val);
  if (result == kIOReturnSuccess) {
    if (val.dataSize > 0) {
      // Handle sp78 format (older chips)
      if (strcmp(val.dataType, DATATYPE_SP78) == 0) {
        int intValue = val.bytes[0] * 256 + (unsigned char)val.bytes[1];
        return intValue / 256.0;
      }
      // Handle flt  format (newer chips like M3) - macmon uses little-endian float
      if (strcmp(val.dataType, DATATYPE_FLT) == 0 && val.dataSize == 4) {
        // Read as little-endian float (macmon approach)
        uint32_t intVal = (uint32_t)val.bytes[0] | ((uint32_t)val.bytes[1] << 8) |
                          ((uint32_t)val.bytes[2] << 16) | ((uint32_t)val.bytes[3] << 24);
        float temp;
        memcpy(&temp, &intVal, 4);
        return (double)temp;
      }
    }
  }
  return 0.0;
}

double SMCGetPower(const char *key) {
  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey((char *)key, &val);
  if (result == kIOReturnSuccess) {
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_FPE2) == 0) {
        int intValue = _strtof(val.bytes, val.dataSize, 2);
        return intValue;
      }
    }
  }
  return 0.0;
}

double SMCGetVoltage(const char *key) {
  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey((char *)key, &val);
  if (result == kIOReturnSuccess) {
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_FPE2) == 0) {
        int intValue = _strtof(val.bytes, val.dataSize, 2);
        return intValue / 1000.0; // Convert to volts
      }
    }
  }
  return 0.0;
}

int SMCGetFanNumber() {
  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey((char *)"FNum", &val);
  if (result == kIOReturnSuccess) {
    // read succeeded - check returned value
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_UINT8) == 0) {
        int intValue = _strtoul((char *)val.bytes, val.dataSize, 10);
        return intValue;
      }
    }
  }
  // read failed
  return 0;
}

// Helper function to read fan values (RPM, Min, Max)
int SMCGetFanValue(int fan_number, const char* key_format) {
  SMCVal_t val;
  kern_return_t result;
  UInt32Char_t key;

  snprintf(key, sizeof(key), key_format, fan_number);

  result = SMCReadKey(key, &val);
  if (result == kIOReturnSuccess) {
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_FPE2) == 0) {
        int intValue = _strtof(val.bytes, val.dataSize, 2);
        return intValue;
      }
    }
  }
  return 0;
}

int SMCGetFanRPM(int fan_number) {
  return SMCGetFanValue(fan_number, SMC_PKEY_FAN_RPM);
}

int SMCGetFanMin(int fan_number) {
  return SMCGetFanValue(fan_number, SMC_PKEY_FAN_MIN);
}

int SMCGetFanMax(int fan_number) {
  return SMCGetFanValue(fan_number, SMC_PKEY_FAN_MAX);
}

void Temperature(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double temperature = SMCGetTemperature();
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, temperature));
}

void Fans(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  int numberOfFans = SMCGetFanNumber();
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, numberOfFans));
}

// Helper function for fan callbacks
typedef int (*FanGetterFunc)(int);

void FanCallback(const FunctionCallbackInfo<Value> &args, FanGetterFunc getter) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.Length() < 1) {
    args.GetReturnValue().Set(Undefined(isolate));
    return;
  }

  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Fan number must be a number").ToLocalChecked()));
    return;
  }

  int fanNumber = args[0]->Int32Value(Nan::GetCurrentContext()).ToChecked();
  SMCOpen();
  int value = getter(fanNumber);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, value));
}

void FanRpm(const FunctionCallbackInfo<Value> &args) {
  FanCallback(args, SMCGetFanRPM);
}

void FanMin(const FunctionCallbackInfo<Value> &args) {
  FanCallback(args, SMCGetFanMin);
}

void FanMax(const FunctionCallbackInfo<Value> &args) {
  FanCallback(args, SMCGetFanMax);
}

void CpuTemperatureDie(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();

  ChipGeneration gen = GetChipGeneration();
  const char *key;

  // Select appropriate CPU die key based on chip generation
  switch (gen) {
    case CHIP_INTEL:
      key = "TC0D";
      break;
    case CHIP_M1:
      key = "Tp01";
      break;
    case CHIP_M2:
      key = "Tp01";
      break;
    case CHIP_M3:
      key = "Tf04";
      break;
    case CHIP_M4:
      key = "Tp01";
      break;
    case CHIP_M5:
      key = "Tp01";  // Use M4 key as fallback for M5
      break;
    default:
      key = "TC0D";
      break;
  }

  double temperature = SMCGetTemperatureKey(key);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, temperature));
}

void GpuTemperature(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  // Try IOKit HID sensors first (Apple Silicon) - macmon approach
  // Look for "GPU MTR Temp Sensor" sensors
  IOKitSensorList sensors = GetIOKitTemperatureSensors();
  double total = 0.0;
  int count = 0;

  for (int i = 0; i < sensors.count; i++) {
    const char *name = sensors.sensors[i].name;
    // Check if sensor name starts with "GPU MTR Temp Sensor"
    if (strncmp(name, "GPU MTR Temp Sensor", 19) == 0) {
      double value = sensors.sensors[i].value;
      if (value > 0.0) {
        total += value;
        count++;
      }
    }
  }

  FreeIOKitSensorList(sensors);

  if (count > 0) {
    args.GetReturnValue().Set(Number::New(isolate, total / count));
    return;
  }

  // Fallback to SMC - scan for Tg** keys (macmon approach)
  ChipGeneration gen = GetChipGeneration();

  if (gen == CHIP_INTEL) {
    // Intel: use traditional SMC key
    SMCOpen();
    double temperature = SMCGetTemperatureKey("TG0D");
    SMCClose();
    args.GetReturnValue().Set(Number::New(isolate, temperature));
    return;
  }

  // Initialize GPU cache on first call - optimized scanning
  if (!gpu_temp_keys_initialized) {
    SMCOpen();

    // Strategy: Check common patterns first
    // GPU temp keys typically follow patterns like Tg05, Tg0f, etc.

    // Pattern 1: Numeric keys (0-9)(0-9) - most common
    for (int i = 0; i <= 9; i++) {
      for (int j = 0; j <= 9; j++) {
        char key[5];
        snprintf(key, sizeof(key), "Tg%d%d", i, j);

        SMCVal_t val;
        kern_return_t result = SMCReadKey(key, &val);

        if (result == kIOReturnSuccess && val.dataSize == 4 && strcmp(val.dataType, DATATYPE_FLT) == 0) {
          gpu_temp_keys_cache.push_back(key);
        }
      }
    }

    // Pattern 2: Hex patterns (0-9)(a-f) - common for GPU sensors
    for (int i = 0; i <= 9; i++) {
      for (char j = 'a'; j <= 'f'; j++) {
        char key[5];
        snprintf(key, sizeof(key), "Tg%d%c", i, j);

        SMCVal_t val;
        kern_return_t result = SMCReadKey(key, &val);

        if (result == kIOReturnSuccess && val.dataSize == 4 && strcmp(val.dataType, DATATYPE_FLT) == 0) {
          gpu_temp_keys_cache.push_back(key);
        }
      }
    }

    SMCClose();
    gpu_temp_keys_initialized = true;
  }

  // Read from cached keys
  total = 0.0;
  count = 0;

  SMCOpen();
  for (const auto& key_str : gpu_temp_keys_cache) {
    SMCVal_t val;
    kern_return_t result = SMCReadKey((char*)key_str.c_str(), &val);

    if (result == kIOReturnSuccess && val.dataSize == 4 && strcmp(val.dataType, DATATYPE_FLT) == 0) {
      float temp;
      memcpy(&temp, val.bytes, sizeof(float));

      if (temp > 0.0 && temp < 150.0) {
        total += temp;
        count++;
      }
    }
  }
  SMCClose();

  if (count > 0) {
    args.GetReturnValue().Set(Number::New(isolate, total / count));
    return;
  }

  // If no Tg keys found, return 0
  args.GetReturnValue().Set(Number::New(isolate, 0.0));
}

void GpuUsage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  double usage = 0.0;

  // Get IOAccelerator services
  io_iterator_t iterator;
  io_service_t service;
  kern_return_t kr;

  CFMutableDictionaryRef matchingDict = IOServiceMatching("IOAccelerator");
  if (!matchingDict) {
    args.GetReturnValue().Set(Number::New(isolate, 0.0));
    return;
  }

  kr = IOServiceGetMatchingServices(kIOMainPortDefault, matchingDict, &iterator);
  if (kr != KERN_SUCCESS) {
    args.GetReturnValue().Set(Number::New(isolate, 0.0));
    return;
  }

  // Get the first GPU accelerator
  service = IOIteratorNext(iterator);
  if (service) {
    CFDictionaryRef stats = (CFDictionaryRef)IORegistryEntryCreateCFProperty(
      service, CFSTR("PerformanceStatistics"), kCFAllocatorDefault, 0);

    if (stats) {
      // Try to get Device Utilization %
      CFNumberRef utilization = (CFNumberRef)CFDictionaryGetValue(stats, CFSTR("Device Utilization %"));
      if (!utilization) {
        // Fallback to GPU Activity(%)
        utilization = (CFNumberRef)CFDictionaryGetValue(stats, CFSTR("GPU Activity(%)"));
      }

      if (utilization) {
        int util_value = 0;
        CFNumberGetValue(utilization, kCFNumberIntType, &util_value);
        usage = (double)util_value / 100.0;
        if (usage > 1.0) usage = 1.0;
      }

      CFRelease(stats);
    }

    IOObjectRelease(service);
  }

  IOObjectRelease(iterator);
  args.GetReturnValue().Set(Number::New(isolate, usage));
}

// Cache for IOReport subscription (reused across calls for better performance)
struct IOReportCache {
  IOReportSubscriptionRef subscription;
  CFMutableDictionaryRef subbedChannels;

  IOReportCache() : subscription(NULL), subbedChannels(NULL) {}

  ~IOReportCache() {
    if (subscription) CFRelease(subscription);
    if (subbedChannels) CFRelease(subbedChannels);
  }
};

static IOReportCache* energyModelCache = nullptr;

// Helper to convert energy to watts based on unit
inline double ConvertEnergyToWatts(int64_t value, const char* unit, double duration_s) {
  if (strcmp(unit, "mJ") == 0) {
    return (value / 1000.0) / duration_s;
  } else if (strcmp(unit, "uJ") == 0) {
    return (value / 1000000.0) / duration_s;
  } else if (strcmp(unit, "nJ") == 0) {
    return (value / 1000000000.0) / duration_s;
  }
  return 0.0;
}

// Helper to check if channel name matches pattern
inline bool MatchesChannelPattern(const char* name, const char* pattern) {
  if (!pattern) return true;

  if (strstr(pattern, "CPU") != NULL) {
    // Match "CPU Energy" or anything ending with "CPU Energy" (e.g., "DIE_0_CPU Energy" for Ultra)
    const char* energyPos = strstr(name, "CPU Energy");
    return (energyPos != NULL && energyPos[10] == '\0');
  } else if (strstr(pattern, "GPU") != NULL) {
    // Match exactly "GPU Energy"
    return (strcmp(name, "GPU Energy") == 0);
  }

  return false;
}

// Helper function to read IOReport power metrics (optimized like macmon)
double GetIOReportPower(const char* group, const char* subgroup, const char* channel_pattern) {
  // Initialize cache on first call
  if (!energyModelCache) {
    energyModelCache = new IOReportCache();

    CFStringRef groupRef = CFStringCreateWithCString(kCFAllocatorDefault, group, kCFStringEncodingUTF8);
    CFStringRef subgroupRef = subgroup ? CFStringCreateWithCString(kCFAllocatorDefault, subgroup, kCFStringEncodingUTF8) : NULL;

    CFDictionaryRef channels = IOReportCopyChannelsInGroup(groupRef, subgroupRef, 0, 0, 0);
    CFRelease(groupRef);
    if (subgroupRef) CFRelease(subgroupRef);

    if (!channels) return 0.0;

    CFMutableDictionaryRef channelsMut = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, channels);
    energyModelCache->subscription = IOReportCreateSubscription(NULL, channelsMut, &energyModelCache->subbedChannels, 0, NULL);
    CFRelease(channels);
    CFRelease(channelsMut);

    if (!energyModelCache->subscription) return 0.0;
  }

  // Take multiple samples and average (like macmon does - improves accuracy)
  const int numSamples = 1; // Single sample is fast enough for real-time monitoring
  const int sampleIntervalMs = 100; // 100ms sample interval
  double totalPower = 0.0;

  for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++) {
    // Get current sample
    CFDictionaryRef sample1 = IOReportCreateSamples(energyModelCache->subscription, energyModelCache->subbedChannels, NULL);
    if (!sample1) return 0.0;

    // Wait for measurement interval
    usleep(sampleIntervalMs * 1000);

    // Get second sample
    CFDictionaryRef sample2 = IOReportCreateSamples(energyModelCache->subscription, energyModelCache->subbedChannels, NULL);
    if (!sample2) {
      CFRelease(sample1);
      return 0.0;
    }

    // Calculate delta
    CFDictionaryRef delta = IOReportCreateSamplesDelta(sample1, sample2, NULL);
    CFRelease(sample1);
    CFRelease(sample2);

    if (!delta) return 0.0;

    // Parse results
    double samplePower = 0.0;
    CFArrayRef channels_array = (CFArrayRef)CFDictionaryGetValue(delta, CFSTR("IOReportChannels"));

    if (channels_array && CFGetTypeID(channels_array) == CFArrayGetTypeID()) {
      CFIndex count = CFArrayGetCount(channels_array);

      for (CFIndex i = 0; i < count; i++) {
        CFDictionaryRef channel = (CFDictionaryRef)CFArrayGetValueAtIndex(channels_array, i);
        if (!channel) continue;

        // Get channel name
        CFStringRef channelName = IOReportChannelGetChannelName(channel);
        if (!channelName) continue;

        char name[256] = {0};
        CFStringGetCString(channelName, name, sizeof(name), kCFStringEncodingUTF8);

        // Pattern matching like macmon
        if (!MatchesChannelPattern(name, channel_pattern)) continue;

        // Get unit label
        CFStringRef unit = IOReportChannelGetUnitLabel(channel);
        if (!unit) continue;

        char unitStr[32];
        CFStringGetCString(unit, unitStr, sizeof(unitStr), kCFStringEncodingUTF8);

        // Get value
        int64_t value = IOReportSimpleGetIntegerValue(channel, 0);

        // Convert to watts
        samplePower += ConvertEnergyToWatts(value, unitStr, sampleIntervalMs / 1000.0);
      }
    }

    CFRelease(delta);
    totalPower += samplePower;
  }

  // Return average across samples
  return totalPower / numSamples;
}

void CpuPower(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  // Use IOReport (Apple Silicon) or SMC fallback (Intel)
  double power = GetIOReportPower("Energy Model", NULL, "CPU Energy");

  if (power == 0.0) {
    // Fallback to SMC for Intel Macs
    SMCOpen();
    power = SMCGetPower("PCTR");
    SMCClose();
  }

  args.GetReturnValue().Set(Number::New(isolate, power));
}

void GpuPower(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  // Use IOReport (Apple Silicon) or SMC fallback (Intel)
  double power = GetIOReportPower("Energy Model", NULL, "GPU Energy");

  if (power == 0.0) {
    // Fallback to SMC for Intel Macs
    SMCOpen();
    power = SMCGetPower("PGTR");
    SMCClose();
  }

  args.GetReturnValue().Set(Number::New(isolate, power));
}

void SystemPower(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double power = SMCGetPower("PSTR");
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, power));
}

void CpuVoltage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double voltage = SMCGetVoltage("VC0C");
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, voltage));
}

void GpuVoltage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double voltage = SMCGetVoltage("VG0C");
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, voltage));
}

void MemoryVoltage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double voltage = SMCGetVoltage("VM0R");
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, voltage));
}

// Helper function for sensor callbacks
typedef IOKitSensorList (*SensorGetterFunc)();

void SensorCallback(const FunctionCallbackInfo<Value> &args, SensorGetterFunc getter) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  IOKitSensorList sensors = getter();
  Local<Array> result = Array::New(isolate, sensors.count);

  for (int i = 0; i < sensors.count; i++) {
    Local<Object> sensor = Object::New(isolate);
    sensor->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "name").ToLocalChecked(),
                String::NewFromUtf8(isolate, sensors.sensors[i].name).ToLocalChecked()).Check();
    sensor->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "value").ToLocalChecked(),
                Number::New(isolate, sensors.sensors[i].value)).Check();
    result->Set(isolate->GetCurrentContext(), i, sensor).Check();
  }

  FreeIOKitSensorList(sensors);
  args.GetReturnValue().Set(result);
}

void GetAllTemperatureSensors(const FunctionCallbackInfo<Value> &args) {
  SensorCallback(args, GetIOKitTemperatureSensors);
}

void GetAllVoltageSensors(const FunctionCallbackInfo<Value> &args) {
  SensorCallback(args, GetIOKitVoltageSensors);
}

void GetAllCurrentSensors(const FunctionCallbackInfo<Value> &args) {
  SensorCallback(args, GetIOKitCurrentSensors);
}

// Helper function to get integer value from CFDictionary
int GetIntValue(CFDictionaryRef dict, CFStringRef key, int defaultValue = 0) {
  CFNumberRef value = (CFNumberRef)CFDictionaryGetValue(dict, key);
  if (value && CFGetTypeID(value) == CFNumberGetTypeID()) {
    int result;
    CFNumberGetValue(value, kCFNumberIntType, &result);
    return result;
  }
  return defaultValue;
}

// Helper function to get boolean value from CFDictionary
bool GetBoolValue(CFDictionaryRef dict, CFStringRef key, bool defaultValue = false) {
  CFBooleanRef value = (CFBooleanRef)CFDictionaryGetValue(dict, key);
  if (value && CFGetTypeID(value) == CFBooleanGetTypeID()) {
    return CFBooleanGetValue(value);
  }
  return defaultValue;
}

// Get battery information from IOKit
BatteryInfo GetBatteryInfo() {
  BatteryInfo info = {};
  info.external_connected = false;
  info.battery_installed = false;
  info.is_charging = false;
  info.fully_charged = false;
  info.voltage = 0;
  info.cycle_count = 0;
  info.design_capacity = 0;
  info.max_capacity = 0;
  info.current_capacity = 0;
  info.design_cycle_count = 1000;
  info.time_remaining = 0;
  info.temperature = 0;
  info.amperage = 0;
  info.charge_percent = 0;
  info.health_percent = 0;

  // Get IOPowerSources info
  CFTypeRef powerSourcesInfo = IOPSCopyPowerSourcesInfo();
  if (!powerSourcesInfo) {
    return info;
  }

  CFArrayRef powerSourcesList = IOPSCopyPowerSourcesList(powerSourcesInfo);
  if (!powerSourcesList || CFArrayGetCount(powerSourcesList) == 0) {
    CFRelease(powerSourcesInfo);
    return info;
  }

  CFRelease(powerSourcesList);
  CFRelease(powerSourcesInfo);

  // Get detailed battery info from AppleSmartBattery service
  io_service_t service = IOServiceGetMatchingService(kIOMainPortDefault,
                                                      IOServiceMatching("AppleSmartBattery"));
  if (service) {
    CFMutableDictionaryRef properties = NULL;
    if (IORegistryEntryCreateCFProperties(service, &properties, kCFAllocatorDefault, 0) == KERN_SUCCESS) {
      info.voltage = GetIntValue(properties, CFSTR("Voltage"), 0);
      info.cycle_count = GetIntValue(properties, CFSTR("CycleCount"), 0);
      info.design_capacity = GetIntValue(properties, CFSTR("DesignCapacity"), 0);
      info.design_cycle_count = GetIntValue(properties, CFSTR("DesignCycleCount9C"), 1000);
      info.temperature = GetIntValue(properties, CFSTR("Temperature"), 0);
      info.amperage = GetIntValue(properties, CFSTR("Amperage"), 0);
      info.external_connected = GetBoolValue(properties, CFSTR("ExternalConnected"), false);
      info.battery_installed = GetBoolValue(properties, CFSTR("BatteryInstalled"), false);
      info.is_charging = GetBoolValue(properties, CFSTR("IsCharging"), false);
      info.fully_charged = GetBoolValue(properties, CFSTR("FullyCharged"), false);

      // Use AppleRaw* values for actual mAh (MaxCapacity and CurrentCapacity are percentages)
      info.max_capacity = GetIntValue(properties, CFSTR("AppleRawMaxCapacity"), 0);
      info.current_capacity = GetIntValue(properties, CFSTR("AppleRawCurrentCapacity"), 0);

      // Get the actual percentages that macOS uses
      info.charge_percent = GetIntValue(properties, CFSTR("CurrentCapacity"), 0);
      info.health_percent = GetIntValue(properties, CFSTR("MaxCapacity"), 0);

      // Get TimeRemaining from properties
      int timeRemaining = GetIntValue(properties, CFSTR("TimeRemaining"), 0);
      if (timeRemaining != 65535) {  // 65535 means "calculating"
        info.time_remaining = timeRemaining * 60;  // Convert minutes to seconds
      } else {
        info.time_remaining = 0;
      }

      CFRelease(properties);
    }
    IOObjectRelease(service);
  }

  return info;
}

// Get system information
SystemInfo GetSystemInfo() {
  SystemInfo info = {};

  // Get total memory
  int mib[2] = {CTL_HW, HW_MEMSIZE};
  size_t length = sizeof(info.total_memory);
  sysctl(mib, 2, &info.total_memory, &length, NULL, 0);

  // Get uptime
  struct timeval boottime;
  int mib_boot[2] = {CTL_KERN, KERN_BOOTTIME};
  size_t boot_length = sizeof(boottime);
  if (sysctl(mib_boot, 2, &boottime, &boot_length, NULL, 0) == 0) {
    time_t now = time(NULL);
    info.uptime = now - boottime.tv_sec;
  } else {
    info.uptime = 0;
  }

  // Get macOS version
  char os_release[64];
  size_t os_length = sizeof(os_release);
  sysctlbyname("kern.osrelease", os_release, &os_length, NULL, 0);

  char os_version[64];
  size_t version_length = sizeof(os_version);
  sysctlbyname("kern.osversion", os_version, &version_length, NULL, 0);

  // Get product version (e.g., "14.1.1")
  char product_version[64] = "";
  size_t product_length = sizeof(product_version);
  sysctlbyname("kern.osproductversion", product_version, &product_length, NULL, 0);

  if (strlen(product_version) > 0) {
    snprintf(info.os_version, sizeof(info.os_version), "macOS %s", product_version);
  } else {
    snprintf(info.os_version, sizeof(info.os_version), "macOS %s", os_release);
  }

  // Get hardware model (e.g., "Mac14,6")
  char hw_model[128];
  size_t hw_length = sizeof(hw_model);
  if (sysctlbyname("hw.model", hw_model, &hw_length, NULL, 0) == 0) {
    strncpy(info.hardware_model, hw_model, sizeof(info.hardware_model) - 1);
    info.hardware_model[sizeof(info.hardware_model) - 1] = '\0';
  } else {
    info.hardware_model[0] = '\0';
  }

  // Get model name from CPU brand string
  char cpu_brand[128];
  size_t cpu_size = sizeof(cpu_brand);
  if (sysctlbyname("machdep.cpu.brand_string", &cpu_brand, &cpu_size, NULL, 0) == 0) {
    // Extract just the chip name (e.g., "Apple M3 Pro" from full brand string)
    if (strstr(cpu_brand, "Apple") != NULL) {
      // Find the Apple chip part of the string
      const char* apple_part = strstr(cpu_brand, "Apple");
      strncpy(info.model_name, apple_part, sizeof(info.model_name) - 1);
      info.model_name[sizeof(info.model_name) - 1] = '\0';
      // Clean up any trailing whitespace or extra info
      char* newline = strchr(info.model_name, '\n');
      if (newline) *newline = '\0';
    } else if (strstr(cpu_brand, "Intel") != NULL) {
      strncpy(info.model_name, "Intel", sizeof(info.model_name) - 1);
      info.model_name[sizeof(info.model_name) - 1] = '\0';
    } else {
      strncpy(info.model_name, "Unknown", sizeof(info.model_name) - 1);
      info.model_name[sizeof(info.model_name) - 1] = '\0';
    }
  } else {
    strncpy(info.model_name, "Unknown", sizeof(info.model_name) - 1);
    info.model_name[sizeof(info.model_name) - 1] = '\0';
  }

  // Get serial number using IOKit
  io_service_t platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault,
      IOServiceMatching("IOPlatformExpertDevice"));
  if (platformExpert) {
    CFTypeRef serialNumberAsCFString = IORegistryEntryCreateCFProperty(platformExpert,
        CFSTR(kIOPlatformSerialNumberKey), kCFAllocatorDefault, 0);
    if (serialNumberAsCFString) {
      CFStringGetCString((CFStringRef)serialNumberAsCFString, info.serial_number,
          sizeof(info.serial_number), kCFStringEncodingUTF8);
      CFRelease(serialNumberAsCFString);
    } else {
      info.serial_number[0] = '\0';
    }
    IOObjectRelease(platformExpert);
  } else {
    info.serial_number[0] = '\0';
  }

  // Map macOS version to codename
  int major_version = 0;
  int minor_version = 0;
  sscanf(product_version, "%d.%d", &major_version, &minor_version);

  const char* codename = "Unknown";
  if (major_version == 15) {
    codename = "Sequoia";
  } else if (major_version == 14) {
    codename = "Sonoma";
  } else if (major_version == 13) {
    codename = "Ventura";
  } else if (major_version == 12) {
    codename = "Monterey";
  } else if (major_version == 11) {
    codename = "Big Sur";
  } else if (major_version == 10) {
    if (minor_version == 15) codename = "Catalina";
    else if (minor_version == 14) codename = "Mojave";
    else if (minor_version == 13) codename = "High Sierra";
  } else if (major_version >= 26) {
    // macOS 26.x is Tahoe (internal development version)
    codename = "Tahoe";
  }

  strncpy(info.os_codename, codename, sizeof(info.os_codename) - 1);
  info.os_codename[sizeof(info.os_codename) - 1] = '\0';

  // Get screen size and release date from hardware model database
  info.screen_size[0] = '\0';
  info.release_year[0] = '\0';

  const MacModelInfo* modelInfo = LookupMacModel(hw_model);
  if (modelInfo) {
    // Copy screen size if available
    if (modelInfo->screen_size && modelInfo->screen_size[0] != '\0') {
      strncpy(info.screen_size, modelInfo->screen_size, sizeof(info.screen_size) - 1);
      info.screen_size[sizeof(info.screen_size) - 1] = '\0';
    }

    // Copy release date if available
    if (modelInfo->release_date && modelInfo->release_date[0] != '\0') {
      strncpy(info.release_year, modelInfo->release_date, sizeof(info.release_year) - 1);
      info.release_year[sizeof(info.release_year) - 1] = '\0';
    }
  }

  return info;
}

void GetSystemData(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  SystemInfo info = GetSystemInfo();
  Local<Object> result = Object::New(isolate);

  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "total_memory").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(info.total_memory))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "uptime").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(info.uptime))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "os_version").ToLocalChecked(),
              String::NewFromUtf8(isolate, info.os_version).ToLocalChecked()).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "os_codename").ToLocalChecked(),
              String::NewFromUtf8(isolate, info.os_codename).ToLocalChecked()).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "model_name").ToLocalChecked(),
              String::NewFromUtf8(isolate, info.model_name).ToLocalChecked()).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "hardware_model").ToLocalChecked(),
              String::NewFromUtf8(isolate, info.hardware_model).ToLocalChecked()).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "serial_number").ToLocalChecked(),
              String::NewFromUtf8(isolate, info.serial_number).ToLocalChecked()).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "screen_size").ToLocalChecked(),
              String::NewFromUtf8(isolate, info.screen_size).ToLocalChecked()).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "release_year").ToLocalChecked(),
              String::NewFromUtf8(isolate, info.release_year).ToLocalChecked()).Check();

  args.GetReturnValue().Set(result);
}

// Get RAM usage information
RAMUsage GetRAMUsage() {
  RAMUsage usage = {};

  // Get total memory
  int mib[2] = {CTL_HW, HW_MEMSIZE};
  size_t length = sizeof(usage.total);
  sysctl(mib, 2, &usage.total, &length, NULL, 0);

  // Get VM statistics
  vm_statistics64_data_t vm_stats;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

  if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm_stats, &count) == KERN_SUCCESS) {
    uint64_t page_size = vm_page_size;

    usage.active = vm_stats.active_count * page_size;
    usage.inactive = vm_stats.inactive_count * page_size;
    usage.wired = vm_stats.wire_count * page_size;
    usage.compressed = vm_stats.compressor_page_count * page_size;

    uint64_t speculative = vm_stats.speculative_count * page_size;
    uint64_t purgeable = vm_stats.purgeable_count * page_size;
    uint64_t external = vm_stats.external_page_count * page_size;

    usage.used = usage.active + usage.inactive + speculative + usage.wired + usage.compressed - purgeable - external;
    usage.free = usage.total - usage.used;

    usage.app = usage.used - usage.wired - usage.compressed;
    usage.cache = purgeable + external;
  }

  // Get memory pressure level
  int pressure_level = 0;
  size_t pressure_size = sizeof(pressure_level);
  sysctlbyname("kern.memorystatus_vm_pressure_level", &pressure_level, &pressure_size, NULL, 0);
  usage.pressure_level = pressure_level;

  // Get swap usage
  struct xsw_usage swap_usage;
  size_t swap_size = sizeof(swap_usage);
  if (sysctlbyname("vm.swapusage", &swap_usage, &swap_size, NULL, 0) == 0) {
    usage.swap_total = swap_usage.xsu_total;
    usage.swap_used = swap_usage.xsu_used;
    usage.swap_free = swap_usage.xsu_avail;
  }

  return usage;
}

void GetRAMUsageData(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  RAMUsage usage = GetRAMUsage();
  Local<Object> result = Object::New(isolate);

  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "total").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.total))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "used").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.used))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "free").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.free))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "active").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.active))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "inactive").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.inactive))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "wired").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.wired))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "compressed").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.compressed))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "app").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.app))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "cache").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.cache))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "swap_total").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.swap_total))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "swap_used").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.swap_used))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "swap_free").ToLocalChecked(),
              Number::New(isolate, static_cast<double>(usage.swap_free))).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "pressure_level").ToLocalChecked(),
              Number::New(isolate, usage.pressure_level)).Check();

  args.GetReturnValue().Set(result);
}

void GetBatteryData(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  BatteryInfo info = GetBatteryInfo();
  Local<Object> result = Object::New(isolate);

  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "external_connected").ToLocalChecked(),
              v8::Boolean::New(isolate, info.external_connected)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "battery_installed").ToLocalChecked(),
              v8::Boolean::New(isolate, info.battery_installed)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "is_charging").ToLocalChecked(),
              v8::Boolean::New(isolate, info.is_charging)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "fully_charged").ToLocalChecked(),
              v8::Boolean::New(isolate, info.fully_charged)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "voltage").ToLocalChecked(),
              Number::New(isolate, info.voltage)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "cycle_count").ToLocalChecked(),
              Number::New(isolate, info.cycle_count)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "design_capacity").ToLocalChecked(),
              Number::New(isolate, info.design_capacity)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "max_capacity").ToLocalChecked(),
              Number::New(isolate, info.max_capacity)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "current_capacity").ToLocalChecked(),
              Number::New(isolate, info.current_capacity)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "design_cycle_count").ToLocalChecked(),
              Number::New(isolate, info.design_cycle_count)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "time_remaining").ToLocalChecked(),
              Number::New(isolate, info.time_remaining)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "temperature").ToLocalChecked(),
              Number::New(isolate, info.temperature)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "amperage").ToLocalChecked(),
              Number::New(isolate, info.amperage)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "charge_percent").ToLocalChecked(),
              Number::New(isolate, info.charge_percent)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "health_percent").ToLocalChecked(),
              Number::New(isolate, info.health_percent)).Check();

  args.GetReturnValue().Set(result);
}

// Static variables to track previous CPU ticks for delta calculation
static unsigned long long prev_user_ticks = 0;
static unsigned long long prev_system_ticks = 0;
static unsigned long long prev_idle_ticks = 0;
static unsigned long long prev_nice_ticks = 0;
static bool first_call = true;

// Get CPU usage information
CPUUsage GetCPUUsage() {
  CPUUsage usage = {};

  // Get load averages
  double loadavg[3];
  if (getloadavg(loadavg, 3) == 3) {
    usage.load_avg_1 = loadavg[0];
    usage.load_avg_5 = loadavg[1];
    usage.load_avg_15 = loadavg[2];
  }

  // Get CPU ticks
  host_cpu_load_info_data_t cpuinfo;
  mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

  if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS) {
    unsigned long long user_ticks = cpuinfo.cpu_ticks[CPU_STATE_USER];
    unsigned long long system_ticks = cpuinfo.cpu_ticks[CPU_STATE_SYSTEM];
    unsigned long long idle_ticks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
    unsigned long long nice_ticks = cpuinfo.cpu_ticks[CPU_STATE_NICE];

    // On first call, just store the values and return zeros
    if (first_call) {
      prev_user_ticks = user_ticks;
      prev_system_ticks = system_ticks;
      prev_idle_ticks = idle_ticks;
      prev_nice_ticks = nice_ticks;
      first_call = false;
      usage.user_load = 0.0;
      usage.system_load = 0.0;
      usage.idle_load = 1.0;
      usage.total_usage = 0.0;
    } else {
      // Calculate deltas
      unsigned long long delta_user = user_ticks - prev_user_ticks;
      unsigned long long delta_system = system_ticks - prev_system_ticks;
      unsigned long long delta_idle = idle_ticks - prev_idle_ticks;
      unsigned long long delta_nice = nice_ticks - prev_nice_ticks;

      unsigned long long total_delta = delta_user + delta_system + delta_idle + delta_nice;

      if (total_delta > 0) {
        usage.user_load = (double)delta_user / (double)total_delta;
        usage.system_load = (double)delta_system / (double)total_delta;
        usage.idle_load = (double)delta_idle / (double)total_delta;
        usage.total_usage = 1.0 - usage.idle_load;
      } else {
        // No change in ticks, maintain previous state
        usage.user_load = 0.0;
        usage.system_load = 0.0;
        usage.idle_load = 1.0;
        usage.total_usage = 0.0;
      }

      // Update previous values for next call
      prev_user_ticks = user_ticks;
      prev_system_ticks = system_ticks;
      prev_idle_ticks = idle_ticks;
      prev_nice_ticks = nice_ticks;
    }
  }

  return usage;
}

void GetCPUUsageData(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  CPUUsage usage = GetCPUUsage();
  Local<Object> result = Object::New(isolate);

  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "user_load").ToLocalChecked(),
              Number::New(isolate, usage.user_load)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "system_load").ToLocalChecked(),
              Number::New(isolate, usage.system_load)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "idle_load").ToLocalChecked(),
              Number::New(isolate, usage.idle_load)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "total_usage").ToLocalChecked(),
              Number::New(isolate, usage.total_usage)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "load_avg_1").ToLocalChecked(),
              Number::New(isolate, usage.load_avg_1)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "load_avg_5").ToLocalChecked(),
              Number::New(isolate, usage.load_avg_5)).Check();
  result->Set(isolate->GetCurrentContext(),
              String::NewFromUtf8(isolate, "load_avg_15").ToLocalChecked(),
              Number::New(isolate, usage.load_avg_15)).Check();

  args.GetReturnValue().Set(result);
}

// Get disk information
DiskList GetDiskInfo() {
  DiskList diskList = {};
  diskList.disks = nullptr;
  diskList.count = 0;

  // Get number of mounted file systems
  int numfs = getfsstat(NULL, 0, MNT_NOWAIT);
  if (numfs <= 0) {
    return diskList;
  }

  // Allocate buffer for file system stats
  struct statfs* mounts = (struct statfs*)malloc(numfs * sizeof(struct statfs));
  if (!mounts) {
    return diskList;
  }

  // Get file system stats
  numfs = getfsstat(mounts, numfs * sizeof(struct statfs), MNT_NOWAIT);
  if (numfs <= 0) {
    free(mounts);
    return diskList;
  }

  // Allocate disk array
  diskList.disks = (DiskInfo*)malloc(numfs * sizeof(DiskInfo));
  if (!diskList.disks) {
    free(mounts);
    return diskList;
  }

  // Create DiskArbitration session
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);

  int diskCount = 0;
  for (int i = 0; i < numfs; i++) {
    struct statfs* mount = &mounts[i];

    // Skip non-local file systems
    if (!(mount->f_flags & MNT_LOCAL)) {
      continue;
    }

    // Skip special file systems
    const char* fstype = mount->f_fstypename;
    if (strcmp(fstype, "devfs") == 0 ||
        strcmp(fstype, "autofs") == 0 ||
        strcmp(fstype, "mtmfs") == 0) {
      continue;
    }

    DiskInfo* disk = &diskList.disks[diskCount];
    memset(disk, 0, sizeof(DiskInfo));

    // Get mount point
    strncpy(disk->mount_point, mount->f_mntonname, sizeof(disk->mount_point) - 1);

    // Get file system type
    strncpy(disk->file_system, mount->f_fstypename, sizeof(disk->file_system) - 1);

    // Get size information
    disk->total_size = (uint64_t)mount->f_blocks * (uint64_t)mount->f_bsize;
    disk->free_size = (uint64_t)mount->f_bfree * (uint64_t)mount->f_bsize;
    disk->used_size = disk->total_size - disk->free_size;

    // Get disk name and BSD name using DiskArbitration
    if (session) {
      CFURLRef url = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault,
        (const UInt8*)mount->f_mntonname,
        strlen(mount->f_mntonname),
        true
      );

      if (url) {
        DADiskRef diskRef = DADiskCreateFromVolumePath(kCFAllocatorDefault, session, url);
        if (diskRef) {
          // Get BSD name
          const char* bsdName = DADiskGetBSDName(diskRef);
          if (bsdName) {
            strncpy(disk->bsd_name, bsdName, sizeof(disk->bsd_name) - 1);
          }

          // Get disk description
          CFDictionaryRef desc = DADiskCopyDescription(diskRef);
          if (desc) {
            // Get volume name
            CFStringRef volumeName = (CFStringRef)CFDictionaryGetValue(desc, kDADiskDescriptionVolumeNameKey);
            if (volumeName) {
              CFStringGetCString(volumeName, disk->name, sizeof(disk->name), kCFStringEncodingUTF8);
            }

            // Check if removable
            CFBooleanRef removable = (CFBooleanRef)CFDictionaryGetValue(desc, kDADiskDescriptionMediaRemovableKey);
            if (removable) {
              disk->is_removable = CFBooleanGetValue(removable);
            }

            CFRelease(desc);
          }

          CFRelease(diskRef);
        }
        CFRelease(url);
      }
    }

    // If no volume name, use mount point name
    if (disk->name[0] == '\0') {
      const char* lastSlash = strrchr(mount->f_mntonname, '/');
      if (lastSlash && lastSlash[1] != '\0') {
        strncpy(disk->name, lastSlash + 1, sizeof(disk->name) - 1);
      } else if (strcmp(mount->f_mntonname, "/") == 0) {
        strncpy(disk->name, "Macintosh HD", sizeof(disk->name) - 1);
      }
    }

    diskCount++;
  }

  if (session) {
    CFRelease(session);
  }

  free(mounts);
  diskList.count = diskCount;

  return diskList;
}

void GetDiskData(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  DiskList diskList = GetDiskInfo();
  Local<Array> result = Array::New(isolate, diskList.count);

  for (int i = 0; i < diskList.count; i++) {
    DiskInfo* disk = &diskList.disks[i];
    Local<Object> diskObj = Object::New(isolate);

    diskObj->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "name").ToLocalChecked(),
                String::NewFromUtf8(isolate, disk->name).ToLocalChecked()).Check();
    diskObj->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "mount_point").ToLocalChecked(),
                String::NewFromUtf8(isolate, disk->mount_point).ToLocalChecked()).Check();
    diskObj->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "file_system").ToLocalChecked(),
                String::NewFromUtf8(isolate, disk->file_system).ToLocalChecked()).Check();
    diskObj->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "total_size").ToLocalChecked(),
                Number::New(isolate, disk->total_size)).Check();
    diskObj->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "free_size").ToLocalChecked(),
                Number::New(isolate, disk->free_size)).Check();
    diskObj->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "used_size").ToLocalChecked(),
                Number::New(isolate, disk->used_size)).Check();
    diskObj->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "bsd_name").ToLocalChecked(),
                String::NewFromUtf8(isolate, disk->bsd_name).ToLocalChecked()).Check();
    diskObj->Set(isolate->GetCurrentContext(),
                String::NewFromUtf8(isolate, "is_removable").ToLocalChecked(),
                v8::Boolean::New(isolate, disk->is_removable)).Check();

    result->Set(isolate->GetCurrentContext(), i, diskObj).Check();
  }

  // Free disk list
  if (diskList.disks) {
    free(diskList.disks);
  }

  args.GetReturnValue().Set(result);
}

void Init(v8::Local<Object> exports, v8::Local<v8::Value> module, void* priv) {
  NODE_SET_METHOD(exports, "temperature", Temperature);
  NODE_SET_METHOD(exports, "cpuTemperatureDie", CpuTemperatureDie);
  NODE_SET_METHOD(exports, "gpuTemperature", GpuTemperature);
  NODE_SET_METHOD(exports, "gpuUsage", GpuUsage);
  NODE_SET_METHOD(exports, "fans", Fans);
  NODE_SET_METHOD(exports, "fanRpm", FanRpm);
  NODE_SET_METHOD(exports, "fanMin", FanMin);
  NODE_SET_METHOD(exports, "fanMax", FanMax);
  NODE_SET_METHOD(exports, "cpuPower", CpuPower);
  NODE_SET_METHOD(exports, "gpuPower", GpuPower);
  NODE_SET_METHOD(exports, "systemPower", SystemPower);
  NODE_SET_METHOD(exports, "cpuVoltage", CpuVoltage);
  NODE_SET_METHOD(exports, "gpuVoltage", GpuVoltage);
  NODE_SET_METHOD(exports, "memoryVoltage", MemoryVoltage);
  NODE_SET_METHOD(exports, "getAllTemperatureSensors", GetAllTemperatureSensors);
  NODE_SET_METHOD(exports, "getAllVoltageSensors", GetAllVoltageSensors);
  NODE_SET_METHOD(exports, "getAllCurrentSensors", GetAllCurrentSensors);
  NODE_SET_METHOD(exports, "getBatteryData", GetBatteryData);
  NODE_SET_METHOD(exports, "getSystemData", GetSystemData);
  NODE_SET_METHOD(exports, "getRAMUsageData", GetRAMUsageData);
  NODE_SET_METHOD(exports, "getCPUUsageData", GetCPUUsageData);
  NODE_SET_METHOD(exports, "getDiskData", GetDiskData);
}

NODE_MODULE(smc, Init)
