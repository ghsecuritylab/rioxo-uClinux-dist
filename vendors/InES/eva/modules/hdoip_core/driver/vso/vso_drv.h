/*
 * vso_drv.h
 *
 * Created on: 06.10.2010
 *     Author: stth
 */

#ifndef VSO_DRV_H_
#define VSO_DRV_H_

#include "std.h"
#include "stdvid.h"
#include "vso_hal.h"
#include "vso_drv_cfg.h"
#include "vso_str.h"

int vso_drv_init(t_vso* handle, void* p_vso, void* p_fec_rx);
int vso_drv_set_buf(t_vso* handle, void* start_ptr, size_t size);
int vso_drv_flush_buf(t_vso* handle);
int vso_drv_handler(t_vso* handle, t_queue* event_queue);
int vso_drv_update(t_vso* handle, t_video_timing* vid_timing, uint32_t vs_delay_us, uint32_t vsync_delay_ns, uint32_t scomm5_delay_ns, uint32_t packet_timeout_ns);
int vso_drv_get_delays(t_vso* handle, uint32_t* vs_delay_us, uint32_t* vsync_delay_ns, uint32_t* scomm5_delay_ns);
uint32_t vso_drv_get_packet_timeout(t_vso* handle);
int vso_drv_start(t_vso* handle); 
int vso_drv_stop(t_vso* handle, void* p_fec_rx);
void vso_set_vsync_blanking(t_vso* handle, t_video_timing* timing);

#endif /* VSO_DRV_H_ */
