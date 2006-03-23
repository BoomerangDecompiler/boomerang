
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

typedef UNICODE_STRING *PUNICODE_STRING;

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

struct IO_SECURITY_CONTEXT {
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;
    PACCESS_STATE AccessState;
    ACCESS_MASK DesiredAccess;
    ULONG FullCreateOptions;
};
typedef IO_SECURITY_CONTEXT *PIO_SECURITY_CONTEXT;

struct WAIT_CONTEXT_BLOCK {
    KDEVICE_QUEUE_ENTRY WaitQueueEntry;
    PDRIVER_CONTROL DeviceRoutine;
    PVOID DeviceContext;
    ULONG NumberOfMapRegisters;
    PVOID DeviceObject;
    PVOID CurrentIrp;
    PKDPC BufferChainingDpc;
};
typedef WAIT_CONTEXT_BLOCK *PWAIT_CONTEXT_BLOCK;

struct DEVICE_OBJECT {
    CSHORT Type;
    USHORT Size;
    LONG ReferenceCount;
    DRIVER_OBJECT *DriverObject;
    DEVICE_OBJECT *NextDevice;
    DEVICE_OBJECT *AttachedDevice;
    IRP *CurrentIrp;
    PIO_TIMER Timer;
    ULONG Flags; 
    ULONG Characteristics;
    PVOID DoNotUse1;
    PVOID DeviceExtension;
    DEVICE_TYPE DeviceType;
    CCHAR StackSize;
    LIST_ENTRY ListEntry;
    ULONG AlignmentRequirement;
    KDEVICE_QUEUE DeviceQueue;
    KDPC Dpc;
    ULONG ActiveThreadCount;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    KEVENT DeviceLock;
    USHORT SectorSize;
    USHORT Spare1;
    DEVOBJ_EXTENSION  *DeviceObjectExtension;
    PVOID  Reserved;
};
typedef DEVICE_OBJECT *PDEVICE_OBJECT; 


struct DEVOBJ_EXTENSION {

    CSHORT          Type;
    USHORT          Size;
    PDEVICE_OBJECT  DeviceObject;
};
typedef DEVOBJ_EXTENSION *PDEVOBJ_EXTENSION;

struct DRIVER_EXTENSION {

    DRIVER_OBJECT *DriverObject;
    PDRIVER_ADD_DEVICE AddDevice;
    ULONG Count;
    UNICODE_STRING ServiceKeyName;
};
typedef DRIVER_EXTENSION *PDRIVER_EXTENSION;

struct DRIVER_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PDEVICE_OBJECT DeviceObject;
    ULONG Flags;
    PVOID DriverStart;
    ULONG DriverSize;
    PVOID DriverSection;
    PDRIVER_EXTENSION DriverExtension;
    UNICODE_STRING DriverName;
    PUNICODE_STRING HardwareDatabase;
    PFAST_IO_DISPATCH FastIoDispatch;
    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_STARTIO DriverStartIo;
    PDRIVER_UNLOAD DriverUnload;
    PDRIVER_DISPATCH MajorFunction[28];
};
typedef DRIVER_OBJECT *PDRIVER_OBJECT; 


typedef RTL_OSVERSIONINFOEXW *PRTL_OSVERSIONINFOEXW;

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

typedef NTSTATUS 
	DriverEntry (
		PDRIVER_OBJECT DriverObject,
		PUNICODE_STRING RegistryPath
		);

NTSTATUS 
  PsCreateSystemThread(
    PHANDLE  ThreadHandle,
    ULONG  DesiredAccess,
    POBJECT_ATTRIBUTES  ObjectAttributes,
    HANDLE  ProcessHandle,
    PCLIENT_ID  ClientId,
    PKSTART_ROUTINE  StartRoutine,
    PVOID  StartContext
    );


void 
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

NTSTATUS 
  IoInitializeTimer(
    PDEVICE_OBJECT  DeviceObject,
    PIO_TIMER_ROUTINE  TimerRoutine,
    PVOID  Context
    );

void
  IoStartTimer(
    PDEVICE_OBJECT  DeviceObject
    );

void
  KeInitializeSpinLock(
    PKSPIN_LOCK  SpinLock
    );

void
  KeInitializeMutex(
    PRKMUTEX  Mutex,
    ULONG  Level
    );

NTSTATUS 
  IoCreateSymbolicLink(
    PUNICODE_STRING  SymbolicLinkName,
    PUNICODE_STRING  DeviceName
    );

NTSTATUS 
  RtlAppendUnicodeStringToString(
    PUNICODE_STRING  Destination,
    PUNICODE_STRING  Source
    );

NTSTATUS 
  RtlQueryRegistryValues(
    ULONG  RelativeTo,
    PCWSTR  Path,
    PRTL_QUERY_REGISTRY_TABLE  QueryTable,
    PVOID  Context,
    PVOID  Environment
    );

NTSTATUS 
  RtlUnicodeStringToAnsiString(
    PANSI_STRING  DestinationString,
    PUNICODE_STRING  SourceString,
    BOOLEAN  AllocateDestinationString
    );

NTSTATUS 
  IoReportResourceUsage(
    PUNICODE_STRING  DriverClassName,
    PDRIVER_OBJECT  DriverObject,
    PCM_RESOURCE_LIST  DriverList,
    ULONG  DriverListSize,
    PDEVICE_OBJECT  DeviceObject,
    PCM_RESOURCE_LIST  DeviceList,
    ULONG  DeviceListSize,
    BOOLEAN  OverrideConflict,
    PBOOLEAN  ConflictDetected
    );

struct CONFIGURATION_INFORMATION {
    ULONG DiskCount;
    ULONG FloppyCount;
    ULONG CDRomCount;
    ULONG TapeCount;
    ULONG ScsiPortCount;
    ULONG SerialCount;
    ULONG ParallelCount;
    BOOLEAN AtDiskPrimaryAddressClaimed;
    BOOLEAN AtDiskSecondaryAddressClaimed;
};

typedef CONFIGURATION_INFORMATION *PCONFIGURATION_INFORMATION;

PCONFIGURATION_INFORMATION 
  IoGetConfigurationInformation();

void 
  ExFreePool(
    PVOID P
    );

NTSTATUS 
  IoRegisterShutdownNotification(
    PDEVICE_OBJECT  DeviceObject
    );

void
  IoDeleteDevice(
    PDEVICE_OBJECT  DeviceObject
    );

NTSTATUS 
  RtlIntegerToUnicodeString(
    ULONG  Value,
    ULONG  Base,
    PUNICODE_STRING  String
    );

NTSTATUS 
  KeWaitForSingleObject(
    PVOID  Object,
    KWAIT_REASON  WaitReason,
    KPROCESSOR_MODE  WaitMode,
    BOOLEAN  Alertable,
    PLARGE_INTEGER  Timeout
    );

UCHAR 
  READ_PORT_UCHAR(
    PUCHAR  Port
    );

void
  WRITE_PORT_UCHAR(
    PUCHAR  Port,
    UCHAR  Value
    );

void
  KeStallExecutionProcessor(
    ULONG  MicroSeconds
    );

ULONG 
  KeQueryTimeIncrement(
    );

BOOLEAN 
  HalTranslateBusAddress(
    INTERFACE_TYPE  InterfaceType,
    ULONG  BusNumber,
    PHYSICAL_ADDRESS  BusAddress,
    PULONG  AddressSpace,
    PPHYSICAL_ADDRESS  TranslatedAddress
    );

SIZE_T 
  RtlCompareMemory(
    const VOID  *Source1,
    const VOID  *Source2,
    SIZE_T  Length
    );

BOOLEAN 
  KeCancelTimer(
    PKTIMER  Timer
    );


LONG 
  HalGetInterruptVector(
    INTERFACE_TYPE  InterfaceType,
    ULONG  BusNumber,
    ULONG  BusInterruptLevel,
    ULONG  BusInterruptVector,
    PKIRQL  Irql,
    PKAFFINITY  Affinity
    );

NTSTATUS 
  IoConnectInterrupt(
    PKINTERRUPT  *InterruptObject,
    PKSERVICE_ROUTINE  ServiceRoutine,
    PVOID  ServiceContext,
    PKSPIN_LOCK  SpinLock,
    ULONG  Vector,
    KIRQL  Irql,
    KIRQL  SynchronizeIrql,
    KINTERRUPT_MODE    InterruptMode,
    BOOLEAN  ShareVector,
    KAFFINITY  ProcessorEnableMask,
    BOOLEAN  FloatingSave
    );

NTSTATUS 
  RtlAppendUnicodeToString(
    PUNICODE_STRING  Destination,
    PCWSTR  Source
    );

ULONG DbgPrint(PCHAR  Format, ...);
