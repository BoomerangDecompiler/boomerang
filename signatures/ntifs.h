
void
KeInitializeEvent (
	PRKEVENT Event,
	EVENT_TYPE Type,
	BOOLEAN State
	);


void
KeClearEvent (
	PRKEVENT Event
	);



LONG
KePulseEvent (
	PRKEVENT Event,
	KPRIORITY Increment,
	BOOLEAN Wait
	);



LONG
KeReadStateEvent (
	PRKEVENT Event
	);


LONG
KeResetEvent (
	PRKEVENT Event
	);


LONG
KeSetEvent (
	PRKEVENT Event,
	KPRIORITY Increment,
	BOOLEAN Wait
	);


void
KeInitializeMutant (
	PRKMUTANT Mutant,
	BOOLEAN InitialOwner
	);

LONG
KeReadStateMutant (
	PRKMUTANT Mutant
	);


LONG
KeReleaseMutant (
	PRKMUTANT Mutant,
	KPRIORITY Increment,
	BOOLEAN Abandoned,
	BOOLEAN Wait
	);

void
IoRegisterFileSystem(
	PDEVICE_OBJECT DeviceObject
	);

NTSTATUS
IoRegisterFsRegistrationChange(
	PDRIVER_OBJECT DriverObject,
	PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
	);


void
SeCaptureSubjectContext (
	PSECURITY_SUBJECT_CONTEXT SubjectContext
	);

void
SeLockSubjectContext(
	PSECURITY_SUBJECT_CONTEXT SubjectContext
	);

void
SeUnlockSubjectContext(
	PSECURITY_SUBJECT_CONTEXT SubjectContext
	);

void
SeReleaseSubjectContext (
	PSECURITY_SUBJECT_CONTEXT SubjectContext
	);

NTSTATUS
SeCaptureAuditPolicy(
	PTOKEN_AUDIT_POLICY Policy,
	KPROCESSOR_MODE RequestorMode,
	PVOID CaptureBuffer,
	ULONG CaptureBufferLength,
	POOL_TYPE PoolType,
	BOOLEAN ForceCapture,
	PTOKEN_AUDIT_POLICY *CapturedPolicy
	);

void
SeReleaseAuditPolicy (
	PTOKEN_AUDIT_POLICY CapturedPolicy,
	KPROCESSOR_MODE RequestorMode,
	BOOLEAN ForceCapture
	);


NTSTATUS
SeAssignSecurity (
	PSECURITY_DESCRIPTOR ParentDescriptor,
	PSECURITY_DESCRIPTOR ExplicitDescriptor,
	PSECURITY_DESCRIPTOR *NewDescriptor,
	BOOLEAN IsDirectoryObject,
	PSECURITY_SUBJECT_CONTEXT SubjectContext,
	PGENERIC_MAPPING GenericMapping,
	POOL_TYPE PoolType
	);

NTSTATUS
SeAssignSecurityEx (
	PSECURITY_DESCRIPTOR ParentDescriptor, 
	PSECURITY_DESCRIPTOR ExplicitDescriptor,
	PSECURITY_DESCRIPTOR *NewDescriptor,
	GUID *ObjectType,
	BOOLEAN IsDirectoryObject,
	ULONG AutoInheritFlags,
	PSECURITY_SUBJECT_CONTEXT SubjectContext,
	PGENERIC_MAPPING GenericMapping,
	POOL_TYPE PoolType
	);

NTSTATUS
SeDeassignSecurity (
	PSECURITY_DESCRIPTOR *SecurityDescriptor
	);

BOOLEAN
SeAccessCheck (
	PSECURITY_DESCRIPTOR SecurityDescriptor,
	PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
	BOOLEAN SubjectContextLocked,
	ACCESS_MASK DesiredAccess,
	ACCESS_MASK PreviouslyGrantedAccess,
	PPRIVILEGE_SET *Privileges,
	PGENERIC_MAPPING GenericMapping,
	KPROCESSOR_MODE AccessMode,
	PACCESS_MASK GrantedAccess,
	PNTSTATUS AccessStatus
	);

BOOLEAN
RtlValidSid (
	PSID Sid
	);

BOOLEAN
RtlEqualSid (
	PSID Sid1,
	PSID Sid2
	);

BOOLEAN
RtlEqualPrefixSid (
	PSID Sid1,
	PSID Sid2
	);

PVOID
RtlFreeSid(
	PSID Sid
	);

NTSTATUS
RtlAllocateAndInitializeSid(
	PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
	UCHAR SubAuthorityCount,
	ULONG SubAuthority0,
	ULONG SubAuthority1,
	ULONG SubAuthority2,
	ULONG SubAuthority3,
	ULONG SubAuthority4,
	ULONG SubAuthority5,
	ULONG SubAuthority6,
	ULONG SubAuthority7,
	PSID *Sid
	);

NTSTATUS
RtlInitializeSid (
	PSID Sid,
	PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
	UCHAR SubAuthorityCount
	);

PSID_IDENTIFIER_AUTHORITY
RtlIdentifierAuthoritySid (
	PSID Sid
	);

ULONG
RtlLengthRequiredSid (
	ULONG SubAuthorityCount
	);

PULONG
RtlSubAuthoritySid (
	PSID Sid,
	ULONG SubAuthority
	);

PUCHAR
RtlSubAuthorityCountSid (
	PSID Sid
	);

ULONG
RtlLengthSid (
	PSID Sid
	);

NTSTATUS
RtlCopySid (
	ULONG DestinationSidLength,
	PSID DestinationSid,
	PSID SourceSid
	);


void
RtlCopyLuid (
	PLUID DestinationLuid,
	PLUID SourceLuid
	);


void
RtlMapGenericMask(
	PACCESS_MASK AccessMask,
	PGENERIC_MAPPING GenericMapping
	);
									   
NTSTATUS
RtlCreateAcl (									
	PACL Acl,									
	ULONG AclLength,							
	ULONG AclRevision							
	);											

NTSTATUS
RtlGetAce (
	PACL Acl,
	ULONG AceIndex,
	PVOID *Ace
	);

NTSTATUS
RtlAddAccessAllowedAce (
	PACL Acl,
	ULONG AceRevision,
	ACCESS_MASK AccessMask,
	PSID Sid
	);

NTSTATUS
PoQueueShutdownWorkItem(
	PWORK_QUEUE_ITEM WorkItem
	);

NTSTATUS
ZwCreateEvent (
	PHANDLE EventHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	EVENT_TYPE EventType,
	BOOLEAN InitialState
	);


NTSTATUS
ZwCreateFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER AllocationSize,
	ULONG FileAttributes,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions,
	PVOID EaBuffer,
	ULONG EaLength
	);


NTSTATUS
ZwDeleteFile(
	POBJECT_ATTRIBUTES ObjectAttributes
	);


NTSTATUS
ZwDeviceIoControlFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG IoControlCode,
	PVOID InputBuffer,
	ULONG InputBufferLength,
	PVOID OutputBuffer,
	ULONG OutputBufferLength
	);


NTSTATUS
ZwOpenFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG OpenOptions
	);


NTSTATUS
ZwQueryDirectoryFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN ReturnSingleEntry,
	PUNICODE_STRING FileName,
	BOOLEAN RestartScan
	);


NTSTATUS
ZwQueryInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass
	);


NTSTATUS
ZwSetInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass
	);


NTSTATUS
ZwQueryVolumeInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FsInformation,
	ULONG Length,
	FS_INFORMATION_CLASS FsInformationClass
	);


NTSTATUS
ZwSetVolumeInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FsInformation,
	ULONG Length,
	FS_INFORMATION_CLASS FsInformationClass
	);


NTSTATUS
ZwReadFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	PLARGE_INTEGER ByteOffset,
	PULONG Key
	);


NTSTATUS
ZwWriteFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	PLARGE_INTEGER ByteOffset,
	PULONG Key
	);


NTSTATUS
ZwFsControlFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG FsControlCode,
	PVOID InputBuffer,
	ULONG InputBufferLength,
	PVOID OutputBuffer,
	ULONG OutputBufferLength
	);

NTSTATUS
ZwClose(
	HANDLE Handle
	);


NTSTATUS
ZwDuplicateObject(
	HANDLE SourceProcessHandle,
	HANDLE SourceHandle,
	HANDLE TargetProcessHandle,
	PHANDLE TargetHandle,
	ACCESS_MASK DesiredAccess,
	ULONG HandleAttributes,
	ULONG Options
	);


NTSTATUS
ZwCreateDirectoryObject(
	PHANDLE DirectoryHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
	);


NTSTATUS
ZwOpenDirectoryObject(
	PHANDLE DirectoryHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
	);


NTSTATUS
ZwOpenSymbolicLinkObject(
	PHANDLE LinkHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
	);


NTSTATUS
ZwQuerySymbolicLinkObject(
	HANDLE LinkHandle,
	PUNICODE_STRING LinkTarget,
	PULONG ReturnedLength
	);


NTSTATUS
ZwMakeTemporaryObject(
	HANDLE Handle
	);


NTSTATUS
ZwCreateKey(
	PHANDLE KeyHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	ULONG TitleIndex,
	PUNICODE_STRING Class,
	ULONG CreateOptions,
	PULONG Disposition
	);


NTSTATUS
ZwOpenKey(
	PHANDLE KeyHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
	);


NTSTATUS
ZwDeleteKey(
	HANDLE KeyHandle
	);


NTSTATUS
ZwDeleteValueKey(
	HANDLE KeyHandle,
	PUNICODE_STRING ValueName
	);


NTSTATUS
ZwEnumerateKey(
	HANDLE KeyHandle,
	ULONG Index,
	KEY_INFORMATION_CLASS KeyInformationClass,
	PVOID KeyInformation,
	ULONG Length,
	PULONG ResultLength
	);


NTSTATUS
ZwEnumerateValueKey(
	HANDLE KeyHandle,
	ULONG Index,
	KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	PVOID KeyValueInformation,
	ULONG Length,
	PULONG ResultLength
	);


NTSTATUS
ZwFlushKey(
	HANDLE KeyHandle
	);


NTSTATUS
ZwQueryKey(
	HANDLE KeyHandle,
	KEY_INFORMATION_CLASS KeyInformationClass,
	PVOID KeyInformation,
	ULONG Length,
	PULONG ResultLength
	);


NTSTATUS
ZwQueryValueKey(
	HANDLE KeyHandle,
	PUNICODE_STRING ValueName,
	KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	PVOID KeyValueInformation,
	ULONG Length,
	PULONG ResultLength
	);


NTSTATUS
ZwSetValueKey(
	HANDLE KeyHandle,
	PUNICODE_STRING ValueName,
	ULONG TitleIndex,
	ULONG Type,
	PVOID Data,
	ULONG DataSize
	);


NTSTATUS
ZwOpenSection(
	PHANDLE SectionHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
	);


NTSTATUS
ZwMapViewOfSection(
	HANDLE SectionHandle,
	HANDLE ProcessHandle,
	PVOID *BaseAddress,
	ULONG ZeroBits,
	SIZE_T CommitSize,
	PLARGE_INTEGER SectionOffset,
	PSIZE_T ViewSize,
	SECTION_INHERIT InheritDisposition,
	ULONG AllocationType,
	ULONG Protect
	);


NTSTATUS
ZwUnmapViewOfSection(
	HANDLE ProcessHandle,
	PVOID BaseAddress
	);


NTSTATUS
ZwSetInformationThread(
	HANDLE ThreadHandle,
	THREADINFOCLASS ThreadInformationClass,
	PVOID ThreadInformation,
	ULONG ThreadInformationLength
	);


NTSTATUS
ZwCreateSection (
	PHANDLE SectionHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PLARGE_INTEGER MaximumSize,
	ULONG SectionPageProtection,
	ULONG AllocationAttributes,
	HANDLE FileHandle
	);


NTSTATUS
ZwAllocateVirtualMemory(
	HANDLE ProcessHandle,
	PVOID *BaseAddress,
	ULONG ZeroBits,
	PSIZE_T RegionSize,
	ULONG AllocationType,
	ULONG Protect
	);


NTSTATUS
ZwFreeVirtualMemory(
	HANDLE ProcessHandle,
	PVOID *BaseAddress,
	PSIZE_T RegionSize,
	ULONG FreeType
	);


NTSTATUS
ZwWaitForSingleObject(
	HANDLE Handle,
	BOOLEAN Alertable,
	PLARGE_INTEGER TimeOPTIONAL
	);


NTSTATUS
ZwSetEvent (
	HANDLE Handle,
	PLONG PreviousState
	);


NTSTATUS
ZwFlushVirtualMemory(
	HANDLE ProcessHandle,
	PVOID *BaseAddress,
	PSIZE_T RegionSize,
	PIO_STATUS_BLOCK IoStatus
	);


NTSTATUS
ZwOpenProcessTokenEx(
	HANDLE ProcessHandle,
	ACCESS_MASK DesiredAccess,
	ULONG HandleAttributes,
	PHANDLE TokenHandle
	);


NTSTATUS
ZwOpenThreadTokenEx(
	HANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	BOOLEAN OpenAsSelf,
	ULONG HandleAttributes,
	PHANDLE TokenHandle
	);


NTSTATUS
ZwSetInformationToken (
	HANDLE TokenHandle,
	TOKEN_INFORMATION_CLASS TokenInformationClass,
	PVOID TokenInformation,
	ULONG TokenInformationLength
	);


NTSTATUS
ZwQueryInformationToken (
	HANDLE TokenHandle,
	TOKEN_INFORMATION_CLASS TokenInformationClass,
	PVOID TokenInformation,
	ULONG TokenInformationLength,
	PULONG ReturnLength
	);


NTSTATUS
ZwSetSecurityObject(
	HANDLE Handle,
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR SecurityDescriptor
	);


NTSTATUS
ZwQuerySecurityObject(
	HANDLE Handle,
	SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR SecurityDescriptor,
	ULONG Length,
	PULONG LengthNeeded
	);


NTSTATUS
ZwLoadDriver(
	PUNICODE_STRING DriverServiceName
	);


NTSTATUS
ZwUnloadDriver(
	PUNICODE_STRING DriverServiceName
	);


NTSTATUS
ZwLockFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER ByteOffset,
	PLARGE_INTEGER Length,
	ULONG Key,
	BOOLEAN FailImmediately,
	BOOLEAN ExclusiveLock
	);


NTSTATUS
ZwUnlockFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER ByteOffset,
	PLARGE_INTEGER Length,
	ULONG Key
	);


NTSTATUS
ZwQueryQuotaInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	BOOLEAN ReturnSingleEntry,
	PVOID SidList,
	ULONG SidListLength,
	PSID StartSid,
	BOOLEAN RestartScan
	);


NTSTATUS
ZwSetQuotaInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length
	);


NTSTATUS
ZwConnectPort(
	PHANDLE PortHandle,
	PUNICODE_STRING PortName,
	PSECURITY_QUALITY_OF_SERVICE SecurityQos,
	PPORT_VIEW ClientView,
	PREMOTE_PORT_VIEW ServerView,
	PULONG MaxMessageLength,
	PVOID ConnectionInformation,
	PULONG ConnectionInformationLength
	);


NTSTATUS
ZwSecureConnectPort(
	PHANDLE PortHandle,
	PUNICODE_STRING PortName,
	PSECURITY_QUALITY_OF_SERVICE SecurityQos,
	PPORT_VIEW ClientView,
	PSID RequiredServerSid,
	PREMOTE_PORT_VIEW ServerView,
	PULONG MaxMessageLength,
	PVOID ConnectionInformation,
	PULONG ConnectionInformationLength
	);


NTSTATUS
ZwRequestWaitReplyPort(
	HANDLE PortHandle,
	PPORT_MESSAGE RequestMessage,
	PPORT_MESSAGE ReplyMessage
	);








void
KeInitializeSemaphore (
	PRKSEMAPHORE Semaphore,
	LONG Count,
	LONG Limit
	);

LONG
KeReadStateSemaphore (
	PRKSEMAPHORE Semaphore
	);

LONG
KeReleaseSemaphore (
	PRKSEMAPHORE Semaphore,
	KPRIORITY Increment,
	LONG Adjustment,
	BOOLEAN Wait
	);

void
KeAttachProcess (
	PRKPROCESS Process
	);

void
KeDetachProcess (
	void
	);

void
KeStackAttachProcess (
	PRKPROCESS PROCESS,
	PRKAPC_STATE ApcState
	);


void
KeUnstackDetachProcess (
	PRKAPC_STATE ApcState
	);

NTSTATUS											
KeDelayExecutionThread (							
	KPROCESSOR_MODE WaitMode,					 
	BOOLEAN Alertable,							 
	PLARGE_INTEGER Interval						 
	);												
													

KPRIORITY											
KeQueryPriorityThread (								
	PKTHREAD Thread								 
	);												
													
ULONG												
KeQueryRuntimeThread (								
	PKTHREAD Thread,							 
	PULONG UserTime								
	);												
													
LONG												
KeSetBasePriorityThread (							
	PKTHREAD Thread,							 
	LONG Increment								 
	);												
													
UCHAR
KeSetIdealProcessorThread (
	PKTHREAD Thread,
	UCHAR Processor
	);

NTSTATUS
IofCallDriver(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
	);

LONG_PTR
ObfReferenceObject(
	PVOID Object
	);

NTSTATUS
ObReferenceObjectByPointer(
	PVOID Object,
	ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType,
	KPROCESSOR_MODE AccessMode
	);

LONG_PTR
ObfDereferenceObject(
	PVOID Object
	);

NTSTATUS
ObQueryNameString(
	PVOID Object,
	POBJECT_NAME_INFORMATION ObjectNameInfo,
	ULONG Length,
	PULONG ReturnLength
	);

NTSTATUS
ObGetObjectSecurity(
	PVOID Object,
	PSECURITY_DESCRIPTOR *SecurityDescriptor,
	PBOOLEAN MemoryAllocated
	);

void
ObReleaseObjectSecurity(
	PSECURITY_DESCRIPTOR SecurityDescriptor,
	BOOLEAN MemoryAllocated
	);

NTSTATUS														
ObQueryObjectAuditingByHandle(									
	HANDLE Handle,											 
	PBOOLEAN GenerateOnClose								
	);	 
