#ifndef PINS_XBOARD_V3_H__
#define PINS_XBOARD_V3_H__

#include <PinDescription.h>

#ifdef SUPPORT_EMULATOR
#define BOARD_SHORT_NAME		"EMU_CB-MA"
#define BOARD_NAME				"BigRep - CB-MA Emulator - KreuzBoard"
#define DEFAULT_BOARD_TYPE		BoardType::Auto
#define FIRMWARE_NAME			"KreuzRapFirmware for CB-MA Emulator"
#else
#define BOARD_SHORT_NAME		"CB-MA-03"
#define BOARD_NAME				"BigRep - CB-MA-03 - KreuzBoard"
#define DEFAULT_BOARD_TYPE		BoardType::Auto
#define FIRMWARE_NAME			"KreuzRapFirmware for CB-MA-03"
#endif

#define IAP_FIRMWARE_FILE		"Duet3Firmware_" BOARD_SHORT_NAME ".bin"

#define IAP_UPDATE_FILE			"Duet3_SDiap32_" BOARD_SHORT_NAME ".bin"
#define IAP_UPDATE_FILE_SBC		"Duet3_SBCiap32_" BOARD_SHORT_NAME ".bin"
#define IAP_CAN_LOADER_FILE		"Duet3_CANiap32_" BOARD_SHORT_NAME ".bin"
constexpr uint32_t IAP_IMAGE_START = 0x20458000;		// last 32kb of RAM

#define WIFI_FIRMWARE_FILE		"DuetWiFiModule_32S3.bin"

// Features definition
// Networking support
#define HAS_LWIP_NETWORKING		1
#define HAS_WIFI_NETWORKING		0

// Storage support
#define HAS_SBC_INTERFACE		0
#define HAS_MASS_STORAGE		1
#define HAS_HIGH_SPEED_SD		1

#define HAS_CPU_TEMP_SENSOR		1

#define SUPPORT_TMC51xx			1
#define TMC51xx_USES_USART		1

#define HAS_VOLTAGE_MONITOR		1
#define ENFORCE_MAX_VIN			0
#define HAS_12V_MONITOR			0
#define ENFORCE_MIN_V12			0
#define HAS_VREF_MONITOR		1

#ifdef SUPPORT_EMULATOR
#define SUPPORT_CAN_EXPANSION	0
#define DUAL_CAN				0
#else
#define SUPPORT_CAN_EXPANSION	1
#define DUAL_CAN_FD				1					// support the second CAN interface as simple CAN (not FD)
#endif
#define SUPPORT_LED_STRIPS		1
#define SUPPORT_DMA_DOTSTAR		0
#define SUPPORT_DMA_NEOPIXEL	0
#define SUPPORT_LASER			1					// support laser cutters and engravers using G1 S parameter
#define SUPPORT_IOBITS			1					// set to support P parameter in G0/G1 commands
#define SUPPORT_DHT_SENSOR		1					// set nonzero to support DHT temperature/humidity sensors
#define SUPPORT_BME280			1
#define SUPPORT_ACCELEROMETERS	1
#define SUPPORT_WORKPLACE_COORDINATES	1			// set nonzero to support G10 L2 and G53..59
#define SUPPORT_OBJECT_MODEL	1
#define SUPPORT_FTP				1
#define SUPPORT_TELNET			1
#define SUPPORT_MQTT			0
#if !defined(SUPPORT_EMULATOR)
#define SUPPORT_MULTICAST_DISCOVERY	1
#endif
#define SUPPORT_ASYNC_MOVES		1
#define SUPPORT_PROBE_POINTS_FILE	1
#define SUPPORT_BRAKE_PWM		1
#define SUPPORT_KEEPOUT_ZONES	1

#define SUPPORT_MACHINE_VERSION 1

#define SUPPORT_ANALOG_THRESHOLD	1

#ifdef SUPPORT_EMULATOR
#define SUPPORT_EMUPOWERMETER	1					// Enables the Emulated Power Meter
#else
#define SUPPORT_POWERMETER		1					// Enables the Power Meter
#endif

#define NETX_ENABLE				0

#define USE_MPU					1					// Needed if USE_CACHE is set, so that we can have non-cacheable memory regions
#define USE_CACHE				1

#define SUPPORT_3WIRES_RTD		1
#define SUPPORT_INDIVIDUAL_ENDSTOPS		1			// The object model will report the endstops individually.

#define CUSTOMIZED_RESURRECT	1

#define SUPPORT_MODBUS_RTU		0
#define SUPPORT_PHASE_STEPPING	0

// The physical capabilities of the machine

#include <Duet3Common.h>							// this file is in the CANlib project because both main and expansion boards need it

#ifdef SUPPORT_EMULATOR
constexpr size_t NumDirectDrivers = 16;				// The maximum number of drives supported by the electronics inc. direct expansion
constexpr size_t MaxSmartDrivers = 16;				// The maximum number of direct smart drivers
#else
constexpr size_t NumDirectDrivers = 1;				// The maximum number of drives supported by the electronics inc. direct expansion
constexpr size_t MaxSmartDrivers = 1;				// The maximum number of direct smart drivers
#endif
constexpr size_t MaxCanDrivers = 30;
constexpr size_t MaxCanBoards = 20;

constexpr float MaxTmc5160Current = 6300.0;			// The maximum current we allow the TMC5160/5161 drivers to be set to
constexpr float Tmc5160SenseResistor = 0.050;

constexpr size_t MaxPortsPerHeater = 3;

constexpr size_t MaxBedHeaters = 12;
constexpr size_t MaxChamberHeaters = 8;
constexpr int8_t DefaultE0Heater = 1;				// Index of the default first extruder heater, used only for the legacy status response

constexpr size_t NumThermistorInputs = 4;
constexpr size_t NumTmcDriversSenseChannels = 1;

constexpr size_t MinAxes = 3;						// The minimum and default number of axes
constexpr size_t MaxAxes = 30;						// The maximum number of movement axes in the machine
constexpr size_t MaxDriversPerAxis = 8;				// The maximum number of stepper drivers assigned to one axis

constexpr size_t MaxExtruders = 20;					// The maximum number of extruders
constexpr size_t MaxAxesPlusExtruders = 32;			// May be <= MaxAxes + MaxExtruders

constexpr size_t MaxHeatersPerTool = 20;			// Increased in 3.4 due to OEM requirement
constexpr size_t MaxExtrudersPerTool = 10;			// Increased in 3.4 due to OEM requirement

constexpr unsigned int MaxTriggers = 32;			// Must be <= 32 because we store a bitmap of pending triggers in a uint32_t

/*
constexpr size_t NumSerialChannels = 3;				// The number of serial IO channels not counting the WiFi serial connection (USB and one auxiliary UART)
#define SERIAL_MAIN_DEVICE serialUSB
#define SERIAL_AUX_DEVICE serialUart1
#define SERIAL_AUX2_DEVICE serialUart2
*/
constexpr size_t NumSerialChannels = 3;				// The number of serial IO channels not counting the WiFi serial connection (USB and one auxiliary UART)
constexpr size_t FirstAuxChannel = 1;
constexpr size_t NumAuxChannels = NumSerialChannels - FirstAuxChannel;

#define SERIAL_MAIN_DEVICE serialUSB
#define SERIAL_AUX_DEVICE serialUart1
#define SERIAL_RS485_DEVICE serialUart2
#define SERIAL_RS485_SELECT PortDPin(28)



// Shared SPI (USART 1)
constexpr Pin APIN_USART_SSPI_SCK = PortBPin(13);
constexpr GpioPinFunction USARTSPIMosiPeriphMode = GpioPinFunction::C;
constexpr Pin APIN_USART_SSPI_MOSI = PortBPin(1);
constexpr GpioPinFunction USARTSPIMisoPeriphMode = GpioPinFunction::C;
constexpr Pin APIN_USART_SSPI_MISO = PortBPin(0);
constexpr GpioPinFunction USARTSPISckPeriphMode = GpioPinFunction::C;

constexpr Pin APIN_NETX_SPI_SCK = PortDPin(22);
constexpr Pin APIN_NETX_SPI_MOSI = PortDPin(21);
constexpr Pin APIN_NETX_SPI_MISO = PortDPin(20);
constexpr Pin APIN_NETX_SPI_CS = PortDPin(25);
constexpr GpioPinFunction NetXSPIPeriphMode = GpioPinFunction::B;

// Duet pin numbers for the SBC interface
#define NETX_SPI				SPI0
#define NETX_SPI_INTERFACE_ID	ID_SPI0
#define NETX_SPI_SPI_IRQn		SPI0_IRQn
#define NETX_SPI_SPI_HANDLER	SPI0_Handler

// These next two are defines to avoid the need to include DmacManager.h here
#define NetXSPI_DmaTxPerid	((uint32_t)DmaTrigSource::spi0tx)
#define NetXSPI_DmaRxPerid	((uint32_t)DmaTrigSource::spi0rx)

constexpr Pin UsbVBusPin = PortCPin(21);			// Pin used to monitor VBUS on USB port

// Drivers
#ifdef SUPPORT_EMULATOR
constexpr Pin STEP_PINS[NumDirectDrivers] =			{ NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin };
constexpr Pin DIRECTION_PINS[NumDirectDrivers] =	{ NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin };
constexpr Pin DIAG_PINS[NumDirectDrivers] =			{ NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin };
#else
constexpr Pin STEP_PINS[NumDirectDrivers] =			{ NoPin };
constexpr Pin DIRECTION_PINS[NumDirectDrivers] =	{ NoPin };
constexpr Pin DIAG_PINS[NumDirectDrivers] =			{ NoPin };
#endif

// Pin assignments etc. using USART1 in SPI mode
constexpr Pin GlobalTmc51xxEnablePin = PortAPin(0);			// It is not used
constexpr Pin GlobalTmc51xxCSPin = PortAPin(1);				// It is not used
Usart * const USART_TMC51xx = USART1;
constexpr uint32_t  ID_TMC51xx_SPI = ID_USART1;
constexpr IRQn TMC51xx_SPI_IRQn = USART1_IRQn;
#define TMC51xx_SPI_Handler	USART1_Handler

// These next two are #defines to avoid the need to #include DmacManager.h here
#define TMC51xx_DmaTxPerid	((uint32_t)DmaTrigSource::usart1tx)
#define TMC51xx_DmaRxPerid	((uint32_t)DmaTrigSource::usart1rx)

constexpr Pin TMC51xxMosiPin = PortBPin(4);
constexpr GpioPinFunction TMC51xxMosiPinPeriphMode = GpioPinFunction::D;
constexpr Pin TMC51xxMisoPin = PortAPin(21);
constexpr GpioPinFunction TMC51xxMisoPinPeriphMode = GpioPinFunction::A;
constexpr Pin TMC51xxSclkPin = PortAPin(23);
constexpr GpioPinFunction TMC51xxSclkPinPeriphMode = GpioPinFunction::A;

constexpr uint32_t DefaultStandstillCurrentPercent = 71;

// Thermistor/PT1000 inputs
constexpr Pin TEMP_SENSE_PINS[NumThermistorInputs] = { PortCPin(30), PortCPin(31), PortCPin(26), PortCPin(27) };	// Thermistor/PT1000 pins
constexpr Pin VssaSensePin = PortCPin(29);
constexpr Pin VrefSensePin = PortCPin(0);

// Thermistor series resistor value in Ohms
constexpr float DefaultThermistorSeriesR = 2200.0;
constexpr float MinVrefLoadR = (DefaultThermistorSeriesR / NumThermistorInputs) * 4700.0/((DefaultThermistorSeriesR / NumThermistorInputs) + 4700.0);
																			// there are 4 temperature sensing channels and a 4K7 load resistor
constexpr float VrefSeriesR = 15.0;

// Digital pins the SPI temperature sensors have their select lines tied to
constexpr Pin SpiTempSensorCsPins[] = { PortAPin(19), PortAPin(18) , PortAPin(17), NoPin };

// Analogue pin numbers
constexpr Pin PowerMonitorVinDetectPin = PortAPin(20);
constexpr float PowerMonitorVoltageRange = (60.4 + 4.7)/4.7 * 3.3;

// Digital pin number to turn the IR LED on (high) or off (low), also controls the DIAG LED
constexpr Pin DiagPin = PortCPin(20);
constexpr bool DiagOnPolarity = true;
constexpr Pin ActLedPin = NoPin;
constexpr bool ActOnPolarity = true;


// SD cards
constexpr size_t NumSdCards = 2;												// we now allow one SPI-connected SD card to be configured at boot time
constexpr Pin SdCardDetectPins[NumSdCards] = { PortBPin(5), NoPin };			// the CD pin for the second SD card is allocated using M950 on MB6HC boards before version 1.02
constexpr Pin SdWriteProtectPins[NumSdCards] = { NoPin, NoPin };
constexpr Pin SdSpiCSPins[1] = { NoPin };								// this one is allocated using M950 on MB6HC boards before version 1.02
constexpr uint32_t ExpectedSdCardSpeed = 25000000;
constexpr IRQn SdhcIRQn = HSMCI_IRQn;

// Ethernet
constexpr Pin EthernetPhyInterruptPinPre102 = PortCPin(6);
constexpr Pin EthernetPhyResetPin = PortDPin(11);
constexpr Pin EthernetPhyOtherPins[] = {
		PortDPin(0), PortDPin(1), PortDPin(2), PortDPin(3), PortDPin(4),
		PortDPin(5), PortDPin(6), PortDPin(7), PortDPin(8), PortDPin(9)
};
constexpr auto EthernetPhyOtherPinsFunction = GpioPinFunction::A;

// Shared SPI definitions
#define USART_SPI		1
#define USART_SSPI		USART0
#define ID_SSPI			ID_USART0

// List of assignable pins and their mapping from names to MPU ports. This is indexed by logical pin number.
// The names must match user input that has been concerted to lowercase and had _ and - characters stripped out.
// Aliases are separate by the , character.
// If a pin name is prefixed by ! then this means the pin is hardware inverted. The same pin may have names for both the inverted and non-inverted cases,
// for example the inverted heater pins on the expansion connector are available as non-inverted servo pins on a DueX.
//TODO change the table below for the V0.6 board
constexpr PinDescription PinTable[] =
{
	//	TC					PWM					ADC				Capability				PinNames
	// Port A
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA00 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA01 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA02 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA03 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA04 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA05 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA06 NC
	{ TcOutput::none,	PwmOutput::pwm0h3_b,AdcInput::none,		PinCapability::wpwm,	"out0,bed0"			},	// PA07 OUT0
	{ TcOutput::none,	PwmOutput::pwm1h3_a,AdcInput::none,		PinCapability::wpwm,	"out3"				},	// PA08 OUT3
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::read,	"serial0.rx"		},	// PA09 SERIAL0 RX - Expansion
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::rw,		"serial0.tx"		},	// PA10 SERIAL0 TX - Expansion
	{ TcOutput::none,	PwmOutput::pwm0h0_b,AdcInput::none,		PinCapability::wpwm,	"fan2"				},	// PA11 FAN2
	{ TcOutput::none,	PwmOutput::pwm1h0_c,AdcInput::none,		PinCapability::wpwm,	"out4"				},	// PA12 OUT4
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::read,	"io6.in"			},	// PA13 IO6_IN
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA14 NC
	{ TcOutput::tioa1,	PwmOutput::none,	AdcInput::none,		PinCapability::wpwm,	"fan0"				},	// PA15 fan0
	{ TcOutput::none,	PwmOutput::pwm0l2_c,AdcInput::none,		PinCapability::wpwm,	"out2"				},	// PA16 OUT2
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::rw,		"spi.cs2"			},	// PA17 SPI_CS2
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::rw,		"spi.cs1"			},	// PA18 SPI_CS1
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::rw,		"spi.cs0"			},	// PA19 SPI_CS0
	{ TcOutput::none,	PwmOutput::none,	AdcInput::adc0_9,	PinCapability::none,	nullptr				},	// PA20 VIN detect
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA21 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::read,	"io2.in"			},	// PA22 IO2_IN
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA23 NC
	{ TcOutput::none,	PwmOutput::pwm0h1_b,AdcInput::none,		PinCapability::wpwm,	"out1"				},	// PA24 OUT1
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA25 SDHC MCCK
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA26 SDHC MCDA2
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA27 SDHC MCDA3
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA28 SDHC MCCDA
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA29 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA30 SDHC MCDA0
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PA31 SDHC MCDA1

	// Port B
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB00 SPI PT100 MISO
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB01 SPI PT100 MOSI
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB02 CAN0_TX
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB03 CAN0_RX
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB04 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB05 SDHC Detect
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB06 SWDIO
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB07 SWCLK
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB08	Crystal
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB09 Crystal
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB10 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB11 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB12 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB13 SPI PT100 SCK
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB14 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB15 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB16 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB17 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB18 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB19 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB20 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB21 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB22 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB23 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB24 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB25 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB26 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB27 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB28 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB29 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB30 not on chip
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PB31 not on chip

	// Port C
	{ TcOutput::none,	PwmOutput::none,	AdcInput::adc1_9,	PinCapability::none,	nullptr				},	// PC00 VrefSensePin
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::read,	"io3.in"			},	// PC01 IO3_IN
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::read,	"io4.in"			},	// PC02 IO4_IN
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::read,	"io5.in"			},	// PC03 IO5_IN
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::read,	"io7.in"			},	// PC04 IO7_IN
	{ TcOutput::tioa6,	PwmOutput::none,	AdcInput::none,		PinCapability::wpwm,	"fan1"				},	// PC05 OUT7
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC06 EthernetPhyInterrupt (up to v1.1), USB_PWR_EN (v1.2)
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC07 NC
	{ TcOutput::tioa7,	PwmOutput::none,	AdcInput::none,		PinCapability::wpwm,	"out5"				},	// PC08 OUT5
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC09 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC10 NC
	{ TcOutput::tioa8,	PwmOutput::none,	AdcInput::none,		PinCapability::wpwm,	"fan3"				},	// PC11 OUT9
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC12 CAN1_RX
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC13 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC14 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC15 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC16 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC17 SPI NETX INT
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC18 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC19 ETH_LED_Y (up to v1.1), USB_UFP_DETECT(v1.2)
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC20 DiagPin
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC21 UsbVBusPin
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC22 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC23 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC24 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC25 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::adc1_7,	PinCapability::ainr,	"temp1"				},	// PC26 thermistor 1
	{ TcOutput::none,	PwmOutput::none,	AdcInput::adc1_8,	PinCapability::ainr,	"temp0"				},	// PC27 thermistor 0
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PC25 NC
	{ TcOutput::tioa5,	PwmOutput::none,	AdcInput::adc1_4,	PinCapability::none,	nullptr				},	// PC29 Vssa
	{ TcOutput::none,	PwmOutput::none,	AdcInput::adc1_5,	PinCapability::ainr,	"temp3"				},	// PC30 thermistor 3
	{ TcOutput::none,	PwmOutput::none,	AdcInput::adc1_6,	PinCapability::ainr,	"temp2"				},	// PC31 thermistor 2

	// PORT D
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD00 GTXCK
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD01 GTXEN
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD02 GTX0
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD03 GTX1
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD04 GRXDV
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD05 GRX0
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD06 GRX1
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD07 GRXER
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD08 GMDC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD09 GMDIO
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD10 SPI NETX - Reset
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD11 EthernetPhyReset
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD12 CAN1_TX
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD13 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD14 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD15 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::rw,		"io7.out"			},	// PD16 IO7_OUT (no PWM)
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD17 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::read,	"serial1.rx"		},	// PD18 SERIAL1 RX - RS485
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::rw,		"serial1.tx"		},	// PD19 SERIAL1 TX - RS485
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD20 SPI NETX - MISO
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD21 SPI NETX - MOSI
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD22 SPI NETX - CLK
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD23 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD24 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD25 SPI NETX - CS
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD26 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::rw,		"io6.out"			},	// PD27 IO7_OUT (no PWM)
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD28 SERIAL1 SELECT - RS485
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD29 SPI NETX - D Interrupt
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::ainr,	"io0.in"			},	// PD30 IO0_IN
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PD31 driver 4 diag

	// Port E
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PE00 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PE01 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PE02 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PE03 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::none,	nullptr				},	// PE04 NC
	{ TcOutput::none,	PwmOutput::none,	AdcInput::none,		PinCapability::ainr,	"io1.in"			},	// PE05 IO1_IN
};

constexpr size_t NumNamedPins = ARRAY_SIZE(PinTable);
constexpr size_t NumRealPins = 32+32+32+32+6;
constexpr size_t NumVirtualPins = 0;

static_assert(NumNamedPins == NumRealPins + NumVirtualPins);

// Serial Interfaces
constexpr Pin APIN_Serial0_RXD = PortAPin(9);
constexpr Pin APIN_Serial0_TXD = PortAPin(10);
constexpr auto Serial0PinFunction = GpioPinFunction::A;
constexpr Pin APIN_Serial1_RXD = PortDPin(18);
constexpr Pin APIN_Serial1_TXD = PortDPin(19);
constexpr auto Serial1PinFunction = GpioPinFunction::C;

// SD Card
constexpr Pin HsmciMclkPin = PortAPin(25);
constexpr auto HsmciMclkPinFunction = GpioPinFunction::D;
constexpr Pin HsmciOtherPins[] = { PortAPin(26), PortAPin(27), PortAPin(28), PortAPin(30), PortAPin(31) };
constexpr auto HsmciOtherPinsFunction = GpioPinFunction::C;

// Duet pin numbers for the SBC interface
#define SBC_SPI					SPI1
#define SBC_SPI_INTERFACE_ID	ID_SPI1
#define SBC_SPI_IRQn			SPI1_IRQn
#define SBC_SPI_HANDLER			SPI1_Handler

constexpr Pin APIN_SBC_SPI_MOSI = APIN_NETX_SPI_MOSI;
constexpr Pin APIN_SBC_SPI_MISO = APIN_NETX_SPI_MISO;
constexpr Pin APIN_SBC_SPI_SCK = APIN_NETX_SPI_SCK;
constexpr Pin APIN_SBC_SPI_SS0 = APIN_NETX_SPI_CS;
constexpr GpioPinFunction SBCPinPeriphMode = NetXSPIPeriphMode;

constexpr Pin SbcTfrReadyPin = PortEPin(2);
// Note, the DMAC peripheral IDs are hard-coded in DataTransfer

// CAN
constexpr Pin APIN_CAN0_RX = PortBPin(3);
constexpr Pin APIN_CAN0_TX = PortBPin(2);
constexpr GpioPinFunction CAN0PinPeriphMode = GpioPinFunction::A;

constexpr Pin APIN_CAN1_RX = PortCPin(12);
constexpr GpioPinFunction CAN1RXPinPeriphMode = GpioPinFunction::C;
constexpr Pin APIN_CAN1_TX = PortDPin(12);
constexpr GpioPinFunction CAN1TXPinPeriphMode = GpioPinFunction::B;

constexpr unsigned int CanDeviceNumber = 1;				// CAN-FD device number

#if (defined(DUAL_CAN) && DUAL_CAN) || (defined(DUAL_CAN_FD) && DUAL_CAN_FD)
constexpr unsigned int SecondaryCanDeviceNumber = 0;	// plan CAN device number
#endif

// Timer allocation
// Step timer is timer 0 aka TC0 channel 0. Also used as the CAN timestamp counter.
#define STEP_TC				(TC0)
#define STEP_TC_CHAN		(0)					// channel for lower 16 bits
#define STEP_TC_CHAN_UPPER	(2)					// channel for upper 16 bits
#define STEP_TC_IRQN		TC0_IRQn
#define STEP_TC_HANDLER		TC0_Handler
#define STEP_TC_ID			ID_TC0
#define STEP_TC_ID_UPPER	ID_TC2

// DMA channel allocation
constexpr DmaChannel DmacChanHsmci = 0;			// this is hard coded in the ASF HSMCI driver
constexpr DmaChannel DmacChanWiFiTx = 1;
constexpr DmaChannel DmacChanWiFiRx = 2;
constexpr DmaChannel DmacChanTmcTx = 3;
constexpr DmaChannel DmacChanTmcRx = 4;
constexpr DmaChannel DmacChanSbcTx = 5;
constexpr DmaChannel DmacChanNetXSPITx = DmacChanSbcTx;	// It is the same channel so SBC can't be used.
constexpr DmaChannel DmacChanSbcRx = 6;
constexpr DmaChannel DmacChanNetXSPIRx = DmacChanSbcRx;	// It is the same channel so SBC can't be used.
constexpr DmaChannel DmacChanDotStarTx = 7;

constexpr size_t NumDmaChannelsUsed = 8;

namespace StepPins
{
	// *** These next three functions must use the same bit assignments in the drivers bitmap ***
	// Each stepper driver must be assigned one bit in a 32-bit word, in such a way that multiple drivers can be stepped efficiently
	// and more or less simultaneously by doing parallel writes to several bits in one or more output ports.
	// All our step pins are on port C, so the bitmap is just the map of step bits in port C.

	// Calculate the step bit for a driver. This doesn't need to be fast. It must return 0 if the driver is remote.
	static inline uint32_t CalcDriverBitmap(size_t driver) noexcept
	{
		return (driver < NumDirectDrivers)
				? 1u << (STEP_PINS[driver] & 0x1Fu)
				: 0;
	}

	// Set the specified step pins high. This needs to be fast.
	static inline __attribute__((always_inline)) void StepDriversHigh(uint32_t driverMap) noexcept
	{
		PIOC->PIO_SODR = driverMap;				// on Duet 3 all step pins are on port C
	}

	// Set the specified step pins low. This needs to be fast.
	static inline __attribute__((always_inline)) void StepDriversLow(uint32_t driverMap) noexcept
	{
		PIOC->PIO_CODR = driverMap;				// on Duet 3 all step pins are on port C
	}
}

#endif	//ifndef PINS_XBOARD_V3_H__
