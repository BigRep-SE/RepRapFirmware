/*
 * NetXStructs.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#include <Hardware/NetX/NetXStructs.h>

#include <Platform/RepRap.h>
#include <Platform/Platform.h>

#if NETX_ENABLE
constexpr bool EnableStructsDebug = true;

uint32_t asNetXCommand(const NetXCommand value)
{
	return static_cast<uint32_t>(value);
}


void debugNetXSystemInfoBlock(NetXSystemInfoBlock& block)
{
#ifdef NETX_DEBUG
	if (EnableStructsDebug)
	{
		debugPrintf("\nNETX_SYSTEM_INFO_BLOCK ------------------------------------------\n");
		debugPrintf("Size: %lu\n", (uint32_t)sizeof(NetXSystemInfoBlock));
		debugPrintf("\t abCookie[] = %c%c%c%c\n", 	(char)block.abCookie[0],(char)block.abCookie[1],(char)block.abCookie[2],(char)block.abCookie[3]);
		debugPrintf("\t ulDpmTotalSize = %lu\n", 	block.ulDpmTotalSize);
		debugPrintf("\t ulDeviceNumber = %lu\n", 	block.ulDeviceNumber);
		debugPrintf("\t ulSerialNumber = %lu\n", 	block.ulSerialNumber);
		debugPrintf("\t ausHwOptions[] = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", block.ausHwOptions[0],block.ausHwOptions[1], block.ausHwOptions[2], block.ausHwOptions[3]);
		debugPrintf("\t usManufacturer = %u\n", 	block.usManufacturer);
		debugPrintf("\t usProductionDate = %u\n", 	block.usProductionDate);
		debugPrintf("\t ulLicenseFlags1 = %lu\n", 	block.ulLicenseFlags1);
		debugPrintf("\t ulLicenseFlags2 = %lu\n", 	block.ulLicenseFlags2);
		debugPrintf("\t usNetxLicenseID = %u\n", 	block.usNetxLicenseID);
		debugPrintf("\t usNetxLicenseFlags = %u\n", block.usNetxLicenseFlags);
		debugPrintf("\t usDeviceClass = %u\n", 		block.usDeviceClass);
		debugPrintf("\t bHwRevision = %u\n", 		block.bHwRevision);
		debugPrintf("\t bHwCompatibility = %u\n", 	block.bHwCompatibility);
		debugPrintf("\t bDevIdNumber = %u\n", 		block.bDevIdNumber);
	}
#endif
}

void debugNetXSystemChannelInfo(NetXSystemChannelInfo& block)
{
#ifdef NETX_DEBUG
	if (EnableStructsDebug)
	{
		debugPrintf("NETX_SYSTEM_CHANNEL_INFO\n");
		debugPrintf("\t bChannelType = %u (==3)\n",	block.bChannelType);
		debugPrintf("\t bSizePositionHandshakeRegisters = %u\n", 	block.bSizePositionHandshakeRegisters);
		debugPrintf("\t bTotalNumberBlocks = %u\n", block.bTotalNumberBlocks);
		debugPrintf("\t ulSizeOfChannel = %lu\n", 	block.ulSizeOfChannel);
		debugPrintf("\t usSizeOfMailbox = %u\n", 	block.usSizeOfMailbox);
		debugPrintf("\t usMailboxStartOffset = %u\n", block.usMailboxStartOffset);
	}
#endif
}

void debugNetXHandshakeChannelInfo(NetXHandshakeChannelInfo& block)
{
#ifdef NETX_DEBUG
	if (EnableStructsDebug)
	{
		debugPrintf("NETX_HANDSHAKE_CHANNEL_INFO\n");
		debugPrintf("\t bChannelType = %u (==4)\n",	block.bChannelType);
		debugPrintf("\t ulSizeOfChannel = %lu\n", 	block.ulSizeOfChannel);
	}
#endif
}

void debugNetXCommunicationChannelInfo(NetXCommunicationChannelInfo& block, uint8_t id)
{
#ifdef NETX_DEBUG
	if (EnableStructsDebug )
	{
		debugPrintf("NETX_COMMUNICATION_CHANNEL_INFO %u\n",id);
		debugPrintf("\t bChannelType = %u (==5)\n",	block.bChannelType);
		if(block.bChannelType != 5)
		{
			debugPrintf("\t Not Initialized\n");
		}
		else
		{
			debugPrintf("\t bChannelID = %u\n",	block.bChannelID);
			debugPrintf("\t bSizePositionHandshakeRegisters = %u\n", 	block.bSizePositionHandshakeRegisters);
			debugPrintf("\t bTotalNumberBlocks = %u\n", block.bTotalNumberBlocks);
			debugPrintf("\t ulSizeOfChannel = %lu\n", 	block.ulSizeOfChannel);
			debugPrintf("\t usCommunicationClass = %u\n", 	block.usCommunicationClass);
			debugPrintf("\t usProtocolClass = %u\n", block.usProtocolClass);
			debugPrintf("\t usProtocolConformanceClass = %u\n", block.usProtocolConformanceClass);
		}
	}
#endif
}

void debugNetXApplicationChannelInfo(NetXApplicationChannelInfo& block, uint8_t id)
{
#ifdef NETX_DEBUG

	debugPrintf("NETX_APPLICATION_CHANNEL_INFO %u\n",id);
	debugPrintf("\t bChannelType = %u (==6)\n",	block.bChannelType);
	if(block.bChannelType != 6)
	{
		debugPrintf("\t Not Initialized\n");
	}
	else
	{
		debugPrintf("\t bChannelID = %u\n",	block.bChannelID);
		debugPrintf("\t bSizePositionHandshakeRegisters = %u\n", 	block.bSizePositionHandshakeRegisters);
		debugPrintf("\t bTotalNumberBlocks = %u\n", block.bTotalNumberBlocks);
		debugPrintf("\t ulSizeOfChannel = %lu\n", 	block.ulSizeOfChannel);
	}
#endif
}

void debugNetXChannelInfoBlock(NetXChannelInfoBlock& block)
{
#ifdef NETX_DEBUG
	debugPrintf("\nNETX_CHANNEL_INFO_BLOCK -----------------------------------------\n");
	debugPrintf("Size: %lu\n", (uint32_t)sizeof(NetXChannelInfoBlock));
	debugNetXSystemChannelInfo(block.tSystemChannelInfo);
	debugNetXHandshakeChannelInfo(block.tHandshakeChannelInfo);
	debugNetXCommunicationChannelInfo(block.atCommunicationChannelInfo[0],0);
	debugNetXCommunicationChannelInfo(block.atCommunicationChannelInfo[1],1);
	debugNetXCommunicationChannelInfo(block.atCommunicationChannelInfo[2],2);
	debugNetXCommunicationChannelInfo(block.atCommunicationChannelInfo[3],3);
	debugNetXApplicationChannelInfo(block.atApplicationChannelInfo[0],0);
	debugNetXApplicationChannelInfo(block.atApplicationChannelInfo[1],1);
#endif
}

void debugNetXPacketEtherCATSMALStatusChangedIndicator(const NetXPacket::EtherCATSMALStatusChangedIndicator &pkt)
{
#ifdef NETX_DEBUG
	if(pkt.tHeader.ulCmd != asNetXCommand(NetXCommand::EtherCATSMALStatusChangedIndicator))
	{
		debugPrintf("Error: EtherCATSMALStatusChangedIndicator - Command not valid \n");
	}
	else
	{
		debugPrintf("EtherCATSMALStatusChangedIndicator --------------------------------\n");
		debugPrintf("\t State: ");
		switch(pkt.tData.tAlStatus.uState)
		{
		case(1):
			debugPrintf("Init\n");
			break;
		case(2):
			debugPrintf("Pre-Operational\n");
			break;
		case(3):
			debugPrintf("Bootstrap\n");
			break;
		case(4):
			debugPrintf("Safe-Operational\n");
			break;
		case(8):
			debugPrintf("Operational\n");
			break;
		default:
			debugPrintf("NOT VALID (ERROR)\n");
			break;
		}
		if(pkt.tData.tAlStatus.fChange == 0)
		{
			debugPrintf("\t No Change From Slave\n");
		}
		else
		{
			debugPrintf("\t The Slave changed the state\n");
		}
		debugPrintf("\t Status Code: %04X\n", pkt.tData.usAlStatusCode);
		debugPrintf("\t Error Led: %04X\n", pkt.tData.usErrorLed);
	}
#endif
}

void debugNetXPacketOSGetBlockInformationConfirmation(const NetXPacket::OSGetBlockInformationConfirmation &pkt)
{
#ifdef NETX_DEBUG
	if(pkt.tHeader.ulCmd != asNetXCommand(NetXCommand::OSGetBlockInformationConfirmation))
	{
		debugPrintf("Error: PacketBlockInformationConfirmation - Command not valid \n");
	}
	else
	{
		debugPrintf("PacketBlockInformationConfirmation --------------------------------\n");
		debugPrintf("\t ulAreaIndex =  %lu\n", pkt.tData.ulAreaIndex	);
		debugPrintf("\t ulType = %lu: ", pkt.tData.ulType	);
		switch(pkt.tData.ulType)
		{
		case 0:
			debugPrintf("UNDEFINED\n");
			break;
		case 1:
			debugPrintf("UNKNOWN\n");
			break;
		case 2:
			debugPrintf("PROCESS DATA IMAGE\n");
			break;
		case 3:
			debugPrintf("HIGH PRIORITY DATA IMAGE\n");
			break;
		case 4:
			debugPrintf("MAILBOX\n");
			break;
		case 5:
			debugPrintf("CONTROL\n");
			break;
		case 6:
			debugPrintf("COMMON STATUS\n");
			break;
		case 7:
			debugPrintf("EXTENDED STATUS\n");
			break;
		case 8:
			debugPrintf("USER\n");
			break;
		default:
			debugPrintf("RESERVED\n");
			break;
		}
		debugPrintf("\t ulOffset =  %lu\n", pkt.tData.ulOffset	);
		debugPrintf("\t ulSize =  %lu\n", pkt.tData.ulSize	);
		debugPrintf("\t usFlags =  %u: ", pkt.tData.usFlags	);
		switch(pkt.tData.usFlags & 0x0F)
		{
		case 0:
			debugPrintf("UNDEFINED | ");
			break;
		case 1:
			debugPrintf("IN (NetX to Host System) | ");
			break;
		case 2:
			debugPrintf("OUT (Host System to netX) | ");
			break;
		case 3:
			debugPrintf("IN-OUT (Bi-Directional) | ");
			break;
		default:
			debugPrintf("ERROR | ");
			break;
		}
		switch( (pkt.tData.usFlags & 0xF0) >> 4)
		{
		case 0:
			debugPrintf("UNDEFINED\n");
			break;
		case 1:
			debugPrintf("DPM\n");
			break;
		case 2:
			debugPrintf("DMA\n");
			break;
		default:
			debugPrintf("ERROR\n");
			break;
		}
		debugPrintf("\t usHandshakeMode =  %u: ", pkt.tData.usHandshakeMode	);
		switch( pkt.tData.usHandshakeMode)
		{
		case 0:
			debugPrintf("UNKNOWN\n");
			break;
		case 3:
			debugPrintf("UNCONTROLLED\n");
			break;
		case 4:
			debugPrintf("BUFFERED, HOST CONTROLLED\n");
			break;
		default:
			debugPrintf("ERROR\n");
			break;
		}
		debugPrintf("\t usHandshakeBit =  %u\n", pkt.tData.usHandshakeBit	);
	}
#else
	delay(10);
#endif
}

void debugNetXPacketOSFirmawareIdentifyConfirmation(const NetXPacket::OSFirmawareIdentifyConfirmation &pkt)
{
#ifdef NETX_DEBUG
	if ( EnableStructsDebug )
	{
		if(pkt.tHeader.ulCmd != 0x00001EB7)
		{
			debugPrintf("Error: NetXPacketFirmwareIdentifyConfirmation - Command not valid \n");
		}
		else
		{
			debugPrintf("NetXPacketFirmwareIdentifyConfirmation ----------------------------\n");
			debugPrintf("\t tFwName.abName = %s\n", pkt.tData.tFwName.abName);
		}
	}
#endif
}

void debugNetXPacketOSLinkStatusChangeIndicator(const NetXPacket::OSLinkStatusChangeIndicator &pkt)
{
#ifdef NETX_DEBUG
	if ( EnableStructsDebug )
	{
		if(pkt.tHeader.ulCmd != asNetXCommand(NetXCommand::OSLinkStatusChangeIndicator))
		{
			debugPrintf("Error: OSLinkStatusChangeIndicator - Command not valid \n");
		}
		else
		{
			debugPrintf("OSLinkStatusChangeIndicator ---------------------------------------\n");
			for(int port = 0; port < 2; ++port)
			{
				debugPrintf("\t Port %lu:\n", pkt.tData.atLinkData[port].ulPort);

				if(pkt.tData.atLinkData[port].ulSpeed == 0)
				{
					debugPrintf("\t\t No Link\n");
				}
				else
				{
					debugPrintf("\t\t ulSpeed = %lu MBit\n", pkt.tData.atLinkData[port].ulSpeed);
				}

				if(pkt.tData.atLinkData[port].fIsLinkUp == 0)
				{
					debugPrintf("\t\t No Link Available\n");
				}
				else
				{
					debugPrintf("\t\t Link Available\n");
				}

				if(pkt.tData.atLinkData[port].fIsFullDuplex == 0)
				{
					debugPrintf("\t\t Not Full-Duplex\n");
				}
				else
				{
					debugPrintf("\t\t Full-Duplex\n");
				}
			}

		}
	}
#endif
}


void debugNetXCommunicationCOSOfCommonStatusBlockOfCommunicationChannel( uint32_t val )
{
#ifdef NETX_DEBUG
	if ( EnableStructsDebug )
	{
		debugPrintf("ulCommunicationCOS =  %02lX: ", val );

		if(val == RCX_COMM_COS_UNDEFINED)
		{
			debugPrintf("UNDEFINED\n");
		}
		else
		{
			if(val & RCX_COMM_COS_READY)
			{
				debugPrintf("READY | ");
			}
			if(val & RCX_COMM_COS_RUN)
			{
				debugPrintf("RUN | ");
			}
			if(val & RCX_COMM_COS_BUS_ON)
			{
				debugPrintf("BUS_ON | ");
			}
			if(val & RCX_COMM_COS_CONFIG_LOCKED)
			{
				debugPrintf("CONFIG_LOCKED | ");
			}
			if(val & RCX_COMM_COS_CONFIG_NEW)
			{
				debugPrintf("CONFIG_NEW | ");
			}
			if(val & RCX_COMM_COS_RESTART_REQUIRED)
			{
				debugPrintf("RESTART_REQUIRED | ");
			}
			if(val & RCX_COMM_COS_RESTART_REQUIRED_ENABLE)
			{
				debugPrintf("RESTART_REQUIRED_ENABLE | ");
			}
			if(val & RCX_COMM_COS_DMA)
			{
				debugPrintf("DMA");
			}

			debugPrintf("\n");
		}
	}
#endif
}

void debugNetXApplicationCOSOfCommonControlBlockOfCommunicationChannel( uint32_t val )
{
#ifdef NETX_DEBUG
	if ( EnableStructsDebug )
	{
		debugPrintf("ulApplicationCOS =  %03lX: ", val );

		if(val == 0)
		{
			debugPrintf("UNDEFINED\n");
		}
		else
		{
			if(val & RCX_APP_COS_APPLICATION_READY)
			{
				debugPrintf("READY | ");
			}
			if(val & RCX_APP_COS_BUS_ON)
			{
				debugPrintf("BUS_ON | ");
			}
			if(val & RCX_APP_COS_BUS_ON_ENABLE)
			{
				debugPrintf("BUS_ON_ENABLE | ");
			}
			if(val & RCX_APP_COS_INITIALIZATION)
			{
				debugPrintf("INITIALIZATION | ");
			}
			if(val & RCX_APP_COS_INITIALIZATION_ENABLE)
			{
				debugPrintf("INITIALIZATION_ENABLE | ");
			}
			if(val & RCX_APP_COS_LOCK_CONFIGURATION)
			{
				debugPrintf("CONFIGURATION | ");
			}
			if(val & RCX_APP_COS_LOCK_CONFIGURATION_ENABLE)
			{
				debugPrintf("CONFIGURATION_ENABLE | ");
			}
			if(val & RCX_APP_COS_DMA)
			{
				debugPrintf("DMA | ");
			}
			if(val & RCX_APP_COS_DMA_ENABLE)
			{
				debugPrintf("DMA_ENABLE");
			}
			debugPrintf("\n");
		}
	}
#endif
}

void debugNetXPacketHeader( NetXPacket::Header &tHeader )
{
#ifdef NETX_DEBUG
	if ( EnableStructsDebug )
	{
		debugPrintf("NetXPacketHeader:\n");

		debugPrintf("\tCommand [0x%06lX]: ", tHeader.ulCmd );
		switch(static_cast<NetXCommand>(tHeader.ulCmd))
		{
#define X(name, number, func, str) case(NetXCommand::name): debugPrintf(str); break;
		NETX_COMMANDS_LIST
#undef X
		default:
			debugPrintf("Not found");
			break;
		}
		debugPrintf("\n");
		debugPrintf("\tSource:            %08lX\n", tHeader.ulSrc );
		debugPrintf("\tDestination:       %08lX\n", tHeader.ulDest );
		debugPrintf("\tData Size:         %lu bytes\n", tHeader.ulLen );
	}
#endif
}
#endif
