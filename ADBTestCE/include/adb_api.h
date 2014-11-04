/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_USB_API_ADBWINAPI_H__
#define ANDROID_USB_API_ADBWINAPI_H__

#include <initguid.h>  
#include <win32_adb.h>
#include <windows.h>
#include <winerror.h>
//#include <errno.h>
#include <usb100.h>
#include <adb_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

/** \file
  This file consists of declarations of routines exported by the API as well
  as types, structures, and constants definitions used in the API.
*/

// Enables compillation for "straight" C
#ifdef __cplusplus
  #define EXTERN_C    extern "C"
#else
  #define EXTERN_C    extern
  typedef int bool;
  #define true  1
  #define false 0
#endif

/** \brief Enumerates ADB endpoint types.

  This enum is taken from WDF_USB_PIPE_TYPE enum found in WDK.
*/
typedef enum _AdbEndpointType {
  /// Unknown (invalid, or not initialized) endpoint type.
  AdbEndpointTypeInvalid = 0,

  /// Endpoint is device control pipe.
  AdbEndpointTypeControl,

  /// Endpoint is isochronous r/w pipe.
  AdbEndpointTypeIsochronous,

  /// Endpoint is a bulk r/w pipe.
  AdbEndpointTypeBulk,

  /// Endpoint is an interrupt r/w pipe.
  AdbEndpointTypeInterrupt,
} AdbEndpointType;

/** \brief Endpoint desriptor.

  This structure is based on WDF_USB_PIPE_INFORMATION structure found in WDK.
*/
typedef struct _AdbEndpointInformation {
  /// Maximum packet size this endpoint is capable of.
  unsigned long max_packet_size;

  /// Maximum size of one transfer which should be sent to the host controller.
  unsigned long max_transfer_size;

  /// ADB endpoint type.
  AdbEndpointType endpoint_type;

  /// Raw endpoint address on the device as described by its descriptor.
  unsigned char endpoint_address;

  /// Polling interval.
  unsigned char polling_interval;

  /// Which alternate setting this structure is relevant for.
  unsigned char setting_index;
} AdbEndpointInformation;

/// Shortcut to default write bulk endpoint in zero-based endpoint index API.
#define ADB_QUERY_BULK_WRITE_ENDPOINT_INDEX  0xFC

/// Shortcut to default read bulk endpoint in zero-based endpoint index API.
#define ADB_QUERY_BULK_READ_ENDPOINT_INDEX  0xFE

// {F72FE0D4-CBCB-407d-8814-9ED673D0DD6B}
/// Our USB class id that driver uses to register our device.
#define ANDROID_USB_CLASS_ID \
{0xf72fe0d4, 0xcbcb, 0x407d, {0x88, 0x14, 0x9e, 0xd6, 0x73, 0xd0, 0xdd, 0x6b}};

/// Defines vendor ID for HCT devices.
#define DEVICE_VENDOR_ID                  0x0BB4

/// Defines product ID for the device with single interface.
#define DEVICE_SINGLE_PRODUCT_ID          0x0C01

/// Defines product ID for the Dream composite device.
#define DEVICE_COMPOSITE_PRODUCT_ID       0x0C02

/// Defines product ID for the Magic composite device.
#define DEVICE_MAGIC_COMPOSITE_PRODUCT_ID 0x0C03

/// Defines interface ID for the device.
#define DEVICE_INTERFACE_ID               0x01

/// Defines vendor ID for the device
#define DEVICE_EMULATOR_VENDOR_ID         0x18D1

/// Defines product ID for a SoftUSB device simulator that is used to test
/// the driver in isolation from hardware.
#define DEVICE_EMULATOR_PROD_ID           0xDDDD

// The following ifdef block is the standard way of creating macros which make
// exporting  from a DLL simpler. All files within this DLL are compiled with
// the ADBWIN_EXPORTS symbol defined on the command line. this symbol should
// not be defined on any project that uses this DLL. This way any other project
// whose source files include this file see ADBWIN_API functions as being
// imported from a DLL, whereas this DLL sees symbols defined with this macro
// as being exported.
#ifdef ADBWIN_EXPORTS
#define ADBWIN_API EXTERN_C __declspec(dllexport)
#define ADBWIN_API_CLASS     __declspec(dllexport)
#else
#define ADBWIN_API EXTERN_C __declspec(dllimport)
#define ADBWIN_API_CLASS     __declspec(dllimport)
#endif

/** \brief Handle to an API object.

  To access USB interface and its components clients must first obtain a
  handle to the required object. API Objects that are represented by a
  handle are:
  1. Interface enumerator that provides access to a list of interfaces that
     match certain criterias that were specified when interface enumerator
     has been created. This handle is created in AdbEnumInterfaces routine.
  2. Interface that is the major object this API deals with. In Windows
     model of the USB stack each USB device (that is physical device,
     attached to a USB port) exposes one or more interfaces that become the
     major entities through which that device gets accessed. Each of these
     interfaces are represented as Windows Device Objects on the USB stack.
     So, to this extent, at least as this API is concerned, terms "interface"
     and "device" are interchangeable, since each interface is represented by
     a device object on the Windows USB stack. This handle is created in
     either AdbCreateInterface or AdbCreateInterfaceByName routines.
  3. Endpoint object (also called a pipe) represents an endpoint on interface
     through which all I/O operations are performed. This handle is created in
     one of these routines: AdbOpenEndpoint, AdbOpenDefaultBulkReadEndpoint,
     or AdbOpenDefaultBulkWriteEndpoint.
  4. I/O completion object that tracks completion information of asynchronous
     I/O performed on an endpoint. When an endpoint object gets opened through
     this API it is opened for asynchronous (or overlapped) I/O. And each time
     an asynchronous I/O is performed by this API an I/O completion object is
     created to track the result of that I/O when it gets completed. Clients
     of the API can then use a handle to I/O completion object to query for
     the status and result of asynchronous I/O as well as wait for this I/O
     completion. This handle is created in one of these routines:
     AdbReadEndpointAsync, or AdbWriteEndpointAsync.
  After object is no longer needed by the client, its handle must be closed
  using AdbCloseHandle routine.
*/
typedef void* ADBAPIHANDLE;

/** \brief Defines access type with which an I/O object (endpoint)
  should be opened.
*/
typedef enum _AdbOpenAccessType {
  /// Opens for read and write access.
  AdbOpenAccessTypeReadWrite,

  /// Opens for read only access.
  AdbOpenAccessTypeRead,

  /// Opens for write only access.
  AdbOpenAccessTypeWrite,

  /// Opens for querying information.
  AdbOpenAccessTypeQueryInfo,
} AdbOpenAccessType;

/** \brief Defines sharing mode with which an I/O object (endpoint)
  should be opened.
*/
typedef enum _AdbOpenSharingMode {
  /// Shares read and write.
  AdbOpenSharingModeReadWrite,

  /// Shares only read.
  AdbOpenSharingModeRead,

  /// Shares only write.
  AdbOpenSharingModeWrite,

  /// Opens exclusive.
  AdbOpenSharingModeExclusive,
} AdbOpenSharingMode;

/** \brief Provides information about an interface.
*/
typedef struct _AdbInterfaceInfo {
  /// Inteface's class id (see SP_DEVICE_INTERFACE_DATA for details)
  GUID          class_id;

  /// Interface flags (see SP_DEVICE_INTERFACE_DATA for details)
  unsigned long flags;

  /// Device name for the interface (see SP_DEVICE_INTERFACE_DETAIL_DATA
  /// for details)
  wchar_t       device_name[1];
} AdbInterfaceInfo;




/** \brief Creates USB interface object

  This routine creates an object that represents a USB interface.
  @param[in] interface_name Name of the interface.
  @return Handle to the interface object or NULL on failure. If NULL is
          returned GetLastError() provides extended error information.
*/
ADBWIN_API ADBAPIHANDLE __cdecl AdbCreateInterfaceByName(const wchar_t* interface_name);


/** \brief Gets interface name.

  @param[in] adb_interface A handle to interface object created with 
         AdbCreateInterface call.
  @param[out] buffer Buffer for the name. Can be NULL in which case
         buffer_char_size will contain number of characters required for
         the name.
  @param[in,out] buffer_char_size On the way in supplies size (in characters)
         of the buffer. On the way out, if method failed and GetLastError
         reports ERROR_INSUFFICIENT_BUFFER, will contain number of characters
         required for the name.
  @param[in] ansi If true the name will be returned as single character
         string. Otherwise name will be returned as wide character string.
  @return true on success, false on failure. If false is returned
          GetLastError() provides extended error information.
*/
ADBWIN_API bool __cdecl AdbGetInterfaceName(ADBAPIHANDLE adb_interface,
                                    void* buffer,
                                    unsigned long* buffer_char_size,
                                    bool ansi);

/** \brief Gets serial number for interface's device.

  @param[in] adb_interface A handle to interface object created with 
         AdbCreateInterface call.
  @param[out] buffer Buffer for the serail number string. Can be NULL in which
         case buffer_char_size will contain number of characters required for
         the string.
  @param[in,out] buffer_char_size On the way in supplies size (in characters)
         of the buffer. On the way out, if method failed and GetLastError
         reports ERROR_INSUFFICIENT_BUFFER, will contain number of characters
         required for the name.
  @param[in] ansi If true the name will be returned as single character
         string. Otherwise name will be returned as wide character string.
  @return true on success, false on failure. If false is returned
          GetLastError() provides extended error information.
*/
ADBWIN_API bool __cdecl AdbGetSerialNumber(ADBAPIHANDLE adb_interface,
                                   void* buffer,
                                   unsigned long* buffer_char_size,
                                   bool ansi);






/** \brief Opens default bulk read endpoint on the given interface.

  Endpoints are always opened for overlapped I/O.
  @param[in] adb_interface A handle to interface object created with 
         AdbCreateInterface call.
  @param[in] access_type Desired access type. In the current implementation
         this parameter has no effect on the way endpoint is opened. It's
         always read / write access.
  @param[in] sharing_mode Desired share mode. In the current implementation
         this parameter has no effect on the way endpoint is opened. It's
         always shared for read / write.
  @return Handle to the opened endpoint object or NULL on failure. If NULL is
          returned GetLastError() provides extended error information.
*/
ADBWIN_API ADBAPIHANDLE __cdecl AdbOpenDefaultBulkReadEndpoint(
                            ADBAPIHANDLE adb_interface,
                            AdbOpenAccessType access_type,
                            AdbOpenSharingMode sharing_mode);

/** \brief Opens default bulk write endpoint on the given interface.

  Endpoints are always opened for overlapped I/O.
  @param[in] adb_interface A handle to interface object created with 
         AdbCreateInterface call.
  @param[in] access_type Desired access type. In the current implementation
         this parameter has no effect on the way endpoint is opened. It's
         always read / write access.
  @param[in] sharing_mode Desired share mode. In the current implementation
         this parameter has no effect on the way endpoint is opened. It's
         always shared for read / write.
  @return Handle to the opened endpoint object or NULL on failure. If NULL is
          returned GetLastError() provides extended error information.
*/
ADBWIN_API ADBAPIHANDLE __cdecl AdbOpenDefaultBulkWriteEndpoint(
                            ADBAPIHANDLE adb_interface,
                            AdbOpenAccessType access_type,
                            AdbOpenSharingMode sharing_mode);



/** \brief Synchronously reads from the given endpoint.

  @param[in] adb_endpoint A handle to opened endpoint object, obtained via one
         of the AdbOpenXxxEndpoint calls.
  @param[out] buffer Pointer to the buffer that receives the data.
  @param[in] bytes_to_read Number of bytes to be read.
  @param[out] bytes_read Number of bytes read. Can be NULL.
  @param[in] time_out A timeout (in milliseconds) required for this I/O to
         complete. Zero value for this parameter means that there is no
         timeout for this I/O.
  @return true on success and false on failure. If false is
          returned GetLastError() provides extended error information.
*/
ADBWIN_API bool __cdecl AdbReadEndpointSync(ADBAPIHANDLE adb_endpoint,
                                    void* buffer,
                                    unsigned long bytes_to_read,
                                    unsigned long* bytes_read,
                                    unsigned long time_out);

/** \brief Synchronously writes to the given endpoint.

  @param[in] adb_endpoint A handle to opened endpoint object, obtained via one
         of the AdbOpenXxxEndpoint calls.
  @param[in] buffer Pointer to the buffer containing the data to be written.
  @param[in] bytes_to_write Number of bytes to be written.
  @param[out] bytes_written Number of bytes written. Can be NULL.
  @param[in] time_out A timeout (in milliseconds) required for this I/O to
         complete. Zero value for this parameter means that there is no
         timeout for this I/O.
  @return true on success and false on failure. If false is
          returned GetLastError() provides extended error information.
*/
ADBWIN_API bool __cdecl AdbWriteEndpointSync(ADBAPIHANDLE adb_endpoint,
                                     void* buffer,
                                     unsigned long bytes_to_write,
                                     unsigned long* bytes_written,
                                     unsigned long time_out);


/** \brief Closes handle previously opened with one of the API calls

  @param[in] adb_handle ADB handle previously opened with one of the API calls
  @return true on success or false on failure. If false is returned
          GetLastError() provides extended error information.
*/
ADBWIN_API bool __cdecl AdbCloseHandle(ADBAPIHANDLE adb_handle);

#endif  // ANDROID_USB_API_ADBWINAPI_H__
