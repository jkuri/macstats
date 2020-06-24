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
#include <nan.h>
#include <node.h>
#include <stdio.h>
#include <string.h>
#include <v8.h>

#include "smc.h"

using namespace v8;

static io_connect_t conn;

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
  sprintf(str, "%c%c%c%c", (unsigned int)val >> 24, (unsigned int)val >> 16,
          (unsigned int)val >> 8, (unsigned int)val);
}

kern_return_t SMCOpen(void) {
  kern_return_t result;
  io_iterator_t iterator;
  io_object_t device;

  CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
  result = IOServiceGetMatchingServices(kIOMasterPortDefault,
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

double SMCGetTemperature() {
  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey((char *)SMC_KEY_CPU_TEMP, &val);
  if (result == kIOReturnSuccess) {
    // read succeeded - check returned value
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_SP78) == 0) {
        // convert sp78 value to temperature
        int intValue = val.bytes[0] * 256 + (unsigned char)val.bytes[1];
        return intValue / 256.0;
      }
    }
  }
  // read failed
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

  sprintf(key, SMC_PKEY_FAN_RPM, fan_number);

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

void Init(v8::Local<Object> exports) {
  NODE_SET_METHOD(exports, "temperature", Temperature);
  NODE_SET_METHOD(exports, "fans", Fans);
  NODE_SET_METHOD(exports, "fanRpm", FanRpm);
}

NODE_MODULE(smc, Init)
