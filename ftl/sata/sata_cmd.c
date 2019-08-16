// Copyright 2011 INDILINX Co., Ltd.
//
// This file is part of Jasmine.
//
// Jasmine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Jasmine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Jasmine. See the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.

#include "jasmine.h"

void ata_check_power_mode(UINT32 lba, UINT32 sector_count)
{
	UINT32 fis_type = FISTYPE_REGISTER_D2H;
	UINT32 flags = B_IRQ;
	UINT32 status = B_DRDY | BIT4;

	SETREG(SATA_FIS_D2H_0, fis_type | (flags << 8) | (status << 16));
	SETREG(SATA_FIS_D2H_1, GETREG(SATA_FIS_H2D_1));
	SETREG(SATA_FIS_D2H_2, GETREG(SATA_FIS_H2D_2) & 0x00FFFFFF);
	SETREG(SATA_FIS_D2H_3, 0x000000FF);
	SETREG(SATA_FIS_D2H_4, 0);
	SETREG(SATA_FIS_D2H_LEN, 5);

	if ((GETREG(SATA_PHY_STATUS) & 0xF0F) != 0x103)
	{
		return;
	}

	SETREG(SATA_CTRL_2, SEND_NON_DATA_FIS);
}

void ata_flush_cache(UINT32 lba, UINT32 sector_count)
{
	ftl_flush();
	send_status_to_host(0);
}

void ata_read_verify_sectors(UINT32 const lba, UINT32 const sector_count)
{
	send_status_to_host(0);
}

void ata_set_features(UINT32 lba, UINT32 sector_count)
{
	BOOL8 invalid = FALSE;

	switch (GETREG(SATA_FIS_H2D_0) >> 24)
	{
	case FEATURE_ENABLE_WRITE_CACHE:
		g_sata_context.write_cache_enabled = TRUE;
		break;
	case FEATURE_SET_TRANSFER_MODE:
		break;
	case FEATURE_ENABLE_USE_OF_SATA:
		if ((sector_count & 0xFF) == 0x02)
		{
			g_sata_context.dma_setup_auto_activate = TRUE;
		}
		break;
	case FEATURE_DISABLE_READ_LOOK_AHEAD:
		g_sata_context.read_look_ahead_enabled = FALSE;
		break;
	case FEATURE_DISABLE_WRITE_CACHE:
		g_sata_context.write_cache_enabled = FALSE;
		break;
	case FEATURE_DISABLE_USE_OF_SATA:
		if ((sector_count & 0xFF) == 0x02)
		{
			g_sata_context.dma_setup_auto_activate = FALSE;
		}
		break;
	case FEATURE_ENABLE_READ_LOOK_AHEAD:
		g_sata_context.read_look_ahead_enabled = TRUE;
		break;

	default:
		invalid = TRUE;
		break;
	}

	if (invalid)
	{
		send_status_to_host(B_ABRT);
	}
	else
	{
		send_status_to_host(0);
	}
}

void ata_seek(UINT32 const lba, UINT32 const sector_count)
{
	if (lba > MAX_LBA)
	{
		send_status_to_host(B_IDNF);
	}
	else
	{
		send_status_to_host(0);
	}
}

void ata_set_multiple_mode(UINT32 lba, UINT32 sector_count)
{
	send_status_to_host(0);
}

void ata_read_buffer(UINT32 lba, UINT32 sector_count)
{
	pio_sector_transfer(HIL_BUF_ADDR, PIO_D2H);
}

void ata_write_buffer(UINT32 lba, UINT32 sector_count)
{
	pio_sector_transfer(HIL_BUF_ADDR, PIO_H2D);
}

void ata_standby(UINT32 lba, UINT32 sector_count)
{
	ftl_flush();

	send_status_to_host(0);
}

void ata_standby_immediate(UINT32 lba, UINT32 sector_count)
{
	ftl_flush();

	send_status_to_host(0);
}

void ata_idle(UINT32 lba, UINT32 sector_count)
{
	ftl_flush();

	send_status_to_host(0);
}

void ata_idle_immediate(UINT32 lba, UINT32 sector_count)
{
	ftl_flush();

	send_status_to_host(0);
}

void ata_sleep(UINT32 lba, UINT32 sector_count)
{
	send_status_to_host(0);
}

void ata_read_native_max_address(UINT32 lba, UINT32 sector_count)
{
	UINT32 fis_type = FISTYPE_REGISTER_D2H;
	UINT32 flags = B_IRQ;
	UINT32 status = B_DRDY | BIT4;

	SETREG(SATA_FIS_D2H_0, fis_type | (flags << 8) | (status << 16));

	UINT32 fis_d1 = GETREG(SATA_FIS_H2D_1);

	if (g_sata_context.slow_cmd.code == ATA_READ_NATIVE_MAX_ADDRESS_EXT)
	{
		SETREG(SATA_FIS_D2H_1, (fis_d1 & 0xFF000000) | (MAX_LBA & 0x00FFFFFF));
		SETREG(SATA_FIS_D2H_2, MAX_LBA >> 24);
	}
	else if ((fis_d1 & BIT30) == 0) // CHS
	{
		SETREG(SATA_FIS_D2H_1, 0x003FFF3F | (((UINT32)CHS_HEADS - 1) << 24));
		SETREG(SATA_FIS_D2H_2, 0);
	}
	else
	{
		SETREG(SATA_FIS_D2H_1, (MAX_LBA & 0x0FFFFFFF) | (fis_d1 & 0xF0000000));
		SETREG(SATA_FIS_D2H_2, 0);
	}

	SETREG(SATA_FIS_D2H_3, GETREG(SATA_FIS_H2D_3) & 0x0000FFFF);
	SETREG(SATA_FIS_D2H_4, 0);
	SETREG(SATA_FIS_D2H_LEN, 5);

	if ((GETREG(SATA_PHY_STATUS) & 0xF0F) != 0x103)
	{
		return;
	}

	SETREG(SATA_CTRL_2, SEND_NON_DATA_FIS);
}

void ata_nop(UINT32 lba, UINT32 sector_count)
{
	send_status_to_host(B_ABRT);
}

void ata_initialize_device_parameters(UINT32 lba, UINT32 sector_count)
{
	if ((sector_count & 0xFF) == 0)
	{
		send_status_to_host(B_ABRT);
	}
	else
	{
		g_sata_context.chs_cur_heads = ((GETREG(SATA_FIS_H2D_1) & 0x0F000000) >> 24) + 1;
		g_sata_context.chs_cur_sectors = sector_count & 0xFF;

#if MAX_LBA >= CHS_MAX_ADDR
		{
			g_sata_context.chs_cur_cylinders = MIN(CHS_MAX_ADDR / (g_sata_context.chs_cur_heads * g_sata_context.chs_cur_sectors), 65535);
		}
#else
		{
			g_sata_context.chs_cur_cylinders = MIN(MAX_LBA / (g_sata_context.chs_cur_heads * g_sata_context.chs_cur_sectors), 65535);
		}
#endif

		send_status_to_host(0);
	}
}

void ata_recalibrate(UINT32 lba, UINT32 sector_count)
{
	send_status_to_host(0);
}

void ata_not_supported(UINT32 lba, UINT32 sector_count)
{
	send_status_to_host(B_ABRT);
}

void send_status_to_host(UINT32 const err_code)
{
	UINT32 fis_type = FISTYPE_REGISTER_D2H;
	UINT32 flags = B_IRQ;
	UINT32 status = (err_code == 0) ? (B_DRDY | BIT4) : (B_DRDY | BIT4 | B_ERR);

	SETREG(SATA_FIS_D2H_0, fis_type | (flags << 8) | (status << 16) | (err_code << 24));
	SETREG(SATA_FIS_D2H_1, GETREG(SATA_FIS_H2D_1));
	SETREG(SATA_FIS_D2H_2, GETREG(SATA_FIS_H2D_2) & 0x00FFFFFF);
	SETREG(SATA_FIS_D2H_3, GETREG(SATA_FIS_H2D_3) & 0x0000FFFF);
	SETREG(SATA_FIS_D2H_4, 0);
	SETREG(SATA_FIS_D2H_LEN, 5);

	if ((GETREG(SATA_PHY_STATUS) & 0xF0F) != 0x103)
	{
		return;
	}

	SETREG(SATA_CTRL_2, SEND_NON_DATA_FIS);
}

void ata_execute_drive_diagnostics(UINT32 lba, UINT32 sector_count)
{
	ata_srst(TRUE, NULL);
}

void ata_srst(UINT32 lba, UINT32 sector_count)
{
	BOOL32 interrupt = (lba != 0);

	SETREG(SATA_FIS_D2H_0, 0x01500000 | (interrupt << 14) | FISTYPE_REGISTER_D2H);
	SETREG(SATA_FIS_D2H_1, 0x00000001);
	SETREG(SATA_FIS_D2H_2, 0x00000000);
	SETREG(SATA_FIS_D2H_3, 0x00000001);
	SETREG(SATA_FIS_D2H_4, 0x00000000);
	SETREG(SATA_FIS_D2H_LEN, 5);

	delay(5000);

	if ((GETREG(SATA_PHY_STATUS) & 0xF0F) != 0x103)
	{
		return;
	}

	SETREG(SATA_CTRL_2, SEND_NON_DATA_FIS);
}

void pio_sector_transfer(UINT32 const dram_addr, UINT32 const protocol)
{
	disable_fiq();

	SETREG(SATA_CTRL_3, BIT3); // switch from Buffer Manager Mode to Manual Mode
	SETREG(SATA_MANUAL_MODE_ADDR, dram_addr - DRAM_BASE);

	UINT32 fis_type = FISTYPE_PIO_SETUP;
	UINT32 flags = B_IRQ;
	UINT32 status = B_DRDY | BIT4 | B_DRQ;
	UINT32 e_status = B_DRDY | BIT4;

	SETREG(SATA_FIS_D2H_0, fis_type | (flags << 8) | (status << 16));
	SETREG(SATA_FIS_D2H_1, GETREG(SATA_FIS_H2D_1));
	SETREG(SATA_FIS_D2H_2, GETREG(SATA_FIS_H2D_2) & 0x00FFFFFF);
	SETREG(SATA_FIS_D2H_3, (e_status << 24) | (GETREG(SATA_FIS_H2D_3) & 0x0000FFFF));
	SETREG(SATA_FIS_D2H_4, BYTES_PER_SECTOR);

	SETREG(SATA_FIS_D2H_LEN, 5);
	SETREG(SATA_XFER_BYTES, BYTES_PER_SECTOR);

	SETREG(SATA_INT_STAT, OPERATION_OK | OPERATION_ERR);

	if (PIO_D2H == protocol)
	{
		SETREG(SATA_CTRL_2, PIO_READ | CLR_STAT_PIO_SETUP);
	}
	else
	{
		SETREG(SATA_CTRL_2, PIO_WRITE);
	}

	while ((GETREG(SATA_INT_STAT) & (OPERATION_OK | OPERATION_ERR)) == 0)
		;

	if (PIO_H2D == protocol)
	{
		// Even though the while() statement above guarantees that SATA protocol is finished,
		// still there is a possibility that internal DMA [FIFO -> SDRAM] is not completed.
		while (GETREG(SATA_FIFO_1_STATUS) & 0x007F0000)
			;
	}

	SETREG(SATA_INT_STAT, OPERATION_OK | OPERATION_ERR);
	SETREG(APB_INT_STS, INTR_SATA);

	enable_fiq();

	// switch back to Buffer Manager Mode
	SETREG(SATA_CTRL_3, 0);

	if (protocol == PIO_H2D)
	{
		send_status_to_host(0);
	}
}

// ogh: S.M.A.R.T
void send_smart_return(UINT32 const err_code)
{
	UINT32 fis_type = FISTYPE_REGISTER_D2H;
	UINT32 flags = B_IRQ;
	UINT32 status = (err_code == 0) ? (B_DRDY | BIT4) : (B_DRDY | BIT4 | B_ERR);

	SETREG(SATA_FIS_D2H_0, fis_type | (flags << 8) | (status << 16) | (err_code << 24));
	SETREG(SATA_FIS_D2H_1, (0xC24F << 8));
	SETREG(SATA_FIS_D2H_2, GETREG(SATA_FIS_H2D_2) & 0x00FFFFFF);
	SETREG(SATA_FIS_D2H_3, GETREG(SATA_FIS_H2D_3) & 0x0000FFFF);
	SETREG(SATA_FIS_D2H_4, 0);
	SETREG(SATA_FIS_D2H_LEN, 5);

	if ((GETREG(SATA_PHY_STATUS) & 0xF0F) != 0x103)
	{
		return;
	}

	SETREG(SATA_CTRL_2, SEND_NON_DATA_FIS);
}

UINT8 jasmine_status[512];
static UINT8 get_checksum(UINT8 *addr)
{
	UINT8 checksum = 0;
	UINT16 i;

	for (i = 0; i < 511; i++)
	{
		checksum += addr[i];
	}

	return -checksum;
}

static void ata_set_defaults(void)
{
	jasmine_status[0] = 0x02; // revision number

	jasmine_status[2] = 0xf1;  // total lbas written (low 7bytes)
	jasmine_status[14] = 0xf3; // total lbas written extended (high 5bytes)

	jasmine_status[26] = 0xf2; // total lbas read (low 7bytes)
	jasmine_status[38] = 0xf4; // total lbas read extended (high 5bytes)

	jasmine_status[50] = 0xf5;  // total gc occurred count
	jasmine_status[62] = 0xf6;  // total erase block count
	jasmine_status[74] = 0xf7;  // total copyback-ed page count
	jasmine_status[86] = 0xf8;  // total hot block count
	jasmine_status[98] = 0xf9;  // total cold block count
	jasmine_status[110] = 0xfA; // total normal block count

	jasmine_status[511] = get_checksum(jasmine_status);
}

static void smart_copy_data(UINT8 *addr, long long val)
{
	int i;
	UINT8 *val_addr = (UINT8 *)&val;
	// smartctl only supports for 6 bytes RAW value
	for (i = 0; i < 6; i++)
	{
		addr[i] = val_addr[i];
	}
}

/** Statistics **/
/** This will be sent through S.M.A.R.T command **/
extern UINT32 g_ftl_num_write;	// # of write sectors
extern UINT32 g_ftl_num_read;	 // # of read sectors
extern UINT32 g_ftl_num_gc;		  // # of gc
extern UINT32 g_ftl_num_erase;	// # of erased blocks
extern UINT32 g_ftl_num_copyback; // # of copy-backed pages

static void ata_set_status(void)
{
	jasmine_status[0] = 0x02; // revision number

	/* default settings */
	jasmine_status[362] = 0x00; // offline data collection status
	jasmine_status[363] = 0x00; // self test execution status
	jasmine_status[367] = 0x00; // offline data collection capability
	jasmine_status[368] = 0x00; // SMART capability
	jasmine_status[369] = 0x00; // SMART capability
	jasmine_status[370] = 0x00; // Error logging capability
	jasmine_status[372] = 0;	// short self-test routine polling time (in minutes)
	jasmine_status[373] = 0;	// Extended self-test polling time (in minutes) 375-376 also used
	jasmine_status[374] = 0;	// conveyance self-test polling time (in minutes)

	/* online updated attributes */
	jasmine_status[4] = BIT1;
	jasmine_status[16] = BIT1;
	jasmine_status[28] = BIT1;
	jasmine_status[40] = BIT1;
	jasmine_status[52] = BIT1;
	jasmine_status[64] = BIT1;
	jasmine_status[76] = BIT1;
	jasmine_status[88] = BIT1;
	jasmine_status[100] = BIT1;
	jasmine_status[112] = BIT1;

	/* real value */
	smart_copy_data(&jasmine_status[7], (0xffffffffffffLL & g_ftl_num_write));
	smart_copy_data(&jasmine_status[31], (0xffffffffffffLL & g_ftl_num_read));
	smart_copy_data(&jasmine_status[55], g_ftl_num_gc);
	smart_copy_data(&jasmine_status[67], g_ftl_num_erase);
	smart_copy_data(&jasmine_status[79], g_ftl_num_copyback);
	smart_copy_data(&jasmine_status[91], 0);
	smart_copy_data(&jasmine_status[103], 0);
	smart_copy_data(&jasmine_status[115], 0);

	jasmine_status[511] = get_checksum(jasmine_status); // checksum

	uart_print("set smart state");
}

void ata_smart(UINT32 const lba, UINT32 const sector_count)
{
	mem_set_sram(jasmine_status, 0x00, sizeof(jasmine_status));
	uart_print("Get smart command");
	uart_print_32(lba);
	if (lba == 0xD0)
	{
		ata_set_defaults();
		ata_set_status();
		mem_copy(HIL_BUF_ADDR, jasmine_status, BYTES_PER_SECTOR);
		pio_sector_transfer(HIL_BUF_ADDR, PIO_D2H);
		//send_status_to_host(0);
	}
	else if (lba == 0xDA)
	{ //get samrt return status
		send_smart_return(0);
	}
	else if (lba == 0xD1)
	{ // get smart attribute threshold
		ata_set_defaults();
		mem_copy(HIL_BUF_ADDR, jasmine_status, BYTES_PER_SECTOR);
		pio_sector_transfer(HIL_BUF_ADDR, PIO_D2H);
		//send_status_to_host(0);
	}
	else
	{
		send_status_to_host(0);
	}
}
