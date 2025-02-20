/*
 * NetXStructs.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#ifndef SRC_HARDWARE_NETX_NETXSTRUCTS_H_
#define SRC_HARDWARE_NETX_NETXSTRUCTS_H_

#include <RepRapFirmware.h>

#if NETX_ENABLE

typedef uint16_t EtherCATCRC_t ;
typedef uint16_t EtherCATLength_t ;
constexpr EtherCATLength_t EtherCATSharedBufferLenght = 500;	 							///< Shared buffer length in bytes.
constexpr EtherCATLength_t EtherCATSingleBufferLenght = EtherCATSharedBufferLenght / 2;	 	///< When the data is too big we devide the buffer in half.
constexpr uint8_t NETX_UART_RESET_INDEX	=	0;												// Index that reset the communication

// This is the INPUT of the master, so the output of the microcontroller.
struct __attribute__((packed)) EtherCATApplicationMISOData {

	EtherCATCRC_t crc16bit;										///< Simple sum from sSerialHeader.idxTx to sharedBuffer[sSerialHeader.length+sSocketHeader.length]
	uint8_t alive;												///< Simple sum from sSerialHeader.idxTx to sharedBuffer[sSerialHeader.length+sSocketHeader.length]

	struct __attribute__((packed)) SerialHeader {
		uint8_t 	idxTx 	= NETX_UART_RESET_INDEX;			///< Index Response: We will copy the idx.
		uint8_t 	idxRx 	= NETX_UART_RESET_INDEX;			///< Index Response: We will copy the idx from the Master.
		EtherCATLength_t 	length 	= 0;						///< Data length in the buffer
	} serialHeader;

	struct __attribute__((packed)) SocketHeader {
		uint8_t 	idxTx  	= NETX_UART_RESET_INDEX;			///< Index Response: We will copy the idx.
		uint8_t 	idxRx  	= NETX_UART_RESET_INDEX;			///< Index Response: We will copy the idx from the Master.
		EtherCATLength_t 	length 	= 0;						///< Data length in the buffer
	} socketHeader;

	/* SHARED BUFFER
	 * This buffer contains the data of the Serial and Socket.
	 * The part of the buffer used for the Serial will be:
	 * 		sharedBuffer[0..(sSerialHeader.length-1)]
	 * 	The bytes reserved for the Socket are:
	 * 		sharedBuffer[sSerialHeader.length..end]
	 */

	uint8_t sharedBuffer[EtherCATSharedBufferLenght];
};

// This is the OUTPUT of the master, so the input of the microcontroller.
struct __attribute__((packed)) EtherCATApplicationMOSIData {

	EtherCATCRC_t crc16bit;								///< Simple XOR from sSerialHeader.idxMOSI to sharedBuffer[sSerialHeader.length+sSocketHeader.length]

	struct __attribute__((packed)) SerialHeader {
		// The index will take values between 1 and 255. A NETX_UART_RESET_INDEX will reset the comm.
		uint8_t 	idxMOSI;	  						///< Index Package:	The master will send this number. When the answer is ready, we will copy it in Index Package.
		uint8_t 	idxMISO;	  						///< Index Package:	The master will send this number. When the answer is ready, we will copy it in Index Package.
		EtherCATLength_t 	length;							///< Data length in the buffer
	} serialHeader;

	struct __attribute__((packed)) SocketHeader {
		uint8_t 	idxMOSI;	  						///< Index Package:	The master will send this number. When the answer is ready, we will copy it in Index Package.
		uint8_t 	idxMISO;	  						///< Index Package:	The master will send this number. When the answer is ready, we will copy it in Index Package.
		EtherCATLength_t 	length;							///< Data length in the buffer
	} socketHeader;

	/* SHARED BUFFER
	 * This buffer contains the data of the Serial and Socket.
	 * The part of the buffer used for the Serial will be:
	 * 		sharedBuffer[0..(sSerialHeader.length-1)]
	 * 	The bytes reserved for the Socket are:
	 * 		sharedBuffer[sSerialHeader.length..end]
	 */
	uint8_t sharedBuffer[EtherCATSharedBufferLenght];

};

#ifndef NETX_COMMANDS_LIST
#define NETX_COMMANDS_LIST 	\
	X(EtherCATSMALStatusChangedIndicator, 	0x000019DE, HandlerNetXPacketEtherCATSMALStatusChangedIndicator,\
			"EtherCAT State Machine Application Layer - Status Changed Indication") \
	X(EtherCATSMALStatusChangesResponse, 	0x000019DF, debugNetXDefault,									\
			"EtherCAT State Machine Application Layer - Status Changed Response") \
	X(OSFirmawareIdentifyRequest, 			0x00001EB6, debugNetXDefault, 									\
			"NetX OS - Identifying Channel Firmware Request") \
	X(OSFirmawareIdentifyConfirmation, 		0x00001EB7, debugNetXPacketOSFirmawareIdentifyConfirmation,		\
			"NetX OS - Identifying Channel Firmware Confirmation") \
	X(OSSetAddressRequest, 					0x00001EEE, debugNetXDefault, 									\
			"NetX OS - Set MAC Address Request") \
	X(OSSetAddressConfirmation, 			0x00001EEF, HandlerNetXPacketOSSetAddressConfirmation,			\
			"NetX OS - Set MAC Address Confirmation") \
	X(OSGetBlockInformationRequest, 		0x00001EF8, debugNetXDefault, 									\
			"NetX OS - Get Block Information Request") \
	X(OSGetBlockInformationConfirmation, 	0x00001EF9, debugNetXPacketOSGetBlockInformationConfirmation,	\
			"NetX OS - Get Block Information Confirmation") \
	X(EtherCATSetConfigurationRequest, 		0x00002CCE, debugNetXDefault, 									\
			"EtherCAT - Set Configuration Request") \
	X(EtherCATSetConfigurationConfirmation, 0x00002CCF, HandlerNetXPacketEtherCATSetConfigurationConfirmation, \
			"EtherCAT - Set Configuration Confirmation") \
	X(OSRegisterApplicationRequest, 		0x00002F10, debugNetXDefault, 									\
			"NetX OS - Register Application Request") \
	X(OSRegisterApplicationConfirmation, 	0x00002F11, HandlerNetXPacketOSRegisterApplicationConfirmation,	\
			"NetX OS - Register Application Confirmation") \
	X(OSStartStopCommunicationRequest, 		0x00002F30, debugNetXDefault, 									\
			"NetX OS - Start / Stop Communication Request") \
	X(OSStartStopCommunicationConfirmation, 0x00002F31, HandlerNetXPacketOSStartStopCommunicationConfirmation, \
			"NetX OS - Start / Stop Communication Confirmation") \
	X(OSChannelInitializationRequest, 		0x00002F80, debugNetXDefault, 									\
			"NetX OS - Channel Initialization Request") \
	X(OSChannelInitializationConfirmation, 	0x00002F81, HandlerNetXPacketOSChannelInitializationConfirmation, \
			"NetX OS - Channel Initialization Confirmation") \
	X(OSLinkStatusChangeIndicator, 			0x00002F8A, HandlerNetXPacketOSLinkStatusChangeIndicator,		\
			"NetX OS - Link Status Change Indicator") \
	X(OSLinkStatusChangeResponse, 			0x00002F8B, debugNetXDefault,									\
			"NetX OS - Link Status Change Response")
#endif

enum class NetXCommand : uint32_t
{
	#define X(name, number, func, str) name = number,
		NETX_COMMANDS_LIST
	#undef X
};

uint32_t asNetXCommand(const NetXCommand value);



namespace NetXDefines{
	namespace ChannelType {
		constexpr uint8_t System 					= 3; 		// NETX_SYSTEM_CHANNEL_INFO
		constexpr uint8_t Handshake		 			= 4; 		// NETX_HANDSHAKE_CHANNEL_INFO
		constexpr uint8_t Communication	 			= 5; 		// NETX_COMMUNICATION_CHANNEL_INFO
		constexpr uint8_t Application	 			= 6; 		// NETX_APPLICATION_CHANNEL_INFO
	};

	namespace ChannelDefaultSize {
		constexpr uint32_t System 					= 0x200; 	// NETX_SYSTEM_CHANNEL_INFO
		constexpr uint32_t Handshake		 		= 0x100;	// NETX_HANDSHAKE_CHANNEL_INFO
		constexpr uint32_t Communication		 	= 0x3D00;	// NETX_COMMUNICATION_CHANNEL_INFO (15616 bytes)
	};

	namespace SizeOrPositionOfHandshakeRegisters {
		constexpr uint8_t System 					= 17; 		// NETX_SYSTEM_CHANNEL_INFO
		constexpr uint8_t Communication		 		= 18;		// NETX_COMMUNICATION_CHANNEL_INFO (15616 bytes)
	};
};

 // NETX_SYSTEM_INFO_BLOCK -----------------------------------------------------------------------------------------------------
struct __attribute__((packed)) NetXSystemInfoBlock{
	uint8_t abCookie[4]; 			// netX DPM Identification (start of DPM) - ASCII characters:	'netX’ = firmware is running
	uint32_t ulDpmTotalSize; 		// DPM Size in bytes
	uint32_t ulDeviceNumber; 		// Device Number
	uint32_t ulSerialNumber; 		// Serial Number
	uint16_t ausHwOptions[4]; 		//Hardware Assembly Options
	uint16_t usManufacturer; 		// Manufacturer Code
	uint16_t usProductionDate; 		// Production Date
	uint32_t ulLicenseFlags1;		// License Code - Flags 1
	uint32_t ulLicenseFlags2;		// License Code - Flags 2
	uint16_t usNetxLicenseID;		// netX License Identification
	uint16_t usNetxLicenseFlags;	// netX License Flags
	uint16_t usDeviceClass;			// Device Class
	uint8_t bHwRevision;			// Hardware Revision
	uint8_t bHwCompatibility;		// Hardware Compatibility
	uint8_t bDevIdNumber; 			// Hardware Device Identification Number(DIP-switch / Rotary Switch
};

// NETX_CHANNEL_INFO_BLOCK -----------------------------------------------------------------------------------------------------

struct __attribute__((packed)) NetXSystemChannelInfo{ // NETX_SYSTEM_CHANNEL_INFO
	uint8_t bChannelType;			// SYSTEM (0x03)
	uint8_t bReserved;				// Reserved (Set to Zero)
	uint8_t bSizePositionHandshakeRegisters;
	uint8_t bTotalNumberBlocks;		// Total Number of blocks
	uint32_t ulSizeOfChannel;		// Size of Channel in bytes
	uint16_t usSizeOfMailbox;		// Size of Send and Receive Mailbox in Bytes
	uint16_t usMailboxStartOffset;	// Mailbox Start Offset
	uint8_t abReserved[4];			// Reserved (Set to Zero)
};


struct __attribute__((packed)) NetXHandshakeChannelInfo{ // NETX_HANDSHAKE_CHANNEL_INFO
	uint8_t bChannelType;			// HANDSHAKE (0x04)
	uint8_t abReserved[3];			// Reserved (Set to Zero)
	uint32_t ulSizeOfChannel;		// Size of Channel in bytes
	uint8_t abReserved2[8];			// Reserved (Set to Zero)
};


struct __attribute__((packed)) NetXCommunicationChannelInfo{ // NETX_COMMUNICATION_CHANNEL_INFO
	uint8_t bChannelType;			// COMMUNICATION (0x05)
	uint8_t bChannelID;				// Channel Number
	uint8_t bSizePositionHandshakeRegisters;
	uint8_t bTotalNumberBlocks;		// Total Number of blocks
	uint32_t ulSizeOfChannel;		// Size of Channel in bytes
	uint16_t usCommunicationClass;	// Communication Class (Master, Slave...)
	uint16_t usProtocolClass;		// Protocol Class (PROFIBUS, PROFINET....)
	uint16_t usProtocolConformanceClass;// Protocol Conformance Class (DPV1, DPV2...)
	uint8_t abReserved[2];			// Reserved (Set to Zero)
};

struct __attribute__((packed)) NetXApplicationChannelInfo{ // NETX_APPLICATION_CHANNEL_INFO
	uint8_t bChannelType;			// APPLICATION (0x06)
	uint8_t bChannelID;				// Channel Number
	uint8_t bSizePositionHandshakeRegisters;
	uint8_t bTotalNumberBlocks;		// Total Number of blocks
	uint32_t ulSizeOfChannel;		// Size of Channel in bytes
	uint8_t abReserved[8];			// Reserved (Set to Zero)
};


struct __attribute__((packed)) NetXChannelInfoBlock{ // NETX_CHANNEL_INFO_BLOCK
	NetXSystemChannelInfo tSystemChannelInfo;
	NetXHandshakeChannelInfo tHandshakeChannelInfo;
	NetXCommunicationChannelInfo atCommunicationChannelInfo[4];
	NetXApplicationChannelInfo atApplicationChannelInfo[2];
};


/* System Channel - HOST Flags */
#define HSF_RESET                         0x01                      /*!< Reset command bitmask */
#define HSF_BOOTSTART                     0x02                      /*!< Set when device has a second stage loader, to enter bootloader mode after a system start */
#define HSF_HOST_COS_CMD                  0x04                      /*!< Host "Change Of State" command bitmask */
#define HSF_NETX_COS_ACK                  0x08                      /*!< NetX "Change Of State" acknowlegde bitmask */
#define HSF_SEND_MBX_CMD                  0x10                      /*!< Send mailbox command bitmask */
#define HSF_RECV_MBX_ACK                  0x20                      /*!< Receive mailbox acknowledge bitmask */
#define HSF_EXT_SEND_MBX_CMD              0x40                      /*!< Second stage loader extended mailbox command bitmask */
#define HSF_EXT_RECV_MBX_ACK              0x80                      /*!< Second stage loader extended mailbox ack bitmask */
/* System Channel - netX Flags */
#define NSF_READY                         0x01                      /*!< netX System READY bitmask */
#define NSF_ERROR                         0x02                      /*!< General system error bitmask */
#define NSF_HOST_COS_ACK                  0x04                      /*!< Host "Change Of State" acknowledge bitmask */
#define NSF_NETX_COS_CMD                  0x08                      /*!< NetX "Change of State command bitmask */
#define NSF_SEND_MBX_ACK                  0x10                      /*!< Send mailbox acknowledge bitmask */
#define NSF_RECV_MBX_CMD                  0x20                      /*!< Receive mailbox command bitmask */
#define NSF_EXT_SEND_MBX_ACK              0x40                      /*!< Second stage loader extended mailbox ack bitmask */
#define NSF_EXT_RECV_MBX_CMD              0x80                      /*!< Second stage loader extended mailbox command bitmask */

/* HOST Communication Channel Flags */
#define HCF_HOST_READY                    0x0001                      /*!< Host application is Ready bitmask */
#define HCF_unused                        0x0002                      /*!< unused */
#define HCF_HOST_COS_CMD                  0x0004                      /*!< Host "Change Of State" command bitmask */
#define HCF_NETX_COS_ACK                  0x0008                      /*!< NetX "Change Of State" acknowledge bitmask */
#define HCF_SEND_MBX_CMD                  0x0010                      /*!< Send mailbox command bitmask */
#define HCF_RECV_MBX_ACK                  0x0020                      /*!< Receive mailbox ackowledge bitmask */
#define HCF_PD0_OUT_CMD                   0x0040                      /*!< Process data, block 0, output command bitmask */
#define HCF_PD0_IN_ACK                    0x0080                      /*!< Process data, block 0, input acknowlegde bitmask */
#define HCF_PD1_OUT_CMD                   0x0100                      /*!< Process data, block 1, output command bitmask */
#define HCF_PD1_IN_ACK                    0x0200                      /*!< Process data, block 1, input acknowlegde bitmask */
/* NetX Communication Channel Flags */
#define NCF_COMMUNICATING                 0x0001                      /*!< Channel has an active conection bitmask */
#define NCF_ERROR                         0x0002                      /*!< Communication channel error bitmask */
#define NCF_HOST_COS_ACK                  0x0004                      /*!< Host "Change Of State" acknowledge bitmask */
#define NCF_NETX_COS_CMD                  0x0008                      /*!< NetX "Change Of State" command bitmask */
#define NCF_SEND_MBX_ACK                  0x0010                      /*!< Send mailbox acknowldege bitmask */
#define NCF_RECV_MBX_CMD                  0x0020                      /*!< Receive mailbox command bitmask */
#define NCF_PD0_OUT_ACK                   0x0040                      /*!< Process data, block 0, output acknowledge bitmask */
#define NCF_PD0_IN_CMD                    0x0080                      /*!< Process data, block 0, input command bitmask */
#define NCF_PD1_OUT_ACK                   0x0100                      /*!< Process data, block 1, output acknowlegde bitmask */
#define NCF_PD1_IN_CMD                    0x0200                      /*!< Process data, block 1, input command bitmask */

struct __attribute__((packed)) NetXHandshakeCellUint8_t{ // NETX_HANDSHAKE_CELL
	uint8_t abReserved[2];			// Reserved (Set to Zero)
	uint8_t bNetxFlags;			// netX system channel handshake register
	uint8_t bHostFlags;				// Host system channel handshake register
};

struct __attribute__((packed)) NetXHandshakeCellUint16_t{ // NETX_HANDSHAKE_CELL
	uint16_t usNetxFlags;			// netX system channel handshake register
	uint16_t usHostFlags;			// Host system channel handshake register
};

union NetXHandshakeCell { // NETX_HANDSHAKE_CELL
	NetXHandshakeCellUint8_t 	t8Bits;
	NetXHandshakeCellUint16_t   t16Bits;
};

struct __attribute__((packed)) NetXSystemControlBlock{ // NETX_SYSTEM_CONTROL_BLOCK
	uint32_t ulSystemCommandCOS;	// System Command Change Of State, System Reset = HIL_SYS_RESET_COOKIE to set Reset, Cookie
	uint32_t ulReserved;			// Reserved in NetX50
};


#define HIL_SYS_STATUS_OK 		(uint32_t)0x00000001  // If set, the data in system status register is valid
struct __attribute__((packed)) NetXSystemStatusBlock{ // NETX_SYSTEM_STATUS_BLOCK
	uint32_t ulSystemCOS;			// System Change Of State, General system states
	uint32_t ulSystemStatus;		// System Status: Information about file system, boot medium etc
	uint32_t ulSystemError;			// System Error: Indicates runtime errors of the firmware
	uint32_t ulBootError;			// Boot Error: Indicates faults during the hardware boot process
	uint32_t ulTimeSinceStart;		// Time Since Startup: Time elapsed since system start in seconds
	uint16_t usCpuLoad;				// CPU Load: CPU Load in 0.01% units
	uint16_t usReserved;			// Reserved (Set to Zero)
	uint16_t ulHWFeatures;			// Hardware Features: Available hardware features
	uint8_t abReserved[36];			// Reserved (Set to Zero)
};

struct __attribute__((packed)) NetXSystemMailBox{ // NETX_SYSTEM_SEND_MAILBOX and NETX_SYSTEM_RECV_MAILBOX
	struct __attribute__((packed)) SendMailbox{ // NETX_SYSTEM_SEND_MAILBOX
		uint16_t usPackagesAccepted;// Packages Acceptable:	Number of packets that can be accepted by the firmware
		uint16_t usReserved;		// Reserved (Set to Zero)
		uint8_t abSendMbx[124];	// Send Mailbox Buffer:	Buffer to insert the send packet
	} tSend;
	struct __attribute__((packed)) ReceiveMailbox{ // NETX_SYSTEM_RECV_MAILBOX
		uint16_t usWaitingPackages; // Waiting Packages:	Counter of packets waiting to be read by the host
		uint16_t usReserved;		// Reserved (Set to Zero)
		uint8_t abRecvMbx[124];	// Receive Mailbox Buffer: Buffer containing a packet received from the firmware
	} tReceive;
};

// NETX_HANDSHAKE_CHANNEL -------------------------------------------------------------
struct __attribute__((packed)) NetXHanshakeChannel{ // NETX_HANDSHAKE_CHANNEL
	NetXHandshakeCellUint8_t tSysFlags; 			// System Channel
	NetXHandshakeCellUint16_t tHskFlags; 			// Handshake Channel (contains the fieldbus synchronization flags)
	NetXHandshakeCellUint16_t tCommChannel0Flags; 	// Communication Channel 0
	NetXHandshakeCellUint16_t tCommChannel1Flags; 	// Communication Channel 1
	NetXHandshakeCellUint16_t tCommChannel2Flags; 	// Communication Channel 2
	NetXHandshakeCellUint16_t tCommChannel3Flags; 	// Communication Channel 3
};


// NETX_DEFAULT_COMM_CHANNEL and NETX_8K_DPM_COMM_CHANNEL ----------------------------

/*-----------------------------------*/
/* CHANNEL CONTROL BLOCK             */
/*-----------------------------------*/
/* Application Change of State */
#define RCX_APP_COS_APPLICATION_READY                       0x00000001
#define RCX_APP_COS_BUS_ON                                  0x00000002
#define RCX_APP_COS_BUS_ON_ENABLE                           0x00000004
#define RCX_APP_COS_INITIALIZATION                          0x00000008
#define RCX_APP_COS_INITIALIZATION_ENABLE                   0x00000010
#define RCX_APP_COS_LOCK_CONFIGURATION                      0x00000020
#define RCX_APP_COS_LOCK_CONFIGURATION_ENABLE               0x00000040
#define RCX_APP_COS_DMA                                     0x00000080
#define RCX_APP_COS_DMA_ENABLE                              0x00000100

/*-----------------------------------*/
/* CHANNEL COMMON STATUS BLOCK       */
/*-----------------------------------*/
/* Channel Change Of State flags */
#define RCX_COMM_COS_UNDEFINED                              0x00000000
#define RCX_COMM_COS_READY                                  0x00000001
#define RCX_COMM_COS_RUN                                    0x00000002
#define RCX_COMM_COS_BUS_ON                                 0x00000004
#define RCX_COMM_COS_CONFIG_LOCKED                          0x00000008
#define RCX_COMM_COS_CONFIG_NEW                             0x00000010
#define RCX_COMM_COS_RESTART_REQUIRED                       0x00000020
#define RCX_COMM_COS_RESTART_REQUIRED_ENABLE                0x00000040
#define RCX_COMM_COS_DMA                                    0x00000080

struct __attribute__((packed)) NetXControlBlock { // NETX_CONTROL_BLOCK
	uint32_t ulApplicationCOS;			// Application Change Of State: READY ; BUS ON ; INITIALIZATION ; LOCK CONFIGURATION
	uint32_t ulDeviceWatchdog;			// Device Watchdog: Watchdog counter necessary for the handling and supervision
};

struct __attribute__((packed)) NetXCommonStatusBlock { // NETX_COMMON_STATUS_BLOCK
	uint32_t ulCommunicationCOS;		// Communication Change of State: READY / RUN ; RESET REQUIRED / NEW CONFIG AVAILABLE ; CONFIG LOCKED ;
	uint32_t ulCommunicationState; 		// Communication State:	OFFLINE / STOP / IDLE / OPERATE
	uint32_t ulCommunicationError; 		// Communication Error:	Protocol Stack error number
	uint16_t usVersion;					// Version Number of this structure
	uint16_t usWatchdogTime; 			// Watchdog Time: Configured watchdog time given in milliseconds
	uint8_t bPDInHskMode;				// Handshake Mode: Configured input process data handshake mode
	uint8_t bPDInSource;				// Input Handshake Event Source
	uint8_t bPDOutHskMode;				// Handshake Mode:Configured output process data handshake mode
	uint8_t bPDOutSource;				// Output Handshake Event Source
	uint32_t ulHostWatchdog;			// Host Watchdog: Host watchdog counter used for watchdog handling
	uint32_t ulErrorCount;				// Error Count: Total Number of Detected Errors Since Power-Up or Reset
	uint8_t bErrorLogInd;				// (Not supported yet) Number of Entries in the internal error log
	uint8_t bErrorPDInCnt; 				// Input process data handshake error counter
	uint8_t bErrorPDOutCnt;				// Output process data handshake error counter
	uint8_t bErrorSyncCnt;				// Synchronization handshake error counter
	uint8_t bSyncHskMode;				// Synchronization Handshake Mode
	uint8_t bSyncSource;				// Synchronization Source
	uint16_t ausReserved[3];			// Reserved: Set to 0
	uint32_t aulReserved[6];			// Reserved: Set to 0
};

struct __attribute__((packed)) NetXSendMailBoxBlock{ // NETX_SEND_MAILBOX_BLOCK
	uint16_t usPackagesAccepted;// Packages Acceptable:	Number of packets that can be accepted by the firmware
	uint16_t usReserved;		// Reserved (Set to Zero)
	uint8_t abSendMbx[1596];	// Send Mailbox Buffer:	Buffer to insert the send packet
} ;
struct __attribute__((packed)) NetXReceiveMailBoxBlock{ // NETX_RECV_MAILBOX_BLOCK
	uint16_t usWaitingPackages; // Waiting Packages:	Counter of packets waiting to be read by the host
	uint16_t usReserved;		// Reserved (Set to Zero)
	uint8_t abRecvMbx[1596];	// Receive Mailbox Buffer: Buffer containing a packet received from the firmware
};

struct __attribute__((packed)) NetX16KCommunicationChannel{ // NETX_DEFAULT_COMM_CHANNEL
	uint8_t abReserved[8];							// Not Supported
	NetXControlBlock tControl; 						// Common Control Block
	NetXCommonStatusBlock tCommonStatus;			// Common Status Block
	uint8_t abExtendedStatus[432];					// Extended Status Block
	NetXSendMailBoxBlock tSendMbx;					// Send Mailbox
	NetXSendMailBoxBlock tRecvMbx;					// Receive mailbox
	uint8_t abPd1Output[64];
	uint8_t abPd1Input[64];
	uint8_t abReserved1[256];
	uint8_t abPd0Output[5760];
	uint8_t abPd0Input[5760];
};

/* codes for parameters of "set configuration" packet */
#define ECAT_SET_CONFIG_COE                                         0x00000001
#define ECAT_SET_CONFIG_EOE                                         0x00000002
#define ECAT_SET_CONFIG_FOE                                         0x00000004
#define ECAT_SET_CONFIG_SOE                                         0x00000008
#define ECAT_SET_CONFIG_SYNCMODES                                   0x00000010
#define ECAT_SET_CONFIG_SYNCPDI                                     0x00000020
#define ECAT_SET_CONFIG_UID                                         0x00000040
#define ECAT_SET_CONFIG_AOE                                         0x00000080
#define ECAT_SET_CONFIG_BOOTMBX                                     0x00000100
#define ECAT_SET_CONFIG_DEVICEINFO                                  0x00000200
#define ECAT_SET_CONFIG_SMLENGTH                                    0x00000400

#define ECAT_SET_CONFIG_SYSTEMFLAGS_AUTOSTART                       0x00000000
#define ECAT_SET_CONFIG_SYSTEMFLAGS_APP_CONTROLLED                  0x00000001

#define ECAT_SET_CONFIG_COEDETAILS_ENABLE_SDO                       0x01
#define ECAT_SET_CONFIG_COEDETAILS_ENABLE_SDOINFO                   0x02
#define ECAT_SET_CONFIG_COEDETAILS_ENABLE_PDOASSIGN                 0x04
#define ECAT_SET_CONFIG_COEDETAILS_ENABLE_PDOCONFIGURATION          0x08
#define ECAT_SET_CONFIG_COEDETAILS_ENABLE_UPLOAD                    0x10
#define ECAT_SET_CONFIG_COEDETAILS_ENABLE_SDOCOMPLETEACCESS         0x20

#define ECAT_SET_CONFIG_COEFLAGS_USE_CUSTOM_OD                      0x01

#define ECAT_SET_CONFIG_SYNCPDI_SYNC0_OUTPUT_TYPE_MASK              0x01
#define ECAT_SET_CONFIG_SYNCPDI_SYNC0_POLARITY_MASK                 0x02
#define ECAT_SET_CONFIG_SYNCPDI_SYNC0_OUTPUT_ENABLE_MASK            0x04
#define ECAT_SET_CONFIG_SYNCPDI_SYNC0_IRQ_ENABLE_MASK               0x08
#define ECAT_SET_CONFIG_SYNCPDI_SYNC1_OUTPUT_TYPE_MASK              0x10
#define ECAT_SET_CONFIG_SYNCPDI_SYNC1_POLARITY_MASK                 0x20
#define ECAT_SET_CONFIG_SYNCPDI_SYNC1_OUTPUT_ENABLE_MASK            0x40
#define ECAT_SET_CONFIG_SYNCPDI_SYNC1_IRQ_ENABLE_MASK               0x80

namespace NetXPacket{

	struct __attribute__((packed)) Header
	{
		uint32_t  ulDest;   							/* destination of the packet (task message queue reference) */
		uint32_t  ulSrc;   								/* source of the packet (task message queue reference) */
		uint32_t  ulDestId; 							/* destination reference (internal use for message routing) */
		uint32_t  ulSrcId;  							/* source reference (internal use for message routing) */
		uint32_t  ulLen;    							/* length of packet data (starting from the end of the header) */
		uint32_t  ulId;     							/* identification reference (internal use by the sender) */
		uint32_t  ulSta;    							/* operation status code (error code, initialize with 0) */
		uint32_t  ulCmd;    							/* operation command code */
		uint32_t  ulExt;    							/* extension count (nonzero in multi-packet transfers) */
		uint32_t  ulRout;  								/* router reference (internal use for message routing) */
	} ;

	struct __attribute__((packed)) EtherCATSMALStatusChangedIndicator{
		Header tHeader;
		struct __attribute__((packed)) StructData{
			struct __attribute__ ((packed)) ALStatus
			{
				uint8_t uState                : 4;
				uint8_t fChange               : 1;
				uint8_t reserved              : 3;
				uint8_t bApplicationSpecific  : 8;
			}  tAlStatus;
			uint16_t      usErrorLed;
			uint16_t      usAlStatusCode;
		}tData;
	};

	struct __attribute__((packed)) EtherCATSMALStatusChangesResponse{
		Header tHeader ={
				.ulDest 	= 0,						// RCX_PACKET_DEST_SYSTEM
				.ulSrc 		= 0,
				.ulDestId 	= 0,
				.ulSrcId 	= 0,
				.ulLen 		= 0,						// No Extra Data
				.ulId 		= 0,
				.ulSta 		= 0,
				.ulCmd 		= asNetXCommand(NetXCommand::EtherCATSMALStatusChangesResponse),
				.ulExt 		= 0,
				.ulRout 	= 0	};
	};



	struct __attribute__((packed)) OSFirmawareIdentifyRequest{
		Header tHeader ={
				.ulDest 	= 0,						// RCX_PACKET_DEST_SYSTEM
				.ulSrc 		= 0,
				.ulDestId 	= 0,
				.ulSrcId 	= 0,
				.ulLen 		= sizeof(tData),			// sizeof(ulExt) + sizeof(ulRout)
				.ulId 		= 0,
				.ulSta 		= 0,
				.ulCmd 		= asNetXCommand(NetXCommand::OSFirmawareIdentifyRequest), // Get Block Information Request (RCX_DPM_GET_BLOCK_INFO_REQ)
				.ulExt 		= 0,
				.ulRout 	= 0	};
		struct __attribute__((packed)) Data{
			uint32_t  ulChannelId = 0;
		}tData;
	};

	struct __attribute__((packed)) OSFirmawareIdentifyConfirmation{
		Header tHeader;
		struct __attribute__((packed)) StructData{
			struct __attribute__((packed)) FwVersion{
				uint16_t usMajor; /* firmware major version */
				uint16_t usMinor; /* firmware minor version */
				uint16_t usBuild; /* firmware build */
				uint16_t usRevision; /* firmware revision */
			} tFwVersion;
			struct __attribute__((packed)) FwName{
				uint8_t bNameLength; /* length of firmware name */
				uint8_t abName[63]; /* firmware name */
			} tFwName;
			struct __attribute__((packed)) FwDate{
				uint16_t usYear; /* firmware creation year */
				uint16_t bMonth; /* firmware creation month */
				uint8_t bDay; /* firmware creation day */
			} tFwDate;
		}tData;

		enum class NetXSubBlockType : uint32_t{
			// This field is used to identify the type of sub block. The following types are defined.
			Undefined 				= 0,
			Unknown 				= 1,
			DataImage 				= 2,
			DataImageHighPriority 	= 3,
			Mailbox 				= 4,
			Control 				= 5,
			CommonStatus 			= 6,
			ExtendedStatus 			= 7,
			User 					= 8,
			Reserved 				= 9
		};
	};

	struct __attribute__((packed)) OSSetAddressRequest{
		Header tHeader ={
					.ulDest 	= 0,						// RCX_PACKET_DEST_SYSTEM
					.ulSrc 		= 0,
					.ulDestId 	= 0,
					.ulSrcId 	= 0,
					.ulLen 		= sizeof(tData),			// sizeof(ulExt) + sizeof(ulRout)
					.ulId 		= 0,
					.ulSta 		= 0,
					.ulCmd 		= asNetXCommand(NetXCommand::OSSetAddressRequest),	// RCX_SET_MAC_ADDR_REQ
					.ulExt 		= 0,
					.ulRout 	= 0	};
			struct __attribute__((packed)) NetXPacketSetMACAddressRequestData{
				uint32_t ulParam;             				/* Parameter Bit Field */
				uint8_t  abMacAddr[6];         				/* MAC address */
				uint8_t  abPad[2];             				/* Pad bytes, set to zero */
			}tData;

	} ;

	struct __attribute__((packed)) OSSetAddressConfirmation{
		Header tHeader;
	};

	struct __attribute__((packed)) OSGetBlockInformationRequest{
		Header tHeader ={
				.ulDest 	= 0,						// RCX_PACKET_DEST_SYSTEM
				.ulSrc 		= 0,
				.ulDestId 	= 0,
				.ulSrcId 	= 0,
				.ulLen 		= sizeof(tData),			// sizeof(ulExt) + sizeof(ulRout)
				.ulId 		= 0,
				.ulSta 		= 0,
				.ulCmd 		= asNetXCommand(NetXCommand::OSGetBlockInformationRequest),		// Get Block Information Request (RCX_DPM_GET_BLOCK_INFO_REQ)
				.ulExt 		= 0,
				.ulRout 	= 0	};
		struct __attribute__((packed)) Data{
			uint32_t  ulAreaIndex;    	/* Area Index (0 to 7):
										 	 This field holds the index of the channel.
										 	 The system channel is identified by an index
										 	 number of 0; the handshake has index 1, the
										 	 first communication channel has index 2 and
										 	 so on.*/
			uint32_t  ulSubblockIndex;   /* Sub Block Index: The sub block index field
											 identifies each of the blocks that reside in the dual-port memory interface for
											 the specified communication channel  */
		}tData;
	};

	struct __attribute__((packed)) OSGetBlockInformationConfirmation{
		Header tHeader;
		struct __attribute__((packed)) Data{
			uint32_t  ulAreaIndex;   	/* Area Index (0 to 7):
										 This field holds the index of the channel.
										 The system channel is identified by an index
										 number of 0; the handshake has index 1, the
										 first communication channel has index 2 and
										 so on.*/
			uint32_t  ulSubblockIndex;  /* Number of Sub Blocks */
			uint32_t  ulType;			/* Type of Sub Block:  */
			uint32_t  ulOffset; 		/* offset of this sub block within the area */
			uint32_t  ulSize; 			/* size of the sub block */
			uint16_t  usFlags; 			/* flags of the sub block */
			uint16_t  usHandshakeMode; 	/* handshake mode */
			uint16_t  usHandshakeBit; 	/* bit position in the handshake register */
			uint16_t  usReserved; 		/* reserved */
		}tData;

		enum class NetXSubBlockType : uint32_t{
			// This field is used to identify the type of sub block. The following types are defined.
			Undefined 				= 0,
			Unknown 				= 1,
			DataImage 				= 2,
			DataImageHighPriority 	= 3,
			Mailbox 				= 4,
			Control 				= 5,
			CommonStatus 			= 6,
			ExtendedStatus 			= 7,
			User 					= 8,
			Reserved 				= 9
		};
	};

	struct __attribute__((packed)) EtherCATSetConfigurationRequest{
		Header tHeader ={
				.ulDest 	= 0x00000020,				// Destination Queue Handle CHANNEL (Local)
				.ulSrc 		= 0,
				.ulDestId 	= 0,
				.ulSrcId 	= 0,
				.ulLen 		= sizeof(tData),
				.ulId 		= 0,
				.ulSta 		= 0,
				.ulCmd 		= asNetXCommand(NetXCommand::EtherCATSetConfigurationRequest), // ECAT_SET_CONFIG_REQ
				.ulExt 		= 0,
				.ulRout 	= 0	};

		struct __attribute__((packed)) Data
		{
			struct __attribute__((packed)) BasicConfigurationData
			{
				uint32_t ulSystemFlags;
				uint32_t ulWatchdogTime;
				uint32_t ulVendorId;
				uint32_t ulProductCode;
				uint32_t ulRevisionNumber;
				uint32_t ulSerialNumber;
				uint32_t ulProcessDataOutputSize;
				uint32_t ulProcessDataInputSize;
				uint32_t ulComponentInitialization;
				uint32_t ulExtensionNumber;
			} tBasicConfig;

			struct __attribute__((packed)) ComponentsConfigurationData
			{
				struct __attribute__ ((packed)) EtherCATConfigurationCOE
				{
					 uint8_t  bCoeFlags;
					 uint8_t  bCoeDetails;
					 uint32_t ulOdIndicationTimeout;
					 uint32_t ulDeviceType;
					 uint16_t usReserved;
				} tCoE;

				struct __attribute__((packed)) EtherCATConfigurationEOE
				{
					uint32_t ulReserved;
				} tEoE;		// Not supported

				struct __attribute__((packed)) EtherCATConfigurationFOE
				{
					uint32_t ulTimeout;
				} tFoE;

				struct __attribute__((packed)) EtherCATConfigurationSOE
				{
					uint32_t ulIdnIndicationTimeout;
				} tSoE;

				 struct __attribute__((packed)) EtherCATConfigurationSyncModes
				{
					uint8_t  bPDInHskMode;
					uint8_t  bPDInSource;
					uint16_t usPDInErrorTh;
					uint8_t  bPDOutHskMode;
					uint8_t  bPDOutSource;
					uint16_t usPDOutErrorTh;
					uint8_t  bSyncHskMode;
					uint8_t  bSyncSource;
					uint16_t usSyncErrorTh;
				} tSyncModes;

				struct __attribute__((packed)) EtherCATConfigurationSyncPDI
				{
					uint8_t  bSyncPdiConfig;
					uint16_t usSyncImpulseLength;
					uint8_t  bReserved;
				}  tSyncPdi;

				struct __attribute__((packed)) EtherCATConfigurationUID
				{
					uint16_t usStationAlias;
					uint16_t usDeviceIdentificationValue;
				}  tUid;

				struct __attribute__((packed)) EtherCATConfigurationBootMailBox
				{
					uint16_t usBootstrapMbxSize;
				}  tBootMxb;

				struct __attribute__((packed)) EtherCATConfigurationDeviceInformation
				{
					uint8_t bGroupIdxLength;
					char   szGroupIdx[127];     /* Matches ESI DeviceType:Group Type     */
					uint8_t bImageIdxLength;
					char   szImageIdx[255];     /* Matches ESI DeviceType:ImageData16x14 */
					uint8_t bOrderIdxLength;
					char   szOrderIdx[127];     /* Matches ESI DeviceType:Type           */
					uint8_t bNameIdxLength;
					char   szNameIdx[127];      /* Matches ESI DeviceType:Name          */
				} tDeviceInfo;

				struct __attribute__((packed)) EtherCATConfigurationSmLength
				{
					uint16_t  usMailboxSize;
					uint16_t usSM2StartAddress;
					uint16_t  usSM3StartAddress;
				}  tSmLength;

			} tComponentsConfig;
		}tData;
	};

	struct __attribute__((packed)) EtherCATSetConfigurationConfirmation{
			Header tHeader;
	};


	struct __attribute__((packed)) OSRegisterApplicationRequest{
		Header tHeader ={
				.ulDest 	= 0x00000020,				// Destination Queue Handle CHANNEL
				.ulSrc 		= 0,
				.ulDestId 	= 0,
				.ulSrcId 	= 0,
				.ulLen 		= 0,						// No Data
				.ulId 		= 0,
				.ulSta 		= 0,
				.ulCmd 		= asNetXCommand(NetXCommand::OSRegisterApplicationRequest),	// Get Block Information Request (RCX_DPM_GET_BLOCK_INFO_REQ)
				.ulExt 		= 0,
				.ulRout 	= 0	};
	};

	struct __attribute__((packed)) OSRegisterApplicationConfirmation{
		Header tHeader;
	};

	struct __attribute__((packed)) OSStartStopCommunicationRequest{
		Header tHeader ={
				.ulDest 	= 0x00000020,				// Destination Queue Handle CHANNEL
				.ulSrc 		= 0,
				.ulDestId 	= 0,
				.ulSrcId 	= 0,
				.ulLen 		= sizeof(tData),
				.ulId 		= 0,
				.ulSta 		= 0,
				.ulCmd 		= asNetXCommand(NetXCommand::OSStartStopCommunicationRequest),	// Get Block Information Request (RCX_DPM_GET_BLOCK_INFO_REQ)
				.ulExt 		= 0,
				.ulRout 	= 0	};
		struct __attribute__((packed)) Data
		{
			uint32_t ulParam;							// 1- Start | 2 - Stop
		}tData;
	};

	struct __attribute__((packed)) OSStartStopCommunicationConfirmation{
		Header tHeader;
	};

	struct __attribute__((packed)) OSChannelInitializationRequest{
		Header tHeader ={
				.ulDest 	= 0x00000020,				// Destination Queue Handle CHANNEL
				.ulSrc 		= 0,
				.ulDestId 	= 0,
				.ulSrcId 	= 0,
				.ulLen 		= 0,						// No Data
				.ulId 		= 0,
				.ulSta 		= 0,
				.ulCmd 		= asNetXCommand(NetXCommand::OSChannelInitializationRequest),	// Get Block Information Request (RCX_DPM_GET_BLOCK_INFO_REQ)
				.ulExt 		= 0,
				.ulRout 	= 0	};
	};

	struct __attribute__((packed)) OSChannelInitializationConfirmation{
		Header tHeader;
	};

	struct __attribute__((packed)) OSLinkStatusChangeIndicator{
		Header tHeader;
		struct __attribute__((packed)) Data
		{
			struct __attribute__((packed)) LinkStatus
			{

				uint32_t  ulPort;           /*!< Port the link status is for */
				uint32_t  fIsFullDuplex;    /*!< If a full duplex link is available on this port */
				uint32_t  fIsLinkUp;        /*!< If a link is available on this port */
				uint32_t  ulSpeed;        	/*!< Speed of the link:
												 0:   No link
												 10:  10MBit
												 100: 100MBit */
			}atLinkData[2];
		}tData;
	};

	struct __attribute__((packed)) OSLinkStatusChangeResponse{
			Header tHeader ={
					.ulDest 	= 0,						// RCX_PACKET_DEST_SYSTEM
					.ulSrc 		= 0,
					.ulDestId 	= 0,
					.ulSrcId 	= 0,
					.ulLen 		= 0,						// No Extra Data
					.ulId 		= 0,
					.ulSta 		= 0,
					.ulCmd 		= asNetXCommand(NetXCommand::OSLinkStatusChangeResponse),
					.ulExt 		= 0,
					.ulRout 	= 0	};
		};
};

void debugNetXSystemInfoBlock(NetXSystemInfoBlock& block);

void debugNetXSystemChannelInfo(NetXSystemChannelInfo& block);

void debugNetXHandshakeChannelInfo(NetXHandshakeChannelInfo& block);

void debugNetXCommunicationChannelInfo(NetXCommunicationChannelInfo& block, uint8_t id);

void debugNetXApplicationChannelInfo(NetXApplicationChannelInfo& block, uint8_t id);

void debugNetXChannelInfoBlock(NetXChannelInfoBlock& block);

template<typename T>
void debugNetXDefault( T &pkt){};


void debugNetXPacketEtherCATSMALStatusChangedIndicator(const NetXPacket::EtherCATSMALStatusChangedIndicator &pkt);
void debugNetXPacketOSGetBlockInformationConfirmation(const NetXPacket::OSGetBlockInformationConfirmation &pkt);
void debugNetXPacketOSFirmawareIdentifyConfirmation(const NetXPacket::OSFirmawareIdentifyConfirmation &pkt);
void debugNetXPacketOSLinkStatusChangeIndicator(const  NetXPacket::OSLinkStatusChangeIndicator &pkt);

void debugNetXCommunicationCOSOfCommonStatusBlockOfCommunicationChannel( uint32_t val );

void debugNetXApplicationCOSOfCommonControlBlockOfCommunicationChannel( uint32_t val );

void debugNetXPacketHeader( NetXPacket::Header &tHeader );

#endif // NETX_ENABLE

#endif /* SRC_HARDWARE_NETX_NETXSTRUCTS_H_ */
