/*
 * hoi_drv_user.c
 * This function do no range testing!
 *
 *  Created on: 18.10.2010
 *      Author: alda
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "hdoipd.h"
#include "hoi_drv_user.h"
#include "version.h"
#include "edid_merge.h"

#include "usb.h"

#include <netdb.h>
extern int h_errno;


int hoi_msg(void* param)
{
    int ret;
    if ((ret = ioctl(hdoipd.drv, HDOIP_IOCPARAM, param))) {
        perrno("hoi_msg(%d, %08x:%d Byte) failed = %d", hdoipd.drv, *(uint32_t*)param, ((uint32_t*)param)[1], ret);
    }
    return ret;
}


//------------------------------------------------------------------------------
// additional driver load/unload command

int hoi_drv_ldrv(uint32_t drivers)
{
    t_hoi_msg_ldrv msg;

    hoi_msg_ldrv_init(&msg);
    msg.drivers = drivers;

    return hoi_msg(&msg);
}

int hoi_drv_buf(void* at, size_t atl, void* vt, size_t vtl, void* ft, size_t ftl)
{
    t_hoi_msg_buf msg;

    hoi_msg_buf_init(&msg);
    msg.aud_tx_buf = at; msg.aud_tx_len = atl; memset(at, 0, atl);
    msg.vid_tx_buf = vt; msg.vid_tx_len = vtl; memset(vt, 0, vtl);
    msg.fec_rx_buf = ft; msg.fec_rx_len = ftl; memset(ft, 0, ftl);

    return hoi_msg(&msg);
}
//-----------------------------------------------------------------------------
// HDCP
//send hdcp keys to kernel
int hoi_drv_hdcp(uint32_t* hdcp_keys)
{
    t_hoi_msg_hdcp_init msg;

    hoi_msg_hdcp_init(&msg);
    msg.key0 = hdcp_keys[0];
    msg.key1 = hdcp_keys[1];
    msg.key2 = hdcp_keys[2];
    msg.key3 = hdcp_keys[3];
    msg.riv0 = hdcp_keys[4];
    msg.riv1 = hdcp_keys[5];

    return hoi_msg(&msg);
}

//get hdcp status
int hoi_drv_hdcpstat(uint32_t *eto_hdcp_audio,uint32_t *eto_hdcp_video,uint32_t *eti_hdcp_audio, uint32_t *eti_hdcp_video)
{
    int ret;
    t_hoi_msg_hdcpstat msg;

    hoi_msg_hdcpstat_init(&msg);
    ret = hoi_msg(&msg);
    *eto_hdcp_audio = msg.status_eto_audio;
    *eto_hdcp_video = msg.status_eto_video;
    *eti_hdcp_audio = msg.status_eti_audio;
    *eti_hdcp_video = msg.status_eti_video;

    return ret;
}
//-----------------------------------------------------------------------------

int hoi_drv_eti(uint32_t addr_dst, uint32_t addr_src_vid, uint32_t addr_src_aud, uint32_t vid, uint32_t aud_emb, uint32_t aud_board, bool fec_disable)
{
    t_hoi_msg_eti msg;

    hoi_msg_eti_init(&msg);
    msg.ip_address_dst = addr_dst;
    msg.ip_address_src_vid = addr_src_vid;
    msg.ip_address_src_aud = addr_src_aud;
    msg.udp_port_aud_emb = aud_emb;
    msg.udp_port_vid = vid;
    msg.udp_port_aud_board = aud_board;
    msg.fec_disable = fec_disable;

    return hoi_msg(&msg);

}

int hoi_drv_info(t_video_timing* timing, uint32_t *advcnt, uint32_t *advcnt_old)
{
    int ret;
    t_hoi_msg_info msg;

    hoi_msg_info_init(&msg);
    msg.advcnt = 0;
    msg.advcnt_old = 0;
    if (advcnt) msg.advcnt = *advcnt;
    if (advcnt_old) msg.advcnt_old = *advcnt_old;
    ret = hoi_msg(&msg);
    if (advcnt) *advcnt = msg.advcnt;
    if (advcnt_old) *advcnt_old = msg.advcnt_old;
    memcpy(timing, &msg.timing, sizeof(t_video_timing));

    return ret;
}

int hoi_drv_info_all(t_hoi_msg_info** nfo)
{
    int ret;
    static t_hoi_msg_info msg;

    hoi_msg_info_init(&msg);
    msg.advcnt = 0;
    ret = hoi_msg(&msg);
    *nfo = &msg;

    return ret;
}

int hoi_drv_acs(uint16_t* acs)
{
    int ret;
    t_hoi_msg_acs msg;

    memcpy(msg.acs, acs, sizeof(uint16_t)*12);
    hoi_msg_acs_init(&msg);
    ret = hoi_msg(&msg);
    memcpy(acs, msg.acs, sizeof(uint16_t)*12);

    return ret;
}

#define HOI_GET_STAT(T)                         \
    int hoi_drv_##T(t_hoi_msg_##T **stat)       \
    {                                           \
        int ret;                                \
        static t_hoi_msg_##T msg;               \
        hoi_msg_##T##_init(&msg);               \
        ret = hoi_msg(&msg);                    \
        if (stat) *stat = &msg;                 \
        return ret;                             \
    }

HOI_GET_STAT(vsostat);
HOI_GET_STAT(ethstat);
HOI_GET_STAT(fecstat);
HOI_GET_STAT(viostat);
HOI_GET_STAT(asoreg);

//------------------------------------------------------------------------------
// setup/read video format for input/output

#define HOI_WRITE(T, L)                         \
    int hoi_drv_##T(uint32_t p)                 \
    {                                           \
        t_hoi_msg_param msg;                    \
        hoi_msg_write_init(&msg, L);            \
        msg.value = p;                          \
        return hoi_msg(&msg);                   \
    }

#define HOI_READ(T, L)                          \
    int hoi_drv_##T(uint32_t *p)                \
    {                                           \
        int ret;                                \
        t_hoi_msg_param msg;                    \
        hoi_msg_read_init(&msg, L);             \
        ret = hoi_msg(&msg);                    \
        *p = msg.value;                         \
        return ret;                             \
    }

HOI_WRITE(ifmt, HOI_MSG_IFMT);
HOI_WRITE(ofmt, HOI_MSG_OFMT);
HOI_WRITE(pfmt, HOI_MSG_PFMT);
HOI_WRITE(set_mtime, HOI_MSG_SETMTIME);
HOI_WRITE(timer, HOI_MSG_TIMER);
HOI_WRITE(reset, HOI_MSG_OFF);
HOI_WRITE(new_audio, HOI_MSG_NEW_AUDIO);
HOI_WRITE(read_ram, HOI_MSG_DEBUG_READ_RAM);
HOI_WRITE(stsync, HOI_MSG_STSYNC);
HOI_WRITE(set_fps_reduction, HOI_MSG_SET_FPS_REDUCTION);


HOI_READ(get_mtime, HOI_MSG_GETMTIME);
HOI_READ(get_syncdelay, HOI_MSG_SYNCDELAY);
HOI_READ(get_fs, HOI_MSG_GET_FS);
HOI_READ(get_analog_timing, HOI_MSG_GET_ANALOG_TIMING);
HOI_READ(get_video_device_id, HOI_MSG_GET_VID_DEV_ID);
HOI_READ(get_audio_device_id, HOI_MSG_GET_AUD_DEV_ID);
HOI_READ(get_reset_to_default, HOI_MSG_GET_RESET_TO_DEFAULT);
HOI_READ(get_encrypted_status, HOI_MSG_GET_ENCRYPTED_STATUS);
HOI_READ(get_active_resolution, HOI_MSG_GET_ACTIVE_RESOLUTION);

//------------------------------------------------------------------------------

int hoi_drv_set_led_status(uint32_t status)
{
    char *s;
    int ret;
    t_hoi_msg_led_status msg;

    hoi_msg_set_led_status_init(&msg);
    msg.status = status;
    s = reg_get("mode-start");
    if (strcmp(s, "vtb") == 0) {
        msg.vrb = false;
    } else {
        msg.vrb = true;
    }
    ret = hoi_msg(&msg);

    return ret;
}

//------------------------------------------------------------------------------
//

int hoi_drv_set_stime(int unsigned slave_nr, uint32_t stime)
{
    int ret;
    t_hoi_msg_stime msg;

    hoi_msg_set_stime_init(&msg);
    msg.slave_nr = slave_nr;
    msg.stime = stime;
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_get_stime(int unsigned slave_nr, uint32_t *stime)
{
    int ret;
    t_hoi_msg_stime msg;

    hoi_msg_get_stime_init(&msg);
    msg.slave_nr = slave_nr;
    ret = hoi_msg(&msg);
    *stime = msg.stime;

    return ret;
}

//------------------------------------------------------------------------------
// capture/show image command

int hoi_drv_vsi(uint32_t compress, uint32_t chroma, uint32_t encrypt, int bandwidth, hdoip_eth_params* eth, t_video_timing* timing, uint32_t* advcnt, int enable_traffic_shaping, t_fec_setting* fec)
{
    int ret;
    t_hoi_msg_vsi msg;

    hoi_msg_vsi_init(&msg);
    msg.bandwidth = bandwidth;
    msg.chroma = chroma;
    msg.compress = compress;
    msg.encrypt = encrypt;
    memcpy(&msg.fec, fec, sizeof(t_fec_setting));
    msg.enable_traffic_shaping = enable_traffic_shaping;
    if (advcnt) msg.advcnt = *advcnt;
    memcpy(&msg.eth, eth, sizeof(struct hdoip_eth_params));
    ret = hoi_msg(&msg);
    memcpy(timing, &msg.timing, sizeof(t_video_timing));
    if (advcnt) *advcnt = msg.advcnt;

    return ret;
}

int hoi_drv_vso(uint32_t compress, uint32_t encrypt, t_video_timing* timing, uint32_t advcnt, uint32_t delay_ms, int traffic_shaping)
{
    int ret;
    t_hoi_msg_vso msg;

    hoi_msg_vso_init(&msg);
    msg.compress = compress;
    msg.advcnt = advcnt;
    msg.delay_ms = delay_ms;
    msg.encrypt = encrypt;
    memcpy(&msg.timing, timing, sizeof(t_video_timing));
    msg.traffic_shaping = traffic_shaping;
    ret = hoi_msg(&msg);

    return ret;
}


int hoi_drv_asi(int unsigned stream_nr, struct hdoip_eth_params* eth, struct hdoip_aud_params* aud, t_fec_setting* fec)
{
    int ret;
    t_hoi_msg_asi msg;

    hoi_msg_asi_init(&msg);
    msg.stream_nr = stream_nr;
    memcpy(&msg.eth, eth, sizeof(struct hdoip_eth_params));
    memcpy(&msg.aud, aud, sizeof(struct hdoip_aud_params));
    memcpy(&msg.fec, fec, sizeof(t_fec_setting));
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_aic23b_adc(uint32_t enable, uint32_t source, int line_gain, uint32_t mic_boost,
                      struct hdoip_aud_params* aud)
{
    int ret;
    t_hoi_msg_aic23b_adc msg;

    hoi_msg_aic23b_adc_init(&msg);
    msg.enable = enable;
    msg.source = source;
    msg.line_gain = line_gain;
    msg.mic_boost = mic_boost;
    memcpy(&msg.aud, aud, sizeof(struct hdoip_aud_params));
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_aic23b_dac(uint32_t enable, int hp_gain,
    uint32_t fs, uint32_t width, uint16_t ch_map)
{
    int ret;
    t_hoi_msg_aic23b_dac msg;

    hoi_msg_aic23b_dac_init(&msg);
    msg.enable = enable;
    msg.hp_gain = hp_gain;
    msg.aud.ch_map = ch_map;
    msg.aud.fs = fs;
    msg.aud.sample_width = width;
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_aso(int unsigned stream_nr, uint32_t fs, uint32_t width, uint16_t ch_map, uint32_t delay_ms, uint32_t av_delay, uint32_t cfg, uint16_t config)
{
    int ret;
    t_hoi_msg_aso msg;

    hoi_msg_aso_init(&msg);
    msg.stream_nr = stream_nr;
    msg.cfg = cfg;
    msg.config = config;
    msg.aud.ch_map = ch_map;
    msg.aud.fs = fs;
    msg.aud.sample_width = width;
    msg.delay_ms = delay_ms;
    msg.av_delay = av_delay;
    ret = hoi_msg(&msg);

    return ret;
}


int hoi_drv_capture(bool compress, void* buffer, size_t size, t_video_timing* timing, uint32_t* advcnt)
{
    int ret;
    t_hoi_msg_image msg;

    hoi_msg_capture_init(&msg);
    msg.compress = compress;
    msg.buffer = buffer;
    msg.size = size;
    ret = hoi_msg(&msg);
    memcpy(timing, &msg.timing, sizeof(t_video_timing));
    if (compress && advcnt) *advcnt = msg.advcnt;

    return ret;
}

int hoi_drv_show(bool compress, void* buffer, t_video_timing* timing, uint32_t advcnt)
{
    int ret;
    t_hoi_msg_image msg;

    hoi_msg_show_init(&msg);
    msg.compress = compress;
    msg.buffer = buffer;
    memcpy(&msg.timing, timing, sizeof(t_video_timing));
    msg.advcnt = advcnt;
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_bw(uint32_t bw, uint32_t chroma)
{
    t_hoi_msg_bandwidth msg;
    hoi_msg_bandwidth_init(&msg);
    msg.bw = bw;
    msg.chroma = chroma;
    return hoi_msg(&msg);
}

int hoi_drv_debug(void)
{
    int ret=0;

    return ret;
}

int hoi_drv_set_fec_tx_params(t_fec_setting* fec)
{
    int ret;
    t_hoi_msg_fec_tx msg;

    hoi_msg_fec_tx_init(&msg);
    memcpy(&msg.fec, fec, sizeof(t_fec_setting));
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_set_timing(t_video_timing* timing, bool osd_disable)
{
    int ret=0;
    t_hoi_msg_image msg;
    
    memcpy(&msg.timing, timing, sizeof(t_video_timing));

    if reg_test("mode-start", "vtb") {
        msg.vtb = true;
    } else {
        msg.vtb = false;
    }

    msg.osd_disable = osd_disable;

    hoi_msg_set_timing_init(&msg);
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_getversion(t_hoic_getversion* cmd)
{
    int ret;
    t_hoi_msg_version msg;

    hoi_msg_getversion_init(&msg);
    ret = hoi_msg(&msg);
    cmd->fpga_date = msg.fpga_date;
    cmd->fpga_svn = msg.fpga_svn;
    cmd->sysid_date = msg.sysid_date;
    cmd->sysid_id = msg.sysid_id;
    cmd->sw_version = VERSION_SOFTWARE;
    strcpy(cmd->sw_tag, VERSION_TAG);

    return ret;
}

int hoi_drv_getusb(t_hoic_getusb* cmd)
{
    strcpy(cmd->device, "1-1.2"); //TODO: correct and variable device ID

    usb_get_dev(cmd->device);

    return 0;
}

//------------------------------------------------------------------------------
// EDID I/O

int hoi_drv_rdedid(void* buffer)
{
    int ret;
    t_hoi_msg_edid msg;

    hoi_msg_rdedid_init(&msg);
    ret = hoi_msg(&msg);
    memcpy(buffer, msg.edid, 256);

    return ret;
}

int hoi_drv_wredid(void* buffer)
{
    int ret;
    t_hoi_msg_edid msg;

    hoi_msg_wredid_init(&msg);
    memcpy(msg.edid, buffer, 256);
    ret = hoi_msg(&msg);

    return ret;
}


//------------------------------------------------------------------------------
// TAG I/O

int hoi_drv_rdvidtag(void* buffer, bool* available)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_rdvidtag_init(&msg);
    ret = hoi_msg(&msg);
    memcpy(buffer, msg.tag, 256);
    *available = msg.available;

    return ret;
}

int hoi_drv_wrvidtag(void* buffer)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_wrvidtag_init(&msg);
    memcpy(msg.tag, buffer, 256);
    ret = hoi_msg(&msg);

    return ret;
}

int hoi_drv_rdaudtag(void* buffer, bool* available)
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_rdaudtag_init(&msg);
    ret = hoi_msg(&msg);
    memcpy(buffer, msg.tag, 256);
    *available = msg.available;

    return ret;
}

int hoi_drv_wraudtag()
{
    int ret;
    t_hoi_msg_tag msg;

    hoi_msg_wraudtag_init(&msg);
    ret = hoi_msg(&msg);

    return ret;
}

//------------------------------------------------------------------------------
// HDCP
int hoi_drv_hdcp_get_timer(t_hoi_msg_hdcp_timer *msg)
{
    hoi_msg_hdcp_get_timer_init(msg);
    return hoi_msg(msg);
}

int hoi_drv_hdcp_set_timer(uint32_t start_time)
{
    t_hoi_msg_hdcp_timer msg;

    hoi_msg_hdcp_set_timer_init(&msg);
    msg.start_time = start_time;
    return hoi_msg(&msg);
}

int hoi_drv_hdcp_get_key(uint32_t key[4])
{
    int ret;
    t_hoi_msg_hdcp_key msg;

    hoi_msg_hdcp_get_key_init(&msg);
    ret = hoi_msg(&msg);

    key[0] = msg.key[0];
    key[1] = msg.key[1];
    key[2] = msg.key[2];
    key[3] = msg.key[3];

    return ret;
}

//------------------------------------------------------------------------------
// Watch dog
int hoi_drv_wdg_init(uint32_t service_time)
{
    t_hoi_msg_wdg msg;

    hoi_msg_wdg_init_init(&msg);
    msg.service_time = service_time;
    return hoi_msg(&msg);
}

//------------------------------------------------------------------------------
// command

#define HOI_CMDSW(T)                            \
    int hoi_drv_##T()                           \
    {                                           \
        t_hoi_msg msg;                          \
        hoi_msg_##T##_init(&msg);               \
        return hoi_msg(&msg);                   \
    }

HOI_CMDSW(poll);
HOI_CMDSW(loop);
HOI_CMDSW(osdon);
HOI_CMDSW(osdoff);
HOI_CMDSW(osd_clr_border);
HOI_CMDSW(hpdon);
HOI_CMDSW(hpdoff);
HOI_CMDSW(repair);

HOI_CMDSW(hdcp_viden_eti);      //enable hdcp eti video encryption
HOI_CMDSW(hdcp_viden_eto);      //enable hdcp eto video encryption
HOI_CMDSW(hdcp_auden_eti);      //enable hdcp eti audio encryption
HOI_CMDSW(hdcp_auden_eto);      //enable hdcp eto audio encryption
HOI_CMDSW(hdcp_viddis_eti);     //disable hdcp eti video encryption
HOI_CMDSW(hdcp_viddis_eto);     //disable hdcp eto video encryption
HOI_CMDSW(hdcp_auddis_eti);     //disable hdcp eti audio encryption
HOI_CMDSW(hdcp_auddis_eto);     //disable hdcp eto audio encryption
HOI_CMDSW(hdcp_adv9889dis);  	//disable AD9889 hdcp encryption
HOI_CMDSW(hdcp_adv9889en);   	//enable AD9889 hdcp encryption

HOI_CMDSW(wdg_enable);
HOI_CMDSW(wdg_disable);
HOI_CMDSW(wdg_service);
HOI_CMDSW(hdcp_timer_enable);
HOI_CMDSW(hdcp_timer_disable);
HOI_CMDSW(hdcp_timer_load);
HOI_CMDSW(hdcp_black_output);
HOI_CMDSW(clr_osd);
