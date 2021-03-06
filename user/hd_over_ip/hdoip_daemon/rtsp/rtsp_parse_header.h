/*
 * rtsp_parse_request.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_PARSE_REQUEST_H_
#define RTSP_PARSE_REQUEST_H_

#include "rtsp.h"
#include "rtsp_connection.h"
#include "map.h"

int rtsp_parse_ip(char* str, uint32_t* p);
int rtsp_parse_timing(char* line, t_video_timing* timing);
int rtsp_parse_audio_channel_status(char* line, uint16_t* acs);
int rtsp_parse_traffic_shaping(char* line, int* traffic_shaping);
int rtsp_parse_rtp_format(char* line, t_rtsp_rtp_format* p);
int rtsp_parse_uint32(char* line, uint32_t* p);
int rtsp_parse_transport(char* line, t_rtsp_transport* p);
int rtsp_parse_transport_protocol(char *line, t_rtsp_transport* p);
int rtsp_parse_rtp_info(char* line, t_rtsp_rtp_info* p);
int rtsp_parse_ui32(char* line, uint32_t* p);
int rtsp_parse_str(char* line, char* p);
int rtsp_parse_edid(char* line, t_rtsp_edid *edid);
int rtsp_parse_hdcp(char* line, t_rtsp_hdcp *hdcp);
int rtsp_parse_usb(char* line, t_rtsp_usb *usb);

int rtsp_ommit_message(t_rtsp_connection* con, int timeout);
int rtsp_ommit_header(t_rtsp_connection* con, int timeout);
int rtsp_ommit_body(t_rtsp_connection* con, int timeout, size_t length);
int rtsp_parse_header(t_rtsp_connection* con, const t_map_fnc attr[], void* base, t_rtsp_header_common* common, int timeout);
int rtsp_parse_response(t_rtsp_connection* con, const t_map_fnc attr[], void* base, t_rtsp_header_common* common, int timeout);
int rtsp_parse_request(t_rtsp_connection* con, const t_map_set srv_method[], const t_map_set** method, void* req, t_rtsp_header_common* common);

#endif /* RTSP_PARSE_REQUEST_H_ */
