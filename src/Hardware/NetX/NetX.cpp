/*
 * NetX.cpp
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#include <Hardware/NetX/NetX.h>

#if NETX_ENABLE

#include <Platform/TaskPriorities.h>
#include "Hardware/NetX/NetXSocket.h"

constexpr uint32_t 	EtherCATProductCode 			= 0x00000038;
constexpr uint32_t 	EtherCATVendorId 				= 0xE0000044;		// (Hilscher)
constexpr uint32_t 	EtherCATRevision 				= 1;
constexpr char 		EtherCATOrderName[] 			= "KreuzBoard";
constexpr char 		EtherCATDeviceName[] 			= "CB-MA XBoard";

constexpr uint32_t 	EtherCATWatchdogTime 			= 1000;
constexpr uint32_t 	EtherCATTimeout 				= 1000;

constexpr size_t NetXTaskStackWords = 800;
static Task<NetXTaskStackWords> *netXTask;

constexpr size_t NetXSocketTaskStackWords = 200;
static Task<NetXSocketTaskStackWords> *netXSocketTask;

static __nocache __attribute__((aligned(4))) EtherCATApplicationMOSIData dataMOSI;
static __nocache __attribute__((aligned(4))) EtherCATApplicationMISOData dataMISO;

void NetX::ProcessApplicationMessages() noexcept
{
	if( ReadDataInput(dataMOSI) )
	{
		applicationProtocol.ProcessMOSIData(dataMOSI, dataMISO);
	}
	applicationProtocol.PrepareMISOData(dataMOSI, dataMISO);
	WriteDataOutput(dataMISO);
}

[[noreturn]] void NetXSocketTaskCode(void*) noexcept
{
	NetXSocket &netXSocket = NetXSocket::GetInstance();
	while(true)
	{
		if(netXSocket.NeedsPolling())
		{
			netXSocket.Poll();
		}
		delay(1);
	}
}

[[noreturn]] void NetXTaskCode(void*) noexcept
{
	// Let's make sure the NetX is ON.
	while(true)
	{

		delay(750);

		NetX &netX = NetX::GetInstance();
		netX.Reset();
		if(netX.EnableChannel())
		{
#ifdef NETX_DEBUG
			debugPrintf("NetX Channel Enabled\n");
			debugPrintf("-----------------------------------------------------\n\n");
#endif
			if(netXSocketTask != nullptr)
			{
				netXSocketTask->Create(NetXSocketTaskCode, "NETXSOCKET", nullptr, TaskPriority::NetXSocketPriority);
			}
			while(true)
			{
				netX.Spin();
			}
		}
	}
}

void NetX::Spin() noexcept
{
	while(GetAndProcessNonCyclicConfirmation(NetXChannel::Communication0) != 0);
	ProcessApplicationMessages();
	delay(1);
}

bool NetX::EnableChannel() noexcept
{
	isInit = false;

	if( IsValidVersion() )
	{
		// It is present!
		DummyRead();

		// Let'sget the name in the system information block
		ASSERT_NETX(GetAndProcessSystemInformationBlock(), true, "ERROR: NetX Processing Information Block\n");

		ASSERT_NETX(GetAndProcessChannelInformationBlock(), true, "ERROR: NetX Processing Channel Information Block\n");

		ASSERT_NETX(IsReady(), true, "ERROR: NetX is not ready\n" )

		// Check if it is in error state
		ASSERT_NETX(ReadHandshakeNetXFlags<NonSystemFlags>(NetXChannel::Handshake) & NCF_ERROR, 0, "ERROR: NetX In Error state\n" )

		// Reading the system status and checking it is OK
		ASSERT_NETX(GetSystemStatus() & HIL_SYS_STATUS_OK, HIL_SYS_STATUS_OK, "ERROR: System status not valid\n" )


		SystemFlags systemNetxFlags = ReadHandshakeNetXFlags<SystemFlags>(NetXChannel::System);
		WaitBitInHandshakeHostFlagToBe<SystemFlags>(NetXChannel::System, HSF_SEND_MBX_CMD, systemNetxFlags & HSF_SEND_MBX_CMD);

		for(uint32_t ulBlockIdX = 0; ulBlockIdX < 9 ; ++ulBlockIdX )
		{
			RequestChannelLayout(ulBlockIdX);
		}

		uint16_t usCommChannel0FlagsHostFlags = ReadHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0);
		uint32_t ulCommunicationCOS = GetCommunicationCOS(NetXChannel::Communication0);

		// GetApplicationCOS(NetXChannel::Communication0);  // Just for Debug

		// TODO: if ulCommunicationCOS and ulApplicationCOS are not 0 ?
		if( ulCommunicationCOS == 0 )
		{
			uint16_t usCommChannel0FlagsNetxFlags = ReadHandshakeNetXFlags<NonSystemFlags>(NetXChannel::Communication0);
			// Toggle NCF_NETX_COS_CMD
			if((usCommChannel0FlagsHostFlags ^ NCF_NETX_COS_CMD) != (usCommChannel0FlagsNetxFlags ^ NCF_NETX_COS_CMD))
			{
				WriteHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0, NCF_NETX_COS_CMD);
				usCommChannel0FlagsNetxFlags = ReadHandshakeNetXFlags<NonSystemFlags>(NetXChannel::Communication0);
			}

			ulCommunicationCOS = GetCommunicationCOS(NetXChannel::Communication0);

			if(usCommChannel0FlagsNetxFlags == 0)
			{
				WriteHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0, 0);
				// TODO: Remove. CHeck that it is 0
				ReadHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0);
			}
		}

		RequestFirmware();

		ASSERT_NETX(GetSystemError(), 0,"ERROR: There is an error in the Firmware");

		Reset();

		SetDefaultMacAddress();

		if(WaitForChannelConnected(DefaultIsConnectedTimeout_ms))
		{
#ifdef NETX_DEBUG
			debugPrintf("NetX is Ready!\n");
#endif
			applicationProtocol.Reset();
			isInit = true;
			return true;
		}
	}
	return false;
}

bool NetX::WaitForChannelConnected(uint32_t timeout) noexcept
{
	const uint32_t CommunicationCOSIsConnected =
			RCX_APP_COS_APPLICATION_READY | RCX_APP_COS_BUS_ON | RCX_APP_COS_BUS_ON_ENABLE;

	for (uint32_t i = 0; i < timeout; ++i )
	{
		delay(1);
		while(GetAndProcessNonCyclicConfirmation(NetXChannel::Communication0) != 0);
		delay(1);
		// Discard the data as we are not connected
		DiscardInputDataFrame();
		delay(1);
		const uint32_t communicationCOS = GetCommunicationCOS(NetXChannel::Communication0);
		// Check if it is connected and running
		if((communicationCOS & CommunicationCOSIsConnected) == CommunicationCOSIsConnected)
		{
			return true;
		}
	}
	return false;
}

void NetX::RequestFirmware() noexcept
{
	// debugPrintf("\n");
	// debugPrintf("Firmware Request --------------------------------------------------\n");
	NetXPacket::OSFirmawareIdentifyRequest tSend;
	SendNonCyclicPacket(NetXChannel::Communication0, tSend);
	delay(10);
	GetAndProcessNonCyclicConfirmation(NetXChannel::Communication0);
	// debugPrintf("Firmware Request DONE ---------------------------------------------\n");
}

void NetX::DiscardInputDataFrame() noexcept
{
	// Wait for COS.
	NonSystemFlags hostFlags = ReadHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0);
	NonSystemFlags netXFlags = ReadHandshakeNetXFlags<NonSystemFlags>(NetXChannel::Communication0);
	if( (hostFlags ^ netXFlags) & NCF_PD0_IN_CMD )
	{
		WriteHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0, hostFlags ^ NCF_PD0_IN_CMD);
	}
}


bool NetX::ReadDataInput(EtherCATApplicationMOSIData &data) noexcept
{
	// Wait for COS.

	NonSystemFlags netXFlags = ReadHandshakeNetXFlags<NonSystemFlags>( NetXChannel::Communication0);
	auto addressToRead = NetXAddress(Nx50AddressMap::CommunicationChannel0)
			.Append(offsetof(NetX16KCommunicationChannel,abPd0Input));

	//ASSERT_NETX(ReadAddressNonBlockingMode(netXTask,addressToRead,data), true, "Error getting Frame\n" );
	if(!ReadAddressNonBlockingMode(netXTask,addressToRead,data))
	{
		debugPrintf("Reading\n");
		delay(1);
	}
	WriteHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0, netXFlags ^ NCF_PD0_IN_CMD );
	WaitBitInHandshakeNetXFlagToBe<NonSystemFlags>(NetXChannel::Communication0, HCF_PD0_IN_ACK, netXFlags ^ HCF_PD0_IN_ACK);

	return true;
}

bool NetX::WriteDataOutput(EtherCATApplicationMISOData &data) noexcept
{
	// Wait for COS.
	NonSystemFlags netXFlags = ReadHandshakeNetXFlags<NonSystemFlags>(NetXChannel::Communication0);
	auto addressToWrite = NetXAddress(Nx50AddressMap::CommunicationChannel0)
			.Append(offsetof(NetX16KCommunicationChannel,abPd0Output));
	//ASSERT_NETX(SendPacketWithHeaderNonBlocking(netXTask,addressToWrite,data), true, "Error writing Frame\n" );
	if(!SendPacketWithHeaderNonBlocking(netXTask,addressToWrite,data))
	{
		debugPrintf("Writing\n");
		delay(1);
	}
	else
	{
		WriteHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0, netXFlags ^ HCF_PD0_OUT_CMD );
		WaitBitInHandshakeNetXFlagToBe<NonSystemFlags>(NetXChannel::Communication0, HCF_PD0_IN_ACK, netXFlags ^ NCF_PD0_OUT_ACK);
	}
	return true;
}

uint32_t NetX::GetApplicationCOS(const NetXChannel channel) noexcept
{
	auto AddressChannel0ApplicationCOS = NetXAddress(Nx50AddressMap::CommunicationChannel0)
		.Append(sizeof(NetX16KCommunicationChannel) * (asDPMAddress(channel) - asDPMAddress(NetXChannel::Communication0)))
		.Append(offsetof(NetX16KCommunicationChannel,tControl))
		.Append(offsetof(NetXControlBlock,ulApplicationCOS));
	uint32_t ulApplicationCOS = spi.GetIntegerFromAddress<uint32_t>(AddressChannel0ApplicationCOS);

	debugNetXApplicationCOSOfCommonControlBlockOfCommunicationChannel(ulApplicationCOS);

	return ulApplicationCOS;
}


uint32_t NetX::GetCommunicationCOS(const NetXChannel channel) noexcept
{
	auto AddressChannel0CommunicationCOS = NetXAddress(Nx50AddressMap::CommunicationChannel0)
		.Append(sizeof(NetX16KCommunicationChannel) * (asDPMAddress(channel) - asDPMAddress(NetXChannel::Communication0)))
		.Append(offsetof(NetX16KCommunicationChannel,tCommonStatus))
		.Append(offsetof(NetXCommonStatusBlock,ulCommunicationCOS));
	uint32_t ulCommunicationCOS =  spi.GetIntegerFromAddress<uint32_t>(AddressChannel0CommunicationCOS);
	debugNetXCommunicationCOSOfCommonStatusBlockOfCommunicationChannel(ulCommunicationCOS);
	return ulCommunicationCOS;

}

uint32_t NetX::GetStatusCOS(const NetXChannel channel) noexcept
{
	auto AddressChannel0StatuCOS = NetXAddress(Nx50AddressMap::CommunicationChannel0)
		.Append(sizeof(NetX16KCommunicationChannel) * (asDPMAddress(channel) - asDPMAddress(NetXChannel::Communication0)))
		.Append(offsetof(NetX16KCommunicationChannel,tCommonStatus) + offsetof(NetXCommonStatusBlock,ulCommunicationState));

	return spi.GetIntegerFromAddress<uint32_t>(AddressChannel0StatuCOS);

}

uint32_t NetX::GetErrorCOS(const NetXChannel channel) noexcept
{
	auto AddressChannel0ErrorCOS = NetXAddress(Nx50AddressMap::CommunicationChannel0)
		.Append(sizeof(NetX16KCommunicationChannel) * (asDPMAddress(channel) - asDPMAddress(NetXChannel::Communication0)))
		.Append(offsetof(NetX16KCommunicationChannel,tCommonStatus))
		.Append(offsetof(NetXCommonStatusBlock,ulCommunicationError));

	return spi.GetIntegerFromAddress<uint32_t>(AddressChannel0ErrorCOS);
}


void NetX::RequestApplication() noexcept
{
	// Wait for COS.
	NetXPacket::OSRegisterApplicationRequest tSend;
	SendNonCyclicPacket(NetXChannel::Communication0, tSend);
}

void NetX::HandlerNetXPacketOSRegisterApplicationConfirmation(const NetXPacket::OSRegisterApplicationConfirmation &pkt) noexcept
{
	SetEtherCATConfiguration(ulSerialNumber);
}

void NetX::SetDefaultMacAddress(const uint8_t pMacAddr[6]) noexcept
{
	// Wait for COS.
	NetXPacket::OSSetAddressRequest tSend;

	memset(&tSend.tData, 0, sizeof(tSend.tData));

	// Loading the default MAC Address
	memcpy(tSend.tData.abMacAddr, pMacAddr, 6 );

	SendNonCyclicPacket(NetXChannel::Communication0, tSend);

}

void NetX::HandlerNetXPacketOSSetAddressConfirmation(const NetXPacket::OSSetAddressConfirmation &pkt) noexcept
{
	RequestApplication();
}

// Device number is available in the NetXSystemInfoBlock
void NetX::SetEtherCATConfiguration(const uint32_t serialNumber) noexcept
{
	NetXPacket::EtherCATSetConfigurationRequest tRequest;
	// Clear the configuration
	memset(&tRequest.tData, 0, sizeof(tRequest.tData));

	// Information about the device
	tRequest.tData.tBasicConfig.ulVendorId 			= EtherCATVendorId;
	tRequest.tData.tBasicConfig.ulProductCode 		= EtherCATProductCode;
	tRequest.tData.tBasicConfig.ulRevisionNumber 	= EtherCATRevision;
	tRequest.tData.tBasicConfig.ulSerialNumber		= serialNumber;

	tRequest.tData.tBasicConfig.ulSystemFlags 		= ECAT_SET_CONFIG_SYSTEMFLAGS_APP_CONTROLLED;
	tRequest.tData.tBasicConfig.ulWatchdogTime 		= EtherCATWatchdogTime;

	// Set the device information
	tRequest.tData.tBasicConfig.ulComponentInitialization |= ECAT_SET_CONFIG_DEVICEINFO;

	strncpy(tRequest.tData.tComponentsConfig.tDeviceInfo.szOrderIdx,
			EtherCATOrderName,
			sizeof(tRequest.tData.tComponentsConfig.tDeviceInfo.szOrderIdx));
	tRequest.tData.tComponentsConfig.tDeviceInfo.bOrderIdxLength = strlen(EtherCATOrderName); 	// TODO: Check strlen(EtherCATOrderName) < 127

	strncpy(tRequest.tData.tComponentsConfig.tDeviceInfo.szNameIdx,
			EtherCATDeviceName,
			sizeof(tRequest.tData.tComponentsConfig.tDeviceInfo.szNameIdx));
		tRequest.tData.tComponentsConfig.tDeviceInfo.bNameIdxLength = strlen(EtherCATDeviceName); // TODO: Check strlen(EtherCATDeviceName) < 127


	// Input and Output Data sizes (From the master view)
	/**< Process Data Output Size from master view */
	tRequest.tData.tBasicConfig.ulProcessDataOutputSize = sizeof(EtherCATApplicationMOSIData);
	/**< Process Data Input Size from master view */
	tRequest.tData.tBasicConfig.ulProcessDataInputSize = sizeof(EtherCATApplicationMISOData);

	// CoE (CAN Application Protocol over EtherCAT)
	tRequest.tData.tBasicConfig.ulComponentInitialization |= ECAT_SET_CONFIG_COE;	 // Enable CoE

	tRequest.tData.tComponentsConfig.tCoE.bCoeDetails =
			ECAT_SET_CONFIG_COEDETAILS_ENABLE_SDO
			| ECAT_SET_CONFIG_COEDETAILS_ENABLE_SDOINFO
			| ECAT_SET_CONFIG_COEDETAILS_ENABLE_UPLOAD
			| ECAT_SET_CONFIG_COEDETAILS_ENABLE_SDOCOMPLETEACCESS;

	tRequest.tData.tComponentsConfig.tCoE.ulOdIndicationTimeout = EtherCATTimeout;
	SendNonCyclicPacket(NetXChannel::Communication0, tRequest);
	delay(1000);
}

void NetX::HandlerNetXPacketEtherCATSetConfigurationConfirmation(const NetXPacket::EtherCATSetConfigurationConfirmation &pkt) noexcept
{
	ChannelInitializationRequest();
}

void NetX::ChannelInitializationRequest() noexcept
{
	NetXPacket::OSChannelInitializationRequest tRequest;
	SendNonCyclicPacket(NetXChannel::Communication0, tRequest);
}

void NetX::HandlerNetXPacketOSChannelInitializationConfirmation(const NetXPacket::OSChannelInitializationConfirmation &pkt) noexcept
{
	StartStopCommunicationRequest(true);
}

void NetX::StartStopCommunicationRequest(bool start) noexcept
{
	NetXPacket::OSStartStopCommunicationRequest tRequest;
	tRequest.tData.ulParam = start ? 1 : 2;
	SendNonCyclicPacket(NetXChannel::Communication0, tRequest);
}

void NetX::HandlerNetXPacketOSStartStopCommunicationConfirmation(const NetXPacket::OSStartStopCommunicationConfirmation &pkt) noexcept
{
	// debugPrintf("Communication Start/Stop DONE -------------------------------------\n");
}


void NetX::WaitForApplicationReady() noexcept
{
	// NonSystemFlags netxFlagsComm0 = ReadHandshakeNetXFlags<NonSystemFlags>(NetXChannel::Communication0);
	NonSystemFlags hostFlagsComm0 = ReadHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0);
	WaitBitInHandshakeNetXFlagToBe<NonSystemFlags>(NetXChannel::Communication0, NCF_NETX_COS_CMD, hostFlagsComm0 ^ NCF_NETX_COS_CMD);
	ToggleHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0, HCF_NETX_COS_ACK); // ACK

	NonSystemFlags netxFlag = WaitBitInHandshakeNetXFlagToBe<NonSystemFlags>(NetXChannel::Communication0, NCF_NETX_COS_CMD, hostFlagsComm0);

	ToggleHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0, HCF_NETX_COS_ACK); // ACK

	// Enable the application Ready
	const DPMAddress AddressApplicationCOS = asDPMAddress(Nx50AddressMap::CommunicationChannel0)
									+ offsetof(NetX16KCommunicationChannel,tControl) + offsetof(NetXControlBlock,ulApplicationCOS);
	WriteIntegerInAddressDPM<uint32_t>(AddressApplicationCOS,RCX_APP_COS_APPLICATION_READY);

	ToggleHandshakeHostFlags<NonSystemFlags>(NetXChannel::Communication0, HCF_HOST_COS_CMD);
	WaitBitInHandshakeNetXFlagToBe<NonSystemFlags>(NetXChannel::Communication0, NCF_HOST_COS_ACK, netxFlag ^ NCF_HOST_COS_ACK);

	if(!CheckProtocolStackStartedProperly(Nx50AddressMap::CommunicationChannel0))
	{
#ifdef NETX_DEBUG
		debugPrintf("WARNING: Communication Channel 0 Stack: NOT RUNNING!");
#endif
	}
}

bool NetX::CheckProtocolStackStartedProperly(Nx50AddressMap communicationChannelOnly) noexcept
{
	if(asDPMAddress(communicationChannelOnly) < asDPMAddress(Nx50AddressMap::CommunicationChannel0))
	{
#ifdef NETX_DEBUG
		debugPrintf("ERROR: CheckProtocolStackStartedProperly Just Communication Channels\n");
#endif
		return false;
	}

	auto AddressChannel0CommunicationCOS = NetXAddress(communicationChannelOnly)
			.Append(offsetof(NetX16KCommunicationChannel,tCommonStatus))
			.Append(offsetof(NetXCommonStatusBlock,ulCommunicationCOS));

	return (spi.GetIntegerFromAddress<uint32_t>(AddressChannel0CommunicationCOS) & RCX_COMM_COS_READY) != 0;
}

void NetX::RequestChannelLayout(uint32_t blockIndex) noexcept
{
	// insert the packet index
	NetXPacket::OSGetBlockInformationRequest tSend;

	// tSend.tHeader.ulId = ulPacketIdx;
	tSend.tData.ulAreaIndex = asDPMAddress(NetXChannel::Communication0);
	tSend.tData.ulSubblockIndex = blockIndex;

	SendNonCyclicPacket(NetXChannel::System, tSend);
	delay(1);
	GetAndProcessNonCyclicConfirmation(NetXChannel::System);
}

bool NetX::IsValidVersion() noexcept
{
	constexpr uint8_t 	ValidVersion = 0x11;
	constexpr uint8_t 	ValidVersionMask = 0x1F;
	constexpr uint8_t 	checkValidMessagePkg[3] = { 0x80, 0xFF, 0x84 };

	bool ret = false;
	uint32_t versionDetected  = 0;
	isInit = false;

	if(spi.TransceiveData( (uint8_t *)checkValidMessagePkg, (uint8_t *)&versionDetected, sizeof(checkValidMessagePkg) ))
	{
		ret = (versionDetected & ValidVersionMask) == ValidVersion ;
	}
	return ret;
}

void NetX::Reset() noexcept
{
	// RESET --------------------------
	// debugPrintf("Reseting the device -----------------------------------------------\n");
	WaitBitInHandshakeNetXFlagToBe<SystemFlags>(NetXChannel::System, NSF_READY, NSF_READY );

	const DPMAddress AddressSystemCommandCOS = asDPMAddress(Nx50AddressMap::SystemControl);
	WriteIntegerInAddressDPM<uint32_t>(AddressSystemCommandCOS,0x55AA55AA); // HIL_SYS_RESET_COOKIE

	// Getting the NetX flags before toggle RESET bit
	SystemFlags netxFlags = ReadHandshakeNetXFlags<SystemFlags>(NetXChannel::System);
	ToggleHandshakeHostFlags<SystemFlags>(NetXChannel::System, HSF_RESET);

	// Read until it changes
	WaitBitInHandshakeNetXFlagToBe<SystemFlags>(NetXChannel::System, NSF_READY, netxFlags ^ NSF_READY);
	delay(500); // Delay 500 ms after reset

	// Let's wait it is back
	WaitBitInHandshakeNetXFlagToBe<SystemFlags>(NetXChannel::System, NSF_READY, NSF_READY );
	delay(10);
	// debugPrintf("Reset DONE --------------------------------------------------------\n\n");
}

void NetX::HandlerNetXPacketEtherCATSMALStatusChangedIndicator(const NetXPacket::EtherCATSMALStatusChangedIndicator &pkt) noexcept
{
	debugNetXPacketEtherCATSMALStatusChangedIndicator(pkt);
	delay(10);
	NetXPacket::EtherCATSMALStatusChangesResponse tResponse;
	tResponse.tHeader.ulSrc = pkt.tHeader.ulSrc;
	tResponse.tHeader.ulSrcId = pkt.tHeader.ulSrcId;
	SendNonCyclicPacket(NetXChannel::Communication0, tResponse);
}

void NetX::HandlerNetXPacketOSLinkStatusChangeIndicator(const  NetXPacket::OSLinkStatusChangeIndicator &pkt) noexcept
{
	debugNetXPacketOSLinkStatusChangeIndicator(pkt);
	delay(10);
	NetXPacket::OSLinkStatusChangeResponse tResponse;
	tResponse.tHeader.ulSrc = pkt.tHeader.ulSrc;
	tResponse.tHeader.ulSrcId = pkt.tHeader.ulSrcId;
	SendNonCyclicPacket(NetXChannel::Communication0, tResponse);
}


uint16_t NetX::GetAndProcessNonCyclicConfirmation(NetXChannel channel) noexcept
{
	if(asDPMAddress(channel) == asDPMAddress(NetXChannel::Handshake))
	{
#ifdef NETX_DEBUG
		debugPrintf("ERROR: Handshake channel not supported\n");
#endif
		return 0;
	}

	// Check if there are waiting packages
	NetXAddress AddressReceiveMbx ;
	NetXAddress AddressWaitingPackages;
	NetXAddress AddressReceiveMbxBuffer;
	uint16_t waitingPackages = 0;
	if(channel == NetXChannel::System)
	{
		AddressReceiveMbx.Append(Nx50AddressMap::SystemSendMailbox)
				.Append(offsetof(NetXSystemMailBox,tReceive));
		AddressWaitingPackages.Append(AddressReceiveMbx)
				.Append(offsetof(NetXSystemMailBox::ReceiveMailbox,usWaitingPackages));
		AddressReceiveMbxBuffer.Append(AddressReceiveMbx)
				.Append(offsetof(NetXSystemMailBox::ReceiveMailbox,abRecvMbx));
	}
	else
	{
		AddressReceiveMbx.Append(Nx50AddressMap::CommunicationChannel0)
				.Append( sizeof(NetX16KCommunicationChannel) * (asDPMAddress(channel) - asDPMAddress(NetXChannel::Communication0)))
				.Append(offsetof(NetX16KCommunicationChannel,tRecvMbx));
		AddressWaitingPackages.Append(AddressReceiveMbx)
				.Append(offsetof(NetXReceiveMailBoxBlock,usWaitingPackages));
		AddressReceiveMbxBuffer.Append(AddressReceiveMbx)
				.Append(offsetof(NetXReceiveMailBoxBlock,abRecvMbx));
	}

	waitingPackages = spi.GetIntegerFromAddress<uint16_t>(AddressWaitingPackages);

	if(waitingPackages > 0)
	{
#ifdef NETX_DEBUG
		debugPrintf("Waiting Packages: %u\n", waitingPackages);
#endif

		NetXPacket::Header tHeader;
		spi.ReadAddress(AddressReceiveMbxBuffer, tHeader);
		debugNetXPacketHeader(tHeader);

		switch(static_cast<NetXCommand>(tHeader.ulCmd))
		{
#define X(name, number, func, str)									\
			case(NetXCommand::name): 								\
			{														\
				NetXPacket::name tReceive;							\
				ReceiveNonCyclicPackage(channel,tReceive);  		\
				func(tReceive);										\
			}														\
			break;
		NETX_COMMANDS_LIST
#undef X
		default:
			// TODO: Read it.
			break;
		}
		return (waitingPackages-1);
	}
	return 0;
}


uint32_t NetX::GetSystemStatus() noexcept
{
	// Reading the system status in the system status block.
	auto AddressSystemStatus = NetXAddress(Nx50AddressMap::SystemStatus)
			.Append(offsetof(NetXSystemStatusBlock,ulSystemStatus));

	return spi.GetIntegerFromAddress<uint32_t>(AddressSystemStatus);
}

uint32_t NetX::GetSystemError() noexcept
{
	// Reading the system status in the system status block.
	auto AddressSystemError = NetXAddress(Nx50AddressMap::SystemStatus)
			.Append(offsetof(NetXSystemStatusBlock,ulSystemError));

	return spi.GetIntegerFromAddress<uint32_t>(AddressSystemError);
}


void NetX::ReadHandshakeCell(NetXChannel channel,NetXHandshakeCell &cell) noexcept
{
	auto AddressHandshakeRegister =	NetXAddress(asDPMAddress(channel) * sizeof(NetXHandshakeCell))
									.Append(Nx50AddressMap::HandshakeRegister);

	spi.ReadAddress(AddressHandshakeRegister, cell);

#ifdef NETX_DEBUG
	switch(channel)
	{
	case NetXChannel::System:
		debugPrintf("[%06lX] Handshake Channel | System Flags : System %02X | Host %02X\n",
				AddressHandshakeRegister.GetAddress(),
				cell.t8Bits.bNetxFlags, cell.t8Bits.bHostFlags );
		break;

	case NetXChannel::Handshake:
		debugPrintf("[%06lX] Handshake Channel | Handshake Flags : System %04X | Host %04X\n",
				AddressHandshakeRegister.GetAddress(),
				cell.t16Bits.usNetxFlags, cell.t16Bits.usHostFlags );
		break;

	case NetXChannel::Communication0:
	case NetXChannel::Communication1:
	case NetXChannel::Communication2:
	case NetXChannel::Communication3:
		debugPrintf("[%06lX] Handshake Channel | Channel %lu Flags : System %04X | Host %04X\n",
				AddressHandshakeRegister.GetAddress(),
				asDPMAddress(channel)-asDPMAddress(NetXChannel::Communication0),
				cell.t16Bits.usNetxFlags, cell.t16Bits.usHostFlags );
		break;
	}
#endif
}

bool NetX::GetAndProcessChannelInformationBlock()
{

	NetXChannelInfoBlock channelInfoBlock;
	if(spi.ReadAddress(Nx50AddressMap::ChannelInformation, channelInfoBlock))
	{
		debugNetXChannelInfoBlock(channelInfoBlock);
		// Check System channel info
		ASSERT_NETX(channelInfoBlock.tSystemChannelInfo.bChannelType, 		NetXDefines::ChannelType::System, "ERROR: System Channel - Type not valid\n" )
		ASSERT_NETX(channelInfoBlock.tSystemChannelInfo.ulSizeOfChannel,	NetXDefines::ChannelDefaultSize::System, "ERROR: System Channel - Size of channel not valid\n" )
		// Check handshake channel info
		ASSERT_NETX(channelInfoBlock.tHandshakeChannelInfo.bChannelType, 	NetXDefines::ChannelType::Handshake, "ERROR: Handshake Channel - Type not valid\n" )
		ASSERT_NETX(channelInfoBlock.tHandshakeChannelInfo.ulSizeOfChannel, NetXDefines::ChannelDefaultSize::Handshake, "ERROR: Handshake Channel - Size of channel not valid\n" )

		// Check handshake channel info
		ASSERT_NETX(channelInfoBlock.atCommunicationChannelInfo[0].bChannelType, 	NetXDefines::ChannelType::Communication, "ERROR: Communication Channel 0 - Type not valid\n" )
		ASSERT_NETX(channelInfoBlock.atCommunicationChannelInfo[0].ulSizeOfChannel, NetXDefines::ChannelDefaultSize::Communication, "ERROR: Communication Channel 0 - Size of channel not valid\n" )
		ASSERT_NETX(channelInfoBlock.atCommunicationChannelInfo[0].bSizePositionHandshakeRegisters, NetXDefines::SizeOrPositionOfHandshakeRegisters::Communication, "ERROR: Communication Channel 0 - Size or Position of Handshake Registers\n" )
		return true;
	}
	return false;
}

bool NetX::GetAndProcessSystemInformationBlock()
{
	NetXSystemInfoBlock systemInfoBlock;
	ASSERT_NETX(spi.ReadAddress(Nx50AddressMap::SystemInformation, systemInfoBlock), true, "ERROR: System Information not available\n")

	debugNetXSystemInfoBlock(systemInfoBlock);
	ulSerialNumber = systemInfoBlock.ulSerialNumber;
	ASSERT_NETX(systemInfoBlock.bDevIdNumber, 0, "ERROR: Device ID not valid\n" );
	ASSERT_NETX(memcmp( (char*)systemInfoBlock.abCookie, DefaultDeviceName, sizeof(systemInfoBlock.abCookie) ), 0, "ERROR: Device Name not valid\n" );

	return true;
}

bool NetX::IsReady() noexcept
{
	return (ReadHandshakeNetXFlags<SystemFlags>(NetXChannel::System) & NSF_READY);
}

// The device needs two dummy reads after detecting it
void NetX::DummyRead() noexcept
{
	uint8_t dummy;
	spi.ReadAddress(Nx50AddressMap::SystemInformation,dummy);
	spi.ReadAddress(Nx50AddressMap::SystemInformation,dummy);
}

void NetX::Init() noexcept
{
	if (netXTask == nullptr)
	{
		netXTask = new Task<NetXTaskStackWords>;
		netXTask->Create(NetXTaskCode, "NETX", nullptr, TaskPriority::NetXPriority);
	}
	if (netXSocketTask == nullptr)
	{
		netXSocketTask = new Task<NetXSocketTaskStackWords>;
	}
}

#endif // NETX_ENABLE
