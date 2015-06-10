/* Copyright (c) 2008-2011, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/raid/pq.h>
#include <linux/clk.h>
#include <mach/boardid.h>

#include "k3_fb.h"
#include "mipi_reg.h"
#include "mipi_dsi.h"
#include "ldi_reg.h"

#ifdef	CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/string.h>

/* used for debugfs to test LCD */
static struct lcd_debug_cmd g_mipi_lcd_debug_info = {0};
int g_panel_lcd_mode = 1;  /* 1: Command LCD, 0: Video LCD, used to set_dsi_mode() */
int g_mipi_div_debug = 8;
#endif

/* 10MHz */
#define	MAX_TX_ESC_CLK	(6)
#define	DSI_BURST_MODE	DSI_BURST_SYNC_PULSES_1
#define	DEFAULT_MIPI_CLK_RATE 	(19200000)

struct dsi_phy_seq_info dphy_seq_info[] = { {80,   90,   0x00, 0x2, 0x02, 32,  20, 26,  13},
                                            {90,   100,  0x10, 0x2, 0x02, 35,  23, 28,  14},
                                            {100,  110,  0x20, 0x2, 0x02, 32,  22, 26,  13},
                                            {110,  130,  0x01, 0x2, 0x01, 31,  20, 27,  13},
                                            {130,  140,  0x11, 0x2, 0x01, 33,  22, 26,  14},
                                            {140,  150,  0x21, 0x2, 0x01, 33,  21, 26,  14},
                                            {150,  170,  0x02, 0x9, 0x01, 32,  20, 27,  13},
                                            {170,  180,  0x12, 0x9, 0x01, 36,  23, 30,  15},
                                            {180,  200,  0x22, 0x9, 0x01, 40,  22, 33,  15},
                                            {200,  220,  0x03, 0x9, 0x04, 40,  22, 33,  15},
                                            {220,  240,  0x13, 0x9, 0x04, 44,  24, 36,  16},
                                            {240,  250,  0x23, 0x9, 0x04, 48,  24, 38,  17},
                                            {250,  270,  0x04, 0x6, 0x04, 48,  24, 38,  17},
                                            {270,  300,  0x14, 0x6, 0x04, 50,  27, 41,  18},
                                            {300,  330,  0x05, 0x9, 0x04, 56,  28, 45,  18},
                                            {330,  360,  0x15, 0x9, 0x04, 59,  28, 48,  19},
                                            {360,  400,  0x25, 0x9, 0x04, 61,  30, 50,  20},
                                            {400,  450,  0x06, 0x6, 0x04, 67,  31, 55,  21},
                                            {450,  500,  0x16, 0x6, 0x04, 73,  31, 59,  22},
                                            {500,  550,  0x07, 0x6, 0x04, 79,  36, 63,  24},
                                            {550,  600,  0x17, 0x6, 0x04, 83,  37, 68,  25},
                                            {600,  650,  0x08, 0xa, 0x04, 90,  38, 73,  27},
                                            {650,  700,  0x18, 0xa, 0x04, 95,  40, 77,  28},
                                            {700,  750,  0x09, 0xa, 0x04, 102, 40, 84,  28},
                                            {750,  800,  0x19, 0xa, 0x04, 106, 42, 87,  30},
                                            {800,  850,  0x29, 0xa, 0x04, 113, 44, 93,  31},
                                            {850,  900,  0x39, 0xa, 0x04, 118, 47, 98,  32},
                                            {900,  950,  0x0a, 0xb, 0x08, 124, 47, 102, 34},
                                            {950,  1000, 0x1a, 0xb, 0x08, 130, 49, 107, 35},
                                            {1000, 1050, 0x2a, 0xb, 0x08, 135, 51, 111, 37},
                                            {1050, 1100, 0x3a, 0xb, 0x08, 139, 51, 114, 38},
                                            {1100, 1150, 0x0b, 0xb, 0x08, 146, 54, 120, 40},
                                            {1150, 1200, 0x1b, 0xb, 0x08, 153, 57, 125, 41},
                                            {1200, 1250, 0x2b, 0xb, 0x08, 158, 58, 130, 42},
                                            {1250, 1300, 0x3b, 0xb, 0x08, 163, 58, 135, 44},
                                            {1300, 1350, 0x0c, 0xb, 0x08, 168, 60, 140, 45},
                                            {1350, 1400, 0x1c, 0xb, 0x08, 172, 64, 144, 47},
                                            {1400, 1450, 0x2c, 0xb, 0x08, 176, 65, 148, 48},
                                            {1450, 1500, 0x3c, 0xb, 0x08, 181, 66, 153, 50}};

void get_dsi_phy_ctrl(u32 *dsi_bit_clk,
	struct mipi_dsi_phy_ctrl *phy_ctrl)
{
	u32 range = 0;
	u32 m_pll = 0;
	u32 i = 0;
	u32 seq_info_count = 0;
	u32 mod_m_pll;

	BUG_ON(phy_ctrl == NULL);
	BUG_ON(dsi_bit_clk == NULL);

	range = *dsi_bit_clk * 2;

	phy_ctrl->pll_unlocking_filter = 0xFF;

	/* Step 1: Determine PLL Input divider ratio (N)
	 * Refernce frequency is 19.2MHz, so N is set to 2 for easy to get any frequency we want.
	 * M is 240*2*2/19.2 = 50, M must be even number.D-PHY PLL = M/N*DEFAULT_MIPI_CLK_RATE = 480M
	 */

    for (i = 1; i < 4; i++) {

    	phy_ctrl->n_pll = i;

	    /* Step 2: Calculate PLL loop divider ratio (M) M_PLL must be even number. */
    	m_pll = range *  phy_ctrl->n_pll * 10 / (DEFAULT_MIPI_CLK_RATE / 100000);
        mod_m_pll = (range *  phy_ctrl->n_pll * 10) % (DEFAULT_MIPI_CLK_RATE / 100000);

    	if (((m_pll % 2) == 0) && mod_m_pll == 0) break;
    }

    m_pll = ALIGN_UP(m_pll, 2);

    range = ((m_pll * 10 / phy_ctrl->n_pll) * (DEFAULT_MIPI_CLK_RATE / 100000) + 50) / 100;
    *dsi_bit_clk =  range / 2;

    k3fb_logi("d-phy n_pll = %d, m_pll = %d, *dsi_bit_clk = %d\n",phy_ctrl->n_pll, m_pll, *dsi_bit_clk);

	phy_ctrl->m_pll_1 = (m_pll - 1) & 0x1F;
	phy_ctrl->m_pll_2 = ((m_pll - 1) >> 5) | 0x80;

	/* Step 3: Determine CP current and LPF ctrl, lp2hs_time,hs2lp_time, hsfreqrange */
	seq_info_count = sizeof(dphy_seq_info)/ sizeof(struct dsi_phy_seq_info);
	for (i = 0; i < seq_info_count; i++) {
	    if (range > dphy_seq_info[i].min_range && range <= dphy_seq_info[i].max_range ) {
	        phy_ctrl->cp_current     = dphy_seq_info[i].cp_current;
	        phy_ctrl->lpf_ctrl       = dphy_seq_info[i].lpf_ctrl;
	        phy_ctrl->hsfreqrange    = dphy_seq_info[i].hsfreqrange;
	        phy_ctrl->clk_hs2lp_time = dphy_seq_info[i].clk_hs2lp_time;
	        phy_ctrl->clk_lp2hs_time = dphy_seq_info[i].clk_lp2hs_time;
	        phy_ctrl->hs2lp_time     = dphy_seq_info[i].hs2lp_time;
	        phy_ctrl->lp2hs_time     = dphy_seq_info[i].lp2hs_time;

	        break;
	    }
	}

	if (i >= seq_info_count) {
	    k3fb_loge("Unsurport this range(%d)!\n", range);
		return;
	}

	/* Step 4: N and M factors effective*/
	phy_ctrl->factors_effective = 0x30;

	phy_ctrl->lane_byte_clk = *dsi_bit_clk / 4;
	phy_ctrl->clk_division = ((phy_ctrl->lane_byte_clk % MAX_TX_ESC_CLK) > 0) ?
		(phy_ctrl->lane_byte_clk / MAX_TX_ESC_CLK + 1) :
		(phy_ctrl->lane_byte_clk / MAX_TX_ESC_CLK);

	phy_ctrl->burst_mode = DSI_BURST_MODE;
}

void mipi_init(struct k3_fb_data_type *k3fd)
{
	u32 hline_time = 0;
	u32 hsa_time = 0;
	u32 hbp_time = 0;
	u32 pixel_clk = 0;
	bool is_ready = false;
	unsigned long dw_jiffies = 0;
	struct mipi_dsi_phy_ctrl phy_ctrl = {0};
       MIPIDSI_PHY_STATUS_UNION    mipidsi_phy_status;

	BUG_ON(k3fd == NULL);

	get_dsi_phy_ctrl(&(k3fd->panel_info.mipi.dsi_bit_clk), &phy_ctrl);

	/* config TE */
	if (k3fd->panel_info.type == PANEL_MIPI_CMD) {
        set_EDC_DSI_CMD_MOD_CTRL(0x1);
        set_EDC_DSI_TE_CTRL(0x4021);
        set_EDC_DSI_TE_HS_NUM(0x0);
        set_EDC_DSI_TE_HS_WD(0x8002);
        set_EDC_DSI_TE_VS_WD(0x1001);
	}



	/*--------------configuring the DPI packet transmission----------------*/
	/*
	** 1. Global configuration
	** Configure Register PHY_IF_CFG with the correct number of lanes
	** to be used by the controller.
	*/
 	set_MIPIDSI_PHY_IF_CFG_n_lanes(k3fd->panel_info.mipi.lane_nums);
	set_MIPIDSI_PHY_IF_CFG_phy_stop_wait_time(0x30);
      	/*
	** 2. Configure the DPI Interface:
	** This defines how the DPI interface interacts with the controller.
	*/
    set_MIPIDSI_DPI_VCID(k3fd->panel_info.mipi.vc);
    set_MIPIDSI_DPI_COLOR_CODING_dpi_color_coding(k3fd->panel_info.mipi.color_mode);
    set_MIPIDSI_DPI_CFG_POL_hsync_active_low(k3fd->panel_info.ldi.hsync_plr);
    set_MIPIDSI_DPI_CFG_POL_vsync_active_low(k3fd->panel_info.ldi.vsync_plr);
    set_MIPIDSI_DPI_CFG_POL_dataen_active_low(k3fd->panel_info.ldi.data_en_plr);
    set_MIPIDSI_DPI_CFG_POL_shutd_active_low(0);
    set_MIPIDSI_DPI_CFG_POL_colorm_active_low(0);
    if (k3fd->panel_info.bpp == EDC_OUT_RGB_666) {
        set_MIPIDSI_DPI_COLOR_CODING_loosely18_en(1);
    }


	if (k3fd->panel_info.type == PANEL_MIPI_VIDEO) {
        set_MIPIDSI_VID_MODE_CFG_lp_vsa_en(1);
        set_MIPIDSI_VID_MODE_CFG_lp_vbp_en(1);
        set_MIPIDSI_VID_MODE_CFG_lp_vfp_en(1);
        set_MIPIDSI_VID_MODE_CFG_lp_vact_en(1);
        set_MIPIDSI_VID_MODE_CFG_lp_hbp_en(1);
        set_MIPIDSI_VID_MODE_CFG_lp_hfp_en(1);
	}
		

	set_MIPIDSI_VID_MODE_CFG_frame_bta_ack_en(0);
    set_MIPIDSI_VID_MODE_CFG_vid_mode_type(phy_ctrl.burst_mode);
    set_MIPIDSI_VID_PKT_SIZE(k3fd->panel_info.xres);
    	/* for dsi read */
    set_MIPIDSI_PCKHDL_CFG_bta_en(1);

	pixel_clk = k3fd->panel_info.clk_rate / 1000000;
	hsa_time = k3fd->panel_info.ldi.h_pulse_width * phy_ctrl.lane_byte_clk / pixel_clk;
	hbp_time = k3fd->panel_info.ldi.h_back_porch * phy_ctrl.lane_byte_clk / pixel_clk;
    hline_time  = ( k3fd->panel_info.ldi.h_pulse_width
                    + k3fd->panel_info.ldi.h_back_porch
                    + k3fd->panel_info.xres
                    + k3fd->panel_info.ldi.h_front_porch)
                * phy_ctrl.lane_byte_clk / pixel_clk;
    set_MIPIDSI_VID_HSA_TIME(hsa_time);
    set_MIPIDSI_VID_HBP_TIME(hbp_time);
    set_MIPIDSI_VID_HLINE_TIME(hline_time);

    	/*
	** 5. Define the Vertical line configuration:
	*/
    if (k3fd->panel_info.ldi.v_pulse_width > 15){
        k3fd->panel_info.ldi.v_pulse_width = 15;
    }
    set_MIPIDSI_VID_VSA_LINES(k3fd->panel_info.ldi.v_pulse_width);
    set_MIPIDSI_VID_VBP_LINES(k3fd->panel_info.ldi.v_back_porch);
    set_MIPIDSI_VID_VFP_LINES(k3fd->panel_info.ldi.v_front_porch);
    set_MIPIDSI_VID_VACTIVE_LINES(k3fd->panel_info.yres);

	/* Configure core's phy parameters */
    set_MIPIDSI_BTA_TO_CNT_bta_to_cnt(4095);
    set_MIPIDSI_PHY_TMR_CFG_phy_lp2hs_time(phy_ctrl.lp2hs_time);
    set_MIPIDSI_PHY_TMR_CFG_phy_hs2lp_time(phy_ctrl.hs2lp_time);
    set_MIPIDSI_PHY_TMR_LPCLK_CFG_phy_clklp2hs_time(phy_ctrl.clk_lp2hs_time);
    set_MIPIDSI_PHY_TMR_LPCLK_CFG_phy_clkhs2lp_time(phy_ctrl.clk_hs2lp_time);
    set_MIPIDSI_PHY_IF_CFG_phy_stop_wait_time(0x40);

		
    	/*
	** 3. Configure the TX_ESC clock frequency to a frequency lower than 20 MHz
	** that is the maximum allowed frequency for D-PHY ESCAPE mode.
	*/
	if (k3fd->panel_info.type == PANEL_MIPI_CMD) {
		if (k3fd->panel_info.xres >= 1920 || k3fd->panel_info.yres >= 1920) {	
        	set_MIPIDSI_CLKMGR_CFG_tx_esc_clk_division(8);
		}else {
			set_MIPIDSI_CLKMGR_CFG_tx_esc_clk_division(4);
		}
	} else {
        	set_MIPIDSI_CLKMGR_CFG_tx_esc_clk_division( phy_ctrl.clk_division );
	}

     
    /*
	** 4. Define the DPI Horizontal timing configuration:
	**
	** Hsa_time = HSA*(PCLK period/Clk Lane Byte Period);
	** Hbp_time = HBP*(PCLK period/Clk Lane Byte Period);
	** Hline_time = (HSA+HBP+HACT+HFP)*(PCLK period/Clk Lane Byte Period);
	*/

	/*
	** 3. Select the Video Transmission Mode:
	** This defines how the processor requires the video line to be
	** transported through the DSI link.
	*/
    
	if (k3fd->panel_info.type == PANEL_MIPI_CMD) {
	    /* differ from k3, no edpi_en reg for EDPI settings */
        	set_MIPIDSI_EDPI_CMD_SIZE(k3fd->panel_info.xres);
        	set_MIPIDSI_CLKMGR_CFG_to_clk_dividsion(phy_ctrl.lane_byte_clk/1000000);
    	 	set_MIPIDSI_HS_WR_TO_CNT(0x1000100);
	}


#ifdef CONFIG_MACH_HI6620OEM

#else
    	/* Set D-PHY direction0 */
      set_MIPIDSI_PHY_TST_CTRL0(0x00000000);
      set_MIPIDSI_PHY_TST_CTRL1(0x000100B0);
      set_MIPIDSI_PHY_TST_CTRL0(0x00000002);
      set_MIPIDSI_PHY_TST_CTRL0(0x00000000);
      set_MIPIDSI_PHY_TST_CTRL1(0x00000001); /* switch to master */
      set_MIPIDSI_PHY_TST_CTRL0(0x00000002);
      set_MIPIDSI_PHY_TST_CTRL0(0x00000000);
#endif

    /* Write CP current */
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010011 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( phy_ctrl.cp_current );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );

	/* Write LPF Control */
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010012 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( phy_ctrl.lpf_ctrl );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );

    /*Configured N and M factors effective*/
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010019 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( phy_ctrl.factors_effective );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );

	/* Write N Pll */
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010017 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( phy_ctrl.n_pll - 1 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );

	/* Write M Pll part 1 */
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010018 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( phy_ctrl.m_pll_1 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );

	/* Write M Pll part 2 */
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010018 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( phy_ctrl.m_pll_2 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );

	/* Set hsfreqrange */
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010044 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( (phy_ctrl.hsfreqrange << 1) & 0x7E );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );


	/* Set PLL unlocking filter */
      set_MIPIDSI_PHY_RSTZ( 0x00000004 );
      set_MIPIDSI_PHY_RSTZ( 0x00000005 );
      set_MIPIDSI_PHY_RSTZ( 0x00000007 );

	is_ready = false;
	dw_jiffies = jiffies + HZ / 2;
	do {
		mipidsi_phy_status.ul32 = get_MIPIDSI_PHY_STATUS();
		if (0x1 == mipidsi_phy_status.bits.phy_lock) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (!is_ready) {
		k3fb_logi("phylock is not ready.\n");
	}
	  
	is_ready = false;
	dw_jiffies = jiffies + HZ / 2;
	do {
		mipidsi_phy_status.ul32 = get_MIPIDSI_PHY_STATUS();
		if ((0x1 == mipidsi_phy_status.bits.phy_stopstateclklane) &&
		    (0x1 == mipidsi_phy_status.bits.phy_stopstate0lane) &&
		    (0x1 == mipidsi_phy_status.bits.phy_stopstate1lane) &&
		    (0x1 == mipidsi_phy_status.bits.phy_stopstate2lane) &&
		    (0x1 == mipidsi_phy_status.bits.phy_stopstate3lane)) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	if (!is_ready) {
		k3fb_logi("phystopstateclk&datalane is not ready.\n");
	}

}

int mipi_dsi_on(struct platform_device *pdev)
{
	int ret = 0;
	struct k3_fb_data_type *k3fd = NULL;

	BUG_ON(pdev == NULL);

	k3fd = (struct k3_fb_data_type *)platform_get_drvdata(pdev);
	BUG_ON(k3fd == NULL);

	/* set LCD init step before LCD on*/
	k3fd->panel_info.lcd_init_step = LCD_INIT_POWER_ON;
	ret = panel_next_on(pdev);

	/* mipi dphy clock enable */
	ret = clk_enable(k3fd->mipi_dphy0_cfg_clk);
	if (ret != 0) {
		k3fb_loge("failed to enable mipi_dphy_cfg_clk, error=%d!\n", ret);
		return ret;
	}

	ret = clk_enable(k3fd->mipi_dphy0_ref_clk);
	if (ret != 0) {
		k3fb_loge("failed to enable mipi_dphy_ref_clk, error=%d!\n", ret);
		return ret;
	}

	ret = clk_enable(k3fd->dsi_cfg_clk);
	if (ret != 0) {
		k3fb_loge("failed to enable dsi_cfg_clk, error=%d!\n", ret);
		return ret;
	}
	/* modified for b052 bbit end */

	/* dsi pixel on */
	set_reg(k3fd->edc_base + LDI_HDMI_DSI_GT, 0x0, 1, 0);
	/* mipi init */
	mipi_init(k3fd);

	/* modified for b052 bbit begin */
      /* switch to command mode */
      set_MIPIDSI_MODE_CFG(MIPIDSI_COMMAND_MODE);
      set_MIPIDSI_CMD_MODE_CFG_all_en_flag(1);

      /* 禁止向Clock Lane发起HS时钟传输请求 */
      set_MIPIDSI_LPCLK_CTRL_phy_txrequestclkhs(0);

      /* 1. Waking up Core */
      set_MIPIDSI_PWR_UP_shutdownz(1);

      ret = panel_next_on(pdev);
      /* modified for b052 bbit begin */
      /* reset Core */
      set_MIPIDSI_PWR_UP_shutdownz(0);

	if (k3fd->panel_info.type == PANEL_MIPI_VIDEO) {
		/* switch to video mode */
        set_MIPIDSI_MODE_CFG(MIPIDSI_VIDEO_MODE);

#ifdef CONFIG_DEBUG_FS
        /* set to video lcd mode */
        g_panel_lcd_mode = 0;
#endif
	}

	if (k3fd->panel_info.type == PANEL_MIPI_CMD) {
        /* switch to cmd mode */
        set_MIPIDSI_CMD_MODE_CFG_all_en_flag(0);

#ifdef CONFIG_DEBUG_FS
        /* set to command lcd mode */
        g_panel_lcd_mode = 1;
#endif
	}

	 /* enable generate High Speed clock */
       set_MIPIDSI_LPCLK_CTRL_phy_txrequestclkhs(1);
      /* Waking up Core */
	set_MIPIDSI_PWR_UP_shutdownz(0);
	/* power off PLL*/
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010013 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00000040 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );

	/* power on PLL*/
	set_MIPIDSI_PHY_TST_CTRL1( 0x00010013 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PHY_TST_CTRL1( 0x00000070 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000002 );
	set_MIPIDSI_PHY_TST_CTRL0( 0x00000000 );
	set_MIPIDSI_PWR_UP_shutdownz(1);

	if(k3fd->panel_info.type != PANEL_MIPI_CMD){
		msleep(70);
	}
	return ret;
}

int mipi_dsi_off(struct platform_device *pdev)
{
	int ret = 0;
	struct k3_fb_data_type *k3fd = NULL;

	BUG_ON(pdev == NULL);

	k3fd = platform_get_drvdata(pdev);
	BUG_ON(k3fd == NULL);

	ret = panel_next_off(pdev);

#ifdef CONFIG_DEBUG_FS
    if ((g_fb_lowpower_debug_flag & DEBUG_MIPI_LOWPOWER_DISABLE) == DEBUG_MIPI_LOWPOWER_DISABLE) {
        k3fb_logi(" mipi off was disable");
        return 0;
    }

#endif


    /* disable generate High Speed clock */
    set_MIPIDSI_LPCLK_CTRL_phy_txrequestclkhs(0);

    /* shutdown d_phy */
    set_MIPIDSI_PHY_RSTZ(0);

    /*  reset test interface */
    set_MIPIDSI_PHY_TST_CTRL0(1);

    /* reset Core */
    set_MIPIDSI_PWR_UP_shutdownz(0);

    /* switch to command mode */
    set_MIPIDSI_MODE_CFG(MIPIDSI_COMMAND_MODE);
    set_MIPIDSI_CMD_MODE_CFG_all_en_flag(1);


	/* mipi dphy clk gating */
	/* modified for b052 bbit begin */
	clk_disable(k3fd->mipi_dphy0_cfg_clk);
	clk_disable(k3fd->mipi_dphy0_ref_clk);
	clk_disable(k3fd->dsi_cfg_clk);
	/* modified for b052 bbit end */

	return ret;
}

int mipi_dsi_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct k3_fb_data_type *k3fd = NULL;

	BUG_ON(pdev == NULL);
	k3fd = platform_get_drvdata(pdev);
	BUG_ON(k3fd == NULL);

	k3fb_logi("index=%d, enter!\n", k3fd->index);

    /* modified for b052 bbit begin */
	if (!IS_ERR(k3fd->mipi_dphy0_cfg_clk)) {
		clk_put(k3fd->mipi_dphy0_cfg_clk);
	}

	if (!IS_ERR(k3fd->mipi_dphy0_ref_clk)) {
		clk_put(k3fd->mipi_dphy0_ref_clk);
	}
    /* modified for b052 bbit end */
	ret = panel_next_remove(pdev);

	k3fb_logi("index=%d, exit!\n", k3fd->index);

	return ret;
}

int mipi_dsi_set_backlight(struct platform_device *pdev)
{
	return panel_next_set_backlight(pdev);
}

int mipi_dsi_set_timing(struct platform_device *pdev)
{
	int ret = 0;
	struct k3_fb_data_type *k3fd = NULL;
	u32 hline_time = 0;
	u32 hsa_time = 0;
	u32 hbp_time = 0;
	u32 pixel_clk = 0;
	struct mipi_dsi_phy_ctrl phy_ctrl = {0};

	BUG_ON(pdev == NULL);

	k3fd = (struct k3_fb_data_type *)platform_get_drvdata(pdev);
	BUG_ON(k3fd == NULL);

	get_dsi_phy_ctrl(&(k3fd->panel_info.mipi.dsi_bit_clk), &phy_ctrl);

    set_MIPIDSI_VID_PKT_SIZE( k3fd->panel_info.xres );

	pixel_clk = k3fd->panel_info.clk_rate / 1000000;
	hsa_time = k3fd->panel_info.ldi.h_pulse_width * phy_ctrl.lane_byte_clk / pixel_clk;
	hbp_time = k3fd->panel_info.ldi.h_back_porch * phy_ctrl.lane_byte_clk / pixel_clk;
	hline_time = (k3fd->panel_info.ldi.h_pulse_width + k3fd->panel_info.ldi.h_back_porch +
		k3fd->panel_info.xres + k3fd->panel_info.ldi.h_front_porch) *
		phy_ctrl.lane_byte_clk / pixel_clk;
    set_MIPIDSI_VID_HSA_TIME( hsa_time );
    set_MIPIDSI_VID_HBP_TIME( hbp_time );
    set_MIPIDSI_VID_HLINE_TIME( hline_time );

	if (k3fd->panel_info.ldi.v_pulse_width > 15)
		k3fd->panel_info.ldi.v_pulse_width = 15;
    set_MIPIDSI_VID_VSA_LINES( k3fd->panel_info.ldi.v_pulse_width );
    set_MIPIDSI_VID_VBP_LINES( k3fd->panel_info.ldi.v_back_porch );
    set_MIPIDSI_VID_VFP_LINES( k3fd->panel_info.ldi.v_front_porch );
    set_MIPIDSI_VID_VACTIVE_LINES( k3fd->panel_info.yres );

	ret = panel_next_set_timing(pdev);

	return ret;
}

STATIC int mipi_dsi_set_frc(struct platform_device *pdev, int target_fps)
{
	struct k3_fb_data_type *k3fd = NULL;
	u32 hline_time = 0;
	u32 pixel_clk = 0;
	u32 vertical_timing = 0;
	u32 horizontal_timing = 0;
	u32 h_front_porch = 0;
	int ret = 0;

	BUG_ON(pdev == NULL);
	k3fd = (struct k3_fb_data_type *)platform_get_drvdata(pdev);
	BUG_ON(k3fd == NULL);

	/* calculate new HFP based on target_fps */
    vertical_timing     = k3fd->panel_info.yres
                        + k3fd->panel_info.ldi.v_back_porch
                        + k3fd->panel_info.ldi.v_front_porch
                        + k3fd->panel_info.ldi.v_pulse_width;
	horizontal_timing = k3fd->panel_info.clk_rate / (vertical_timing * target_fps);

	/* new HFP*/
	/*
	k3fd->panel_info.ldi.h_front_porch = horizontal_timing - k3fd->panel_info.xres
		-k3fd->panel_info.ldi.h_back_porch - k3fd->panel_info.ldi.h_pulse_width;
	*/
    h_front_porch       = horizontal_timing
                        - k3fd->panel_info.xres
                        - k3fd->panel_info.ldi.h_back_porch
                        - k3fd->panel_info.ldi.h_pulse_width;

	pixel_clk = k3fd->panel_info.clk_rate / 1000000;

    /*update hline_time*/
    hline_time          = ( k3fd->panel_info.ldi.h_pulse_width
                            + k3fd->panel_info.ldi.h_back_porch
                            + k3fd->panel_info.xres
                            + h_front_porch)
                        * (k3fd->panel_info.mipi.dsi_bit_clk / 4) / pixel_clk;

    /* remember current fps*/
	k3fd->panel_info.frame_rate = target_fps;

    /* Reset DSI core */
    set_MIPIDSI_PWR_UP( 0 );

    set_MIPIDSI_VID_HLINE_TIME( hline_time );

	set_LDI_HRZ_CTRL0_hfp(k3fd->edc_base, h_front_porch);

    /* power on DSI core */
    set_MIPIDSI_PWR_UP( 1 );

	return ret;
}

STATIC int mipi_dsi_check_esd(struct platform_device *pdev)
{
	BUG_ON(pdev == NULL);
	return panel_next_check_esd(pdev);
}
#ifdef CONFIG_FB_CHECK_MIPI_TR
static int mipi_dsi_check_mipi_tr(struct platform_device *pdev)
{
	BUG_ON(pdev == NULL);

	return panel_next_check_mipi_tr(pdev);
}
#endif
#ifdef CONFIG_FB_SET_INVERSION
static int mipi_dsi_set_inversion_type(struct platform_device *pdev, unsigned int inversion_mode)
{
	BUG_ON(pdev == NULL);

	return panel_next_set_inversion_type(pdev, inversion_mode);
}
#endif

#ifdef CONFIG_FB_MIPI_DETECT
static int mipi_dsi_mipi_detect(struct platform_device *pdev)
{
	BUG_ON(pdev == NULL);

	return panel_next_mipi_detect(pdev);
}
#endif

int mipi_dsi_probe(struct platform_device *pdev)
{
	struct k3_fb_data_type *k3fd = NULL;
	struct platform_device *ldi_dev = NULL;
	struct k3_fb_panel_data *pdata = NULL;
	int ret = 0;

    pr_info("%s enter succ!\n",__func__);
	k3fd = (struct k3_fb_data_type *)platform_get_drvdata(pdev);
	BUG_ON(k3fd == NULL);

    /* get dsi addr space start address:convert from phyical addr to virtual addr */
    k3fd->dsi_base      = k3fd_reg_base_dsi0;

	/* mipi dphy clock */
	/* modified for b052 bbit begin */
	if (k3fd->index == 0) {
		k3fd->mipi_dphy0_cfg_clk = clk_get(NULL, CLK_MIPI_DPHY0_CFG_NAME);
		k3fd->mipi_dphy0_ref_clk = clk_get(NULL, CLK_MIPI_DPHY0_REF_NAME);
		k3fd->dsi_cfg_clk        = clk_get(NULL, CLK_DSI0_CFG_NAME);
	} else if (k3fd->index == 1) {
		k3fd->mipi_dphy0_cfg_clk = clk_get(NULL, CLK_MIPI_DPHY0_CFG_NAME);
		k3fd->mipi_dphy0_ref_clk = clk_get(NULL, CLK_MIPI_DPHY0_REF_NAME);
		k3fd->dsi_cfg_clk        = clk_get(NULL, CLK_DSI0_CFG_NAME);
	} else {
		k3fb_loge("fb%d not support now!\n", k3fd->index);
		return EINVAL;
	}

	if (IS_ERR(k3fd->mipi_dphy0_cfg_clk)) {
		k3fb_loge("failed to get mipi_dphy0_cfg_clk!\n");
		return PTR_ERR(k3fd->mipi_dphy0_cfg_clk);
	}
	if (IS_ERR(k3fd->mipi_dphy0_ref_clk)) {
		k3fb_loge("failed to get mipi_dphy0_ref_clk!\n");
		return PTR_ERR(k3fd->mipi_dphy0_ref_clk);
	}

	if (IS_ERR(k3fd->dsi_cfg_clk)) {
	    k3fb_loge("failed to get dsi_cfg_clk!\n");
		return PTR_ERR(k3fd->dsi_cfg_clk);
	}

    /* modified for b052 bbit end */

	/* alloc ldi device */
	ldi_dev = platform_device_alloc(DEV_NAME_LDI, pdev->id);
	if (!ldi_dev) {
		k3fb_loge("ldi platform_device_alloc failed!\n");
		return -ENOMEM;
	}

	/* link to the latest pdev */
	k3fd->pdev = ldi_dev;

	/* alloc panel device data */
	if (platform_device_add_data(ldi_dev, pdev->dev.platform_data,
		sizeof(struct k3_fb_panel_data))) {
		k3fb_loge("failed to platform_device_add_data, error=%d!\n", ret);
		platform_device_put(ldi_dev);
		return -ENOMEM;
	}

	/* data chain */
	pdata = (struct k3_fb_panel_data *)ldi_dev->dev.platform_data;
	pdata->on = mipi_dsi_on;
	pdata->off = mipi_dsi_off;
	pdata->remove = mipi_dsi_remove;
	pdata->set_backlight = mipi_dsi_set_backlight;
	pdata->set_timing = mipi_dsi_set_timing;
	pdata->set_frc = mipi_dsi_set_frc;
	pdata->check_esd = mipi_dsi_check_esd;
#ifdef CONFIG_FB_CHECK_MIPI_TR
	pdata->check_mipi_tr = mipi_dsi_check_mipi_tr;
#endif
#ifdef CONFIG_FB_SET_INVERSION
	pdata->set_inversion_type = mipi_dsi_set_inversion_type;
#endif
#ifdef CONFIG_FB_MIPI_DETECT
	pdata->mipi_detect = mipi_dsi_mipi_detect;
#endif
	pdata->next = pdev;

	/* get/set panel info */
	memcpy(&k3fd->panel_info, pdata->panel_info, sizeof(struct k3_panel_info));

	/* set driver data */
	platform_set_drvdata(ldi_dev, k3fd);
	/* register in ldi driver */
	ret = platform_device_add(ldi_dev);
	if (ret) {
		k3fb_loge("failed to platform_device_add, error=%d!\n", ret);
		platform_device_put(ldi_dev);
		return ret;
	}
    pr_info("%s exit succ!\n",__func__);
	return ret;
}

static struct platform_driver this_driver = {
	.probe = mipi_dsi_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = DEV_NAME_MIPIDSI,
		},
};

int __init mipi_dsi_driver_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		k3fb_loge("not able to register the driver, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(mipi_dsi_driver_init);

#ifdef	CONFIG_DEBUG_FS
void debug_lcd_parse_cmd_str(char ** str)
{
    char *cmd_seq = NULL;
    int  cmd_seq_num = 0;

    k3fb_logi("enter succ ! \n");
    memset(&g_mipi_lcd_debug_info, 0, sizeof(g_mipi_lcd_debug_info));

    while (NULL != (cmd_seq = strsep(str, " \\"))) {
        k3fb_logi("cmd_seq: %s, cmd_seq_num: %d \n", cmd_seq, cmd_seq_num);
        if (0 == cmd_seq_num) {
            if (strnicmp(cmd_seq, "GW", 2) == 0) {
                g_mipi_lcd_debug_info.cmd_type = DTYPE_GEN_WRITE;
                g_mipi_lcd_debug_info.ops_type = LCD_DEBUG_OPS_TYPE_WRITE;
            } else if (strnicmp(cmd_seq, "GR", 2) == 0) {
                g_mipi_lcd_debug_info.cmd_type = DTYPE_GEN_READ1;
                g_mipi_lcd_debug_info.ops_type = LCD_DEBUG_OPS_TYPE_READ;
            } else if (strnicmp(cmd_seq, "DW", 2) == 0) {
                g_mipi_lcd_debug_info.cmd_type = DTYPE_DCS_WRITE;
                g_mipi_lcd_debug_info.ops_type = LCD_DEBUG_OPS_TYPE_WRITE;
            } else if (strnicmp(cmd_seq, "DR", 2) == 0) {
                g_mipi_lcd_debug_info.cmd_type = DTYPE_DCS_READ;
                g_mipi_lcd_debug_info.ops_type = LCD_DEBUG_OPS_TYPE_READ;
            }
        } else if (cmd_seq_num == 1) {
            if (strnicmp(cmd_seq, "-1", 2) == 0) {
                g_mipi_lcd_debug_info.lcd_reg_msb = -1;
            } else {
                if(strict_strtol(cmd_seq, 0, &g_mipi_lcd_debug_info.lcd_reg_msb) < 0)
                {
                    k3fb_loge("strict_strtol lcd_reg_msb fail\n");
                }
            }
        } else if (cmd_seq_num == 2) {
             if(strict_strtol(cmd_seq, 0, &g_mipi_lcd_debug_info.lcd_reg_lsb) < 0)
            {
                k3fb_loge("strict_strtol lcd_reg_lsb fail\n");
            }
        } else {
            if (g_mipi_lcd_debug_info.ops_type == LCD_DEBUG_OPS_TYPE_READ) {
                /* the reg para num needed to read  */
                if(strict_strtol(cmd_seq, 0, &g_mipi_lcd_debug_info.para_num) < 0)
                {
                    k3fb_loge("strict_strtol para_num fail\n");
                }

                break;
            } else {
                if (g_mipi_lcd_debug_info.para_num >= DEBUG_LCD_REG_PARA_MAX_NUM) {
                    g_mipi_lcd_debug_info.para_num = DEBUG_LCD_REG_PARA_MAX_NUM;
                    break;
                }

                if (0 == strict_strtol(cmd_seq, 0, &g_mipi_lcd_debug_info.para_array[g_mipi_lcd_debug_info.para_num])) {
                    g_mipi_lcd_debug_info.para_num++;
                    k3fb_logi("g_mipi_lcd_debug_info.para_array[%ld] = %ld \n ",g_mipi_lcd_debug_info.para_num - 1,
                                                                          g_mipi_lcd_debug_info.para_array[g_mipi_lcd_debug_info.para_num - 1] );
                }
                else
                {
                    k3fb_loge("strict_strtol para_array fail\n");
                }
            }
        }

        cmd_seq_num++;
    }

    if (DTYPE_GEN_WRITE == g_mipi_lcd_debug_info.cmd_type) {
        if (1 == g_mipi_lcd_debug_info.para_num) {
            g_mipi_lcd_debug_info.cmd_type = DTYPE_GEN_WRITE1;
        } else if (2 == g_mipi_lcd_debug_info.para_num) {
            g_mipi_lcd_debug_info.cmd_type = DTYPE_GEN_WRITE2;
        } else if (2 < g_mipi_lcd_debug_info.para_num) {
            g_mipi_lcd_debug_info.cmd_type = DTYPE_GEN_LWRITE;
        } else {
            k3fb_loge("generic write must have para! para_num = %ld\n", g_mipi_lcd_debug_info.para_num);
        }
    }

    if (DTYPE_DCS_WRITE == g_mipi_lcd_debug_info.cmd_type) {
        if (1 == g_mipi_lcd_debug_info.para_num) {
            g_mipi_lcd_debug_info.cmd_type = DTYPE_DCS_WRITE1;
        } else if (1 < g_mipi_lcd_debug_info.para_num) {
            g_mipi_lcd_debug_info.cmd_type = DTYPE_DCS_LWRITE;
        } else {
           /* do nothing */
        }
    }

    if (LCD_DEBUG_OPS_TYPE_READ == g_mipi_lcd_debug_info.ops_type) {
        if (0 == g_mipi_lcd_debug_info.para_num) {
            g_mipi_lcd_debug_info.para_num = 1;
        }

        if (g_mipi_lcd_debug_info.para_num > DEBUG_LCD_REG_PARA_MAX_NUM) {
            g_mipi_lcd_debug_info.para_num = DEBUG_LCD_REG_PARA_MAX_NUM;
        }


        if ((g_mipi_lcd_debug_info.para_num > 1)
         && (g_mipi_lcd_debug_info.cmd_type == DTYPE_GEN_READ1)) {
            g_mipi_lcd_debug_info.cmd_type = DTYPE_GEN_READ2;
        }
    }

    k3fb_logi("g_mipi_lcd_debug_info.cmd_type = 0x%x \n"
              " g_mipi_lcd_debug_info.lcd_reg_msb  = 0x%lx \n "
              " g_mipi_lcd_debug_info.lcd_reg_lsb  = 0x%lx \n "
              " g_mipi_lcd_debug_info.ops_type = %d (0:write,1:read) \n"
              " g_mipi_lcd_debug_info.para_num = %ld \n", g_mipi_lcd_debug_info.cmd_type,
                                                        (unsigned long)g_mipi_lcd_debug_info.lcd_reg_msb,
                                                        (unsigned long)g_mipi_lcd_debug_info.lcd_reg_lsb,
                                                        g_mipi_lcd_debug_info.ops_type,
                                                         (unsigned long)g_mipi_lcd_debug_info.para_num);

    return;
}

void debug_lcd_write_lcd_reg(void)
{
    struct dsi_cmd_desc cmd_set;
    char*   cmd_payload = NULL;
    int     i = 0;
    int     start_idx = 0;

    cmd_set.dtype = g_mipi_lcd_debug_info.cmd_type;
    cmd_set.vc    = 0;
    cmd_set.wait  = 0;
    cmd_set.waittype = WAIT_TYPE_MS;
    cmd_set.payload  = NULL;

    /* lcd reg num, 1 or 2 byte */
    cmd_set.dlen = 1;
    if (-1 != g_mipi_lcd_debug_info.lcd_reg_msb) {
        cmd_set.dlen = 2;
    }

    cmd_set.dlen += g_mipi_lcd_debug_info.para_num;

    cmd_payload = kmalloc(cmd_set.dlen, GFP_KERNEL);

    if (cmd_payload == NULL) {
        k3fb_loge("cmd_payload malloc is fail ! \n");
        return;
    }

    cmd_payload[0] = g_mipi_lcd_debug_info.lcd_reg_lsb;

     start_idx = 1;
    if (-1 != g_mipi_lcd_debug_info.lcd_reg_msb) {
        start_idx = 2;
        cmd_payload[1] = g_mipi_lcd_debug_info.lcd_reg_msb;
    }

    for (i = 0; i < g_mipi_lcd_debug_info.para_num; i++) {
        cmd_payload[start_idx + i] = g_mipi_lcd_debug_info.para_array[i];
    }

    cmd_set.payload = cmd_payload;

    mipi_dsi_cmds_tx(&cmd_set, 1);

    k3fb_logi("cmd_set.dtype = 0x%x \n cmd_set.dlen  = %d \n", cmd_set.dtype, cmd_set.dlen);

    for (i = 0; i < cmd_set.dlen; i++) {
        k3fb_logi("cmd_set.payload[%d] = 0x%x \n", i, cmd_set.payload[i]);
    }

    kfree(cmd_payload);
}

void debug_lcd_read_lcd_reg(void)
{
    struct dsi_cmd_desc packet_size_cmd_set;
    volatile int cmd_reg;
    int max_times;
    bool is_timeout = true;
    int delay_count = 0;
    u32 tmp = 0;

    int i = 0;
    int reg_msb = g_mipi_lcd_debug_info.lcd_reg_msb;

    if (-1 == g_mipi_lcd_debug_info.lcd_reg_msb) {
        reg_msb = 0;
    }

    max_times = g_mipi_lcd_debug_info.para_num / 4 + 1;

    k3fb_logi("g_mipi_lcd_debug_info.para_num = %ld \n", g_mipi_lcd_debug_info.para_num);

    packet_size_cmd_set.dtype = DTYPE_MAX_PKTSIZE;
    packet_size_cmd_set.vc    = 0;

    /* video LCD must set the DSI to Command Mode */
    if (g_panel_lcd_mode == 0) {
        set_dsi_mode(CMD_MODE);
        mdelay(100);
    }

    packet_size_cmd_set.dlen = g_mipi_lcd_debug_info.para_num;

    mipi_dsi_max_return_packet_size(&packet_size_cmd_set);


    cmd_reg = (reg_msb << 8 | g_mipi_lcd_debug_info.lcd_reg_lsb) << 8 | g_mipi_lcd_debug_info.cmd_type;

    /* send cmd */
    set_MIPIDSI_GEN_HDR(cmd_reg);

    for (i = 0; i < max_times; i++) {
        /* check command fifo empty */
	while (1) {
		tmp = inp32(k3fd_reg_base_dsi0 + MIPIDSI_CMD_PKT_STATUS_ADDR);
		if (((tmp & 0x00000001) == 0x00000001) || delay_count > 100) {
			is_timeout = (delay_count > 100) ? true : false;
			delay_count = 0;
			break;
		} else {
			udelay(1);
			++delay_count;
		}
	}	
	
	/* check read command finish */
	while (1) {
		tmp = inp32(k3fd_reg_base_dsi0 + MIPIDSI_CMD_PKT_STATUS_ADDR);
		if (((tmp & 0x00000040) != 0x00000040) || delay_count > 100) {
			is_timeout = (delay_count > 100) ? true : false;
			delay_count = 0;
			break;
		} else {
			udelay(1);
			++delay_count;
		}
	}	

	if (false == is_timeout) {
        	g_mipi_lcd_debug_info.para_array[i] = get_MIPIDSI_GEN_PLD_DATA();
        	k3fb_logi("get_MIPIDSI_GEN_PLD_DATA cmd_reg = 0x%x, lcd_reg_val = 0x%lx \n", cmd_reg, (unsigned long)g_mipi_lcd_debug_info.para_array[i]);
	}
    }

    if (g_panel_lcd_mode == 0) {
        set_dsi_mode(HSPD_MODE);
        mdelay(20);
    }

    return;
}
void debug_lcd_send_cmd_set(void)
{

    if (LCD_DEBUG_OPS_TYPE_WRITE == g_mipi_lcd_debug_info.ops_type) {
        debug_lcd_write_lcd_reg();
    }


    if (LCD_DEBUG_OPS_TYPE_READ == g_mipi_lcd_debug_info.ops_type) {
        debug_lcd_read_lcd_reg();
    }

    return;
}

void debug_lcd_build_cmd_to_str(char * buf)
{
    int buf_para_start;
    int i = 0;
    int read_times;

    if (buf == NULL) {
        k3fb_loge("buf is null ! \n");
        return;
    }

    /* Generic or Dcs */
    buf[0] = 'G';
    if ((DTYPE_DCS_WRITE  == g_mipi_lcd_debug_info.cmd_type)
     || (DTYPE_DCS_WRITE1 == g_mipi_lcd_debug_info.cmd_type)
     || (DTYPE_DCS_READ   == g_mipi_lcd_debug_info.cmd_type)
     || (DTYPE_DCS_LWRITE == g_mipi_lcd_debug_info.cmd_type)) {
        buf[0] = 'D';
    }

    /* Write or Read */
    buf[1] = 'R';
    if (LCD_DEBUG_OPS_TYPE_WRITE == g_mipi_lcd_debug_info.ops_type) {
        buf[1] = 'W';
    }

    buf[2] = ' ';

    /* -1 or MSB of LCD_REG */
    if (-1 == g_mipi_lcd_debug_info.lcd_reg_msb) {
        sprintf(buf + 3, "-1 ");
        buf_para_start = 6;
    } else {
        sprintf(buf + 3, "0x%02lx ",  (unsigned long)g_mipi_lcd_debug_info.lcd_reg_msb);
        buf_para_start = 8;
    }

    /* LSB of LCD_REG */
    sprintf(buf + buf_para_start, "0x%02lx ",  (unsigned long)g_mipi_lcd_debug_info.lcd_reg_lsb);

    buf_para_start += 5;

    /* Para array for write LCD */
    if (LCD_DEBUG_OPS_TYPE_WRITE == g_mipi_lcd_debug_info.ops_type) {

        if (g_mipi_lcd_debug_info.para_num > 0) {
            /* Para_num */
            sprintf(buf + buf_para_start, "0x%02lx ",  (unsigned long)g_mipi_lcd_debug_info.para_num);
        }

        for (i = 0; i < g_mipi_lcd_debug_info.para_num; i++) {
            buf_para_start +=  5;

            if ((buf_para_start + 5) > DEBUG_LCD_CMD_STR_MAX_BYTE) {
                k3fb_loge(" buf_para_start + 5 is overflow, buf_para_start + 5:%d", buf_para_start + 5);
                return;
            }

            sprintf(buf + buf_para_start, "0x%02lx ",  (unsigned long)g_mipi_lcd_debug_info.para_array[i]);
        }
    } else {

        read_times = g_mipi_lcd_debug_info.para_num /4 + 1;

        /* Para_num */
        sprintf(buf + buf_para_start, "0x%02lx\n",  (unsigned long)g_mipi_lcd_debug_info.para_num);

        buf_para_start += 5;

        for (i = 0; i < read_times; i++) {
            if ((buf_para_start + 11) > DEBUG_LCD_CMD_STR_MAX_BYTE) {
                k3fb_loge(" buf_para_start + 11 is overflow, buf_para_start + 11:%d\n", buf_para_start + 11);
                return;
            }

            sprintf(buf + buf_para_start, "0x%08lx\n",  (unsigned long)g_mipi_lcd_debug_info.para_array[i]);

            buf_para_start += 11;
        }
    }


    if ((buf_para_start) < DEBUG_LCD_CMD_STR_MAX_BYTE) {
        buf[buf_para_start] = '\0';
    }

    k3fb_logi("buf is %s, buf_para_start = %d \n", buf, buf_para_start);
    return;

}

ssize_t debug_lcd_read_file(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    char buf[DEBUG_LCD_CMD_STR_MAX_BYTE];
    int    len = 0;

    if (g_mipi_lcd_debug_info.cmd_type < DTYPE_GEN_WRITE) {

        k3fb_loge("no cmd was sent to lcd! g_mipi_lcd_debug_info.cmd_type = %d, count =%d  \n", g_mipi_lcd_debug_info.cmd_type, count);
        if (copy_to_user(user_buf, "\0", 1)) {
            k3fb_loge("copy_to_user fail! \n");
            return -EFAULT;
        }

        return 0;
    }

    debug_lcd_build_cmd_to_str(buf);

    len = strlen(buf) + 1;

    k3fb_logi("buf is %s, len = %d \n", buf, len);

    return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

ssize_t debug_lcd_write_file(struct file *filp, const char __user *ubuf, size_t cnt, loff_t *ppos)
{
    char cmd_buf[DEBUG_LCD_CMD_STR_MAX_BYTE] = {0};
    char *cmd_buf_str;


    k3fb_logi("enter succ! \n");

    if (cnt > DEBUG_LCD_CMD_STR_MAX_BYTE) {
        k3fb_loge("input buffer cnt > cmd_buf_Len, cnt:%d,cmd_buf_len:%d \n", cnt, DEBUG_LCD_CMD_STR_MAX_BYTE);
        return -EINVAL;
    }

    if (cnt == DEBUG_LCD_CMD_STR_MAX_BYTE) {
        cnt = DEBUG_LCD_CMD_STR_MAX_BYTE - 1;
    }
    if (copy_from_user(&cmd_buf, ubuf, cnt)) {
        k3fb_loge("copy_from_user fail! \n");
        return -EFAULT;
    }

    cmd_buf[cnt] = 0;

    k3fb_logi(" cmd_buf: %s, len= %d \n", cmd_buf, sizeof(struct lcd_debug_cmd));

    cmd_buf_str = cmd_buf;

    debug_lcd_parse_cmd_str(&cmd_buf_str);

    debug_lcd_send_cmd_set();

    return cnt;
}

static const struct file_operations debug_lcd_cmd_fops = {
    .read    = debug_lcd_read_file,
    .write   = debug_lcd_write_file,
};

int __init mipi_lcd_debug_init(void)
{
    struct dentry *lcd_debugfs_dir = NULL;

    k3fb_logi("enter succ! \n");

    lcd_debugfs_dir = debugfs_create_dir(LCD_DEBUG_DIR_NAME, NULL);

    if (ERR_PTR(-ENODEV) == lcd_debugfs_dir) {
        k3fb_loge("debugfs_create_dir create dir %s fail! \n ", LCD_DEBUG_DIR_NAME);
        return -EINVAL;
    }

    debugfs_create_file(LCD_DEBUG_FILE_NAME, 0664, lcd_debugfs_dir, NULL, &debug_lcd_cmd_fops);
    debugfs_create_u32("mipi_div_debug", 0664, lcd_debugfs_dir, &g_mipi_div_debug);

    return 0;
}

late_initcall(mipi_lcd_debug_init);

#endif




