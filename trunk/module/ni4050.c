/*
  * A driver for the National Instruments PCMCIA 4050 Digital Multimeter
  *
  * ni4050.c
  *
  * Written by Miklós Márton martonmiklosqdev@gmail.com according to the
  * provided MHDDK Windows CE example provided by the NI here:
  * https://lumen.ni.com/nicif/us/evalmhddk/content.xhtml
  * All rights reserved. Licensed under LGPL license.
  */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/bitrev.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/wait.h>

#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ciscode.h>
#include <pcmcia/ds.h>

#include "ni4050.h"

typedef struct
{
	unsigned int range;
	unsigned int calConstantOffset;
	unsigned char inputRange;
	unsigned char ohmsMode;
	unsigned char acRange;
	unsigned char ohmsRange;
	unsigned char measurmentMode;
	unsigned char gain;
	unsigned int filterValueH;
	unsigned int filterValueL;
} MeasurementData;

MeasurementData measurmentInfo[] =
{
	{
		NI4050_RANGE_250VDC,
		NI4050_EEPROM_MODE_VDC | NI4050_EEPROM_RANGE_250V | NI4050_EEPROM_FILTER_10HZ, // calConstantOffset
		NI4050_CONFIG_I_R_250VDC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_VDC,
		NI4050_CONFIG_O_R_VDC,
		NI4050_ADC_COMMAND_MODE_VDC,
		NI4050_ADC_WRITE_GAIN_250VDC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_10HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_10HZ // filterValueL
	}, // 0

	{
		NI4050_RANGE_25VDC,
		NI4050_EEPROM_MODE_VDC | NI4050_EEPROM_RANGE_25V | NI4050_EEPROM_FILTER_50HZ, // calConstantOffset
		NI4050_CONFIG_I_R_25VDC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_VDC,
		NI4050_CONFIG_O_R_VDC,
		NI4050_ADC_COMMAND_MODE_VDC,
		NI4050_ADC_WRITE_GAIN_25VDC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_50HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_50HZ // filterValueL
	}, // 1

	{
		NI4050_RANGE_2VDC,
		NI4050_EEPROM_MODE_VDC | NI4050_EEPROM_RANGE_2V | NI4050_EEPROM_FILTER_60HZ, // calConstantOffset
		NI4050_CONFIG_I_R_2VDC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_VDC,
		NI4050_CONFIG_O_R_VDC,
		NI4050_ADC_COMMAND_MODE_VDC,
		NI4050_ADC_WRITE_GAIN_2VDC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_60HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_60HZ // filterValueL
	}, // 2

	{
		NI4050_RANGE_200mVDC,
		NI4050_EEPROM_MODE_VDC | NI4050_EEPROM_RANGE_200mV | NI4050_EEPROM_FILTER_60HZ, // calConstantOffset
		NI4050_CONFIG_I_R_200mVDC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_VDC,
		NI4050_CONFIG_O_R_VDC,
		NI4050_ADC_COMMAND_MODE_VDC,
		NI4050_ADC_WRITE_GAIN_200mVDC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_60HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_60HZ // filterValueL
	}, // 3

	{
		NI4050_RANGE_20mVDC,
		NI4050_EEPROM_MODE_VDC | NI4050_EEPROM_RANGE_20mV | NI4050_EEPROM_FILTER_60HZ, // calConstantOffset
		NI4050_CONFIG_I_R_20mVDC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_VDC,
		NI4050_CONFIG_O_R_VDC,
		NI4050_ADC_COMMAND_MODE_VDC,
		NI4050_ADC_WRITE_GAIN_20mVDC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_60HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_60HZ // filterValueL
	}, // 3


	{
		NI4050_RANGE_250VAC,
		NI4050_EEPROM_MODE_VAC | NI4050_EEPROM_RANGE_250V | NI4050_EEPROM_FILTER_60HZ, // calConstantOffset
		NI4050_CONFIG_I_R_250VAC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_250VAC,
		NI4050_CONFIG_O_R_VAC,
		NI4050_ADC_COMMAND_MODE_VAC,
		NI4050_ADC_WRITE_GAIN_250VAC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_60HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_60HZ // filterValueL
	}, // 4

	{
		NI4050_RANGE_25VAC,
		NI4050_EEPROM_MODE_VAC | NI4050_EEPROM_RANGE_25V | NI4050_EEPROM_FILTER_10HZ, // calConstantOffset
		NI4050_CONFIG_I_R_25VAC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_25VAC,
		NI4050_CONFIG_O_R_VAC,
		NI4050_ADC_COMMAND_MODE_VAC,
		NI4050_ADC_WRITE_GAIN_25VAC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_10HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_10HZ // filterValueL
	}, // 5

	{
		NI4050_RANGE_2VAC,
		NI4050_EEPROM_MODE_VAC | NI4050_EEPROM_RANGE_2V | NI4050_EEPROM_FILTER_10HZ, // calConstantOffset
		NI4050_CONFIG_I_R_2VAC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_2VAC,
		NI4050_CONFIG_O_R_VAC,
		NI4050_ADC_COMMAND_MODE_VAC,
		NI4050_ADC_WRITE_GAIN_2VAC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_10HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_10HZ // filterValueL
	}, // 5

	{
		NI4050_RANGE_200mVAC,
		NI4050_EEPROM_MODE_VAC | NI4050_EEPROM_RANGE_200mV | NI4050_EEPROM_FILTER_50HZ, // calConstantOffset
		NI4050_CONFIG_I_R_200mVAC,
		NI4050_CONFIG_OHMS_VDCVAC,
		NI4050_CONFIG_AC_R_200mVAC,
		NI4050_CONFIG_O_R_VAC,
		NI4050_ADC_COMMAND_MODE_VAC,
		NI4050_ADC_WRITE_GAIN_200mVAC, // gain
		NI4050_ADC_WRITE_FILTERHIGH_50HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_50HZ // filterValueL
	}, // 6

	{
		NI4050_RANGE_DIODE,
		NI4050_EEPROM_MODE_DIODE | NI4050_EEPROM_FILTER_50HZ, // calConstantOffset
		NI4050_CONFIG_I_R_DIODE,
		NI4050_CONFIG_OHMS_DIODE,
		NI4050_CONFIG_AC_R_DIODE,
		NI4050_CONFIG_O_R_DIODE,
		NI4050_ADC_COMMAND_MODE_DIODE,
		NI4050_ADC_WRITE_GAIN_DIODE, // gain
		NI4050_ADC_WRITE_FILTERHIGH_60HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_60HZ // filterValueL
	}, // 7

	{
		NI4050_RANGE_EXTOHM,
		NI4050_EEPROM_MODE_OHMS | NI4050_EEPROM_RANGE_EXTOHM | NI4050_EEPROM_FILTER_10HZ, // calConstantOffset
		NI4050_CONFIG_I_R_EXTOHMS,
		NI4050_CONFIG_OHMS_OHMS,
		NI4050_CONFIG_AC_R_OHMS,
		NI4050_CONFIG_O_R_EXTOHM,
		NI4050_ADC_COMMAND_MODE_OHMS,
		NI4050_ADC_WRITE_GAIN_EXTOHM, // gain
		NI4050_ADC_WRITE_FILTERHIGH_10HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_10HZ // filterValueL
	}, // 8

	{
		NI4050_RANGE_2MOHM,
		NI4050_EEPROM_MODE_OHMS | NI4050_EEPROM_RANGE_2MOHM | NI4050_EEPROM_FILTER_10HZ, // calConstantOffset
		NI4050_CONFIG_I_R_OHMS,
		NI4050_CONFIG_OHMS_OHMS,
		NI4050_CONFIG_AC_R_OHMS,
		NI4050_CONFIG_O_R_2MOHM,
		NI4050_ADC_COMMAND_MODE_OHMS,
		NI4050_ADC_WRITE_GAIN_2MOHM, // gain
		NI4050_ADC_WRITE_FILTERHIGH_10HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_10HZ // filterValueL
	}, // 9

	{
		NI4050_RANGE_20kOHM,
		NI4050_EEPROM_MODE_OHMS | NI4050_EEPROM_RANGE_20kOHM | NI4050_EEPROM_FILTER_10HZ, // calConstantOffset
		NI4050_CONFIG_I_R_OHMS,
		NI4050_CONFIG_OHMS_OHMS,
		NI4050_CONFIG_AC_R_OHMS,
		NI4050_CONFIG_O_R_20kOHM,
		NI4050_ADC_COMMAND_MODE_OHMS,
		NI4050_ADC_WRITE_GAIN_20kOHM, // gain
		NI4050_ADC_WRITE_FILTERHIGH_10HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_10HZ // filterValueL
	}, // 10

	{
		NI4050_RANGE_2kOHM,
		NI4050_EEPROM_MODE_OHMS | NI4050_EEPROM_RANGE_2kOHM | NI4050_EEPROM_FILTER_10HZ, // calConstantOffset
		NI4050_CONFIG_I_R_OHMS,
		NI4050_CONFIG_OHMS_OHMS,
		NI4050_CONFIG_AC_R_OHMS,
		NI4050_CONFIG_O_R_2kOHM,
		NI4050_ADC_COMMAND_MODE_OHMS,
		NI4050_ADC_WRITE_GAIN_2kOHM, // gain
		NI4050_ADC_WRITE_FILTERHIGH_10HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_10HZ // filterValueL
	}, // 12

	{
		NI4050_RANGE_200OHM,
		NI4050_EEPROM_MODE_OHMS | NI4050_EEPROM_RANGE_200OHM | NI4050_EEPROM_FILTER_10HZ, // calConstantOffset
		NI4050_CONFIG_I_R_OHMS,
		NI4050_CONFIG_OHMS_OHMS,
		NI4050_CONFIG_AC_R_OHMS,
		NI4050_CONFIG_O_R_200OHM,
		NI4050_ADC_COMMAND_MODE_OHMS,
		NI4050_ADC_WRITE_GAIN_200OHM, // gain
		NI4050_ADC_WRITE_FILTERHIGH_10HZ, //filterValueH,
		NI4050_ADC_WRITE_FILTERLOW_10HZ // filterValueL
	}, // 13

	{
		NI4050_RANGE_INVALID,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	}
};

static DEFINE_MUTEX(ni4050_mutex);

static void ni4050_release(struct pcmcia_device *link);

static int major;		/* major number we get from the kernel */

struct ni4050_dev {
	struct pcmcia_device *p_dev;

	// internal resistance of the card readed from the EEPROM
	unsigned int dIntResistorValue;

	// the current measurement type
	NI4050_RANGES measurmentMode;

	int ZeroScaleCalCoeff;
	int FullScaleCalCoeff;

	unsigned char flags0;	/* cardman IO-flags 0 */
	unsigned char flags1;	/* cardman IO-flags 1 */

	unsigned char ta1;
	unsigned char proto;	/* T=0, T=1, ... */
	unsigned long flags;	/* lock+flags (MONITOR,IO,ATR) * for concurrent
				   access */


};


static struct pcmcia_device *dev_table[NI4050_MAX_DEV];
static struct class *ni4050_class;

#ifndef ni4050_DEBUG
#define	xoutb	outb
#define	xinb	inb
#else
const char *addressNames[7] = {
	"NI4050_COMMAND_REG",
	"NI4050_ADC_COMMAND_REG",
	"NI4050_ADC_WRITE_REG",
	"NI4050_CONFIG_REG",
	"NI4050_EEPROM_ADDR1_REG",
	"NI4050_EEPROM_ADDR2_REG",
	"NI4050_EEPROM_DATA_REG"
};

char buf[128];

const char *getAddressName(unsigned short address) 
{
	if ((address < 0x5107) && (address >= 0x5100)) {
		return addressNames[address - 0x5100];
	} else {
		sprintf(buf, "Unknown address: 0x%08x", address);
		return buf;
	}
}

static inline void xoutb(unsigned char val, unsigned short port)
{
	pr_debug("outb(%s, %02x)\n", getAddressName(port), val);
	outb(val, port);
}
static inline unsigned char xinb(unsigned short port)
{
	unsigned char val;

	val = inb(port);
	pr_debug("inb(%s) = 0x%02x\n", getAddressName(port), val);

	return val;
}

#endif

unsigned char readEEPROM(struct ni4050_dev *dev, unsigned int address) 
{
	unsigned int iobase = dev->p_dev->resource[0]->start;
	unsigned char ret = 0;
	xoutb((address >> 8) & 0xFF, iobase + NI4050_EEPROM_ADDR2_REG);
	xoutb((address) & 0xFF, iobase + NI4050_EEPROM_ADDR1_REG);
	ret = xinb(iobase + NI4050_EEPROM_DATA_REG);
	return ret;
}

// Read 3-byte EEPROM value (calibration coefficient) from
// the specified EEPROM address offset
int readEEPROMWord(struct ni4050_dev *dev, unsigned int address) 
{
	int i = 0, ret = 0;

	for (;i<3;i++)
	{
		ret += (readEEPROM(dev, (address + i)) << (8*i));
	}

	return ret;
}

void setWriteEEPROMEnable(struct ni4050_dev *dev, unsigned char enabled)
{
	unsigned char tmp = 0;
	unsigned int iobase = dev->p_dev->resource[0]->start;
	if (enabled) {
		tmp = xinb(iobase + NI4050_COMMAND_REG);
		tmp |= NI4050_COMMAND_EEPROM_WE;
		xoutb(tmp, iobase + NI4050_COMMAND_REG);
	} else {
		tmp = xinb(iobase + NI4050_COMMAND_REG);
		tmp &= ~NI4050_COMMAND_EEPROM_WE;
		xoutb(tmp, iobase + NI4050_COMMAND_REG);
	}
}

/*static unsigned char writeEEPROM(struct ni4050_dev *dev, unsigned int address, unsigned char data) 
{
	unsigned int iobase = dev->p_dev->resource[0]->start;

	//check to see if EEPROM offset is within allowed range
	if (address >= 0x300 && address < 0x400)
	{
		setWriteEEPROMEnable(dev, 1);

		xoutb((address >> 8) & 0xFF, iobase + NI4050_EEPROM_ADDR2_REG);
		xoutb((address) & 0xFF, iobase + NI4050_EEPROM_ADDR1_REG);
		xoutb(data, iobase + NI4050_EEPROM_DATA_REG);

		setWriteEEPROMEnable(dev, 0);

		pr_debug("writeEEPROM 0x%04x t0x%02x\n", address, data);
	}
	else
	{
		pr_debug("Error writing to EEPROM: invalid address: 0x%04x.\n", address);
		return -1;
	}

	return 0;
}*/

// Check if ADC is ready for the next write operation
int adcReady(struct ni4050_dev *dev)
{
	unsigned int iobase = dev->p_dev->resource[0]->start;
	return (int) (xinb(iobase + NI4050_COMMAND_REG) & NI4050_STATUS_ADC_RDY);
};


// Loops on adcReady until it gets ready
int waitForAdcReady(struct ni4050_dev *dev)
{
	unsigned int ready = 0;
	unsigned int timeOutCounter = 0;
	while (!ready) {
		ready = adcReady(dev);
		msleep(1);
		timeOutCounter++;
		if (timeOutCounter == NI4050_ADC_READY_TIMEOUT_MS)
			return -1;
	}
	return 0;
};

// Read actual internal resistor value from EEPROM 
int eepromReadResistance(struct ni4050_dev *dev)
{
	unsigned char i = 0;
	unsigned int address = NI4050_EEPROM_AREA_LOAD | NI4050_EEPROM_INTERNAL_RESISTANCE;

	dev->dIntResistorValue = 0;

	for (; i<3; i++)
	{
		dev->dIntResistorValue += (double)(readEEPROM(dev, (address + i))<<(8*i));
	}

	pr_debug("ni 4050 internal resistance: %d Ohm\n", dev->dIntResistorValue);

	if ( (dev->dIntResistorValue < 0.8 * NI4050_INTERNAL_RESISTANCE_SPEC_MIN) ||
		 (dev->dIntResistorValue > 1.2 * NI4050_INTERNAL_RESISTANCE_SPEC_MAX) )
	{
		pr_debug("ni 4050 internal resistance: %d is out of range\n \
			   not between %d and %d\n",
			   dev->dIntResistorValue,
			   NI4050_INTERNAL_RESISTANCE_SPEC_MIN,
			   NI4050_INTERNAL_RESISTANCE_SPEC_MAX);
		return -1;
	}

	return 0;
}

int measurmentIsReady(struct ni4050_dev *dev)
{
	unsigned char ret = xinb(dev->p_dev->resource[0]->start + NI4050_STATUS_REG);
	if (ret & NI4050_STATUS_OVERFLOW)
	{
		pr_debug("Overflow\n");
	}
	
	return (ret & NI4050_STATUS_NEW_DATA);
}


// Read 3-byte data value (binary measurement) from the board
int measurmentDataRead(struct ni4050_dev *dev, int *value)
{
	unsigned char i = 0;
	unsigned int iobase = dev->p_dev->resource[0]->start;


	*value = 0x7fffff;
	while (measurmentIsReady(dev) == 0)
	{
		msleep(1);
		i++;
		if (i == NI4050_MEASURE_READY_TIMEOUT_MS) 
			return -1;
	}


	*value = 0;
	for (i = 0; i<3; i++)
	{
		*value += (xinb(iobase + NI4050_ADC_DATA1_REG + i) << (8*i));
	}
	pr_debug ("Measurement raw value: %06x after %d ms\n", (*value) & 0xffffff, i);

	return 0;
};

// Convert binary measurement to scaled engineering value
int convertMeasureValue(struct ni4050_dev *dev, int value, double *converted)
{
	double scaleValue = (double)(((double)value / 0x7fffff) - 1);

	switch (dev->measurmentMode)
	{
	// DC Volt Ranges
	case NI4050_RANGE_250VDC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_250VDC;
		break;
	case NI4050_RANGE_25VDC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_25VDC;
		break;
	case NI4050_RANGE_2VDC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_2VDC;
		break;
	case NI4050_RANGE_200mVDC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_200mVDC;
		break;
	case NI4050_RANGE_20mVDC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_20mVDC;
		break;

	// AC Volt Ranges
	case NI4050_RANGE_250VAC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_250VAC;
		break;
	case NI4050_RANGE_25VAC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_25VAC;
		break;
	case NI4050_RANGE_2VAC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_2VAC;
		break;
	case NI4050_RANGE_200mVAC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_200mVAC;
		break;
	case NI4050_RANGE_20mVAC:
		*converted = scaleValue * NI4050_CONVERT_RANGE_20mVAC;
		break;
		
	// Ohm Ranges
	case NI4050_RANGE_EXTOHM:
		*converted = scaleValue * NI4050_CONVERT_RANGE_2MOHM * dev->dIntResistorValue /
				(dev->dIntResistorValue - (scaleValue * NI4050_CONVERT_RANGE_2MOHM));
		break;
	case NI4050_RANGE_2MOHM:
		*converted = scaleValue * NI4050_CONVERT_RANGE_2MOHM;
		break;
	case NI4050_RANGE_200kOHM:
		*converted = scaleValue * NI4050_CONVERT_RANGE_200kOHM;
		break;
	case NI4050_RANGE_20kOHM:
		*converted = scaleValue * NI4050_CONVERT_RANGE_20kOHM;
		break;
	case NI4050_RANGE_2kOHM:
		*converted = scaleValue * NI4050_CONVERT_RANGE_2kOHM;
		break;
	case NI4050_RANGE_200OHM:
		*converted = scaleValue * NI4050_CONVERT_RANGE_200OHM;
		break;

	// Diode
	case NI4050_RANGE_DIODE:
		*converted = scaleValue * NI4050_CONVERT_RANGE_DIODE;
		break;
	default:
		*converted = 0x7fffffff;
		return -1;
	}
	pr_debug("Conversion done, raw value: %d\n", value);
	return 0;
};


int startMeasurment(struct ni4050_dev *dev, NI4050_RANGES measurementMode)
{
	unsigned int iobase = dev->p_dev->resource[0]->start;
	unsigned int EEPROMAddress = 0;
	unsigned int i = 0;
	unsigned char tmp;

	pr_debug("-> startMeasurment mode: %d\n", measurementMode);

	dev->measurmentMode = measurementMode;

	dev->ZeroScaleCalCoeff = 0;
	dev->FullScaleCalCoeff = 0;

	do {
		if (measurmentInfo[i].range == measurementMode)
		{
			pr_debug("startMeasurment mode found: %d\n", i);

			if (eepromReadResistance(dev))
			{
				return -1;
			}

			// Reset registers to known state
			pr_debug("// Reset registers to known state\n");
			xoutb(0x00, iobase + NI4050_COMMAND_REG);
			xoutb(NI4050_ADC_COMMAND_DEFAULT, iobase + NI4050_ADC_COMMAND_REG);
			xoutb(0x00, iobase + NI4050_ADC_WRITE_REG);
			xoutb(0x00, iobase + NI4050_CONFIG_REG);

			// Reset board
			xoutb(NI4050_ADC_COMMAND_RESET, iobase + NI4050_ADC_COMMAND_REG);

			// Read calibration constants
			EEPROMAddress = NI4050_EEPROM_AREA_LOAD + measurmentInfo[i].calConstantOffset + NI4050_EEPROM_CAL_ZERO;
			dev->ZeroScaleCalCoeff = readEEPROMWord(dev, EEPROMAddress);
			pr_debug("Zero scale coeff: %d\n", dev->ZeroScaleCalCoeff);

			EEPROMAddress = NI4050_EEPROM_AREA_LOAD + measurmentInfo[i].calConstantOffset + NI4050_EEPROM_CAL_FULL;
			dev->FullScaleCalCoeff = readEEPROMWord(dev, EEPROMAddress);
			pr_debug("Full scale coeff: %d\n", dev->FullScaleCalCoeff);

			// Set Config Register
			if (waitForAdcReady(dev))
				return -1;
			pr_debug("// Set Config Register\n");
			tmp =   measurmentInfo[i].inputRange |
					measurmentInfo[i].ohmsMode |
					measurmentInfo[i].acRange |
					measurmentInfo[i].ohmsRange;
			xoutb(tmp, iobase + NI4050_CONFIG_REG); // flush

			// Set ADC Mode
			if (waitForAdcReady(dev))
				return -1;
			pr_debug("// Set ADC mode\n");
			tmp =  measurmentInfo[i].measurmentMode;
			tmp |= NI4050_ADC_COMMAND_REGSEL_MODEREG; // setRegisterSelect: Mode register
			tmp |= NI4050_ADC_COMMAND_DEFAULT;
			xoutb(tmp, iobase + NI4050_ADC_COMMAND_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			tmp =   1; // Reset filter
			tmp |=  measurmentInfo[i].gain;
			xoutb(tmp, iobase + NI4050_ADC_WRITE_REG); // flush


			// Set Filter Frequency
			if (waitForAdcReady(dev))
				return -1;
			pr_debug("// Set Filter Frequency\n");
			xoutb(NI4050_ADC_COMMAND_REGSEL_FILTERHIGH | NI4050_ADC_COMMAND_DEFAULT | NI4050_ADC_WRITE_FSYNCH,
				  iobase + NI4050_ADC_COMMAND_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb(measurmentInfo[i].filterValueH, iobase + NI4050_ADC_WRITE_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb(NI4050_ADC_COMMAND_REGSEL_FILTERLOW | NI4050_ADC_COMMAND_DEFAULT | NI4050_ADC_WRITE_FSYNCH,
				  iobase + NI4050_ADC_COMMAND_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb(measurmentInfo[i].filterValueL, iobase + NI4050_ADC_WRITE_REG); // flush

			// Set Calibration Coefficients
			pr_debug("// Set Calibration Coefficients\n");
			// Zero Scale
			if (waitForAdcReady(dev))
				return -1;
			pr_debug("// Zero Scale\n");
			xoutb(NI4050_ADC_COMMAND_REGSEL_ZEROCALIB | NI4050_ADC_COMMAND_DEFAULT | NI4050_ADC_WRITE_FSYNCH,
				  iobase + NI4050_ADC_COMMAND_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb((unsigned char)(dev->ZeroScaleCalCoeff >> 16), iobase + NI4050_ADC_WRITE_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb((unsigned char)(dev->ZeroScaleCalCoeff >> 8), iobase + NI4050_ADC_WRITE_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb((unsigned char)(dev->ZeroScaleCalCoeff), iobase + NI4050_ADC_WRITE_REG); // flush

			// Full Scale
			if (waitForAdcReady(dev))
				return -1;
			pr_debug("// Full Scale\n");
			xoutb(NI4050_ADC_COMMAND_REGSEL_FULLCALIB | NI4050_ADC_COMMAND_DEFAULT | NI4050_ADC_WRITE_FSYNCH,
				  iobase + NI4050_ADC_COMMAND_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb((unsigned char)(dev->FullScaleCalCoeff >> 16), iobase + NI4050_ADC_WRITE_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb((unsigned char)(dev->FullScaleCalCoeff >> 8), iobase + NI4050_ADC_WRITE_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			xoutb((unsigned char)(dev->FullScaleCalCoeff), iobase + NI4050_ADC_WRITE_REG); // flush

			// Set Mode and Start Modulator/Filter
			if (waitForAdcReady(dev))
				return -1;
			pr_debug("// Set Mode and Start Modulator/Filter\n");
			tmp = measurmentInfo[i].measurmentMode |
				NI4050_ADC_COMMAND_REGSEL_MODEREG |
				NI4050_ADC_COMMAND_DEFAULT | 
				NI4050_ADC_WRITE_FSYNCH;
			xoutb(tmp, iobase + NI4050_ADC_COMMAND_REG); // flush

			if (waitForAdcReady(dev))
				return -1;
			tmp = measurmentInfo[i].gain;
			xoutb(tmp, iobase + NI4050_ADC_WRITE_REG); // flush

			// Set card to read
			if (waitForAdcReady(dev))
				return -1;
			pr_debug("// Set card to read\n");
			tmp = measurmentInfo[i].measurmentMode |
				NI4050_ADC_COMMAND_REGSEL_DATAREG |
				NI4050_ADC_COMMAND_READ |
				NI4050_ADC_COMMAND_DEFAULT |
			 	NI4050_ADC_WRITE_FSYNCH;
			xoutb(tmp, iobase + NI4050_ADC_COMMAND_REG); // flush

			pr_debug("// Measurement started\n");
			return 0;
		}
		i++;
	} while (measurmentInfo[i].range != NI4050_RANGE_INVALID);

	if (measurmentInfo[i].range == NI4050_RANGE_INVALID)
	{
		pr_debug("Measurement mode: %d is not yet supported\n", measurementMode);
		return -1;
	}

	return -1;
}

static long ni4050_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct ni4050_dev *dev = filp->private_data;
	struct inode *inode = filp->f_path.dentry->d_inode;
	struct pcmcia_device *link;
	int size;
	int rc;
	void __user *argp = (void __user *)arg;
	unsigned int *dIntResistorValue;
	EEPROMInfo *eepromInfo;
	NI4050_RANGES *range;
	double *argDouble;
	int value = 0;

	mutex_lock(&ni4050_mutex);
	rc = -ENODEV;
	link = dev_table[iminor(inode)];
	if (!pcmcia_dev_present(link)) {
		pr_debug("DEV_OK false\n");
		goto out;
	}

	rc = -EINVAL;

	size = _IOC_SIZE(cmd);
	rc = -EFAULT;
	pr_debug("iocdir=%.4x iocr=%.4x iocw=%.4x iocsize=%d cmd=%.4x\n",
		   _IOC_DIR(cmd), _IOC_READ, _IOC_WRITE, size, cmd);

	if (_IOC_DIR(cmd) & _IOC_READ) {
		if (!access_ok(VERIFY_WRITE, argp, size))
			goto out;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (!access_ok(VERIFY_READ, argp, size))
			goto out;
	}

	rc = 0;

	switch (cmd) {
	case NIDMM_IOCEEPROMREAD:
		eepromInfo = (EEPROMInfo*) arg;
		eepromInfo->data = readEEPROM(dev, eepromInfo->address);
		break;
	case NIDMM_IOCEEPROMWRITE:
		rc = -1;
		break;
	case NIDMM_IOCEEPROMREADINTRES:
		dIntResistorValue = (unsigned int*) arg;
		*dIntResistorValue = dev->dIntResistorValue;
		break;
	case NIDMM_IOCSTARTMEASUREMENT:
		range = (NI4050_RANGES *)arg;
		startMeasurment(dev, *range);
		break;
	case NIDMM_IOCREADDATA:
		argDouble = (double *)arg;
		if (measurmentDataRead(dev, &value) == -1)
		{
			rc = -1;
			goto out;
		}
		convertMeasureValue(dev, value, argDouble);
		break;
	default:
		pr_debug("... in default (unknown IOCTL code)\n");
		rc = -ENOTTY;
	}
out:
	mutex_unlock(&ni4050_mutex);
	return rc;
}

static int ni4050_open(struct inode *inode, struct file *filp)
{
	struct ni4050_dev *dev;
	struct pcmcia_device *link;
	int minor = iminor(inode);
	int ret;

	pr_debug("-> ni4050_open\n");
	if (minor >= NI4050_MAX_DEV) {
		pr_debug("-> ni4050 minor: %d > %d\n", minor, NI4050_MAX_DEV);
		return -ENODEV;
	}

	mutex_lock(&ni4050_mutex);
	link = dev_table[minor];

	if (link == NULL || !pcmcia_dev_present(link)) {
		pr_debug("-> ni4050 ENODEV\n");
		ret = -ENODEV;
		goto out;
	}

	if (link->open) {
		pr_debug("-> ni4050 already open\n");
		ret = -EBUSY;
		goto out;
	}

	dev = link->priv;
	filp->private_data = dev;

	pr_debug("-> ni4050_open(device=%d.%d process=%s,%d)\n",
		   imajor(inode), minor, current->comm, current->pid);

	link->open = 1;		/* only one open per device */

	pr_debug("<- ni4050_open\n");
	ret = nonseekable_open(inode, filp);
out:
	mutex_unlock(&ni4050_mutex);
	return ret;
}

static int ni4050_close(struct inode *inode, struct file *filp)
{
	struct ni4050_dev *dev;
	struct pcmcia_device *link;
	int minor = iminor(inode);

	if (minor >= NI4050_MAX_DEV)
		return -ENODEV;

	link = dev_table[minor];
	if (link == NULL)
		return -ENODEV;

	dev = link->priv;

	pr_debug("-> ni4050_close(maj/min=%d.%d)\n", imajor(inode), minor);

	link->open = 0;		/* only one open per device */
	//wake_up(&dev->devq);	/* socket removed? */

	pr_debug("ni4050_close\n");
	return 0;
}

static void ni4050_ni4050_release(struct pcmcia_device * link)
{
	//	struct ni4050_dev *dev = link->priv;

	/* dont terminate the monitor, rather rely on
	 * close doing that for us.
	 */
	pr_debug("-> ni4050_ni4050_release\n");
	while (link->open) {
		pr_debug(KERN_INFO MODULE_NAME ": delaying release until "
			   "process has terminated\n");
		/* note: don't interrupt us:
		 * close the applications which own
		 * the devices _first_ !
		 */
		//wait_event(dev->devq, (link->open == 0));
	}
	/* dev->devq=NULL;	this cannot be zeroed earlier */
	pr_debug("<- ni4050_ni4050_release\n");
	return;
}

/*==== Interface to PCMCIA Layer =======================================*/

static int ni4050_config_check(struct pcmcia_device *p_dev, void *priv_data)
{
	pr_debug("-> ni4050_config_check\n");
	return pcmcia_request_io(p_dev);
}

static int ni4050_config(struct pcmcia_device * link, int devno)
{
	struct ni4050_dev *dev;
	int iobase = -1;

	pr_debug("-> ni4050_config\n");
	link->config_flags |= CONF_AUTO_SET_IO;

	/* read the config-tuples */
	if (pcmcia_loop_config(link, ni4050_config_check, NULL))
		goto cs_release;

	if (pcmcia_enable_device(link))
		goto cs_release;

	dev = link->priv;
	iobase = dev->p_dev->resource[0]->start;

	pr_debug("<- ni4050 iobase: %d\n", iobase);
	eepromReadResistance(dev);
	/*	for (; i<255; i++)
		pr_debug("%03d == %02x\n",i, readEEPROM(dev, i));*/
	pr_debug("<- ni4050_config OK\n");
	return 0;

cs_release:
	pr_debug("<- ni4050_config NODEV\n");
	ni4050_release(link);
	return -ENODEV;
}

static int ni4050_suspend(struct pcmcia_device *link)
{
	struct ni4050_dev *dev;
	pr_debug("-> ni4050_suspend\n");
	dev = link->priv;

	return 0;
}

static int ni4050_resume(struct pcmcia_device *link)
{
	struct ni4050_dev *dev;
	pr_debug("-> ni4050_resume\n");
	dev = link->priv;

	return 0;
}

static void ni4050_release(struct pcmcia_device *link)
{
	ni4050_ni4050_release(link);	/* delay release until device closed */
	pcmcia_disable_device(link);
}

static int ni4050_probe(struct pcmcia_device *link)
{
	struct ni4050_dev *dev;
	int i, ret;

	for (i = 0; i < NI4050_MAX_DEV; i++)
		if (dev_table[i] == NULL)
			break;

	if (i == NI4050_MAX_DEV) {
		pr_debug(KERN_NOTICE MODULE_NAME ": all devices in use\n");
		return -ENODEV;
	}

	/* create a new ni4050_cs device */
	dev = kzalloc(sizeof(struct ni4050_dev), GFP_KERNEL);
	if (dev == NULL)
		return -ENOMEM;

	dev->p_dev = link;
	link->priv = dev;
	dev_table[i] = link;

	/*	init_waitqueue_head(&dev->devq);
	init_waitqueue_head(&dev->ioq);
	init_waitqueue_head(&dev->atrq);
	init_waitqueue_head(&dev->readq);*/

	ret = ni4050_config(link, i);
	if (ret) {
		dev_table[i] = NULL;
		kfree(dev);
		return ret;
	}

	device_create(ni4050_class, NULL, MKDEV(major, i), NULL, "nidmm%d", i);
	pr_debug("<- ni4050_probe OK\n");
	return 0;
}

static void ni4050_detach(struct pcmcia_device *link)
{
	struct ni4050_dev *dev = link->priv;
	int devno;
	pr_debug("-> ni4050_detach\n");

	/*if (link->open)
		ni4050_close();*/

	/* find device */
	for (devno = 0; devno < NI4050_MAX_DEV; devno++)
		if (dev_table[devno] == link)
			break;

	if (devno == NI4050_MAX_DEV)
		return;

	ni4050_release(link);

	dev_table[devno] = NULL;
	kfree(dev);

	device_destroy(ni4050_class, MKDEV(major, devno));

	return;
}

static const struct file_operations ni4050_fops = {
	.owner	= THIS_MODULE,
	.unlocked_ioctl	= ni4050_ioctl,
	.open	= ni4050_open,
	.release= ni4050_close,
};

static struct pcmcia_device_id ni4050_ids[] = {
	PCMCIA_DEVICE_MANF_CARD(0x010b, 0x011a),
	//PCMCIA_DEVICE_PROD_ID12("National Instruments", "DAQCard-4050", 0x2FB368CA, 0xA2BD8C39),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, ni4050_ids);

static struct pcmcia_driver ni4050_driver = {
	.owner	  = THIS_MODULE,
	.name	  = "ni4050",
	.probe	= ni4050_probe,
	.remove   = ni4050_detach,
	.suspend  = ni4050_suspend,
	.resume   = ni4050_resume,
	.id_table = ni4050_ids,
};

static int __init ni4050_init(void)
{
	int rc;

	ni4050_class = class_create(THIS_MODULE, "ni_4050");
	if (IS_ERR(ni4050_class))
		return PTR_ERR(ni4050_class);

	major = register_chrdev(0, DEVICE_NAME, &ni4050_fops);
	if (major < 0) {
		pr_debug(KERN_WARNING MODULE_NAME
			   ": could not get major number\n");
		class_destroy(ni4050_class);
		return major;
	}

	rc = pcmcia_register_driver(&ni4050_driver);
	if (rc < 0) {
		unregister_chrdev(major, DEVICE_NAME);
		class_destroy(ni4050_class);
		return rc;
	}

	return 0;
}

static void __exit ni4050_exit(void)
{
	pcmcia_unregister_driver(&ni4050_driver);
	unregister_chrdev(major, DEVICE_NAME);
	class_destroy(ni4050_class);
};

module_init(ni4050_init);
module_exit(ni4050_exit);
MODULE_LICENSE("Dual BSD/GPL");
