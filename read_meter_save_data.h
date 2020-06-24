#ifndef __READ_METER_SAVE_DATA_H__
#define __READ_METER_SAVE_DATA_H__
INT8U save_read_data(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len);
INT8U save_read_data_GB645_2007(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len);
void trans_phy_cur_2_cycle_day(INT8U phy[4]);
void save_batch_meter_task_auth(READ_PARAMS *read_params,INT8U *data,INT8U datalen);
void save_batch_meter_task_time(READ_PARAMS *read_params,BOOLEAN is_success);
void plc_router_save_cjq_meter_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen,BOOLEAN is_catch);
void plc_router_save_report_meter_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen);
void plc_router_save_report_meter_ext_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen);
void save_meter_event_record(INT16U meter_idx,READ_PARAMS *read_params,INT8U *frame,INT16U framelen,report_ctrl reportCtrl);
void save_area_different_info(INT8U *frame,INT8U meter_protocol);
INT8U check_meter_idx_same_or_not(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len,INT8U* buffer);
void save_meter_power_off_event(INT8U *frame,INT8U data_len, INT8U flag);
#if defined(__SHANXI_READ_BPLC_NETWORK_INFO__)
void save_area_sta_change_info(INT8U *frame);
#endif
#ifdef __READ_OOP_METER__
INT8U save_read_data_gb_oop(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len);
void save_oop_cur_hold_data(READ_PARAMS *read_params, INT8U *data, INT16U data_len);
#endif
INT8U save_read_data_dlms(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len);
#endif //#ifndef __READ_METER_SAVE_DATA_H__
