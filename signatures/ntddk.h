
typedef void *PVOID;
typedef unsigned int POOL_TYPE;
typedef unsigned int SIZE_T;
typedef unsigned int ULONG;
typedef ULONG *PULONG;
typedef unsigned long long ULONGLONG;
typedef const short *PCWSTR;
typedef short *PWSTR;
typedef unsigned int NTSTATUS;
typedef void VOID;
typedef unsigned long long LARGE_INTEGER;

typedef unsigned short USHORT;
typedef unsigned short WCHAR;
typedef unsigned char UCHAR;

struct UNICODE_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PWSTR	 Buffer;
};

struct RTL_OSVERSIONINFOEXW {
  ULONG	 dwOSVersionInfoSize;
  ULONG	 dwMajorVersion;
  ULONG	 dwMinorVersion;
  ULONG	 dwBuildNumber;
  ULONG	 dwPlatformId;
  WCHAR	 szCSDVersion[ 128 ];
  USHORT  wServicePackMajor;
  USHORT  wServicePackMinor;
  USHORT  wSuiteMask;
  UCHAR	 wProductType;
  UCHAR	 wReserved;
};

typedef RTL_OSVERSIONINFOEXW *PRTL_OSVERSIONINFOEXW;

typedef UNICODE_STRING *PUNICODE_STRING;

PVOID 
  ExAllocatePoolWithTag(
	POOL_TYPE  PoolType,
	SIZE_T	NumberOfBytes,
	ULONG  Tag
	);

void 
  RtlInitUnicodeString(
	PUNICODE_STRING	 DestinationString,
	PCWSTR	SourceString
	);

NTSTATUS 
  IoCreateDevice(
	PDRIVER_OBJECT	DriverObject,
	ULONG  DeviceExtensionSize,
	PUNICODE_STRING	 DeviceName,
	DEVICE_TYPE	 DeviceType,
	ULONG  DeviceCharacteristics,
	BOOLEAN	 Exclusive,
	PDEVICE_OBJECT	*DeviceObject
	);

MM_SYSTEMSIZE 
  MmQuerySystemSize(
	);

void 
  KeInitializeTimer(
	PKTIMER	 Timer
	);

typedef void
  CustomDpc(
	KDPC  *Dpc,
	PVOID  DeferredContext,
	PVOID  SystemArgument1,
	PVOID  SystemArgument2
	);

void 
  KeInitializeDpc(
	KDPC  *Dpc,
	PKDEFERRED_ROUTINE	DeferredRoutine,
	PVOID  DeferredContext
	);

BOOLEAN 
  KeSetTimer(
	PKTIMER	 Timer,
	LARGE_INTEGER  DueTime,
	KDPC  *Dpc
	);

void
  KeInitializeEvent(
	PRKEVENT  Event,
	EVENT_TYPE	Type,
	BOOLEAN	 State
	);

NTSTATUS 
  ExInitializeResourceLite(
	PERESOURCE	Resource
	);

typedef VOID
(*PDRIVER_REINITIALIZE)(
	PDRIVER_OBJECT DriverObject,
	PVOID Context,
	ULONG Count
	);

void
  IoRegisterDriverReinitialization(
	PDRIVER_OBJECT	DriverObject,
	PDRIVER_REINITIALIZE  DriverReinitializationRoutine,
	PVOID  Context
	);

void
  ExFreePoolWithTag(
	PVOID  P,
	ULONG  Tag 
	); 

void
  KeBugCheck(
	ULONG  BugCheckCode
	);

NTSTATUS
  RtlVerifyVersionInfo(
	PRTL_OSVERSIONINFOEXW  VersionInfo,
	ULONG  TypeMask,
	ULONGLONG  ConditionMask
	);

BOOLEAN 
  MmIsThisAnNtAsSystem(
	);

PEPROCESS 
  IoGetCurrentProcess(
	);

NTSTATUS 
  RtlCreateSecurityDescriptor(
	PSECURITY_DESCRIPTOR  SecurityDescriptor,
	ULONG  Revision
	);

NTSTATUS 
  RtlSetDaclSecurityDescriptor(
	PSECURITY_DESCRIPTOR  SecurityDescriptor,
	BOOLEAN	 DaclPresent,
	PACL  Dacl,
	BOOLEAN	 DaclDefaulted
	);

typedef unsigned int ACCESS_MASK;

struct GENERIC_MAPPING {
	ACCESS_MASK GenericRead;
	ACCESS_MASK GenericWrite;
	ACCESS_MASK GenericExecute;
	ACCESS_MASK GenericAll;
};

typedef GENERIC_MAPPING *PGENERIC_MAPPING;

PGENERIC_MAPPING 
  IoGetFileObjectGenericMapping(
	);

NTSTATUS 
  SeAssignSecurity(
	PSECURITY_DESCRIPTOR  ParentDescriptor,
	PSECURITY_DESCRIPTOR  ExplicitDescriptor,
	PSECURITY_DESCRIPTOR  *NewDescriptor,
	BOOLEAN	 IsDirectoryObject,
	PSECURITY_SUBJECT_CONTEXT  SubjectContext,
	PGENERIC_MAPPING  GenericMapping,
	POOL_TYPE  PoolType
	);

ULONG 
  RtlLengthSecurityDescriptor(
	PSECURITY_DESCRIPTOR  SecurityDescriptor
	);

void
  ExRaiseStatus(
	NTSTATUS  Status
	);

void
  KeQuerySystemTime(
	PLARGE_INTEGER	CurrentTime
	);

NTSTATUS 
  ZwOpenKey(
	PHANDLE	 KeyHandle,
	ACCESS_MASK	 DesiredAccess,
	POBJECT_ATTRIBUTES	ObjectAttributes
	);

NTSTATUS 
  ZwQueryValueKey(
	HANDLE	KeyHandle,
	PUNICODE_STRING	 ValueName,
	KEY_VALUE_INFORMATION_CLASS	 KeyValueInformationClass,
	PVOID  KeyValueInformation,
	ULONG  Length,
	PULONG	ResultLength
	);

NTSTATUS 
  ZwClose(
	HANDLE	Handle
	);

typedef PVOID
ALLOCATE_FUNCTION (
	POOL_TYPE PoolType,
	SIZE_T	NumberOfBytes,
	ULONG  Tag
	);

typedef ALLOCATE_FUNCTION *PALLOCATE_FUNCTION;

typedef void
FREE_FUNCTION (
	PVOID  Buffer
	);

typedef FREE_FUNCTION *PFREE_FUNCTION;

void
  ExInitializeNPagedLookasideList(
	PNPAGED_LOOKASIDE_LIST	Lookaside,
	PALLOCATE_FUNCTION	Allocate,
	PFREE_FUNCTION	Free,
	ULONG  Flags,
	SIZE_T	Size,
	ULONG  Tag,
	USHORT	Depth
	);

void
  ExInitializePagedLookasideList(
	PPAGED_LOOKASIDE_LIST  Lookaside,
	PALLOCATE_FUNCTION	Allocate,
	PFREE_FUNCTION	Free,
	ULONG  Flags,
	SIZE_T	Size,
	ULONG  Tag,
	USHORT	Depth
	);

NTSTATUS 
  PsCreateSystemThread(
	PHANDLE	 ThreadHandle,
	ULONG  DesiredAccess,
	POBJECT_ATTRIBUTES	ObjectAttributes,
	HANDLE	ProcessHandle,
	PCLIENT_ID	ClientId,
	PKSTART_ROUTINE	 StartRoutine,
	PVOID  StartContext
	);

void
  ExDeleteNPagedLookasideList(
	PNPAGED_LOOKASIDE_LIST	Lookaside
	);

void
  ExAcquireFastMutex(
	PFAST_MUTEX	 FastMutex
	);

void
  ExReleaseFastMutex(
	PFAST_MUTEX	 FastMutex
	);

NTSTATUS 
  IoGetDeviceObjectPointer(
	PUNICODE_STRING	 ObjectName,
	ACCESS_MASK	 DesiredAccess,
	PFILE_OBJECT  *FileObject,
	PDEVICE_OBJECT	*DeviceObject
	);

PIRP 
  IoBuildDeviceIoControlRequest(
	ULONG  IoControlCode,
	PDEVICE_OBJECT	DeviceObject,
	PVOID  InputBuffer,
	ULONG  InputBufferLength,
	PVOID  OutputBuffer,
	ULONG  OutputBufferLength,
	BOOLEAN	 InternalDeviceIoControl,
	PKEVENT	 Event,
	PIO_STATUS_BLOCK  IoStatusBlock
	);
