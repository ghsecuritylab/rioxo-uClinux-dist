/*
 * hoi_drv.c
 *
 *  Created on: 08.10.2010
 *      Author: alda
 */
#include "hoi_drv.h"
#include "vid_const.h"
#include "ext_irq_pio.h"
#include "hoi_msg.h"
#include "wdg_hal.h"
#include "si598.h"
#include "fec_rx_drv.h"

/* base driver initialization
 *
 * @param handle hoi driver handle
 */
void hoi_drv_init(t_hoi* handle)
{
    // TODO proper range in config.h
    handle->p_tx                            = ioremap(BASE_VIDEO_TX,    0xffffffff);
    handle->p_rx                            = ioremap(BASE_VIDEO_RX,    0xffffffff);
    handle->p_i2c_tag_aud                   = ioremap(BASE_I2C_AUD_TAG, 0xffffffff);
    handle->p_i2c_tag_vid                   = ioremap(BASE_I2C_VID_TAG, 0xffffffff);
    handle->p_vio                           = ioremap(BASE_VIO,         0xffffffff);
    handle->p_vsi                           = ioremap(BASE_VSI,         0xffffffff);
    handle->p_vso                           = ioremap(BASE_VSO,         0xffffffff);
    handle->p_aso[AUD_STREAM_NR_EMBEDDED]   = ioremap(BASE_ASO_0,       0xffffffff);
    handle->p_aso[AUD_STREAM_NR_IF_BOARD]   = ioremap(BASE_ASO_1,       0xffffffff);
    handle->p_asi                           = ioremap(BASE_ASI,         0xffffffff);
    handle->p_adv212                        = ioremap(BASE_ADV212,      0xffffffff);
    handle->p_vrp                           = ioremap(BASE_VRP,         0xffffffff);
    handle->p_tmr                           = ioremap(BASE_TIMER,       0xffffffff);
    handle->p_wdg                           = ioremap(BASE_WATCHDOG,    0xffffffff);
    handle->p_hdcp                          = ioremap(BASE_HDCP,        0xffffffff);
    handle->p_irq                           = ioremap(BASE_EXT_IRQ,     0xffffffff);
    handle->p_esi                           = ioremap(BASE_ETI,         0xffffffff);
    handle->p_eso                           = ioremap(BASE_ETO,         0xffffffff);
    handle->p_ver                           = ioremap(BASE_VER,         0xffffffff);
    handle->p_sysid                         = ioremap(BASE_SYSID,       0xffffffff);
    handle->p_led                           = ioremap(BASE_LED,         0xffffffff);
    handle->p_video_mux                     = ioremap(BASE_VIDEO_MUX,   0xffffffff);
    handle->p_spi_tx                        = ioremap(BASE_SPI_TX,      0xffffffff);
    handle->p_spi_rx                        = ioremap(BASE_SPI_RX,      0xffffffff);
    handle->p_fec_tx                        = ioremap(BASE_FEC_TX,      0xffffffff);
    handle->p_fec_rx                        = ioremap(BASE_FEC_RX,      0xffffffff);
    handle->p_fec_ip_tx                     = ioremap(BASE_FEC_IP_TX,   0xffffffff);
    handle->p_fec_ip_rx                     = ioremap(BASE_FEC_IP_RX,   0xffffffff);
    handle->p_fec_memory_interface          = ioremap(BASE_FEC_MEMORY,  0xffffffff);

    // init
    handle->event = queue_init(100);

    // setup components

    // timer init
    tmr_init(handle->p_tmr);

    // init io-expander i2c with 400kHz
    i2c_drv_init(&handle->i2c_tag_aud, handle->p_i2c_tag_aud, 400000);
    i2c_drv_init(&handle->i2c_tag_vid, handle->p_i2c_tag_vid, 400000);
 
    // read video card id
    bdt_drv_read_video_id(&handle->bdt, &handle->i2c_tag_vid);
    // read audio card id
    bdt_drv_read_audio_id(&handle->bdt, handle->p_aso[AUD_STREAM_NR_IF_BOARD]);

    // set video card multiplexer
    bdt_drv_set_video_mux(&handle->bdt, handle->p_video_mux);

    // init hdmi i2c with 400kHz/100kHz
    if (bdt_return_video_device(&handle->bdt) == BDT_ID_HDMI_BOARD) {
        i2c_drv_init(&handle->i2c_tx, handle->p_tx, 100000);
        i2c_drv_init(&handle->i2c_rx, handle->p_rx, 400000);
    }

    // init
    led_drv_init(&handle->led, &handle->i2c_tag_vid, &handle->i2c_tag_aud, handle->p_led, &handle->bdt);
    eti_drv_set_ptr(&handle->eti, handle->p_esi);
    eto_drv_set_ptr(&handle->eto, handle->p_eso);
    vio_drv_init(&handle->vio, handle->p_vio, handle->p_adv212, &handle->si598);
    vsi_drv_init(&handle->vsi, handle->p_vsi);
    vso_drv_init(&handle->vso, handle->p_vso, handle->p_fec_rx);
    asi_drv_init(&handle->asi, handle->p_asi);
    for (int st = 0; st < AUD_STREAM_CNT; st++)
        aso_drv_init(&handle->aso[st], handle->p_aso[st], st);
    vio_drv_setup_osd(&handle->vio, (t_osd_font*)&vid_font_8x13, bdt_return_video_device(&handle->bdt));
    vrp_drv_init(&handle->vrp, &handle->vio, handle->p_vrp);
    stream_sync_init(&handle->sync_slave_0, SIZE_MEANS, SIZE_RISES, handle->p_esi, handle->p_tmr, AUD_STREAM_NR_EMBEDDED, DEAD_TIME, P_GAIN, I_GAIN, INC_PPM);
    stream_sync_init(&handle->sync_slave_1, SIZE_MEANS, SIZE_RISES, handle->p_esi, handle->p_tmr, AUD_STREAM_NR_IF_BOARD, DEAD_TIME, P_GAIN, I_GAIN, INC_PPM);

    if (bdt_return_video_device(&handle->bdt) == BDT_ID_SDI8_BOARD) {
        si598_clock_control_init(&handle->si598, &handle->i2c_tag_vid, handle->p_vio);
    }

    hoi_drv_stop(handle);
}

/** partially reset driver
 *
 * @param handle hoi handle
 * @param reset vectore
 */
void hoi_drv_reset(t_hoi* handle, uint32_t rv)
{
    // Stop Video I/O
    if (rv & (DRV_RST_VID_OUT | DRV_RST_VID_IN)) {
        REPORT(INFO, "reset vio");
        vio_drv_reset(&handle->vio, bdt_return_video_device(&handle->bdt));

        // Stop vio clock control (also need on SDI)
        vio_clock_control_reset(&handle->vio);

        // Stop SI598 clock control
        if (bdt_return_video_device(&handle->bdt) == BDT_ID_SDI8_BOARD) {
            si598_clock_control_deactivate(&handle->si598);
        }

    }

    // Deactivate VRP
    if (rv & DRV_RST_VRP) {
        REPORT(INFO, "reset vrp");
        vrp_drv_off(&handle->vrp);
    }

    // Stop Video Output
    if (rv & DRV_RST_VID_OUT) {
        REPORT(INFO, "reset vso/eti");
        // deactivate FEC RX block
        init_fec_rx_ip_video(handle->p_fec_ip_rx, 0, &handle->fec_rx);
        fec_rx_disable_video_out(handle->p_fec_rx);
        vso_drv_stop(&handle->vso, handle->p_fec_rx);
        eti_drv_stop_vid(&handle->eti);
        eti_set_config_video_enc_dis(handle->p_esi);
        vso_drv_flush_buf(&handle->vso);
    }

    // Stop Video Input
    if (rv & DRV_RST_VID_IN) {
        REPORT(INFO, "reset eto/vsi");
        eto_drv_stop_vid(&handle->eto);
        eto_set_config_video_enc_dis(handle->p_eso);
        vsi_drv_stop(&handle->vsi);
    }

    // Stop Audio Output to Audio Board
    if (rv & DRV_RST_AUD_BOARD_OUT) {
        REPORT(INFO, "reset aso/eti (board)");
        // deactivate FEC RX block
        init_fec_rx_ip_audio_board(handle->p_fec_ip_rx, 0, &handle->fec_rx);
        fec_rx_disable_audio_board_out(handle->p_fec_rx); 
        aso_drv_stop(&handle->aso[AUD_STREAM_NR_IF_BOARD]);
        aso_drv_flush_buf(&handle->aso[AUD_STREAM_NR_IF_BOARD]);
        if(!(handle->aso[AUD_STREAM_NR_IF_BOARD].stream_status & ASO_DRV_STATUS_ACTIV)){       //if embedded stream is NOT running
            eti_drv_stop_aud_board(&handle->eti);
            eti_set_config_audio_enc_dis(handle->p_esi);
        }
    }

    // Stop Audio Output to Video Board
    if (rv & DRV_RST_AUD_EMB_OUT) {
        REPORT(INFO, "reset aso/eti (embedded)");
        // deactivate FEC RX block
        init_fec_rx_ip_audio_emb(handle->p_fec_ip_rx, 0, &handle->fec_rx);
        fec_rx_disable_audio_emb_out(handle->p_fec_rx);
        aso_drv_stop(&handle->aso[AUD_STREAM_NR_EMBEDDED]);
        aso_drv_flush_buf(&handle->aso[AUD_STREAM_NR_EMBEDDED]);
        if(!(handle->aso[AUD_STREAM_NR_EMBEDDED].stream_status & ASO_DRV_STATUS_ACTIV)){       //if if_board stream is NOT running
            eti_drv_stop_aud_emb(&handle->eti);
            eti_set_config_audio_enc_dis(handle->p_esi);
        }
    }

    // Stop Audio Input from Audio Board
    if (rv & DRV_RST_AUD_BOARD_IN) {
        REPORT(INFO, "reset eto/asi (board)");
        asi_drv_stop(&handle->asi, AUD_STREAM_NR_IF_BOARD);
        if(!(handle->asi.stream_status[AUD_STREAM_NR_EMBEDDED] & ASI_DRV_STREAM_STATUS_ACTIV)){             //if embedded stream is NOT running
            eto_drv_stop_aud(&handle->eto);
            eto_set_config_audio_enc_dis(handle->p_eso);
            asi_drv_flush_buf(&handle->asi);
            REPORT(INFO, "eto stop");
        }
        else REPORT(INFO, "eto not stop because of audio (embedded) is running");
    }

    // Stop Audio Input from Video Board
    if (rv & DRV_RST_AUD_EMB_IN) {
        REPORT(INFO, "reset eto/asi (embedded)");
        asi_drv_stop(&handle->asi, AUD_STREAM_NR_EMBEDDED);
        if(!(handle->asi.stream_status[AUD_STREAM_NR_IF_BOARD] & ASI_DRV_STREAM_STATUS_ACTIV)){             //if if_board stream is NOT running
            eto_drv_stop_aud(&handle->eto);
            eto_set_config_audio_enc_dis(handle->p_eso);
            asi_drv_flush_buf(&handle->asi);
            REPORT(INFO, "eto stop");
        }
        else REPORT(INFO, "eto not stop because of audio (board) is running");
    }

    // Stop stream sync
    if (rv & DRV_RST_STSYNC) {
        REPORT(INFO, "reset st-sync");
        stream_sync_stop_slave_0(&handle->sync_slave_0);
        stream_sync_stop_slave_1(&handle->sync_slave_1);
    }

    if (rv & DRV_RST_TMR) {
        REPORT(INFO, "reset timer");
        tmr_init(handle->p_tmr);
    }

    REPORT(INFO, "reset done");
}


/** This function force the driver into idle state
 *
 * @param handle hoi handle
 */
void hoi_drv_stop(t_hoi* handle)
{
    hoi_drv_reset(handle, DRV_RST);
}

void hoi_drv_interrupt(t_hoi* handle)
{
    uint32_t irq = HOI_RD32(handle->p_irq, 0);

    irq = irq ^ EXT_IRQ_POLARITY;

    if (handle->drivers & DRV_ADV7441) {
        if (irq & EXT_IRQ_HDMI_RX_INT1) {
            adv7441a_irq1_handler(&handle->adv7441a, handle->event);
        }
        if (irq & EXT_IRQ_HDMI_RX_INT2) {
            adv7441a_irq2_handler(&handle->adv7441a, handle->event);
        }
    }

    if (handle->drivers & DRV_ADV9889) {
        if (irq & EXT_IRQ_HDMI_TX_INT1) {
            adv9889_irq_handler(&handle->adv9889, handle->event);
        }
    }

    if (irq & EXT_IRQ_J2K_CODEC_0) {
        vio_drv_irq_adv212(&handle->vio, 0, handle->event);
    }
    if (irq & EXT_IRQ_J2K_CODEC_1) {
        vio_drv_irq_adv212(&handle->vio, 1, handle->event);
    }
    if (irq & EXT_IRQ_J2K_CODEC_2) {
        vio_drv_irq_adv212(&handle->vio, 2, handle->event);
    }
    if (irq & EXT_IRQ_J2K_CODEC_3) {
        vio_drv_irq_adv212(&handle->vio, 3, handle->event);
    }
}

void hoi_drv_handler(t_hoi* handle)
{
    vio_drv_handler(&handle->vio, handle->event);
    vsi_drv_handler(&handle->vsi, handle->event);
    vso_drv_handler(&handle->vso, handle->event);
    asi_drv_handler(&handle->asi, handle->event);
    for (int st = 0; st < AUD_STREAM_CNT; st++)
        aso_drv_handler(&handle->aso[st], handle->event);
    eto_drv_handler(&handle->eto, handle->event);
    eti_drv_handler(&handle->eti, handle->event);
    bdt_drv_handler(handle->p_video_mux, handle->event);
    led_drv_handler(&handle->led);
    fec_rx_handler(handle, handle->p_fec_ip_rx, handle->p_fec_rx, &handle->fec_rx, handle->event);
    stream_sync_slave_0(&handle->sync_slave_0);
    stream_sync_slave_1(&handle->sync_slave_1);
    
    wdg_reset(handle->p_wdg); //reset watchdog

    if (bdt_return_video_device(&handle->bdt) == BDT_ID_SDI8_BOARD) {
        si598_clock_control_handler(&handle->si598);
    }
    if (bdt_return_video_device(&handle->bdt) == BDT_ID_HDMI_BOARD) {
        vio_clock_control(&handle->vio);
    }
    if (handle->drivers & DRV_ADV9889) {
        adv9889_drv_handler(&handle->adv9889, handle->event);
    }
    if (handle->drivers & DRV_ADV7441) {
        adv7441a_handler(&handle->adv7441a, handle->event);
    }
    if ((handle->drivers & DRV_ADV9889) && (handle->drivers & DRV_ADV7441)) {
        hdcp_drv_handler(&handle->eti, &handle->eto, &handle->adv7441a, &handle->adv9889, &handle->vsi, &handle->vso, &handle->asi, &handle->aso[AUD_STREAM_NR_EMBEDDED], &handle->drivers, handle->event, handle->p_fec_rx); //hdcp handler
    }
    if (handle->drivers & DRV_GS2971) {
        gs2971_handler(&handle->gs2971, handle->event);
    }
    if (handle->drivers & DRV_GS2972) {
        gs2972_handler(&handle->gs2972, handle->event);
    }
}

void hoi_drv_timer(t_hoi* handle)
{
    // interrupt
    hoi_drv_interrupt(handle);

    // handler
    hoi_drv_handler(handle);
}
