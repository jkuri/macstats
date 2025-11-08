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
#include <string.h>
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

// IOKit HID constants
#define kIOHIDEventTypeTemperature 15
#define kIOHIDEventTypePower 25
#define IOHIDEventFieldBase(type) (type << 16)

using namespace v8;

static io_connect_t conn;

// Get detailed Apple Silicon model
AppleSiliconModel GetAppleSiliconModel() {
  static AppleSiliconModel cached_model = MODEL_UNKNOWN;

  if (cached_model != MODEL_UNKNOWN) {
    return cached_model;
  }

  char cpu_brand[128];
  size_t size = sizeof(cpu_brand);

  if (sysctlbyname("machdep.cpu.brand_string", &cpu_brand, &size, NULL, 0) != 0) {
    cached_model = MODEL_INTEL;
    return cached_model;
  }

  // Check if Intel
  if (strstr(cpu_brand, "Intel") != NULL) {
    cached_model = MODEL_INTEL;
    return cached_model;
  }

  // Check if Apple Silicon
  if (strstr(cpu_brand, "Apple") == NULL) {
    cached_model = MODEL_INTEL;
    return cached_model;
  }

  // Detect specific Apple Silicon model
  if (strstr(cpu_brand, "M1") != NULL) {
    if (strstr(cpu_brand, "Ultra") != NULL) {
      cached_model = MODEL_M1_ULTRA;
    } else if (strstr(cpu_brand, "Max") != NULL) {
      cached_model = MODEL_M1_MAX;
    } else if (strstr(cpu_brand, "Pro") != NULL) {
      cached_model = MODEL_M1_PRO;
    } else {
      cached_model = MODEL_M1;
    }
  } else if (strstr(cpu_brand, "M2") != NULL) {
    if (strstr(cpu_brand, "Ultra") != NULL) {
      cached_model = MODEL_M2_ULTRA;
    } else if (strstr(cpu_brand, "Max") != NULL) {
      cached_model = MODEL_M2_MAX;
    } else if (strstr(cpu_brand, "Pro") != NULL) {
      cached_model = MODEL_M2_PRO;
    } else {
      cached_model = MODEL_M2;
    }
  } else if (strstr(cpu_brand, "M3") != NULL) {
    if (strstr(cpu_brand, "Ultra") != NULL) {
      cached_model = MODEL_M3_ULTRA;
    } else if (strstr(cpu_brand, "Max") != NULL) {
      cached_model = MODEL_M3_MAX;
    } else if (strstr(cpu_brand, "Pro") != NULL) {
      cached_model = MODEL_M3_PRO;
    } else {
      cached_model = MODEL_M3;
    }
  } else if (strstr(cpu_brand, "M4") != NULL) {
    if (strstr(cpu_brand, "Ultra") != NULL) {
      cached_model = MODEL_M4_ULTRA;
    } else if (strstr(cpu_brand, "Max") != NULL) {
      cached_model = MODEL_M4_MAX;
    } else if (strstr(cpu_brand, "Pro") != NULL) {
      cached_model = MODEL_M4_PRO;
    } else {
      cached_model = MODEL_M4;
    }
  } else if (strstr(cpu_brand, "M5") != NULL) {
    if (strstr(cpu_brand, "Ultra") != NULL) {
      cached_model = MODEL_M5_ULTRA;
    } else if (strstr(cpu_brand, "Max") != NULL) {
      cached_model = MODEL_M5_MAX;
    } else if (strstr(cpu_brand, "Pro") != NULL) {
      cached_model = MODEL_M5_PRO;
    } else {
      cached_model = MODEL_M5;
    }
  } else {
    // Unknown Apple Silicon model
    cached_model = MODEL_M1; // Default to M1 keys as fallback
  }

  return cached_model;
}

// Get human-readable model name
const char* GetModelName(AppleSiliconModel model) {
  switch (model) {
    case MODEL_INTEL: return "Intel";
    case MODEL_M1: return "Apple M1";
    case MODEL_M1_PRO: return "Apple M1 Pro";
    case MODEL_M1_MAX: return "Apple M1 Max";
    case MODEL_M1_ULTRA: return "Apple M1 Ultra";
    case MODEL_M2: return "Apple M2";
    case MODEL_M2_PRO: return "Apple M2 Pro";
    case MODEL_M2_MAX: return "Apple M2 Max";
    case MODEL_M2_ULTRA: return "Apple M2 Ultra";
    case MODEL_M3: return "Apple M3";
    case MODEL_M3_PRO: return "Apple M3 Pro";
    case MODEL_M3_MAX: return "Apple M3 Max";
    case MODEL_M3_ULTRA: return "Apple M3 Ultra";
    case MODEL_M4: return "Apple M4";
    case MODEL_M4_PRO: return "Apple M4 Pro";
    case MODEL_M4_MAX: return "Apple M4 Max";
    case MODEL_M4_ULTRA: return "Apple M4 Ultra";
    case MODEL_M5: return "Apple M5";
    case MODEL_M5_PRO: return "Apple M5 Pro";
    case MODEL_M5_MAX: return "Apple M5 Max";
    case MODEL_M5_ULTRA: return "Apple M5 Ultra";
    default: return "Unknown";
  }
}

// Check if running on Apple Silicon (backward compatibility)
bool IsAppleSilicon() {
  return GetAppleSiliconModel() != MODEL_INTEL;
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

  CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
  result = IOServiceGetMatchingServices(kIOMainPortDefault,
                                        matchingDictionary, &iterator);
  if (result != kIOReturnSuccess) {
    printf("Error: IOServiceGetMatchingServices() = %08x\n", result);
    return 1;
  }

  device = IOIteratorNext(iterator);
  IOObjectRelease(iterator);
  if (device == 0) {
    printf("Error: no SMC found\n");
    return 1;
  }

  result = IOServiceOpen(device, mach_task_self(), 0, &conn);
  IOObjectRelease(device);
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

// Get average CPU temperature across all cores
double SMCGetTemperature() {
  AppleSiliconModel model = GetAppleSiliconModel();

  if (model == MODEL_INTEL) {
    // Intel: use SMC
    return SMCGetTemperatureKey(SMC_KEY_CPU_TEMP_INTEL);
  }

  // Apple Silicon: use IOKit HID sensors
  IOKitSensorList sensors = GetIOKitTemperatureSensors();

  if (sensors.count == 0) {
    FreeIOKitSensorList(sensors);
    return 0.0;
  }

  // Calculate average of all temperature sensors
  double total = 0.0;
  int valid_count = 0;

  for (int i = 0; i < sensors.count; i++) {
    if (sensors.sensors[i].value > 0.0) {
      total += sensors.sensors[i].value;
      valid_count++;
    }
  }

  FreeIOKitSensorList(sensors);

  if (valid_count > 0) {
    return total / valid_count;
  }

  return 0.0;
}

double SMCGetTemperatureKey(const char *key) {
  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey((char *)key, &val);
  if (result == kIOReturnSuccess) {
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_SP78) == 0) {
        int intValue = val.bytes[0] * 256 + (unsigned char)val.bytes[1];
        return intValue / 256.0;
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

  result = SMCReadKey((char *)SMC_KEY_FAN_NUMBER, &val);
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

int SMCGetFanRPM(int fan_number) {
  SMCVal_t val;
  kern_return_t result;
  UInt32Char_t key;

  snprintf(key, sizeof(key), SMC_PKEY_FAN_RPM, fan_number);

  result = SMCReadKey(key, &val);
  if (result == kIOReturnSuccess) {
    // read succeeded - check returned value
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_FPE2) == 0) {
        int intValue = _strtof(val.bytes, val.dataSize, 2);
        return intValue;
      }
    }
  }
  // read failed
  return 0;
}

int SMCGetFanMin(int fan_number) {
  SMCVal_t val;
  kern_return_t result;
  UInt32Char_t key;

  snprintf(key, sizeof(key), SMC_PKEY_FAN_MIN, fan_number);

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

int SMCGetFanMax(int fan_number) {
  SMCVal_t val;
  kern_return_t result;
  UInt32Char_t key;

  snprintf(key, sizeof(key), SMC_PKEY_FAN_MAX, fan_number);

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

void FanRpm(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  if (args.Length() < 1) {
    // Fan number (id) isn't specified
    args.GetReturnValue().Set(Undefined(isolate));
    return;
  }
  if (!args[0]->IsNumber()) {
    size_t size = 100;
    char *CharBuff = new char[size + 1];
    v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
        isolate, CharBuff, v8::NewStringType::kNormal, static_cast<int>(size));
    isolate->ThrowException(Exception::TypeError(result.ToLocalChecked()));
    return;
  }
  int fanNumber = args[0]->Int32Value(Nan::GetCurrentContext()).ToChecked();
  SMCOpen();
  int rpm = SMCGetFanRPM(fanNumber);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, rpm));
}

void FanMin(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  if (args.Length() < 1) {
    args.GetReturnValue().Set(Undefined(isolate));
    return;
  }
  if (!args[0]->IsNumber()) {
    size_t size = 100;
    char *CharBuff = new char[size + 1];
    v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
        isolate, CharBuff, v8::NewStringType::kNormal, static_cast<int>(size));
    isolate->ThrowException(Exception::TypeError(result.ToLocalChecked()));
    return;
  }
  int fanNumber = args[0]->Int32Value(Nan::GetCurrentContext()).ToChecked();
  SMCOpen();
  int min = SMCGetFanMin(fanNumber);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, min));
}

void FanMax(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  if (args.Length() < 1) {
    args.GetReturnValue().Set(Undefined(isolate));
    return;
  }
  if (!args[0]->IsNumber()) {
    size_t size = 100;
    char *CharBuff = new char[size + 1];
    v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
        isolate, CharBuff, v8::NewStringType::kNormal, static_cast<int>(size));
    isolate->ThrowException(Exception::TypeError(result.ToLocalChecked()));
    return;
  }
  int fanNumber = args[0]->Int32Value(Nan::GetCurrentContext()).ToChecked();
  SMCOpen();
  int max = SMCGetFanMax(fanNumber);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, max));
}

void CpuTemperatureDie(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  const char *key = IsAppleSilicon() ? SMC_KEY_CPU_PCORE1_M1 : SMC_KEY_CPU_DIE_INTEL;
  double temperature = SMCGetTemperatureKey(key);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, temperature));
}

void GpuTemperature(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();

  AppleSiliconModel model = GetAppleSiliconModel();
  const char *key;

  // Select appropriate GPU key based on model
  switch (model) {
    case MODEL_INTEL:
      key = SMC_KEY_GPU_TEMP_INTEL;
      break;
    case MODEL_M1:
    case MODEL_M1_PRO:
    case MODEL_M1_MAX:
    case MODEL_M1_ULTRA:
      key = SMC_KEY_GPU1_M1;
      break;
    case MODEL_M2:
    case MODEL_M2_PRO:
    case MODEL_M2_MAX:
    case MODEL_M2_ULTRA:
      key = SMC_KEY_GPU1_M2;
      break;
    case MODEL_M3:
    case MODEL_M3_PRO:
    case MODEL_M3_MAX:
    case MODEL_M3_ULTRA:
      key = SMC_KEY_GPU1_M3;
      break;
    case MODEL_M4:
    case MODEL_M4_PRO:
    case MODEL_M4_MAX:
    case MODEL_M4_ULTRA:
      key = SMC_KEY_GPU1_M4;
      break;
    case MODEL_M5:
    case MODEL_M5_PRO:
    case MODEL_M5_MAX:
    case MODEL_M5_ULTRA:
      key = SMC_KEY_GPU1_M5;
      break;
    default:
      key = SMC_KEY_GPU_TEMP_INTEL;
      break;
  }

  double temperature = SMCGetTemperatureKey(key);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, temperature));
}

void CpuPower(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double power = SMCGetPower(SMC_KEY_CPU_POWER);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, power));
}

void GpuPower(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double power = SMCGetPower(SMC_KEY_GPU_POWER);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, power));
}

void SystemPower(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double power = SMCGetPower(SMC_KEY_SYSTEM_POWER);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, power));
}

void CpuVoltage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double voltage = SMCGetVoltage(SMC_KEY_CPU_VOLTAGE);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, voltage));
}

void GpuVoltage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double voltage = SMCGetVoltage(SMC_KEY_GPU_VOLTAGE);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, voltage));
}

void MemoryVoltage(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SMCOpen();
  double voltage = SMCGetVoltage(SMC_KEY_MEMORY_VOLTAGE);
  SMCClose();
  args.GetReturnValue().Set(Number::New(isolate, voltage));
}

void GetModelInfo(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  AppleSiliconModel model = GetAppleSiliconModel();
  const char* model_name = GetModelName(model);

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, model_name).ToLocalChecked());
}

void GetAllTemperatureSensors(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  IOKitSensorList sensors = GetIOKitTemperatureSensors();
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

void GetAllVoltageSensors(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  IOKitSensorList sensors = GetIOKitVoltageSensors();
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

void GetAllCurrentSensors(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  IOKitSensorList sensors = GetIOKitCurrentSensors();
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

  // Get model name
  AppleSiliconModel model = GetAppleSiliconModel();
  const char* model_name = GetModelName(model);
  strncpy(info.model_name, model_name, sizeof(info.model_name) - 1);
  info.model_name[sizeof(info.model_name) - 1] = '\0';

  // Get hardware model (e.g., "Mac14,6")
  char hw_model[128];
  size_t hw_length = sizeof(hw_model);
  if (sysctlbyname("hw.model", hw_model, &hw_length, NULL, 0) == 0) {
    strncpy(info.hardware_model, hw_model, sizeof(info.hardware_model) - 1);
    info.hardware_model[sizeof(info.hardware_model) - 1] = '\0';
  } else {
    info.hardware_model[0] = '\0';
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

  // Get screen size and release date from hardware model
  // This is a mapping of known models - not exhaustive
  info.screen_size[0] = '\0';
  info.release_year[0] = '\0';

  // M3 MacBook Pro models (Nov 2023)
  if (strstr(hw_model, "Mac15,3") != NULL || strstr(hw_model, "Mac15,6") != NULL) {
    strcpy(info.screen_size, "14-inch");
    strcpy(info.release_year, "Nov 2023");
  } else if (strstr(hw_model, "Mac15,7") != NULL || strstr(hw_model, "Mac15,8") != NULL ||
             strstr(hw_model, "Mac15,9") != NULL || strstr(hw_model, "Mac15,10") != NULL ||
             strstr(hw_model, "Mac15,11") != NULL) {
    strcpy(info.screen_size, "16-inch");
    strcpy(info.release_year, "Nov 2023");
  }
  // M3 MacBook Air models (Mar 2024)
  else if (strstr(hw_model, "Mac15,12") != NULL) {
    strcpy(info.screen_size, "13-inch");
    strcpy(info.release_year, "Mar 2024");
  } else if (strstr(hw_model, "Mac15,13") != NULL) {
    strcpy(info.screen_size, "15-inch");
    strcpy(info.release_year, "Mar 2024");
  }
  // M4 MacBook Pro models (Nov 2024)
  else if (strstr(hw_model, "Mac16,1") != NULL || strstr(hw_model, "Mac16,6") != NULL) {
    strcpy(info.screen_size, "14-inch");
    strcpy(info.release_year, "Nov 2024");
  } else if (strstr(hw_model, "Mac16,10") != NULL || strstr(hw_model, "Mac16,11") != NULL) {
    strcpy(info.screen_size, "16-inch");
    strcpy(info.release_year, "Nov 2024");
  }
  // M2 MacBook Pro models
  else if (strstr(hw_model, "Mac14,5") != NULL || strstr(hw_model, "Mac14,9") != NULL) {
    strcpy(info.screen_size, "14-inch");
    strcpy(info.release_year, "Jan 2023");
  } else if (strstr(hw_model, "Mac14,6") != NULL || strstr(hw_model, "Mac14,10") != NULL) {
    strcpy(info.screen_size, "16-inch");
    strcpy(info.release_year, "Jan 2023");
  } else if (strstr(hw_model, "Mac14,7") != NULL) {
    strcpy(info.screen_size, "13-inch");
    strcpy(info.release_year, "Jun 2022");
  }
  // M2 MacBook Air models
  else if (strstr(hw_model, "Mac14,2") != NULL) {
    strcpy(info.screen_size, "13-inch");
    strcpy(info.release_year, "Jun 2022");
  } else if (strstr(hw_model, "Mac14,15") != NULL) {
    strcpy(info.screen_size, "15-inch");
    strcpy(info.release_year, "Jun 2023");
  }
  // M1 MacBook Pro models
  else if (strstr(hw_model, "Mac14,12") != NULL || strstr(hw_model, "Mac14,14") != NULL) {
    strcpy(info.screen_size, "14-inch");
    strcpy(info.release_year, "Oct 2021");
  } else if (strstr(hw_model, "Mac14,13") != NULL) {
    strcpy(info.screen_size, "16-inch");
    strcpy(info.release_year, "Oct 2021");
  } else if (strstr(hw_model, "Mac14,1") != NULL) {
    strcpy(info.screen_size, "13-inch");
    strcpy(info.release_year, "Nov 2020");
  }
  // M1 MacBook Air
  else if (strstr(hw_model, "MacBookAir10,1") != NULL) {
    strcpy(info.screen_size, "13-inch");
    strcpy(info.release_year, "Nov 2020");
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
    unsigned long long totalTicks = 0;
    for (int i = 0; i < CPU_STATE_MAX; i++) {
      totalTicks += cpuinfo.cpu_ticks[i];
    }

    if (totalTicks > 0) {
      usage.user_load = (double)cpuinfo.cpu_ticks[CPU_STATE_USER] / (double)totalTicks;
      usage.system_load = (double)cpuinfo.cpu_ticks[CPU_STATE_SYSTEM] / (double)totalTicks;
      usage.idle_load = (double)cpuinfo.cpu_ticks[CPU_STATE_IDLE] / (double)totalTicks;
      usage.total_usage = 1.0 - usage.idle_load;
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
  NODE_SET_METHOD(exports, "getModelInfo", GetModelInfo);
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
