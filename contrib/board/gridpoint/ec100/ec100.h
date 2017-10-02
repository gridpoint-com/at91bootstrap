/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __EC100_H__
#define __EC100_H__

/*
 * PMC Setting
 *
 * The main oscillator is enabled as soon as possible in the c_startup
 * and MCK is switched on the main oscillator.
 * PLL initialization is done later in the hw_init() function
 */

/*
 * From http://www.atmel.com/Images/Atmel-6384-32-bit-ARM926-Embedded-Microprocessor-SAM9G20_Datasheet.pdf
 * 
 * Section 24.4
 * MAINCK = CRYSTAL_HZ = 18432000
 * 
 * Section 24.5 and 25.9.9
 * PLLACK = (CKGR_PLLAR_DIVA ? (MAINCK / CKGR_PLLAR_DIVA * (CKGR_PLLAR_MULA + 1)) : 0)
 * 
 * Section 8.5, 25.2 and 25.9.11
 * PMC_MCKR_CSS = 2 = PLLACK source selected
 * MCK = PLLACK / (1 << PMC_MCKR_PRES) / (PMC_MCKR_MDIV ? (PMC_MCKR_MDIV << 1) : 1)
 * PCK = PLLACK / (1 << PMC_MCKR_PRES) / (PMC_MCKR_PDIV + 1)
 * MASTER_CLOCK = MCK
 * CPU_CLOCK = PCK
 * 
 * Section 40.5.7: PLLA clock frequency range is 400-800 MHz
 * Section 40.11 and 4.2: The maximum speed is 133 MHz on the pin SDCK (SDRAM Clock) loaded with 10 pF
 * Section 40.4:
 *      Max Processor Clock Frequency is 400 MHz
 *      Max Master Clock Frequency is 133 MHz
 * 
 * Original at91sam9g20 bootloader code set frequencies with these values:
 *      CKGR_PLLAR_DIVA = 1
 *      CKGR_PLLAR_MULA = 42
 *      PMC_MCKR_PRES = 0
 *      PMC_MCKR_MDIV = 3
 *      PMC_MCKR_PDIV = 1
 *      PLLACK = 18432000 / 1 * (42+1) = 792576000 = 792.576 MHz
 *      MCK = 792576000 / (1 << 0) / (3 << 1) = 792576000 / 6 = 132096000 = 132.096 MHz
 *      PCK = 792576000 / (1 << 0) / (1 + 1)  = 792576000 / 2 = 396288000 = 396.288 MHz
 * 
 * This will get us closer to the max PLLACK frequency of 800 MHz while also keeping within
 * the max SDCK frequency of 133 MHz:
 *      CKGR_PLLAR_DIVA = 5
 *      CKGR_PLLAR_MULA = 215
 *      PMC_MCKR_PRES = 0
 *      PMC_MCKR_MDIV = 3
 *      PMC_MCKR_PDIV = 1
 *      PLLACK = 18432000 / 5 * (215+1) = 796262400 = 796.2624 MHz
 *      MCK = 796262400 / (1 << 0) / (3 << 1) = 796262400 / 6 = 132710400 = 132.7104 MHz
 *      PCK = 796262400 / (1 << 0) / (1 + 1)  = 796262400 / 2 = 398131200 = 398.1312 MHz
 *
 * Since we seem to be having a problem with SDRC at ~133MHz (132.096 MHz), we will try
 * with SDRC at ~100MHz.
 * With this scenario, we can get even closer to the max PLLACK frequency of 800 MHz with
 * these values:
 *      CKGR_PLLAR_DIVA = 3
 *      CKGR_PLLAR_MULA = 129
 *      PMC_MCKR_PRES = 1
 *      PMC_MCKR_MDIV = 2
 *      PMC_MCKR_PDIV = 0
 *      PLLACK = 18432000 / 3 * (129+1) = 798720000 = 798.720 MHz
 *      MCK = 798720000 / (1 << 1) / (2 << 1) = 798720000 / 2 / 4 =  99840000 =  99.840 MHz
 *      PCK = 798720000 / (1 << 1) / (0 + 1) =  798720000 / 2 / 1 = 399360000 = 399.360 MHz
 */ 
#define CRYSTAL_HZ      18432000
#define MAINCK  CRYSTAL_HZ

#if defined(CONFIG_BUS_SPEED_133MHZ)

#define CKGR_PLLA_DIVA          5
#define CKGR_PLLA_MULA          215
#define PMC_MCKR_PRES           0
#define PMC_MCKR_MDIV           3
#define PMC_MCKR_PDIV           1

#elif defined(CONFIG_BUS_SPEED_100MHZ)

#define CKGR_PLLA_DIVA          3
#define CKGR_PLLA_MULA          129
#define PMC_MCKR_PRES           1
#define PMC_MCKR_MDIV           2
#define PMC_MCKR_PDIV           0

#else
#error "Please define CONFIG_BUS_SPEED_*MHZ"
#endif

#define PLLA_SETTINGS   (0x20003F00 | (CKGR_PLLA_MULA << 16) | CKGR_PLLA_DIVA)
#define MCKR_SETTINGS   ((PMC_MCKR_PDIV << 12) | (PMC_MCKR_MDIV << 8) | (PMC_MCKR_PRES << 2))
#define MCKR_CSS_SETTINGS  (AT91C_PMC_CSS_PLLA_CLK|MCKR_SETTINGS)

#define PLLACK  (CKGR_PLLA_DIVA ? (MAINCK / CKGR_PLLA_DIVA * (CKGR_PLLA_MULA + 1)) : 0)
#define MCK     (PLLACK / (1 << PMC_MCKR_PRES) / (PMC_MCKR_MDIV ? (PMC_MCKR_MDIV << 1) : 1))
#define PCK     (PLLACK / (1 << PMC_MCKR_PRES) / (PMC_MCKR_PDIV + 1))

#define MASTER_CLOCK    MCK
#define CPU_CLOCK       PCK

/*
* DataFlash Settings
*/
#define CONFIG_SYS_SPI_CLOCK	AT91C_SPI_CLK
#define CONFIG_SYS_SPI_MODE	SPI_MODE0

#if defined(CONFIG_SPI_BUS0)
#define CONFIG_SYS_BASE_SPI	AT91C_BASE_SPI0
#elif defined(CONFIG_SPI_BUS1)
#define CONFIG_SYS_BASE_SPI	AT91C_BASE_SPI1
#endif

#if (AT91C_SPI_PCS_DATAFLASH == AT91C_SPI_PCS0_DATAFLASH)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PA(3)
#elif (AT91C_SPI_PCS_DATAFLASH == AT91C_SPI_PCS1_DATAFLASH)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PC(11)
#endif

/*
 * NandFlash Settings
 */
#define CONFIG_SYS_NAND_BASE		AT91C_BASE_CS3
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)

#define CONFIG_SYS_NAND_ENABLE_PIN	AT91C_PIN_PC(14)

/*
 * MCI Settings
 */
#define CONFIG_SYS_BASE_MCI	AT91C_BASE_MCI

/*
 * Recovery
 */
#define CONFIG_SYS_RECOVERY_BUTTON_PIN	AT91C_PIN_PA(31)
#define RECOVERY_BUTTON_NAME	"BP4"

#endif	/* #ifndef __EC100_H__ */
