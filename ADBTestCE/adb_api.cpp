
#include <windows.h>
#include "adb_api.h"
#include "EditLog.h"

#define SPINT_ACTIVE 0x00000001 
#define SPINT_DEFAULT 0x00000002 
#define SPINT_REMOVED 0x00000004

// Android ADB interface identifier
static const GUID kAdbInterfaceId = ANDROID_USB_CLASS_ID;

// Number of interfaces detected in TestEnumInterfaces.

ADBAPIHANDLE hr = 0;
// Constants used to initialize a "handshake" message
#define MAX_PAYLOAD 4096
#define A_SYNC 0x434e5953
#define A_CNXN 0x4e584e43
#define A_OPEN 0x4e45504f
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257
#define A_VERSION 0x01000000



#define ADB_CTL_GET_USB_DEVICE_DESCRIPTOR         10
/// Control code for IOCTL that gets USB_CONFIGURATION_DESCRIPTOR
#define ADB_CTL_GET_USB_CONFIGURATION_DESCRIPTOR  11

/// Control code for IOCTL that gets USB_INTERFACE_DESCRIPTOR
#define ADB_CTL_GET_USB_INTERFACE_DESCRIPTOR      12

/// Control code for IOCTL that gets endpoint information
#define ADB_CTL_GET_ENDPOINT_INFORMATION          13

/// Control code for bulk read IOCTL
#define ADB_CTL_BULK_READ                         14
#define ADB_CTL_GET_ENDPOINT_DESCRIPTORS          14

/// Control code for bulk write IOCTL
#define ADB_CTL_BULK_WRITE                        15

/// Control code for IOCTL that gets device serial number
#define ADB_CTL_GET_SERIAL_NUMBER                 16



/// IOCTL that gets USB_DEVICE_DESCRIPTOR
#define ADB_IOCTL_GET_USB_DEVICE_DESCRIPTOR \
              CTL_CODE(FILE_DEVICE_UNKNOWN, \
                       ADB_CTL_GET_USB_DEVICE_DESCRIPTOR, \
                       METHOD_BUFFERED, \
                       FILE_READ_ACCESS)


/// IOCTL that gets USB_CONFIGURATION_DESCRIPTOR
#define ADB_IOCTL_GET_USB_CONFIGURATION_DESCRIPTOR \
              CTL_CODE(FILE_DEVICE_UNKNOWN, \
                       ADB_CTL_GET_USB_CONFIGURATION_DESCRIPTOR, \
                       METHOD_BUFFERED, \
                       FILE_READ_ACCESS)

/// IOCTL that gets USB_INTERFACE_DESCRIPTOR
#define ADB_IOCTL_GET_USB_INTERFACE_DESCRIPTOR \
              CTL_CODE(FILE_DEVICE_UNKNOWN, \
                       ADB_CTL_GET_USB_INTERFACE_DESCRIPTOR, \
                       METHOD_BUFFERED, \
                       FILE_READ_ACCESS)

/// IOCTL that gets endpoint information
#define ADB_IOCTL_GET_ENDPOINT_INFORMATION \
              CTL_CODE(FILE_DEVICE_UNKNOWN, \
                       ADB_CTL_GET_ENDPOINT_INFORMATION, \
                       METHOD_BUFFERED, \
                       FILE_READ_ACCESS)

/// Bulk read IOCTL
#define ADB_IOCTL_BULK_READ \
              CTL_CODE(FILE_DEVICE_UNKNOWN, \
                       ADB_CTL_BULK_READ, \
                       METHOD_OUT_DIRECT, \
                       FILE_READ_ACCESS)

// For bulk write IOCTL we send request data in the form of AdbBulkTransfer
// structure and output buffer is just ULONG that receives number of bytes
// actually written. Since both of these are tiny we can use buffered I/O
// for this IOCTL.
/// Bulk write IOCTL
#define ADB_IOCTL_BULK_WRITE \
              CTL_CODE(FILE_DEVICE_UNKNOWN, \
                       ADB_CTL_BULK_WRITE, \
                       METHOD_BUFFERED, \
                       FILE_WRITE_ACCESS)

/// IOCTL that gets device serial number
#define ADB_IOCTL_GET_SERIAL_NUMBER \
              CTL_CODE(FILE_DEVICE_UNKNOWN, \
                       ADB_CTL_GET_SERIAL_NUMBER, \
                       METHOD_BUFFERED, \
                       FILE_READ_ACCESS)


#define IOCTL_GET_USB_ADB_ENDPOINT_DESCRIPTOR \
              CTL_CODE(FILE_DEVICE_UNKNOWN, \
                       ADB_CTL_GET_ENDPOINT_DESCRIPTORS, \
                       METHOD_BUFFERED, \
                        FILE_READ_ACCESS)
// Formats message sent to USB device
struct message {
    unsigned int command;       /* command identifier constant      */
    unsigned int arg0;          /* first argument                   */
    unsigned int arg1;          /* second argument                  */
    unsigned int data_length;   /* length of payload (0 is allowed) */
    unsigned int data_crc32;    /* crc32 of data payload            */
    unsigned int magic;         /* command ^ 0xffffffff             */
};


typedef struct __USB_DEVICE_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    USHORT bcdUSB;
    UCHAR bDeviceClass;
    UCHAR bDeviceSubClass;
    UCHAR bDeviceProtocol;
    UCHAR bMaxPacketSize0;
    USHORT idVendor;
    USHORT idProduct;
    USHORT bcdDevice;
    UCHAR iManufacturer;
    UCHAR iProduct;
    UCHAR iSerialNumber;
    UCHAR bNumConfigurations;
} _ST_USB_DEVICE_DESCRIPTOR, *_PUSB_DEVICE_DESCRIPTOR;


typedef struct
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 wTotalLengthL;
    UINT8 wTotalLengthH;
    UINT8 bNumInterfaces;
    UINT8 bConfigurationValue;
    UINT8 iConfiguration;
    UINT8 bmAttributes;
    UINT8 maxPower;
} USB_CONFIGURATION_DESCRIPTOR_;

typedef struct
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 bInterfaceNumber;
    UINT8 bAlternateSetting;
    UINT8 bNumEndpoints;
    UINT8 bInterfaceClass;
    UINT8 bInterfaceSubClass;
    UINT8 bInterfaceProtocol;
    UINT8 iInterface;
} USB_INTERFACE_DESCRIPTOR_;

typedef struct
{
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bEndpointAddress;
    UCHAR bmAttributes;
    USHORT wMaxPacketSize;
    UCHAR bInterval;
} USB_ENDPOINT_DESCRIPTOR_;


struct AdbQueryEndpointInformation {
  /// Zero-based endpoint index for which information is queried.
  /// See ADB_QUERY_BULK_xxx_ENDPOINT_INDEX for shortcuts.
  UCHAR endpoint_index;
};

static ADBAPIHANDLE g_ADBHANDLE;

ADBAPIHANDLE AdbCreateInterfaceByName(const wchar_t* interface_name)
{
	g_ADBHANDLE = CreateFile(
							L"AID1:",
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							0,
							NULL
							);
	if ((g_ADBHANDLE == NULL) || (g_ADBHANDLE == INVALID_HANDLE_VALUE)) {
		//MessageBox(NULL, L"AdbCreateInterfaceByName error", NULL, MB_OK|MB_TOPMOST);
		g_ADBHANDLE = NULL;
	}
	return g_ADBHANDLE;
}

bool AdbGetInterfaceName(ADBAPIHANDLE adb_interface,
                                    void* buffer,
                                    unsigned long* buffer_char_size,
                                    bool ansi)
{
//	logcat(L"AdbGetInterfaceName");
	if (buffer == NULL) {
		*buffer_char_size = strlen("custom interface");
		return true;
	}
	strcpy((char*)buffer, "custom interface");
	*buffer_char_size = strlen("custom interface");
	return true;
}


bool AdbGetSerialNumber(ADBAPIHANDLE adb_interface,
                                   void* buffer,
                                   unsigned long* buffer_char_size,
                                   bool ansi)
{
	logcat(L"AdbGetSerialNumber");
	if (g_ADBHANDLE != NULL) {
		strcpy((char*)buffer, "1234567890");
		*buffer_char_size = strlen("1234567890");
	}

	logcat(L"AdbGetSerialNumber1");
	return true;
}

ADBAPIHANDLE AdbOpenDefaultBulkReadEndpoint(
                            ADBAPIHANDLE adb_interface,
                            AdbOpenAccessType access_type,
                            AdbOpenSharingMode sharing_mode)
{
	return g_ADBHANDLE;
}


ADBAPIHANDLE AdbOpenDefaultBulkWriteEndpoint(
                            ADBAPIHANDLE adb_interface,
                            AdbOpenAccessType access_type,
                            AdbOpenSharingMode sharing_mode)
{
	//logcat(L"AdbOpenDefaultBulkWriteEndpoint");
	return g_ADBHANDLE;
}

bool AdbReadEndpointSync(ADBAPIHANDLE adb_endpoint,
                                    void* buffer,
                                    unsigned long bytes_to_read,
                                    unsigned long* bytes_read,
                                    unsigned long time_out)
{
	wchar_t wLog[1024];

	bool ret = ReadFile(adb_endpoint,buffer,bytes_to_read,(LPDWORD)(bytes_read), NULL) ? true : false;
	_stprintf(wLog, L"AdbReadEndpointSync len = %d", (*bytes_read));
	logcat(wLog);
	if (!ret) {
		
		wchar_t wLog1[1024];
		char* readBuf = (char*)buffer;
		int i = 0;
		for(; i < (*bytes_read); ++i) {
			wLog[i] = readBuf[i];
		}
		wLog[i] = 0;
		_stprintf(wLog1, L"read -- %s", wLog);
		logcat(wLog1);
	} else {
		wchar_t wLog1[1024];
		char* readBuf = (char*)buffer;
		int i = 0;
		for(; i < (*bytes_read); ++i) {
			wLog[i] = readBuf[i];
		}
		wLog[i] = 0;
		_stprintf(wLog1, L"read sucess -- %s", wLog);
		logcat(wLog1);
	}
	//Sleep(1000);
	logcat(L"AdbReadEndpointSync1");
	return ret;
}

bool AdbWriteEndpointSync(ADBAPIHANDLE adb_endpoint,
                                     void* buffer,
                                     unsigned long bytes_to_write,
                                     unsigned long* bytes_written,
                                     unsigned long time_out)
{
	logcat(L"AdbWriteEndpointSync");
	wchar_t wLog[1024] = {0};
	wchar_t wLog1[1024] = {0};
	char* writeBuf = (char*)buffer;
	/*
	int i = 0;
	for(; i < bytes_to_write; ++i) {
		wLog[i] = writeBuf[i];
	}
	wLog[i] = 0;
	_stprintf(wLog1, L"write -- %s", wLog);
	logcat(wLog1);
	*/
	DWORD tick = GetTickCount();
	bool ret = WriteFile(adb_endpoint,buffer,bytes_to_write,(LPDWORD)(bytes_written), NULL) ? true : false;
	DWORD costTick = GetTickCount();
	costTick -= tick;
	_stprintf(wLog1, L"cost --- %d", costTick);
	logcat(wLog1);
	logcat(L"AdbWriteEndpointSync1");
	return ret;
}

bool AdbCloseHandle(ADBAPIHANDLE adb_handle)
{
	CloseHandle(adb_handle);
	return true;
}