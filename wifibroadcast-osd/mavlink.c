#include "mavlink.h"
#include <stdio.h>
#include <unistd.h>

#ifdef MAVLINK
mavlink_status_t status;
mavlink_message_t msg;


int mavlink_read(telemetry_data_t *td, uint8_t *buf, int buflen) {
	td->datarx++;
	int i = 0;
	int render_data = 0;
//	fprintf(stdout, "buflen: %d  ",buflen);
//	fprintf(stdout, "mavlink_read datarx: %d\n",td->datarx);
	for(i=0; i<buflen; i++) {
		uint8_t c = buf[i];
		if (mavlink_parse_char(0, c, &msg, &status)) {
                    	td->validmsgsrx++;
			fprintf(stdout, "Msg seq:%d sysid:%d, compid:%d  ", msg.seq, msg.sysid, msg.compid);
                	switch (msg.msgid){
                        	case MAVLINK_MSG_ID_GPS_RAW_INT:
					fprintf(stdout, "GPS_RAW_INT: ");
					td->fix = mavlink_msg_gps_raw_int_get_fix_type(&msg);
					td->sats = mavlink_msg_gps_raw_int_get_satellites_visible(&msg);
					td->cog = mavlink_msg_gps_raw_int_get_cog(&msg)/100.0f;
//					td->heading = mavlink_msg_gps_raw_int_get_cog(&msg)/100.0f;
//                                      td->altitude = mavlink_msg_gps_raw_int_get_alt(&msg)/1000.0f;
//                                      td->latitude = mavlink_msg_gps_raw_int_get_lat(&msg)/10000000.0f;
//                                      td->longitude = mavlink_msg_gps_raw_int_get_lon(&msg)/10000000.0f;
//					fprintf(stdout, "heading:%.2f  ", td->heading);
//					fprintf(stdout, "altitude:%.2f  ", td->altitude);
//					fprintf(stdout, "latitude:%.2f  ", td->latitude);
//					fprintf(stdout, "longitude:%.2f  ", td->longitude);
					fprintf(stdout, "fix:%d  ", td->fix);
					fprintf(stdout, "sats:%d  ", td->sats);
					fprintf(stdout, "cog:%d  ", td->cog);
					break;
                                case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
					fprintf(stdout, "GLOBAL_POSITION_INT: ");
					td->heading = mavlink_msg_global_position_int_get_hdg(&msg)/100.0f;
                                        td->altitude = mavlink_msg_global_position_int_get_relative_alt(&msg)/1000.0f;
                                        td->latitude = mavlink_msg_global_position_int_get_lat(&msg)/10000000.0f;
                                        td->longitude = mavlink_msg_global_position_int_get_lon(&msg)/10000000.0f;
					fprintf(stdout, "heading:%.2f  ", td->heading);
					fprintf(stdout, "altitude:%.2f  ", td->altitude);
					fprintf(stdout, "latitude:%.6f  ", td->latitude);
					fprintf(stdout, "longitude:%.6f  ", td->longitude);
                                        break;
                                case MAVLINK_MSG_ID_ATTITUDE:
					fprintf(stdout, "ATTITUDE: ");
					td->roll = mavlink_msg_attitude_get_roll(&msg)*57.2958;
					td->pitch = mavlink_msg_attitude_get_pitch(&msg)*57.2958;
					fprintf(stdout, "roll:%.2f  ", td->roll);
					fprintf(stdout, "pitch:%.2f  ", td->pitch);
					render_data = 1; // render when we got attitude data (the data that needs to be most up-to-date)
					break;
                                case MAVLINK_MSG_ID_SYS_STATUS:
					fprintf(stdout, "SYS_STATUS: ");
					td->voltage = mavlink_msg_sys_status_get_voltage_battery(&msg)/1000.0f;
					td->ampere = mavlink_msg_sys_status_get_current_battery(&msg)/100.0f;
					fprintf(stdout, "voltage:%.2f  ", td->voltage);
					fprintf(stdout, "ampere:%.2f  ", td->ampere);
                                        break;
                                case MAVLINK_MSG_ID_VFR_HUD:
					fprintf(stdout, "VFR_HUD: ");
                                        td->speed = mavlink_msg_vfr_hud_get_groundspeed(&msg)*3.6f;
                                        td->airspeed = mavlink_msg_vfr_hud_get_airspeed(&msg)*3.6f;
//					td->heading = mavlink_msg_vfr_hud_get_heading(&msg)/100.0f;
					td->mav_climb = mavlink_msg_vfr_hud_get_climb(&msg);
					fprintf(stdout, "speed:%.2f  ", td->speed);
					fprintf(stdout, "airspeed:%.2f  ", td->airspeed);
//					fprintf(stdout, "heading:%.2f  ", td->heading);
					fprintf(stdout, "climb:%f  ", td->mav_climb);
                                        break;
                                case MAVLINK_MSG_ID_GPS_STATUS:
					fprintf(stdout, "GPS_STATUS ");
                                        break;
                                case MAVLINK_MSG_ID_HEARTBEAT:
					fprintf(stdout, "HEARTBEAT ");
					td->mav_flightmode = mavlink_msg_heartbeat_get_custom_mode(&msg);
					if (((mavlink_msg_heartbeat_get_base_mode(&msg) & 0b10000000) >> 7) == 0) {
					    td->armed = 0;
					} else {
					    td->armed = 1;
					}
					fprintf(stdout, "mode:%d  ", td->mav_flightmode);
					fprintf(stdout, "armed:%d  ", td->armed);
                                        break;
                                case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
					fprintf(stdout, "RC_CHANNELS_RAW ");
					td->rssi = mavlink_msg_rc_channels_raw_get_rssi(&msg)*100/255;
					fprintf(stdout, "rssi:%d  ", td->rssi);
                                        break;
                                case MAVLINK_MSG_ID_COMMAND_ACK:
					fprintf(stdout, "COMMAND_ACK:%d ",mavlink_msg_command_ack_get_command(&msg));
                                        break;
                                case MAVLINK_MSG_ID_COMMAND_INT:
					fprintf(stdout, "COMMAND_INT:%d ",mavlink_msg_command_int_get_command(&msg));
                                        break;
                                case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:
					fprintf(stdout, "EXTENDED_SYS_STATE: vtol_state:%d, landed_state:%d",mavlink_msg_extended_sys_state_get_vtol_state(&msg),mavlink_msg_extended_sys_state_get_landed_state(&msg));
                                        break;
/*                                case MAVLINK_MSG_ID_BATTERY_STATUS:
					fprintf(stdout, "BATTERY_STATUS ");
                                        break;
                                case MAVLINK_MSG_ID_ALTITUDE:
					fprintf(stdout, "ALTITUDE ");
                                        break;
                                case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
					fprintf(stdout, "LOCAL_POSITION_NED ");
                                        break;
                                case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:
					fprintf(stdout, "SERVO_OUTPUT_RAW ");
                                        break;
                                case MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
					fprintf(stdout, "POSITION_TARGET_LOCAL_NED ");
                                        break;
                                case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
					fprintf(stdout, "POSITION_TARGET_GLOBAL_INT ");
                                        break;
                                case MAVLINK_MSG_ID_ESTIMATOR_STATUS:
					fprintf(stdout, "ESTIMATOR_STATUS ");
                                        break;
                                case MAVLINK_MSG_ID_WIND_COV:
					fprintf(stdout, "WIND_COV ");
                                        break;
                                case MAVLINK_MSG_ID_VIBRATION:
					fprintf(stdout, "VIBRATION ");
                                        break;
                                case MAVLINK_MSG_ID_HOME_POSITION:
					fprintf(stdout, "HIGHRES_IMU ");
                                        break;
                                case MAVLINK_MSG_ID_NAMED_VALUE_FLOAT:
					fprintf(stdout, "NAMED_VALUE_FLOAT ");
                                        break;
                                case MAVLINK_MSG_ID_NAMED_VALUE_INT:
					fprintf(stdout, "NAMED_VALUE_INT ");
                                        break;
                                case MAVLINK_MSG_ID_PARAM_VALUE:
					fprintf(stdout, "PARAM_VALUE ");
                                        break;
                                case MAVLINK_MSG_ID_PARAM_SET:
					fprintf(stdout, "PARAM_SET ");
                                        break;
                                case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
					fprintf(stdout, "PARAM_REQUEST_READ ");
                                        break;
                                case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
					fprintf(stdout, "PARAM_REQUEST_LIST ");
                                        break;
                                case MAVLINK_MSG_ID_RC_CHANNELS_SCALED:
					fprintf(stdout, "RC_CHANNELS_SCALED ");
                                        break;
                                case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
					fprintf(stdout, "RC_CHANNELS_OVERRIDE ");
                                        break;
                                case MAVLINK_MSG_ID_RC_CHANNELS:
					fprintf(stdout, "RC_CHANNELS ");
                                        break;
                                case MAVLINK_MSG_ID_MANUAL_CONTROL:
					fprintf(stdout, "MANUAL_CONTROL ");
                                        break;
                                case MAVLINK_MSG_ID_COMMAND_LONG:
					fprintf(stdout, "COMMAND_LONG:%d ",mavlink_msg_command_long_get_command(&msg));
                                        break;
                                case MAVLINK_MSG_ID_STATUSTEXT:
					fprintf(stdout, "STATUSTEXT: severity:%d ",mavlink_msg_statustext_get_severity(&msg));
                                        break;
                                case MAVLINK_MSG_ID_SYSTEM_TIME:
					fprintf(stdout, "SYSTEM_TIME ");
                                        break;
                                case MAVLINK_MSG_ID_PING:
					fprintf(stdout, "PING ");
                                        break;
                                case MAVLINK_MSG_ID_CHANGE_OPERATOR_CONTROL:
					fprintf(stdout, "CHANGE_OPERATOR_CONTROL ");
                                        break;
                                case MAVLINK_MSG_ID_CHANGE_OPERATOR_CONTROL_ACK:
					fprintf(stdout, "CHANGE_OPERATOR_CONTROL_ACK ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_WRITE_PARTIAL_LIST:
					fprintf(stdout, "MISSION_WRITE_PARTIAL_LIST ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_ITEM:
					fprintf(stdout, "MISSION_ITEM ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_REQUEST:
					fprintf(stdout, "MISSION_REQUEST ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_SET_CURRENT:
					fprintf(stdout, "MISSION_SET_CURRENT ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_CURRENT:
					fprintf(stdout, "MISSION_CURRENT ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
					fprintf(stdout, "MISSION_REQUEST_LIST ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_COUNT:
					fprintf(stdout, "MISSION_COUNT ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
					fprintf(stdout, "MISSION_CLEAR_ALL ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:
					fprintf(stdout, "MISSION_ITEM_REACHED ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_ACK:
					fprintf(stdout, "MISSION_ACK ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_ITEM_INT:
					fprintf(stdout, "MISSION_ITEM_INT ");
                                        break;
                                case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
					fprintf(stdout, "MISSION_REQUEST_INT ");
                                        break;
                                case MAVLINK_MSG_ID_SET_MODE:
					fprintf(stdout, "SET_MODE ");
                                        break;
                                case MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
					fprintf(stdout, "REQUEST_DATA_STREAM ");
                                        break;
                                case MAVLINK_MSG_ID_DATA_STREAM:
					fprintf(stdout, "DATA_STREAM ");
*/                                        break;
                    		default:
                        		fprintf(stdout, "OTHER MESSAGE ID:%d ",msg.msgid);
                        		break;
			}
			fprintf(stdout, "\n");
			fflush(stdout);
		}
	}
	return render_data;
}
#endif
