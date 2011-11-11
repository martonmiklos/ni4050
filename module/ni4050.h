#ifndef	_NI4050_H_
#define	_NI4050_H_

#include <linux/types.h>

typedef struct 
{
	unsigned int address;
	unsigned char data;
} EEPROMInfo;

typedef enum _NI4050_RANGES
{
   NI4050_RANGE_INVALID = -1, 
   NI4050_RANGE_250VDC = 0, 
   NI4050_RANGE_25VDC,
   NI4050_RANGE_2VDC,
   NI4050_RANGE_200mVDC,
   NI4050_RANGE_20mVDC,
   NI4050_RANGE_250VAC,
   NI4050_RANGE_25VAC,
   NI4050_RANGE_2VAC,
   NI4050_RANGE_200mVAC,
   NI4050_RANGE_20mVAC,
   NI4050_RANGE_EXTOHM,
   NI4050_RANGE_2MOHM,
   NI4050_RANGE_200kOHM,
   NI4050_RANGE_20kOHM,
   NI4050_RANGE_2kOHM,
   NI4050_RANGE_200OHM,
   NI4050_RANGE_DIODE
} NI4050_RANGES;


#define	NI4050_MAX_DEV		4

#define	NIDMM_IOC_MAXNR	        255
#define NIDMM_IOC_MAGIC 		'n'


//#define	NIDMM_IOCSTATUS						_IOR (NIDMM_IOC_MAGIC, 0, int *)
#define	NIDMM_IOCADCREADY					_IOR (NIDMM_IOC_MAGIC, 0, int *)

#define	NIDMM_IOCEEPROMREAD					_IOR (NIDMM_IOC_MAGIC, 1, EEPROMInfo *)
#define	NIDMM_IOCEEPROMWRITE				_IOW (NIDMM_IOC_MAGIC, 2, EEPROMInfo *)
#define	NIDMM_IOCEEPROMREADINTRES			_IOR (NIDMM_IOC_MAGIC, 3, unsigned int *)
#define NIDMM_IOCSTARTMEASUREMENT			_IOW (NIDMM_IOC_MAGIC, 4, NI4050_RANGES *)
#define NIDMM_IOCREADDATA					_IOR (NIDMM_IOC_MAGIC, 5, double *)


/* card and device states */
/*#define	NIDMM_CARD_INSERTED		0x01
#define	NIDMM_CARD_POWERED			0x02
#define	NIDMM_ATR_PRESENT			0x04
#define	NIDMM_ATR_VALID	 		0x08
#define	NIDMM_STATE_VALID			0x0f*/
/* extra info only from NI4050 */
/*#define	NIDMM_NO_READER			0x10
#define	NIDMM_BAD_CARD			0x20*/


#ifdef __KERNEL__

#define	DEVICE_NAME		"nidmm"
#define	MODULE_NAME		"ni4050"

#endif	/* __KERNEL__ */


// Binary Conversion Values

// These values are used in the conversion from binary
// values to engineering units. These are the actual 
// unipolar ranges of the ADC

#define NI4050_CONVERT_RANGE_250VDC             250.0
#define NI4050_CONVERT_RANGE_25VDC              31.25
#define NI4050_CONVERT_RANGE_2VDC               2.5
#define NI4050_CONVERT_RANGE_200mVDC            0.3125
#define NI4050_CONVERT_RANGE_20mVDC             0.0390625

#define NI4050_CONVERT_RANGE_250VAC             250.0
#define NI4050_CONVERT_RANGE_25VAC              31.25
#define NI4050_CONVERT_RANGE_2VAC               3.125
#define NI4050_CONVERT_RANGE_200mVAC            0.3125
#define NI4050_CONVERT_RANGE_20mVAC             0.03125

#define NI4050_CONVERT_RANGE_EXTOHM             200000000.0
#define NI4050_CONVERT_RANGE_2MOHM              2500000.0
#define NI4050_CONVERT_RANGE_200kOHM            312500.0
#define NI4050_CONVERT_RANGE_20kOHM             25000
#define NI4050_CONVERT_RANGE_2kOHM              3125
#define NI4050_CONVERT_RANGE_200OHM             390.625

#define NI4050_CONVERT_RANGE_DIODE              2.5


// Default value of internal resistor and its limits

#define NI4050_INTERNAL_RESISTANCE_SPEC         1000000.0
#define NI4050_INTERNAL_RESISTANCE_SPEC_MAX     1200000
#define NI4050_INTERNAL_RESISTANCE_SPEC_MIN     800000



// All following constants define the registers 
// and bits on the NI4050 register map

// Register Offsets

#define NI4050_COMMAND_REG          0x0
#define NI4050_ADC_COMMAND_REG      0x1
#define NI4050_ADC_WRITE_REG        0x2
#define NI4050_CONFIG_REG           0x3
#define NI4050_EEPROM_ADDR1_REG     0x4
#define NI4050_EEPROM_ADDR2_REG     0x5
#define NI4050_EEPROM_DATA_REG      0x6

#define NI4050_STATUS_REG           0x00
#define NI4050_ADC_DATA1_REG        0x01
#define NI4050_ADC_DATA2_REG        0x02
#define NI4050_ADC_DATA3_REG        0x03

// Command Register Bits

#define NI4050_COMMAND_EEPROM_WE    0x40
#define NI4050_COMMAND_PWRDN        0x10
#define NI4050_COMMAND_ADCINTEN     0x04

#define NI4050_COMMAND_DEFAULT      0x00

// Status Register Bits

#define NI4050_STATUS_D_AVAIL       0x20
#define NI4050_STATUS_PWRDN         0x10
#define NI4050_STATUS_ACQ_EN        0x08
#define NI4050_STATUS_ADC_RDY       0x04
#define NI4050_STATUS_OVERFLOW      0x02
#define NI4050_STATUS_NEW_DATA      0x01

#define NI4050_STATUS_REG_DEFAULT   0xC0

// ADC Command Register Bits

#define NI4050_ADC_COMMAND_DEFAULT                 0x04

#define NI4050_ADC_COMMAND_START                   0x80

#define NI4050_ADC_COMMAND_REGSEL_REGONLY          0x00
#define NI4050_ADC_COMMAND_REGSEL_MODEREG          0x10
#define NI4050_ADC_COMMAND_REGSEL_FILTERHIGH       0x20
#define NI4050_ADC_COMMAND_REGSEL_FILTERLOW        0x30
#define NI4050_ADC_COMMAND_REGSEL_DATAREG          0x50
#define NI4050_ADC_COMMAND_REGSEL_ZEROCALIB        0x60
#define NI4050_ADC_COMMAND_REGSEL_FULLCALIB        0x70

#define NI4050_ADC_COMMAND_READ                    0x08
#define NI4050_ADC_COMMAND_WRITE                   0x00

#define NI4050_ADC_COMMAND_MODE_OHMS               0x00
#define NI4050_ADC_COMMAND_MODE_VDC                0x01
#define NI4050_ADC_COMMAND_MODE_VAC                0x02
#define NI4050_ADC_COMMAND_MODE_AUTOZERO           0x03
#define NI4050_ADC_COMMAND_MODE_DIODE              0x01

#define NI4050_ADC_COMMAND_RESET                   0xFF

// ADC Write Register Bits - Mode Register

#define NI4050_ADC_WRITE_MODE_DEFAULT              0x00

#define NI4050_ADC_WRITE_MODE_NORMAL               0x00
#define NI4050_ADC_WRITE_MODE_SELFCAL              0x20
#define NI4050_ADC_WRITE_MODE_ZEROCAL              0x40
#define NI4050_ADC_WRITE_MODE_FULLCAL              0x60
#define NI4050_ADC_WRITE_MODE_SYSOFFSET            0x80
#define NI4050_ADC_WRITE_MODE_BACKGROUND           0xA0
#define NI4050_ADC_WRITE_MODE_ZEROSELF             0xC0
#define NI4050_ADC_WRITE_MODE_FULLSELF             0xE0

#define NI4050_ADC_WRITE_GAIN_1                    0x00
#define NI4050_ADC_WRITE_GAIN_2                    0x04
#define NI4050_ADC_WRITE_GAIN_4                    0x08
#define NI4050_ADC_WRITE_GAIN_8                    0x0C
#define NI4050_ADC_WRITE_GAIN_16                   0x10
#define NI4050_ADC_WRITE_GAIN_32                   0x14
#define NI4050_ADC_WRITE_GAIN_64                   0x18
#define NI4050_ADC_WRITE_GAIN_128                  0x1C

#define NI4050_ADC_WRITE_GAIN_250VDC               0x04
#define NI4050_ADC_WRITE_GAIN_25VDC                0x10
#define NI4050_ADC_WRITE_GAIN_2VDC                 0x00
#define NI4050_ADC_WRITE_GAIN_200mVDC              0x0C
#define NI4050_ADC_WRITE_GAIN_20mVDC               0x18
#define NI4050_ADC_WRITE_GAIN_250VAC               0x04
#define NI4050_ADC_WRITE_GAIN_25VAC                0x10
#define NI4050_ADC_WRITE_GAIN_2VAC                 0x10
#define NI4050_ADC_WRITE_GAIN_200mVAC              0x0C
#define NI4050_ADC_WRITE_GAIN_20mVAC               0x0C
#define NI4050_ADC_WRITE_GAIN_EXTOHM               0x00
#define NI4050_ADC_WRITE_GAIN_2MOHM                0x00
#define NI4050_ADC_WRITE_GAIN_200kOHM              0x0C
#define NI4050_ADC_WRITE_GAIN_20kOHM               0x00
#define NI4050_ADC_WRITE_GAIN_2kOHM                0x0C
#define NI4050_ADC_WRITE_GAIN_200OHM               0x18
#define NI4050_ADC_WRITE_GAIN_DIODE                0x00

#define NI4050_ADC_WRITE_FSYNCH                    0x01

// ADC Write Register Bits - Filter High Register

#define NI4050_ADC_WRITE_FILTERHIGH_10HZ           0x67
#define NI4050_ADC_WRITE_FILTERHIGH_50HZ           0x61
#define NI4050_ADC_WRITE_FILTERHIGH_60HZ           0x61

// ADC Write Register Bits - Filter Low Register

#define NI4050_ADC_WRITE_FILTERLOW_10HZ            0x80
#define NI4050_ADC_WRITE_FILTERLOW_50HZ            0x80
#define NI4050_ADC_WRITE_FILTERLOW_60HZ            0x40

// Configuration Register

#define NI4050_CONFIG_O_R_VDC                   0x00
#define NI4050_CONFIG_O_R_VAC                   0x00
#define NI4050_CONFIG_O_R_EXTOHM                0x20
#define NI4050_CONFIG_O_R_2MOHM                 0x20
#define NI4050_CONFIG_O_R_200kOHM               0x20
#define NI4050_CONFIG_O_R_20kOHM                0x40
#define NI4050_CONFIG_O_R_2kOHM                 0x40
#define NI4050_CONFIG_O_R_200OHM                0x40
#define NI4050_CONFIG_O_R_DIODE                 0x40

#define NI4050_CONFIG_AC_R_VDC                  0x00
#define NI4050_CONFIG_AC_R_250VAC               0x00
#define NI4050_CONFIG_AC_R_25VAC                0x00
#define NI4050_CONFIG_AC_R_2VAC                 0x10
#define NI4050_CONFIG_AC_R_200mVAC              0x00
#define NI4050_CONFIG_AC_R_20mVAC               0x10
#define NI4050_CONFIG_AC_R_OHMS                 0x00
#define NI4050_CONFIG_AC_R_DIODE                0x00

#define NI4050_CONFIG_OHMS_VDCVAC               0x08
#define NI4050_CONFIG_OHMS_OHMS                 0x00
#define NI4050_CONFIG_OHMS_DIODE                0x00

#define NI4050_CONFIG_I_R_250VDC                0x06
#define NI4050_CONFIG_I_R_25VDC                 0x06
#define NI4050_CONFIG_I_R_2VDC                  0x05
#define NI4050_CONFIG_I_R_200mVDC               0x05
#define NI4050_CONFIG_I_R_20mVDC                0x05
#define NI4050_CONFIG_I_R_250VAC                0x02
#define NI4050_CONFIG_I_R_25VAC                 0x02
#define NI4050_CONFIG_I_R_2VAC                  0x02
#define NI4050_CONFIG_I_R_200mVAC               0x03
#define NI4050_CONFIG_I_R_20mVAC                0x03
#define NI4050_CONFIG_I_R_EXTOHMS               0x07
#define NI4050_CONFIG_I_R_OHMS                  0x05
#define NI4050_CONFIG_I_R_DIODE                 0x05

// EEPROM Offsets

#define NI4050_EEPROM_AREA_USER                 0x0400
#define NI4050_EEPROM_AREA_LOAD                 0x0800
#define NI4050_EEPROM_AREA_FACTORY              0x0C00

#define NI4050_EEPROM_MODE_VDC                  0x0000
#define NI4050_EEPROM_MODE_VAC                  0x00A0
#define NI4050_EEPROM_MODE_OHMS                 0x0140
#define NI4050_EEPROM_MODE_DIODE                0x01E0

#define NI4050_EEPROM_INTERNAL_RESISTANCE       0x01F2

#define NI4050_EEPROM_RANGE_250V                0x00
#define NI4050_EEPROM_RANGE_25V                 0x20
#define NI4050_EEPROM_RANGE_2V                  0x40
#define NI4050_EEPROM_RANGE_200mV               0x60
#define NI4050_EEPROM_RANGE_20mV                0x80
#define NI4050_EEPROM_RANGE_EXTOHM              0x00
#define NI4050_EEPROM_RANGE_2MOHM               0x00
#define NI4050_EEPROM_RANGE_200kOHM             0x20
#define NI4050_EEPROM_RANGE_20kOHM              0x40
#define NI4050_EEPROM_RANGE_2kOHM               0x60
#define NI4050_EEPROM_RANGE_200OHM              0x80

#define NI4050_EEPROM_FILTER_10HZ               0x00
#define NI4050_EEPROM_FILTER_50HZ               0x06
#define NI4050_EEPROM_FILTER_60HZ               0x0C

#define NI4050_EEPROM_CAL_ZERO                  0x00
#define NI4050_EEPROM_CAL_FULL                  0x03
#endif	/* _NI4050_H_ */
