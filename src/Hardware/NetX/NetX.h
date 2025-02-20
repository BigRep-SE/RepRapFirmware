/*
 * NetX.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#ifndef SRC_HARDWARE_NETX_NETX_H_
#define SRC_HARDWARE_NETX_NETX_H_

#include <RepRapFirmware.h>
#include <RTOSIface/RTOSIface.h>
#include "Hardware/Spi/SpiMode.h"

#if NETX_ENABLE

#include <Platform/RepRap.h>
#include <Platform/Platform.h>
#include <Hardware/NetX/NetXStructs.h>
#include <Hardware/NetX/NetXSpi.h>
#include <Hardware/NetX/ProtocolOverEtherCAT.h>

#ifdef NETX_DEBUG
#ifndef ASSERT_NETX
#define ASSERT_NETX(value,spectedValue,msg) if((value) != (spectedValue)){ 		\
												if (reprap.Debug(moduleNetX)) 	\
												{	debugPrintf(msg); } 		\
												return false; 					\
											}
#endif

#ifndef WARNING_NETX
#define WARNING_NETX(value,spectedValue,msg) if((value) != (spectedValue)){ 		\
												if (reprap.Debug(moduleNetX)) 	\
												{	debugPrintf(msg); } 		\
											}
#endif
#else
#ifndef ASSERT_NETX
#define ASSERT_NETX(value,spectedValue,msg)
#endif

#ifndef WARNING_NETX
#define WARNING_NETX(value,spectedValue,msg)
#endif

#endif

class NetX
{
public:

	void Init() noexcept;

	static NetX& GetInstance() noexcept
	{
		static NetX instance;
		return instance;
	}

	NetX(NetX const&)     = delete;
	void operator=(NetX const&)  = delete;

	bool EnableChannel() noexcept;
	bool IsReady() noexcept;
	void Reset() noexcept;

	void Spin() noexcept;

private:
	static constexpr uint32_t 	DefaultIsConnectedTimeout_ms 	= 3000; // [ms] Max. Time waiting for connection
	static constexpr uint8_t 	DefaultDeviceMacAddr[6] 		= { 0x00, 0x02, 0xA2, 0x2F, 0x90, 0x58 };
	static constexpr char 		DefaultDeviceName[] 			= "netX";


	NetX() noexcept {};

	using  DPMAddress 		= NetXSpi::DPMAddress;
	using  NonSystemFlags 	= uint16_t;
	using  SystemFlags 		= uint8_t;

	ProtocolOverEtherCAT& applicationProtocol = ProtocolOverEtherCAT::GetInstance();

	NetXSpi& spi = NetXSpi::GetInstance( );

	uint32_t ulSerialNumber = 0;

	enum class Nx50AddressMap: DPMAddress
	{
		SystemInformation =		0x00000000,		// System Information [48 bytes]
		ChannelInformation =	0x00000030,		// Channel Information [128 bytes]
		SystemHandshakeCell =	0x000000B0,		// NOT SUPPORTED
		SystemControl =			0x000000B8,		// System control and command [8 bytes]
		SystemStatus =			0x000000C0,		// System status Information [64 bytes]
		SystemSendMailbox =		0x00000100,		// System Send Mailbox [128 bytes]
		SystemReceiveMailbox =	0x00000180,		// System Receive Mailbox [128 bytes]
		HandshakeRegister =		0x00000200,		// Handshake Registers Area [64 bytes]
		CommunicationChannel0 =	0x00000300,		// Communication Channel 0 [15616 bytes]
		CommunicationChannel1 =	0x00004000,		// Communication Channel 1 [15616 bytes]
		CommunicationChannel2 =	0x00007D00,		// Communication Channel 2 [15616 bytes]
		CommunicationChannel3 =	0x0000BA00		// Communication Channel 3 [15616 bytes]
	};

	enum class NetXChannel : uint8_t { System = 0, Handshake = 1, Communication0 = 2, Communication1 = 3, Communication2 = 4, Communication3 = 5};

	bool isInit = false;

	bool IsValidVersion() noexcept;

	void WaitForApplicationReady() noexcept;
	bool WaitForChannelConnected(uint32_t timeout) noexcept;
	bool CheckProtocolStackStartedProperly(Nx50AddressMap communicationChannelOnly) noexcept;
	void RequestFirmware() noexcept;
	void RequestApplication() noexcept;
	void DiscardInputDataFrame() noexcept;
	void SetDefaultMacAddress(const uint8_t pMacAddr[6] = DefaultDeviceMacAddr) noexcept;
	void SetEtherCATConfiguration(uint32_t serialNumber) noexcept;
	void ChannelInitializationRequest() noexcept;
	void StartStopCommunicationRequest(bool start) noexcept;
	void RequestChannelLayout(uint32_t blockIndex) noexcept;
	bool GetAndProcessSystemInformationBlock();
	bool GetAndProcessChannelInformationBlock();
	uint32_t GetSystemStatus() noexcept;
	uint32_t GetSystemError() noexcept;

	bool ReadDataInput(EtherCATApplicationMOSIData &data) noexcept;
	bool WriteDataOutput(EtherCATApplicationMISOData &data) noexcept;

	uint32_t GetApplicationCOS(const NetXChannel channel) noexcept;
	uint32_t GetCommunicationCOS(const NetXChannel channel) noexcept;
	uint32_t GetStatusCOS(const NetXChannel channel) noexcept;
	uint32_t GetErrorCOS(const NetXChannel channel) noexcept;

	void HandlerNetXPacketEtherCATSMALStatusChangedIndicator(const NetXPacket::EtherCATSMALStatusChangedIndicator &pkt) noexcept;
	void HandlerNetXPacketOSRegisterApplicationConfirmation(const NetXPacket::OSRegisterApplicationConfirmation &pkt) noexcept;
	void HandlerNetXPacketOSLinkStatusChangeIndicator(const  NetXPacket::OSLinkStatusChangeIndicator &pkt) noexcept;
	void HandlerNetXPacketOSChannelInitializationConfirmation (const NetXPacket::OSChannelInitializationConfirmation &pkt) noexcept;
	void HandlerNetXPacketOSStartStopCommunicationConfirmation(const NetXPacket::OSStartStopCommunicationConfirmation &pkt) noexcept;
	void HandlerNetXPacketOSSetAddressConfirmation(const NetXPacket::OSSetAddressConfirmation &pkt) noexcept;
	void HandlerNetXPacketEtherCATSetConfigurationConfirmation(const NetXPacket::EtherCATSetConfigurationConfirmation &pkt) noexcept;

	void ReadHandshakeCell(NetXChannel channel,NetXHandshakeCell &cell) noexcept;

	void DummyRead() noexcept;

	inline uint8_t SetReadBit(uint8_t val){ return val | 0x80; }

	template <typename T>
	auto asInteger(T const value) -> typename std::underlying_type<T>::type
	{
	    return static_cast<typename std::underlying_type<T>::type>(value);
	};

	template <typename T>
	DPMAddress asDPMAddress(T const value)
	{
		return static_cast<DPMAddress>(value);
	};

	template<typename TSend>
	void SendNonCyclicPacket(NetXChannel channel, TSend &tSend, bool enableDebug = false) noexcept
	{
		if(channel == NetXChannel::System)
		{
			const DPMAddress addressSendMbx = asDPMAddress(Nx50AddressMap::SystemSendMailbox) + offsetof(NetXSystemMailBox::SendMailbox,abSendMbx);

			// We are writing in the System Send Mailbox
			WriteAddress( addressSendMbx, tSend, enableDebug);

			SystemFlags wantedNetXFlags = ToggleHandshakeHostFlags<SystemFlags>(NetXChannel::System, HSF_SEND_MBX_CMD );

			// Wait for ACK
			// debugPrintf("Waiting for NSF_SEND_MBX_ACK: ");
			WaitBitInHandshakeNetXFlagToBe<SystemFlags>(channel,NSF_SEND_MBX_ACK, wantedNetXFlags & NSF_SEND_MBX_ACK);
			// debugPrintf(" DONE\n");
		}
		else
		{
			// Calculate the Address to read and write
			const DPMAddress offsetCommChannel = asDPMAddress(Nx50AddressMap::CommunicationChannel0)
					+ sizeof(NetX16KCommunicationChannel) * (asDPMAddress(channel) - asDPMAddress(NetXChannel::Communication0));

			const DPMAddress addressSendMbx = offsetCommChannel + offsetof(NetX16KCommunicationChannel,tSendMbx) + offsetof(NetXSendMailBoxBlock,abSendMbx);

			// We are writing in the Send Mailbox
			WriteAddress(addressSendMbx, tSend, enableDebug);

			// Notify there is a new command
			NonSystemFlags wantedNetXFlags = ToggleHandshakeHostFlags<NonSystemFlags>(channel, HCF_SEND_MBX_CMD );

			// debugPrintf("Waiting for NCF_SEND_MBX_ACK: ");
			WaitBitInHandshakeNetXFlagToBe<NonSystemFlags>(channel,NCF_SEND_MBX_ACK, wantedNetXFlags & NCF_SEND_MBX_ACK);
			// debugPrintf(" DONE\n");
		}
	};

	template<typename TReceive>
	void ReceiveNonCyclicPackage(NetXChannel channel, TReceive &tReceive, bool enableDebug = false) noexcept
	{
		if(channel == NetXChannel::System)
		{
			auto AddressReceiveMbx = NetXAddress(Nx50AddressMap::SystemReceiveMailbox)
					.Append(offsetof(NetXSystemMailBox::ReceiveMailbox,abRecvMbx));

			SystemFlags wantedNetXFlags = ReadHandshakeHostFlags<SystemFlags>(NetXChannel::System);

			// Wait for a new received command
			// debugPrintf("Waiting for NSF_RECV_MBX_CMD to Change: ");
			wantedNetXFlags = WaitBitInHandshakeNetXFlagToBe<SystemFlags>(channel,NSF_RECV_MBX_CMD, wantedNetXFlags ^ NSF_RECV_MBX_CMD);
			// debugPrintf(" DONE\n");
			// Read the new command
			spi.ReadAddress(AddressReceiveMbx,	tReceive, enableDebug);

			// Let's make them match
			WriteHandshakeHostFlags<SystemFlags>(NetXChannel::System, wantedNetXFlags & 0xFC);
		}
		else
		{
			// Calculate the Address to read and write
			auto AddressReceiveMbx = NetXAddress(Nx50AddressMap::CommunicationChannel0)
					.Append( sizeof(NetX16KCommunicationChannel) * (asDPMAddress(channel) - asDPMAddress(NetXChannel::Communication0)) )
					.Append( offsetof(NetX16KCommunicationChannel,tRecvMbx) )
					.Append( offsetof(NetXReceiveMailBoxBlock,abRecvMbx) );


			// Notify there is a new command
			NonSystemFlags wantedNetXFlags = ReadHandshakeHostFlags<NonSystemFlags>(channel);

			// debugPrintf("Waiting for NSF_RECV_MBX_CMD to Change: ");
			wantedNetXFlags = WaitBitInHandshakeNetXFlagToBe<NonSystemFlags>(channel,NCF_RECV_MBX_CMD, wantedNetXFlags ^ NCF_RECV_MBX_CMD, !isInit);
			// debugPrintf(" DONE\n");

			spi.ReadAddress(AddressReceiveMbx, tReceive, enableDebug);

			// Change the HSF_RECV_MBX_ACK bit.
			WriteHandshakeHostFlags<NonSystemFlags>(channel, wantedNetXFlags & 0xFFFC);
		}
	};

	uint16_t GetAndProcessNonCyclicConfirmation(NetXChannel channel) noexcept;

	template<typename TSend,typename TReceive>
	void SendNonCyclicCommandWithResponse(NetXChannel channel, TSend &tSend, TReceive &tReceive, bool enableDebug = false) noexcept
	{
		SendNonCyclicPacket(channel, tSend, enableDebug );
		ReceiveNonCyclicPackage(channel, tReceive, enableDebug );
	};


	template<typename T>
	bool WriteAddressDPM(DPMAddress addressDPM, T &varToWrite) noexcept
	{
		return WriteAddress(addressDPM, varToWrite);
	};

	template<typename T, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
	bool WriteIntegerInAddressDPM(DPMAddress addressDPM, T varToWrite) noexcept
	{
		const T var = varToWrite;
		return WriteAddress(addressDPM, var);
	};

	template<typename T, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
	bool ToggleBitsInAddressDPM(DPMAddress addressDPM, T bitsToToggle) noexcept
	{
		T var;
		ReadAddress(addressDPM,var);
		var ^= bitsToToggle;
		return WriteAddress(addressDPM, var);
	};

	template<typename T>
	void WriteHandshakeNetXFlags(NetXChannel channel, T val)
	{
		if(NetXChannel::System == channel)
		{
			WARNING_NETX(sizeof(T), sizeof(SystemFlags),"WARNING: System flags are uint8_t\n");
			const DPMAddress addressSystemNetxFlags = asDPMAddress(Nx50AddressMap::HandshakeRegister) + offsetof(NetXHandshakeCellUint8_t,bNetxFlags);
			WriteIntegerInAddressDPM<SystemFlags>(addressSystemNetxFlags, (SystemFlags)val);
		}
		else
		{
			WARNING_NETX(sizeof(T), sizeof(uint16_t),"WARNING: The requested Flags should be uint16_t\n");
			const DPMAddress addressNetxFlags = asDPMAddress(Nx50AddressMap::HandshakeRegister)
					+ sizeof(NetXHandshakeCell) * asDPMAddress(channel) +  offsetof(NetXHandshakeCellUint16_t,usNetxFlags);
			WriteIntegerInAddressDPM<NonSystemFlags>(addressNetxFlags, (NonSystemFlags)val);
		}
	};

	template<typename T>
	T WaitBitInHandshakeHostFlagToBe(NetXChannel channel, T bitMask, T bitValueWanted, bool with1msDelay = false )
	{
		T ret;
		if(NetXChannel::System == channel)
		{
			WARNING_NETX(sizeof(T), sizeof(SystemFlags),"WARNING: System Host flags are uint8_t\n");
			ret = ReadHandshakeHostFlags<SystemFlags>(NetXChannel::System);
			while((ret & bitMask) != (bitValueWanted & bitMask) )
			{
				if(with1msDelay)
				{
					delay(1);
				}
				ret = ReadHandshakeHostFlags<SystemFlags>(NetXChannel::System);
			};
		}
		else
		{
			WARNING_NETX(sizeof(T), sizeof(NonSystemFlags),"WARNING: The requested Host Flags should be uint16_t\n");
			ret = ReadHandshakeHostFlags<NonSystemFlags>(channel);
			while((ret & bitMask) != (bitValueWanted & bitMask) )
			{
				if(with1msDelay)
				{
					delay(1);
				}
				ret = ReadHandshakeHostFlags<NonSystemFlags>(channel);
			};
		}
		return ret;
	};

	template<typename T>
	T WaitBitInHandshakeNetXFlagToBe(NetXChannel channel, T bitMask, T bitValueWanted, bool with1msDelay = false )
	{
		T ret;
		if(NetXChannel::System == channel)
		{
			WARNING_NETX(sizeof(T), sizeof(SystemFlags),"WARNING: System NetX flags are uint8_t\n");
			ret = (T) ReadHandshakeNetXFlags<SystemFlags>(NetXChannel::System);
			while((ret & bitMask) != (bitValueWanted & bitMask) )
			{
				if(with1msDelay)
				{
					delay(1);
				}
				ret = (T) ReadHandshakeNetXFlags<SystemFlags>(NetXChannel::System);
			};
		}
		else
		{
			WARNING_NETX(sizeof(T), sizeof(NonSystemFlags),"WARNING: The requested NetX Flags should be uint16_t\n");
			ret = (T)ReadHandshakeNetXFlags<NonSystemFlags>(channel);
			while((ret & bitMask) != (bitValueWanted & bitMask) )
			{
				if(with1msDelay)
				{
					delay(1);
				}
				ret = (T)ReadHandshakeNetXFlags<NonSystemFlags>(channel);
			};
		}
		return ret;
	};

	template<typename T>
	T ToggleHandshakeHostFlags(NetXChannel channel, T flag)
	{
		T wantedValue = ReadHandshakeHostFlags<T>(channel) ^ flag;
		WriteHandshakeHostFlags<T>(channel, wantedValue);
		return wantedValue;
	};

	template<typename T>
	T ToggleHandshakeNetXFlags(NetXChannel channel, T flag)
	{
		T wantedValue = ReadHandshakeNetXFlags<T>(channel) ^ flag;
		WriteHandshakeNetXFlags<T>(channel, wantedValue);
		return wantedValue;
	};

	template<typename T>
	void WriteHandshakeHostFlags(NetXChannel channel, T val)
	{
		if(NetXChannel::System == channel)
		{
			WARNING_NETX(sizeof(T), sizeof(SystemFlags),"WARNING: System flags are uint8_t\n");
			const DPMAddress addressSystemHostFlags = asDPMAddress(Nx50AddressMap::HandshakeRegister) + offsetof(NetXHandshakeCellUint8_t,bHostFlags);
			WriteIntegerInAddressDPM<SystemFlags>(addressSystemHostFlags, (SystemFlags)val);
		}
		else
		{
			WARNING_NETX(sizeof(T), sizeof(uint16_t),"WARNING: The requested Flags should be uint16_t\n");
			const DPMAddress addressHostFlags = asDPMAddress(Nx50AddressMap::HandshakeRegister)
					+ sizeof(NetXHandshakeCell) * asDPMAddress(channel) +  offsetof(NetXHandshakeCellUint16_t,usHostFlags);
			WriteIntegerInAddressDPM<NonSystemFlags>(addressHostFlags, (NonSystemFlags)val);
		}
	}

	template<typename T>
	T ReadHandshakeHostFlags(NetXChannel channel, bool enableDebug = false)
	{
		if(NetXChannel::System == channel)
		{
			WARNING_NETX(sizeof(T), sizeof(SystemFlags),"WARNING: System flags are uint8_t\n");
			auto AddressSystemHostFlags = NetXAddress(Nx50AddressMap::HandshakeRegister)
					.Append(offsetof(NetXHandshakeCellUint8_t,bHostFlags));
			return (T)spi.GetIntegerFromAddress<SystemFlags>(AddressSystemHostFlags, enableDebug);
		}
		else
		{
			WARNING_NETX(sizeof(T), sizeof(NonSystemFlags),"WARNING: The requested Flags should be uint16_t\n");
			auto AddressHostFlags = NetXAddress(Nx50AddressMap::HandshakeRegister)
					.Append(sizeof(NetXHandshakeCell) * asDPMAddress(channel))
					.Append(offsetof(NetXHandshakeCellUint16_t,usHostFlags));
			return (T)spi.GetIntegerFromAddress<NonSystemFlags>(AddressHostFlags, enableDebug);
		}
	}

	template<typename T>
	T ReadHandshakeNetXFlags(NetXChannel channel, bool enableDebug = false)
	{
		if(NetXChannel::System == channel)
		{
			WARNING_NETX(sizeof(T), sizeof(SystemFlags),"WARNING: System flags are uint8_t\n");
			const DPMAddress addressSystemNetxFlags = asDPMAddress(Nx50AddressMap::HandshakeRegister) + offsetof(NetXHandshakeCellUint8_t,bNetxFlags);
			return (T)spi.GetIntegerFromAddress<SystemFlags>(addressSystemNetxFlags, enableDebug);
		}
		else
		{
			WARNING_NETX(sizeof(T), sizeof(NonSystemFlags),"WARNING: The requested Flags should be uint16_t\n");
			const DPMAddress addressNetxFlags = asDPMAddress(Nx50AddressMap::HandshakeRegister)
					+ sizeof(NetXHandshakeCell) * asDPMAddress(channel) + offsetof(NetXHandshakeCellUint16_t,usNetxFlags);
			return (T)spi.GetIntegerFromAddress<NonSystemFlags>(addressNetxFlags, enableDebug);
		}
	}

	template<typename T>
	bool SendPacketWithHeaderNonBlocking(TaskBase *task,NetXAddress &addressToRead, T& data, bool enableDebug = false) noexcept
	{
		return spi.SendPacketWithHeaderNonBlocking(task,addressToRead, data,enableDebug);
	}

	template<typename T>
	bool ReadAddressNonBlockingMode(TaskBase *task,const DPMAddress addressToRead, T& data, bool enableDebug = false) noexcept
	{
		auto addressForHeader = NetXAddress(addressToRead);
		return ReadAddressNonBlockingMode(task, addressForHeader,data, enableDebug);
	};

	template<typename T>
	bool ReadAddressNonBlockingMode(TaskBase *task,NetXAddress &addressToRead, T& data, bool enableDebug = false) noexcept
	{
		return spi.ReceivePacketWithHeaderNonBlocking(task, addressToRead, data, enableDebug);
	};

	void ProcessApplicationMessages() noexcept;

	template<typename T>
	bool WriteAddress(const DPMAddress addressToWrite, const T &data, bool enableDebug = false) noexcept
	{
		const uint8_t abSend[3] = {
				(uint8_t)(addressToWrite >> 16), 	// Byte[0] Address 23 to 16 bits
				(uint8_t)(addressToWrite >> 8),				// Byte[1] Address 15 to 8 bits
				(uint8_t)(addressToWrite >> 0)				// Byte[2] Address 7 to 0 bits
			};

#ifdef NETX_DEBUG
		if (enableDebug)
		{
			debugPrintf("Writing [%06lX]: ", addressToWrite );
			uint8_t *pData = (uint8_t *) &data;
			for(size_t i = 0; i < sizeof(T); ++i )
			{
				if((i % 32) == 0 && i != 0)
				{
					debugPrintf("\n                  " );
				}
				debugPrintf("%02X ", pData[i] );
			}
			debugPrintf("\n" );
		}
#endif

		return spi.SendPacketWithHeader(abSend, sizeof(abSend), (uint8_t *)&data, sizeof(T) );
	}

};
#endif // NETX_ENABLE

#endif /* SRC_HARDWARE_NETX_NETX_H_ */
