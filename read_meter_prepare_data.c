#include "app_dev_plat.h"
#include "read_meter_prepare_data.h"
#include "gb_645.h"
#include "sh_2009.h"
#include "tpos_readport_common.h"
//#include "constdef.h"
#include "tpos_pulse_meter.h"
#include "voltage_monitor.h"     //__VOLTAGE_MONITOR__
#include "protocol_library_hengtong.h"
#include "protocol_library_edmi.h"
#ifdef __READ_OOP_METER__
#include "protocol_library_oop.h"
#endif
#include "protocol_library_iec1107.h"
//#ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
PRIORITY_NODE priority_node;
//#endif
#ifdef __BATCH_TRANSPARENT_METER_TASK__
INT8U COST_CONTROL_FAIL_FLAG[256];      //__BATCH_TRANSPARENT_METER_TASK_COST_CONTROL__
#endif
extern volatile VOLTAGE_MONITOR V_MONITOR[];
BOOLEAN get_phy_data(READ_WRITE_DATA* in,INT8U idx,READ_WRITE_DATA* out);
INT16U get_C1_F19F20F21F22(INT8U *agp_data,INT16U pi,INT16U fi,BOOLEAN flag,INT8U update);
INT16U get_C1_F17_F18(INT8U *data,INT16U pi,INT16U fi);
void save_day_hold_power_on_off(READ_PARAMS *read_params);
#if defined (__POWER_CTRL__)
INT16U get_group_leftfee(INT8U *data,INT16U agp_id);
INT16U get_C1_F23(INT8U *data,INT16U Pi);
#endif
BOOLEAN prepare_plc_cycle_meter_event_item(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len);
//BOOLEAN prepare_plc_read_meter_event_item(INT16U meter_idx,READPORT_METER_DOCUMENT *meter_doc,INT8U *frame,INT8U* frame_len,INT8U event_level);
BOOLEAN prepare_plc_read_meter_event_level_item(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len,INT8U event_level);
BOOLEAN prepare_plc_read_meter_event_item(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len,INT8U event_level);
INT8U get_plan_read_item(INT8U event_level,INT8U plan_id,INT8U *data);
BOOLEAN check_precice_time(INT8U report_channel);
INT8U make_gb645_2007_write_frame(INT8U *frame,INT8U *meter_no,INT32U item,INT8U *pwd,INT8U *data,INT8U datalen);
BOOLEAN check_dayhold_data_right(INT8U *frame); //��������Ƿ��������ܺͷַ��ʲ
INT32U check_settime_dayhold_data(INT32U item);
INT32U yunnan_check_dayhold_data(INT32U item);
INT32U check_month_hold_dyhgl_data(INT32U item);
void plc_router_save_voltmeter_data(INT16U meter_idx,INT32U data_item,INT8U *data,INT8U datalen,BOOLEAN is_month);
void plc_router_save_vlotmeter_curve_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_data);
INT8U check_cjq_meter_more_data(INT16U meter_idx);
BOOLEAN prepare_read_item_last_curve_cycle_day(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len);
INT8U check_plc_router_save_vlotmeter_curve_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U remain_num);
INT8U check_plc_router_save_last_gx_load_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U remain_num);
INT8U time_subsection (INT8U time_hour);
INT8U check_frame_blockFF_convert_EE(READ_WRITE_DATA *phy,INT8U *frame,INT8U datalen);
BOOLEAN check_read_meter_event_cycle(INT16U meter_idx);
#ifdef __PRECISE_TIME__
BOOLEAN prepare_plc_read_meter_precise_time(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len,INT16U delay,BOOLEAN is_f3);
void plc_router_save_meter_precise_time(INT16U meter_idx,INT32U phy,INT8U *data,INT8U datalen);
#endif
#ifdef __INSTANT_FREEZE__
BOOLEAN prepare_plc_read_meter_instant_freeze(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len);
INT8U get_item_idx(INT8U item[4]);
INT8U get_item_datalen(INT8U item[4]);
BOOLEAN read_instant_freeze_data(INT16U meter_idx,INT8U td[5],INT8U mask_idx);
#endif
void calculat_curve_time(INT8U* start_time,INT8U* end_time,INT8U num,INT8U midu);
BOOLEAN prepare_read_cjq_update_task(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len,BOOLEAN is_high);
void  save_trans_meter_report_data(CYCLE_TASK_DATA_REPORT_RETRIEVE *task_data);
BOOLEAN  read_trans_meter_report_data(CYCLE_TASK_DATA_REPORT_RETRIEVE *task_data,INT32U *read_pos);

BOOLEAN prepare_israel_cast_set_meter_task(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len);
INT16U  get_plc_cast_statinfo(INT8U *statinfo);
INT16U get_plc_cast_taskinfo(INT8U *taskinfo,INT8U *param,INT16U max_left_len);
void single_meter_tran_three_meter(INT8U *frame);
void single_meter_loadprofile_tran_three_meter(INT8U *frame);
void plc_router_save_israel_last_load_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num);
void set_israel_di_item(INT8U *data,INT32U phy);
void check_erc_37(INT16U spot_idx,INT8U *frame);
INT8U check_save_last_israel_load_profile_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U remain_num, INT8U* start_time);
void calculat_israel_curve_time(INT8U* start_time,INT8U* end_time,INT8U num,INT8U midu,INT8U muli_flag);
void save_last_load_profile_data_israel(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num,INT8U* start_time);
INT8U check_plc_router_save_last_israel_hour_load_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U load_cnt);
void save_last_load_hour_data_israel(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U patch_num,INT8U* start_time);
INT16U make_oop_meter_load_frame(INT8U* frame,INT8U* meter_no,INT8U *item,INT8U item_count,INT8U load_start_time[6],INT8U load_end_time[6],INT8U unit,INT16U cycle);
INT8U get_oop_last_load_data_patch_num(READ_PARAMS *read_params,READ_WRITE_DATA *phy,INT8U remain_num,INT8U* start_time);
#ifdef __HUBEI_STEP_CONTROL__
INT8U add_amount_curdata_diff_lastdata(INT8U yestoday_data[4],INT8U today_data[4],INT8U process_add_data[4],INT32U ct);
void calculate_line_lost(READPORT_METER_DOCUMENT *meter_doc,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U data_type);
INT32U get_line_lost_curve_save_offset(INT8U td[5],INT8U midu);
void line_lost_cal(INT16U meter_idx,READPORT_METER_DOCUMENT *meter_doc,INT32U phy,INT8U last_data[4],INT8U cur_data[4], LINE_LOST_F309 meter_flag,COMP_LINE_LOST *cal_data);
void compute_phase_lost_rate(INT8U sale[4], INT8U supply[4], INT8U *xlost_rate_str);
void phase_line_lost_cal(INT8U phase,INT8U last_data[4],INT8U cur_data[4], LINE_LOST_F309 meter_flag,COMP_LINE_LOST *cal_data,INT32U ct);
#endif

//Ҫ����ʱ��
READ_WRITE_DATA const CYCLE_DAY_PHY_LIST[] =
{
    //������          ƫ��                 ����=ʱ��+����ʱ��+���ݳ���+Ԥ���ֽ�         ������                    block_offset  block_len
     #ifdef __PROVICE_JILIN1__
    {0x00002620,    PIM_DAY_HOLD_ZXYG,            8+20,     SAVE_FLAG_DENY_NO_SAVE,                      0,  20,},//0x05000101,F161:�����й��ܼ�����
    {0x00002640,    PIM_DAY_HOLD_FXYG,            8+20,     SAVE_FLAG_DENY_NO_SAVE,                      0,  20,},//0x05000201,F163:�����й��ܼ�����
    {0x00002680,    PIM_DAY_HOLD_ZXWG,            8+20,     SAVE_FLAG_DENY_NO_SAVE,                      0,  20,},//0x05000301,F162:�����޹��ܼ�����
    {0x000026C0,    PIM_DAY_HOLD_FXWG,            8+20,     SAVE_FLAG_DENY_NO_SAVE,                      0,  20,},//0x05000401,F164:�����޹��ܼ�����
    {0x00002700,    PIM_DAY_HOLD_WG1,             8+20,     SAVE_FLAG_DENY_NO_SAVE,                      0,  20,},//0x05000501,F165:һ�����޹��ܼ�����
    {0x00002740,    PIM_DAY_HOLD_WG2,             8+20,     SAVE_FLAG_DENY_NO_SAVE,                      0,  20,},//0x05000601,F166:�������޹��ܼ�����
    {0x00002780,    PIM_DAY_HOLD_WG3,             8+20,     SAVE_FLAG_DENY_NO_SAVE,                      0,  20,},//0x05000701,F167:�������޹��ܼ�����
    {0x000027C0,    PIM_DAY_HOLD_WG4,             8+20,     SAVE_FLAG_DENY_NO_SAVE,                      0,  20,},//0x05000801,F168:�������޹��ܼ�����
    {0x00002800,    PIM_DAY_HOLD_ZYG_ZDXL,        8+35,     SAVE_FLAG_DENY_NO_SAVE,                      0,  35,},//0x05000901,F185:���й��������������ʱ��
    {0x00002840,    PIM_DAY_HOLD_FYG_ZDXL,        8+35,     SAVE_FLAG_DENY_NO_SAVE,                      0,  35,},//0x05000A01,F186:���й��������
   // {0x0000????,    PIM_DAY_HOLD_ZWG_ZDXL,        8+35,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0, 35,}, //0x0103FF00,F187:�ն��ᷴ���й��������������ʱ��
   // {0x0000????,    PIM_DAY_HOLD_FWG_ZDXL,        8+35,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,  35,}, //0x0104FF00,188:�ն��ᷴ���޹��������������ʱ��
     #else

     #ifdef __PROVICE_GANSU__
     {S1C_DSDJ_ZXYG_DN_SJK,   PIM_DAY_HOLD_ZXYG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},  //�¶��ᴦ��һ�£�����
     {DR_LD_DL,   PIM_DAY_HOLD_ZXYG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    4,},
     #endif
    #ifdef __COUNTRY_ISRAEL__ //�ȼ��
    {ISRAEL_HOUR_DATA,   PIM_ISRAEL_DAYHOLD_ZERO_FH_DATA,       3+5+45+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  45,},//	0x02800113 00  00��ĸ�������
    #else
    {0x00002C7F,   PIM_DAY_HOLD_ZXYG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    #endif
    {0x00002C40,   PIM_DAY_HOLD_ZXYG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000040,   PIM_DAY_HOLD_ZXYG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,},

    {0x00002CBF,   PIM_DAY_HOLD_FXYG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000080,   PIM_DAY_HOLD_FXYG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,},

    //#ifdef __ITEM_PRIORITY__
    {0xFFFFFFFF,   0,                            0,                     0,                                            0,    0 ,},
    //#endif

    {0x00002CFF,   PIM_DAY_HOLD_ZXWG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,                       0,    20,},
    {0x000000C0,   PIM_DAY_HOLD_ZXWG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,},

    {0x00002D3F,   PIM_DAY_HOLD_FXWG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000100,   PIM_DAY_HOLD_FXWG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,},

    #ifdef __PROVICE_NEIMENG__ 
    //F225 F215 F210 ���¶��Ṻ�õ���Ϣ
    {S1C_RDJ_SYJE, PIM_DAY_HOLD_GDCS_PREPAY,     3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   2,},	//0x05080201      //�������+ʣ����+͸֧��������ֻռλ���洢�ڵ�ǰ����
    {0x00008FC0,   PIM_DAY_HOLD_GDCS_PREPAY,     3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   2,},	//0x03330201      //�������
    {0x00001240,   PIM_DAY_HOLD_SYJE_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900200      //ʣ����
    #if( (defined __SICHUAN_F220_SP__) || (defined __PROVICE_NEIMENG__))
    {TZ_JE,        PIM_DAY_HOLD_LJGDJE_PREPAY,   3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900201     //͸֧���
    #else
    {0x000090C0,   PIM_DAY_HOLD_LJGDJE_PREPAY,   3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x03330601      //�ۼƹ�����
    #endif
    {0x00001140,   PIM_DAY_HOLD_SYDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900100      //ʣ�����
    #ifdef __PROVICE_NINGXIA__//����Ҫ�󳭵�ǰ��ۣ�����͸֧������λ�úͳ�����͸֧������ͬ�����Թ���һ��ƫ����
    {0x00002480,   PIM_DAY_HOLD_TZDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900101      //��ǰ���
    #else
    {0x00001180,   PIM_DAY_HOLD_TZDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900101      //͸֧����
    #endif
    {0x00008F40,   PIM_DAY_HOLD_LJGDL_PREPAY,    3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900102      //�ۼƹ�����
    {0x000070C0,   PIM_DAY_HOLD_SQMXDL_PREPAY,   3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F04      //��Ƿ���޵���
    {0x00007000,   PIM_DAY_HOLD_BJDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F01      //��������
    {0x00007040,   PIM_DAY_HOLD_GZDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F02      //���ϵ���
    #endif
    {0x00002D7F,   PIM_DAY_HOLD_WG1,             3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000140,   PIM_DAY_HOLD_WG1,             3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,},

    {0x00002DBF,   PIM_DAY_HOLD_WG2,             3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000180,   PIM_DAY_HOLD_WG2,             3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,},

    {0x00002DFF,   PIM_DAY_HOLD_WG3,             3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x000001C0,   PIM_DAY_HOLD_WG3,             3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,},

    {0x00002E3F,   PIM_DAY_HOLD_WG4,             3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000200,   PIM_DAY_HOLD_WG4,             3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,},

    {0x00002E7F,   PIM_DAY_HOLD_ZYG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_XL_TIME,     0,    40,},
    {0x0000143F,   PIM_DAY_HOLD_ZYG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_XL_TIME,     0,    40,},
    {0x00009B80,   PIM_DAY_HOLD_ZYG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009BC0,   PIM_DAY_HOLD_ZYG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},

    {0x00002EBF,   PIM_DAY_HOLD_FYG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_XL_TIME,     0,    40 ,},
    {0x0000147F,   PIM_DAY_HOLD_FYG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_XL_TIME,     0,    40 ,},
    {0x00009C80,   PIM_DAY_HOLD_FYG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009CC0,   PIM_DAY_HOLD_FYG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},

    {0x000014BF,   PIM_DAY_HOLD_ZWG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_XL_TIME,     0,    40,},
    {0x00009C00,   PIM_DAY_HOLD_ZWG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009C40,   PIM_DAY_HOLD_ZWG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},

    {0x000014FF,   PIM_DAY_HOLD_FWG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_XL_TIME,     0,    40,},
    {0x00009D00,   PIM_DAY_HOLD_FWG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009D40,   PIM_DAY_HOLD_FWG_ZDXL,        3+5+40+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
     #endif
     #ifdef __PROVICE_ABROAD_DEMO1__    //F153��F157 ������Ŀʹ�õ��Ǵ����Ṧ�ܵĵ��.û��ʹ�ã�����ɾ��
    {0x00000480,    PIM_DAY_HOLD_ZXYG_PARSE,      3+5+12+RESERVE_DATA,    0,    0,    12,},//	0x05200901,A/B/C�����ն��������й�
    {0x000008C0,    PIM_DAY_HOLD_ZXYG_PARSE,      3+5+12+RESERVE_DATA,    0,    0,    12,},//	0x05200901,A/B/C�����ն��������й�
    {0x00000D00,    PIM_DAY_HOLD_ZXYG_PARSE,      3+5+12+RESERVE_DATA,    0,    0,    12,},//	0x05200901,A/B/C�����ն��������й�
      #else
    {0x00000480,   PIM_DAY_HOLD_ZXYG_PARSE,      3+5+12+RESERVE_DATA,   0,     0,    4,}, //A�������й��ܵ���ʾֵ
    {0x000008C0,   PIM_DAY_HOLD_ZXYG_PARSE,      3+5+12+RESERVE_DATA,   0,     4,    4,}, //B�������й��ܵ���ʾֵ
    {0x00000D00,   PIM_DAY_HOLD_ZXYG_PARSE,      3+5+12+RESERVE_DATA,   0,     8,    4,}, //C�������й��ܵ���ʾֵ
     #endif
    {0x000004C0,   PIM_DAY_HOLD_FXYG_PARSE,      3+5+12+RESERVE_DATA,   0,     0,    4,  }, //A�෴���й��ܵ���ʾֵ
    {0x00000900,   PIM_DAY_HOLD_FXYG_PARSE,      3+5+12+RESERVE_DATA,   0,     4,    4,  }, //B�෴���й��ܵ���ʾֵ
    {0x00000D40,   PIM_DAY_HOLD_FXYG_PARSE,      3+5+12+RESERVE_DATA,   0,     8,    4,  }, //C�෴���й��ܵ���ʾֵ
    {0x00000500,   PIM_DAY_HOLD_WG1_PARSE,       3+5+12+RESERVE_DATA,   0,     0,    4,  }, //A������޹�1����ʾֵ
    {0x00000940,   PIM_DAY_HOLD_WG1_PARSE,       3+5+12+RESERVE_DATA,   0,     4,    4,  }, //B������޹�1����ʾֵ
    {0x00000D80,   PIM_DAY_HOLD_WG1_PARSE,       3+5+12+RESERVE_DATA,   0,     8,    4,  }, //C������޹�1����ʾֵ
    {0x00000540,   PIM_DAY_HOLD_WG2_PARSE,       3+5+12+RESERVE_DATA,   0,     0,    4,  }, //A������޹�2����ʾֵ
    {0x00000980,   PIM_DAY_HOLD_WG2_PARSE,       3+5+12+RESERVE_DATA,   0,     4,    4,  }, //B������޹�2����ʾֵ
    {0x00000DC0,   PIM_DAY_HOLD_WG2_PARSE,       3+5+12+RESERVE_DATA,   0,     8,    4,  }, //C������޹�2����ʾֵ

    #ifndef __PROVICE_NEIMENG__
     //F225 F215 F210 ���¶��Ṻ�õ���Ϣ
    {S1C_RDJ_SYJE, PIM_DAY_HOLD_GDCS_PREPAY,     3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   2,},	//0x05080201      //�������+ʣ����+͸֧��������ֻռλ���洢�ڵ�ǰ����
    {0x00008FC0,   PIM_DAY_HOLD_GDCS_PREPAY,     3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   2,},	//0x03330201      //�������
    {0x00001240,   PIM_DAY_HOLD_SYJE_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900200      //ʣ����
    #if( (defined __SICHUAN_F220_SP__) || (defined __PROVICE_NEIMENG__))
    {TZ_JE,        PIM_DAY_HOLD_LJGDJE_PREPAY,   3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900201     //͸֧���
    #else
    {0x000090C0,   PIM_DAY_HOLD_LJGDJE_PREPAY,   3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x03330601      //�ۼƹ�����
    #endif
    {0x00001140,   PIM_DAY_HOLD_SYDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900100      //ʣ�����
    #ifdef __PROVICE_NINGXIA__//����Ҫ�󳭵�ǰ��ۣ�����͸֧������λ�úͳ�����͸֧������ͬ�����Թ���һ��ƫ����
    {0x00002480,   PIM_DAY_HOLD_TZDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900101      //��ǰ���
    #else
    {0x00001180,   PIM_DAY_HOLD_TZDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900101      //͸֧����
    #endif
    {0x00008F40,   PIM_DAY_HOLD_LJGDL_PREPAY,    3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900102      //�ۼƹ�����
    {0x000070C0,   PIM_DAY_HOLD_SQMXDL_PREPAY,   3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F04      //��Ƿ���޵���
    {0x00007000,   PIM_DAY_HOLD_BJDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F01      //��������
    {0x00007040,   PIM_DAY_HOLD_GZDL_PREPAY,     3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F02      //���ϵ���
    #endif
   #ifdef __QGDW_376_2013_PROTOCOL__
    {0x00008480,    PIM_DAY_HOLD_DXCS_Z,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13000001  //3, �ܶ������
    {0x00008580,    PIM_DAY_HOLD_DXCS_A,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13010001 //3, A��������
    {0x00008640,    PIM_DAY_HOLD_DXCS_B,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13020001  //3, B��������
    {0x00008700,    PIM_DAY_HOLD_DXCS_C,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13030001  //3, C��������
   #else
    {0x00008480,    PIM_DAY_HOLD_DXCS_Z,         3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  2,},//	0x13000001  //2, �ܶ������
    {0x00008580,    PIM_DAY_HOLD_DXCS_A,         3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  2,},//	0x13010001 //2, A��������
    {0x00008640,    PIM_DAY_HOLD_DXCS_B,         3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  2,},//	0x13020001  //2, B��������
    {0x00008700,    PIM_DAY_HOLD_DXCS_C,         3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  2,},//	0x13030001  //2, C��������
   #endif
    {0x000084C0,    PIM_DAY_HOLD_DXSJ_Z,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x130000020,   //3, ����ʱ���ۼ�ֵ
    {0x000085C0,    PIM_DAY_HOLD_DXSJ_A,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13010002  //3, A�����ʱ���ۼ�ֵ
    {0x00008680,    PIM_DAY_HOLD_DXSJ_B,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13020002  //3, B�����ʱ���ۼ�ֵ
    {0x00008740,    PIM_DAY_HOLD_DXSJ_C,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13030002 //3, C�����ʱ���ۼ�ֵ
    {0x00008500,    PIM_DAY_HOLD_QSSK_Z,         3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x13000101 //6, ���һ�ζ�����ʼʱ��
    {0x00008600,    PIM_DAY_HOLD_QSSK_A,         3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x13010101 //6, A�����������ʼʱ��
    {0x000086C0,    PIM_DAY_HOLD_QSSK_B,         3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x13020101  //6, B�����������ʼʱ��
    {0x00008780,    PIM_DAY_HOLD_QSSK_C,         3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x13030101 //6, C�����������ʼʱ��
    {0x00008540,    PIM_DAY_HOLD_JSSK_Z,         3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x13000201 //6, ���һ�ζ������ʱ��
    {0x00008624,    PIM_DAY_HOLD_JSSK_A,         3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x13020101  //6, A������������ʱ��
    {0x000086E4,    PIM_DAY_HOLD_JSSK_B,         3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x13030101 //6, B������������ʱ��
    {0x000087A4,    PIM_DAY_HOLD_JSSK_C,         3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x13000201 //6, C�����һ�ζ������ʱ��

    {0x0000AF00,    PIM_DAY_HOLD_DDCS_Z,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13110000 //3, �������
    {0x0000AF40,    PIM_DAY_HOLD_DDJL,           3+5+120+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE|9,                    0,  12,},//	0x13110001 //12, ��1�ε����¼

    //��ѹ��
    #ifdef __PLC_REC_VOLTMETER1__
    {DY_A_HGL,  PIM_DAY_HOLD_DYHGL_A,         3+5+26+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  26,},//	0x03100100  //27, A���ѹ�ϸ���,���ּ������ķ��ص���30���ֽ�
    {DY_B_HGL,  PIM_DAY_HOLD_DYHGL_B,         3+5+26+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  26,},//	0x03100200 //27, B���ѹ�ϸ���
    {DY_C_HGL,  PIM_DAY_HOLD_DYHGL_C,         3+5+26+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  26,},//	0x03100300 //27, C���ѹ�ϸ���
    {DY_HOUR_HOLD,  PIM_DAY_HOLD_DY_HOUR_HOLD ,  3+5+24+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                      0,  24, },//	0x05200101 //��1Сʱ�ĵ�ѹ����
    #endif
    #ifdef __PROVICE_NEIMENG__
    {0x00000000,   PIM_DAY_HOLD_ZHYG,           3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},//(��ǰ)����й��������ݿ�
    #endif
      //����һ������������ʾCJT�����ݣ����鵽����������901F����Ҫ����һ��CJT188��Э���Լ��  �������������ٴι�Լ�����޷��鵽��Ӧ������ ������
     //��ʱ����һ����������׼������ʱ������Э��̶����á�20150909
    {CJT_DQ_LJLL_LSB,   PIM_DAY_HOLD_CJT_DATA,            3+5+60+RESERVE_DATA,   0,     0,   60 ,}, //����F188��ʽ�������ֱ����ݸ�ʽͳһ ��44���ֽ��е�һ���ֽ����Ǳ�����
    {CJT_DQ_LJLL_ZSB,   PIM_DAY_HOLD_CJT_DATA,            3+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_CJSB,   PIM_DAY_HOLD_CJT_DATA,            3+5+19+RESERVE_DATA,   0,     0,   19 ,},                    //�Ժ�Ҫ�ĳ�һ�������������մ�С��Ÿı��������͸��188���ã�������43������
    {CJT_DQ_LJLL_RSB,   PIM_DAY_HOLD_CJT_DATA,            3+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_DZSB,   PIM_DAY_HOLD_CJT_DATA,            3+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_JRLB,   PIM_DAY_HOLD_CJT_DATA,            3+5+43+RESERVE_DATA,   0,     0,   43 ,},
    {CJT_DQ_LJLL_JLLB,   PIM_DAY_HOLD_CJT_DATA,            3+5+43+RESERVE_DATA,   0,     0,   43 ,},
    {CJT_DQ_LJLL_RQB,   PIM_DAY_HOLD_CJT_DATA,            3+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_DDB,   PIM_DAY_HOLD_CJT_DATA,            3+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_S1CDJ,   PIM_DAY_HOLD_CJT_DATA,            3+5+60+RESERVE_DATA,   0,     0,   60 ,},
 //   #ifdef __PRECISE_TIME__

    {RQ_XQ ,  PIM_METER_DAY_WEEK,         3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  4,},//	0x04000101
    {SJ,      PIM_METER_TIME,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x04000102
    {JZQ_READ_TIME,  PIM_DAY_HOLD_JZQ_READ_TIME,  3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,              0,  6,},//	0x04000101
    {(S1R_DYHGL_TJ_SJ_SJK-0x3F+1),PIM_DAY_HOLD_VOLTAGE_STATE,3+5+27+RESERVE_DATA,SAVE_FLAG_DENY_NO_SAVE,0,27}, // 03100000 ʣ����������� ���� ��ѹ�ϸ���ͳ��
 //   #endif

     //breaker //F235�еĵ�ǰ���ʱ����������ϱ߳��˲����ظ����
    {SYDL_ZDX_ZDZ_FSSK_SJK-63+1 ,      PIM_DAY_HOLD_SYDLZDX_S1R,           3+5+9+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  9,},
    {AX_DL_RZD_FSSK_SJK-63+1 ,         PIM_DAY_HOLD_DLRZDZ_A_S1R,          3+5+15+RESERVE_DATA,     SAVE_FLAG_DENY_NO_SAVE,                      0,  15,},
    {BX_DL_RZD_FSSK_SJK-63+1 ,         PIM_DAY_HOLD_DLRZDZ_B_S1R,          3+5+15+RESERVE_DATA,     SAVE_FLAG_DENY_NO_SAVE,                      0,  15,},
    {CX_DL_RZD_FSSK_SJK-63+1 ,         PIM_DAY_HOLD_DLRZDZ_C_S1R,          3+5+15+RESERVE_DATA,     SAVE_FLAG_DENY_NO_SAVE,                      0,  15,},

    {SJQL_CS ,                  PIM_DAY_HOLD_SJQLCS,                3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},
    {TZCS_CS_MK_SJK ,           PIM_DAY_HOLD_TZCS,                  3+5+25+RESERVE_DATA,     SAVE_FLAG_DENY_NO_SAVE,                      0,  25,},
    {TC_SYDL_BH_CS ,            PIM_DAY_HOLD_TCBHCS,                3+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  2,},
    {BHQ_YX_LJSJ ,              PIM_DAY_HOLD_YXSJZLJ,               3+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  4,},
    #ifdef __COUNTRY_ISRAEL__
    {ISRAEL_WATER_DATA,  PIM_ISRAEL_DAYHOLD_WATER_DATA,         3+5+80+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  80,},//	0x200101FF
    {ISRAEL_GAS_DATA,    PIM_ISRAEL_DAYHOLD_GAS_DATA,           3+5+80+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  80,},//	0x200102FF
    {ISRAEL_APM_DATA,    PIM_ISRAEL_DAYHOLD_APM_DATA,           3+5+48+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  48,},//	0x20019901 APM������
    {SOUTH_AFRICA_DATA,  PIM_SOUTHAFRICA_DAYHOLD_DATA,          3+5+38+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  38,},//	0x5063001,38�ֽڣ�20+2+16
    {0x00002C7F,   PIM_DAY_HOLD_ZXYG,            3+5+20+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    #endif
    #ifdef __FUJIAN_CURRENT_BREAK__
    {FUJIAN_CURRENT_BREAK_V,  PIM_CURRENT_BREAK_V_DATA,          3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0x0201FF00
    {FUJIAN_CURRENT_BREAK_REMAIN_I,  PIM_CURRENT_BREAK_REMAIN_I_DATA,         3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0xEA020000
    {FUJIAN_CURRENT_BREAK_ZERO_LINE_I,    PIM_CURRENT_BREAK_ZERO_LINE_I_DATA,           3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0xEA0200001
    {FUJIAN_CURRENT_BREAK_ZERO_ORDER_I,    PIM_CURRENT_BREAK_ZERO_ORDER_I_DATA,           3+5+3+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x? û�ҵ���
    {FUJIAN_CURRENT_BREAK_I,  PIM_CURRENT_BREAK_I_DATA,          3+5+9+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  9,},//	0xEA0202FF

    {FUJIAN_CURRENT_BREAK_TRIP_BLOCK,  PIM_CURRENT_BREAK_TRIP_BLOCK_DATA,          3+5+16+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  16,},//	0xEA0300FF ��բ�������ݿ�
    {FUJIAN_CURRENT_BREAK_PROTECTOR_RUN,  PIM_CURRENT_BREAK_PROTECTOR_RUN_DATA,          3+5+6+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                      0,  6,},//	0xEA0301FF �������������
    #endif
    #ifdef __HIGH_PRECISION_DATA__
    {(HIGH_PRECISION_ZXYG_DN_SJK-0x3F),   PIM_DAY_HOLD_HIGH_PRECISION_ZXYG,            3+5+25+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    5 ,},

    {(HIGH_PRECISION_FXYG_DN_SJK-0x3F),   PIM_DAY_HOLD_HIGH_PRECISION_FXYG,            3+5+25+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    5 ,},

    {HIGH_PRECISION_ZHWG1_DN_SJK-0x3F,   PIM_DAY_HOLD_HIGH_PRECISION_ZXWG,            3+5+25+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    5 ,},

    {HIGH_PRECISION_ZHWG2_DN_SJK-0x3F,   PIM_DAY_HOLD_HIGH_PRECISION_FXWG,            3+5+25+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    5 ,},

    {HIGH_PRECISION_D1XX_WG_DN_SJK,   PIM_DAY_HOLD_HIGH_PRECISION_WG1,             3+5+25+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    5 ,},

    {HIGH_PRECISION_D2XX_WG_DN_SJK,   PIM_DAY_HOLD_HIGH_PRECISION_WG2,             3+5+25+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    5 ,},

    {HIGH_PRECISION_D3XX_WG_DN_SJK,   PIM_DAY_HOLD_HIGH_PRECISION_WG3,             3+5+25+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    5 ,},

    {HIGH_PRECISION_D4XX_WG_DN_SJK,   PIM_DAY_HOLD_HIGH_PRECISION_WG4,             3+5+25+RESERVE_DATA,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    5 ,},
    #endif
};

//Ҫ����ʱ��
READ_WRITE_DATA const CYCLE_MONTH_PHY_LIST[] =
{
    //������          ƫ��    ����=ʱ��+���ݳ���+Ԥ���ֽ�    flag     tmp
#ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
    {0x00003B80,   PIM_RECDAY_ZXYG,        2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)�����й��������ݿ�
#else
    {0x00003B80,   PIM_MONTH_HOLD_ZXYG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)�����й��������ݿ�
#endif
    {0x00002C7F,   PIM_MONTH_HOLD_ZXYG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000040,   PIM_MONTH_HOLD_ZXYG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},
    #ifdef __SH_2009_METER__
    {S1JSR_ZXYG_DN_SJK,   PIM_MONTH_HOLD_ZXYG,     2+5+20+5,   0,     0,    20,},
    #endif

    {0x00003BC0,   PIM_MONTH_HOLD_FXYG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)�����й��������ݿ�
    {0x00002CBF,   PIM_MONTH_HOLD_FXYG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000080,   PIM_MONTH_HOLD_FXYG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},
    
    {0x00003C00,   PIM_MONTH_HOLD_ZXWG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)����޹�1 �������ݿ�
    {0x00002CFF,   PIM_MONTH_HOLD_ZXWG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE,                       0,   20,},
    {0x000000C0,   PIM_MONTH_HOLD_ZXWG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},
    
    {0x00003C40,   PIM_MONTH_HOLD_FXWG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)����޹�2 �������ݿ�
    {0x00002D3F,   PIM_MONTH_HOLD_FXWG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000100,   PIM_MONTH_HOLD_FXWG,            2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},
    
#ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
    {0x00003C80,   PIM_RECDAY_WG1,         2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)��һ�����޹��������ݿ�
#else
    {0x00003C80,   PIM_MONTH_HOLD_WG1,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)��һ�����޹��������ݿ�
#endif
    {0x00002D7F,   PIM_MONTH_HOLD_WG1,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000140,   PIM_MONTH_HOLD_WG1,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},
    
#ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
    {0x00003CC0,   PIM_RECDAY_WG2,         2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)��һ�����޹��������ݿ�
#else
    {0x00003CC0,   PIM_MONTH_HOLD_WG2,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)�ڶ������޹��������ݿ�
#endif
    {0x00002DBF,   PIM_MONTH_HOLD_WG2,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000180,   PIM_MONTH_HOLD_WG2,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},
    
#ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
    {0x00003D00,   PIM_RECDAY_WG3,         2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)��һ�����޹��������ݿ�
#else
    {0x00003D00,   PIM_MONTH_HOLD_WG3,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)���������޹��������ݿ�
#endif
    {0x00002DFF,   PIM_MONTH_HOLD_WG3,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x000001C0,   PIM_MONTH_HOLD_WG3,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},

#ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
    {0x00003D40,   PIM_RECDAY_WG4,         2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)��һ�����޹��������ݿ�
#else
    {0x00003D40,   PIM_MONTH_HOLD_WG4,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},    //(��1 ������)���������޹��������ݿ�
#endif
    {0x00002E3F,   PIM_MONTH_HOLD_WG4,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    20,},
    {0x00000200,   PIM_MONTH_HOLD_WG4,             2+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},

    {0x00004EBF,   PIM_MONTH_HOLD_ZYG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},    //(��1 ������)�����й��������������ʱ�����ݿ�
    {0x00002E7F,   PIM_MONTH_HOLD_ZYG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},
    {0x0000143F,   PIM_MONTH_HOLD_ZYG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},
    {0x00009B80,   PIM_MONTH_HOLD_ZYG_ZDXL,        2+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009BC0,   PIM_MONTH_HOLD_ZYG_ZDXL,        2+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    
    {0x00004EFF,   PIM_MONTH_HOLD_FYG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},    //(��1 ������)�����й��������������ʱ�����ݿ�
    {0x00002EBF,   PIM_MONTH_HOLD_FYG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},
    {0x0000147F,   PIM_MONTH_HOLD_FYG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},
    {0x00009C80,   PIM_MONTH_HOLD_FYG_ZDXL,        2+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009CC0,   PIM_MONTH_HOLD_FYG_ZDXL,        2+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    
    {0x00004F3F,   PIM_MONTH_HOLD_ZWG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},    //(��1 ������ )����޹�1 �������������ʱ�����ݿ�
    {0x000014BF,   PIM_MONTH_HOLD_ZWG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},
    {0x00009C00,   PIM_MONTH_HOLD_ZWG_ZDXL,        2+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009C40,   PIM_MONTH_HOLD_ZWG_ZDXL,        2+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},

    {0x00004F7F,   PIM_MONTH_HOLD_FWG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},    //(��1 ������ )����޹�2 �������������ʱ�����ݿ�
    {0x000014FF,   PIM_MONTH_HOLD_FWG_ZDXL,        2+5+40+5,   SAVE_FLAG_XL_TIME,     0,    40 ,},
    {0x00009D00,   PIM_MONTH_HOLD_FWG_ZDXL,        2+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009D40,   PIM_MONTH_HOLD_FWG_ZDXL,        2+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    

    {0x00000480,   PIM_MONTH_HOLD_ZXYG_PARSE,      2+5+12+5,   0,     0,    4,},
    {0x000008C0,   PIM_MONTH_HOLD_ZXYG_PARSE,      2+5+12+5,   0,     4,    4,},
    {0x00000D00,   PIM_MONTH_HOLD_ZXYG_PARSE,      2+5+12+5,   0,     8,    4,},
    {0x000004C0,   PIM_MONTH_HOLD_FXYG_PARSE,      2+5+12+5,   0,     0,    4,  }, //A�෴���й��ܵ���ʾֵ
    {0x00000900,   PIM_MONTH_HOLD_FXYG_PARSE,      2+5+12+5,   0,     4,    4,  }, //B�෴���й��ܵ���ʾֵ
    {0x00000D40,   PIM_MONTH_HOLD_FXYG_PARSE,      2+5+12+5,   0,     8,    4,  }, //C�෴���й��ܵ���ʾֵ
    {0x00000500,   PIM_MONTH_HOLD_WG1_PARSE,       2+5+12+5,   0,     0,    4,  }, //A������޹�1����ʾֵ
    {0x00000940,   PIM_MONTH_HOLD_WG1_PARSE,       2+5+12+5,   0,     4,    4,  }, //B������޹�1����ʾֵ
    {0x00000D80,   PIM_MONTH_HOLD_WG1_PARSE,       2+5+12+5,   0,     8,    4,  }, //C������޹�1����ʾֵ
    {0x00000540,   PIM_MONTH_HOLD_WG2_PARSE,       2+5+12+5,   0,     0,    4,  }, //A������޹�2����ʾֵ
    {0x00000980,   PIM_MONTH_HOLD_WG2_PARSE,       2+5+12+5,   0,     4,    4,  }, //B������޹�2����ʾֵ
    {0x00000DC0,   PIM_MONTH_HOLD_WG2_PARSE,       2+5+12+5,   0,     8,    4,  }, //C������޹�2����ʾֵ

  #ifdef __PLC_REC_VOLTMETER1__
    {Y_DY_A_HGL,   PIM_MONTH_HOLD_DYHGL_A,       2+5+26+5,   0,     0,    26,  }, //A���ѹ�ϸ���
    {Y_DY_B_HGL,   PIM_MONTH_HOLD_DYHGL_B,       2+5+26+5,   0,     0,    26,  }, //B���ѹ�ϸ���
    {Y_DY_C_HGL,   PIM_MONTH_HOLD_DYHGL_C,       2+5+26+5,   0,     0,    26,  }, //C���ѹ�ϸ���
  #endif
   {BCZ_CS,    PIM_MONTH_HOLD_BCZ_CS,           2+5+3+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                    0,  3,},//	0x03300000//����ܴ���
    {S1C_BC_JL,    PIM_MONTH_HOLD_S1C_BC_JL,           2+5+6+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                    0,  6,},//  0x03300001(��1	��)��̼�¼��
    {KDNH_ZCS,    PIM_MONTH_HOLD_KDNH_ZCS,           2+5+3+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                    0,  3,},// 0x03300E00//����ť���ܴ���
    {S1C_KDG_JL,    PIM_MONTH_HOLD_S1C_KDG_JL,           2+5+6+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                    0,  6,},// 0x03300E01(��1	��)���˸Ǽ�¼��

    {JS_ZCS,    PIM_MONTH_HOLD_JS_ZCS,           2+5+3+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                    0,  3,},//Уʱ�ܴ���
    {S1C_JS_JL,    PIM_MONTH_HOLD_S1C_JS_JL,           2+5+16+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                    0,  16,},//(��1	��)Уʱ��¼��
    {SDB_BC_ZCS,    PIM_MONTH_HOLD_SDB_BC_ZCS,           2+5+3+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                    0,  3,},// ʱ�α����ܴ���
    {S1C_SDB_BC_JL,    PIM_MONTH_HOLD_S1C_SDB_BC_JL,           2+5+6+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                    0,  6,},// (��1	��)ʱ�α��̼�¼��

    //F225 F215 F210 ���¶��Ṻ�õ���Ϣ
    {0x00008FC0,   PIM_MONTH_HOLD_GDCS_PREPAY,     2+5+2+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   2,},	//0x03330201      //�������
    {0x00001240,   PIM_MONTH_HOLD_SYJE_PREPAY,     2+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900200      //ʣ����
    {0x000090C0,   PIM_MONTH_HOLD_LJGDJE_PREPAY,   2+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x03330601      //�ۼƹ�����
   // {0x00001140,   PIM_MONTH_HOLD_SYDL_PREPAY,     2+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900100      //ʣ�����
   // {0x00001180,   PIM_MONTH_HOLD_TZDL_PREPAY,     2+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900101      //͸֧����
    //{0x000011C0,   PIM_MONTH_HOLD_LJGDL_PREPAY,    2+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900102      //�ۼƹ�����
    //{0x000070C0,   PIM_MONTH_HOLD_SQMXDL_PREPAY,   2+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F04      //��Ƿ���޵���
    //{0x00007000,   PIM_MONTH_HOLD_BJDL_PREPAY,     2+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F01      //��������
    //{0x00007040,   PIM_MONTH_HOLD_GZDL_PREPAY,     2+5+4+RESERVE_DATA,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F02      //���ϵ���
    {(S1Y_A_DYHGL_TJ_SJ_SJK-0x3F+1),PIM_MONTH_HOLD_A_VOLTAGE_STATE,2+5+27+RESERVE_DATA,SAVE_FLAG_DENY_NO_SAVE,            0,   27}, // 03100100 ʣ����������� ���� ��ѹ�ϸ���ͳ��
    {(S1Y_B_DYHGL_TJ_SJ_SJK-0x3F+1),PIM_MONTH_HOLD_B_VOLTAGE_STATE,2+5+27+RESERVE_DATA,SAVE_FLAG_DENY_NO_SAVE,            0,   27}, // 03100200 ʣ����������� ���� ��ѹ�ϸ���ͳ��
    {(S1Y_C_DYHGL_TJ_SJ_SJK-0x3F+1),PIM_MONTH_HOLD_C_VOLTAGE_STATE,2+5+27+RESERVE_DATA,SAVE_FLAG_DENY_NO_SAVE,            0,   27}, // 03100300 ʣ����������� ���� ��ѹ�ϸ���ͳ��

    {CJT_DQ_LJLL_LSB,    PIM_MONTH_HOLD_CJT_DATA,            2+5+60+RESERVE_DATA,   0,     0,    60,},
    {CJT_DQ_LJLL_ZSB,   PIM_MONTH_HOLD_CJT_DATA,            2+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_CJSB,   PIM_MONTH_HOLD_CJT_DATA,            2+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_RSB,   PIM_MONTH_HOLD_CJT_DATA,            2+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_DZSB,   PIM_MONTH_HOLD_CJT_DATA,            2+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_JRLB,   PIM_MONTH_HOLD_CJT_DATA,            2+5+43+RESERVE_DATA,   0,     0,   43 ,},
    {CJT_DQ_LJLL_JLLB,   PIM_MONTH_HOLD_CJT_DATA,            2+5+43+RESERVE_DATA,   0,     0,   43 ,},
    {CJT_DQ_LJLL_RQB,   PIM_MONTH_HOLD_CJT_DATA,            2+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_DDB,   PIM_MONTH_HOLD_CJT_DATA,            2+5+19+RESERVE_DATA,   0,     0,   19 ,},
    {CJT_DQ_LJLL_S1CDJ,   PIM_MONTH_HOLD_CJT_DATA,            2+5+60+RESERVE_DATA,   0,     0,   60 ,},

};

READ_WRITE_DATA const DAY_INIT_PHY_LIST[] =    //��Ҫ��5���ֽڵĳ���ʱ�䣬�����ʱ��͵�ǰֵ�Ƚϡ�
{
    //������          ƫ��                 ����=���+����ʱ��+���ݳ���+Ԥ���ֽ�         ������                    block_offset  block_len
    {0x00000040,    PIM_DAY_INIT_ZXYG,     4+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  }, //(��ǰ)�����й�����
    {0x00000080,    PIM_DAY_INIT_FXYG,     4+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  }, //(��ǰ)�����й����� 
    {0x000000C0,    PIM_DAY_INIT_ZXWG,     4+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  }, //(��ǰ)����޹�1 ����
    {0x00000100,    PIM_DAY_INIT_FXWG,     4+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  }, //(��ǰ)����޹�2 ����
    {0x00001E80,    PIM_DAY_INIT_YGGL,     4+5+12+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,     0,    3 ,  },  //˲ʱ�й�����
    {0x00001EC0,    PIM_DAY_INIT_WGGL,     4+5+12+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,     0,    3 ,  },  //˲ʱ�޹�����
};

READ_WRITE_DATA const MONTH_INIT_PHY_LIST[] =
{
    //������          ƫ��                 ����=���+ʱ��+����ʱ��+���ݳ���+Ԥ���ֽ�         ������                    block_offset  block_len
    {0x00000040,    PIM_MONTH_INIT_ZXYG,     4+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  }, //(��ǰ)�����й�����
    {0x00000080,    PIM_MONTH_INIT_FXYG,     4+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  }, //(��ǰ)�����й����� 
    {0x000000C0,    PIM_MONTH_INIT_ZXWG,     4+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  }, //(��ǰ)����޹�1 ����
    {0x00000100,    PIM_MONTH_INIT_FXWG,     4+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  }, //(��ǰ)����޹�2 ����
    {0x00001E80,    PIM_MONTH_INIT_YGGL,     4+5+12+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,     0,    3 ,  },  //˲ʱ�й�����
    {0x00001EC0,    PIM_MONTH_INIT_WGGL,     4+5+12+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,     0,    3 ,  },  //˲ʱ�޹�����
};

READ_WRITE_DATA const CRUVE_PHY_LIST[] =
{
    //������          ƫ��            ����=ʱ��+���ݳ���+Ԥ���ֽ�      flag                   block_offset  block_len
    #ifdef __PROVICE_BEIJING__  //ͳһ�������Ǹ����ʵĶ����ϣ����Ǳ��������µ���Щ��������������ʱ���������ϣ�����ֻ�������ݣ���������ʹ洢����������Ŷ����üӱ������ء����ǹ����Ĵ洢�ռ���˷�һЩ�����Է�������������
    {(FHJL6_YG_WG_ZDN_14-0x3F),  PIM_CURVE_ZFXYWG,     5+80+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,    0,              0,  },  // 14��ʽ ���������޹����ɼ�¼ 061006FF 01 02 03 04
    {FHJL4_ZFXYG,                PIM_CURVE_ZFXYWG,     5+80+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      0,              0,  },  // 12��ʽ ���������޹����ɼ�¼ 06040001
    {0x00002BC1,                 PIM_CURVE_ZFXYWG,     5+80+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      0,              4,  },  //(��1��)���㶳�������й��ܵ���
    {0x00000040,                 PIM_CURVE_ZFXYWG,     5+80+CURVE_RESERVE_SIZE ,       SAVE_FLAG_BLOCK|5,                           0,              4,  },  //(��ǰ)�����й��ܵ���
    {0x00002BC2,                 PIM_CURVE_ZFXYWG,     5+80+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      20,             4,  },  //(��1��)���㶳�������й��ܵ���
    {0x00000080,                 PIM_CURVE_ZFXYWG,     5+80+CURVE_RESERVE_SIZE ,       SAVE_FLAG_BLOCK|5,                           20,             4,  },  //(��ǰ)�����й��ܵ���
    {0x000000C0,                 PIM_CURVE_ZFXYWG,     5+80+CURVE_RESERVE_SIZE ,       SAVE_FLAG_BLOCK|5,                           40,             4,  },  //(��ǰ)����޹�1 �ܵ���
    {0x00000100,                 PIM_CURVE_ZFXYWG,     5+80+CURVE_RESERVE_SIZE ,       SAVE_FLAG_BLOCK|5,                           60,             4,  },  //(��ǰ)����޹�2 �ܵ���
    #else
    {(FHJL6_YG_WG_ZDN_14-0x3F),  PIM_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,    0,            4,  },  // 14��ʽ ���������޹����ɼ�¼ 061006FF 01 02 03 04
    {FHJL4_ZFXYG,                PIM_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      0,           16,  },  // 12��ʽ ���������޹����ɼ�¼ 06040001
    {0x00002BC1,                 PIM_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      0,            4,  },  //(��1��)���㶳�������й��ܵ���
    {0x00000040,                 PIM_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       0,                                           0,            4,  },  //(��ǰ)�����й��ܵ���
    {0x00002BC2,                 PIM_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      4,            4,  },  //(��1��)���㶳�������й��ܵ���
    {0x00000080,                 PIM_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      4,            4,  },  //(��ǰ)�����й��ܵ���
    {0x000000C0,                 PIM_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      8,            4,  },  //(��ǰ)����޹�1 �ܵ���
    {0x00000100,                 PIM_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                     12,            4,  },  //(��ǰ)����޹�2 �ܵ���
    #endif
    {(FHJL7_SXX_WG_ZDN_14-0x3F), PIM_CURVE_WG1234,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,    0,            4,  },  // 14��ʽ �������޹����ɼ�¼ 061007FF 01 02 03 04
    {FHJL5_SXX_WG,               PIM_CURVE_WG1234,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      0,           16,  },  // 12��ʽ �������޹����ɼ�¼ 06050001
    {0x00000140,                 PIM_CURVE_WG1234,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      0,            4,  },  //(��ǰ)��һ�����޹��ܵ���
    {0x00000180,                 PIM_CURVE_WG1234,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      4,            4,  },  //(��ǰ)�ڶ������޹��ܵ���
    {0x000001C0,                 PIM_CURVE_WG1234,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      8,            4,  },  //(��ǰ)���������޹��ܵ���
    {0x00000200,                 PIM_CURVE_WG1234,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                     12,            4,  },  //(��ǰ)���������޹��ܵ���

    {(FHJL1_DY_14-0x3F),         PIM_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,    0,            2,  },  //14��ʽ ��ѹ���ɼ�¼ 061001FF 01 02 03
    {(FHJL2_DL_14-0x3F),         PIM_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,    6,            3,  },  //14��ʽ �������ɼ�¼ 061002FF 01 02 03
    {FHJL1_DY_DL_PL,             PIM_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE,                      0,           15,  },  //12��ʽ ��ѹ���� ���ɼ�¼ 06010001
    {0x00001E00,                 PIM_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,    0,            2,  },  //��ѹ
    {0x00001E40,                 PIM_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,    6,            3,  },  //����
    {0x000021C0,                 PIM_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE,                     15,            3,  },  //���ߵ���

    {(FHJL3_YGGL_14-0x3F),       PIM_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,    0,            3,  },  //14��ʽ �й����ʸ��ɼ�¼ 061003FF 00 01 02 03
    {(FHJL4_WGGL_14-0x3F),       PIM_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,   12,            3,  },  //14��ʽ �޹����ʸ��ɼ�¼ 061004FF 00 01 02 03
    {FHJL2_YG_WG_GL,             PIM_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE,                      0,           24,  },  //12��ʽ �С��޹����ʸ��ɼ�¼ 06020001
    {0x00001E80,                 PIM_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,    0,            3,  },  //˲ʱ���й�����
    {0x00001EC0,                 PIM_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,   12,            3,  },  //˲ʱ���޹�����

    {(FHJL5_GLYS_14-0x3F),       PIM_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,   24,            2,  },  //14��ʽ �����������ɼ�¼ 061005FF 00 01 02 03 
    {FHJL3_GL_YS,                PIM_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE,                     24,            8,  },  //12��ʽ �����������ɼ�¼  06030001
    {0x00001F40,                 PIM_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,   24,            2,  },  //�ܹ�������

    {0x00001F80,    PIM_CURVE_XWJ  ,      5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,    0,            2,  },  //���
    {0x0000A8C0,    PIM_CURVE_XWJ  ,      5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|6,    6,            2,  },  //���

    {NBQ_SJ,        PIM_CURVE_GF_NBQ  ,   5+NBQ_SJ_MAX_LEN+CURVE_RESERVE_SIZE ,               0,                                   0,           NBQ_SJ_MAX_LEN,  },  //���
    
    {0x00000000,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,   SAVE_FLAG_DENY_NO_SAVE,               0,           4,  },  //(��ǰ)����й�  �ܵ���
    #ifdef __MX_BDZZD__
    {0x00000041,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,   SAVE_FLAG_DENY_NO_SAVE,               4,           4,  },//����ǰ�������й����ܷ���1
    {0x00000042,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,   SAVE_FLAG_DENY_NO_SAVE,               8,           4,  },//����ǰ�������й����ܷ���2
    {0x00000043,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,   SAVE_FLAG_DENY_NO_SAVE,              12,           4,  },//����ǰ�������й����ܷ���3
    {0x00000044,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,   SAVE_FLAG_DENY_NO_SAVE,              16,           4,  },//����ǰ�������й����ܷ���4
    {0x000000C1,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,   SAVE_FLAG_DENY_NO_SAVE,              20,           4,  },//(��ǰ)����޹�1 ����1
    {0x000000C2,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,    SAVE_FLAG_DENY_NO_SAVE,              24,           4,  },//(��ǰ)����޹�1 ����2
    {0x000000C3,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,    SAVE_FLAG_DENY_NO_SAVE,              28,           4,  },//(��ǰ)����޹�1 ����3
    {0x000000C4,    PIM_CURVE_ZXYWGSJK,   5+4+16+16+CURVE_RESERVE_SIZE ,    SAVE_FLAG_DENY_NO_SAVE,              32,           4,  },//(��ǰ)����޹�1 ����4
    #endif
    //��ѹ�� __PLC_REC_VOLTMETER__����������ʱ��������Ȳ�����
//    {0x0000A8C0,    PIM_CURVE_DYHGL  ,    5+6+27+CURVE_RESERVE_SIZE ,     0,    6,            2,  },  //0x05200101����ѹ����

    //BREAKER
    {SYDL_SJK,    PIM_CURVE_DQSYDLZ,   5+3+CURVE_RESERVE_SIZE ,    SAVE_FLAG_DENY_NO_SAVE,              0,           3,  }, //��ǰʣ�����ֵ

    {BPLC_STA_INFO,     PIM_CURVE_BPLC_STA_INFO,   5+53+CURVE_RESERVE_SIZE ,    0,              0,           53,  }, //���·��������Ϣ

    #ifdef __COUNTRY_ISRAEL__
    {ISRAEL_HOUR_DATA, PIM_ISRAEL_CURDATA_CURVE ,  6+5+45+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                      0,  45, },   //	0x02800113 //��1Сʱ���ɼ�¼
    {SOUTH_AFRICA_HOUR, PIM_ISRAEL_CURDATA_CURVE ,  6+5+45+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                      0,  45, },   //	0x02800113 //��1Сʱ���ɼ�¼
    {ISRAEL_THREE_OR_SIGLE_PHASE,    PIM_ISRAEL_XL  ,      5+12+CURVE_RESERVE_SIZE ,     0,    0,            4,  },  //��ɫ�е������ʶ��û��ȥ������ֻ���˴洢�Ͷ�ȡ
    {ISRAEL_HZ,                      PIM_ISRAEL_XL  ,      5+12+CURVE_RESERVE_SIZE ,     0,    4,            2,  },
    {ISRAEL_XL_ZX,                   PIM_ISRAEL_XL  ,      5+12+CURVE_RESERVE_SIZE ,     0,    6,            3,  },  //
    {ISRAEL_XL_FX,                   PIM_ISRAEL_XL  ,      5+12+CURVE_RESERVE_SIZE ,     0,    9,            3,  },  //

    {ISRAEL_ZXWG_T1234 ,    PIM_ISRAEL_WGT1234,     5+40+CURVE_RESERVE_SIZE ,  SAVE_FLAG_DENY_NO_SAVE,                                    0,            20,  },  //(��ǰ)����޹�1 �ܵ���
    {ISRAEL_FXWG_T1234 ,    PIM_ISRAEL_WGT1234,     5+40+CURVE_RESERVE_SIZE ,  SAVE_FLAG_DENY_NO_SAVE,                                   20,            20,  },  //(��ǰ)����޹�2 �ܵ���

    {ISRAEL_ZHYG_T1234 ,    PIM_ISRAEL_ZHYG,        5+20+CURVE_RESERVE_SIZE ,              0,                                    0,            20,  },  //(��ǰ)����й� �ܵ���

    {ISRAEL_LOAD_PROFILE,  PIM_ISRAEL_LOAD_PRO   ,   5+91+CURVE_RESERVE_SIZE ,          SAVE_FLAG_DENY_NO_SAVE,    0,            96,  },  //
    #endif

    #ifdef __MEXICO_RAIL__
    {0x0000DB40,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,    0,            3,  },  //ī��������ʱ�л����ظ�ʱ�������й�����
    {0x0000DB80,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,   12,            3,  },  //ī��������ʱ�л����ظ�ʱ�������޹�����
    {0x0000DBC0,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,    0,            2,  },  //ī��������ʱ�л����ظ�ʱ���ڵ�ѹ
    {0x0000DC00,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,    6,            3,  },  //ī��������ʱ�л����ظ�ʱ���ڵ���
    {0x0000DC40,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_V_I   ,     5+6+12+CURVE_RESERVE_SIZE ,     SAVE_FLAG_DENY_NO_SAVE,                     15,            3,  },  //ī��������ʱ�л����ظ�ʱ�������ߵ���
    {0x0000DCC0,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       0,                                           0,            4,  },  //ī��������ʱ�л����ظ�ʱ���������й��ܵ���
    {0x0000DD00,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_ZFXYWG,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      4,            4,  },  //ī��������ʱ�л����ظ�ʱ���ڷ����й��ܵ���
    {0x0000DC80,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_GL,         5+12+12+8+CURVE_RESERVE_SIZE,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,   24,            2,  },  //ī��������ʱ�л����ظ�ʱ�����ܹ�������
    {0x0000DD40,                 PIM_MEXICO_DAYLIGHT_SAVING_CURVE_WG1234,     5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      0,            4,  },  //ī��������ʱ�л����ظ�ʱ���ڵ�һ�����޹��ܵ���
    #endif
    
    #ifdef __INDONESIA_DLMS_TEST__
    {SS_PLN_SZGL,               PIM_CURVE_PLN_EX,     5+27+CURVE_RESERVE_SIZE,        SAVE_FLAG_DENY_NO_SAVE,                      0,            3,  },  //˲ʱ���й�����
    {SS_SZZXDL_SJK,             PIM_CURVE_PLN_EX,     5+27+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      3,            4,  },  //���������й��ܵ���
    {SS_SZFXDL_SJK,             PIM_CURVE_PLN_EX,     5+27+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      7,            4,  },  //���ڷ����й��ܵ���
    {A_ZXYG_DN,                 PIM_CURVE_PLN_EX,     5+27+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      11,            4,  },  //A�������й�
    {A_FXYG_DN,                 PIM_CURVE_PLN_EX,     5+27+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      15,            4,  },  //A�෴���й�
    {A_ZHWG1_DN,                PIM_CURVE_PLN_EX,     5+27+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      19,            4,  },  //(A�������޹�
    {A_ZHWG2_DN,                PIM_CURVE_PLN_EX,     5+27+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                      23,            4,  },  //(A�෴���޹�
    #endif

    #ifdef __HIGH_PRECISION_DATA__//�߾�������
    {(HIGH_PRECISION_ZXYG_DN_SJK-0x3F),                 PIM_EDMI_HIGH_PRECISION_ZFXYWG,     5+25+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                           0,              5,  },  //(��ǰ)�����й��ܵ���
    {(HIGH_PRECISION_FXYG_DN_SJK-0x3F),                 PIM_EDMI_HIGH_PRECISION_ZFXYWG,     5+25+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                           5,              5,  },  //(��ǰ)�����й��ܵ���
    {(HIGH_PRECISION_ZHWG1_DN_SJK-0x3F),                 PIM_EDMI_HIGH_PRECISION_ZFXYWG,     5+25+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                           10,             5,  },  //(��ǰ)����޹�1 �ܵ���
    {(HIGH_PRECISION_ZHWG2_DN_SJK-0x3F),                 PIM_EDMI_HIGH_PRECISION_ZFXYWG,     5+25+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                           15,             5,  },  //(��ǰ)����޹�2 �ܵ���
    #endif
};
#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
READ_WRITE_DATA const CRUVE_PHY_LIST_HUNAN[] =
{
    //������          ƫ��            ����=ʱ��+���ݳ���+Ԥ���ֽ�      flag                   block_offset  block_len
    {0x00001E00,    PIM_CURVE_V_HUNAN,       5+6+5 ,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,        0,            2,  },  //��ѹ
    {0x00001E40,    PIM_CURVE_I_HUNAN,       5+9+5 ,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,        0,            3,  },  //����
    {0x000021C0,    PIM_CURVE_ZERO_I_HUNAN,  5+3+5 ,               0,                                   0,            3,  },  //���ߵ���

    {0x00000040,    PIM_CURVE_ZXYG_HUNAN,    5+4+5 ,               0,                                   0,            4,  },  //�����й�

    {0x00001EBF,    PIM_CURVE_YGGL_HUNAN,    5+12+5 ,               0,                                   0,            12,  },  //�й�����
};
#endif

READ_WRITE_DATA const RECDAY_PHY_LIST[] =   //���һ��flag�ͳ�����Щ�鳭���漰��־λ��������������ţ�Ҳû����
{
    //������          ƫ��              ���� = ʱ��+����ʱ��+���ݳ���+Ԥ���ֽ�     flag                   block_offset  block_len
    #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
    {0x00003B80,    PIM_RECDAY_ZXYG,            3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00003BC0,    PIM_RECDAY_FXYG,            3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00003C00,    PIM_RECDAY_ZXWG,            3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00003C40,    PIM_RECDAY_FXWG,            3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    #else
    {0x00000040,    PIM_RECDAY_ZXYG,            3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00000080,    PIM_RECDAY_FXYG,            3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x000000C0,    PIM_RECDAY_ZXWG,            3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00000100,    PIM_RECDAY_FXWG,            3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    #endif
    #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
    {0x00003C80,    PIM_RECDAY_WG1,             3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00003CC0,    PIM_RECDAY_WG2,             3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00003D00,    PIM_RECDAY_WG3,             3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00003D40,    PIM_RECDAY_WG4,             3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    #else
    {0x00000140,    PIM_RECDAY_WG1,             3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00000180,    PIM_RECDAY_WG2,             3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x000001C0,    PIM_RECDAY_WG3,             3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    {0x00000200,    PIM_RECDAY_WG4,             3+5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },
    #endif
    #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
    {0x00004E80,    PIM_RECDAY_ZYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009B80,    PIM_RECDAY_ZYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009BC0,    PIM_RECDAY_ZYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    {0x00004EC0,    PIM_RECDAY_FYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009C80,    PIM_RECDAY_FYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009CC0,    PIM_RECDAY_FYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    {0x00004F00,    PIM_RECDAY_ZWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009C00,    PIM_RECDAY_ZWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009C40,    PIM_RECDAY_ZWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    {0x00004F40,    PIM_RECDAY_FWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009D00,    PIM_RECDAY_FWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009D40,    PIM_RECDAY_FWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    #else
    {0x00001400,    PIM_RECDAY_ZYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009B80,    PIM_RECDAY_ZYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009BC0,    PIM_RECDAY_ZYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    {0x00001440,    PIM_RECDAY_FYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009C80,    PIM_RECDAY_FYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009CC0,    PIM_RECDAY_FYG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    {0x00001480,    PIM_RECDAY_ZWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009C00,    PIM_RECDAY_ZWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009C40,    PIM_RECDAY_ZWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    {0x000014C0,    PIM_RECDAY_FWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009D00,    PIM_RECDAY_FWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009D40,    PIM_RECDAY_FWG_ZDXL,        3+5+40+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    #endif
};
/*���ȳ���150�ֽ�ʱ����Ҫ����ȡ�����еĻ��������ȣ�Ŀǰ������������������150�ֽ�*/
READ_WRITE_DATA const CUR_DATA_PHY_LIST[] =
{
    //������          ƫ��     ����      ״̬��                                      block_offset  block_len
    //{0x00001E00,    1792*12,    4+5+6,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,        0,            2,  },     //��ѹ
    //{0x00001E40,    1792*13,    4+5+9,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,        0,            3,  },

    //F170
    {0x0000AEC0,    PIM_CUR_READ_INFO,             4+5+18,   0,        0,            18, },

    //������          ƫ��                        ����    ״̬��     block_offset  block_len
    //F33
    {0x00000040,    RS485_REC_PHY_OFFSET_007F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x000000C0,    RS485_REC_PHY_OFFSET_00FF,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },  
    {0x00000140,    RS485_REC_PHY_OFFSET_017F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00000200,    RS485_REC_PHY_OFFSET_023F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, }, //20
    //F34
    {0x00000080,    RS485_REC_PHY_OFFSET_00BF,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00000100,    RS485_REC_PHY_OFFSET_013F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00000180,    RS485_REC_PHY_OFFSET_01BF,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x000001C0,    RS485_REC_PHY_OFFSET_01FF,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },//20
    //F25
    {0x00001E80,    RS485_REC_PHY_OFFSET_1EBF,     4+5+12,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,        0,            3, },
    {0x00001EC0,    RS485_REC_PHY_OFFSET_1EFF,     4+5+12,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,        0,            3, },
    {0x00001F40,    RS485_REC_PHY_OFFSET_1F7F,     4+5+8,    SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,        0,            2, },
    {0x00001E00,    RS485_REC_PHY_OFFSET_1E3F,     4+5+6,    SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,        0,            2, },
    {0x00001E40,    RS485_REC_PHY_OFFSET_1E7F,     4+5+9,    SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,        0,            3, },
    {0x000021C0,    RS485_REC_PHY_OFFSET_21C0,     4+5+3,    0,                                               0,            3,  },
    {0x00001F00,    RS485_REC_PHY_OFFSET_1F3F,     4+5+12,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4,        0,            3, }, //23

    //F26
    {0x00008580,    RS485_REC_PHY_OFFSET_8580,     4+5+3,   0,        0,            3,  },
    {0x00008640,    RS485_REC_PHY_OFFSET_8640,     4+5+3,   0,        0,            3,  },
    {0x00008700,    RS485_REC_PHY_OFFSET_8700,     4+5+3,   0,        0,            3,  },
    {0x00008480,    RS485_REC_PHY_OFFSET_8480,     4+5+3,   0,        0,            3,  },
    #if((defined __HUNAN_NEW_RECORDING__) && ((defined __PROVICE_HUNAN__) || (defined __PROVICE_JILIN__)))  //���Ϻͼ��ֵ�F26����ʧѹ��������ˣ�����ʡ�ݲ���//
    {RDJ_ZXYG_DNL_SJK,    RS485_REC_PHY_OFFSET_85C0,     4+5+3,   0,        0,           12,  },
    {A_SY_ZLJ_SJ,    RS485_REC_PHY_OFFSET_85C0,     4+5+3,   0,        0,            3,  },
    {B_SY_ZLJ_SJ,    RS485_REC_PHY_OFFSET_8680,     4+5+3,   0,        0,            3,  },
    {C_SY_ZLJ_SJ,    RS485_REC_PHY_OFFSET_8740,     4+5+3,   0,        0,            3,  },
    {SY_ZLJ_SJ,      RS485_REC_PHY_OFFSET_84C0,     4+5+3,   0,        0,            3,  },
	#else
    {RDJ_ZXYG_DNL_SJK,  RS485_REC_PHY_OFFSET_85C0, 4+5+3,   0,        0,            3,  },
    {0x000085C0,    RS485_REC_PHY_OFFSET_85C0,     4+5+3,   0,        0,            3,  },
    {0x00008680,    RS485_REC_PHY_OFFSET_8680,     4+5+3,   0,        0,            3,  },
    {0x00008740,    RS485_REC_PHY_OFFSET_8740,     4+5+3,   0,        0,            3,  },
    {0x000084C0,    RS485_REC_PHY_OFFSET_84C0,     4+5+3,   0,        0,            3,  },

    #endif
    {0x00008600,    RS485_REC_PHY_OFFSET_8600,     4+5+6,   0,        0,            6,  },
    {0x000086C0,    RS485_REC_PHY_OFFSET_86C0,     4+5+6,   0,        0,            6,  },
    {0x00008780,    RS485_REC_PHY_OFFSET_8780,     4+5+6,   0,        0,            6,  },
    {0x00008500,    RS485_REC_PHY_OFFSET_8500,     4+5+6,   0,        0,            6,  },
    {0x00008624,    RS485_REC_PHY_OFFSET_8624,     4+5+6,   0,        0,            6,  },
    {0x000086E4,    RS485_REC_PHY_OFFSET_86E4,     4+5+6,   0,        0,            6,  },
    {0x000087A4,    RS485_REC_PHY_OFFSET_87A4,     4+5+6,   0,        0,            6,  },
    {0x00008540,    RS485_REC_PHY_OFFSET_8540,     4+5+6,   0,        0,            6,  },// 16

    //F27
    {0x00005880,    RS485_REC_PHY_OFFSET_5880,     4+5+4,   0,        0,            4,  },
    {0x000058C0,    RS485_REC_PHY_OFFSET_58C0,     4+5+3+6,   0,        0,            3+6,  }, // ����6�ֽڵĵ�ǰϵͳʱ�䣬�����¼�ERC12
    {0x00008A80,    RS485_REC_PHY_OFFSET_8A80,     4+5+3,   0,        0,            3,  },
    {0x00008AC0,    RS485_REC_PHY_OFFSET_8AC0,     4+5+50,  0,        0,            50,  },
    {0x00008B00,    RS485_REC_PHY_OFFSET_8B00,     4+5+3,   0,        0,            3,  },
    {0x00008B40,    RS485_REC_PHY_OFFSET_8B40,     4+5+106, 0,        0,            106,  },
    {0x00008B80,    RS485_REC_PHY_OFFSET_8B80,     4+5+3,   0,        0,            3,  },
    {0x00008BC0,    RS485_REC_PHY_OFFSET_8BC0,     4+5+34,  0,        0,            34,  },
    {0x00008C00,    RS485_REC_PHY_OFFSET_8C00,     4+5+3,   0,        0,            3,  },
    {0x00008C40,    RS485_REC_PHY_OFFSET_8C40,     4+5+14,  0,        0,            14,  },
    {0x00008C80,    RS485_REC_PHY_OFFSET_8C80,     4+5+3,   0,        0,            3,  },
    {0x00008CC0,    RS485_REC_PHY_OFFSET_8CC0,     4+5+16,  0,        0,            16,  },
    {0x00002400,    RS485_REC_PHY_OFFSET_2400,     4+5+4,   0,        0,            4,  },
    {0x00007680,    RS485_REC_PHY_OFFSET_7680,     4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|14,        0,            3,  }, //����������14���������ڴ洢����Ȼ97ֻ��10��������ֻ��2����Ҳ����07�滮14��������07�Ĵ治��
    #ifdef __PROVICE_HEILONGJIANG__
    {0x00007640,    RS485_REC_PHY_OFFSET_76C0,     4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|14,        0,            3,  },
    #else
    {0x000076C0,    RS485_REC_PHY_OFFSET_76C0,     4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|14,        0,            3,  },
    #endif
    {0x000068C0,    RS485_REC_PHY_OFFSET_68C0,     4+5+2,   0,        0,            2,  },
    {0x00006100,    RS485_REC_PHY_OFFSET_6100,     4+5+3,   0,        0,            3,  },
    {0x00006140,    RS485_REC_PHY_OFFSET_6140,     4+5+3,   0,        0,            3,  },
    {0x00005E40,    RS485_REC_PHY_OFFSET_5E40,     4+5+3,   0,        0,            3,  },
    {0x00005E80,    RS485_REC_PHY_OFFSET_5E80,     4+5+3,   0,        0,            3,  },//20 
    
    //F49
    {0x00001F80,    RS485_REC_PHY_OFFSET_1FBF,     4+5+6,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,        0,            2, },//3
    //F28
    {0x000062C0,    RS485_REC_PHY_OFFSET_62FF,     4+5+18,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|7,        0,            2, }, //7
    //35
    {0x00001400,    RS485_REC_PHY_ZYG_ZDXL,        4+5+35,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009B80,    RS485_REC_PHY_ZYG_ZDXL,        4+5+35,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009BC0,    RS485_REC_PHY_ZYG_ZDXL,        4+5+35,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,}, 
    {0x00001480,    RS485_REC_PHY_ZWG_ZDXL,        4+5+35,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, }, 
    {0x00009C00,    RS485_REC_PHY_ZWG_ZDXL,        4+5+35,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009C40,    RS485_REC_PHY_ZWG_ZDXL,        4+5+35,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    {0x00001680,    RS485_REC_PHY_OFFSET_1680,     4+5+8,   0,        0,            8,  },
    {0x00001900,    RS485_REC_PHY_OFFSET_1900,     4+5+8,   0,        0,            8,  },
    {0x00001B80,    RS485_REC_PHY_OFFSET_1B80,     4+5+8,   0,        0,            8,  },//13
    //F36
    {0x00001440,    RS485_REC_PHY_FYG_ZDXL,     4+5+35,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00009C80,    RS485_REC_PHY_FYG_ZDXL,     4+5+35,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009CC0,    RS485_REC_PHY_FYG_ZDXL,     4+5+35,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    {0x000014C0,    RS485_REC_PHY_FWG_ZDXL,     4+5+35,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, }, //10
    {0x00009D00,    RS485_REC_PHY_FWG_ZDXL,     4+5+35,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    3,},
    {0x00009D40,    RS485_REC_PHY_FWG_ZDXL,     4+5+35,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,    15,    4,},
    
    //F39
    {0x00004E80,    RS485_REC_PHY_OFFSET_4EBF,     4+5+40,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00004F00,    RS485_REC_PHY_OFFSET_4F3F,     4+5+40,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },  //10
    //F40
    {0x00004EC0,    RS485_REC_PHY_OFFSET_4EFF,     4+5+40,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },
    {0x00004F40,    RS485_REC_PHY_OFFSET_4F7F,     4+5+40,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|SAVE_FLAG_XL_TIME|5,        0,            8, },//10
    //F37
    {0x00003B80,    RS485_REC_PHY_OFFSET_3BBF,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00003C00,    RS485_REC_PHY_OFFSET_3C3F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00003C80,    RS485_REC_PHY_OFFSET_3CBF,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00003D40,    RS485_REC_PHY_OFFSET_3D7F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },//20
    //F38
    {0x00003BC0,    RS485_REC_PHY_OFFSET_3BFF,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00003C40,    RS485_REC_PHY_OFFSET_3C7F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00003CC0,    RS485_REC_PHY_OFFSET_3CFF,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, },
    {0x00003D00,    RS485_REC_PHY_OFFSET_3D3F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, }, //20
    //F32
    {0x00003FC0,    RS485_REC_PHY_OFFSET_3FC0,     4+5+4,   0,        0,            4,  },
    {0x00004000,    RS485_REC_PHY_OFFSET_4000,     4+5+4,   0,        0,            4,  },
    {0x00004040,    RS485_REC_PHY_OFFSET_4040,     4+5+4,   0,        0,            4,  },
    {0x00004080,    RS485_REC_PHY_OFFSET_4080,     4+5+4,   0,        0,            4,  },
    {0x00004400,    RS485_REC_PHY_OFFSET_4400,     4+5+4,   0,        0,            4,  },
    {0x00004440,    RS485_REC_PHY_OFFSET_4440,     4+5+4,   0,        0,            4,  },
    {0x00004480,    RS485_REC_PHY_OFFSET_4480,     4+5+4,   0,        0,            4,  },
    {0x000044C0,    RS485_REC_PHY_OFFSET_44C0,     4+5+4,   0,        0,            4,  },
    {0x00004840,    RS485_REC_PHY_OFFSET_4840,     4+5+4,   0,        0,            4,  },
    {0x00004880,    RS485_REC_PHY_OFFSET_4880,     4+5+4,   0,        0,            4,  },
    {0x000048C0,    RS485_REC_PHY_OFFSET_48C0,     4+5+4,   0,        0,            4,  },
    {0x00004900,    RS485_REC_PHY_OFFSET_4900,     4+5+4,   0,        0,            4,  },//12
    //F161������                 
    //{0x000062C2,    RS485_REC_PHY_OFFSET_62C2,     4+5+2,   0,        0,            2,  },//�ظ�

    //F165//F166//F167//f162û��
   
    //F177
    //{0x00000000,    RS485_REC_PHY_OFFSET_003F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, }, //5
    //F178
    //{0x00003B40,    RS485_REC_PHY_OFFSET_3B7F,     4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,        0,            4, }, //5
    //F31
    {0x00000480,    RS485_REC_PHY_OFFSET_0480,     4+5+4,   0,        0,            4,  },
    {0x000004C0,    RS485_REC_PHY_OFFSET_04C0,     4+5+4,   0,        0,            4,  },
    {0x00000500,    RS485_REC_PHY_OFFSET_0500,     4+5+4,   0,        0,            4,  },
    {0x00000540,    RS485_REC_PHY_OFFSET_0540,     4+5+4,   0,        0,            4,  },
    {0x000008C0,    RS485_REC_PHY_OFFSET_08C0,     4+5+4,   0,        0,            4,  },
    {0x00000900,    RS485_REC_PHY_OFFSET_0900,     4+5+4,   0,        0,            4,  },
    {0x00000940,    RS485_REC_PHY_OFFSET_0940,     4+5+4,   0,        0,            4,  },
    {0x00000980,    RS485_REC_PHY_OFFSET_0980,     4+5+4,   0,        0,            4,  },
    {0x00000D00,    RS485_REC_PHY_OFFSET_0D00,     4+5+4,   0,        0,            4,  },
    {0x00000D40,    RS485_REC_PHY_OFFSET_0D40,     4+5+4,   0,        0,            4,  },
    {0x00000D80,    RS485_REC_PHY_OFFSET_0D80,     4+5+4,   0,        0,            4,  },
    {0x00000DC0,    RS485_REC_PHY_OFFSET_0DC0,     4+5+4,   0,        0,            4,  }, //12
    //ERC8
    {0x00008D00,    PIM_CUR_SDBBCZCS,                4+5+3,   0,        0,            3,  },
    {0x00008D40,    PIM_CUR_LAST_SDBBCJU,            4+5+6,  0,        0,            6,  }, //ֻ�淢��ʱ��
    //ERC37
    {0x00008D80,    PIM_CUR_KBGZCS,                4+5+3,   0,        0,            3,  },
    {0x00008DC0,    PIM_CUR_LAST_KBGJL,            4+5+60,  0,        0,            60,  },
    // ERC38
    {0x0000A680,    PIM_CUR_KDNHZCS,               4+5+3,   0,        0,            3,  },
    {0x0000A6C0,    PIM_CUR_LAST_KDNHJL,           4+5+60,  0,        0,            60,  },
    //ERC40
    {0x0000AF80,    PIM_CUR_CCGRZCS,               4+5+3,   0,        0,            3,  },

    {SY_ZCS,        PIM_CUR_SYZCS,                 4+5+3,   0,        0,            3,  },
    {QSY_ZCS_ZLJ_SJ,PIM_CUR_QSYZCS,                4+5+6,   0,        0,            6,  },
    {DL_A_ZCS,      PIM_CUR_DLAZCS,                4+5+3,   0,        0,            3,  },
    {DL_B_ZCS,      PIM_CUR_DLBZCS,                4+5+3,   0,        0,            3,  },
    {DL_C_ZCS,      PIM_CUR_DLCZCS,                4+5+3,   0,        0,            3,  },

    //�����¼
    {0x0000AF00,    PIM_CUR_DDCS_Z,          4+5+3+5,      SAVE_FLAG_DENY_NO_SAVE,                      0,  3,},//	0x13110000 //3, �������
    {0x0000AF40,    PIM_CUR_DDJL,            4+5+120+5,    SAVE_FLAG_DENY_NO_SAVE|9,                      0,  12,},//	0x13110001 //12, ��1�ε����¼
    
    //г����ֻ�н��ɳ�г���������ɳ���������������г�����棬���ж˿ڶ����ķ���г��ǰ��
    {0x00002040,    PIM_CUR_HARMONIC_AXDYXB,       4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|21,      0,      2,  },
    {0x00002080,    PIM_CUR_HARMONIC_BXDYXB,       4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|21,      0,      2,  },
    {0x000020C0,    PIM_CUR_HARMONIC_CXDYXB,       4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|21,      0,      2,  },
    {0x00002100,    PIM_CUR_HARMONIC_AXDLXB,       4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|21,      0,      2,  },
    {0x00002140,    PIM_CUR_HARMONIC_BXDLXB,       4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|21,      0,      2,  },
    {0x00002180,    PIM_CUR_HARMONIC_CXDLXB,       4+5+42,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|21,      0,      2,  },
    //����״̬��
    {0x0000A880,    PIM_CUR_CYMETER_STAT_WORD,     4+5+4,   0,      0,      4,  },

   // {S1C_GDH_ZGD_JE_CS,    PIM_CUR_GDCS,     4+5+2,   0,      0,      2,  },
   // {S1C_GDH_LJGD_JE,      PIM_CUR_LJGDJE,   4+5+4,   0,      0,      4,  },
    //{SY_DL,                PIM_CUR_SYDL,     4+5+4,   0,      0,      4,  },

    {CY_XJ_SJK,            PIM_CUR_CY_XJ,     4+5+12,   0,      0,      12,  },

    //һ��ʣ������ӱ������������ó�������ʱ�����ӱ�����  ������Ҫǰ3��
    //F225 F215 F210 ���¶��Ṻ�õ���Ϣ
    {0x00008FC0,   PIM_CUR_GDCS_PREPAY,     4+5+2,      SAVE_FLAG_DENY_NO_SAVE,                     0,   2,},	//0x03330201      //�������
    {0x00001240,   PIM_CUR_SYJE_PREPAY,     4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   5,},	//0x00900200      //ʣ����
    #if(defined __PROVICE_NEIMENG__)/*������͸֧������ռ���ۼƹ�����Ĵ洢λ��*/
    {TZ_JE,        PIM_CUR_LJGDJE_PREPAY,   4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   5,},	//0x00900201      //͸֧���
    #else
    {0x000090C0,   PIM_CUR_LJGDJE_PREPAY,   4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   5,},	//0x03330601      //�ۼƹ�����
    #endif
    {0x00001140,   PIM_CUR_SYDL_PREPAY,     4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900100      //ʣ�����
    {0x00001180,   PIM_CUR_TZDL_PREPAY,     4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900101      //͸֧����
    {0x00008F40,   PIM_CUR_LJGDL_PREPAY,    4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x00900102      //�ۼƹ�����
    {0x000070C0,   PIM_CUR_SQMXDL_PREPAY,   4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F04      //��Ƿ���޵���
    {0x00007000,   PIM_CUR_BJDL_PREPAY,     4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F01      //��������
    {0x00007040,   PIM_CUR_GZDL_PREPAY,     4+5+4,      SAVE_FLAG_DENY_NO_SAVE,                     0,   4,},	//0x04000F02      //���ϵ���

    {0x00000000,   RS485_REC_PHY_CUR_ZHYG,           4+5+20,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,},//(��ǰ)����й��������ݿ�
    //����ʧ���  F58
    {(DY_BXSZD_SJK-63),    PIM_CUR_DY_BXSZD,       4+5+6,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,      0,      2,  },
    {(DL_BXSZD_SJK-63),    PIM_CUR_DL_BXSZD,       4+5+6,  SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3,      0,      2,  },

    #if (defined __PROVICE_JIANGSU__)
    //03050000  07 ȫʧѹ�ܴ��� ���ۼ�ʱ��   ʣ����������� ʣ����������ܴ���  ����6���ֽ� ���������仯��
    //{QSY_ZCS_ZLJ_SJ,                 PIM_CUR_RESIDUAL_CURRENT_OVERRUN_CNT,      4+5+6,      SAVE_FLAG_DENY_NO_SAVE,      0,   6,},
    //03050001 ...0A
    {(S1C_QSY_FSSK_DLZ_JSSK-0x3F),   PIM_CUR_RESIDUAL_CURRENT_OVERRUN_RECORD,   4+5+150,    SAVE_FLAG_DENY_NO_SAVE|9,    0,   15,},
    //03060000  07 ������Դʧ���ܴ��������ۼ�ʱ��	ʣ����������� ʣ�����������·�����ܴ���  ����6���ֽ� ���������仯��
    {FZDY_SD_ZCS_ZLJ_SJ,             PIM_CUR_RESIDUAL_CURRENT_BREAK_CNT,        4+5+6,      SAVE_FLAG_DENY_NO_SAVE,      0,   6,},
    {(S1C_FZDY_SD_FSSK_JSSK-0x3F),   PIM_CUR_RESIDUAL_CURRENT_BREAK_RECORD,     4+5+120,    SAVE_FLAG_DENY_NO_SAVE|9,    0,   12,},
    
//ʣ���·������5���Ӷ�ȡ
    //{DBYXZTZ_SJK,               PIM_CUR_BREAK_YXZTZ,            4+5+2,      SAVE_FLAG_DENY_NO_SAVE,                     0,   2,}, //ʣ���������������״̬��Ϊ2��,����������͵�����ͬ���޸�һ�£���Ҫ�ظ���������Ҫע��洢ƫ�ƣ���
    {SYDL_SJK,                  PIM_CUR_BREAK_ZDXJSYDLZ,        4+5+3,      SAVE_FLAG_DENY_NO_SAVE,                     0,   3,},  //�����ݣ���һ��ת������ʣ�����ֵ���Ǹ�û������࣡��
    
    //ʣ�����������Сʱ��ȡ ����;�Ǹ������ݺ�ͳ�ƣ���485��Ҳ����5���Ӷ�ȡ��ÿСʱ����ͳ�ƺ���  ����ѹ������ǰ���Ѿ�����������
    {SYDL_ZDX_ZDZ_FSSK_SJK-63,         PIM_CUR_BREAK_DRZDXZDZJSK,      4+5+9,      SAVE_FLAG_DENY_NO_SAVE,                     0,   9,}, //����F201
    {AX_DL_RZD_FSSK_SJK-63,            PIM_CUR_BREAK_DLRZDZFSSK_A,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,}, //����F202
    {BX_DL_RZD_FSSK_SJK-63,            PIM_CUR_BREAK_DLRZDZFSSK_B,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {CX_DL_RZD_FSSK_SJK-63,            PIM_CUR_BREAK_DLRZDZFSSK_C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    //�¼���أ��¼����ɷ���������״̬���Ժ󣬸���״̬��ֵ�������������룻��������������������������ֵ�ü�¼�����룻���ڳ������֮���¼����ɺ�����ȡ���ڳ���洢�����������¼���
    {TZCS_CS_MK_SJK,          	PIM_CUR_BREAK_TCCS,                 4+5+25,      SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {SYDL_CX_SJ_SJK-63,                PIM_CUR_BREAK_SYDLCXSJJL_S1C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+1,              PIM_CUR_BREAK_SYDLCXSJJL_S2C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+2,              PIM_CUR_BREAK_SYDLCXSJJL_S3C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+3,              PIM_CUR_BREAK_SYDLCXSJJL_S4C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+4,              PIM_CUR_BREAK_SYDLCXSJJL_S5C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+5,              PIM_CUR_BREAK_SYDLCXSJJL_S6C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+6,              PIM_CUR_BREAK_SYDLCXSJJL_S7C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+7,              PIM_CUR_BREAK_SYDLCXSJJL_S8C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+8,              PIM_CUR_BREAK_SYDLCXSJJL_S9C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},
    {SYDL_CX_SJ_SJK-63+9,              PIM_CUR_BREAK_SYDLCXSJJL_S10C,     4+5+15,     SAVE_FLAG_DENY_NO_SAVE,                     0,   15,},

    //{TZCS_CS_MK+11,             PIM_CUR_BREAK_XLQX_XZCS,         4+5+1,      SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    {XLQX_JL_SJK-63,                   PIM_CUR_BREAK_XLQXSJJL_S1C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+1,                 PIM_CUR_BREAK_XLQXSJJL_S2C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+2,                 PIM_CUR_BREAK_XLQXSJJL_S3C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+3,                 PIM_CUR_BREAK_XLQXSJJL_S4C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+4,                 PIM_CUR_BREAK_XLQXSJJL_S5C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+5,                 PIM_CUR_BREAK_XLQXSJJL_S6C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+6,                 PIM_CUR_BREAK_XLQXSJJL_S7C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+7,                 PIM_CUR_BREAK_XLQXSJJL_S8C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+8,                 PIM_CUR_BREAK_XLQXSJJL_S9C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {XLQX_JL_SJK-63+9,                 PIM_CUR_BREAK_XLQXSJJL_S10C,    4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},

    //{TZCS_CS_MK+12,             PIM_CUR_BREAK_SYDLBHTC_XZCS,        4+5+1,      SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    {SYDL_BJ_SJ_SJK-63,                PIM_CUR_BREAK_SYDLBHTCSJJL_S1C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+1,              PIM_CUR_BREAK_SYDLBHTCSJJL_S2C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+2,              PIM_CUR_BREAK_SYDLBHTCSJJL_S3C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+3,              PIM_CUR_BREAK_SYDLBHTCSJJL_S4C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+4,              PIM_CUR_BREAK_SYDLBHTCSJJL_S5C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+5,              PIM_CUR_BREAK_SYDLBHTCSJJL_S6C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+6,              PIM_CUR_BREAK_SYDLBHTCSJJL_S7C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+7,              PIM_CUR_BREAK_SYDLBHTCSJJL_S8C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+8,              PIM_CUR_BREAK_SYDLBHTCSJJL_S9C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_BJ_SJ_SJK-63+9,              PIM_CUR_BREAK_SYDLBHTCSJJL_S10C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},

    //{TZCS_CS_MK+13,             PIM_CUR_BREAK_CXDSD_XZCS,      4+5+1,      SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    {CXD_SD_JL_SJK-63,                 PIM_CUR_BREAK_CXDSDJL_S1C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+1,               PIM_CUR_BREAK_CXDSDJL_S2C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+2,               PIM_CUR_BREAK_CXDSDJL_S3C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+3,               PIM_CUR_BREAK_CXDSDJL_S4C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+4,               PIM_CUR_BREAK_CXDSDJL_S5C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+5,               PIM_CUR_BREAK_CXDSDJL_S6C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+6,               PIM_CUR_BREAK_CXDSDJL_S7C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+7,               PIM_CUR_BREAK_CXDSDJL_S8C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+8,               PIM_CUR_BREAK_CXDSDJL_S9C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},
    {CXD_SD_JL_SJK-63+9,               PIM_CUR_BREAK_CXDSDJL_S10C,     4+5+13,     SAVE_FLAG_DENY_NO_SAVE,                     0,   13,},

    //{TZCS_CS_MK+14,             PIM_CUR_BREAK_SYDLCYHLDX_XZCS,        4+5+1,      SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    {SYDL_CYHL_DX_JL_SJK-63,           PIM_CUR_BREAK_SYDLCYHLDXSJJL_S1C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+1,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S2C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+2,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S3C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+3,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S4C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+4,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S5C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+5,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S6C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+6,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S7C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+7,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S8C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+8,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S9C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {SYDL_CYHL_DX_JL_SJK-63+9,         PIM_CUR_BREAK_SYDLCYHLDXSJJL_S10C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},

    //{TZCS_CS_MK+1,              PIM_CUR_BREAK_TZCS,        4+5+2,      SAVE_FLAG_DENY_NO_SAVE,                     0,   2,},
    //��բ�¼������ض�ȡ��բ�ܴ�����ʹ��״̬���е���բ����ָ�뼴�ɣ�Э����ע���ġ�
    {BHQ_TZ_SJ_SJK-63,                 PIM_CUR_BREAK_TZJL_S1C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+1,               PIM_CUR_BREAK_TZJL_S2C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+2,               PIM_CUR_BREAK_TZJL_S3C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+3,               PIM_CUR_BREAK_TZJL_S4C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+4,               PIM_CUR_BREAK_TZJL_S5C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+5,               PIM_CUR_BREAK_TZJL_S6C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+6,               PIM_CUR_BREAK_TZJL_S7C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+7,               PIM_CUR_BREAK_TZJL_S8C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+8,               PIM_CUR_BREAK_TZJL_S9C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},
    {BHQ_TZ_SJ_SJK-63+9,               PIM_CUR_BREAK_TZJL_S10C,     4+5+25,     SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},

    // ERC55 56 57 58 61 ������ֳ��� 
    {TZCS_CS_MK_A,               PIM_CUR_BREAK_TZCS_CS_MK_A,     4+5+1,     SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    {TZCS_CS_MK_B,               PIM_CUR_BREAK_TZCS_CS_MK_B,     4+5+1,     SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    {TZCS_CS_MK_C,               PIM_CUR_BREAK_TZCS_CS_MK_C,     4+5+1,     SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    {TZCS_CS_MK_D,               PIM_CUR_BREAK_TZCS_CS_MK_D,     4+5+1,     SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    {TZCS_CS_MK_E,               PIM_CUR_BREAK_TZCS_CS_MK_E,     4+5+1,     SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},
    
#endif
    {METER_SJ_READ_TIME,    RS485_REC_PHY_OFFSET_58C0,     4+5+3+6,   0,       0,            3+6,  }, //�������ʱ��ʱ��������ʱ��
    
    //erc10
    //ʧѹ
    {A_SY_ZCS,      PIM_CUR_ABC_SY_ZCS_LJSJ,        4+5+21,     0,        0,            3,},
    {A_SY_ZLJ_SJ,   PIM_CUR_ABC_SY_ZCS_LJSJ,        4+5+21,     0,        3,            3,},
    {B_SY_ZCS,      PIM_CUR_ABC_SY_ZCS_LJSJ,        4+5+21,     0,        6,            3,},
    {B_SY_ZLJ_SJ,   PIM_CUR_ABC_SY_ZCS_LJSJ,        4+5+21,     0,        7,            3,},
    {C_SY_ZCS,      PIM_CUR_ABC_SY_ZCS_LJSJ,        4+5+21,     0,        12,           3,},
    {C_SY_ZLJ_SJ,   PIM_CUR_ABC_SY_ZCS_LJSJ,        4+5+21,     0,        15,           3,},
    {ABC_DX_LJSJ,   PIM_CUR_ABC_SY_ZCS_LJSJ,        4+5+21,     0,        0,            18,},
    {SY_ZLJ_SJ,     PIM_CUR_ABC_SY_ZCS_LJSJ,        4+5+21,     0,        18,           3,},
    
    {S1C_A_SY_SJK,      PIM_CUR_S1C_A_SY_SJK,     4+5+195,   0,        0,            195,  },//����һ��洢��
    {S1C_B_SY_SJK,      PIM_CUR_S1C_B_SY_SJK,     4+5+195,   0,        0,            195,  },
    {S1C_C_SY_SJK,      PIM_CUR_S1C_C_SY_SJK,     4+5+195,   0,        0,            195,  },
    {S1C_A_SYJL_07,     PIM_CUR_S1C_A_SY_SJK,     4+5+169,   0,        0,            169,  },
    {S1C_B_SYJL_07,     PIM_CUR_S1C_B_SY_SJK,     4+5+169,   0,        0,            169,  },
    {S1C_C_SYJL_07,     PIM_CUR_S1C_C_SY_SJK,     4+5+169,   0,        0,            169,  },
    
    //���࣬����ʱ����F26���Ѿ�����
    {S1C_A_DX_SJK,    PIM_CUR_S1C_A_DX_SJK,     4+5+195,   0,        0,            195,  },
    {S1C_B_DX_SJK,    PIM_CUR_S1C_B_DX_SJK,     4+5+195,   0,        0,            195,  },
    {S1C_C_DX_SJK,    PIM_CUR_S1C_C_DX_SJK,     4+5+195,   0,        0,            195,  },
    
    //��ѹ�ϸ���ͳ��
    {DY_A_HGL,    PIM_CUR_DY_A_HGL,             4+5+27,   0,        0,            27,  },//���µ�ѹ�ϸ���ͳ������
    {DY_B_HGL,    PIM_CUR_DY_B_HGL,             4+5+27,   0,        0,            27,  },//���µ�ѹ�ϸ���ͳ������
    {DY_C_HGL,    PIM_CUR_DY_C_HGL,             4+5+27,   0,        0,            27,  },//���µ�ѹ�ϸ���ͳ������
    
    {ZJ1C_SY_FSSK,           PIM_ZJ1C_SY_FSSK,     4+5+6,     0,                     0,   6,},
    {( S1C_A_SY_SJK-63),     PIM_S1C_A_SY_KSSK,     4+5+6,     0,                     0,   6,},
    {( S1C_B_SY_SJK-63),     PIM_S1C_B_SY_KSSK,     4+5+6,     0,                     0,   6,},
    {( S1C_C_SY_SJK-63),     PIM_S1C_C_SY_KSSK,     4+5+6,     0,                     0,   6,},

    {ZJ1C_SY_JSSK,           PIM_ZJ1C_SY_JSSK,     4+5+6,     0,                     0,   6,},
    {( S1C_A_SY_SJK-27),     PIM_S1C_A_SY_JSSK,     4+5+6,     0,                     0,   6,},
    {( S1C_B_SY_SJK-27),     PIM_S1C_B_SY_JSSK,     4+5+6,     0,                     0,   6,},
    {( S1C_C_SY_SJK-27),     PIM_S1C_C_SY_JSSK,     4+5+6,     0,                     0,   6,},
    #ifdef __FUJIAN_CURRENT_BREAK__
    {FUJIAN_CUR_BREAK_QLZCS,           PIM_CUR_BREAK_FUJIAN_QL_CS,        4+5+3,      SAVE_FLAG_DENY_NO_SAVE,                     0,   3,},     //�����ܴ���
    {(FUJIAN_CUR_BREAK_QL_SLK-63),                PIM_CUR_BREAK_FUJIAN_QL_JL_S1C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+1,              PIM_CUR_BREAK_FUJIAN_QL_JL_S2C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+2,              PIM_CUR_BREAK_FUJIAN_QL_JL_S3C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+3,              PIM_CUR_BREAK_FUJIAN_QL_JL_S4C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+4,              PIM_CUR_BREAK_FUJIAN_QL_JL_S5C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+5,              PIM_CUR_BREAK_FUJIAN_QL_JL_S6C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+6,              PIM_CUR_BREAK_FUJIAN_QL_JL_S7C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+7,              PIM_CUR_BREAK_FUJIAN_QL_JL_S8C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+8,              PIM_CUR_BREAK_FUJIAN_QL_JL_S9C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},
    {FUJIAN_CUR_BREAK_QL_SLK-63+9,              PIM_CUR_BREAK_FUJIAN_QL_JL_S10C,     4+5+10,     SAVE_FLAG_DENY_NO_SAVE,                     0,   10,},

    {FUJIAN_CUR_BREAK_DDZCS,               PIM_CUR_BREAK_FUJIAN_DD_CS,         4+5+3,     SAVE_FLAG_DENY_NO_SAVE,                     0,   3,},     //�����ܴ���
    {(FUJIAN_CUR_BREAK_DD_SLK-63),         PIM_CUR_BREAK_FUJIAN_DD_JL_S1C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+1,         PIM_CUR_BREAK_FUJIAN_DD_JL_S2C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+2,         PIM_CUR_BREAK_FUJIAN_DD_JL_S3C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+3,         PIM_CUR_BREAK_FUJIAN_DD_JL_S4C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+4,         PIM_CUR_BREAK_FUJIAN_DD_JL_S5C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+5,         PIM_CUR_BREAK_FUJIAN_DD_JL_S6C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+6,         PIM_CUR_BREAK_FUJIAN_DD_JL_S7C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+7,         PIM_CUR_BREAK_FUJIAN_DD_JL_S8C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+8,         PIM_CUR_BREAK_FUJIAN_DD_JL_S9C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},
    {FUJIAN_CUR_BREAK_DD_SLK-63+9,         PIM_CUR_BREAK_FUJIAN_DD_JL_S10C,     4+5+12,     SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},

    {FUJIAN_CUR_BREAK_HZZCS,               PIM_CUR_BREAK_FUJIAN_HZ_CS,        4+5+3,      SAVE_FLAG_DENY_NO_SAVE,                     0,   3,},     //��բ�ܴ���
    {FUJIAN_CUR_BREAK_HZ_SLK-63,           PIM_CUR_BREAK_FUJIAN_HZ_JL_S1C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+1,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S2C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+2,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S3C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+3,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S4C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+4,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S5C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+5,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S6C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+6,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S7C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+7,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S8C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+8,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S9C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,   7,},
    {FUJIAN_CUR_BREAK_HZ_SLK-63+9,         PIM_CUR_BREAK_FUJIAN_HZ_JL_S10C,     4+5+7,     SAVE_FLAG_DENY_NO_SAVE,                     0,  7,},

    {FUJIAN_CUR_BREAK_ZTZ,                 PIM_CUR_BREAK_FUJIAN_ZTZ,            4+5+1,      SAVE_FLAG_DENY_NO_SAVE,                     0,   1,},     //״̬��
    {FUJIAN_CURRENT_BREAK_TRIP_JL_1C,      PIM_CUR_BREAK_FUJIAN_TZ_JL_S1C,      4+5+25,      SAVE_FLAG_DENY_NO_SAVE,                     0,   25,},     //��բ��¼
    {FUJIAN_CURRENT_BREAK_REMAIN_I_JL_1C,  PIM_CUR_BREAK_FUJIAN_SYDL_JL_S1C,    4+5+12,      SAVE_FLAG_DENY_NO_SAVE,                     0,   12,},     //ʣ���¼
    {FUJIAN_CURRENT_BREAK_CHECK_JL_1C,     PIM_CUR_BREAK_FUJIAN_ZJ_JL_S1C,      4+5+26,      SAVE_FLAG_DENY_NO_SAVE,                     0,   26,},     //�Լ��¼
    #endif
    #ifdef __INDONESIA_DLMS_TEST__
    {DLMS_CLOCK_JL,  PIM_DLMS_CLOCK_JL,    4+5+71,      SAVE_FLAG_DENY_NO_SAVE,                     0,   71,},     //dlms���ʱ��¼
    {DLMS_COVER_JL,  PIM_DLMS_COVER_JL,    4+5+71,      SAVE_FLAG_DENY_NO_SAVE,                     0,   71,},     //dlms���Ǽ�¼
    #endif
};
//---------------------------------------------------------------------------
//�ܼ������  ,�ݶ�ÿ���ܼ���һ���ļ�����ǰ��ֵ����һ�㣻������������8����һ���ļ�����ǰ��ֵ��8���㣬�����ܼ������ȷ��ƫ������
READ_WRITE_DATA const AGROUP_CUR_DATA_PHY_LIST[] =
{
    {0x000092C0,    PIM_AGP_CUR_DAY_YG,       5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },//�����ܼ��й�������(�ܡ�����1~4) //��������Ҫ��չ
    {0x00009300,    PIM_AGP_CUR_DAY_WG,       5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },//�����ܼ��޹�������(�ܡ�����1~4)  
    {0x000093C0,    PIM_AGP_CUR_MONTH_YG,     5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },//�����ܼ��й�������(�ܡ�����1~4) 
    {0x00009400,    PIM_AGP_CUR_MONTH_WG,     5+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },//�����ܼ��޹�������(�ܡ�����1~4) 
    {0x0000B500,    PIM_AGP_CUR_YGGL,         5+2+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    2,  },//��ǰ�ܼ��й�����
    {0x0000B540,    PIM_AGP_CUR_WGGL,         5+2+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    2,  },//��ǰ�ܼ��޹�����
    {SY_DL,         PIM_AGP_LEFT_AMOUNT,         5+4+5,   SAVE_FLAG_DENY_NO_SAVE,     0,    4,  },//��ǰ�ܼ��޹�����
};
READ_WRITE_DATA const AGROUP_CYCLE_DAY_DATA_PHY_LIST[] =
{
    {0x000092C0,    PIM_AGP_DAY_HOLD_YG,       4+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },//�ܼ����ۼ��й�������(�ܡ�����1~4) //��������Ҫ��չ
    {0x00009300,    PIM_AGP_DAY_HOLD_WG,       4+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },//�ܼ����ۼ��޹�������(�ܡ�����1~4)  
    {0x0000B100,    PIM_C2_F57,                3+12,     0,     0,    12 ,  },//2��F57
};
READ_WRITE_DATA const AGROUP_CYCLE_MONTH_DATA_PHY_LIST[] =
{
    {0x000093C0,    PIM_AGP_MONTH_HOLD_YG,     3+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },//�ܼ����ۼ��й�������(�ܡ�����1~4) //��������Ҫ��չ
    {0x00009400,    PIM_AGP_MONTH_HOLD_WG,     3+20+5,   SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4 ,  },//�ܼ����ۼ��޹�������(�ܡ�����1~4)  
    {0x0000B100,    PIM_C2_F60,                2+12,     0,     0,    12 ,  },//2��F60
};
READ_WRITE_DATA const AGROUP_CURVE_PHY_LIST[] =
{
    {0x000092C0,    PIM_AGP_CURVE_YG,       5+4+5,   0,     0,    4 ,  },//�����ܼ��й�������(�ܡ�����1~4) //��������Ҫ��չ
    {0x00009300,    PIM_AGP_CURVE_WG,       5+4+5,   0,     0,    4 ,  },//�����ܼ��޹�������(�ܡ�����1~4) 
    {0x0000B500,    PIM_AGP_CURVE_YGGL,     5+2+5,   0,     0,    2,  },//��ǰ�ܼ��й�����
    {0x0000B540,    PIM_AGP_CURVE_WGGL,     5+2+5,   0,     0,   2,  },//��ǰ�ܼ��޹�����
};
READ_WRITE_DATA const LAST_CURVE_CYCLE_DAY_PHY_LIST[] =
{
    //������          ƫ��            ����=ʱ��+���ݳ���+Ԥ���ֽ�      flag                   block_offset  block_len
    {SYC_ZDDJ_YG,    PIM_LAST_CURVE_DAY_TMP1,     5+10+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,    0,            10,  },  //(��1��)���㶳�������й��ܵ���
    {SYC_ZDDJ_WG,    PIM_LAST_CURVE_DAY_TMP2   ,  5+10+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,    0,            10,  },  //

    {SYC_GXGY_YG,    PIM_LAST_CURVE_DAY_TMP3   ,  5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,    0,            16,  },  //

    //Ҫ�ø��ɼ�¼����     
    {(FHJL1_DY_14-0x3F),         PIM_LAST_CURVE_DAY_TMP1   ,  5+8+17+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3, 0,  2, }, // 061001FF 01 02 03 ��ѹ����
    {(FHJL2_DL_14-0x3F),         PIM_LAST_CURVE_DAY_TMP1   ,  5+8+17+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|3, 0,  3, }, // 061002FF 01 02 03 ��������
    {FHJL1_DY_DL_PL,             PIM_LAST_CURVE_DAY_TMP1   ,  5+8+17+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                   0, 30, },  //
     
    {(FHJL3_YGGL_14-0x3F),       PIM_LAST_CURVE_DAY_TMP2   ,  5+8+24+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4, 0,  4, }, // 061003FF 00 01 02 03 �й����ʸ���
    {(FHJL4_WGGL_14-0x3F),       PIM_LAST_CURVE_DAY_TMP2   ,  5+8+24+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4, 0,  4, }, // 061004FF 00 01 02 03 �޹����ʸ���
    {FHJL2_YG_WG_GL,             PIM_LAST_CURVE_DAY_TMP2   ,  5+8+24+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                   0, 40, },  //
     
    {(FHJL5_GLYS_14-0x3F),       PIM_LAST_CURVE_DAY_TMP3   ,  5+8+8+CURVE_RESERVE_SIZE ,        SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4, 0,  2, }, // 061005FF 00 01 02 03 ������������
    {FHJL3_GL_YS,                PIM_LAST_CURVE_DAY_TMP3   ,  5+8+3+CURVE_RESERVE_SIZE ,        SAVE_FLAG_DENY_NO_SAVE,                   0, 20, },  //
     
    {(FHJL6_YG_WG_ZDN_14-0x3F),  PIM_LAST_CURVE_DAY_TMP4   ,  5+8+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4, 0,  4, },  //061006FF 01 02 03 04 ���޹����ܸ���
    {FHJL4_ZFXYG,                PIM_LAST_CURVE_DAY_TMP4   ,  5+8+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                   0, 30, },  //
    
    {(FHJL7_SXX_WG_ZDN_14-0x3F), PIM_LAST_CURVE_DAY_TMP5   ,  5+8+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|4, 0,  4, },  //061007FF 01 02 03 04 �������޹�����
    {FHJL5_SXX_WG,               PIM_LAST_CURVE_DAY_TMP5   ,  5+8+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,                   0, 30, },  //
    
    {(FHJL8_DQXL_14-0x3F),       PIM_LAST_CURVE_DAY_TMP6   ,  5+8+6+CURVE_RESERVE_SIZE ,        SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|2, 0,  3, },  // 061008FF 00 01 ��������   
    {FHJL6_XL,                   PIM_LAST_CURVE_DAY_TMP6   ,  5+8+6+CURVE_RESERVE_SIZE ,        SAVE_FLAG_DENY_NO_SAVE,                   0, 30, },  //
//     {FHJL5_WG,          PIM_LAST_CURVE_DAY_TMP2   ,  5+16+CURVE_RESERVE_SIZE ,       SAVE_FLAG_DENY_NO_SAVE,    0,            20,  },  //
     {ISRAEL_LOAD_PROFILE,  PIM_ISRAEL_LOAD_PRO   ,   5+8+6+CURVE_RESERVE_SIZE ,          SAVE_FLAG_DENY_NO_SAVE,    0,            96,  },  //
     {ISRAEL_HOUR_DATA, PIM_ISRAEL_CURDATA_CURVE ,  6+5+45+RESERVE_DATA,    SAVE_FLAG_DENY_NO_SAVE,                      0,  45, },   //	0x02800113 //��1Сʱ���ɼ�¼

};
INT16U get_phy_form_list_agp_cur_data(INT32U phy,READ_WRITE_DATA* out,INT16U *block_begin_idx,INT8U *block_count)
{
    INT8U idx,idx_sub;
    INT16U mask_idx;

    mask_idx = 0;
    *block_begin_idx = 0;
    *block_count = 0;

    for(idx=0;idx<sizeof(AGROUP_CUR_DATA_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        *block_begin_idx = mask_idx;
        *block_count = (AGROUP_CUR_DATA_PHY_LIST[idx].flag&0x1F)+1 ;
        for(idx_sub=0;idx_sub<((AGROUP_CUR_DATA_PHY_LIST[idx].flag&0x1F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(AGROUP_CUR_DATA_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFFFF;
}
INT8U writedata_agp_cur_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U *buffer)
{
    INT32U offset;
    //INT8U midu;

    //�����С��λ��д����
    offset = phy->offset;  //get_cur_data_save_offset(meter_idx,phy,FALSE);

    fread_array(meter_idx,offset,buffer,phy->data_len);
    //����ʱ��
    mem_cpy(buffer,datetime+MINUTE,5);
    //��������

    datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
    mem_cpy(buffer+5+phy->block_offset,data,datalen);

    fwrite_array(meter_idx,offset,buffer,phy->data_len);
    return TRUE;
}
INT8U readdata_agp_cur_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U rec_datetime[5],INT8U *data,INT8U *datalen)
{
    INT32U offset;
    //INT8U midu;

    offset = phy->offset;

    fread_array(meter_idx,offset,data,phy->data_len);

    mem_cpy(rec_datetime,data,5);
    if(check_is_all_ch(data+5+phy->block_offset,phy->block_len,0xFF))
    {
        *datalen = 0;
        mem_set(data,phy->block_len,0xFF);
        return FALSE;
    }
    else
    {
        *datalen = phy->block_len;
        mem_cpy(data,data+5+phy->block_offset,phy->block_len);
        return TRUE;
    }
}
INT16S app_readdata_agp_cur_data(INT16U meter_idx,INT32U phy_id,INT8U rec_datetime[5],INT8U *data,INT8U max_datalen)
{
    READ_WRITE_DATA phy;
    INT16U idx,sub_idx,block_begin_idx;
    INT8U datalen;
    INT8U block_count;
    INT8U agp_data[21];

    idx = get_phy_form_list_agp_cur_data(phy_id,&phy,&block_begin_idx,&block_count);
    if (idx == 0xFFFF) return -1;
    if (phy.data_len > max_datalen) return -1;
    //readdata_agp_cur_data(meter_idx,&phy,rec_datetime,data,&datalen);

    //if((idx >= 0) && (idx < 6))
    if(idx < 6)
    {
        get_C1_F19F20F21F22(agp_data,meter_idx,DT_F19,TRUE,0);
        sub_idx = idx;
        if(sub_idx == 0)
        {
            datalen = 20;
            mem_cpy(data,agp_data+1,datalen);
        }
        else 
        {
            datalen = 4;
            mem_cpy(data,agp_data+1+(sub_idx-1)*4,datalen);
        }
    }
    else if((idx >= 6) && (idx < 12))
    {
        get_C1_F19F20F21F22(agp_data,meter_idx,DT_F20,TRUE,0);
        sub_idx = idx-6;
        if(sub_idx == 0)
        {
            datalen = 20;
            mem_cpy(data,agp_data+1,datalen);
        }
        else 
        {
            datalen = 4;
            mem_cpy(data,agp_data+1+(sub_idx-1)*4,datalen);
        }
    }
    else if((idx >= 12) && (idx < 18))
    {
        get_C1_F19F20F21F22(agp_data,meter_idx,DT_F21,TRUE,0);
        sub_idx = idx-12;
        if(sub_idx == 0)
        {
            datalen = 20;
            mem_cpy(data,agp_data+1,datalen);
        }
        else 
        {
            datalen = 4;
            mem_cpy(data,agp_data+1+(sub_idx-1)*4,datalen);
        }
    }
    else if((idx >= 18) && (idx < 24))
    {
        get_C1_F19F20F21F22(agp_data,meter_idx,DT_F22,TRUE,0);
        sub_idx = idx-18;
        if(sub_idx == 0)
        {
            datalen = 20;
            mem_cpy(data,agp_data+1,datalen);
        }
        else 
        {
            datalen = 4;
            mem_cpy(data,agp_data+1+(sub_idx-1)*4,datalen);
        }
    }
    else if(idx ==24)
    {
        get_C1_F17_F18(agp_data,meter_idx,DT_F17);
        datalen = 2;
        mem_cpy(data,agp_data,datalen);
    }
    else if(idx == 25)
    {
        get_C1_F17_F18(agp_data,meter_idx,DT_F18);
        datalen = 2;
        mem_cpy(data,agp_data,datalen);
    }
    #if defined (__POWER_CTRL__)
    else if(idx == 26)
    {
        get_C1_F23(agp_data,meter_idx);
        datalen = 4;
        mem_cpy(data,agp_data,datalen);
    }
    #endif
    else
    {
        return -1;
    }
    
    return datalen;
}
INT8U save_agroup_cur_date(INT16U meter_idx,INT32U phy_seq,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    INT16U idx,block_begin_idx;
    INT8U block_count;
    //BOOLEAN write_seq;

    idx = get_phy_form_list_agp_cur_data(phy_seq,&phy,&block_begin_idx,&block_count);
    if(idx == 0xFFFF) return 0;
    //�ܼ����
    //meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    writedata_agp_cur_data(meter_idx,&phy,frame,frame_len,buffer);

    /*if(((phy.phy & 0x000000FF) == 0x3F) 
        || ((phy.phy & 0x000000FF) == 0x7F) 
        || ((phy.phy & 0x000000FF) == 0xBF) 
        || ((phy.phy & 0x000000FF) == 0xFF)) //�ǿ������� 
    {
        if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            //����msak
            clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,idx);
        }
        else  
        {
            writedata_agp_cur_data(meter_idx,&phy,frame,frame_len,buffer);
            writedata_cur_data_seq(meter_idx,&phy);
            for(idx=0;idx<block_count;idx++)
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,block_begin_idx+idx);
            } 
        }
    }
    else
    {
        if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            //����msak
            clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,idx);
        }
        else
        {
            writedata_cur_data(meter_idx,&phy,frame,frame_len,buffer);
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
        }
        //����Ƿ񳭶���ɣ����Ҫ�������
        write_seq = TRUE;
        for(idx=0;idx<block_count;idx++)
        {
            if(get_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,block_begin_idx+idx)) write_seq = FALSE;
        }
        if(write_seq)
        {
            writedata_cur_data_seq(meter_idx,&phy);
        }
    } */

    return 0;
}
INT8U get_phy_form_list_agp_cruve(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(AGROUP_CURVE_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((AGROUP_CURVE_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(AGROUP_CURVE_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}
//��Ҫ���������ܶȣ������ĵ���
//td��0~5 �� ʱ �� �� �� �ܶ�
INT8U readdata_agp_curve(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[6],INT8U *data,INT8U *datalen)
{
    void compute_agp_amount_interval(INT8U *data,INT8U for_amount[4],INT8U cur_amount[4]);
    INT32U offset;//,former_data;
    INT8U  former_amount[20],cur_amount[4],former_td[5];
    INT8U curve_interval;//midu,
    INT8U td_bin[5],td_bcd[5];
    CommandDate   cmddate;

    //��ȡ�ܶ�
    //fread_array(meter_idx,phy->offset,&midu,1);

    mem_set(cur_amount,sizeof(cur_amount),0x00);
	mem_set(former_amount,sizeof(former_amount),0x00);
	mem_set(former_td,sizeof(former_td),0x00);
	
    offset = get_agp_curve_save_offset(phy,td,15);

    fread_array(FILEID_AGP + meter_idx,offset,data,phy->data_len);
    if (compare_string(td,data,5) == 0)
    {
        if(check_is_all_ch(data+5+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            if((phy->phy == 0x92C0) ||(phy->phy == 0x9300))
            {
                mem_cpy(cur_amount,data+5,sizeof(cur_amount));

                //�����ܶ�֧�� 1��15����  2�� 30����  3: 60����
                if( (td[5] <1) || (td[5] > 3) )
                {
                   td[5] = 1;   //�����ܶ�15����
                }
                if(1==td[5])      curve_interval = 15;
                else if(2==td[5]) curve_interval = 30;
                else              curve_interval = 60;

                td_bcd[0] = byte2BCD(td[0]);
                td_bcd[1] = byte2BCD(td[1]);
                td_bcd[2] = byte2BCD(td[2]);
                td_bcd[3] = byte2BCD(td[3]);
                td_bcd[4] = byte2BCD(td[4]);

                setCommandBCDDate(&cmddate,td_bcd);

                //���ն�ȡ���ȡ���յ���һ�������,��һ��ʱ����� 0��0��ʱ����������Ϊ0.
                commandDateMinusMinute(&cmddate,curve_interval);

                assign_td_value(&cmddate,former_td,5);
                
                if((0==cmddate.hour)  && (0==cmddate.minute))
                {
                    //ÿ�յ�һ���㣬֮ǰû�����ݵ㣬�������Ϊ0
                    mem_set(former_amount,4,0x00);
                }
                else
                {
                    td_bin[0] = BCD2byte(former_td[0]);
                    td_bin[1] = BCD2byte(former_td[1]);
                    td_bin[2] = BCD2byte(former_td[2]);
                    td_bin[3] = BCD2byte(former_td[3]);
                    td_bin[4] = BCD2byte(former_td[4]);
                    offset = get_agp_curve_save_offset(phy,td_bin,15);
                    fread_array(FILEID_AGP + meter_idx,offset,data,phy->data_len);
                    mem_cpy(former_amount,data+5,sizeof(former_amount));
                }
      
                compute_agp_amount_interval(data,former_amount,cur_amount);
        
                *datalen = phy->block_len;    
         
                if(check_is_all_ch(data,*datalen,0xEE)) 
                {
                    *datalen = 0;
                    return FALSE;
                }
         
            }
            else
            {
            *datalen = phy->block_len;
            mem_cpy(data,data+5+phy->block_offset,phy->block_len);
            }
            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

INT16S app_readdata_agp_curve(INT16U meter_idx,INT32U phy_id,INT8U td[6],INT8U *data,INT8U max_datalen)
{
    READ_WRITE_DATA phy;
    INT16U agp_idx;
    INT8U idx,datalen;
    //INT8U midu;
    INT8U td_bin[6];

    datalen = 0;
    idx = get_phy_form_list_agp_cruve(phy_id,&phy);
    if (idx == 0xFF) return -1;
    if (phy.data_len > max_datalen) return -1;

    td_bin[0] = BCD2byte(td[0]);
    td_bin[1] = BCD2byte(td[1]);
    td_bin[2] = BCD2byte(td[2]);
    td_bin[3] = BCD2byte(td[3]);
    td_bin[4] = BCD2byte(td[4]);
    td_bin[5] = td[5];
    //�ܼ����
    agp_idx = trans_set_pn_2_pnidx(meter_idx);
    if((agp_idx > MAX_ADDGROUP_COUNT) || (agp_idx == 0) )  return -1;
    agp_idx--;
    readdata_agp_curve(agp_idx,&phy,td_bin,data,&datalen);
    return datalen;
}
INT8U get_phy_form_list_agp_cycle_day(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(AGROUP_CYCLE_DAY_DATA_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((AGROUP_CYCLE_DAY_DATA_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(AGROUP_CYCLE_DAY_DATA_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}
INT8U readdata_agp_cycle_day(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[3],INT8U rec_datetime[5],INT8U *data,INT8U *datalen)
{
    INT32U offset;
    INT8U begin_pos;
    INT8U bin_td[3]={0};

    offset = get_cycle_day_save_offset(phy,td);
 
    fread_array(FILEID_AGP + meter_idx,offset,data,phy->data_len);
    
    if(phy->phy == 0x0000B100)
    {
         begin_pos = 0;
         bin_td[0] = BCD2byte(data[0]);
         bin_td[1] = BCD2byte(data[1]);
         bin_td[2] = BCD2byte(data[2]);
    }
    else
    {
        begin_pos = 1;
        mem_cpy(bin_td,data+1,3);
    }
    
    if ((td[2] == bin_td[2]) && (td[1] == bin_td[1]) && (td[0] == bin_td[0]))
    {
       // mem_cpy(rec_datetime,data+3,5);
        if(check_is_all_ch(data+3+begin_pos+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+3+begin_pos+phy->block_offset,phy->block_len);
            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

INT16S app_readdata_agp_cycle_day(INT16U meter_idx,INT32U phy_id,INT8U td[3],INT8U rec_datetime[5],INT8U *data,INT8U max_datalen)
{
    READ_WRITE_DATA phy;
    INT16U agp_idx;
    INT8U idx,datalen;
    INT8U td_bin[3];

    datalen = 0;
    idx = get_phy_form_list_agp_cycle_day(phy_id,&phy);
    if(idx == 0xFF) return -1;
    if (phy.data_len > max_datalen) return -1;

    td_bin[0] = BCD2byte(td[0]);
    td_bin[1] = BCD2byte(td[1]);
    td_bin[2] = BCD2byte(td[2]);

    //�ܼ����
    agp_idx = trans_set_pn_2_pnidx(meter_idx);
    if((agp_idx > MAX_ADDGROUP_COUNT) || (agp_idx == 0) )  return -1;
    
    readdata_agp_cycle_day(agp_idx-1,&phy,td_bin,rec_datetime,data,&datalen);

    return datalen;
}
INT8U get_phy_form_list_agp_cycle_month(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(AGROUP_CYCLE_MONTH_DATA_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((AGROUP_CYCLE_MONTH_DATA_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(AGROUP_CYCLE_MONTH_DATA_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}
INT8U readdata_agp_cycle_month(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[2],INT8U rec_datetime[5],INT8U *data,INT8U *datalen)
{
    INT32U offset;
    INT8U begin_pos;
    INT8U bin_td[2]={0};

    offset = get_cycle_month_save_offset(phy,td);

    fread_array(FILEID_AGP + meter_idx,offset,data,phy->data_len);
    
    if(phy->phy == 0x0000B100)
    {
         begin_pos = 0;
         bin_td[0] = BCD2byte(data[0]);
         bin_td[1] = BCD2byte(data[1]);
    }
    else
    {
        begin_pos = 1;
        mem_cpy(bin_td,data+1,2);
    }
    
    if ((td[1] == bin_td[1]) && (td[0] == bin_td[0]))
    {
        //mem_cpy(rec_datetime,data+2,5);
        if(check_is_all_ch(data+2+begin_pos+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+2+begin_pos+phy->block_offset,phy->block_len);
            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

INT16S app_readdata_agp_cycle_month(INT16U meter_idx,INT32U phy_id,INT8U td[2],INT8U rec_datetime[5],INT8U *data,INT8U max_datalen)
{
    READ_WRITE_DATA phy;
    INT16U agp_idx;
    INT8U idx,datalen;
    INT8U td_bin[2];

    datalen = 0;
    idx = get_phy_form_list_agp_cycle_month(phy_id,&phy);
    if(idx == 0xFF) return -1;
    if (phy.data_len > max_datalen) return -1;
    td_bin[0] = BCD2byte(td[0]);
    td_bin[1] = BCD2byte(td[1]);

    //�ܼ����
    agp_idx = trans_set_pn_2_pnidx(meter_idx);
    if((agp_idx > MAX_ADDGROUP_COUNT) || (agp_idx == 0) )  return -1;
    
    readdata_agp_cycle_month(agp_idx-1,&phy,td_bin,rec_datetime,data,&datalen);
    return datalen;
}
//---------------------------------------------------------------------------
BOOLEAN get_phy_data(READ_WRITE_DATA* in,INT8U idx,READ_WRITE_DATA* out)
{
    if(idx > (in->flag & 0x1F)) return FALSE;
    
    if ((in->flag & 0x1F) == 0)
    {
        mem_cpy((INT8U*)&(out->phy),(INT8U*)&(in->phy),sizeof(READ_WRITE_DATA));
    }
    else if ((in->flag & SAVE_FLAG_BLOCK) && (idx == 0))
    {
        out->phy = in->phy+64-1;
        out->offset = in->offset;
        out->data_len = in->data_len;
        out->flag = in->flag & 0xE0;
        out->block_offset = in->block_offset;
        out->block_len = in->block_len*(in->flag & 0x1F);
    }
    else
    {
        if (in->flag & SAVE_FLAG_BLOCK) idx--;
        out->phy = in->phy+idx;
        out->offset = in->offset;
        out->data_len = in->data_len;
        out->flag = in->flag & (SAVE_FLAG_DENY_NO_SAVE | SAVE_FLAG_XL_TIME);
        out->block_offset = in->block_offset + idx*in->block_len;
        out->block_len = in->block_len;
    }
    return TRUE;
}
//---------------------------------------------------------------------------
INT8U get_phy_form_list_cycle_day(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(CYCLE_DAY_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        if(CYCLE_DAY_PHY_LIST[idx].phy == 0xFFFFFFFF)continue;
        for(idx_sub=0;idx_sub<((CYCLE_DAY_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(CYCLE_DAY_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}

INT32U get_cycle_day_save_offset(READ_WRITE_DATA *phy,INT8U td[3])
{
    INT32U offset;

    offset = getPassedDays(2000+td[2],td[1],td[0]);
    offset = offset % SAVE_POINT_NUMBER_DAY_HOLD;
    offset *= phy->data_len;
    offset += phy->offset;
    return offset;
}

INT8U format_data_save(INT8U format_id,INT8U* data,INT8U data_len,INT8U* format_data,READ_WRITE_DATA *phy)
{
    XL_DATA_FORMAT* xldata;
    XL_TIME *xltime;
    INT8U idx,fl_count;
    //INT8U tmp[24];

    xldata = (XL_DATA_FORMAT*)format_data;
    xltime = (XL_TIME*)data;

    switch(format_id)
    {
    case 0: //��������
            if((phy->phy & 0x0000003F) == 0x3F)/*��������*/
            {
        fl_count = data_len/sizeof(XL_TIME);
        //data_len = fl_count*sizeof(XL_TIME);û�����壬����
        fl_count = (fl_count > DEFAULT_SAVE_FL_DATA_COUNT) ? DEFAULT_SAVE_FL_DATA_COUNT : fl_count;

        mem_set(xldata->value,sizeof(XL_DATA_FORMAT),0xFF);
        for(idx=0;idx<fl_count;idx++)
        {
            mem_cpy(xldata->xl[idx],xltime->xl,3);
            mem_cpy(xldata->time[idx],xltime->time,4);
            xltime++;
        }
        return sizeof(XL_DATA_FORMAT);
    }
            else/*��������*/
            {
                    idx =  (phy->phy & 0x0000003F) ;

                    mem_cpy(xldata->xl[idx],xltime->xl,3);
                    mem_cpy(xldata->time[idx],xltime->time,4);

                    return sizeof(XL_DATA_FORMAT);
            }
            break;
        default:
            break;
    }
       
    return 0;
}

INT8U writedata_cycle_day(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[3],INT8U *data,INT8U datalen,INT8U* buffer,INT8U *reserve_data,INT8U remain_num)
{
    INT32U offset;
	#if (defined __PROVICE_JIANGSU__)
    METER_DOCUMENT meter_doc;
	#endif
	#if ((defined __SICHUAN_METER_TIME__) || (defined __OTHER_METER_TIME__))
    INT8U idx;
	#endif
    #if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
    //METER_DOCUMENT meter_doc;
    C2_F27F35 c2_f27;
    INT32U  A_over_high_time = 0x0000FFFF; //Խ����
    INT32U  A_over_low_time  = 0x0000FFFF; //���� 
    BOOLEAN valid_bcd = FALSE;
    #endif
    INT8U idx1,real_datalen = 0;
    READ_WRITE_DATA phy_new;
    #ifdef __COUNTRY_ISRAEL__
    INT8U td_tmp[5] = {0};
    #endif
    extern INT32U get_curve_save_offset(READ_WRITE_DATA *phy,INT8U td[5],INT8U midu);
    offset = get_cycle_day_save_offset(phy,td);

    fread_array(meter_idx,offset,buffer,phy->data_len);

    //����ʱ��
    if(compare_string(buffer,td,3) != 0)
    {
        mem_set(buffer,phy->data_len,0xFF);
        mem_cpy(buffer,td,3);
        #ifdef __PROVICE_TIANJIN__
        buffer[3]=0x00;
        buffer[4]=0x00;
        mem_cpy(buffer+5,datetime+DAY,3);
        #elif defined __PROVICE_JIANGSU__
        fread_meter_params(meter_idx,PIM_METER_DOC,meter_doc.value,sizeof(METER_DOCUMENT));
        #ifdef __READ_OOP_METER__
        if((meter_doc.protocol == GB645_2007)||(meter_doc.protocol == GB_OOP))
        #else
        if(meter_doc.protocol == GB645_2007)
        #endif
        {
        buffer[3]=59;
        buffer[4]=23;
        get_yesterday(buffer+5);
        }
        else
        mem_cpy(buffer+3,datetime+MINUTE,5);
/*
            if((pLib->item == 0x901F)||(pLib->item == 0x902F)||(pLib->item == 0x911F)||(pLib->item == 0x912F)
        ||(pLib->item == 0x913F)||(pLib->item == 0x914F)||(pLib->item == 0x915F)||(pLib->item == 0x916F)||(pLib->item == 0xA01F)||(pLib->item == 0xB01F)||(pLib->item == 0xA11F)||(pLib->item == 0xB11F)
        ||(pLib->item == 0xA02F)||(pLib->item == 0xB02F)||(pLib->item == 0xA12F)||(pLib->item == 0xB12F)||(pLib->item == 0xA13F)||(pLib->item == 0xA14F)||(pLib->item == 0xA15F)||(pLib->item == 0xA16F)
        ||(pLib->item == 0xB13F)||(pLib->item == 0xB14F)||(pLib->item == 0xB15F)||(pLib->item == 0xB16F))
      {
        if(GB645_1997_JINANGSU_4FL == meter_doc.protocol)
        {
            convert_phy_data_zfpgj2zjfpg_c2(data,4);
        }
        if(GB645_1997_JINANGSU_2FL == meter_doc.protocol)
        {
        //    if(phy_seq &0x3F == 0x3F)    //���12��Լ��4���ʿ�����  ������
        //        return len;
            convert_phy_data_zfg2zjfpg_c2(data,4);
        }
      }
*/
        #elif defined __PROVICE_JIANGXI__
        if((phy->phy == S1C_RDJ_ZXYGDN_SJK) || (phy->phy == S1C_RDJ_FXYGDN_SJK))
        {
            buffer[3]=0x00;
            buffer[4]=0x00;
            mem_cpy(buffer+5,datetime+DAY,3);
        }
        else
        {
            mem_cpy(buffer+3,datetime+MINUTE,5);
        }
        #else
        mem_cpy(buffer+3,datetime+MINUTE,5);
        #endif
    }

    if (phy->flag & SAVE_FLAG_XL_TIME)
    {
        (void)format_data_save(0,data,datalen,buffer+8,phy);
    }
    else
    {
       /*��ȡ�������ص���ʵ���ȣ���Ϊ����Գ��������¸�ֵ*/        
        real_datalen = datalen;
        
        datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
        #ifdef __PROVICE_ABROAD_DEMO__
        if(phy->phy == 0x00000480)
        {
          datalen = 12;
        mem_cpy(buffer+8+phy->block_offset,data+5,datalen);
        }
        else
        #endif
        #ifdef __COUNTRY_ISRAEL__
        if((phy->phy == ISRAEL_HOUR_DATA) ) //��ɫ��Сʱ���ݣ��������ֽڵ�ʱ�꣬Ҫ�����
        {
          if(datalen == 37) //����ǵ������ѹ�͵�������ת�������򳤶Ȳ�һ��
          {
           single_meter_tran_three_meter(data+2);
           datalen = 45;
          }
          mem_cpy(buffer+8+phy->block_offset,data+2,datalen);
        }
        else
        #endif
        {
           #ifdef __PROVICE_JIANGSU__
            if((meter_doc.protocol == GB645_1997)&&(datalen == 12))
            {
                mem_set(buffer+8+phy->block_offset, 25, 0x00);
                mem_cpy(buffer+8+phy->block_offset, data, 4);//��
                mem_cpy(buffer+8+phy->block_offset+8, data+4, 4);//��
                mem_cpy(buffer+8+phy->block_offset+16, data+8, 4);//��
            }
            else
            {
                mem_cpy(buffer+8+phy->block_offset,data,datalen);
            }
           #else
           mem_cpy(buffer+8+phy->block_offset,data,datalen);
           
           #ifdef __HIGH_PRECISION_DATA__
           //�ڴ˴������ն������ݵĸ߾�������
           if((phy->phy >= (ZXYG_DN_SJK-0x3F)) && (phy->phy <= D4XX_WG_DN_SJK))
           {
              if((datalen == 4) && (real_datalen ==5))//�������������ĳ�����4��ʵ�ʷ�����5�����ж�Ϊ�߾�������
              {
                 mem_cpy(buffer+8+phy->block_offset,data+1,datalen);
              }
           }
           #endif
           #endif
        }
    }
    if(reserve_data)
    {
        mem_cpy(buffer+phy->data_len-RESERVE_DATA,reserve_data,RESERVE_DATA);
    }


    #ifdef  __PLC_REC_VOLTMETER1__
    if(phy->phy == DY_HOUR_HOLD) //��������24�Σ����Բ��ܱ��棬�������ж��������Ƿ������ݵ�ʱ�����Ϊ�Ѿ����������
    {
        //
    }
    else
    #endif
    {
        fwrite_array(meter_idx,offset,buffer,phy->data_len);

        if(phy->phy == S1C_RDJ_SYJE)
        {
          if(real_datalen == 10) /*˵���ǰ���13-645��Լ�ظ���*/
          {
              idx1 = get_phy_form_list_cycle_day(0x00001240,&phy_new);/*��ȡʣ�������ز���*/
              if(idx1 == 0xFF)
              {
                  return 0;
              }

              offset = get_cycle_day_save_offset(&phy_new,td);
              mem_cpy(buffer+8,data+2,4);
              fwrite_array(meter_idx,offset,buffer,phy_new.data_len);
          }
        }
    }

    #ifdef  __PLC_REC_VOLTMETER1__
    if((phy->phy == DY_A_HGL) || (phy->phy == DY_B_HGL) || (phy->phy == DY_C_HGL))
    {
        plc_router_save_voltmeter_data(meter_idx,phy->phy,buffer+8,phy->data_len,0);
    }
    if(phy->phy == DY_HOUR_HOLD)//��ѹ���������ݴ洢
    {
        plc_router_save_vlotmeter_curve_data(meter_idx,phy,buffer+8,phy->data_len,remain_num);
    }
    #endif
    #if ((defined __SICHUAN_METER_TIME__) || (defined __OTHER_METER_TIME__))
    if(phy->phy == SJ)
    {
        mem_cpy(buffer+3,datetime+MINUTE,5);
        for(idx =0;idx<6;idx++)
        {
            buffer[8+idx] = byte2BCD(datetime[idx+SECOND]);
        }
        phy->data_len = 19;
        phy->offset = PIM_DAY_HOLD_JZQ_READ_TIME;
        offset = get_cycle_day_save_offset(phy,td);
        fwrite_array(meter_idx,offset,buffer,14);
    }
    #endif
    #ifdef __COUNTRY_ISRAEL__
    if((phy->phy == ISRAEL_HOUR_DATA) ) //��ɫ��Сʱ���ݣ��������ֽڵ�ʱ�꣬Ҫ�����
    {
        phy->phy = 0x00002C7F;
        phy->data_len = 29;
        phy->block_offset = 0;
        phy->offset = PIM_DAY_HOLD_ZXYG;

        /*ת��161*/
        get_phy_form_list_cycle_day(0x00002C7F,phy);

        offset = get_cycle_day_save_offset(phy,td);

        fread_array(meter_idx,offset,buffer,phy->data_len);

        //����ʱ��
        if(compare_string(buffer,td,3) != 0)
        {
            mem_set(buffer,phy->data_len,0xFF);
            mem_cpy(buffer,td,3);
            mem_cpy(buffer+3,datetime+MINUTE,5);

            mem_cpy(buffer+8+phy->block_offset,data+2,20);

            fwrite_array(meter_idx,offset,buffer,phy->data_len);
        }

        /*ת��0���F96����*/

        mem_cpy(td_tmp+2,datetime+DAY,3);

        get_phy_form_list_cruve(ISRAEL_HOUR_DATA,phy);
        offset = get_curve_save_offset(phy,td_tmp,60);

        fread_array(meter_idx,offset,buffer,phy->data_len);
               //����ʱ��
        if(compare_string(buffer,td_tmp,5) != 0)
        {
           mem_set(buffer,phy->data_len,0xFF);
           mem_cpy(buffer,td_tmp,5);

           mem_cpy(buffer+5+phy->block_offset,data+2,datalen);

           fwrite_array(meter_idx,offset,buffer,phy->data_len);
        }

    }
    #endif
    #if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
    if(phy->phy == S1R_DYHGL_TJ_SJ_SJK-0x3F+1)
    {
        //
        mem_set(meter_doc.value,sizeof(METER_DOCUMENT),0x00);
        fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
        if(meter_doc.meter_class.user_class == 14)
        {
            //��֮ǰ��ⱨ�� ��䲿��00��EE�������Ȱ���EE�������ݼ�����޸�TODO ?????            
            mem_set(c2_f27.value,sizeof(C2_F27F35),0xEE);

            if(datalen >= 12)
            {
                A_over_high_time = bcd2u32(data+9,3,&valid_bcd);
                if(FALSE == valid_bcd)
                {
                    A_over_high_time = 0x0000FFFF;
                }
            }
            if(datalen >= 15)
            {
                A_over_low_time  = bcd2u32(data+12,3,&valid_bcd);
                if(FALSE == valid_bcd)
                {
                    A_over_low_time = 0x0000FFFF;
                }
            }
            //AԽ�����ۼ�ʱ��
            
            c2_f27.va_over_time[4] = (INT8U )(A_over_high_time);
            c2_f27.va_over_time[5] = (INT8U )(A_over_high_time>>8);
            // A Խ�����ۼ�ʱ��
            c2_f27.va_over_time[6] = (INT8U )(A_over_low_time);
            c2_f27.va_over_time[7] = (INT8U )(A_over_low_time>>8);
            if(datalen >= 21)
            {
                //��ߵ�ѹ ����ʱ��
                mem_cpy(c2_f27.va_max,data+15,2);
                mem_cpy(c2_f27.va_max_time,data+17,3);
            }
            if(datalen >= 27)
            {
                //��͵�ѹ ����ʱ��
                mem_cpy(c2_f27.va_min,data+21,2);
                mem_cpy(c2_f27.va_min_time,data+23,3);
            }
            offset = getPassedDays(2000+td[2],td[1],td[0]);
            offset = (offset % SAVE_POINT_NUMBER_DAY_HOLD);
            offset *= sizeof(C2_F27F35);
            offset += PIM_C2_F27;
            fwrite_array(meter_idx,offset,c2_f27.value,sizeof(C2_F27F35));
        }
    }
	#endif
    #ifdef __HIGH_PRECISION_DATA__
	writedata_precision_cycle_day_and_curve(meter_idx,phy,td,data,real_datalen,buffer,reserve_data,0x55);
    #endif
    return TRUE;
}

INT8U readdata_cycle_day(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[3],INT8U rec_datetime[5],INT8U *data,INT8U *datalen,INT8U *reserve_data)
{
    INT32U offset;

    offset = get_cycle_day_save_offset(phy,td);

    fread_array(meter_idx,offset,data,phy->data_len);
    if ((td[2] == data[2]) && (td[1] == data[1]) && (td[0] == data[0]))
    {
        mem_cpy(rec_datetime,data+3,5);
        if(check_is_all_ch(data+8+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+8+phy->block_offset,phy->block_len);
            if(reserve_data)
            {
                mem_cpy(reserve_data,data+phy->data_len-RESERVE_DATA,RESERVE_DATA);
            }

            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

INT16S app_readdata_cycle_day(INT16U meter_idx,INT32U phy_id,INT8U td[3],INT8U rec_datetime[5],INT8U *data,INT8U max_datalen)
{
    READ_WRITE_DATA phy;
    INT8U idx,datalen;
    INT8U td_bin[3];

	datalen = 0;
    idx = get_phy_form_list_cycle_day(phy_id,&phy);
    if(idx == 0xFF) return -1;
    if (phy.data_len > max_datalen) return -1;

    td_bin[0] = BCD2byte(td[0]);
    td_bin[1] = BCD2byte(td[1]);
    td_bin[2] = BCD2byte(td[2]);
    if( FALSE == readdata_cycle_day(meter_idx,&phy,td_bin,rec_datetime,data,&datalen,NULL) )
    {
        //coverity��麯������ֵ���⣬�����±�֤���澯
        datalen = 0;
    }

    return datalen;
}
#if (defined __CHONGQING_DAY_HOLD_CTRL__) || (defined __PROVICE_SHANGHAI__)
/*�����ն��᳭����ʱ�겻�ԵĻ����Ͱ��ն�������붼���?*/
void clr_day_hold_mask_flag(READ_PARAMS *read_params)
{
    //��һ��һ����������Ƕ����??
    //����淶˵�� ���07����ն���������Ҫ�жϵ��ܱ���һ�ն���ʱ�꣬ʱ�겻�Բ������ɶ������ݡ�
    //��ʱ�������
    #ifdef __PROVICE_SHANGHAI__
    if(read_params->meter_doc.baud_port.port != COMMPORT_PLC)
    #endif
    {
    mem_set(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,0x00);
    #ifndef __ALL_MONTH_DATA_FROM_JSR_DATA__ //�ڳ��ն���ת�¶��������£��ն���ʱ�겻���¶���Ҳ������
    mem_set(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,0x00);
    #endif
    }
}
#endif
INT8U save_cycle_day(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    INT32U begintd,endtd,metertd,curtd;
    READ_WRITE_DATA phy;
    INT8U idx;
    INT8U begin_td[3],end_td[3];
    INT8U day_month_hold_td[13];//5�ֽڵ��ն�ʱ�� +3 �ֽڵ� �ն���ʱ�� + 2�ֽڵ��¶���ʱ�� +3�ֽڵ��ʱ��
    #ifdef __PROVICE_SHANGHAI__
    INT16U passed_days_sys;
    INT16U passed_days_meter;
    #endif

    mem_set(begin_td,sizeof(begin_td),0x00);
    mem_set(end_td,sizeof(end_td),0x00);
	mem_set(day_month_hold_td,sizeof(day_month_hold_td),0x00);
    if (bin2_int32u(read_params->phy) == 0x00002C00)    //�ն���ʱ��
    {
        if(check_is_all_ch(frame,frame_len,0xEE)) //����
        {
            #if (defined __CHONGQING_DAY_HOLD_CTRL__) || (defined __PROVICE_SHANGHAI__)
            clr_day_hold_mask_flag(read_params);//��֧�ֶ���ʱ�꣬�����
			#endif
            #ifdef __CHECK_MONTH_HOLD_TD__
            fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
            #endif
            get_yesterday(read_params->day_hold_td);
            #ifndef __CHECK_MONTH_HOLD_TD__
            get_former_month(read_params->month_hold_td);
            #endif
            mem_cpy(day_month_hold_td,datetime+MINUTE,5);
            mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
            #ifndef __CHECK_MONTH_HOLD_TD__
            mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
            #endif
            mem_set(day_month_hold_td+10,3,0xEE);    //��֧�ֶ���ʱ��
            fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));

            //ʱ����ϲ�֧�֣�������������������ǰ
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXYG_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXYG_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXWG_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXWG_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG1_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG2_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG3_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG4_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZYG_ZDXL_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FYG_ZDXL_DH);
            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_GDCS_BLOCK);

        }
        else
        {
            read_params->day_hold_td[0] = BCD2byte(frame[2]);    //��
            read_params->day_hold_td[1] = BCD2byte(frame[3]);    //��
            read_params->day_hold_td[2] = BCD2byte(frame[4]);    //��
            #ifdef __CHECK_MONTH_HOLD_TD__
            fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
            #endif
            #ifndef __CHECK_MONTH_HOLD_TD__
            mem_cpy(read_params->month_hold_td,read_params->day_hold_td+1,2);
            #endif
            mem_cpy(day_month_hold_td+10,read_params->day_hold_td,3);    //��һ�µ����ն���ʱ��
//            rs232_debug_info("\xB0\xB0",2);
//            rs232_debug_info(read_params->day_hold_td,3);
            if(compare_string(read_params->day_hold_td,datetime+DAY,3) != 0)    //���ڲ���ȷ
            {
                #if (defined __CHONGQING_DAY_HOLD_CTRL__) || (defined __PROVICE_SHANGHAI__)
                //����ʱ�겻�ԣ��洢�����������������ݳ���ʱ��� 10 11 12���ж��Ƿ���Ҫ����
                mem_cpy(day_month_hold_td,datetime+MINUTE,5);//�洢�³���ʱ��
                mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                #ifndef __CHECK_MONTH_HOLD_TD__
                mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                #endif
				fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                clr_day_hold_mask_flag(read_params);
                #ifdef __PROVICE_SHANGHAI__ //�����ʱ�������ն�ʱ�ӵ����,���ٳ���Ӧ���ն�������
                passed_days_sys = getPassedDays(datetime[YEAR], datetime[MONTH], datetime[DAY]);
                passed_days_meter = getPassedDays(read_params->day_hold_td[2], read_params->day_hold_td[1], read_params->day_hold_td[0]);
                if(passed_days_meter > passed_days_sys)
                {
                    clr_bit_value(read_meter_flag_cycle_day.flag, READ_FLAG_BYTE_NUM, bin2_int16u(read_params->meter_doc.meter_idx));
                }
                #endif
				return 0;
				#endif

                #if defined __PROVICE_HUNAN__ || defined __PROVICE_JIANGXI__
                if((read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS))
                {
                    #ifdef __PROVICE_JIANGXI__
                    if(!((datetime[HOUR] == 0x00) && (datetime[MINUTE] <= 30))) /**< ����Ҫ��00:30֮������ʵʱת�棬����ʱ��ʱ����󲻳�����.*/
                    {
                        clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                        return 0;
                    }
                    else
                    #endif
                    {
                        get_yesterday(read_params->day_hold_td);
                        #ifndef __CHECK_MONTH_HOLD_TD__
                        get_former_month(read_params->month_hold_td);
                        #endif
                        mem_cpy(day_month_hold_td,datetime+MINUTE,5);
                        mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                        #ifndef __CHECK_MONTH_HOLD_TD__
                        mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                        #endif
                        fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                        //������������������ǰ
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXYG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXYG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXWG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXWG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG1_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG2_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG3_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG4_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZYG_ZDXL_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FYG_ZDXL_DH);
                    }
                }
                else
                #endif
                {
                if (read_params->control.according_to_day_hold_td_save == 1)   //1���������ᰴ�յ��ն���洢��
                {
//                    rs232_debug_info("\xB1\xB1",2);
                    get_yesterday(read_params->day_hold_td);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    get_former_month(read_params->month_hold_td);
                    #endif
                    mem_cpy(day_month_hold_td,datetime+MINUTE,5);
                    mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                    #endif
                    fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                }
                else if (read_params->control.according_to_day_hold_td_save == 2)  //2��������ǰ���ݰ��յ��ն���洢
                {
//                    rs232_debug_info("\xB2\xB2",2);
                    get_yesterday(read_params->day_hold_td);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    get_former_month(read_params->month_hold_td);
                    #endif
                    mem_cpy(day_month_hold_td,datetime+MINUTE,5);
                    mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                    #endif
                    fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                    //������������������ǰ
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXYG_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXYG_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXWG_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXWG_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG1_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG2_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG3_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG4_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZYG_ZDXL_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FYG_ZDXL_DH);
                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_GDCS_BLOCK);
                }
                else  //0������������������ն���ʱ��洢��
                {
//                    rs232_debug_info("\xB3\xB3",2);
                    if(check_const_ertu_switch(CONST_ERTU_SWITCH_DAY_HOLD_TD_ERR_NO_SAVE))
                    {
                        //��������־���Ӻ󳭶�
                        clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx)); 
                    }
                    else
                    {
                    if ((read_params->day_hold_td[1] >= 1) && (read_params->day_hold_td[1] <= 12)
                    && (read_params->day_hold_td[0] >= 1) && (read_params->day_hold_td[0] <= 31))
                    {
                        mem_cpy(begin_td,datetime+DAY,3);
                        mem_cpy(end_td,datetime+DAY,3);
                        date_add_days(begin_td+2,begin_td+1,begin_td+0,25);
                        date_add_days(begin_td+2,begin_td+1,begin_td+0,25);
                        date_add_days(begin_td+2,begin_td+1,begin_td+0,25);
                        date_add_days(begin_td+2,begin_td+1,begin_td+0,15);
                        begintd = begin_td[2]*10000+begin_td[1]*100+begin_td[0];

                        date_minus_days(end_td+2,end_td+1,end_td+0,25);
                        date_minus_days(end_td+2,end_td+1,end_td+0,25);
                        date_minus_days(end_td+2,end_td+1,end_td+0,25);
                        date_minus_days(end_td+2,end_td+1,end_td+0,15);
                        endtd = end_td[2]*10000+end_td[1]*100+end_td[0];

                        metertd = read_params->day_hold_td[2]*10000+read_params->day_hold_td[1]*100+read_params->day_hold_td[0];
                        curtd = datetime[YEAR]*10000+datetime[MONTH]*100+datetime[DAY];

                        //�ն���ʱ��ǰ�󳬹�90�죬�������ն�������
                        if ((metertd > begintd) || (metertd < endtd))
                        {
//                            rs232_debug_info("\xB4\xB4",2);
                            clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                            return 0;
                        }

                        //��Ҫ�ȴ�,���ڵȴ��ն���ʱ�������м��ʱ���Ƿ���
                        clr_bit_value(read_meter_flag_wati_day_hold,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                        if ((metertd < curtd) && ((curtd - metertd) == 1) && (read_params->control.wait_day_hold_td))
                        {
//                            rs232_debug_info("\xB5\xB5",2);
                            clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                            set_bit_value(read_meter_flag_wati_day_hold,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                            return 0;
                        }
//                        rs232_debug_info("\xB6\xB6",2);
                        date_minus_days(read_params->day_hold_td+2,read_params->day_hold_td+1,read_params->day_hold_td,1);
                        #ifndef __CHECK_MONTH_HOLD_TD__
                        get_former_month_from_param(read_params->month_hold_td);
                        #endif
                        mem_cpy(day_month_hold_td,datetime+MINUTE,5);
                        mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                        #ifndef __CHECK_MONTH_HOLD_TD__
                        mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                        #endif
                        fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                    }
                    else //������Ч���Ȱ��յ�ǰ���ڴ���
                    {
//                        rs232_debug_info("\xB7\xB7",2);
                        get_yesterday(read_params->day_hold_td);
                        #ifndef __CHECK_MONTH_HOLD_TD__
                        get_former_month(read_params->month_hold_td);
                        #endif
                        mem_cpy(day_month_hold_td,datetime+MINUTE,5);
                        mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                        #ifndef __CHECK_MONTH_HOLD_TD__
                        mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                        #endif
                        fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                    }
                }
              }
            }
            }
            else
            {
                if ((read_params->day_hold_td[1] >= 1) && (read_params->day_hold_td[1] <= 12)
                && (read_params->day_hold_td[0] >= 1) && (read_params->day_hold_td[0] <= 31))
                {
                    #ifdef __CHECK_MONTH_HOLD_TD__
                    fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                    #endif
//                    rs232_debug_info("\xB8\xB8",2);
                    date_minus_days(read_params->day_hold_td+2,read_params->day_hold_td+1,read_params->day_hold_td,1);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    get_former_month_from_param(read_params->month_hold_td);
                    #endif
                    mem_cpy(day_month_hold_td,datetime+MINUTE,5);
                    mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                    #endif
                    fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                }
                else //������Ч���Ȱ��յ�ǰ���ڴ���
                {
                    #if (defined __CHONGQING_DAY_HOLD_CTRL__) || (defined __PROVICE_SHANGHAI__)
                    //������Ч���洢�����������������ݳ���ʱ��� 10 11 12���ж��Ƿ���Ҫ����
                    //����� day_hold_td ��month_hold_td ��������Чֵ���Ȳ����� TODO ???? 
                    #ifdef __CHECK_MONTH_HOLD_TD__
                    fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                    #endif
                    mem_cpy(day_month_hold_td,datetime+MINUTE,5);//�洢�³���ʱ��
                    mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                    #endif
    				fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                    clr_day_hold_mask_flag(read_params);
    				return 0;
    				#endif

                    get_yesterday(read_params->day_hold_td);
                    #ifdef __CHECK_MONTH_HOLD_TD__
                    fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                    #endif
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    get_former_month(read_params->month_hold_td);
                    #endif
                    mem_cpy(day_month_hold_td,datetime+MINUTE,5);
                    mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                    #endif
                    fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                    #ifdef __PROVICE_HUNAN__
                    if((read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS))
                    {
                        //������������������ǰ
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXYG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXYG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXWG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXWG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG1_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG2_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG3_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG4_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZYG_ZDXL_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FYG_ZDXL_DH);
                    }
                    #endif
                }
            }
        }

        return 0;
    }

    idx = get_phy_form_list_cycle_day(bin2_int32u(read_params->phy),&phy);
    if(idx == 0xFF) return 0;
    if (phy.flag & SAVE_FLAG_DENY_NO_SAVE)
    {
        if(check_is_all_ch(frame,frame_len,0xEE)) //����
        {

            #ifdef __CHECK_ZFXYG_DENY_NO_RECORD__ /*�������й����ϲ�Ҫ��¼*/
            if((phy.phy == 0x00000040) || (phy.phy == 0x00000080) )
            {
                read_params->is_read_priority_ctrl = 1;  /*������ǰ�ܻ��Ƿ��ϣ���Ҫ���ַ�����*/
            }
            else
            {
                //����msak
                clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_CYCLE_DAY,idx);
            }
            #else
            //����msak
            clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_CYCLE_DAY,idx);
            #endif

        }
        else
        {
            #ifdef __CHECK_DAYHOLD_DATA__
            if (((bin2_int32u(read_params->phy))==0x00002C7F) && (read_params->control.is_check_data))
            {
                read_params->control.is_check_data = 0;
                if (check_dayhold_data_right(frame) == FALSE)
                {
                    //�����棬���³�
                    return 0;
                }
            }
            #endif
            
            #ifdef __PROVICE_TIANJIN__
            if((read_params->meter_doc.baud_port.port != COMMPORT_485_CY) ||((datetime[HOUR] == 0) && (datetime[MINUTE] <= 5)))
            #endif
            #ifdef __PLC_REC_VOLTMETER1__
            if (bin2_int32u(read_params->phy) == DY_HOUR_HOLD)
            writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,NULL,read_params->patch_num);
            else
            #endif
            writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,NULL,0xFF);
            #ifdef __PRECISE_TIME__
            plc_router_save_meter_precise_time(bin2_int16u(read_params->meter_doc.meter_idx),phy.phy ,frame,frame_len);
            #endif
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** save day hold data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d-%02d",
                    bin2_int16u(read_params->meter_doc.meter_idx),
                    bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
                    read_params->day_hold_td[2],read_params->day_hold_td[1],read_params->day_hold_td[0]);
            debug_println_ext(info);
            #endif
        }
    }
    else
    {
        #ifdef __PROVICE_TIANJIN__
        if((read_params->meter_doc.baud_port.port != COMMPORT_485_CY) ||((datetime[HOUR] == 0) && (datetime[MINUTE] <= 5)))
        #endif
            #ifdef __PLC_REC_VOLTMETER1__
            if (bin2_int32u(read_params->phy) == DY_HOUR_HOLD)
            writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,NULL,read_params->patch_num);
            else
            #endif
        writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,NULL,0xFF);
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** save day hold data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d-%02d",
                bin2_int16u(read_params->meter_doc.meter_idx),
                bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
                read_params->day_hold_td[2],read_params->day_hold_td[1],read_params->day_hold_td[0]);
        debug_println_ext(info);
        #endif
    }
    #ifdef __PLC_REC_VOLTMETER1__
    if (bin2_int32u(read_params->phy) == DY_HOUR_HOLD) //��ѹ�����ݳ�����һ�յ�ѹ��¼������ to do
    {
         if(read_params->patch_num >= 23) //���ݳ�������23�㶳��
         {
         clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
         read_params->patch_num = 0 ;
         }
         else
         read_params->patch_num ++ ;
    }
    else
    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
    #elif __PLC_REC_GUANGXI_PROTOL__
    if (bin2_int32u(read_params->phy) == DY_HOUR_HOLD) //��ѹ�����ݳ�����һ�յ�ѹ��¼������ to do
    {
         if(read_params->patch_num >= 6) //һ֡4���������
         {
         clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
         read_params->patch_num = 0 ;
         }
         else
         read_params->patch_num ++ ;
    }
    else
    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
    #else
    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
    #endif
    #ifdef __HUBEI_STEP_CONTROL__
    calculate_line_lost(&(read_params->meter_doc),&phy,frame,frame_len,READ_TYPE_CYCLE_DAY);
    #endif
    return 0;
}

//����frame��frame_lenΪNULLʱ��ֻ�Ǽ���Ƿ��г������ݣ������ɱ���
BOOLEAN prepare_read_item_cycle_day(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT16U meter_idx,mask_idx;
	#ifdef __PROVICE_YUNNAN__
	INT16U meter_num;
	#endif
	#ifdef __POWER_CTRL__
	INT16U spot_idx;
	#endif
    INT8U idx,idx_sub,is_plus;//tmp,
    INT8U item_priority_idx;
    INT8U	day_month_hold_td[13]={0};//5�ֽڵ��ն�ʱ�� +3 �ֽڵ� �ն���ʱ�� + 2�ֽڵ��¶���ʱ��
    #ifdef __READ_901F_TRY_3__
    INT8U try_date[4];
    #endif
    INT32U last_read_phy_offset = 0;
    INT32U last_read_phy_block_offset;
    INT8U need_read_item_no =0;
    INT8U gb_645_zuhe_frame[200] = {0};
    INT8U zuhe_frame_len = 0;
    INT8U parall_mode = 0;
    INT8U temp[4] = {0};
    BOOLEAN dayhold_syje_flag = FALSE;
#if (defined __PROVICE_CHONGQING__) || (defined __PROVICE_CHONGQING_FK__) || (defined __PROVICE_SHANGHAI__)
    INT8U hour_minute[2] = {0};
    #endif
    is_plus = 0;
    item_priority_idx = 0;
    mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    #ifdef __READ_OOP_METER__
    INT8U oad_byte[4];
    INT8U oad_count=0;
    #endif
    INT8U load_time[2];	
    INT8U resp_byte;

    if (get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;
    if (check_is_all_ch(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,0x00))
    {
        clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);

        return FALSE;
    }
    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
      if( portContext_plc.plc_other_read_mode == CYCLE_REC_MODE_PARALLEL )
      {
         #ifdef __READ_DLMS_METER__
             parall_mode = 0; //dlmsһ�η�һ��
         #else
             parall_mode = 0xAA;
         #endif
      }
    }

    #if (defined __PROVICE_CHONGQING__) || (defined __PROVICE_CHONGQING_FK__)
    /*
     * ���켯���� 485 0:10���Ӻ�ſɳ�������
     *            �ز���:��ʱ����ʱ�ο��ư�   
     * ����ר��   485 0:15���Ӻ�ſɳ�������
     */
    if( (read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS) )
    {
        tpos_enterCriticalSection();
        mem_cpy(hour_minute,datetime+MINUTE,2);
        tpos_leaveCriticalSection();
        #if (defined __PROVICE_CHONGQING__)
        if( (0x00 == hour_minute[1]) && (hour_minute[0] < 10) )
        #else
        if( (0x00 == hour_minute[1]) && (hour_minute[0] < 15) )
        #endif
        {
            clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
            return FALSE;
        }
    }
    #endif
    #if (defined __PROVICE_CHONGQING_FK__)
    if( (read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS) )
    {
        
        
    }
    #endif
    
    #ifdef __PROVICE_SHANGHAI__
    /*
     * �Ϻ�������Ҫ�� 0:10���Ӻ�ſɳ����ն���  
     */
    tpos_enterCriticalSection();
    mem_cpy(hour_minute,datetime+MINUTE,2);
    tpos_leaveCriticalSection();
    if( (0x00 == hour_minute[1]) && (hour_minute[0] < 10) )
    {
        return FALSE;
    }
    #endif
    #ifdef __POWER_CTRL__
    spot_idx = bin2_int16u(read_params->meter_doc.spot_idx); //������⣬Э������е���������ֱ���жϸò������Ƿ������塣
    is_plus = is_pulse_meter(spot_idx);
    #endif

    #ifdef __READ_OOP_METER__
    if ((read_params->meter_doc.protocol == GB645_2007)||(read_params->meter_doc.protocol == GB_OOP))
    #else
    if (read_params->meter_doc.protocol == GB645_2007)
    #endif
    {
        if(read_params->control.day_hold_td_flag) //��Ҫ�����ն����ն���ʱ��
        {
        	
            if (check_is_all_ch(read_params->day_hold_td,3,0x00))
            {
                fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                if( compare_string(day_month_hold_td+2,datetime+DAY,3) == 0 )// �ǽ��쳭���Ķ���ʱ��
                {
                    //�ն�����¶����ʱ�긳ֵ
                    mem_cpy(read_params->day_hold_td,day_month_hold_td+5,3);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    mem_cpy(read_params->month_hold_td,day_month_hold_td+8,2);
                    #endif
                    if( compare_string(day_month_hold_td+10,datetime+DAY,3) != 0)
                    {
                        #if (defined __CHONGQING_DAY_HOLD_CTRL__) || (defined __PROVICE_SHANGHAI__)	
						//����ʱ�겻�ԣ��������ˣ������������Ҳ�����ظ�����,���ʱ����ϣ�Ҫ��ô����
						//��������?��ʱ���������������?
                        clr_day_hold_mask_flag(read_params);
            			#endif
                      if(read_params->control.according_to_day_hold_td_save == 0x02)
                      {
                        //������������������ǰ
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXYG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXYG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZXWG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FXWG_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG1_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG2_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG3_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_WG4_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_ZYG_ZDXL_DH);
                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FYG_ZDXL_DH);

                        clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_GDCS_BLOCK);
                      }
                    }
                }
                else
                {
                    if(get_data_item(0x00002C00,read_params->meter_doc.protocol,&library))
                    {
                        if(library.item != 0xFFFFFFFF)
                        {
                            int32u2_bin(0x00002C00,read_params->phy);
                            int32u2_bin(library.item,read_params->item);
                            read_params->resp_byte_num = 40;
                            read_params->read_type = READ_TYPE_CYCLE_DAY;
                            #ifdef __READ_OOP_METER__
                            if (read_params->meter_doc.protocol == GB_OOP)
                            {
                                int32u2_bin_reserve(library.item, oad_byte);
                                *frame_len = (INT8U)make_oop_read_frame(frame, read_params->meter_doc.meter_no, oad_byte, 1, 0x5004, 1);
                                mem_cpy(read_params->phy_bak, (INT8U *)&library.phy, 4);
                                mem_cpy(read_params->oad, oad_byte, 4);
                                read_params->oad_cnt=1;
                            }
                            else
                            #endif
                            {
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #ifdef __SOFT_SIMULATOR__
                            snprintf(info,100,"*** prepare item day hold : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                                 meter_idx,library.item,0x00002C00);
                            debug_println_ext(info);
                            #endif

                            return TRUE;
                        }
                    }
                }
            }

        }
    }

    if (check_is_all_ch(read_params->day_hold_td,3,0x00))
    {
        get_yesterday(read_params->day_hold_td);
        #ifndef __CHECK_MONTH_HOLD_TD__
        get_former_month(read_params->month_hold_td);
        #else
        if(read_params->meter_doc.protocol != GB645_2007)
        {
            get_former_month(read_params->month_hold_td);
        }
        #endif
    }
    #ifdef __READ_IEC1107_METER__
    if(read_params->meter_doc.protocol==IEC1107)
    {
        clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;        
    }
    #endif    
    
    #ifdef __READ_DLMS_METER__
CHECK_LOGIN:
    if(read_params->meter_doc.protocol == METER_DLMS) //��check�Ƿ�������Ҫ�����������һֱ��½
    {
    if(read_params->dlms_login_ok)
    {
        if(read_params->login_try_count > 10)
        {
            read_params->read_ctrl_state = 2;//��½��10�λ�û��½�ɹ����Ͽ����µ�¼
        }
        if(read_params->read_ctrl_state ==0)
        {
            if(read_params->read_ctrl_step==0)
            {
                *frame_len = make_dlms_snrm(frame,read_params->meter_doc.meter_no);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms snrm  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                read_params->login_try_count ++;
            }
            else if(read_params->read_ctrl_step==1)
            {
                *frame_len = make_dlms_aarq(frame,read_params->meter_doc.meter_no,0,0);
                read_params->dlms_SSS = 1;
                read_params->dlms_RRR = 1;
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms aarq  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
               read_params->login_try_count ++;
            }
            else if(read_params->read_ctrl_more_frame)
            {
                *frame_len = make_dlms_rr(frame,read_params->meter_doc.meter_no,read_params->dlms_RRR);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms rr  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                read_params->login_try_count ++;
            }
            return TRUE;
        }
        else if(read_params->read_ctrl_state == 2)
        {
            *frame_len = make_dlms_disc(frame,read_params->meter_doc.meter_no);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"dlms disc");
            debug_println_ext(info);
            #endif
            read_params->login_try_count = 0;
            return TRUE;
        }
     }
    }
    #endif
    for(idx=0;idx<sizeof(CYCLE_DAY_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        //#ifdef __ITEM_PRIORITY__

        if (CYCLE_DAY_PHY_LIST[idx].phy == 0xFFFFFFFF)
        {
            if(parall_mode == 0xAA)
            {
                continue;
            }
            else if(check_is_sqr_protocol(read_params->meter_doc.protocol))
            {
                continue;
            }
            else
            {
                if (item_priority_idx < ITEM_PRIORITY_CYCLE_DAY_COUNT)
                {
                    if (get_bit_value(read_priority_ctrl_item_cycle_day[item_priority_idx],READ_FLAG_BYTE_NUM,meter_idx))
                    {
                        read_params->is_read_priority_ctrl = 1;
                        clr_bit_value(read_priority_ctrl_item_cycle_day[item_priority_idx],READ_FLAG_BYTE_NUM,meter_idx);
                    }
                }
                item_priority_idx++;
                continue;
            }
        }

         if(dayhold_syje_flag)/*flagΪTRUEʱ����ʾ�Ѿ�׼�����ն���ʣ����������*/
         {
            if(CYCLE_DAY_PHY_LIST[idx].phy == 0x00001240 ) /*������ڳ��ն���ʣ�����Ҫ����ǰʣ���������*/
            {
                continue;
            }

         }
        //#endif
        if(CYCLE_DAY_PHY_LIST[idx].offset != last_read_phy_offset)  /*���ƫ�ƶ����ˣ�˵����ǰ��Ӧ��������϶�Ҫ�ж��Ƿ񳭶������û�䣬����֮ǰ��ƫ�Ʊȶ�*/
        {
            last_read_phy_block_offset = 0;
        }
        
        for(idx_sub=0;idx_sub<((CYCLE_DAY_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            if(get_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,mask_idx))
            {
                get_phy_data((READ_WRITE_DATA*)&(CYCLE_DAY_PHY_LIST[idx]),idx_sub,&phy);

                #ifdef __READ_OOP_METER__
                if((parall_mode == 0xAA)||(read_params->meter_doc.protocol == GB_OOP))
                #else
                if(parall_mode == 0xAA)
                #endif
                {
                    if(CYCLE_DAY_PHY_LIST[idx].offset == last_read_phy_offset)   //����洢һ�����ڶ����Ȳ�Ҫ��
                    {

                        if(CYCLE_DAY_PHY_LIST[idx].block_offset == last_read_phy_block_offset) /*�������ݣ�ͨ��offset���֡��������ඳ��F153�����ݣ����ж�Ϊһ���洢�������ܷ�3���·�*/
                        {
                            mask_idx++;
                            continue ;
                        }
                        last_read_phy_block_offset = CYCLE_DAY_PHY_LIST[idx].block_offset;
                    }

                    last_read_phy_offset = CYCLE_DAY_PHY_LIST[idx].offset;
                }
                else
                {

                }
                if(readdata_cycle_day(meter_idx,&phy,read_params->day_hold_td,frame,frame+5,frame_len,NULL) == FALSE)
                {
                    //��Ҫ����
                    //����Լ�⺯��
                    if(get_data_item(phy.phy,read_params->meter_doc.protocol,&library))
                    {
                        if(library.item != 0xFFFFFFFF)
                        {
                            #ifdef __READ_901F_TRY_3__
                            if((library.item == 0x901F) || (library.item == 0x9E10))
                            {
                              fread_meter_params(meter_idx,PIM_METER_F128,try_date,4);
                              if(compare_string(try_date,datetime+3,3) == FALSE )
                              {
                                if(try_date[3] >= 2)
                                {
                                clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CYCLE_DAY,mask_idx);
                                clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,mask_idx);
                                }
                                else
                                {
                                try_date[3] ++;
                                fwrite_meter_params(meter_idx,PIM_METER_F128,try_date,4);
                                }
                              }
                              else
                              {
                                mem_cpy(try_date,datetime+3,3);
                                try_date[3] = 0;
                                fwrite_meter_params(meter_idx,PIM_METER_F128,try_date,4);
                              }
                            }
                            #endif

                            #ifdef __PROVICE_GANSU__
                            if((read_params->meter_doc.meter_class.meter_class == 7)&&(library.item == 0x00900200))
                            {
                                 library.item = 0x05900200;
                            }
                            #endif
                            
                          //���ж�̬�жϣ��Ƿ񳭶���ʱ��������
                          #ifdef __PROVICE_JILIN_TIMEHOLD__
                          if(check_const_ertu_switch(CONST_ERTU_SWITCH_SETTIME_DAYHOLD_DATA))
                          library.item = check_settime_dayhold_data(library.item);
                          #endif
                           #ifdef __PLC_REC_VOLTMETER1__
                           if(phy.phy == DY_HOUR_HOLD )
                           {
                           read_params->patch_num = check_plc_router_save_vlotmeter_curve_data(meter_idx,&phy,read_params->patch_num);
                           if(read_params->patch_num > 23)
                           {
                            clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
                            read_params->patch_num = 0 ;
                             return false;
                           }
                           library.item += read_params->patch_num;

                           }
                           #endif
                           #ifdef __PROVICE_ABROAD_DEMO__
                          if(phy.phy == 0x00000480)
                          {
                           library.item = 0x05200901;
                          }
                          #endif
                          if(parall_mode == 0xAA)
                          {
                              if(phy.phy == S1C_RDJ_SYJE) /*׼�����ն���ʣ�����������¼һ����ʶ�����������ʱ�򣬲�Ҫ׼����ǰʣ������*/
                              {
                                 dayhold_syje_flag = TRUE;
                              }

                              int32u2_bin(phy.phy,read_params->phy+need_read_item_no*4);
                              int32u2_bin(library.item,read_params->item+need_read_item_no*4);
                          }
                          else
                          {
                              int32u2_bin(phy.phy,read_params->phy);
                              int32u2_bin(library.item,read_params->item);
                          }

                            
                            resp_byte = CYCLE_DAY_PHY_LIST[idx].data_len + 10;
                            if((resp_byte)<40)
                            {
                                read_params->resp_byte_num = 40;
                            }
                            else
                            {
                                read_params->resp_byte_num = (resp_byte);
                            }

                            read_params->read_type = READ_TYPE_CYCLE_DAY;

                            if (read_params->meter_doc.protocol == GB645_2007)
                            {
                              #ifdef __PROVICE_YUNNAN__
                              meter_num = get_readport_meter_count_from_fast_index(COMMPORT_PLC);
                              if(meter_num <16)
                              {
                              library.item = yunnan_check_dayhold_data(library.item);
                                *frame_len = make_gb645_yunnan_mode3_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                              }
                              else
                              #endif
                                if( ISRAEL_HOUR_DATA == phy.phy)
                                {
                                    load_time[0] = 0;
                                    load_time[1] = 0;
                                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,2);
                                }
                                else
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            else if ((read_params->meter_doc.protocol == GB645_1997) || (read_params->meter_doc.protocol == GB645_1997_JINANGSU_4FL)
                                    || (read_params->meter_doc.protocol == GB645_1997_JINANGSU_2FL) || (is_plus))
                            {
                                *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            else if (read_params->meter_doc.protocol == GUANGXI_V30)
                            {
                                *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #ifdef __READ_DLMS_METER__
                            else if((read_params->meter_doc.protocol == METER_DLMS))
                            {
                                 if(read_params->dlms_login_ok == 0) /*�б���Ҫ���ͣ��ͷ��ص�¼����*/
                                 {
                                    int32u2_bin(0,read_params->phy);
                                    read_params->dlms_login_ok = 1;
                                    goto
                                        CHECK_LOGIN;
                                 }

                                *frame_len = make_dlms_read_energy(frame,read_params->meter_doc.meter_no,(INT8U*)library.item,read_params->dlms_SSS,read_params->dlms_RRR,3);
                                read_params->dlms_SSS ++;
                                read_params->dlms_RRR ++;
                            }
                            #endif
                            #ifdef __SH_2009_METER__
                            else if (read_params->meter_doc.protocol == SHANGHAI_2009)
                            {
                                *frame_len = make_sh_2009_meter_read_frame(frame,read_params->meter_doc.meter_no,library.item);
                            }
                            #endif
                            #ifdef __JIANGSU_READ_CURRENT_MONITOR__
                            else if (read_params->meter_doc.protocol == BREAKER_MONITOR)
                            {
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #endif
                            #ifdef __FUJIAN_CURRENT_BREAK__
                            else if (read_params->meter_doc.protocol == FUJIAN_BREAKER_MONITOR)
                            {
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #endif
                            #ifdef __READ_OOP_METER__
                            else if(read_params->meter_doc.protocol == GB_OOP)
                            {
                                oad_byte[0] = library.item>>24;
                                oad_byte[1] = library.item>>16;
                                oad_byte[2] = library.item>>8;
                                oad_byte[3] = library.item;
                                
                                if(0 == oad_count)
                                {
                                    read_params->oad_cnt = 0;
                                }
                                if(library.item == 0xFFFFFFFF)
                                {
                                    clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,mask_idx);
                                    mask_idx++;
                                    continue;
                                    }
                                mem_cpy(read_params->phy_bak+oad_count*4, (INT8U *)&library.phy, 4);
                                mem_cpy(read_params->oad+oad_count*4, oad_byte, 4);
                                oad_count++;
                                if(oad_count < PARALL_MAX_OAD)
                                    {
                                    mask_idx++;
                                    continue;
                                }
                                *frame_len = make_oop_read_frame(frame, read_params->meter_doc.meter_no, read_params->oad, oad_count, 0x5004, 1);//todo:���ʵʱת���Ὺ��
                                read_params->oad_cnt = oad_count;
                                return TRUE;
                            }
                            #endif
                            else if(check_is_sqr_protocol(read_params->meter_doc.protocol))
                            {
                               //  #ifdef __WATERMETER_READ_NUM_CONTROL__
                                 if(portContext_plc.watermeter_read_num_control != 0)  //˵�����λ���в��������������ܿ�,
                                 {
                                     fread_array(meter_idx,PIM_WATERMETER_READ_NUM,temp,4);
                                     if(compare_string(temp,datetime+DAY,3) != 0) /*���ڲ�һ��*/
                                     {
                                           mem_cpy(temp,datetime+DAY,3);
                                           temp[3] = 1;
                                           fwrite_array(meter_idx,PIM_WATERMETER_READ_NUM,temp,4);
                                     }
                                     else
                                     {

                                         if(temp[3] >= portContext_plc.watermeter_read_num_control)
                                         {
                                            return FALSE;
                                         }
                                         temp[3] ++;
                                         fwrite_array(meter_idx,PIM_WATERMETER_READ_NUM,temp,4);
                                     }
                                 }
                               //  #endif
                                if(read_params->meter_doc.protocol == TIANFU)
                                {
                                    library.item=0xE5E50081;
                                    int32u2_bin(library.item,read_params->item);
                                }
                                *frame_len = make_dzc_read_frame(frame,(METER_DOCUMENT*)&(read_params->meter_doc),library.item,NULL,0);
                            }
                            else  //������Լ�Ĳ�ִ�У�ֱ�����flag������������
                            {
                                clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
                                return FALSE;
                            }
                            #ifdef __SOFT_SIMULATOR__
                            snprintf(info,100,"*** prepare item day hold : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
                                    meter_idx,library.item,phy.phy,mask_idx);
                            debug_println_ext(info);
                            #endif

                            read_params->cur_mask_idx = mask_idx;

                            if((parall_mode == 0xAA) &&((read_params->meter_doc.protocol == GB645_2007)
                            || (read_params->meter_doc.protocol == GB645_1997)))
                            {
                                mem_cpy(gb_645_zuhe_frame+zuhe_frame_len,frame, *frame_len );
                                zuhe_frame_len += *frame_len;
                                need_read_item_no ++ ;

                                if(need_read_item_no >=(portContext_plc.parall_max_item)) /*һ����������645֡��*/
                                {
                                  mem_cpy(frame,gb_645_zuhe_frame,zuhe_frame_len);
                                  *frame_len = zuhe_frame_len;
                                   return TRUE;
                                }
                                else
                                {
                                   mask_idx++;
                                   continue;
                                }
                            }
                            else
                            {
                                return TRUE;
                            }
                        }
                        else
                        {
                            //����msak
                            clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CYCLE_DAY,mask_idx);
                        }
                    }
                    else
                    {
                        //����msak
                        clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CYCLE_DAY,mask_idx);
                    }
                }
                clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,mask_idx);
            }
            mask_idx++;
        }
    }
    #ifdef __READ_OOP_METER__
    if(oad_count>0)
    {
        *frame_len = make_oop_read_frame(frame, read_params->meter_doc.meter_no, read_params->oad, oad_count, 0x5004, 1);//todo:���ʵʱת���Ὺ��
        read_params->oad_cnt = oad_count;
        return TRUE;
    }
    #endif
    if(parall_mode == 0xAA)
    {
        if(need_read_item_no )
        {
           mem_cpy(frame,gb_645_zuhe_frame,zuhe_frame_len);
           *frame_len = zuhe_frame_len;
           return TRUE;
        }
        else
        {
           clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
        }
    }
    else
    {
       clr_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
    }


    return FALSE;
}
//---------------------------------------------------------------------------
INT8U get_phy_form_list_cycle_month(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(CYCLE_MONTH_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((CYCLE_MONTH_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(CYCLE_MONTH_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}

INT32U get_cycle_month_save_offset(READ_WRITE_DATA *phy,INT8U td[2])
{
    INT32U offset;

    offset = td[0] - 1;
    offset *= phy->data_len;
    offset += phy->offset;
    return offset;
}

INT8U writedata_cycle_month(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[2],INT8U *data,INT8U datalen,INT8U* buffer)
{
    INT32U offset;
	#if (defined __PROVICE_JIANGSU__)
    METER_DOCUMENT meter_doc;
	#endif
	#if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
    //METER_DOCUMENT meter_doc;
    C2_F27F35 c2_f35;
    INT32U  over_high_time = 0x0000FFFF; //Խ����
    INT32U  over_low_time  = 0x0000FFFF; //���� 
    BOOLEAN valid_bcd = FALSE;
    #endif
    #ifdef __HIGH_PRECISION_DATA__
    INT8U real_datalen;
    real_datalen = datalen;
    #endif
    offset = get_cycle_month_save_offset(phy,td);

    fread_array(meter_idx,offset,buffer,phy->data_len);
    #ifndef __INDONESIA_DLMS_TEST__/*Ҫ���¶�����14�Ŵ�����һ���У��˴��������ƣ���Ҫ����*/
    if(FALSE == is_month_hold_td_valid(td))
    {
        return TRUE;
    }
    #endif
    //����ʱ��
    if(compare_string(buffer,td,2) != 0)
    {
        mem_set(buffer,phy->data_len,0xFF);
        mem_cpy(buffer,td,2);
         #ifdef __PROVICE_TIANJIN__
        buffer[2]=0x00;
        buffer[3]=0x00;
        mem_cpy(buffer+4,datetime+DAY,3);
        #elif defined __PROVICE_JIANGSU__
        fread_meter_params(meter_idx,PIM_METER_DOC,meter_doc.value,sizeof(METER_DOCUMENT));
        #ifdef __READ_OOP_METER__
        if((meter_doc.protocol == GB645_2007)||(meter_doc.protocol == GB_OOP))
        #else
        if(meter_doc.protocol == GB645_2007)
        #endif
        {
            buffer[2]=59;
            buffer[3]=23;
            get_last_day_of_former_month(buffer+4);
        }
        else
            mem_cpy(buffer+2,datetime+MINUTE,5);
        #else
        mem_cpy(buffer+2,datetime+MINUTE,5);
        #endif
    }

    if (phy->flag & SAVE_FLAG_XL_TIME)
    {
        (void)format_data_save(0,data,datalen,buffer+7,phy);
    }
    else
    {
        datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
        #ifdef __PROVICE_JIANGSU__
        if((meter_doc.protocol == GB645_1997)&&(datalen == 12))
        {
            mem_set(buffer+7+phy->block_offset, 25, 0x00);
            mem_cpy(buffer+7+phy->block_offset, data, 4);//��
            mem_cpy(buffer+7+phy->block_offset+8, data+4, 4);//��
            mem_cpy(buffer+7+phy->block_offset+16, data+8, 4);//��
        }
        else
        {
            mem_cpy(buffer+7+phy->block_offset,data,datalen);
        }
       #else
       mem_cpy(buffer+7+phy->block_offset,data,datalen);

        #ifdef __HIGH_PRECISION_DATA__
        if((phy->phy >= (ZXYG_DN_SJK-0x3F)) && (phy->phy <= D4XX_WG_DN_SJK))
        {
            if((datalen == 4) && (real_datalen ==5))
            {
                mem_cpy(buffer+7+phy->block_offset,data+1,datalen);
            }
        }
        #endif
       #endif
    }

    fwrite_array(meter_idx,offset,buffer,phy->data_len);

    #ifdef  __PLC_REC_VOLTMETER1__
    if((phy->phy == Y_DY_A_HGL) || (phy->phy == Y_DY_B_HGL) || (phy->phy == Y_DY_C_HGL))
    plc_router_save_voltmeter_data(meter_idx,phy->phy,buffer+7,phy->data_len,1);
    #endif

    #if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
    if( (phy->phy == S1Y_A_DYHGL_TJ_SJ_SJK-0x3F+1) || (phy->phy == S1Y_B_DYHGL_TJ_SJ_SJK-0x3F+1)
      || (phy->phy == S1Y_C_DYHGL_TJ_SJ_SJK-0x3F+1) )
    {
        //
        mem_set(meter_doc.value,sizeof(METER_DOCUMENT),0x00);
        fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
        if(meter_doc.meter_class.user_class == 14)
        {
            //
            if((phy->phy == S1Y_A_DYHGL_TJ_SJ_SJK-0x3F+1) )
            {
                // ֮ǰ��⴦��ɲ���00��EE�������Ȱ���EE�����������ݼ�����޸� TODO ????
                mem_set(c2_f35.value,sizeof(C2_F27F35),0xEE);
            }
            else
            {
                offset = td[0]-1;
                offset *= sizeof(C2_F27F35);
                offset += PIM_C2_F35;
                fread_array(meter_idx,offset,c2_f35.value,sizeof(C2_F27F35));
            }
            if(datalen >= 12)
            {
                over_high_time = bcd2u32(data+9,3,&valid_bcd);
                if(FALSE == valid_bcd)
                {
                    over_high_time = 0x0000FFFF;
                }
            }
            if(datalen >= 15)
            {
                over_low_time  = bcd2u32(data+12,3,&valid_bcd);
                if(FALSE == valid_bcd)
                {
                    over_low_time = 0x0000FFFF;
                }
            }

            if(phy->phy == S1Y_A_DYHGL_TJ_SJ_SJK-0x3F+1)
            {
                //AԽ�����ۼ�ʱ��
                c2_f35.va_over_time[4] = (INT8U )(over_high_time);
                c2_f35.va_over_time[5] = (INT8U )(over_high_time>>8);
                // A Խ�����ۼ�ʱ��
                c2_f35.va_over_time[6] = (INT8U )(over_low_time);
                c2_f35.va_over_time[7] = (INT8U )(over_low_time>>8);
                if(datalen >= 21)
                {
                    //��ߵ�ѹ ����ʱ��
                    mem_cpy(c2_f35.va_max,data+15,2);
                    mem_cpy(c2_f35.va_max_time,data+17,3);
                }
                if(datalen >= 27)
                {
                    //��͵�ѹ ����ʱ��
                    mem_cpy(c2_f35.va_min,data+21,2);
                    mem_cpy(c2_f35.va_min_time,data+23,3);
                }
            }
            else if(phy->phy == S1Y_B_DYHGL_TJ_SJ_SJK-0x3F+1)
            {
                //BԽ�����ۼ�ʱ��
                c2_f35.vb_over_time[4] = (INT8U )(over_high_time);
                c2_f35.vb_over_time[5] = (INT8U )(over_high_time>>8);
                // B Խ�����ۼ�ʱ��
                c2_f35.vb_over_time[6] = (INT8U )(over_low_time);
                c2_f35.vb_over_time[7] = (INT8U )(over_low_time>>8);
                if(datalen >= 21)
                {
                    //��ߵ�ѹ ����ʱ��
                    mem_cpy(c2_f35.vb_max,data+15,2);
                    mem_cpy(c2_f35.vb_max_time,data+17,3);
                }
                if(datalen >= 27)
                {
                    //��͵�ѹ ����ʱ��
                    mem_cpy(c2_f35.vb_min,data+21,2);
                    mem_cpy(c2_f35.vb_min_time,data+23,3);
                }
            }
            else
            {
                //cԽ�����ۼ�ʱ��
                c2_f35.vc_over_time[4] = (INT8U )(over_high_time);
                c2_f35.vc_over_time[5] = (INT8U )(over_high_time>>8);
                // c Խ�����ۼ�ʱ��
                c2_f35.vc_over_time[6] = (INT8U )(over_low_time);
                c2_f35.vc_over_time[7] = (INT8U )(over_low_time>>8);
                if(datalen >= 21)
                {
                    //��ߵ�ѹ ����ʱ��
                    mem_cpy(c2_f35.vc_max,data+15,2);
                    mem_cpy(c2_f35.vc_max_time,data+17,3);
                }
                if(datalen >= 27)
                {
                    //��͵�ѹ ����ʱ��
                    mem_cpy(c2_f35.vc_min,data+21,2);
                    mem_cpy(c2_f35.vc_min_time,data+23,3);
                }
            }
            
            
            offset = td[0]-1;
            offset *= sizeof(C2_F27F35);
            offset += PIM_C2_F35;
            fwrite_array(meter_idx,offset,c2_f35.value,sizeof(C2_F27F35));
        }
    }
    #endif
    return TRUE;
}

INT8U readdata_cycle_month(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[2],INT8U rec_datetime[5],INT8U *data,INT8U *datalen)
{
    INT32U offset;

    offset = get_cycle_month_save_offset(phy,td);

    fread_array(meter_idx,offset,data,phy->data_len);
    if ((td[1] == data[1]) && (td[0] == data[0]))
    {
        mem_cpy(rec_datetime,data+2,5);
        if(check_is_all_ch(data+7+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+7+phy->block_offset,phy->block_len);
            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

INT16S app_readdata_cycle_month(INT16U meter_idx,INT32U phy_id,INT8U td[2],INT8U rec_datetime[5],INT8U *data,INT8U max_datalen)
{
    READ_WRITE_DATA phy;
    INT8U idx,datalen;
    INT8U td_bin[2];

	datalen = 0;
    idx = get_phy_form_list_cycle_month(phy_id,&phy);
    if(idx == 0xFF) return -1;
    if (phy.data_len > max_datalen) return -1;
    td_bin[0] = BCD2byte(td[0]);
    td_bin[1] = BCD2byte(td[1]);
    if( FALSE == readdata_cycle_month(meter_idx,&phy,td_bin,rec_datetime,data,&datalen) )
    {
        //coverity�澯����
        datalen = 0;
    }
    return datalen;
}

INT8U save_cycle_month(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    INT8U idx;
    INT8U day_month_hold_td[13];
    INT8U rec_td[2];

#ifdef __CHECK_MONTH_HOLD_TD__
    if ((read_params->meter_doc.protocol == GB645_2007)&&(bin2_int32u(read_params->phy) == 0x00005880)&&((read_params->meter_doc.baud_port.port == COMMPORT_485_REC)||(read_params->meter_doc.baud_port.port == COMMPORT_485_CAS)))    //���ǰʱ��
    {
        if(check_is_all_ch(frame,frame_len,0xEE)) //����
        {
            get_former_month(read_params->month_hold_td);
			fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
            mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
            fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
        }
        else
        {
            read_params->month_hold_td[0] = BCD2byte(frame[2]);    //��
            read_params->month_hold_td[1] = BCD2byte(frame[3]);    //��
//            rs232_debug_info("\xD0\xD0",2);
//            rs232_debug_info(read_params->month_hold_td,2);
            if(compare_string(read_params->month_hold_td,datetime+MONTH,2) != 0)    //���ڲ���ȷ
            {
                get_former_month(rec_td);
                if(compare_string(rec_td,read_params->month_hold_td,2)==0)
                {
//                    rs232_debug_info("\xD1\xD1",2);
                    clr_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                    set_bit_value(read_meter_flag_wati_month_hold,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                }
                else
                {
//                    rs232_debug_info("\xD2\xD2",2);
                    clr_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                }
                return 0;
            }
            else
            {
//                rs232_debug_info("\xD3\xD3",2);
                get_former_month(read_params->month_hold_td);
                fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
            }
        }
    }
#endif
    idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
    if(idx == 0xFF) return 0;
    if (phy.flag & SAVE_FLAG_DENY_NO_SAVE)
    {
        if(check_is_all_ch(frame,frame_len,0xEE)) //����
        {
            //�����������
            clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_CYCLE_MONTH,idx);
            #ifdef __GUANGXI_V3__
            if(phy.phy == (0x00003B80) )
            {
            clr_bit_value(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,idx+1);
            clr_bit_value(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,idx+2);
            clr_bit_value(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,idx+3);
            clr_bit_value(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,idx+4);
            }
            #endif
        }
        else
        {
            #ifdef __PROVICE_TIANJIN__
            if((read_params->meter_doc.baud_port.port != COMMPORT_485_CY) ||((datetime[HOUR] == 0) && (datetime[MINUTE] <= 5)))
            #endif
//            rs232_debug_info("\xD4\xD4",2);
//            rs232_debug_info(read_params->month_hold_td,2);
            writedata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,frame,frame_len,buffer);

            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** save month hold data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d",
                    bin2_int16u(read_params->meter_doc.meter_idx),
                    bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
                    read_params->month_hold_td[1],read_params->month_hold_td[0]);
            debug_println_ext(info);
            #endif
        }
    }
    else
    {
        #ifdef __PROVICE_TIANJIN__
        if((read_params->meter_doc.baud_port.port != COMMPORT_485_CY) ||((datetime[HOUR] == 0) && (datetime[MINUTE] <= 5)))
        #endif
//        rs232_debug_info("\xD5\xD5",2);
//        rs232_debug_info(read_params->month_hold_td,2);
        writedata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,frame,frame_len,buffer);
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** save month hold data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d",
                bin2_int16u(read_params->meter_doc.meter_idx),
                bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
                read_params->month_hold_td[1],read_params->month_hold_td[0]);
        debug_println_ext(info);
        #endif
    }

    clr_bit_value(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,idx);

    return 0;
}

BOOLEAN prepare_read_item_cycle_month(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT16U meter_idx;
	#ifdef __POWER_CTRL__
	INT16U spot_idx = 0;
	#endif
    INT8U idx,idx_sub,mask_idx,is_plus;
    #ifdef __READ_OOP_METER__
    INT8U oad_byte[4];
    INT8U oad_count=0;
    INT32U last_read_phy_offset = 0;
    #endif
#ifdef __CHECK_MONTH_HOLD_TD__
    INT8U day_month_hold_td[13]={0};
    INT8U rec_td[2];
#endif
    is_plus = 0;
    mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
//    if (read_params->control.day_hold_save_month_hold)
    if(0) 
    {
       clr_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,meter_idx);
       return FALSE;
    }
    if (get_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;
    if (check_is_all_ch(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,0x00))
    {
        clr_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }

    #ifdef __READ_IEC1107_METER__
    if(read_params->meter_doc.protocol==IEC1107)
    {
        clr_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;        
    }
    #endif
    #ifdef __CHECK_MONTH_HOLD_TD__
    if((check_is_all_ch(read_params->month_hold_td,2,0x00))&&(read_params->meter_doc.protocol == GB645_2007)&&((read_params->meter_doc.baud_port.port == COMMPORT_485_REC)||(read_params->meter_doc.baud_port.port == COMMPORT_485_CAS)))
    {
        fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
        get_former_month(rec_td);
//        rs232_debug_info("\xC0\xC0",2);
//        rs232_debug_info(rec_td,2);
//        rs232_debug_info(day_month_hold_td+8,2);
        if(compare_string(day_month_hold_td+8,rec_td,2)==0)
        {
            mem_cpy(read_params->month_hold_td,rec_td,2);
        }
        else
        {
            if(get_data_item(0x00005880,read_params->meter_doc.protocol,&library))
            {
                if(library.item != 0xFFFFFFFF)
                {
//                    rs232_debug_info("\xC1\xC1",2);
                    int32u2_bin(0x00005880,read_params->phy);
                    int32u2_bin(library.item,read_params->item);
                    read_params->resp_byte_num = 40;
                    read_params->read_type = READ_TYPE_CYCLE_MONTH;
                    #ifdef __READ_OOP_METER__
                    if (read_params->meter_doc.protocol == GB_OOP)
                    {
                        int32u2_bin_reserve(library.item, oad_byte);
                        make_oop_cur_frame(frame, read_params->meter_doc.meter_no,1,oad_byte);
                        //*frame_len = (INT8U)make_oop_read_frame(frame, read_params->meter_doc.meter_no, oad_byte, 1, 0x5004, 1);
                    }
                    else
                    #endif
                    {
                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                    }
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** prepare item day hold : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                         meter_idx,library.item,0x00002C00);
                    debug_println_ext(info);
                    #endif
    
                    return TRUE;
                }
            }
        }
    }
    #endif
    #ifdef __POWER_CTRL__
    spot_idx = bin2_int16u(read_params->meter_doc.spot_idx); //������⣬Э������е���������ֱ���жϸò������Ƿ������塣
    is_plus = is_pulse_meter(spot_idx);
    #endif

    if(check_is_all_ch(read_params->month_hold_td,2,0x00))
    {
        get_former_month(read_params->month_hold_td);
    }
    #ifdef __READ_DLMS_METER__
    #ifdef __INDONESIA_DLMS_TEST__
    get_indonesia_month_hold_td(read_params->month_hold_td);
    #endif
CHECK_LOGIN:
    if(read_params->meter_doc.protocol == METER_DLMS) //��check�Ƿ�������Ҫ�����������һֱ��½
    {
    if(read_params->dlms_login_ok)
    {
        if(read_params->login_try_count > 10)
        {
            read_params->read_ctrl_state = 2;//��½��10�λ�û��½�ɹ����Ͽ����µ�¼
        }
        if(read_params->read_ctrl_state ==0)
        {
            if(read_params->read_ctrl_step==0)
            {
                *frame_len = make_dlms_snrm(frame,read_params->meter_doc.meter_no);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms snrm  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                read_params->login_try_count ++;
            }
            else if(read_params->read_ctrl_step==1)
            {
                *frame_len = make_dlms_aarq(frame,read_params->meter_doc.meter_no,0,0);
                read_params->dlms_SSS = 1;
                read_params->dlms_RRR = 1;
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms aarq  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
               read_params->login_try_count ++;
            }
            else if(read_params->read_ctrl_more_frame)
            {
                *frame_len = make_dlms_rr(frame,read_params->meter_doc.meter_no,read_params->dlms_RRR);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms rr  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                read_params->login_try_count ++;
            }
            return TRUE;
        }
        else if(read_params->read_ctrl_state == 2)
        {
            *frame_len = make_dlms_disc(frame,read_params->meter_doc.meter_no);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"dlms disc");
            debug_println_ext(info);
            #endif
            read_params->login_try_count = 0;
            return TRUE;
         }
     }
    }
    #endif
    for(idx=0;idx<sizeof(CYCLE_MONTH_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((CYCLE_MONTH_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            if(get_bit_value(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,mask_idx))
            {
                get_phy_data((READ_WRITE_DATA*)&(CYCLE_MONTH_PHY_LIST[idx]),idx_sub,&phy);
                if(readdata_cycle_month(meter_idx,&phy,read_params->month_hold_td,frame,frame+5,frame_len) == FALSE)
                {
                    //��Ҫ����
                    //����Լ�⺯��
                    if(get_data_item(phy.phy,read_params->meter_doc.protocol,&library))
                    {
                        if(library.item != 0xFFFFFFFF)
                        {
                            #ifdef  __PLC_REC_VOLTMETER1__
                            if((phy.phy == Y_DY_A_HGL) || (phy.phy == Y_DY_B_HGL) || (phy.phy == Y_DY_C_HGL))
                            {
                            library.item = check_month_hold_dyhgl_data(library.item);
                            int32u2_bin(phy.phy,read_params->phy);
                            int32u2_bin(library.item,read_params->item);
                            read_params->resp_byte_num = 40;
                            read_params->read_type = READ_TYPE_CYCLE_MONTH;
                            }
                            else
                            #endif
                            {
                            int32u2_bin(phy.phy,read_params->phy);
                            int32u2_bin(library.item,read_params->item);
                            read_params->resp_byte_num = 40;
                            read_params->read_type = READ_TYPE_CYCLE_MONTH;
                            }
                            #ifdef __PLC_GUANGXI_VIP_CURVE__
                            read_params->read_type = READ_TYPE_CYCLE_MONTH;
                            #endif

                            if((read_params->meter_doc.protocol == GB645_1997) || (read_params->meter_doc.protocol == GUANGXI_V30) ||
                            (read_params->meter_doc.protocol == GB645_1997_JINANGSU_4FL) || (read_params->meter_doc.protocol == GB645_1997_JINANGSU_2FL)
                             || (is_plus))
                            {
                                *frame_len = make_gb645_1997_frame(frame,read_params->meter_doc.meter_no,0x01,(INT16U)(library.item),NULL,0);
                            }
                            #ifdef __SH_2009_METER__
                            else if (read_params->meter_doc.protocol == SHANGHAI_2009)
                            {
                                *frame_len = make_sh_2009_meter_read_frame(frame,read_params->meter_doc.meter_no,library.item);
                            }
                            #endif
                            else if(read_params->meter_doc.protocol == GB645_2007)
                            {
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #ifdef __READ_OOP_METER__
                            else if(read_params->meter_doc.protocol == GB_OOP)
                            {
                                if(CYCLE_MONTH_PHY_LIST[idx].offset == last_read_phy_offset)
                                {
                                    mask_idx++;
                                    continue;
                                }
                                last_read_phy_offset = CYCLE_MONTH_PHY_LIST[idx].offset;
                                
                                oad_byte[0] = library.item>>24;
                                oad_byte[1] = library.item>>16;
                                oad_byte[2] = library.item>>8;
                                oad_byte[3] = library.item;
                                
                                if(0 == oad_count)
                                {
                                    read_params->oad_cnt = 0;
                                }
                                if(library.item == 0xFFFFFFFF)
                                {
                                    clr_bit_value(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,mask_idx);
                                    mask_idx++;
                                    continue;
                                }
                                mem_cpy(read_params->phy_bak+oad_count*4, (INT8U *)&library.phy, 4);
                                mem_cpy(read_params->oad+oad_count*4, oad_byte, 4);
                                oad_count++;
                                if(oad_count < PARALL_MAX_OAD)
                                {
                                    mask_idx++;
                                    continue;
                                }
                                *frame_len = make_oop_read_frame(frame, read_params->meter_doc.meter_no, read_params->oad, oad_count, 0x5006, 1);//todo:���ʵʱת���Ὺ��
                                read_params->oad_cnt = oad_count;
                                return TRUE;
                            }
                            #endif
                            #ifdef __READ_DLMS_METER__
                            else if((read_params->meter_doc.protocol == METER_DLMS))
                            {
                                 if(read_params->dlms_login_ok == 0)
                                 {
                                    int32u2_bin(0,read_params->phy);
                                    read_params->dlms_login_ok = 1;
                                    goto
                                        CHECK_LOGIN;
                                 }


                                *frame_len = make_dlms_read_energy(frame,read_params->meter_doc.meter_no,(INT8U*)library.item,read_params->dlms_SSS,read_params->dlms_RRR,3);
                                read_params->dlms_SSS ++;
                                read_params->dlms_RRR ++;

                            }
                            #endif
                            else
                            {
                            clr_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,meter_idx);
                            return FALSE;
                            }
                            #ifdef __SOFT_SIMULATOR__
                            snprintf(info,100,"*** prepare item month hold : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
                                    meter_idx,library.item,phy.phy,mask_idx);
                            debug_println_ext(info);
                            #endif
                            read_params->cur_mask_idx = mask_idx;
                            return TRUE;
                        }
                    }
                }
                clr_bit_value(read_params->read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,mask_idx);
            }
            mask_idx++;
        }
    }

    #ifdef __READ_OOP_METER__
    if(oad_count>0)
    {
        *frame_len = make_oop_read_frame(frame, read_params->meter_doc.meter_no, read_params->oad, oad_count, 0x5006, 1);//todo:���ʵʱת���Ὺ��
        read_params->oad_cnt = oad_count;
        return TRUE;
    }
    #endif
    clr_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,meter_idx);

    return FALSE;
}
//---------------------------------------------------------------------------
INT8U get_phy_form_list_cruve(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(CRUVE_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((CRUVE_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(CRUVE_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            else if ((63 == phy%64) && (out->phy == (phy - 63)))
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}

INT32U get_curve_save_offset(READ_WRITE_DATA *phy,INT8U td[5],INT8U midu)
{
    INT32U offset;
    INT8U num;

    num = 60 / midu;
    offset = getPassedDays(2000+td[4],td[3],td[2]);
    offset *= num * 24;
    offset += td[1] * num;
    offset += td[0] / midu;
    offset = offset % SAVE_POINT_NUMBER_CURVE;
    offset *= phy->data_len;
    offset += phy->offset;
    offset++;     //�ܶ�

    return offset;
}
 INT32U get_agp_curve_save_offset(READ_WRITE_DATA *phy,INT8U td[5],INT8U midu)
{
    INT32U offset;
    INT8U num;

    num = 60 / midu;
    offset = getPassedDays(2000+td[4],td[3],td[2]);
    offset *= 96;
    offset += td[1] * num;
    offset += td[0] / midu;
    offset = offset % SAVE_POINT_NUMBER_AGP_CURVE;
    offset *= phy->data_len;
    offset += phy->offset;
    //offset++;     //�ܶ�

    return offset;
}

//#if (defined __HOUR_CURVE_READ_SELF_ADAPTION__)
READ_LOAD_RECORD_PHY const load_record_list[]=
{
    {FHJL1_DY_DL_PL         },
    {FHJL2_YG_WG_GL         },
    {FHJL3_GL_YS            },
    {FHJL4_ZFXYG            },
    {FHJL5_SXX_WG           },
    {FHJL6_XL               },

    {FHJL1_DY_14            },//
    {FHJL2_DL_14            },
    {FHJL3_YGGL_14          },
    {FHJL4_WGGL_14          },
    {FHJL5_GLYS_14          },
    {FHJL6_YG_WG_ZDN_14     }, 
    {FHJL7_SXX_WG_ZDN_14    },
    {FHJL8_DQXL_14          },
};

#define PHY_MASK 0xFFFFFFF0 // ���� 
BOOLEAN check_load_record_phy(INT32U phy)
{
    INT8U idx = 0;
    for(idx=0;idx<sizeof(load_record_list)/sizeof(READ_LOAD_RECORD_PHY);idx++)
    {
        if( (load_record_list[idx].phy == phy) || ((load_record_list[idx].phy-0x3F) == (phy & PHY_MASK)) )
        {
            return TRUE;
        }
    }
    return FALSE; // ��Ч
}

#if (defined __HOUR_CURVE_READ_SELF_ADAPTION__)
/**/
INT8U extract_load_record_data(READ_PARAMS *read_params,READ_WRITE_DATA *phy,INT8U td[5],INT8U *data,INT8U datalen)
{
    //
    INT8U expect_len = 0;
    INT8U tmp_len = 0;
    INT8U record_len = 0;
    INT8U recv_td[5] = {0};
    INT8U data_pos = 0;
    INT8U check_AA_pos = 0;
    switch(phy->phy)
    {
        case FHJL1_DY_DL_PL:
            //mem_cpy(data,data+6,datalen-6);
            //datalen -= 6;
            if(datalen >= 8 )// A0 A0 +1�ֽ���+5 ���ɼ�¼ʱ��
            {
                buffer_bcd_to_bin(data+3,recv_td,5);
                record_len = data[2];// ���ɼ�¼�ֽ��� 5+6�ָ��� ����2�ֽ�Ƶ�� ���Գ��ȴ���13
                if( (0 == compare_string(td,recv_td,5)) && (TRUE == check_is_all_ch(data+3+record_len-6,6,0xAA)) 
                   && (record_len >= 13) )
                //if(1)  
                {
                    tmp_len = record_len-13;// ʵ�ʵļ�¼����
                    mem_cpy(data,data+8,tmp_len);
                    if( tmp_len < phy->block_len )
                    {
                        mem_set(data+tmp_len,phy->block_len-tmp_len,0xEE);
                    }
                    
                    return phy->block_len;
                }
                else// ʱ�겻�ԣ������̶�EE
                {
                    read_params->delay_read_flag = 0xAA;
                    mem_set(data,phy->block_len,0xEE); 
                    return phy->block_len;
                }
            }
            else //�������� �ظ�EE
            {
                read_params->delay_read_flag = 0xAA;
                mem_set(data,phy->block_len,0xEE); 
                return phy->block_len;
            }
            break;
        case FHJL2_YG_WG_GL:
            // for test
            //mem_cpy(data,data+6,datalen-6);
            //datalen -= 6;
            
            if(datalen >= 8 )// A0 A0 +1�ֽ���+5 ���ɼ�¼ʱ��
            {
                buffer_bcd_to_bin(data+3,recv_td,5);
                record_len = data[2];/* ���ɼ�¼�ֽ��� 5+6�ָ��� ���Գ��ȴ���11 */
                data_pos = 8+1;//ʵ�����ݵĴ洢λ��
                check_AA_pos = 5;
                if( (0 == compare_string(td,recv_td,5)) && (TRUE == check_is_all_ch(data+3+record_len-check_AA_pos,check_AA_pos,0xAA)) 
                   && (record_len >= 11) )
                //if(1)
                {
                    tmp_len = record_len-11;// ʵ�ʵļ�¼����
                    mem_cpy(data,data+data_pos,tmp_len);
                    if( tmp_len < phy->block_len )
                    {
                        mem_set(data+tmp_len,phy->block_len-tmp_len,0xEE);
                    }
                    
                    return phy->block_len;
                }
                else// ʱ�겻�ԣ������̶�EE
                {
                    read_params->delay_read_flag = 0xAA;
                    mem_set(data,phy->block_len,0xEE); 
                    return phy->block_len;
                }
            }
            else //�������� �ظ�EE
            {
                read_params->delay_read_flag = 0xAA;
                mem_set(data,phy->block_len,0xEE); 
                return phy->block_len;
            }
            break;
        case FHJL3_GL_YS:
            // for test
            //mem_cpy(data,data+6,datalen-6);
            //datalen -= 6;
            
            if(datalen >= 8 )// A0 A0 +1�ֽ���+5 ���ɼ�¼ʱ��
            {
                buffer_bcd_to_bin(data+3,recv_td,5);
                record_len = data[2];/* ���ɼ�¼�ֽ��� 5+6�ָ��� ���Գ��ȴ���11 */
                data_pos = 8+2;//ʵ�����ݵĴ洢λ��
                check_AA_pos = 4;
                if( (0 == compare_string(td,recv_td,5)) && (TRUE == check_is_all_ch(data+3+record_len-check_AA_pos,check_AA_pos,0xAA)) 
                   && (record_len >= 11) )
                //if(1)
                {
                    tmp_len = record_len-11;// ʵ�ʵļ�¼����
                    mem_cpy(data,data+data_pos,tmp_len);
                    if( tmp_len < phy->block_len )
                    {
                        mem_set(data+tmp_len,phy->block_len-tmp_len,0xEE);
                    }
                    
                    return phy->block_len;
                }
                else// ʱ�겻�ԣ������̶�EE
                {
                    read_params->delay_read_flag = 0xAA;
                    mem_set(data,phy->block_len,0xEE); 
                    return phy->block_len;
                }
            }
            else //�������� �ظ�EE
            {
                read_params->delay_read_flag = 0xAA;
                mem_set(data,phy->block_len,0xEE); 
                return phy->block_len;
            }
            break;
        case FHJL4_ZFXYG:
            //mem_cpy(data,data+6,datalen-6);// test
            //datalen -= 6;//test
            if(datalen >= 8 )// A0 A0 +1�ֽ���+5 ���ɼ�¼ʱ��
            {
                buffer_bcd_to_bin(data+3,recv_td,5);
                record_len = data[2];/* ���ɼ�¼�ֽ��� 5+6�ָ��� ���Գ��ȴ���11 */
                data_pos = 8+3;//ʵ�����ݵĴ洢λ��
                check_AA_pos = 3;
                if( (0 == compare_string(td,recv_td,5)) && (TRUE == check_is_all_ch(data+3+record_len-check_AA_pos,check_AA_pos,0xAA)) 
                   && (record_len >= 11) )
                //if(1)
                {
                    tmp_len = record_len-11;// ʵ�ʵļ�¼����
                    mem_cpy(data,data+data_pos,tmp_len);
                    if( tmp_len < phy->block_len )
                    {
                        mem_set(data+tmp_len,phy->block_len-tmp_len,0xEE);
                    }
                    
                    return phy->block_len;
                }
                else// ʱ�겻�ԣ������̶�EE
                {
                    read_params->delay_read_flag = 0xAA;
                    mem_set(data,phy->block_len,0xEE); 
                    return phy->block_len;
                }
            }
            else //�������� �ظ�EE
            {
                read_params->delay_read_flag = 0xAA;
                mem_set(data,phy->block_len,0xEE); 
                return phy->block_len;
            }
            break;
        case FHJL5_SXX_WG:
            // for test
            //mem_cpy(data,data+6,datalen-6);
            //datalen -= 6;
            
            if(datalen >= 8 )// A0 A0 +1�ֽ���+5 ���ɼ�¼ʱ��
            {
                buffer_bcd_to_bin(data+3,recv_td,5);
                record_len = data[2];/* ���ɼ�¼�ֽ��� 5+6�ָ��� ���Գ��ȴ���11 */
                data_pos = 8+4;//ʵ�����ݵĴ洢λ��
                check_AA_pos = 2;
                if( (0 == compare_string(td,recv_td,5)) && (TRUE == check_is_all_ch(data+3+record_len-check_AA_pos,check_AA_pos,0xAA)) 
                   && (record_len >= 11) )
                //if(1)
                {
                    tmp_len = record_len-11;// ʵ�ʵļ�¼����
                    mem_cpy(data,data+data_pos,tmp_len);
                    if( tmp_len < phy->block_len )
                    {
                        mem_set(data+tmp_len,phy->block_len-tmp_len,0xEE);
                    }
                    
                    return phy->block_len;
                }
                else// ʱ�겻�ԣ������̶�EE
                {
                    read_params->delay_read_flag = 0xAA;
                    mem_set(data,phy->block_len,0xEE); 
                    return phy->block_len;
                }
            }
            else //�������� �ظ�EE
            {
                read_params->delay_read_flag = 0xAA;
                mem_set(data,phy->block_len,0xEE); 
                return phy->block_len;
            }
            break;
            
        case FHJL1_DY_14:// ��ѹ
        case (FHJL1_DY_14-0x3F+0):
        case (FHJL1_DY_14-0x3F+1):
        case (FHJL1_DY_14-0x3F+2):
        case FHJL2_DL_14:// ����
        case (FHJL2_DL_14-0x3F+0):
        case (FHJL2_DL_14-0x3F+1):
        case (FHJL2_DL_14-0x3F+2):
        case FHJL3_YGGL_14:// �й����ʿ�
        case (FHJL3_YGGL_14-0x3F+0): // �й�������
        case (FHJL3_YGGL_14-0x3F+1): // �й�����A
        case (FHJL3_YGGL_14-0x3F+2): // �й�����B
        case (FHJL3_YGGL_14-0x3F+3): // �й�����C
        case FHJL4_WGGL_14:// �޹����ʿ�
        case (FHJL4_WGGL_14-0x3F+0): // �޹�������
        case (FHJL4_WGGL_14-0x3F+1): // �޹�����A
        case (FHJL4_WGGL_14-0x3F+2): // �޹�����B
        case (FHJL4_WGGL_14-0x3F+3): // �޹�����C

        case FHJL5_GLYS_14:// ����������
        case (FHJL5_GLYS_14-0x3F+0): // ����������
        case (FHJL5_GLYS_14-0x3F+1): // ��������A
        case (FHJL5_GLYS_14-0x3F+2): // ��������B
        case (FHJL5_GLYS_14-0x3F+3): // ��������C
        
        case FHJL6_YG_WG_ZDN_14: // ���������޹���
        case (FHJL6_YG_WG_ZDN_14-0x3F+0):// �����й�
        case (FHJL6_YG_WG_ZDN_14-0x3F+1):// �����й�
        case (FHJL6_YG_WG_ZDN_14-0x3F+2):// ����޹�1
        case (FHJL6_YG_WG_ZDN_14-0x3F+3):// ����޹�2 

        case FHJL7_SXX_WG_ZDN_14: // �������޹���
        case (FHJL7_SXX_WG_ZDN_14-0x3F+0):// ��һ�����޹��ܵ���
        case (FHJL7_SXX_WG_ZDN_14-0x3F+1):// �ڶ������޹��ܵ���
        case (FHJL7_SXX_WG_ZDN_14-0x3F+2):// ���������޹��ܵ���
        case (FHJL7_SXX_WG_ZDN_14-0x3F+3):// ���������޹��ܵ��� 
            //mem_cpy(data,data+6,datalen-6);//  FOR TEST
            //datalen -= 6;
            if(datalen >=5 )
            {
                buffer_bcd_to_bin(data,recv_td,5);
                //if(1)//
                if(0 == compare_string(td,recv_td,5))
                {
                    tmp_len = datalen-5;
                    mem_cpy(data,data+5,tmp_len);
                    //return tmp_len;
                    if( tmp_len < phy->block_len )
                    {
                        mem_set(data+tmp_len,phy->block_len-tmp_len,0xEE);
                    }
                    
                    return phy->block_len;
                }
                else/* ʱ�겻�ԣ������̶�EE ������ֻ����һ���� ����ʱ�겻�� ֱ�����EE */
                {
                    read_params->delay_read_flag = 0xAA;
                    mem_set(data,phy->block_len,0xEE);
                    return phy->block_len;
                }
            }
            else// ����ʱ�꣬�����̶���EE 
            {
                read_params->delay_read_flag = 0xAA;
                mem_set(data,phy->block_len,0xEE); 
                return phy->block_len;
            }
            break;
        
    }
    
}
#endif

INT8U writedata_curve(READ_PARAMS *read_params,READ_WRITE_DATA *phy,INT8U td[5],INT8U *data,INT8U datalen,INT8U *buffer,INT8U curve_save_flag)
{
    INT32U offset;
    INT16U meter_idx;
    INT8U midu = 0;
    #ifdef __MEXICO_RAIL__    
    READ_WRITE_DATA mxc_phy;
    INT32U mxc_offset=0;     
    BOOLEAN need_save_mxc=FALSE;
    INT8U tmp_datetime[6]={0};
    #endif
    #ifdef __HIGH_PRECISION_DATA__
    INT8U real_datalen;
    real_datalen = datalen;
    #endif

    read_params->delay_read_flag = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    //��ȡ�ܶ�
    fread_array(meter_idx,phy->offset,&midu,1);
    #ifdef __MEXICO_GUIDE_RAIL__
    if(midu != 5)
    {
        midu = 5;
        fwrite_array(meter_idx,phy->offset,&midu,1);
    }
    #endif
    offset = get_curve_save_offset(phy,td,midu);
    #ifdef __MEXICO_RAIL__
    mem_cpy(tmp_datetime, g_DST_params.end_time, 6);
    tmp_datetime[MONTH] &= 0x1F;
    buffer_bcd_to_bin(tmp_datetime, tmp_datetime, 6);
    if((0xAA == g_DST_params.time_state)&&(diff_min_between_dt(tmp_datetime, datetime)<=60)&&(compare_string_reverse(datetime, tmp_datetime, 6)<=0))
    {
        switch(phy->phy)
        {
            case 0x00001E80:
                mxc_phy.phy=0x0000DB40;//�й���������
                need_save_mxc=TRUE;
                break;
            case 0x00001EBF:
                mxc_phy.phy=0x0000DB7F;//�й���������
                need_save_mxc=TRUE;
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,READ_MASK_CURVE_YGGL_Z);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,READ_MASK_CURVE_YGGL_A);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,READ_MASK_CURVE_YGGL_B);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,READ_MASK_CURVE_YGGL_C);
                break;
            case 0x00001EC0:
                mxc_phy.phy=0x0000DB80;//�޹���������
                need_save_mxc=TRUE;
                break;
            case 0x00001EFF:
                mxc_phy.phy=0x0000DBBF;//�޹���������
                need_save_mxc=TRUE;
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,READ_MASK_CURVE_WGGL_Z);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,READ_MASK_CURVE_WGGL_A);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,READ_MASK_CURVE_WGGL_B);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,READ_MASK_CURVE_WGGL_C);
                break;
            case 0x00001E00:
                mxc_phy.phy=0x0000DBC0;//��ѹ����
                need_save_mxc=TRUE;
                break;
            case 0x00001E3F:
                mxc_phy.phy=0x0000DBFF;//��ѹ����
                need_save_mxc=TRUE;
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_V_A);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_V_B);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_V_C);
                break;
            case 0x00001E40:
                mxc_phy.phy=0x0000DC00;//��������
                need_save_mxc=TRUE;
                break;
            case 0x00001E7F:
                mxc_phy.phy=0x0000DC3F;//��������
                need_save_mxc=TRUE;
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_I_A);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_I_A);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_I_A);
                break;
            case 0x000021C0:
                mxc_phy.phy=0x0000DC40;//�����������
                need_save_mxc=TRUE;
                break;
            case 0x00000040:
                mxc_phy.phy=0x0000DCC0;//�����й�������
                need_save_mxc=TRUE;
                break;
            case 0x00000080:
                mxc_phy.phy=0x0000DD00;//�����й�������
                need_save_mxc=TRUE;
                break;
            case 0x00001F40:
                mxc_phy.phy=0x0000DC80;//������������
                need_save_mxc=TRUE;
                break;
            case 0x00001F7F:
                mxc_phy.phy=0x0000DCBF;//������������
                need_save_mxc=TRUE;
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_GLYS_Z);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_GLYS_A);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_GLYS_B);
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE, READ_MASK_CURVE_GLYS_C);
                break;
            case 0x00000140:
                mxc_phy.phy=0x0000DD40;//��һ�����޹�����
                need_save_mxc=TRUE;
                break;
            default:
                break;
        }
        if(need_save_mxc==TRUE)
        {
            curve_save_flag = 0xAA;//������������
            if(0xFF != get_phy_form_list_cruve(mxc_phy.phy, &mxc_phy))
            {
                //��ȡ�ܶ�
                fread_array(meter_idx, mxc_phy.offset, &midu, 1);
                #ifdef __MEXICO_GUIDE_RAIL__
                if(midu != 5)
                {
                    midu = 5;
                    fwrite_array(meter_idx,phy->offset,&midu,1);
                }
                #endif
                mxc_offset = get_curve_save_offset(&mxc_phy,td,midu);
                fwrite_array(meter_idx,mxc_phy.offset,&midu,1);
            }
        }
    }
    if(need_save_mxc==TRUE)
         fread_array(meter_idx,mxc_offset,buffer,mxc_phy.data_len);
    else
    #endif
    fread_array(meter_idx,offset,buffer,phy->data_len);

    //����ʱ��
    if(compare_string(buffer,td,5) != 0)
    {
        mem_set(buffer,phy->data_len,0xFF);
        mem_cpy(buffer,td,5);
    }
    else
    {
      if(curve_save_flag == 0x55)
      {
        if((check_is_all_ch(buffer+5+phy->block_offset,phy->block_len,0xFF)) == FALSE)
        return TRUE;
      }

    }


    if(read_params->item_format)
    {
        if((read_params->meter_doc.protocol == GB645_1997) || (read_params->meter_doc.protocol == GUANGXI_V30))
        {
            mem_cpy(buffer+5+phy->block_offset,data,datalen);
            
        }
        else
        {
            //��������
            datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
            mem_cpy(buffer+5+phy->block_offset,data,datalen);

            #ifdef __HIGH_PRECISION_DATA__
            if((phy->phy >= (ZXYG_DN_SJK-0x3F)) && (phy->phy <= D4XX_WG_DN_SJK) && (read_params->meter_doc.protocol == METER_EDMI))
            {
                if((datalen == 4) && (real_datalen ==5))
                {
                    mem_cpy(buffer+5+phy->block_offset,data+1,datalen);
                }
            }
            #endif
        }
    }
    else
    {
        //��������
        #if (defined __HOUR_CURVE_READ_SELF_ADAPTION__)
        if( TRUE == check_load_record_phy(phy->phy) )// ���ɼ�¼�����ʱ�� ��Ҫ���⴦��
        {
            //
            datalen = extract_load_record_data(read_params,phy,td,data,datalen);
        }
        else
        #endif
        {
            datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
        }

        mem_cpy(buffer+5+phy->block_offset,data,datalen);
        
        #ifdef __HIGH_PRECISION_DATA__
        if((phy->phy >= (ZXYG_DN_SJK-0x3F)) && (phy->phy <= D4XX_WG_DN_SJK) && (read_params->meter_doc.protocol == METER_EDMI))
        {
            if((datalen == 4) && (real_datalen ==5))
            {
                mem_cpy(buffer+5+phy->block_offset,data+1,datalen);
            }
        }
        #endif
        
        if(phy->phy ==NBQ_SJ)//��������ݲ������Ȳ�FF
        mem_set(buffer+5+phy->block_offset+datalen,phy->block_len - datalen,0xFF);
    }
    if(0xAA != read_params->delay_read_flag)
    {
        #ifdef __MEXICO_RAIL__
        if(need_save_mxc==TRUE)
        {
            fwrite_array(meter_idx, mxc_offset, buffer, mxc_phy.data_len);
        }
        else
        #endif
        fwrite_array(meter_idx,offset,buffer,phy->data_len);
    }
    #ifdef __SHANDONG_VOLTAGE_PRO__
    INT8U check_num,i,is_valid,tmp_voltage[4],td_tmp[5];
    if(((phy->phy == 0x00001E3F)|| (phy->phy == 0x00001E00)) )
    {
      if((datetime[HOUR] < 0x08) || (datetime[HOUR] >= 0x14))
      {
          check_num = (datetime[HOUR]);
          mem_cpy(td_tmp,td,5);
          for(i = 0;i< check_num;i++)
          {
           td_tmp[1] = i;

           offset = get_curve_save_offset(phy,td_tmp,midu);

           fread_array(meter_idx,offset,buffer,phy->data_len);
               //����ʱ��
           if(compare_string(buffer,td_tmp,5) != 0)
           {
              mem_set(buffer,phy->data_len,0xFF);
              mem_cpy(buffer,td_tmp,5);

              tmp_voltage[0] = data[0];
              tmp_voltage[1] = data[1];

              if( td_tmp[1] % 2 == 0) //ż��ʱ��+������ʱ��-
              {
                tmp_voltage[0] += ((datetime[MSECOND_H] + td_tmp[1])%5) ;
               if((tmp_voltage[0] & 0x0F) >9)
                tmp_voltage[0] &= 0xF0;
               if((tmp_voltage[0] & 0xF0) >0x90)
                 tmp_voltage[0] &= 0x6F;
              }
              else
              {

              tmp_voltage[0] -= ((datetime[MSECOND_L] + td_tmp[1]+data[1])%3);
                if((tmp_voltage[0] & 0x0F) >9)
                tmp_voltage[0] &= 0xF0;
               if((tmp_voltage[0] & 0xF0) >0x90)
                 tmp_voltage[0] &= 0x0F;
              }


             mem_cpy(buffer+5,tmp_voltage,2);

            fwrite_array(meter_idx,offset,buffer,phy->data_len);

           }

          }
      }

        mem_cpy(buffer,datetime+3,3);
        buffer[3] = 0xAA;
        offset = getPassedDays(2000+buffer[2],buffer[1],buffer[0]);
        offset = offset % SAVE_POINT_NUMBER_DAY_HOLD;
        offset *= 4;
        offset += PIM_DAY_HOLD_VIP_VOLTAGE_READ;
        fwrite_array(meter_idx,offset,buffer,4);
    }
    #endif
    #ifdef __HIGH_PRECISION_DATA__
    writedata_precision_cycle_day_and_curve(meter_idx,phy,td,data,real_datalen,buffer,0,0xAA);
    #endif
    return TRUE;
}
#ifdef __MEXICO_GUIDE_RAIL__
#define __NEW_07000000_FORMAT__ /* �¸�ʽ�� 07000000 */
#ifdef __NEW_07000000_FORMAT__
#define CURVE_DATA_LEN_WITHOUT_MSG   31 /* 2 3 2 3 3 4 4 4 6 31 */
#else
#define CURVE_DATA_LEN_WITHOUT_MSG  18 /* 2 3 3 4 6 */
#endif
INT8U write_multi_curve(READ_PARAMS *read_params,INT8U td[5],INT8U *data,INT8U datalen,INT8U *buffer,INT8U curve_save_flag)
{
    INT32U offset;
    #ifndef __NEW_07000000_FORMAT__
    /* ֮ǰ���� ��ѹ2 ����3  �й����� 3  �����й��ܵ��� 4 */
    INT32U phy_32u[4] = {DY_SJK-0x3F,DL_SJK-0x3F,SS_YGGL_SJK-0x3F,ZXYG_DN_SJK-0x3F};
    #else
    /* ���ڶ��� 
     * ��ѹ              2
     * ����              3
     * ��������          2
     * �й�����          3
     * �޹�����          3
     * �����й��ܵ���    4
     * �����й��ܵ���    4
     * ��һ�����޹�����  4
     * total = 25  ������6�ֽڵ�ʱ�� + �䳤message 
     */
    INT32U phy_32u[8] = {DY_SJK-0x3F,      DL_SJK-0x3F,      GLYS_SJK-0x3F,    SS_YGGL_SJK-0x3F,
                         SS_WGGL_SJK-0x3F, ZXYG_DN_SJK-0x3F, FXYG_DN_SJK-0x3F, D1XX_WG_DN_SJK-0x3F };
    INT32U phy_mxc_32u[8] = {MEXICO_DAYLIGHT_SAVING_CURVE_DY-0x3F,      MEXICO_DAYLIGHT_SAVING_CURVE_DL-0x3F,      MEXICO_DAYLIGHT_SAVING_CURVE_GLYS-0x3F,    MEXICO_DAYLIGHT_SAVING_CURVE_YGGL-0x3F,
                         MEXICO_DAYLIGHT_SAVING_CURVE_WGGL-0x3F, MEXICO_DAYLIGHT_SAVING_CURVE_ZXYGZ, MEXICO_DAYLIGHT_SAVING_CURVE_FXYGZ, MEXICO_DAYLIGHT_SAVING_CURVE_WG1 };
    #endif
    INT16U meter_idx;
    INT8U midu = 0;

    READ_WRITE_DATA phy;
    INT8U idx;
    INT8U idx_phy;
    INT8U data_pos = 0;
    INT8U tmp_len = 0;
    INT8U is_in_reappear_time_seg_flag = 0;

    is_in_reappear_time_seg_flag = is_in_reappear_time_seg();
    /* ���ܴ�msg or not  datalen �����ж�  TODO:  */
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    for(idx_phy=0;idx_phy <(sizeof(phy_32u)/sizeof(INT32U));idx_phy++)
    {
        midu = 0;                         
        idx = get_phy_form_list_cruve(phy_32u[idx_phy],&phy);
        //��ȡ�ܶ�
        fread_array(meter_idx,phy.offset,&midu,1);
        
        if(is_in_reappear_time_seg_flag)
        {
            idx = get_phy_form_list_cruve(phy_mxc_32u[idx_phy],&phy);      
        }
        
        offset = get_curve_save_offset(&phy,td,midu);
    
        fread_array(meter_idx,offset,buffer,phy.data_len);
    
        //����ʱ��
        if(compare_string(buffer,td,5) != 0)
        {
            mem_set(buffer,phy.data_len,0xFF);
            mem_cpy(buffer,td,5);
        }
    
        /* �������� */
        tmp_len = (datalen > phy.block_len) ? phy.block_len : datalen;
        mem_cpy(buffer+5+phy.block_offset,data+data_pos,tmp_len);
        
        if(0xAA != read_params->delay_read_flag)
        {
            fwrite_array(meter_idx,offset,buffer,phy.data_len);
        }

        datalen -= phy.block_len;/* ��ȥ���ݳ��� */
        data_pos += phy.block_len;/* ����λ�� */ 
        
    }
    return TRUE;
}
#endif
INT8U readdata_curve(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[5],INT8U *data,INT8U *datalen)
{
    INT32U offset;
    INT8U midu = 0;
    METER_DOCUMENT meter_doc;

    //��ȡ�ܶ�
    fread_array(meter_idx,phy->offset,&midu,1);
    //�Ϻ�����ִֻ�в����������������ǻ᳭����
    //#if ( (defined __PLC_PATCH_96_CURVE__) && !(defined __PROVICE_SHANGHAI__) )
    // ����ز����ܶ�ֵ��������ʼ����ʱ��ᱻ��������Դ���ֵ��ȡʱ����ܶȲ���15 д��15
    fread_meter_params(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
    if( (meter_doc.baud_port.port == COMMPORT_PLC) || (meter_doc.baud_port.port == COMMPORT_PLC_REC) )
    {
        /*�����ز�����  */
        if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD) )
        {
            #if 0 //ȥ���ˣ���һ���ǵ��첹�����ж�24�㻹��96�㣬
            /* 96�� ��һ�� ��ʱֻ֧�� 96�� */
            if(0 == check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PATCH_TODAY_LOAD_RECORD))
            {
                if(15 != midu)
                {
                    midu = 15;
                    fwrite_array(meter_idx,phy->offset,&midu,1);
                }
            }
            else/* �������� ����24�� 96�� ������ */
            #endif
            {
                /* 24�� */
                if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_24_DOT))
                {
                    if(60 != midu)
                    {
                        midu = 60;
                        fwrite_array(meter_idx,phy->offset,&midu,1);
                    }
                }
                else /* 96�� */
                {
                    if(15 != midu)
                    {
                        midu = 15;
                        fwrite_array(meter_idx,phy->offset,&midu,1);
                    }
                }
            }
        }       
    }
    
    #ifdef __MEXICO_GUIDE_RAIL__
    if(midu != 5)
    {
        midu = 5;
        fwrite_array(meter_idx,phy->offset,&midu,1);
    }
    #endif

    offset = get_curve_save_offset(phy,td,midu);

    fread_array(meter_idx,offset,data,phy->data_len);
    if (compare_string(td,data,5) == 0)
    {
        if(check_is_all_ch(data+5+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+5+phy->block_offset,phy->block_len);
            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

INT16S app_readdata_curve(INT16U meter_idx,INT32U phy_id,INT8U td[5],INT8U *data,INT8U max_datalen)
{
    READ_WRITE_DATA phy;
    INT8U idx;
	INT8U datalen = 0;
    //INT8U midu;
    INT8U td_bin[5];

    idx = get_phy_form_list_cruve(phy_id,&phy);
    if (idx == 0xFF) return -1;
    if (phy.data_len > max_datalen) return -1;

    td_bin[0] = BCD2byte(td[0]);
    td_bin[1] = BCD2byte(td[1]);
    td_bin[2] = BCD2byte(td[2]);
    td_bin[3] = BCD2byte(td[3]);
    td_bin[4] = BCD2byte(td[4]);
    readdata_curve(meter_idx,&phy,td_bin,data,&datalen);
    return datalen;
}

INT8U save_curve_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer,INT8U curve_save_flag)
{
    READ_WRITE_DATA phy;
    INT8U* td;
    #ifdef __MEXICO_CIU__
    INT16U meter_idx = 0;
    INT8U  msg[100] = {0}; /* ��ʽ ��־ 1+ ����1 + ���� */
    INT8U len = 0;
    #endif
    INT8U idx,midu;
    #ifdef __COUNTRY_ISRAEL__
    INT8U idx_temp;
    #endif

    midu = 0;
    idx = get_phy_form_list_cruve(bin2_int32u(read_params->phy),&phy);
    if(idx == 0xFF) return 0;
    fread_array(bin2_int16u(read_params->meter_doc.meter_idx),phy.offset,&midu,1);
    #ifdef __MEXICO_GUIDE_RAIL__
    if(midu != 5)
    {
        midu = 5;
    }
    #endif
    #ifdef __JIANGSU_READ_CURRENT_MONITOR__
    if((read_params->meter_doc.protocol == BREAKER_MONITOR) && (phy.phy == (SYDL_SJK)))
    {
        midu = 5;
        fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),phy.offset,&midu,1);
    }
    #endif
    switch(midu)
    {
    case 5:  td = read_meter_flag_curve.cycle_05_minute; break;
    case 15: td = read_meter_flag_curve.cycle_15_minute; break;
    case 30: td = read_meter_flag_curve.cycle_30_minute; break;
    case 60: td = read_meter_flag_curve.cycle_60_minute; break;
    default: clr_bit_value(read_params->read_mask.curve,SAVE_POINT_NUMBER_CURVE,idx); return 0;
    }

    if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
    {
        clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,idx);
        //����msak
        clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_CURVE,idx);
    }
    else
    {
        check_frame_blockFF_convert_EE(&phy,frame,frame_len);
        #ifdef __MEXICO_GUIDE_RAIL__
        if( (COMMPORT_PLC == read_params->meter_doc.baud_port.port) && ((DY_SJK-0x3F) == phy.phy) )
        {
            INT32U item32 = 0x07000000;
            INT8U idx_frame = 0;
            write_multi_curve(read_params,td,frame,frame_len,buffer,curve_save_flag);

            #ifdef __MEXICO_CIU__ /*����CIU  �ŷ��͸�CIU ���� */

            /* 180716 ��� ��������Ϣ  2 3 3 4 6 = 18 */
            if(frame_len <= CURVE_DATA_LEN_WITHOUT_MSG) // 18
            {
                /* ��ȡ��Ϣ�洢���� ȡ��δ���ͳɹ������� Ȼ���͸�CIU */
                meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
                fread_array(meter_idx,PIM_MEXICO_RAIL_MESSAGE,msg,100);
                if( (0xAA == msg[0]) && (msg[1] <= 50) )
                {
                    //for(idx_frame=0;idx_frame<msg[1];idx_frame++)
                    {
                        //msg[2] += 0x33;
                    }
                    mem_cpy(frame+frame_len,msg+2,msg[1]);
                    frame_len += msg[1];
                }
            }
            else
            {
                msg[0] = 0xAA;
                msg[1] = frame_len - CURVE_DATA_LEN_WITHOUT_MSG; // 18 
                mem_cpy(msg+2,frame+CURVE_DATA_LEN_WITHOUT_MSG,msg[1]); // 18 
                meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
                fwrite_array(meter_idx,PIM_MEXICO_RAIL_MESSAGE,msg,100);
            }
            /* copy ���� Ȼ��ֱ�ӷ��͸�CIU ʱ����Ϣ����Ϣ������CIU �������Ϊ 11 */
            if(frame_len >100) /* 12 */
            {
                frame_len = 100;
            }           
            /* ����4�� ��д item */
            mem_cpy_right(frame+4,frame,frame_len);
            mem_cpy(frame,(void *)&item32,4);
            frame_len += 4;

            #if 0
            frame[frame_len] = 0x00;/* �� */
            mem_cpy(frame+frame_len+1,td,5);/* ������ʱ�� */
            frame_len += 6;/* ��������  */
            #endif
            
            /* ��33 ���� */
            for(idx_frame=0;idx_frame< frame_len;idx_frame++)
            {
                frame[idx_frame] += 0x33;
            }
            /* ��֡���͸�CIU ctrl�����⣬TODO::???? 
             * 18-09-27 �洢���ģ����͵�ʱ����ȡ���� 
             */
            //portContext_plc.snd_ciu_framelen = make_write_ciu_frame(portContext_plc.snd_ciu_frame,read_params->meter_doc.meter_no,frame,frame_len);
            len = make_write_ciu_frame(msg,read_params->meter_doc.meter_no,frame,frame_len);
            meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
            fwrite_array(meter_idx,PIM_MEXICO_SND_CIU_FRAME,msg,len);
            
            /* fram  ��ʱע�͵� ��֧�� ���͸�CIU  */
            read_params->snd_rail_2_ciu = 0xAA;/* ��Ҫ���͸�CIU�������� */
            #endif
        }
        else
        #endif
        #ifdef __COUNTRY_ISRAEL__
        if ((bin2_int32u(read_params->phy) == ISRAEL_HOUR_DATA) )
        {
          if(frame_len < 30) //���Ȳ�������Ϊ�Ǵ������ݣ����շ��ϴ���
          {
            mem_set(frame+2,frame_len,0xEE);
          }
          else
          {
           if(frame_len == 37) //����ǵ������ѹ�͵�������ת�������򳤶Ȳ�һ��
           {
           single_meter_tran_three_meter(frame+2);
           frame_len = 45;
           }
           check_erc_37(bin2_int16u(read_params->meter_doc.spot_idx),frame+2);
          }
         writedata_curve(read_params,&phy,td,frame+2,frame_len,buffer,0);

        }
        else if(bin2_int32u(read_params->phy) == SOUTH_AFRICA_HOUR)
        {
          if(frame_len < 30)
          {
            mem_set(frame,frame_len,0xEE);
          }
          else
          {
            if(frame_len == 35) //����ǵ������ѹ�͵�������ת�������򳤶Ȳ�һ��
           {
            single_meter_tran_three_meter(frame);
            frame_len = 45;
           }
           check_erc_37(bin2_int16u(read_params->meter_doc.spot_idx),frame);
          }
          writedata_curve(read_params,&phy,td,frame,frame_len,buffer,0);
        }
        else if(bin2_int32u(read_params->phy)==ISRAEL_LOAD_PROFILE)//loadprofile
        {
           if(frame[3] == 0)
           single_meter_loadprofile_tran_three_meter(frame);

           frame_len = 91;

             buffer_bcd_to_bin(frame+5,frame+5,5);
             if(compare_string(frame+5,td,5) != 0)
             {
                mem_set(frame+5,frame_len,0xFF);
                mem_cpy(frame,td,5);
             }
             else
             {
               frame[6] = frame[0];
               frame[7] = frame[1];
               frame[8] = frame[2];
               frame[9] = frame[3];
             }
             writedata_curve(read_params,&phy,td,frame+6,frame_len,buffer,0);
        }
        else
        #endif
        {
            writedata_curve(read_params,&phy,td,frame,frame_len,buffer,curve_save_flag);
        }
        #ifdef __COUNTRY_ISRAEL__
        if ((bin2_int32u(read_params->phy) == ISRAEL_HOUR_DATA) && (datetime[HOUR] == 0))
        {
             #ifdef __SOFT_SIMULATOR__
              snprintf(info,100,"*** curve_to_dayhold : meter_idx = %d ,  phy = 0x%08X , mask_idx = %03d",
                                    bin2_int16u(read_params->meter_doc.meter_idx),phy.phy,idx);
                            debug_println_ext(info);
                            #endif

           idx_temp = get_phy_form_list_cycle_day(ISRAEL_HOUR_DATA,&phy);
           if(idx_temp != 0xFF)
           {
           
              writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,td,frame,frame_len,buffer,NULL,0xFF);
           }
        }
        #endif
        clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,idx);
    }
    #ifdef __HUBEI_STEP_CONTROL__
    calculate_line_lost(&(read_params->meter_doc),&phy,frame,frame_len,READ_TYPE_CURVE);
    #endif
    return 0;
}

BOOLEAN prepare_read_item_curve(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT32U last_read_phy_offset = 0;
    INT32U last_read_phy_block_offset;

    INT8U* td;
    INT16U meter_idx;
	#ifdef __POWER_CTRL__
	INT16U spot_idx;
	#endif
    INT8U idx,idx_sub,mask_idx;
    INT8U midu,is_plus;
    #if (defined __HOUR_CURVE_READ_SELF_ADAPTION__)
    INT8U load_td[6] = {0};
    INT8U idx_td = 0;
    #endif
    BOOLEAN read_curve_flag = FALSE;
    INT8U need_read_item_no =0;
    INT8U gb_645_zuhe_frame[200] = {0};
    INT8U zuhe_frame_len = 0;
    INT8U parall_mode = 0;
    #ifdef __READ_OOP_METER__
    INT8U oad_byte[4];
    INT8U oad_count=0;
    #endif
    #ifdef __MEXICO_CIU__
    INT8U ciu_frame[100] = {0};
    INT8U ciu_frm_len = 0;
    #endif
    #ifdef __COUNTRY_ISRAEL__
    INT8U load_time[10];
    #endif
    INT8U need_login_flag = 0;

    midu = 0;
    is_plus = 0;
    mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    #ifdef __MEXICO_CIU__
    if(0xAA == read_params->snd_rail_2_ciu)
    {
        //mem_cpy(frame,portContext_plc.snd_ciu_frame,portContext_plc.snd_ciu_framelen);
        //*frame_len = portContext_plc.snd_ciu_framelen;
        /*
         * 18-09-27 ��ȡ�洢������
         */
        fread_array(meter_idx,PIM_MEXICO_SND_CIU_FRAME,ciu_frame,100);
        ciu_frm_len = ciu_frame[POS_GB645_DLEN] + 12;
        if( (ciu_frm_len <= 12) || (ciu_frm_len > 100)
          || (FALSE == check_frame_body_gb645(ciu_frame,ciu_frm_len)) )
        {
            return FALSE;
        }
        
        mem_cpy(frame,ciu_frame,ciu_frm_len);
        *frame_len = ciu_frm_len;
        return TRUE;
    }
    if(check_DST_delay() == FALSE)
    {
        return FALSE;
    }
    #endif
    if (get_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;
    if (check_is_all_ch(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,0x00))
    {
        #ifdef __READ_EDMI_METER__
        if(read_params->meter_doc.protocol == METER_EDMI)
        {
            if(TRUE == edmi_to_exit_state(read_params))
            {
                *frame_len = edmi_read_ctrl(read_params,NULL,0,frame,0,NULL);
                return TRUE;
            }
            read_params->read_ctrl_state = 0;
            read_params->read_ctrl_step = 0;
        }
        #endif   
        #ifdef __READ_IEC1107_METER__
        if(read_params->meter_doc.protocol == IEC1107)
        {
            if(TRUE == iec1107_to_exit_state(read_params))
            {
                *frame_len = iec1107_read_ctrl(read_params,frame,0,NULL);
                if(*frame_len > 0)
                    return TRUE;
            }
        }
        #endif
        clr_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }
    
    #ifndef __IEC1107_READ_LOAD__
    if(read_params->meter_doc.protocol == IEC1107)
    {
        clr_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;        
    }
    #endif
    /*�ز������������� ���� �������� �������������� �����������������*/
    #ifndef __PROVICE_SHAANXI_CHECK__/*��������ʱ�򣬿���������ݶ�Ҫ�󳭶����ɼ�¼�����ҵ��첻������������*/
    #ifndef __PROVICE_SICHUAN__
    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {                
        if( 0 == check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD) )
        {
            read_curve_flag = TRUE;
        }
        if( check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD) 
         && check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PATCH_TODAY_LOAD_RECORD)  )
        {
            read_curve_flag = TRUE;
        }
        if(FALSE == read_curve_flag)
        {
            portContext_plc.active_read_curve_one_cycle = 0;
            /*���߲������Ļ� ��Ҫ ���flag ��ֹ�����Ӱ��ͳ�ƻ��߳���*/
            clr_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx);
            return FALSE;
        }
    }
    #endif
    #endif
    
//    #ifdef __READ_IEC1107_METER__
//    if(read_params->meter_doc.protocol == IEC1107)
//    {
//        if(read_params->read_ctrl_state ==0)
//        {
//            *frame_len = 0;
//            if(read_params->read_ctrl_step ==0)
//            {
//                mem_set(read_params->protocolCtrl.value,sizeof(PROTOCOL_CTRL),0);
//                *frame_len = make_iec1107_handshake_step0(frame,read_params->meter_doc.meter_no);
//                return TRUE;
//            }
//            else if(read_params->read_ctrl_step ==1)
//            {
//                *frame_len = make_iec1107_handshake_step1(frame,'0',read_params->rec_ctrl_baudrate,'1');
//                return TRUE;
//            }
//            else //if(read_params->read_ctrl_step ==2)
//            {
//                read_params->read_ctrl_state = 5;
//                read_params->read_ctrl_step = 0;
//                //mem_set(read_params->rec_ctrl_passwrod,8,'0');
//                //*frame_len=make_iec1107_handshake_step2(frame,read_params->rec_ctrl_passwrod);
//            }
//        }
//        else if(read_params->read_ctrl_state == 3)
//        {
//            *frame_len = make_iec1107_break_message(frame);
//                #ifdef __SOFT_SIMULATOR__
//                snprintf(info,100,"make_iec1107_break_message 1");
//                debug_println_ext(info);
//                #endif
//            //read_params->read_ctrl_step++;
//            return TRUE;
//        }
//    }
//    #endif
    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
      if( portContext_plc.plc_other_read_mode == CYCLE_REC_MODE_PARALLEL )
      {
         #ifdef __READ_DLMS_METER__
             parall_mode = 0;
         #else
             parall_mode = 0xAA;
         #endif
      }
    }

    #ifdef __READ_DLMS_METER__
CHECK_LOGIN:
    if(read_params->meter_doc.protocol == METER_DLMS) /*��¼*/
    {
        if(read_params->dlms_login_ok)
        {
        if(read_params->login_try_count > 10)
        {
            read_params->read_ctrl_state = 2;//��½��10�λ�û��½�ɹ����Ͽ����µ�¼
        }

        if(read_params->read_ctrl_state ==0)
        {
            if(read_params->read_ctrl_step==0)
            {
                *frame_len = make_dlms_snrm(frame,read_params->meter_doc.meter_no);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms snrm  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                read_params->login_try_count ++;
            }
            else if(read_params->read_ctrl_step==1)
            {
                *frame_len = make_dlms_aarq(frame,read_params->meter_doc.meter_no,read_params->dlms_SSS,read_params->dlms_RRR);
                read_params->dlms_SSS =1;
                read_params->dlms_RRR =1;
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms aarq  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
               read_params->login_try_count ++;
            }
            else if(read_params->read_ctrl_more_frame)
            {
                *frame_len = make_dlms_rr(frame,read_params->meter_doc.meter_no,read_params->dlms_RRR);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms rr  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                read_params->login_try_count ++;
            }
            return TRUE;
        }
        else if(read_params->read_ctrl_state == 2)
        {
            *frame_len = make_dlms_disc(frame,read_params->meter_doc.meter_no);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"dlms disc");
            debug_println_ext(info);
            #endif
            read_params->login_try_count = 0;
            return TRUE;
        }
       }

    }
    #endif
    #ifdef __POWER_CTRL__
    spot_idx = bin2_int16u(read_params->meter_doc.spot_idx); //������⣬Э������е���������ֱ���жϸò������Ƿ������塣
    is_plus = is_pulse_meter(spot_idx);
    #endif
    for(idx=0;idx<sizeof(CRUVE_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        if(CRUVE_PHY_LIST[idx].offset != last_read_phy_offset)
        {
            last_read_phy_block_offset = 0;  /*���ƫ�ƶ����ˣ�˵����ǰ��Ӧ��������϶�Ҫ�ж��Ƿ񳭶������û�䣬����֮ǰ��ƫ�Ʊȶ�*/
        }

        for(idx_sub=0;idx_sub<((CRUVE_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            if(get_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,mask_idx))
            {
                get_phy_data((READ_WRITE_DATA*)&(CRUVE_PHY_LIST[idx]),idx_sub,&phy);

                #ifdef __READ_OOP_METER__
                if((parall_mode == 0xAA)||(read_params->meter_doc.protocol == GB_OOP))
                #else
                if(parall_mode == 0xAA)
                #endif
                {
                    if(CRUVE_PHY_LIST[idx].offset == last_read_phy_offset)   /*����洢һ��������������������*/
                    {
                        if(CRUVE_PHY_LIST[idx].block_offset == last_read_phy_block_offset) /*�������ݣ�ͨ��offset�жϣ������ѹ�͵����ȵĴ洢һ������������*/
                        {
                            mask_idx++;
                            continue ;
                        }
                        last_read_phy_block_offset = CRUVE_PHY_LIST[idx].block_offset;
                    }
                    last_read_phy_offset = CRUVE_PHY_LIST[idx].offset;

                }

                //��ȡ�ܶ�
                fread_array(meter_idx,phy.offset,&midu,1);
                #if (defined __HOUR_CURVE_READ_SELF_ADAPTION__) //�Զ��л����д�ܶ�15�����ʹ�õ�ʱ��Ҫע�⣬����û��ϸ����֮ǰ������
                if(read_params->meter_doc.baud_port.port == COMMPORT_PLC)
                {
                    if(15 != midu)
                    {
                        midu = 15;/*  15 �ܶȸĳ�15�������� 2018-04-03 */
                        fwrite_array(meter_idx,phy.offset,&midu,1);
                    }
                }
                #endif
                #ifdef __MEXICO_GUIDE_RAIL__
                if(midu != 5)
                {
                    midu = 5;
                    fwrite_array(meter_idx,phy.offset,&midu,1);
                }
                #endif
                switch(midu)
                {
                case 5:  break;
                case 15: break;
                case 30: break;
                case 60: break;
                default:
                    if(read_params->meter_doc.baud_port.port == COMMPORT_PLC)
                    {
                        midu = 60;
                        fwrite_array(meter_idx,phy.offset,&midu,1);
                    }
                    else
                    {
                        midu = 15;
                        fwrite_array(meter_idx,phy.offset,&midu,1);
                    }
                    break;
                }

                switch(midu)
                {
                case 5:  td = read_meter_flag_curve.cycle_05_minute; break;
                case 15: td = read_meter_flag_curve.cycle_15_minute; break;
                case 30: td = read_meter_flag_curve.cycle_30_minute; break;
                case 60: td = read_meter_flag_curve.cycle_60_minute; break;
                default: clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,mask_idx);continue;
                }
                
                #if (defined __HOUR_CURVE_READ_SELF_ADAPTION__)
                if(check_is_all_ch(td,5,0x00))// 5�������ߵ�ʱ�򣬻������������������
                {
                    //clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,mask_idx);
                    //continue;
                    clr_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx);
                    return FALSE;
                }
                #endif
                #ifdef __MEXICO_RAIL__
                if((readdata_curve(meter_idx,&phy,td,frame,frame_len) == FALSE) || is_in_reappear_time_seg())/*������������붬���ظ�ʱ�����������������*/
                #else
                if(readdata_curve(meter_idx,&phy,td,frame,frame_len) == FALSE)
                #endif
                {
                    //��Ҫ����
                    if(get_data_item(phy.phy,read_params->meter_doc.protocol,&library))
                    {
                        if(library.item != 0xFFFFFFFF)
                        {
                            #ifdef __PROVICE_JIANGXI__ /*�����ϱ�F129��ͨ����������������Ԥ�����ݣ�ˢ��4����*/
                             if(library.item  == 0x00010000)
                             {
                                 library.item = 0x0001FF00;
                             }
                             else if(library.item == 0x00020000)
                             {
                                 library.item = 0x0002FF00;
                             }
                            #endif
                            if(parall_mode == 0xAA)
                            {
                                int32u2_bin(phy.phy,read_params->phy+need_read_item_no*4);
                                int32u2_bin(library.item,read_params->item+need_read_item_no*4);
                            }
                            else
                            {
                                int32u2_bin(phy.phy,read_params->phy);
                                int32u2_bin(library.item,read_params->item);
                            }

                            read_params->item_len = library.len;
                            read_params->item_format = library.format;
                            if((CRUVE_PHY_LIST[idx].data_len + 10)<40)
                            {
                                read_params->resp_byte_num = 40;
                            }
                            else if((CRUVE_PHY_LIST[idx].data_len + 10)>200)
                            {
                                read_params->resp_byte_num = 80;
                            }
                            else                         
                            {
                                read_params->resp_byte_num = (CRUVE_PHY_LIST[idx].data_len + 10);
                            }
                            read_params->read_type = READ_TYPE_CURVE;
                            if (read_params->meter_doc.protocol == GB645_2007)
                            {
                                if(TRUE == check_load_record_phy(phy.phy))
                                {
                                    //
                                    #if (defined __HOUR_CURVE_READ_SELF_ADAPTION__)
                                    load_td[0] = 1;//���� 
                                    load_td[1] = byte2BCD(td[0]);// �� 00  byte2BCD(td[0]) 2018-04-03  �ܶ�15
                                    for(idx_td=0;idx_td<4;idx_td++)// ʱ �� �� �� 
                                    {
                                        load_td[2+idx_td] = byte2BCD(td[1+idx_td]);
                                        //load_td[1+idx_td] = byte2BCD(td[idx_td]);
                                    }
                                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_td,6);
                                    #else
                                    // ������� ������ ��ֹ���������쳣
                                    clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,mask_idx);
                                    mask_idx++;
                                    continue;
                                    #endif
                                }
                                else
                                {
                                    #ifdef __MEXICO_GUIDE_RAIL__
                                    INT32U rail_phy = CIU_DBSJ;
                                    if( ((DY_SJK-0x3F) == phy.phy) && (read_params->meter_doc.baud_port.port == COMMPORT_PLC) )
                                    {
                                        rail_phy = CIU_DBSJ;
                                        if ( get_data_item(rail_phy,read_params->meter_doc.protocol,&library)
                                        && (library.item != 0xFFFFFFFF)  )
                                        {
                                            /*������ı� ���������� */
                                            int32u2_bin(library.item,read_params->item);
                                            *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                        }
                                        else
                                        {
                                            /*  */
                                            clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,mask_idx);
                                        }
                                    }
                                    /*����ĳ������ ��CIU �������� */
                                    #if 0
                                    else if( ((DL_SJK-0x3F) == phy.phy) && (read_params->meter_doc.baud_port.port == COMMPORT_PLC) )
                                    {
                                        mem_cpy(frame,portContext_plc.snd_ciu_frame,portContext_plc.snd_ciu_framelen);
                                        *frame_len = portContext_plc.snd_ciu_framelen;
                                    }
                                    #endif
                                    else
                                    #endif
                                    #ifdef __COUNTRY_ISRAEL__
                                    if( ISRAEL_HOUR_DATA == phy.phy) //Сʱ�������ݸ�ʽ
                                    {
                                        load_time[0] = 0;
                                        load_time[1] = (td[1]);//datetime[HOUR];  //����ת��bcd??
                                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,2);
                                    }
                                    else if(phy.phy == ISRAEL_LOAD_PROFILE)
                                    {
                                        load_time[0] = 1;
                                        load_time[1] = byte2BCD(td[0]);
                                        load_time[2] = byte2BCD(td[1]);
                                        load_time[3] = byte2BCD(td[2]);
                                        load_time[4] = byte2BCD(td[3]);
                                        load_time[5] = byte2BCD(td[4]);
                                        set_israel_di_item(load_time+6,phy.phy);
                                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,10);
                                    }
                                    else
                                    #endif
                                    {
                                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                    }
                                }
                            }
                            else if ((read_params->meter_doc.protocol == GB645_1997) || (read_params->meter_doc.protocol == GB645_1997_JINANGSU_4FL) 
                                    || (read_params->meter_doc.protocol == GB645_1997_JINANGSU_2FL) || (is_plus))
                            {
                                *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            else if (read_params->meter_doc.protocol == GUANGXI_V30)
                            {
                                *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #ifdef __READ_EDMI_METER__
                            else if(read_params->meter_doc.protocol == METER_EDMI)
                            {
                                #ifdef __EDMI_READ_LOAD__
                                *frame_len = edmi_read_ctrl(read_params,&(library.item),1,frame,1,td);
                                #else
                                *frame_len = edmi_read_ctrl(read_params,&(library.item),1,frame,0,NULL);
                                #endif
                            }
                            #endif
                            #ifdef __READ_HENGTONG_METER__
                            else if((read_params->meter_doc.protocol == PROTOCOL_HENGTONG_OLD) || (read_params->meter_doc.protocol == PROTOCOL_HENGTONG_NEW))
                            {
                                 //library.len = 4;//������
                                 *frame_len = rs485_hengt_query(read_params->meter_doc.meter_no , library.item, library.len, frame); //�������ʲô��˼��
                            }
                            #endif
                            #ifdef __IEC1107_READ_LOAD__
//                           else if((read_params->meter_doc.meter_class.user_class==7) && (read_params->meter_doc.protocol == IEC1107))
                            else if(read_params->meter_doc.protocol == IEC1107)
                            {
                                *frame_len = iec1107_read_ctrl(read_params,frame,1,td);
                            }
                            #endif
                            #ifdef __READ_OOP_METER__
                            else if(GB_OOP == read_params->meter_doc.protocol)
                            {
                                oad_byte[0] = library.item>>24;
                                oad_byte[1] = library.item>>16;
                                oad_byte[2] = library.item>>8;
                                oad_byte[3] = library.item;

                                if(0 == oad_count)
                                {
                                    read_params->oad_cnt = 0;
                                }
                                if(library.item == 0xFFFFFFFF)
                                {
                                    clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,mask_idx);
                                    mask_idx++;
                                    continue;
                                }
                                mem_cpy(read_params->phy_bak+oad_count*4, (INT8U *)&library.phy, 4);
                                mem_cpy(read_params->oad+oad_count*4, oad_byte, 4);
                                oad_count++;
                                if(oad_count < PARALL_MAX_OAD)
                                {
                                    mask_idx++;
                                    continue;
                                }
                                #ifdef __READ_OOP_METER_CURVE_REAL_TO_HOLD__
                                *frame_len = make_oop_read_frame(frame, read_params->meter_doc.meter_no, read_params->oad, oad_count, 0xFFFF, 1);
                                #else
                                *frame_len = make_oop_read_frame(frame, read_params->meter_doc.meter_no, read_params->oad, oad_count, 0x5002,1);
                                #endif
                                read_params->oad_cnt = oad_count;
                                return TRUE;
                            }
                            #endif

                            #ifdef __FUJIAN_CURRENT_BREAK__
                            else if((read_params->meter_doc.protocol == FUJIAN_BREAKER_MONITOR))
                            {
                                 if(library.item == 0x0202FF00)
                                 {
                                    library.item = 0xEA0202FF;
                                 }
                                 int32u2_bin(library.item,read_params->item);
                                 *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0); //
                            }
                            #endif
                            #ifdef __READ_DLMS_METER__
                            else if((read_params->meter_doc.protocol == METER_DLMS))
                            {

                                 if(read_params->dlms_login_ok == 0)
                                 {
                                    int32u2_bin(0,read_params->phy);
                                    read_params->dlms_login_ok = 1;
                                    goto
                                        CHECK_LOGIN;
                                 }
                                *frame_len = make_dlms_read_energy(frame,read_params->meter_doc.meter_no,(INT8U*)library.item,read_params->dlms_SSS,read_params->dlms_RRR,3);
                                read_params->dlms_SSS ++;
                                read_params->dlms_RRR ++;
                            }
                            #endif
                            #ifdef __READ_IEC1107_METER__
                            else if((read_params->meter_doc.protocol == IEC1107)&& (read_params->read_ctrl_state==5))
                            {
                                mem_cpy(read_params->protocolCtrl.iec1107.td,td,5);
                                if(read_params->read_ctrl_step==0)
                                {
                                    *frame_len = make_iec1107_pro_read(frame,td);
                                }
                                else
                                {
                                    *frame_len = 0;
                                    return FALSE;
                                }
                            }
                            #endif
                            else
                            {
                            clr_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx);
                            return FALSE;
                            }
                          //  *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);

                            #ifdef __SOFT_SIMULATOR__
                            snprintf(info,100,"*** prepare item curve : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
                                    bin2_int16u(read_params->meter_doc.meter_idx),library.item,phy.phy,mask_idx);
                            debug_println_ext(info);
                            #endif
                            read_params->cur_mask_idx = mask_idx;

                            if((parall_mode == 0xAA) &&((read_params->meter_doc.protocol == GB645_2007)
                            || (read_params->meter_doc.protocol == GB645_1997)))
                            {
                                mem_cpy(gb_645_zuhe_frame+zuhe_frame_len,frame, *frame_len );
                                zuhe_frame_len += *frame_len;
                                need_read_item_no ++ ;

                                if(need_read_item_no >= (portContext_plc.parall_max_item)) /*һ����������645֡��*/
                                {
                                   mem_cpy(frame,gb_645_zuhe_frame,zuhe_frame_len);
                                   *frame_len = zuhe_frame_len;
                                   return TRUE;
                                }
                                else
                                {
                                   mask_idx++;
                                   continue;
                                }
                            }
                            else
                            {
                                return TRUE;
                            }
                        }
                    }
                }
                clr_bit_value(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,mask_idx);
            }
            mask_idx++;
        }
    }
    #ifdef __READ_EDMI_METER__
    if(read_params->meter_doc.protocol == METER_EDMI)
    {
        if(TRUE == edmi_to_exit_state(read_params))
        {
            *frame_len = edmi_read_ctrl(read_params,NULL,0,frame,0,NULL);
            return TRUE;
        }
        read_params->read_ctrl_state = 0;
        read_params->read_ctrl_step = 0;
    }
    #endif
    #ifdef __READ_IEC1107_METER__
    if(read_params->meter_doc.protocol == IEC1107)
    {
        if(TRUE == iec1107_to_exit_state(read_params))
        {
            *frame_len = iec1107_read_ctrl(read_params,frame,0,NULL);
            if(*frame_len > 0)
                return TRUE;
        }
    }
    #endif
    #ifdef __READ_OOP_METER__
    if(oad_count>0)
    {
        #ifdef __READ_OOP_METER_CURVE_REAL_TO_HOLD__
        *frame_len = make_oop_read_frame(frame, read_params->meter_doc.meter_no, read_params->oad, oad_count, 0xFFFF, 1);
        #else
        *frame_len = make_oop_read_frame(frame, read_params->meter_doc.meter_no, read_params->oad, oad_count, 0x5002,1);
        #endif
        read_params->oad_cnt = oad_count;
        return TRUE;
    }
    #endif
    if(parall_mode == 0xAA)
    {
        if(need_read_item_no )
        {
           mem_cpy(frame,gb_645_zuhe_frame,zuhe_frame_len);
           *frame_len = zuhe_frame_len;
           return TRUE;
        }
        else
        {
           clr_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx);
        }
    }
    else
    {
       clr_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx);
    }

    return FALSE;
}
#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
INT32U get_curve_save_offset_hunan(READ_WRITE_DATA *phy,INT8U dh_td[3],INT8U td[5])
{
    INT32U 	offset;
    //INT8U 	num;
	INT8U	pos;

	pos = 0xFF;
	/*
	if( (datetime[HOUR]>=7) && (datetime[HOUR]<11) )
		pos = 0;
	else if ( (datetime[HOUR]>=11) && (datetime[HOUR]<15) )
		pos = 1;
	else if ( (datetime[HOUR]>=15) && (datetime[HOUR]<19) )
		pos = 2;
	else if ( (datetime[HOUR]>=19) && (datetime[HOUR]<22) )
		pos = 3;
	*/
	if (td[1] == 7)  
		pos = 0;
	else if (td[1] == 11)
		pos = 1;
	else if (td[1] == 15) 
		pos = 2;
	else if (td[1] == 19)
		pos = 3;
	if(pos == 0xFF)
	{
		offset = 0xFFFFFFFF;
		return offset;
	}
	offset = getPassedDays(2000+dh_td[2],dh_td[1],dh_td[0]);
    offset = offset % SAVE_POINT_NUMBER_CURVE_HUNAN_DAY;
    offset *= phy->data_len*SAVE_POINT_NUMBER_CURVE_HUNAN;//�ĸ�����
    offset += phy->offset;
	offset += phy->data_len*pos;//�ҵ�����λ��
    return offset;
	
}
INT8U get_phy_form_list_cruve_hunan(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(CRUVE_PHY_LIST_HUNAN)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((CRUVE_PHY_LIST_HUNAN[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(CRUVE_PHY_LIST_HUNAN[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}
INT8U writedata_curve_hunan(READ_PARAMS *read_params,READ_WRITE_DATA *phy,INT8U td[5],INT8U *data,INT8U datalen,INT8U *buffer)
{
    INT32U 	offset;
    INT16U 	meter_idx;
    //INT8U 	midu;
	INT8U	dh_td[3] = {0};
    INT8U td_bcd[5] = {0};
    READ_WRITE_DATA phy_other;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    mem_cpy(dh_td,read_meter_flag_curve_hunan.cycle_day,3);//bin
	
    offset = get_curve_save_offset_hunan(phy,dh_td,td);
      
    fread_array(meter_idx,offset,buffer,phy->data_len);

    //����ʱ��
    if(compare_string(buffer,td,5) != 0)
    {
        mem_set(buffer,phy->data_len,0xFF);
        mem_cpy(buffer,td,5);

        #ifdef __PROVICE_ANHUI__
        buffer_bin_to_bcd(datetime+MINUTE,td_bcd,5);


        if((td_bcd[0]) >= 0x45)td_bcd[0] = 0x45;
        else if((td_bcd[0]) >= 0x30)td_bcd[0] = 0x30;
        else if((td_bcd[0]) >= 0x15)td_bcd[0] = 0x15;
        else td_bcd[0] = 0;

        mem_cpy(buffer+5+phy->block_len,td_bcd,5);/*����Ҫ����ʱ�꣬Ԥ����5�ֽڴ�һ�£�*/
        #endif
    }

    if(read_params->item_format)
    {
        if(read_params->meter_doc.protocol == GB645_1997)
        {
            mem_cpy(buffer+5+phy->block_offset,data,datalen);
            
        }
        else
        {
            //��������
            datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
            mem_cpy(buffer+5+phy->block_offset,data,datalen);
        }
    }
    else
    {
        //��������
        datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
        mem_cpy(buffer+5+phy->block_offset,data,datalen);
    }

    fwrite_array(meter_idx,offset,buffer,phy->data_len);
    return TRUE;
}
INT8U save_curve_data_hunan(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    INT8U* td;
    INT8U idx,midu;

    idx = get_phy_form_list_cruve_hunan(bin2_int32u(read_params->phy),&phy);
    if(idx == 0xFF) return 0;
    fread_array(bin2_int16u(read_params->meter_doc.meter_idx),phy.offset,&midu,1);
    td = read_meter_flag_curve_hunan.cycle_4_hour;

    if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
    {
        clr_bit_value(read_params->read_mask.curve_hunan,READ_MASK_BYTE_NUM_CURVE_HUNAN,idx);
        //����mask
        clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_CURVE_HUNAN,idx);
    }
    else
    {
        writedata_curve_hunan(read_params,&phy,td,frame,frame_len,buffer);
        clr_bit_value(read_params->read_mask.curve_hunan,READ_MASK_BYTE_NUM_CURVE_HUNAN,idx);
    }
    return 0;
}

INT8U readdata_curve_hunan(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U dh_td[3],INT8U td[5],INT8U *data,INT8U *datalen)
{
    INT32U 	offset;
    //INT8U 	midu;
	//INT8U	dh_td[3];

	//mem_cpy(dh_td,read_meter_flag_curve_hunan.cycle_day,3);//bin
	//get_yesterday(dh_td);
    offset = get_curve_save_offset_hunan(phy,dh_td,td);

    fread_array(meter_idx,offset,data,phy->data_len);
    if (compare_string(td,data,5) == 0)
    {
        if(check_is_all_ch(data+5+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+5+phy->block_offset,phy->block_len);
            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

BOOLEAN prepare_read_item_curve_hunan(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT32U last_read_phy_offset = 0;
    INT32U last_read_phy_block_offset;
    INT16U meter_idx;
    INT8U* td;
	INT8U *dh_td;
    INT8U idx,idx_sub,mask_idx;

    INT8U need_read_item_no =0;
    INT8U gb_645_zuhe_frame[200] = {0};
    INT8U zuhe_frame_len = 0;
    INT8U parall_mode = 0;
    #ifdef __READ_OOP_METER__
    INT8U oad_byte[4];
    #endif

    mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    if (get_bit_value(read_meter_flag_curve_hunan.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;
    if (check_is_all_ch(read_params->read_mask.curve_hunan,READ_MASK_BYTE_NUM_CURVE_HUNAN,0x00))
    {
        clr_bit_value(read_meter_flag_curve_hunan.flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }

    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
       if( portContext_plc.plc_other_read_mode == CYCLE_REC_MODE_PARALLEL )
       {
          parall_mode = 0xAA;
       }
    }

    for(idx=0;idx<sizeof(CRUVE_PHY_LIST_HUNAN)/sizeof(READ_WRITE_DATA);idx++)
    {
        if(CRUVE_PHY_LIST_HUNAN[idx].offset != last_read_phy_offset) /*���ƫ�ƶ����ˣ�˵����ǰ��Ӧ��������϶�Ҫ�ж��Ƿ񳭶������û�䣬����֮ǰ��ƫ�Ʊȶ�*/
        {
            last_read_phy_block_offset = 0;
        }

        for(idx_sub=0;idx_sub<((CRUVE_PHY_LIST_HUNAN[idx].flag&0x0F)+1);idx_sub++)
        {
            if(get_bit_value(read_params->read_mask.curve_hunan,READ_MASK_BYTE_NUM_CURVE_HUNAN,mask_idx))
            {
                get_phy_data((READ_WRITE_DATA*)&(CRUVE_PHY_LIST_HUNAN[idx]),idx_sub,&phy);

                if(parall_mode == 0xAA)/*����ģʽ*/
                {
                    if(CRUVE_PHY_LIST_HUNAN[idx].offset == last_read_phy_offset)   /*����洢һ��������������������*/
                    {
                        if(CRUVE_PHY_LIST_HUNAN[idx].block_offset == last_read_phy_block_offset) /*�������ݣ�ͨ��offset�жϣ������ѹ�͵����ȵĴ洢һ������������*/
                        {
                            mask_idx++;
                            continue ;
                        }
                        last_read_phy_block_offset = CRUVE_PHY_LIST_HUNAN[idx].block_offset;

                    }
                    last_read_phy_offset = CRUVE_PHY_LIST_HUNAN[idx].offset;
                }

				td = read_meter_flag_curve_hunan.cycle_4_hour;
				dh_td = read_meter_flag_curve_hunan.cycle_day;
                if(readdata_curve_hunan(meter_idx,&phy,dh_td,td,frame,frame_len) == FALSE)
                {
                    //��Ҫ����
                    if(get_data_item(phy.phy,read_params->meter_doc.protocol,&library))
                    {
                        if(library.item != 0xFFFFFFFF)
                        {
                            if(parall_mode == 0xAA)/*��������ģʽ�£���¼���������*/
                            {
                                int32u2_bin(phy.phy,read_params->phy+need_read_item_no*4);
                                int32u2_bin(library.item,read_params->item+need_read_item_no*4);
                            }
                            else
                            {
                                int32u2_bin(phy.phy,read_params->phy);
                                int32u2_bin(library.item,read_params->item);
                            }

                            read_params->item_len = library.len;
                            read_params->item_format = library.format;
                            read_params->resp_byte_num = 40;
                            read_params->read_type = READ_TYPE_CURVE_HUNAN;

                            if (read_params->meter_doc.protocol == GB645_2007)
                            {
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            else if (read_params->meter_doc.protocol == GB645_1997)
                            {
                                *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #ifdef __READ_OOP_METER__
                            else if(GB_OOP == read_params->meter_doc.protocol)
                            {
                                oad_byte[0] = library.item>>24;
                                oad_byte[1] = library.item>>16;
                                oad_byte[2] = library.item>>8;
                                oad_byte[3] = library.item;
                                if((library.phy_prop != 0xFF)&&(library.item != 0xFFFFFFFF))
                                {
                                    *frame_len = make_oop_read_frame(frame,read_params->meter_doc.meter_no,oad_byte,1,(0x5000 + library.phy_prop),1);
                                }
                                else
                                {
                                    *frame_len = make_oop_read_frame(frame,read_params->meter_doc.meter_no,oad_byte,1,0xFFFF,1);
                                }
                            }
                            #endif
                            else
                               *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            #ifdef __SOFT_SIMULATOR__
                            snprintf(info,100,"*** prepare item curve hunan: meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
                                    bin2_int16u(read_params->meter_doc.meter_idx),library.item,phy.phy,mask_idx);
                            debug_println_ext(info);
                            #endif

                            if((parall_mode == 0xAA) &&((read_params->meter_doc.protocol == GB645_2007)
                            || (read_params->meter_doc.protocol == GB645_1997)))
                            {
                                mem_cpy(gb_645_zuhe_frame+zuhe_frame_len,frame, *frame_len );
                                zuhe_frame_len += *frame_len;
                                need_read_item_no ++ ;

                                if(need_read_item_no >= portContext_plc.parall_max_item) /*���������������ã�Ĭ��3*/
                                {
                                   mem_cpy(frame,gb_645_zuhe_frame,zuhe_frame_len);
                                   *frame_len = zuhe_frame_len;
                                   return TRUE;
                                }
                                else
                                {
                                   mask_idx++;
                                   continue;
                                }
                            }
                            else
                            {
                                return TRUE;
                            }
                        }
                    }
                }
                clr_bit_value(read_params->read_mask.curve_hunan,READ_MASK_BYTE_NUM_CURVE_HUNAN,mask_idx);
            }
            mask_idx++;
        }
    }

    if(parall_mode == 0xAA)
    {
        if(need_read_item_no )
        {
           mem_cpy(frame,gb_645_zuhe_frame,zuhe_frame_len);
           *frame_len = zuhe_frame_len;
           return TRUE;
        }
        else
        {
           clr_bit_value(read_meter_flag_curve_hunan.flag,READ_FLAG_BYTE_NUM,meter_idx);
        }
    }
    else
    {
        clr_bit_value(read_meter_flag_curve_hunan.flag,READ_FLAG_BYTE_NUM,meter_idx);
    }

    return FALSE;
}
#endif
//---------------------------------------------------------------------------
INT16U get_phy_form_list_cur_data(INT32U phy,READ_WRITE_DATA* out,INT16U *block_begin_idx,INT8U *block_count)
{
    INT8U idx,idx_sub;
    INT16U mask_idx;

    mask_idx = 0;
    *block_begin_idx = 0;
    *block_count = 0;

    for(idx=0;idx<sizeof(CUR_DATA_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        *block_begin_idx = mask_idx;
        *block_count = (CUR_DATA_PHY_LIST[idx].flag&0x1F)+1 ;
        for(idx_sub=0;idx_sub<((CUR_DATA_PHY_LIST[idx].flag&0x1F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(CUR_DATA_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFFFF;
}

//idx 0: ��һ��  1���϶��Σ���������
INT32U get_last_data_save_offset(INT16U meter_idx,READ_WRITE_DATA *phy,BOOLEAN is_max)
{
    INT32U offset[2];
    INT32U seq[2]={0};
    INT8U idx;

    for(idx=0;idx<2;idx++)
    {
        offset[idx] = phy->offset+idx*phy->data_len;
        fread_array(meter_idx,offset[idx],(INT8U*)(seq+idx),4);
    }

    if ((seq[0] == 0xFFFFFFFF) || (seq[1] == 0xFFFFFFFF))
    {
        if(seq[1] == 0xFFFFFFFF)
    {
        seq[0] = 0;
        seq[1] = 1;
        }
        else
        {
            seq[0] = 1;
            seq[1] = 0;
        }    
        fwrite_array(meter_idx,offset[0],(INT8U*)(seq+0),4);
        fwrite_array(meter_idx,offset[1],(INT8U*)(seq+1),4);
    }


    if(is_max)
    {
        return (seq[0] > seq[1]) ? offset[0] : offset[1];
    }
    else //Ҫ���С��
    {
        return (seq[0] < seq[1]) ? offset[0] : offset[1];
    }
}
//idx 0: ��һ��  1���϶��Σ���������,����ֵΪƫ�Ƶ�ַ��dataΪ����
INT32U get_last_data_save_offset_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U* data,BOOLEAN is_max)
{
    INT32U offset[2];
    INT32U seq[2]={0};
    INT8U idx,need_read;
    INT8U tmp_buffer[260];

    need_read = FALSE;
    if(phy->data_len > 130)
    {
        fread_array(meter_idx,phy->offset,tmp_buffer,4+phy->data_len);
        need_read = TRUE;
    } 
    else
    {
        fread_array(meter_idx,phy->offset,tmp_buffer,2*phy->data_len);
    }

    for(idx=0;idx<2;idx++)
    {
        offset[idx] = phy->offset+idx*phy->data_len;
        seq[idx] = bin2_int32u(tmp_buffer+idx*phy->data_len);
    }

    if ((seq[0] == 0xFFFFFFFF) || (seq[1] == 0xFFFFFFFF))
    {
        if(seq[1] == 0xFFFFFFFF)
        {
            seq[0] = 0;
            seq[1] = 1;
        }
        else
        {
            seq[0] = 1;
            seq[1] = 0;
        }    
        fwrite_array(meter_idx,offset[0],(INT8U*)(seq+0),4);
        fwrite_array(meter_idx,offset[1],(INT8U*)(seq+1),4);
    }


    if(is_max)
    {   
        if(seq[0] > seq[1])
        {
            mem_cpy(data,tmp_buffer+4,phy->data_len-4);
        }
        else
        {
            offset[0] = offset[1];
            if(need_read)
            {
                fread_array(meter_idx,phy->offset+phy->data_len,tmp_buffer,phy->data_len);
                mem_cpy(data,tmp_buffer+4,phy->data_len-4);
            }
            else
            {
                mem_cpy(data,tmp_buffer+4+phy->data_len,phy->data_len-4);
            }
        } 
    }
    else //Ҫ���С��
    {
        if(seq[0] > seq[1])
        {
            offset[0] = offset[1];
            if(need_read)
            {
                fread_array(meter_idx,phy->offset+phy->data_len,tmp_buffer,phy->data_len);
                mem_cpy(data,tmp_buffer+4,phy->data_len-4);
            }
            else
            {
                mem_cpy(data,tmp_buffer+4+phy->data_len,phy->data_len-4);
            }
        }
        else
        {
            mem_cpy(data,tmp_buffer+4,phy->data_len-4);
        } 
    }
    return offset[0];
}
INT8U writedata_cur_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U *buffer)
{
    INT32U offset;

    #ifdef __HIGH_PRECISION_DATA__
    INT8U real_datalen;
    real_datalen = datalen;
    #endif

     #ifdef __SOFT_SIMULATOR__
     snprintf(info,100,"*** save curdata : meter_idx = %d ,  phy = 0x%08X"  ,meter_idx, (phy->phy));
     debug_println_ext(info);
     #endif
    //��97����������ʱ���⣬���������С��λ��д����
    if((ZXYG_ZDXLSJ_SJK == phy->phy)||
       (ZXWG_ZDXLSJ_SJK == phy->phy)||
       (FXYG_ZDXLSJ_SJK == phy->phy)||
       (FXWG_ZDXLSJ_SJK == phy->phy))
    {
        offset = get_last_data_save_offset_data(meter_idx,phy,buffer,TRUE);
    }
    else
    {
        offset = get_last_data_save_offset_data(meter_idx,phy,buffer,FALSE);
    }

    //����ʱ��
    mem_cpy(buffer,datetime+MINUTE,5);
    //��������
    if (phy->flag & SAVE_FLAG_XL_TIME)
    {
        datalen = format_data_save(0,data,datalen,buffer+5,phy);
    }
    else
    {
        datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
        mem_cpy(buffer+5+phy->block_offset,data,datalen);
    }
    #ifdef __HIGH_PRECISION_DATA__
    //������������޹���Щ����������5�ֽ����ݣ�Ҫ���⴦����
    if((phy->phy >= (ZXYG_DN_SJK-0x3F)) && (phy->phy <= D4XX_WG_DN_SJK))
    {
       if((datalen == 4) && (real_datalen ==5))
       {
           mem_cpy(buffer+5+phy->block_offset,data+1,datalen); 
       }
    }
    #endif
    if(phy->phy == SJ )//���ʱ��
    {
        mem_cpy(buffer+5+phy->block_offset+datalen,datetime,6);
    }
    fwrite_array(meter_idx,offset+4,buffer,phy->data_len-4);
    return TRUE;
}

INT8U writedata_cur_data_seq(INT16U meter_idx,READ_WRITE_DATA *phy)
{
    INT32U seq,seq1;
    INT8U tmp_buffer[260];

    seq = 0;
    seq1 = 0;
    //�����С��λ��д����
    fread_array(meter_idx,phy->offset,tmp_buffer,4+phy->data_len);
    
    seq = bin2_int32u(tmp_buffer);
    seq1 = bin2_int32u(tmp_buffer+phy->data_len);
    
    if(seq > seq1)
    {
        seq += 1;
        fwrite_array(meter_idx,phy->offset+phy->data_len,(INT8U*)&seq,4);
    }
    else
    {
        seq1 += 1;
        fwrite_array(meter_idx,phy->offset,(INT8U*)&seq1,4);
    }
    return TRUE;
}
#ifdef __ZhuZhou_CITY__
//�ж�ʱ������
INT8U time_subsection (INT8U time_hour)
{
  INT8U seq;
  switch (time_hour)
  {
  case 7:
  case 8:
  case 9:
  case 10:
       seq = 1;
       break;
  case 11:
  case 12:
  case 13:
  case 14:
       seq = 2;
       break;
  case 15:
  case 16:
  case 17:
  case 18:
       seq = 3;
       break;
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
       seq = 4;
       break;
  default:
       seq = 0;
       break;
  }
  return seq;
}
#endif

INT8U readdata_cur_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U rec_datetime[5],INT8U *data,INT8U *datalen,BOOLEAN flag)
{
    //INT32U offset;
    //INT8U midu;
    #ifdef __ZhuZhou_CITY__
    INT8U rec_seq,cur_seq;
    #endif

    //��ȡ�ܶ�
    (void)get_last_data_save_offset_data(meter_idx,phy,data,flag); 

    //   set_DATAFMT_01((DATAFMT_01*)(data+16));
    mem_cpy(rec_datetime,data,5);
    if(flag ==true)
    {

    }
    #ifdef __ZhuZhou_CITY__
    if((rec_datetime[4] != datetime[YEAR])||(rec_datetime[3] != datetime[MONTH])||(rec_datetime[2] != datetime[DAY]))
    {
        *datalen = 0;
        mem_set(data,phy->block_len,0xFF);
        return FALSE;
    }
    rec_seq = time_subsection(rec_datetime[1]);
    cur_seq = time_subsection(datetime[HOUR]);
    if((rec_seq == 0)||(cur_seq == 0))
    {
        *datalen = 0;
        mem_set(data,phy->block_len,0xFF);
        return FALSE;
    }
    if(rec_seq != cur_seq)
    {
        *datalen = 0;
        mem_set(data,phy->block_len,0xFF);
        return FALSE;
    }
    #endif
    if(check_is_all_ch(data+5+phy->block_offset,phy->block_len,0xFF))
    {
        *datalen = 0;
        mem_set(data,phy->block_len,0xFF);
        return FALSE;
    }
    else
    {
        *datalen = phy->block_len;
        mem_cpy(data,data+5+phy->block_offset,phy->block_len);
        return TRUE;
    }
}

INT16S app_readdata_cur_data(INT16U meter_idx,INT32U phy_id,INT8U rec_datetime[5],INT8U *data,INT8U max_datalen,BOOLEAN flag)
{
    READ_WRITE_DATA phy;
    INT16U idx;
    INT16U block_begin_idx = 0;
    INT8U datalen;
    INT8U block_count = 0;
    //INT8U midu;

    mem_set((void *)&phy,sizeof(READ_WRITE_DATA),0x00);
    idx = get_phy_form_list_cur_data(phy_id,&phy,&block_begin_idx,&block_count);
    if (idx == 0xFFFF) return -1;
    if (phy.data_len > max_datalen) return -1;
    if( FALSE == readdata_cur_data(meter_idx,&phy,rec_datetime,data,&datalen,flag) )
    {
        //coverity��麯������ֵ���⣬�����±�֤���澯
        datalen = 0;
    }
    return datalen;
}

//���ܣ���ȡָ�����ˡ�ָ�����ȵ����ݣ����ö������Ļ�����
//read_time������ʱ��
//is_cur������ǰ����һ��
INT16U app_read_cur_part_data(INT16U meter_idx,INT32U phy,INT8U *data,INT16U read_offset,INT16U read_len,INT8U *read_time,BOOLEAN is_cur)
{
    INT32U offset1,offset2;
    INT32U seq1,seq2;
    READ_WRITE_DATA pPHY;
    INT16U block_begin_idx = 0;
    INT16U list_idx;
    INT16U data_len = 0;
    INT8U seq_tmp[8];
    INT8U block_count;
    
    list_idx = get_phy_form_list_cur_data(phy,(READ_WRITE_DATA *)&pPHY,&block_begin_idx,&block_count);
    read_offset += 9;   //seq-4 read_time-5
    //δ�ҵ����߻�ȡ���ݳ��ȴ��ڴ��뻺�����ĳ���,set data to FF 
    if((list_idx == 0xFFFF) || (read_offset >= pPHY.data_len))
    {
        mem_set(data,read_len,0xFF);
        return 0;
    }
    
    offset1 = pPHY.offset;
    offset2 = pPHY.offset+pPHY.data_len;
    
    fread_array(meter_idx,offset1,seq_tmp,4);
    fread_array(meter_idx,offset2,seq_tmp+4,4);
    seq1 = bin2_int32u(seq_tmp);
    seq2 = bin2_int32u(seq_tmp+4);
    if ((seq1 == 0xFFFFFFFF) || (seq2 == 0xFFFFFFFF))
    {
        if(seq2 == 0xFFFFFFFF)
        {
            seq1 = 0;
            seq2 = 1;
        }
        else
        {
            seq1 = 1;
            seq2 = 0;
        }
        fwrite_array(meter_idx,offset1,(INT8U*)&seq1,4);
        fwrite_array(meter_idx,offset2,(INT8U*)&seq2,4);
    }
    
    if((read_len + read_offset) > pPHY.data_len)
    {
        data_len = pPHY.data_len - read_offset;
    }
    else
    {
        data_len = read_len;
    }
    
    if(is_cur)
    {
        if(seq1 < seq2)
        {
            offset1 = offset2;
        }
    }
    else    //Ҫ���С��
    {
        if(seq1 > seq2)
        {
            offset1 = offset2;
        }
    }
    fread_array(meter_idx,offset1+read_offset,data,data_len);
    if(read_time != NULL)
    {
        fread_array(meter_idx,offset1+4,read_time,5);
    }
    return data_len;
}

void clear_read_mask_from_meter_param(INT16U meter_idx,INT8U mask_type,INT16U idx)
{
    READ_MASK read_mask;

    fread_meter_params(meter_idx,PIM_READ_MASK,read_mask.value,sizeof(READ_MASK));
    switch(mask_type)
    {
    case READ_TYPE_CYCLE_DAY:
        clr_bit_value(read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
        break;
    case READ_TYPE_CYCLE_MONTH:
        clr_bit_value(read_mask.month_hold,READ_MASK_BYTE_NUM_MONTH_HOLD,idx);
        break;
    case READ_TYPE_CURVE:
        clr_bit_value(read_mask.curve,READ_MASK_BYTE_NUM_CURVE,idx);
        break;
    case READ_TYPE_CUR_DATA:
        clr_bit_value(read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
        break;
//    case READ_TYPE_DAY_HOLD_PATCH:
//        clr_bit_value(read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,idx);
//        break;
	#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
	case READ_TYPE_CURVE_HUNAN:
        clr_bit_value(read_mask.curve_hunan,READ_MASK_BYTE_NUM_CURVE_HUNAN,idx);
        break;	
	#endif
    case READ_TYPE_LAST_CURVE_CYCLE_DAY:
        clr_bit_value(read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
        break;

    default:
        return;
    }
    fwrite_meter_params(meter_idx,PIM_READ_MASK,read_mask.value,sizeof(READ_MASK));
}

BOOLEAN check_cur_date_power_on_off(INT16U meter_idx,READ_WRITE_DATA *phy,READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    INT32U count,last_count;
    INT8U idx;
    BOOLEAN is_valid;

    if( FALSE == readdata_cur_data(meter_idx,phy,buffer,buffer+6,buffer+5,TRUE) )
    {
        //coverity��麯������ֵ���⣬�����±�֤���澯
        buffer[5] = 0;
    }
    if(compare_string(buffer+6,frame,3) != 0)
    {
      if(phy->phy == 0x0000AF00)
      {
        count = bcd2u32(frame,3,&is_valid);
        for(idx=0;idx<10;idx++)
        {
            if(idx >= count)  break;
            set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_DDCS_Z+idx+1);
        }
        return TRUE;
      }
      if(phy->phy == KBG_ZCS)
      {
            count = bcd2u32(frame,3,&is_valid);
            last_count = bcd2u32(buffer+6,3,&is_valid);
            if(count > last_count)
            set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_DBKBGCS+1);

           return TRUE;

      }
      if(phy->phy == KDNH_ZCS)
      {
            count = bcd2u32(frame,3,&is_valid);
            last_count = bcd2u32(buffer+6,3,&is_valid);
            if(count > last_count)
            set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_KDNGCS+1);

           return TRUE;

      }
    }

    return FALSE;
}

#if ((defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__) || (defined __FUJIAN_CURRENT_BREAK__))
BOOLEAN check_cur_date_plc_residual_current_transformer(INT16U meter_idx,READ_WRITE_DATA *phy,READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    INT32U count,last_count;
    INT8U idx;
    BOOLEAN is_valid;

    if( FALSE == readdata_cur_data(meter_idx,phy,buffer,buffer+6,buffer+5,TRUE) )
    {
        //coverity��麯������ֵ���⣬�����±�֤���澯
        buffer[5] = 0;       
    }
    if(compare_string(buffer+6,frame,3) != 0)
    {
    #ifdef  __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__
        if(phy->phy == DD_ZCS)
        {
            count = bcd2u32(frame,3,&is_valid);
            last_count = bcd2u32(buffer+6,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//��һ�������� 
            }
            if(count > last_count)
            {
                count = count - last_count;//���ӵĴ��� 
                for(idx=0;idx<10;idx++)
                {
                    if(idx >= count)  break;
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_DDCS_Z+idx+1);
                }
                //set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_RESIDUAL_CURRENT_OVERRUN_RECORD+idx);
            }
            return TRUE;
        }
        if(phy->phy == QSY_ZCS_ZLJ_SJ) // 03050000 ʣ����������ܴ���
        {
            count = bcd2u32(frame,3,&is_valid);
            last_count = bcd2u32(buffer+6,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//��һ�������� 
            }
            if(count > last_count)
            {
                count = count - last_count;//���ӵĴ��� 
                for(idx=0;idx<10;idx++)
                {
                    if(idx >= count)  break;
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_RESIDUAL_CURRENT_OVERRUN_RECORD_01+idx);
                }
                //set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_RESIDUAL_CURRENT_OVERRUN_RECORD+idx);
            }
            return TRUE;

        }
        if(phy->phy == FZDY_SD_ZCS_ZLJ_SJ) // 03060000 ʣ�����������·�����ܴ���
        {
            count = bcd2u32(frame,3,&is_valid);
            last_count = bcd2u32(buffer+6,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//��һ�������� 
            }
            if(count > last_count)
            {
                count = count - last_count;//���ӵĴ��� 
                for(idx=0;idx<10;idx++)
                {
                    if(idx >= count)  break;
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_RESIDUAL_CURRENT_BREAK_RECORD_01+idx);
                }
            }


            return TRUE;

        }
     #endif
        #ifdef __FUJIAN_CURRENT_BREAK__
        if(phy->phy == FUJIAN_CUR_BREAK_QLZCS)
        {
            count = bcd2u32(frame,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//���γ�����Ч����
            }
            last_count = bcd2u32(buffer+6,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//��һ��������
            }
            if(count > last_count)
            {
                count = count - last_count;//���ӵĴ���
                for(idx=0;idx<10;idx++)
                {
                    if(idx >= count)  break;
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_FUJIAN_CURRENT_BREAK_QLSJK+idx);
                }
            }


            return TRUE;
        }
        if(phy->phy == FUJIAN_CUR_BREAK_DDZCS)
        {
            count = bcd2u32(frame,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//���γ�����Ч����
            }
            last_count = bcd2u32(buffer+6,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//��һ��������
            }
            if(count > last_count)
            {
                count = count - last_count;//���ӵĴ���
                for(idx=0;idx<10;idx++)
                {
                    if(idx >= count)  break;
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_FUJIAN_CURRENT_BREAK_DDSJK+idx);
                }
            }


            return TRUE;
        }
        if(phy->phy == FUJIAN_CUR_BREAK_HZZCS)
        {
            count = bcd2u32(frame,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//���γ�����Ч����
            }
            last_count = bcd2u32(buffer+6,3,&is_valid);
            if(FALSE == is_valid)
            {
                return TRUE;//��һ��������
            }
            if(count > last_count)
            {
                count = count - last_count;//���ӵĴ���
                for(idx=0;idx<10;idx++)
                {
                    if(idx >= count)  break;
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_FUJIAN_CURRENT_BREAK_HZSJK+idx);
                }
            }


            return TRUE;
        }
        #endif
    }

    return FALSE;
}
#endif

void format_07syjl_to_phy(INT32U phy,INT8U* data,INT8U* data_len)
{
    INT8U tmp[6];
    switch(phy)
    {
//        case ABC_DX_LJSJ:
//            if(*data_len < 18)
//            {   
//                *data_len = 0;
//                return;
//            }
//            mem_cpy(data+3,data+6,3);
//            mem_cpy(data+6,data+12,3);
//            *data_len = 9;
//            break;
        case S1C_A_SYJL_07:
        case S1C_B_SYJL_07:
        case S1C_C_SYJL_07:
            if(*data_len < 131)
            {   
                *data_len = 0;
                return;
            }
            mem_cpy(tmp,data+6,6);//����ʱ��
            mem_cpy(data+6,data+12,157);
            mem_cpy(data+163,tmp,6);
            break; 
        default:
            break;
    }
}


INT8U save_cur_date(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    BOOLEAN read_cur_data(INT16U meter_idx,INT32U phy,INT8U *data,INT8U *datalen,INT16U max_datalen,BOOLEAN is_cur);
    READ_WRITE_DATA phy;
    INT16U meter_idx,idx,block_begin_idx,seq;   //�������idxҪ���ã���Ϊ���������������ڱ������ţ�ʲôʱ��ʹ�����������������FORѭ��������ⶨ�������
	#ifdef __ZhuZhou_CITY__
    INT16U sy_idx;
	#endif
    INT8U block_count;
    BOOLEAN write_seq;
    INT8U tmpstr[21],meter_time[6];
    INT8U flag1,flag2;
    #ifdef __PRECISE_TIME__
    PRECISE_TIME_CAST_CTRL cast_time_ctrl;
    INT8U read_time[3],td[3];
    SET_F298 F298;
    #endif
    #ifdef __JIANGSU_READ_CURRENT_MONITOR__
    INT8U i,cnt;
    #endif
    INT16U mask_idx = 0;
    
    idx = get_phy_form_list_cur_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count);
    if(idx == 0xFFFF) return 0;
    mask_idx = idx;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    #if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
    if( ((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || ((read_params->meter_doc.baud_port.port == COMMPORT_PLC)) )
       && (read_params->meter_doc.meter_class.user_class == 14) )
    {
        // 03050000 ��03060000 03110000 �����ܴ��� 
        if( (phy.phy == QSY_ZCS_ZLJ_SJ) || (phy.phy == FZDY_SD_ZCS_ZLJ_SJ) || (phy.phy == DD_ZCS) )
        {
            //
            if(check_cur_date_plc_residual_current_transformer(meter_idx,&phy,read_params,frame,frame_len,buffer) == FALSE)
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            }
        }
    }
    else
    {
        //
    if(phy.phy == 0x0000AF00)
    {
        if(check_cur_date_power_on_off(meter_idx,&phy,read_params,frame,frame_len,buffer) == FALSE)
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            return 0;
        }
    }
    //����Ǵ����ж�
    if(phy.phy == KBG_ZCS)
    {
        if(check_cur_date_power_on_off(meter_idx,&phy,read_params,frame,frame_len,buffer) == FALSE)
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
 //           return 0;
        }
    }
            //����ť�Ǵ����ж�
        if(phy.phy == KDNH_ZCS)
        {
            if(check_cur_date_power_on_off(meter_idx,&phy,read_params,frame,frame_len,buffer) == FALSE)
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
     //           return 0;
            }
        }
    }
    #else

    if(phy.phy == 0x0000AF00)
    {
        if(check_cur_date_power_on_off(meter_idx,&phy,read_params,frame,frame_len,buffer) == FALSE)
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            return 0;
        }
    }
    //����Ǵ����ж�
    if(phy.phy == KBG_ZCS)
    {
        if(check_cur_date_power_on_off(meter_idx,&phy,read_params,frame,frame_len,buffer) == FALSE)
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
 //           return 0;
        }
    }
        //����ť�Ǵ����ж�
    if(phy.phy == KDNH_ZCS)
    {
        if(check_cur_date_power_on_off(meter_idx,&phy,read_params,frame,frame_len,buffer) == FALSE)
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
 //           return 0;
        }
    }
    #endif
#ifndef __PROVICE_BEIJING__
    if ((phy.phy == ABC_DX_LJSJ) && (frame[0] != REC_DATA_IS_DENY))
    {
        mem_set(tmpstr,12,REC_DATA_IS_DENY);
        mem_cpy(tmpstr,frame,3);
        mem_cpy(tmpstr+3,frame+6,3);
        mem_cpy(tmpstr+6,frame+12,3);
        mem_cpy(frame,tmpstr,12);
        frame_len = 12;
    }
#endif
#ifdef __JIANGSU_READ_CURRENT_MONITOR__
 //ʣ�����--------------------------------------------------------------
    if(read_params->meter_doc.protocol == BREAKER_MONITOR)
    {
        if ((phy.phy == DBYXZTZ_SJK) && (frame[0] != REC_DATA_IS_DENY))
        {
            if(frame[1]&0xF8) //���¼���־λ��λ����ȡ��բ����ģ�飬�������ȡ��������
            {
                //set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_TZCS);
                if(frame[1] & 0x08) // ERC56 ʧ��
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_CXDSDXZCS);
                }
                if(frame[1] & 0x10) // ERC58 �����˳�
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLBHTCXZCS);
                }
                if(frame[1] & 0x20) // ERC57 ����
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLCYHLDXXZCS);
                }
                if(frame[1] & 0x40) // ERC55  ȱ�� 
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_XLQXXZCS);
                }
                if(frame[1] & 0x80) // ERC61 ����
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLCXXZCS);
                }
            }

            if(frame[1]&0x07) //��բ����,ֱ����״̬��2�ĵ�3λ����բ����ָ��ó���Э����ע���ġ�
            {
                for(i=0;i<(frame[1]&0x07);i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_TZSJJL_S1C+i);
                }
            }
        }

        if((phy.phy == TZCS_CS_MK_A) && (frame[0] != REC_DATA_IS_DENY)) //ERC61
        {
            if(TRUE == is_valid_bcd(frame,1))
            {
                cnt = BCD2byte(frame[0]);
                if(cnt > 10) cnt = 10;
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLCXSJJL_S1C+i);
                }
            }
        }
        if((phy.phy == TZCS_CS_MK_B) && (frame[0] != REC_DATA_IS_DENY)) //ERC55
        {
            if(TRUE == is_valid_bcd(frame,1))
            {
                cnt = BCD2byte(frame[0]);
                if(cnt > 10) cnt = 10;
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_XLQXSJJL_S1C+i);
                }
            }
        }
        if((phy.phy == TZCS_CS_MK_C) && (frame[0] != REC_DATA_IS_DENY)) //ERC58
        {
            if(TRUE == is_valid_bcd(frame,1))
            {
                cnt = BCD2byte(frame[0]);
                if(cnt > 10) cnt = 10;
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLBHTCSJJL_S1C+i);
                }
            }
        }
        if((phy.phy == TZCS_CS_MK_D) && (frame[0] != REC_DATA_IS_DENY)) //ERC56
        {
            if(TRUE == is_valid_bcd(frame,1))
            {
                cnt = BCD2byte(frame[0]);
                if(cnt > 10) cnt = 10;
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_CXDSDJL_S1C+i);
                }
            }
        }
        if((phy.phy == TZCS_CS_MK_E) && (frame[0] != REC_DATA_IS_DENY)) //ERC57
        {
            if(TRUE == is_valid_bcd(frame,1))
            {
                cnt = BCD2byte(frame[0]);
                if(cnt > 10) cnt = 10;
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLCYHLDXSJJL_S1C+i);
                }
            }
        }
        

        if((phy.phy == TZCS_CS_MK_SJK) && (frame[0] != REC_DATA_IS_DENY))//����ϻ��20��EE�������жϵ���20���Ժ�ġ�����ϴ�EE����Ӧ�ô���һ�£�����ʵ�ʳ��ȴ洢��
        {
            if(frame[20] != REC_DATA_IS_DENY)//���������¼�
            {
                cnt = (frame[20] > 10)? 10 : frame[20];
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLCXSJJL_S1C+i);
                }
            }

            if(frame[21] != REC_DATA_IS_DENY)//��·ȱ���¼�
            {
                cnt = (frame[21] > 10)? 10 : frame[21];
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_XLQXSJJL_S1C+i);
                }
            }

            if(frame[22] != REC_DATA_IS_DENY)//�����˳��¼�
            {
                cnt = (frame[22] > 10)? 10 : frame[22];
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLBHTCSJJL_S1C+i);
                }
            }

            if(frame[23] != REC_DATA_IS_DENY)//���߶�ʧ���¼�
            {
                cnt = (frame[23] > 10)? 10 : frame[23];
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_CXDSDJL_S1C+i);
                }
            }

            if(frame[24] != REC_DATA_IS_DENY)//ʣ�����������·�����¼�
            {
                cnt = (frame[24] > 10)? 10 : frame[24];
                for(i=0;i<cnt;i++)
                {
                    set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_SYDLCYHLDXSJJL_S1C+i);
                }
            }
        }


    }
    //---------------------------------------------------------------
#endif
#ifdef __FUJIAN_CURRENT_BREAK__
 //ʣ�����--------------------------------------------------------------
    if(read_params->meter_doc.protocol == FUJIAN_BREAKER_MONITOR)
    {
        if ((phy.phy == FUJIAN_CUR_BREAK_ZTZ) && (frame[0] != REC_DATA_IS_DENY)) //״̬�֣�
        {

            if(((frame[0] & 0x1F) == 0x0C) || ((frame[0] & 0x1F) == 0x0E) || ((frame[0] & 0x1F) == 0x10)) // ERC84�Լ�
            {
                set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_FUJIAN_CURRENT_BREAK_ZJSJK);
            }
            else if(frame[0] & 0x1F) // ERC85��բ�¼�
            {
                set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_FUJIAN_CURRENT_BREAK_TZSJK);
            }

            if(frame[0] & 0x80) // ERC86 ʣ������澯
            {
                set_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_FUJIAN_CURRENT_BREAK_SYDLSJK);
            }

        }

        if((phy.phy == FUJIAN_CUR_BREAK_QLZCS) && (frame[0] != REC_DATA_IS_DENY)) //ERC80������,
        {

            check_cur_date_plc_residual_current_transformer(meter_idx,&phy,read_params,frame,frame_len,buffer);

            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);

        }
        if((phy.phy == FUJIAN_CUR_BREAK_DDZCS) && (frame[0] != REC_DATA_IS_DENY)) //ERC81������
        {
            check_cur_date_plc_residual_current_transformer(meter_idx,&phy,read_params,frame,frame_len,buffer);
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            }
        }
        if((phy.phy == FUJIAN_CUR_BREAK_HZZCS) && (frame[0] != REC_DATA_IS_DENY)) //ERC82����բ
        {
            check_cur_date_plc_residual_current_transformer(meter_idx,&phy,read_params,frame,frame_len,buffer);
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            }
        }

    }
    //---------------------------------------------------------------
#endif
    if(frame[0] == REC_DATA_IS_DENY)
    {
        if(A_SY_ZCS == bin2_int32u(read_params->phy))
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_B_SY_ZCS);
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_C_SY_ZCS);
            
            for(seq=READ_MASK_CUR_DATA_S1C_A_SY_SJK;seq<=READ_MASK_CUR_DATA_S1C_C_SY_SJK;seq++)
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,seq);
            }
        }
        if(ABC_DX_LJSJ == bin2_int32u(read_params->phy))
        {
            for(seq=READ_MASK_CUR_DATA_S1C_A_SYJL_07;seq<=READ_MASK_CUR_DATA_S1C_C_SYJL_07;seq++)
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,seq);
            }
        }
    }
    else
    {
        if(A_SY_ZCS == bin2_int32u(read_params->phy))
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,READ_MASK_CUR_DATA_ABC_DX_LJSJ);
            for(seq=READ_MASK_CUR_DATA_S1C_A_SYJL_07;seq<=READ_MASK_CUR_DATA_S1C_C_SYJL_07;seq++)
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,seq);
            }
        }
        format_07syjl_to_phy(bin2_int32u(read_params->phy),frame,&frame_len);
    }
    
    
    if(((phy.phy & 0x000000FF) == 0x3F) 
        || ((phy.phy & 0x000000FF) == 0x7F) 
        || ((phy.phy & 0x000000FF) == 0xBF) 
        || ((phy.phy & 0x000000FF) == 0xFF)) //�ǿ������� 
    {
        if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            //����msak
            clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,idx);
        }
        else
        {
            writedata_cur_data(meter_idx,&phy,frame,frame_len,buffer);
            if((ZXYG_ZDXLSJ_SJK != phy.phy)&&
               (ZXWG_ZDXLSJ_SJK != phy.phy)&&
               (FXYG_ZDXLSJ_SJK != phy.phy)&&
               (FXWG_ZDXLSJ_SJK != phy.phy))
            {
                writedata_cur_data_seq(meter_idx,&phy);
            }
            for(idx=0;idx<block_count;idx++)
            {
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,block_begin_idx+idx);
            }
        }
    }
    else
    {
        if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
        {
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
            //����msak
            clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,idx);
        }
        else
        {
            writedata_cur_data(meter_idx,&phy,frame,frame_len,buffer);
            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
        }
        //����Ƿ񳭶���ɣ����Ҫ�������
        write_seq = TRUE;
        #ifdef __ZhuZhou_CITY__
        if((phy.phy == A_SY_ZLJ_SJ)||(phy.phy == B_SY_ZLJ_SJ)||(phy.phy == C_SY_ZLJ_SJ)||(phy.phy == SY_ZLJ_SJ))
        {
                sy_idx = get_phy_form_list_cur_data(A_SY_ZLJ_SJ,&phy,&block_begin_idx,&block_count);
                for(idx=sy_idx;idx<sy_idx+4;idx++)
                {
                        if(get_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx)) write_seq = FALSE;
                }
        }
        if((phy.phy == 0x00001E00)||(phy.phy == 0x00001E01)||(phy.phy == 0x00001E02))                           //��ѹ������
        {
                for(idx=0;idx<4;idx++)
                {
                        if(get_bit_value(read_params->read_mask.curve_hunan,READ_MASK_BYTE_NUM_CURVE_HUNAN,idx)) write_seq = FALSE;
                }
        }
        if((phy.phy == 0x00001E40)||(phy.phy == 0x00001E41)||(phy.phy == 0x00001E42))                            //��ѹ������
        {
                for(idx=4;idx<8;idx++)
                {
                        if(get_bit_value(read_params->read_mask.curve_hunan,READ_MASK_BYTE_NUM_CURVE_HUNAN,idx)) write_seq = FALSE;
                }
        }

        #endif

        for(idx=0;idx<block_count;idx++)
        {
            if(get_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,block_begin_idx+idx)) write_seq = FALSE;
        }
        
        if((mask_idx>=READ_MASK_CUR_DATA_A_SY_ZCS) && (mask_idx<=READ_MASK_CUR_DATA_SYLJSJ))
        {
            for(idx=READ_MASK_CUR_DATA_A_SY_ZCS;idx<=READ_MASK_CUR_DATA_SYLJSJ;idx++)/**�⼸����ͬһ���洢��ַ����ŵ��������붼������ڸ���*/
            {
                if(get_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx)) write_seq = FALSE;
            }
        }
        
        if(write_seq)
        {
            writedata_cur_data_seq(meter_idx,&phy);
        }
    }
    if((phy.phy == SJ) && get_system_flag(SYS_CLOCK_DOUBT,SYS_FLAG_BASE) && (read_params->meter_doc.baud_port.port!=COMMPORT_485_CY))
    {
        //�ȶ�ȡ����
        flag1 = read_cur_data(meter_idx,SJ,tmpstr,(INT8U*)&idx,20,TRUE);
        mem_cpy(meter_time,tmpstr,3);
        flag2 = read_cur_data(meter_idx,RQ_XQ,tmpstr,(INT8U*)&idx,20,TRUE);
        if(flag1 && flag2)
        {
            mem_cpy(meter_time+3,tmpstr+1,3);
            //��Ҫ����ʽת��Ϊ��-��-ʱ-��-��|��-��
			fix_clock_update_time(meter_time,1);
            record_log_code(LOG_SYS_CLOCK_FIX_DOUBT,NULL,0,LOG_ALL);
        }
       
    }
    #ifdef __PRECISE_TIME__
    if((phy.phy == SJ) || (phy.phy == RQ_XQ ))
    {
    fread_array(meter_idx,PIM_PRECISE_TIME_CAST_COMPLETE_OR_NO,read_time ,3);//��
     mem_cpy(td,datetime+DAY,3);
     if((compare_string(read_time,td,3) != 0) )
     {
          fread_ertu_params(EEADDR_PRECISE_CAST_PARAM,cast_time_ctrl.value ,sizeof(PRECISE_TIME_CAST_CTRL));
          fread_ertu_params(EEADDR_SET_F298,F298.value,sizeof(SET_F298));
          if((cast_time_ctrl.cast_485_complete == 1) && (F298.flag == 3))
          {
          portContext_plc.router_work_info.channel_id = 4;
          plc_router_save_meter_precise_time(meter_idx,phy.phy ,frame,frame_len);

          if(phy.phy == SJ )
          {
          mem_cpy(read_time,datetime+DAY,3);
          fwrite_array(meter_idx,PIM_PRECISE_TIME_CAST_COMPLETE_OR_NO,read_time ,3);
          }
          }
     }
    }
    #endif
    return 0;
}
#ifdef __READ_IEC1107_METER__
INT8U save_cur_data_iec1107(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    READ_WRITE_DATA formerWritePhy;
    INT16U meter_idx,idx,block_begin_idx;
    INT8U block_count;
    INT32U phyValue;
    static INT32U formerPhy=0xFFFFFFFF;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if((frame==NULL)&&(buffer==NULL)&&(frame_len==0))
    {
        if(0xFFFF != get_phy_form_list_cur_data(formerPhy,&formerWritePhy,&block_begin_idx,&block_count))
            writedata_cur_data_seq(meter_idx,&formerWritePhy);
        formerPhy = 0xFFFFFFFF;
        return 0;
    }

    phyValue = bin2_int32u(read_params->phy);
    idx = get_phy_form_list_cur_data(phyValue,&phy,&block_begin_idx,&block_count);
    if(idx == 0xFFFF)
    {
        return 0;
    }

    if(formerPhy==0xFFFFFFFF)
        formerPhy = phyValue;
    if(((phyValue/64) != (formerPhy/64))&&(formerPhy!=0xFFFFFFFF))
    {
        if(0xFFFF != get_phy_form_list_cur_data(formerPhy,&formerWritePhy,&block_begin_idx,&block_count))
            writedata_cur_data_seq(meter_idx,&formerWritePhy);
        formerPhy = phyValue;
    }

    if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
    {
        clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
    }
    else
    {
        writedata_cur_data(meter_idx,&phy,frame,frame_len,buffer);
        clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx);
    }

    return 0;
}
#endif

BOOLEAN prepare_read_item_cur_data(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    //INT8U* td;
    INT16U meter_idx,mask_idx,spot_idx;
    #ifdef __READ_IEC1107_METER__
    READ_WRITE_DATA phy_iec1107;
    INT16U block_begin_idx;
    INT8U idx_iec1107,block_count,write_seq_iec1107;
    #endif
    INT8U idx,idx_sub;
    INT8U stat_flag,is_plus;//midu,
    BOOLEAN read_cur_flag = TRUE;// ��ǰ�����Ƿ���Ҫ������Ĭ����Ҫ�������ز���Ҫ�ж��Ƿ񳭶�
    BOOLEAN flag = FALSE;//
    INT8U   rec_datetime[5];//�ն˵ĳ���ʱ��
    #if ( (defined __HUNAN_NEW_RECORDING__ ) || (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__) )
    INT8U   save_section = 0;
    INT8U   cur_section  = 0;
    #endif
    RS485PortContext* pRs485Context;

    INT32U last_read_phy_offset = 0;
    INT32U last_read_phy_block_offset = 0;
    INT8U need_read_item_no =0;
    INT8U gb_645_zuhe_frame[260] = {0};
    INT8U zuhe_frame_len = 0;
    INT8U parall_mode = 0;
    #ifdef __READ_OOP_METER__
    INT8U oad_byte[4];
    INT8U oad_count=0;
    #endif

    #ifdef __MEXICO_RAIL__
    if(check_DST_delay() == FALSE)
    {
        return FALSE;
    }
    #endif
    is_plus = 0;
    mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    if (get_bit_value(read_meter_flag_cur_data,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;
    if (check_is_all_ch(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,0x00))
    {
        #if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
        if(14 == read_params->meter_doc.meter_class.user_class)
        {
            //��ع����� ������ ��ֹ�ظ����� 
            if(PLC_TASK_READ_VIP_METER == portContext_plc.cur_plc_task)
            {
                spot_idx = bin2_int16u(read_params->meter_doc.spot_idx);
                router_event_erc56(spot_idx);
                router_event_erc57(spot_idx);
                router_event_erc61(spot_idx);
            }
        }
        #endif
        #ifdef __READ_EDMI_METER__
        if(read_params->meter_doc.protocol == METER_EDMI)
        {
            if(TRUE == edmi_to_exit_state(read_params))
            {
                *frame_len = edmi_read_ctrl(read_params,NULL,0,frame,0,NULL);
                return TRUE;
            }
            read_params->read_ctrl_state = 0;
            read_params->read_ctrl_step = 0;
        }
        #endif
        #ifdef __READ_IEC1107_METER__
        if(read_params->meter_doc.protocol == IEC1107)
        {
            if(TRUE == iec1107_to_exit_state(read_params))
            {
                *frame_len = iec1107_read_ctrl(read_params,frame,0,NULL);
                if(*frame_len > 0)
                    return TRUE;
            }
        }
        #endif
        clr_bit_value(read_meter_flag_cur_data,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }

    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
        if( portContext_plc.plc_other_read_mode == CYCLE_REC_MODE_PARALLEL )
        {
             #ifdef __READ_DLMS_METER__
                 parall_mode = 0;
             #else
                 parall_mode = 0xAA;
             #endif
        }
    }

     #ifdef __READ_DLMS_METER__
CHECK_LOGIN:
    if(read_params->meter_doc.protocol == METER_DLMS) /*dlms�ȵ�¼*/
    {
       if(read_params->dlms_login_ok)
       {
        if(read_params->login_try_count > 10)
        {
            read_params->read_ctrl_state = 2;//��½��10�λ�û��½�ɹ����Ͽ����µ�¼
        }
        if(read_params->read_ctrl_state ==0)
        {
            if(read_params->read_ctrl_step==0)
            {
                *frame_len = make_dlms_snrm(frame,read_params->meter_doc.meter_no);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms snrm  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                read_params->login_try_count ++;
            }
            else if(read_params->read_ctrl_step==1)
            {
                *frame_len = make_dlms_aarq(frame,read_params->meter_doc.meter_no,0,0);
                read_params->dlms_SSS =1;
                read_params->dlms_RRR =1;
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms aarq  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
               read_params->login_try_count ++;
            }
            else if(read_params->read_ctrl_more_frame)
            {
                *frame_len = make_dlms_rr(frame,read_params->meter_doc.meter_no,read_params->dlms_RRR);
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms rr  %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                read_params->login_try_count ++;
            }
            return TRUE;
        }
        else if(read_params->read_ctrl_state == 2)
        {
            *frame_len = make_dlms_disc(frame,read_params->meter_doc.meter_no);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"dlms disc");
            debug_println_ext(info);
            #endif
            read_params->login_try_count = 0;
            return TRUE;
        }
      }

    }
    #endif   
    #ifdef __POWER_CTRL__
    spot_idx = bin2_int16u(read_params->meter_doc.spot_idx); //������⣬Э������е���������ֱ���жϸò������Ƿ������塣
    is_plus = is_pulse_meter(spot_idx);
    #endif

    for(idx=0;idx<sizeof(CUR_DATA_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        if(CUR_DATA_PHY_LIST[idx].offset != last_read_phy_offset)  /*���ƫ�ƶ����ˣ�˵����ǰ��Ӧ��������϶�Ҫ�ж��Ƿ񳭶������û�䣬����֮ǰ��ƫ�Ʊȶ�*/
        {
            last_read_phy_block_offset = 0;
        }
        
        for(idx_sub=0;idx_sub<((CUR_DATA_PHY_LIST[idx].flag&0x1F)+1);idx_sub++)
        {
            if(get_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,mask_idx))
            {
                get_phy_data((READ_WRITE_DATA*)&(CUR_DATA_PHY_LIST[idx]),idx_sub,&phy);
                
                #ifdef __READ_OOP_METER__
                if((parall_mode == 0xAA)||(read_params->meter_doc.protocol == GB_OOP))
                #else
                if(parall_mode == 0xAA )
                #endif
                {
                    if(CUR_DATA_PHY_LIST[idx].offset == last_read_phy_offset)   //����洢һ�����ڶ����Ȳ�Ҫ��
                    {
                        if(CUR_DATA_PHY_LIST[idx].block_offset == last_read_phy_block_offset)
                        {                            
                            mask_idx++;
                            continue ;
                        }
                        last_read_phy_block_offset = CUR_DATA_PHY_LIST[idx].block_offset;
                    }
                    last_read_phy_offset = CUR_DATA_PHY_LIST[idx].offset;
                }
                else
                {

                }
                read_cur_flag = TRUE;

                if(READPORT_PLC == read_params->meter_doc.baud_port.port) 
                {    
                    //�����������ز����쳭���������ٳ�����
                    flag = readdata_cur_data(meter_idx,&phy,rec_datetime,frame,frame_len,TRUE);
                    #ifdef __PLC_EVENT_READ__
                    if( read_params->meter_doc.meter_class.value == 0x00)flag = 0;
                    #endif
                    #ifdef __FUJIAN_CURRENT_BREAK__
                    if( read_params->meter_doc.protocol == FUJIAN_BREAKER_MONITOR)
                    {
                        flag = 0;
                    }
                    #endif
                    #ifdef __PROVICE_JIANGXI__
                    if((datetime[MINUTE] % 15) == 0)
                    {
                        if(portContext_plc.router_base_info.router_info1.comm_mode == 2)
                        {
                            flag = 0;/*�����Ҫ���е�ǰ����ִ�У���Ҫ�ϱ�F129��*/
                        }
                    }
                    #endif
                    #ifdef __INDONESIA_DLMS_TEST__
                     flag = 0;/*�����ݣ�ҲҪ����һ�鵱ǰ���ݣ�һ��Ҫ��3-4��*/
                    #endif
                    if(TRUE == flag)
                    {
                        //
                        #ifdef __HUNAN_NEW_RECORDING__//�ݶ������ۼ�ʱ��
                        #if ( defined __PROVICE_SHAANXI__ || defined __PROVICE_SICHUAN__ )
                        if(ABC_DX_LJSJ  == phy.phy)
                        {
                            cur_section = (datetime[HOUR]-7)/4;
                            save_section = (rec_datetime[1]-7)/4;
                            //Сʱ��ͬ��ͬʱ������Ҳ��ͬ
                            if( (cur_section == save_section) && (rec_datetime[4] == datetime[YEAR]) && (rec_datetime[3] == datetime[MONTH]) && (rec_datetime[2] == datetime[DAY]) )
                            {
                                /*ֻҪ��һ������EE��˵����2007-09��4�����������*/
                                if((check_is_all_ch(frame,*frame_len,0xEE)==TRUE) || (check_is_all_ch(frame,*frame_len,0xFF)==TRUE))
                                {
                                    read_cur_flag = TRUE;
                                }
                                else
                                {
                                     read_cur_flag = FALSE;
                                }
                            }

                        }
                       // else
                        #else                        
                        if( (SY_ZLJ_SJ == phy.phy ) || (A_SY_ZLJ_SJ == phy.phy ) || (B_SY_ZLJ_SJ == phy.phy ) || (C_SY_ZLJ_SJ == phy.phy ) )
                        {
                            //7 11 15 19 ����ʱ��Ϊ22:59:59
                            if( (datetime[HOUR]>=7) && (datetime[HOUR]<= 22) )
                            {
                            //
                                cur_section = (datetime[HOUR]-7)/4;
                                save_section = (rec_datetime[1]-7)/4;
                                //Сʱ��ͬ��ͬʱ������Ҳ��ͬ
                                if( (cur_section == save_section) && (rec_datetime[4] == datetime[YEAR]) && (rec_datetime[3] == datetime[MONTH]) && (rec_datetime[2] == datetime[DAY]) )
                                {
                                    //�Ѿ��������ˣ�����Ҫ������
                                    read_cur_flag = FALSE;
                                }
                            }
                        }
                        else
                        {
                            if( (rec_datetime[4] == datetime[YEAR]) && (rec_datetime[3] == datetime[MONTH]) && (rec_datetime[2] == datetime[DAY]) )
                            {
                                //���쳭���Ͳ��ٳ����ˡ�
                                read_cur_flag = FALSE;
                            }
                        }                        
                        #endif
                        #elif (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
                        if(14 == read_params->meter_doc.meter_class.user_class)
                        {
                            // 03110000 �����ܴ���  03050000 ʣ����������ܴ���  03060000 ʣ�����������·�����ܴ���
                            if( (phy.phy == QSY_ZCS_ZLJ_SJ) || (phy.phy == FZDY_SD_ZCS_ZLJ_SJ) || (phy.phy == DD_ZCS) 
                               || ((phy.phy >= S1_10C_DD_FSSK_JSSK_SJK-0x3F) && (phy.phy <= S1_10C_DD_FSSK_JSSK_SJK)) 
                               || ((phy.phy >= S1C_QSY_FSSK_DLZ_JSSK-0x3F) && (phy.phy <= S1C_QSY_FSSK_DLZ_JSSK))
                               || ((phy.phy >= S1C_FZDY_SD_FSSK_JSSK-0x3F) && (phy.phy <= S1C_FZDY_SD_FSSK_JSSK)))
                            {
                                //
                                if( (datetime[HOUR]>=8) && (datetime[HOUR]<= 23) )
                                {
                                    //
                                    cur_section = (datetime[HOUR]-8)/4;
                                    save_section = (rec_datetime[1]-8)/4;
                                    //Сʱ������������ͬ��ͬʱ������Ҳ��ͬ
                                    if( (cur_section == save_section) && (rec_datetime[4] == datetime[YEAR]) && (rec_datetime[3] == datetime[MONTH]) && (rec_datetime[2] == datetime[DAY]) )
                                    {
                                        //�Ѿ��������ˣ�����Ҫ������
                                        read_cur_flag = FALSE;
                                    }
                                }
                            }
                            else
                            {
                                if( (rec_datetime[4] == datetime[YEAR]) && (rec_datetime[3] == datetime[MONTH]) && (rec_datetime[2] == datetime[DAY]) )
                                {
                                    //���쳭���Ͳ��ٳ����ˡ�
                                    read_cur_flag = FALSE;
                                }
                            }
                        }
                        else
                        {
                            //
                            if( (rec_datetime[4] == datetime[YEAR]) && (rec_datetime[3] == datetime[MONTH]) && (rec_datetime[2] == datetime[DAY]) )
                            {
                                //���쳭���Ͳ��ٳ����ˡ�
                                read_cur_flag = FALSE;
                            }
                        }
                        #else//������������  TODO ????
                        if( (rec_datetime[4] == datetime[YEAR]) && (rec_datetime[3] == datetime[MONTH]) && (rec_datetime[2] == datetime[DAY]) )
                        {
                            //���쳭���Ͳ��ٳ����ˡ�
                            read_cur_flag = FALSE;
                        }
                        #endif
                    }
                }
                if(TRUE == read_cur_flag)
                {
                    //��Ҫ����
                    if(get_data_item(phy.phy,read_params->meter_doc.protocol,&library))
                    {
                        if(library.item != 0xFFFFFFFF)
                        {

                            if(parall_mode == 0xAA)
                            {
                                int32u2_bin(phy.phy,read_params->phy+need_read_item_no*4);
                                int32u2_bin(library.item,read_params->item+need_read_item_no*4);
                            }
                            else
                            {
                                int32u2_bin(phy.phy,read_params->phy);
                                int32u2_bin(library.item,read_params->item);
                            }

                            read_params->resp_byte_num = 40;
                            read_params->read_type = READ_TYPE_CUR_DATA;
                            if((read_params->meter_doc.protocol == GB645_1997) ||(read_params->meter_doc.protocol == GUANGXI_V30)  ||
                                (read_params->meter_doc.protocol == GB645_1997_JINANGSU_4FL) || (read_params->meter_doc.protocol == GB645_1997_JINANGSU_2FL)
                                   || (is_plus))
                            {
                                *frame_len = make_gb645_1997_frame(frame,read_params->meter_doc.meter_no,0x01,(INT16U)(library.item),NULL,0);
                            }
                            #ifdef __READ_DLMS_METER__
					    	else if((read_params->meter_doc.protocol == METER_DLMS))
                            {
                                if(read_params->dlms_login_ok == 0)
                                 {
                                    int32u2_bin(0,read_params->phy);
                                    read_params->dlms_login_ok = 1;
                                    goto
                                        CHECK_LOGIN;
                                 }
                                *frame_len = make_dlms_read_energy(frame,read_params->meter_doc.meter_no,(INT8U*)library.item,read_params->dlms_SSS,read_params->dlms_RRR,7);
                                read_params->dlms_SSS ++;
                                read_params->dlms_RRR ++;
                            }
                            #endif
                            #ifdef __POWER_CTRL__
                            else if(read_params->meter_doc.protocol == 0)
                            {
                                
                            }
                            #endif
                            else if(read_params->meter_doc.protocol == GB645_2007)
                            {
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #ifdef __READ_EDMI_METER__
                            else if(read_params->meter_doc.protocol == METER_EDMI)
                            {
                                *frame_len = edmi_read_ctrl(read_params,&(library.item),1,frame,0,NULL);
                            }
                            #endif
                            #ifdef __READ_HENGTONG_METER__
                            else if((read_params->meter_doc.protocol == PROTOCOL_HENGTONG_OLD) || (read_params->meter_doc.protocol == PROTOCOL_HENGTONG_NEW))
                            {
                                 //library.len = 4;//������
                                 *frame_len = rs485_hengt_query(read_params->meter_doc.meter_no , library.item, library.len, frame); //�������ʲô��˼��
                            }
                            #endif
                            #ifdef __JIANGSU_READ_CURRENT_MONITOR__
                            else if(read_params->meter_doc.protocol == BREAKER_MONITOR)
                            {
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #endif
                            #ifdef __FUJIAN_CURRENT_BREAK__
                            else if(read_params->meter_doc.protocol == FUJIAN_BREAKER_MONITOR)
                            {
                                 if(library.item == 0x0202FF00)
                                 {
                                    library.item = 0xEA0202FF;
                                 }
                                 int32u2_bin(library.item,read_params->item);
                                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            #endif
                            #ifdef __READ_OOP_METER__
                            else if(read_params->meter_doc.protocol == GB_OOP)
                            {
                                if(SJ == library.phy)//oop������ʱ��ֻ��Ҫ��һ��
                                {
                                    clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,mask_idx);
                                    mask_idx++;
                                    continue;
                                }
                                
                                oad_byte[0] = library.item>>24;
                                oad_byte[1] = library.item>>16;
                                oad_byte[2] = library.item>>8;
                                oad_byte[3] = library.item;
                                
                                if(0 == oad_count)
                                {
                                    read_params->oad_cnt = 0;
                                }
                                if(library.item == 0xFFFFFFFF)
                                {
                                    clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,mask_idx);
                                    mask_idx++;
                                    continue;
                                }
                                mem_cpy(read_params->phy_bak+oad_count*4, (INT8U *)&library.phy, 4);
                                mem_cpy(read_params->oad+oad_count*4, oad_byte, 4);
                                oad_count++;
                                if(oad_count < PARALL_MAX_OAD)
                                {
                                    mask_idx++;
                                    continue;
                                }
                                *frame_len = make_oop_read_frame(frame,read_params->meter_doc.meter_no,read_params->oad,oad_count,0xFFFF,1);
                                read_params->oad_cnt = oad_count;
                                return TRUE;
                            }
                            #endif
                            #ifdef __READ_IEC1107_METER__
                            else if((read_params->meter_doc.protocol == IEC1107))
                            {
                                *frame_len = iec1107_read_ctrl(read_params,frame,0,NULL);
//                                 *frame_len = 0;//make_iec1107_pro_read(frame,(INT8U*)library.item,library.len);
//                                 clr_bit_value(read_meter_flag_cur_data,READ_FLAG_BYTE_NUM,meter_idx);
                            }
                            #endif
                            else
                            {
                                clr_bit_value(read_meter_flag_cur_data,READ_FLAG_BYTE_NUM,meter_idx);
                                return FALSE;
                            }
                            #ifdef __SOFT_SIMULATOR__
                            snprintf(info,100,"*** prepare item cur data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
                                    bin2_int16u(read_params->meter_doc.meter_idx),library.item,phy.phy,mask_idx);
                            debug_println_ext(info);
                            #endif
                            read_params->cur_mask_idx = mask_idx;

                            if((parall_mode == 0xAA) &&((read_params->meter_doc.protocol == GB645_2007)
                            || (read_params->meter_doc.protocol == GB645_1997)))
                            {
                                mem_cpy(gb_645_zuhe_frame+zuhe_frame_len,frame, *frame_len );
                                zuhe_frame_len += *frame_len;
                                need_read_item_no ++ ;

                                if(need_read_item_no >= (portContext_plc.parall_max_item)) /*һ����������645֡��*/
                                {
                                   mem_cpy(frame,gb_645_zuhe_frame,zuhe_frame_len);
                                   *frame_len = zuhe_frame_len;
                                   return TRUE;
                                }
                                else
                                {
                                   //�������������һ�������������ڹ�Լ����û�ж�Ӧ����������´洢��ţ�Ӧ�Ե��״̬���������
                                   if((idx_sub == (CUR_DATA_PHY_LIST[idx].flag&0x1F)) &&(CUR_DATA_PHY_LIST[idx].flag&SAVE_FLAG_BLOCK))
                                   {
                                       writedata_cur_data_seq(meter_idx,&phy);
                                   }
                                   mask_idx++;
                                   continue;
                                }
                            }
                            else
                            {
                                return TRUE;
                            }
                        }
                    }
                }
                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,mask_idx);
                //�������������һ�������������ڹ�Լ����û�ж�Ӧ����������´洢��ţ�Ӧ�Ե��״̬���������
                if((idx_sub == (CUR_DATA_PHY_LIST[idx].flag&0x1F)) &&(CUR_DATA_PHY_LIST[idx].flag&SAVE_FLAG_BLOCK))
                {
                    writedata_cur_data_seq(meter_idx,&phy);
                }
            }
            mask_idx++;
            #ifdef __READ_IEC1107_METER__
            if((read_params->meter_doc.protocol == IEC1107)&&((CUR_DATA_PHY_LIST[idx].flag&0x1F) == idx_sub))
			{
    			if(get_phy_form_list_cur_data(phy.phy,&phy_iec1107,&block_begin_idx,&block_count) != 0xFFFF)
    			{
					//����Ƿ񳭶���ɣ����Ҫ�������
					write_seq_iec1107 = TRUE;
					for(idx_iec1107=0;idx_iec1107<((CUR_DATA_PHY_LIST[idx].flag&0x1F)+1);idx_iec1107++)
					{
						if(get_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,block_begin_idx+idx_iec1107)) write_seq_iec1107 = FALSE;
					}
					if(write_seq_iec1107)
					{
						//writedata_cur_data_seq(meter_idx,&phy);
						writedata_cur_data_seq(meter_idx,&phy_iec1107);
					}
    			}
			}
            #endif
        }
    }
    stat_flag = 1;
    if((read_params->meter_doc.baud_port.port == READPORT_RS485_JC) ||(read_params->meter_doc.baud_port.port == READPORT_RS485_1) ||(read_params->meter_doc.baud_port.port == READPORT_RS485_2)||(read_params->meter_doc.baud_port.port == READPORT_RS485_3))
    {
        idx = get_readport_idx(read_params->meter_doc.baud_port.port);
        if(idx != 0xFF) pRs485Context = &portContext_rs485[idx];
        else stat_flag = 0;
    }
    else stat_flag = 0;
    
    if(stat_flag)
    {
        //��Ҫ��֤�ȵ�ͳ�ƣ��ڵ��¼�
        pcoress_meter_stat_data(pRs485Context);
        spot_idx = bin2_int16u(read_params->meter_doc.spot_idx);
        process_meter_event(spot_idx,read_params->meter_doc.baud_port.port);
        check_rs485_rec_fail_event(pRs485Context);
    }
    else
    {
        if(read_params->meter_doc.baud_port.port == READPORT_PLC) //�ز�������ж�������룬���굱ǰ���һ���¼�����
        {
            spot_idx = bin2_int16u(read_params->meter_doc.spot_idx);
            process_meter_event(spot_idx,read_params->meter_doc.baud_port.port);
        }
    }


    #ifdef __READ_EDMI_METER__
    if(read_params->meter_doc.protocol == METER_EDMI)
    {
        if(TRUE == edmi_to_exit_state(read_params))
        {
            *frame_len = edmi_read_ctrl(read_params,NULL,0,frame,0,NULL);
            return TRUE;
        }
        read_params->read_ctrl_state = 0;
        read_params->read_ctrl_step = 0;
    }
    #endif
    #ifdef __READ_IEC1107_METER__
    if(read_params->meter_doc.protocol == IEC1107)
    {
        if(TRUE == iec1107_to_exit_state(read_params))
        {
            *frame_len = iec1107_read_ctrl(read_params,frame,0,NULL);
            if(*frame_len > 0)
                return TRUE;
        }
    }
    #endif
    #ifdef __READ_OOP_METER__
    if(oad_count>0)
    {
        *frame_len = make_oop_read_frame(frame,read_params->meter_doc.meter_no,read_params->oad,oad_count,0xFFFF,1);
        read_params->oad_cnt = oad_count;
        return TRUE;
    }
    #endif
    if(parall_mode == 0xAA )
    {
        if(need_read_item_no )
        {
            mem_cpy(frame,gb_645_zuhe_frame,zuhe_frame_len);
            *frame_len = zuhe_frame_len;
            return TRUE;
        }
        else
        {
            clr_bit_value(read_meter_flag_cur_data,READ_FLAG_BYTE_NUM,meter_idx);
        }
    }
    else
    {
        clr_bit_value(read_meter_flag_cur_data,READ_FLAG_BYTE_NUM,meter_idx);
    }
    
    return FALSE;
}
//---------------------------------------------------------------------------
INT8U get_phy_form_list_recday_data(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(RECDAY_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((RECDAY_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(RECDAY_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}
INT32U get_recday_data_save_offset(READ_WRITE_DATA *phy,INT8U td[3])
{
    INT32U offset;

    offset = getPassedDays(2000+td[2],td[1],td[0]);
    offset = offset % 31;
    offset *= phy->data_len;
    offset += phy->offset;
    return offset;
}
INT8U writedata_recday_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[3],INT8U *data,INT8U datalen,INT8U* buffer)
{
    INT32U offset;

    offset = get_recday_data_save_offset(phy,td);

    fread_array(meter_idx,offset,buffer,phy->data_len);

    //����ʱ��
    if(compare_string(buffer,td,3) != 0)
    {
        mem_set(buffer,phy->data_len,0xFF);
        mem_cpy(buffer,td,3);
        mem_cpy(buffer+3,datetime+MINUTE,5);
    }

    if (phy->flag & SAVE_FLAG_XL_TIME)
    {
        (void)format_data_save(0,data,datalen,buffer+8,phy);
    }
    else
    {
        datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
        mem_cpy(buffer+8+phy->block_offset,data,datalen);
    }

    fwrite_array(meter_idx,offset,buffer,phy->data_len);

    return TRUE;
}
INT8U save_recday_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    INT8U idx;

    idx = get_phy_form_list_recday_data(bin2_int32u(read_params->phy),&phy);
    if(idx == 0xFF) return 0;
    if (phy.flag & SAVE_FLAG_DENY_NO_SAVE)
    {
        if(check_is_all_ch(frame,frame_len,0xEE)) //����
        {
            //����msak
            //clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_CYCLE_DAY,idx);
        }
        else
        {
            writedata_recday_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,datetime+DAY,frame,frame_len,buffer);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** save recday data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d-%02d",
                    bin2_int16u(read_params->meter_doc.meter_idx),
                    bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
                    read_params->day_hold_td[2],read_params->day_hold_td[1],read_params->day_hold_td[0]);
            debug_println_ext(info);
            #endif
        }
    }
    else
    {
        writedata_recday_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,datetime+DAY,frame,frame_len,buffer);
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** save recday data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d-%02d ",
                bin2_int16u(read_params->meter_doc.meter_idx),
                bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
                read_params->day_hold_td[2],read_params->day_hold_td[1],read_params->day_hold_td[0]);
        debug_println_ext(info);
        #endif
    }
    //clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
    return 0;
}
INT8U readdata_recday_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[3],INT8U rec_datetime[5],INT8U *data,INT8U *datalen)
{
    INT32U offset;

    offset = get_recday_data_save_offset(phy,td);

    fread_array(meter_idx,offset,data,phy->data_len);
    if ((td[2] == data[2]) && (td[1] == data[1]) && (td[0] == data[0]))
    {
        mem_cpy(rec_datetime,data+3,5);
        if(check_is_all_ch(data+8+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+8+phy->block_offset,phy->block_len);
            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}
INT16S app_readdata_recday_data(INT16U meter_idx,INT32U phy_id,INT8U td[3],INT8U rec_datetime[5],INT8U *data,INT8U max_datalen,INT8U port)
{
    READ_WRITE_DATA phy;
    INT8U idx,datalen;
    INT8U td_bin[3],td_tmp[3];

    datalen = 0;
	mem_set(td_tmp,sizeof(td_tmp),0x00);

    if(port == COMMPORT_PLC)
    {
        #ifdef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
        mem_cpy(td_bin,td,3);
        previous_monthhold_td_BCD(td_bin+1,td_bin+1);//����ȥ10���Ƶ�td���� app_readdata_cycle_month��ת����bin
        datalen = app_readdata_cycle_month(meter_idx,phy_id,td_bin+1,rec_datetime,data,max_datalen);

        #else
        idx = get_phy_form_list_cycle_day(phy_id,&phy);//�ز�ֱ�Ӵ��ն���ȡֵ
        if(idx == 0xFF) return -1;
        if (phy.data_len > max_datalen) return -1;
    
        mem_cpy(td_tmp,td,3);
        to_byte(td_tmp,3);
        date_minus_days(td_tmp+2,td_tmp+1,td_tmp,1);
        //    to_bcd(td_tmp,3);
        /*
        td_bin[0] = BCD2byte(td[0]);
        td_bin[1] = BCD2byte(td[1]);
        td_bin[2] = BCD2byte(td[2]);
        */
        if( FALSE == readdata_cycle_day(meter_idx,&phy,td_tmp,rec_datetime,data,&datalen,NULL) )
        {
            //coverity��麯������ֵ���⣬�����±�֤���澯
            datalen = 0;
        }
        #endif
    }
    else

    {
        idx = get_phy_form_list_recday_data(phy_id,&phy);
        if(idx == 0xFF) return -1;
        if (phy.data_len > max_datalen) return -1;
    
        td_bin[0] = BCD2byte(td[0]);
        td_bin[1] = BCD2byte(td[1]);
        td_bin[2] = BCD2byte(td[2]);
        if(FALSE == readdata_recday_data(meter_idx,&phy,td_bin,rec_datetime,data,&datalen) )
        {
            //coverity��麯������ֵ���⣬�����±�֤���澯
            datalen = 0;
        }
    }
    return datalen;
}

//---------------------------------------------------------------------------
//�ճ�ֵ��ȡ����
INT16U get_phy_from_list_day_init_data(INT32U phy,READ_WRITE_DATA* out,INT16U *block_begin_idx,INT8U *block_count)
{
    INT8U idx,idx_sub;
    INT16U mask_idx;

    mask_idx = 0;
    *block_begin_idx = 0;
    *block_count = 0;

    for(idx=0;idx<sizeof(DAY_INIT_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        *block_begin_idx = mask_idx;
        *block_count = (DAY_INIT_PHY_LIST[idx].flag&0x1F)+1 ;
        for(idx_sub=0;idx_sub<((DAY_INIT_PHY_LIST[idx].flag&0x1F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(DAY_INIT_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFFFF;
}
INT8U writedata_day_init_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U *buffer,INT8U rec_datetime[5])
{
    INT32U offset;
    //INT8U midu;

    //�����С��λ��д����
    offset = get_last_data_save_offset_data(meter_idx,phy,buffer,FALSE); 

    //����ʱ��
    mem_cpy(buffer,rec_datetime,5);
    //��������
    if (phy->flag & SAVE_FLAG_XL_TIME)
    {
        datalen = format_data_save(0,data,datalen,buffer+9,phy);
    }
    else
    {
        datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
        mem_cpy(buffer+5+phy->block_offset,data,datalen);
    }
    if(phy->phy == 0x000058C0)
    {
         mem_cpy(buffer+5+phy->block_offset+datalen,datetime,6);
    }
    fwrite_array(meter_idx,offset+4,buffer,phy->data_len-4);
    return TRUE;
}
INT8U writedata_day_init_data_seq(INT16U meter_idx,READ_WRITE_DATA *phy)
{
    return writedata_cur_data_seq(meter_idx,phy);
}
INT8U readdata_day_init_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U rec_datetime[5],INT8U *data,INT8U *datalen,BOOLEAN flag)
{
    //INT32U offset;   
    #ifdef __POWER_CTRL__
    INT16U spot_idx;
    INT8U tmp[10]={0}; //midu,
    #endif
   

    #ifdef __POWER_CTRL__
    fread_meter_params(meter_idx,PIM_METER_DOC,tmp,10);
    spot_idx = bin2_int16u(tmp+2);
 
    if(is_pulse_meter(spot_idx))
    {
        if(phy->phy == 0x0000007F)
        {
            //phy->data_len = 39;
            phy->block_len = 25;
        }
    }
    #endif

    (void)get_last_data_save_offset_data(meter_idx,phy,data,flag); 


    mem_cpy(rec_datetime,data,5);
    if(check_is_all_ch(data+5+phy->block_offset,phy->block_len,0xFF))
    {
        *datalen = 0;
        mem_set(data,phy->block_len,0xFF);
        return FALSE;
    }
    else
    {
        *datalen = phy->block_len;
        mem_cpy(data,data+5+phy->block_offset,phy->block_len);
        return TRUE;
    }
}
INT16S app_readdata_day_init_data(INT16U meter_idx,INT32U phy_id,INT8U rec_datetime[5],INT8U *data,INT8U max_datalen,BOOLEAN flag)
{
    READ_WRITE_DATA phy;
    INT16U idx;
    INT16U block_begin_idx = 0;
    INT8U datalen = 0;
    INT8U block_count = 0;
    //INT8U midu;

	mem_set((void *)&phy,sizeof(READ_WRITE_DATA),0x00);
    idx = get_phy_from_list_day_init_data(phy_id,&phy,&block_begin_idx,&block_count);
    if (idx == 0xFFFF) return -1;
    if (phy.data_len > max_datalen) return -1;
    readdata_day_init_data(meter_idx,&phy,rec_datetime,data,&datalen,flag);
    return datalen;
}
//û�п鳭����ת����������ĸ��� //������Ҫ�е�����ȡ�ģ�Ҫô��������д�����ġ�Ҫô�����־λ�����յ�ʱ����λһ�Ρ�
INT8U save_day_init_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    INT16U meter_idx,idx,block_begin_idx;
    INT8U block_count;
    BOOLEAN write_seq;

    idx = get_phy_from_list_day_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count);
    if(idx == 0xFFFF) return 0;

    if((read_params->meter_doc.protocol == 0)&&(phy.phy == 0x0000007F))
    {
        //phy.data_len = 39;
        phy.block_len = 25;
    }

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if(((phy.phy & 0x000000FF) == 0x3F) 
        || ((phy.phy & 0x000000FF) == 0x7F) 
        || ((phy.phy & 0x000000FF) == 0xBF) 
        || ((phy.phy & 0x000000FF) == 0xFF)) //�ǿ������� 
    {
        if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
        {
            clr_bit_value(read_params->read_mask.day_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
            //����msak
           // clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,idx);
        }
        else
        {
            writedata_day_init_data(meter_idx,&phy,frame,frame_len,buffer,read_params->init_data_rec_time);
            writedata_day_init_data_seq(meter_idx,&phy);
            for(idx=0;idx<block_count;idx++)
            {
                clr_bit_value(read_params->read_mask.day_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,block_begin_idx+idx);
            }
        }
    }
    else
    {
        if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
        {
            clr_bit_value(read_params->read_mask.day_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
            //����msak
            //clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,idx);
        }
        else
        {
            writedata_day_init_data(meter_idx,&phy,frame,frame_len,buffer,read_params->init_data_rec_time);
            clr_bit_value(read_params->read_mask.day_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
        }
        //����Ƿ񳭶���ɣ����Ҫ�������
        write_seq = TRUE;
        for(idx=0;idx<block_count;idx++)
        {
            if(get_bit_value(read_params->read_mask.day_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,block_begin_idx+idx)) write_seq = FALSE;
        }
        if(write_seq)
        {
            writedata_day_init_data_seq(meter_idx,&phy);
        }
    }

    return 0;
}
void check_day_init_data_depend_fast_index_no_port(INT8U* rec_time)
{
    INT8U port;  
    
    for(port=COMMPORT_485_CY;port<=COMMPORT_485_MIN;port++)
    {
        check_day_init_data_depend_fast_index(port,rec_time);
    }
}
void check_day_init_data_depend_fast_index(INT8U port,INT8U* rec_time)
{
    INT16U meter_count,idx,meter_idx; // file_id,
    INT16S len;
    FAST_INDEX* fast_index;
    INT8U buffer[40],tmp_buf[40];
    INT8U seq,data_len;
    READ_WRITE_DATA phy;
    INT16U block_begin_idx = 0;
    INT8U block_count = 0;
    
    if(false == tpos_mutexRequest(&SIGNAL_FAST_IDX)) return;

    meter_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    if(meter_count == 0)
    {
        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return;
    }


    for(idx=0;idx<meter_count;idx++)
    {
        //mem_cpy(fast_index->value,fast_index_list.fast_index[idx].value,sizeof(FAST_INDEX));
        fast_index = &(fast_index_list.fast_index[idx]);
        //������

        if (fast_index->port > port) break;
        if (fast_index->port != port) continue;

        meter_idx = bin2_int16u(fast_index->seq_spec);
        meter_idx &= FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;
        
        for(seq=0;seq<sizeof(DAY_INIT_PHY_LIST)/sizeof(READ_WRITE_DATA);seq++)
        {
            //phy = (READ_WRITE_DATA*)&(DAY_INIT_PHY_LIST[seq]);
            if (get_phy_from_list_day_init_data(DAY_INIT_PHY_LIST[seq].phy+0x03F,&phy,&block_begin_idx,&block_count) == 0xFFFF) continue;
            
            readdata_day_init_data(meter_idx,&phy,buffer,buffer+5,&data_len,TRUE);

            if(0 != compare_string(buffer+2,rec_time+2,3))
            {
                len = app_readdata_cur_data(meter_idx,phy.phy,buffer,buffer+5,40,TRUE);
                if(len > 0)
                {
                    writedata_day_init_data(meter_idx,&phy,buffer+5,len,tmp_buf,rec_time);
                    writedata_day_init_data_seq(meter_idx,&phy);
                }
            }
        }
   
    }
    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return;
}
//---------------------------------------------------------------------------

 //�³�ֵ��ȡ����
INT16U get_phy_from_list_month_init_data(INT32U phy,READ_WRITE_DATA* out,INT16U *block_begin_idx,INT8U *block_count)
{
    INT8U idx,idx_sub;
    INT16U mask_idx;

    mask_idx = 0;
    *block_begin_idx = 0;
    *block_count = 0;

    for(idx=0;idx<sizeof(MONTH_INIT_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        *block_begin_idx = mask_idx;
        *block_count = (MONTH_INIT_PHY_LIST[idx].flag&0x1F)+1 ;
        for(idx_sub=0;idx_sub<((MONTH_INIT_PHY_LIST[idx].flag&0x1F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(MONTH_INIT_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFFFF;
}
INT8U writedata_month_init_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U *buffer,INT8U rec_datetime[5])
{
    INT32U offset;
    //INT8U midu;

    //�����С��λ��д����
    offset = get_last_data_save_offset_data(meter_idx,phy,buffer,FALSE);

    //����ʱ��
    mem_cpy(buffer,rec_datetime,5);
    //��������

    datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
    mem_cpy(buffer+5+phy->block_offset,data,datalen);

    fwrite_array(meter_idx,offset+4,buffer,phy->data_len-4);
    return TRUE;
}
INT8U writedata_month_init_data_seq(INT16U meter_idx,READ_WRITE_DATA *phy)
{
    return writedata_cur_data_seq(meter_idx,phy);
}
INT8U readdata_month_init_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U rec_datetime[5],INT8U *data,INT8U *datalen,BOOLEAN flag)
{
    //INT32U offset;
    #ifdef __POWER_CTRL__
    INT16U spot_idx;
    //INT8U midu;
    INT8U tmp[10]={0};
    #endif

    #ifdef __POWER_CTRL__
    fread_meter_params(meter_idx,PIM_METER_DOC,tmp,10);
    spot_idx = bin2_int16u(tmp+2);
    
    if(is_pulse_meter(spot_idx))
    {
        if(phy->phy == 0x0000007F)
        {
            //phy->data_len = 39;
            phy->block_len = 25;
        }
    }
    #endif
    (void)get_last_data_save_offset_data(meter_idx,phy,data,flag);


    mem_cpy(rec_datetime,data,5);
    if(check_is_all_ch(data+5+phy->block_offset,phy->block_len,0xFF))
    {
        *datalen = 0;
        mem_set(data,phy->block_len,0xFF);
        return FALSE;
    }
    else
    {
        *datalen = phy->block_len;
        mem_cpy(data,data+5+phy->block_offset,phy->block_len);
        return TRUE;
    }
}
INT16S app_readdata_month_init_data(INT16U meter_idx,INT32U phy_id,INT8U rec_datetime[5],INT8U *data,INT8U max_datalen,BOOLEAN flag)
{
    READ_WRITE_DATA phy;
    INT16U idx,block_begin_idx;
    INT8U datalen;
    INT8U block_count;
    //INT8U midu;

    idx = get_phy_from_list_month_init_data(phy_id,&phy,&block_begin_idx,&block_count);
    if (idx == 0xFFFF) return -1;
    if (phy.data_len > max_datalen) return -1;
    readdata_month_init_data(meter_idx,&phy,rec_datetime,data,&datalen,flag);
    return datalen;
}
//û�п鳭����ת����������ĸ��� //������Ҫ�е�����ȡ�ģ�Ҫô��������д�����ġ�Ҫô�����־λ�����յ�ʱ����λһ�Ρ�
INT8U save_month_init_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    INT16U meter_idx,idx,block_begin_idx;
    INT8U block_count;
    BOOLEAN write_seq;

    idx = get_phy_from_list_month_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count);
    if(idx == 0xFFFF) return 0;

     if((read_params->meter_doc.protocol == 0)&&(phy.phy == 0x0000007F))
    {
        //phy.data_len = 39;
        phy.block_len = 25;
    }

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if(((phy.phy & 0x000000FF) == 0x3F) 
        || ((phy.phy & 0x000000FF) == 0x7F) 
        || ((phy.phy & 0x000000FF) == 0xBF) 
        || ((phy.phy & 0x000000FF) == 0xFF)) //�ǿ������� 
    {
        if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
        {
            clr_bit_value(read_params->read_mask.month_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
            //����msak
           // clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,idx);
        }
        else
        {
            writedata_month_init_data(meter_idx,&phy,frame,frame_len,buffer,read_params->init_data_rec_time);
            writedata_month_init_data_seq(meter_idx,&phy);
            for(idx=0;idx<block_count;idx++)
            {
                clr_bit_value(read_params->read_mask.month_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,block_begin_idx+idx);
            }
        }
    }
    else
    {
        if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
        {
            clr_bit_value(read_params->read_mask.month_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
            //����msak
            //clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,idx);
        }
        else
        {
            writedata_month_init_data(meter_idx,&phy,frame,frame_len,buffer,read_params->init_data_rec_time);
            clr_bit_value(read_params->read_mask.month_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
        }
        //����Ƿ񳭶���ɣ����Ҫ�������
        write_seq = TRUE;
        for(idx=0;idx<block_count;idx++)
        {
            if(get_bit_value(read_params->read_mask.month_init_data,READ_MASK_BYTE_NUM_DAY_HOLD,block_begin_idx+idx)) write_seq = FALSE;
        }
        if(write_seq)
        {
            writedata_month_init_data_seq(meter_idx,&phy);
        }
    }

    return 0;
}

//---------------------------------------------------------------------------
BOOLEAN prepare_read_item(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    #ifdef __F30_STOP_READ__
    if ((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
        if (read_params->meter_doc.is_allow_read) return FALSE;
    }
    #endif

    read_params->control.rec_delay_flag = 0;
    
    #if defined(__PROVICE_JIANGSU__)
    if(prepare_read_cjq_update_task(read_params,frame,frame_len,TRUE))  return TRUE;
    #endif
    
    #ifdef __ENABLE_ESAM2__
    if (prepare_exec_batch_meter_task(read_params,frame,frame_len)) return TRUE;
    #endif

    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    if (prepare_batch_transparent_meter_task(read_params,frame,frame_len)) return TRUE;
    #endif
    #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
    #ifdef __GW_CYCLE_TASK__ /*����ǰ�� �������ȼ����� */
    if (prepare_batch_transparent_meter_cycle_task(read_params,frame,frame_len)) return TRUE;
    #endif
    #endif
     #ifdef __ISRAEL_CAST_SET_METER_TASK__
    if(portContext_plc.plc_cast_task.b.taskflag == PLCCASTTASK_EXEC)
    {
       get_plc_cast_statinfo(NULL);
    }
    if (prepare_israel_cast_set_meter_task(read_params,frame,frame_len)) return TRUE;
    #endif

    #ifdef __METER_EVENT_REPORT__
    if(read_params->meter_doc.protocol != 0)
    {
        if (prepare_plc_read_report_meter_event_state(read_params,frame,frame_len)) return TRUE;
        /* ��չ�¼� */
        if (prepare_plc_read_report_meter_ext_event_state(read_params,frame,frame_len))
        {
            return TRUE;
        }
    }
    #endif

    #ifdef __HOUR_CURVE_READ_SELF_ADAPTION__
    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
        if (prepare_read_item_curve(read_params,frame,frame_len)) return TRUE;
		#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
		if (prepare_read_item_curve_hunan(read_params,frame,frame_len)) return TRUE;
		#endif
		/* ֻ����Կ�� ���� */
        if( (portContext_plc.router_base_info.router_info1.comm_mode == 2) 
         && (CYCLE_REC_MODE_PARALLEL == portContext_plc.plc_other_read_mode) )
        {
            if(portContext_plc.active_read_curve_one_cycle != 0)
            {
                return FALSE;
            }
        }
    }    
    #endif

    #if ((defined __PROVICE_GUANGXI_PB__) || (defined __GUANGXI_V3__) || (defined __ALL_MONTH_DATA_FROM_JSR_DATA__))
    if (prepare_read_item_cycle_month(read_params,frame,frame_len)) return TRUE;
    if (prepare_read_item_cycle_day(read_params,frame,frame_len))  return TRUE;
    #else
    if (prepare_read_item_cycle_day(read_params,frame,frame_len))  return TRUE;

    if ((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
        if((portContext_plc.plc_other_read_mode == CYCLE_REC_MODE_PARALLEL)
           || (portContext_plc.router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_CONCENTRATOR)
           || (portContext_plc.router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ALL))
        {
            if(portContext_plc.concentrator_read_cycle_no < 3) return FALSE;  //�������ȼ�
        }
    }

    if (prepare_read_item_cycle_month(read_params,frame,frame_len)) return TRUE;
    #endif

    #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
    #ifndef __GW_CYCLE_TASK__
    if (prepare_batch_transparent_meter_cycle_task(read_params,frame,frame_len)) return TRUE;
    #endif
    #endif

    //������485����˳��
    #ifndef __HOUR_CURVE_READ_SELF_ADAPTION__
    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
        #ifndef __ISRAEL__SIMPLE_TEST__
        if (prepare_read_item_curve(read_params,frame,frame_len)) return TRUE;
        #endif
		#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
		if (prepare_read_item_curve_hunan(read_params,frame,frame_len)) return TRUE;
		#endif
    }
    #endif

    #ifdef __PROVICE_CHONGQING__
    INT16U meter_cnt = fast_index_list.count;
    /* ̨���� 50����ʱ��
     * ȫ�¼����Ը�ʱ��̫�̣���ֹ�г�ȥ����û���᳭���������Ƴ����ն������ʧ���� */
    if(meter_cnt > 100) 
    #endif
    {
        if (get_bit_value(read_priority_ctrl_cur_data,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx)))
        {
            read_params->is_read_priority = 1;
            clr_bit_value(read_priority_ctrl_cur_data,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
        }
    }

	//�ŵ�ʵʱ����ǰ��Ӧ��̨�������⡣
	if (prepare_plc_cycle_meter_event_item(read_params,frame,frame_len))  return TRUE;
    if (prepare_read_item_cur_data(read_params,frame,frame_len)) return TRUE;
    else
    {
        //ת��F246����
        #ifndef __POWER_CTRL__
        if(is_read_power_on_off(bin2_int16u(read_params->meter_doc.meter_idx)))
        #endif
        {
            save_day_hold_power_on_off(read_params);
        }
    }
	#ifdef __METER_DAY_FREEZE_EVENT__
	if(prepare_plc_meter_dayfreeze_event_item(read_params,frame,frame_len))
    {
        return TRUE;
    }
	#endif
    //if (prepare_plc_cycle_meter_event_item(read_params,frame,frame_len))  return TRUE;
    if((read_params->meter_doc.baud_port.port != COMMPORT_PLC) && (read_params->meter_doc.baud_port.port != COMMPORT_PLC_REC))
    {
        if(prepare_read_item_curve(read_params,frame,frame_len)) return TRUE;
    }

  //  if(check_const_ertu_switch(CONST_ERTU_SWITCH_PATCH_DAYHOLD_DATA))
    {
        if (get_bit_value(read_priority_ctrl_patch_day_hold,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx)))
        {
            read_params->is_read_priority_ctrl = 1;
            clr_bit_value(read_priority_ctrl_patch_day_hold,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
        }
        if (prepare_read_item_patch_day_hold(read_params,frame,frame_len)) return TRUE;
        if (prepare_read_wait_cycle_day(read_params,frame,frame_len)) return TRUE;
        #ifdef __CHECK_MONTH_HOLD_TD__
        if (prepare_read_wait_cycle_month(read_params,frame,frame_len)) return TRUE;    
        #endif
    }
    /*�ſ� Ҫ��Ҫ���� ͨ��flag ��mask ���� */
   #ifdef __SICHUAN_FK_PATCH_CURVE_DATA__
   if (prepare_read_item_last_curve_edmi_iec(read_params,frame,frame_len))  return TRUE; //�������ݲ�������������˼·��
   #else
   if (prepare_read_item_last_curve_cycle_day(read_params,frame,frame_len))  return TRUE; //�������ݲ���
   #endif


    #ifdef __VOLTAGE_MONITOR__
    if (prepare_read_voltage_monitor(read_params,frame,frame_len))  return TRUE;
    #endif
    #ifdef __PRECISE_TIME__
    if(prepare_plc_read_meter_precise_time(read_params,frame,frame_len,0,0))  return TRUE;
    #endif
    #ifdef __INSTANT_FREEZE__
    if(prepare_plc_read_meter_instant_freeze(read_params,frame,frame_len))  return TRUE;
    #endif
    #ifdef __PROVICE_JIANGSU__
//    if(prepare_plc_cjq_read_item(read_params,frame,frame_len))  return TRUE; //�����⣬�ȹر�
    #endif
    #ifdef __PROVICE_SHAANXI_CHECK__
    if(prepare_meter_type_read_item(read_params,frame,frame_len))  return TRUE; //�����жϵ��Э��
    #endif
//    #ifdef __READ_IEC1107_METER__
//	if((read_params->meter_doc.protocol==IEC1107) && ((read_params->read_ctrl_state==2)))//||(read_params->read_ctrl_state==5))
//    {
//        *frame_len = make_iec1107_break_message(frame);
//        #ifdef __SOFT_SIMULATOR__
//        snprintf(info,100,"make_iec1107_break_message");
//        debug_println_ext(info);
//        #endif
//        read_params->read_ctrl_state=3;//��������
//        read_params->read_ctrl_step=0;
//        return TRUE;
//    }
//    #endif
    return FALSE;
}
//---------------------------------------------------------------------------
BOOLEAN check_read_meter_flag(INT16U meter_idx)
{
    if (get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
    if (get_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
    if (get_bit_value(read_meter_flag_cur_data,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
    if (get_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
    if (get_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
    if (get_bit_value(read_meter_flag_wati_day_hold,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
#ifdef __ENABLE_ESAM2__
    if (get_bit_value(read_meter_flag_esam,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
#endif
	//if(get_bit_value(read_meter_grade_flag,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
	if(get_bit_value(read_meter_grade_flag_level_1,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
	if(get_bit_value(read_meter_grade_flag_level_2,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
	if(get_bit_value(read_meter_grade_flag_level_3,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
	if(get_bit_value(read_meter_grade_flag_level_4,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;

//        if(check_cjq_meter_more_data(meter_idx)) return true;   //�ɼ����µĵ���Ƿ��������������������������������д�˶��ڴ洦��
	#ifdef __METER_DAY_FREEZE_EVENT__
	if (get_bit_value(read_meter_flag_day_freeze_event,READ_FLAG_BYTE_NUM,meter_idx)) return TRUE;
	#endif
    return FALSE;
}
//---------------------------------------------------------------------------
/*+++
  ����:  ���µ�ʳ�����Ϣ
  ������
         INT16U meter_idx,     ������
         INT8U port,           �˿�
         INT8U phase           ������λ��Ϣ����λ��1��ʶLN����
         INT8U act_phase,      ʵ����λ, ��λ��1��ʶ�����
         INT8U relay           �м����
         INT8U plc_q           �ź�Ʒ��
  ���أ�
         ��

---*/
void update_meter_recstate(INT16U meter_idx,INT8U port,INT8U phase,INT8U act_phase,INT8U relay,INT8U plc_q,BOOLEAN canRec)
{
    struct{
        INT8U seq[4];
        INT8U read_date[5];
        METER_READ_INFO meter_read_info;
        INT8U value[27];
    }var;
    INT8U phase_temp=0;
    //INT16U itemDataLen;

    if(meter_idx == 0) return;
    if(meter_idx > MAX_METER_COUNT) return;
    fread_array(meter_idx,PIM_CUR_READ_INFO,var.seq,sizeof(var));
//    if(compare_string(var.seq,datetime+DAY,3) == 0) return;

    mem_cpy(var.seq,datetime+DAY,3);
    mem_cpy(var.read_date,datetime+MINUTE,5);

    phase_temp = var.meter_read_info.phase.value ;
    mem_set(var.value,27,0x00);
    
    if(var.meter_read_info.port != port)
    {
        mem_set(var.meter_read_info.value,sizeof(METER_READ_INFO),0x00);
        phase_temp = 0;
    }
    
    //���µ������Ϣ
    var.meter_read_info.port = port;
    var.meter_read_info.depth = relay;
    if((portContext_plc.router_base_info.router_info1.comm_mode != 2) || (var.meter_read_info.phase.value == 0)) /*���ʹ��10HF31���£����ǿ����ȸ�����λ����ֹ��·�ɲ�֧��10HF31*/
    {

        var.meter_read_info.phase.value = 0x00;

        if((port == COMMPORT_PLC) || (port == COMMPORT_PLC_REC))
        {
            //D7Ϊ1��ʶLN����
            var.meter_read_info.phase.reserved2 = (phase & 0x80) ? 1 : 0;  //1;���죻0��������
        }
        act_phase &= 0x7F;
        switch(act_phase) /*���µĹ���V2.6Э�飬������λ�������ʵ����λ*/
        {
        case 1:    var.meter_read_info.phase.rec_A = 1;   break;
        case 2:    var.meter_read_info.phase.rec_B = 1;   break;
        case 3:    var.meter_read_info.phase.rec_C = 1;   break;
        }
        //D3Ϊ1��ʶ�����
        var.meter_read_info.phase.reserved1 = (act_phase & 0x80) ? 1 : 0;

        /*ʵ����λ���������������²��ˣ���ʱ�����ǣ��԰�֮ǰ�ķ�ʽ�洢����10HF31����*/
        act_phase &= 0x7F;
        switch(act_phase)
        {
        case 1:    var.meter_read_info.phase.act_A = 1;   break;
        case 2:    var.meter_read_info.phase.act_B = 1;   break;
        case 3:    var.meter_read_info.phase.act_C= 1;   break;
        }
        
    }
    else
    {
        var.meter_read_info.phase.value = phase_temp;
    }
    var.meter_read_info.plc_q.value = plc_q;

    #ifdef __PROVICE_ANHUI__
    //����ʹ���������ź�Ʒ�ʣ�������д���ŵ���ʾ���ִ�
    var.meter_read_info.plc_q.resp_q = (var.meter_read_info.plc_q.resp_q == 0) ? 1 : var.meter_read_info.plc_q.resp_q;
    var.meter_read_info.plc_q.send_q = (var.meter_read_info.plc_q.send_q == 0) ? 1 : var.meter_read_info.plc_q.send_q;
    #endif

    if(canRec)
    {
        #ifdef __F170_REC_TIME_IS_FIRST_READ_TIME__ //f170�����һ�γ���ɹ�ʱ��ȡ�����һ�γ����ɹ���ʱ�򣬵���������ʱ�䲻�����ˡ�
          if((var.meter_read_info.last_rec_date[YEAR] != byte2BCD(datetime[YEAR]))
        || (((var.meter_read_info.last_rec_date[MONTH]) & 0x1F) != byte2BCD(datetime[MONTH]))
        || (var.meter_read_info.last_rec_date[DAY] != byte2BCD(datetime[DAY])))
        #endif
        {
        var.meter_read_info.last_rec_date[SECOND] = byte2BCD(datetime[SECOND]);
        var.meter_read_info.last_rec_date[MINUTE] = byte2BCD(datetime[MINUTE]);
        var.meter_read_info.last_rec_date[HOUR] = byte2BCD(datetime[HOUR]);
        var.meter_read_info.last_rec_date[DAY] = byte2BCD(datetime[DAY]);
        var.meter_read_info.last_rec_date[MONTH] = byte2BCD(datetime[MONTH]);
        var.meter_read_info.last_rec_date[YEAR] = byte2BCD(datetime[YEAR]);
        set_DATAFMT_01((DATAFMT_01*)(var.meter_read_info.last_rec_date));
        var.meter_read_info.fail_count = 0;
        var.meter_read_info.rec_flag = 1;
        }
    }
    else
    {
        if((var.meter_read_info.last_fail_date[YEAR] != byte2BCD(datetime[YEAR]))
            || (var.meter_read_info.last_fail_date[MONTH] != byte2BCD(datetime[MONTH]))
            || (var.meter_read_info.last_fail_date[DAY] != byte2BCD(datetime[DAY]))
            || (var.meter_read_info.last_fail_date[HOUR] != byte2BCD(datetime[HOUR]))
            || (var.meter_read_info.last_fail_date[MINUTE] != byte2BCD(datetime[MINUTE])))
        {
            var.meter_read_info.last_fail_date[SECOND] = byte2BCD(datetime[SECOND]);
            var.meter_read_info.last_fail_date[MINUTE] = byte2BCD(datetime[MINUTE]);
            var.meter_read_info.last_fail_date[HOUR] = byte2BCD(datetime[HOUR]);
            var.meter_read_info.last_fail_date[DAY] = byte2BCD(datetime[DAY]);
            var.meter_read_info.last_fail_date[MONTH] = byte2BCD(datetime[MONTH]);
            var.meter_read_info.last_fail_date[YEAR] = byte2BCD(datetime[YEAR]);

            set_DATAFMT_01((DATAFMT_01*)(var.meter_read_info.last_fail_date));

            var.meter_read_info.fail_count ++;
            var.meter_read_info.rec_flag = 0;
            if(var.meter_read_info.fail_count > 200)
            {
                var.meter_read_info.fail_count = 200;
            }
        }
    }
    fwrite_array(meter_idx,PIM_CUR_READ_INFO,var.seq,sizeof(var));
}

INT8U exec_xlost(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
	#ifdef __COMPUTE_XLOST__
    tagXLOSTCALFLAG cal_flag;
	#endif

    portContext = (PLCPortContext*)readportcontext;

    if(check_const_ertu_switch(CONST_ERTU_SWITCH_COMPUTE_XLOST))
    {
        #ifdef __COMPUTE_XLOST__
        fread_array(FILEID_RUN_DATA,FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);
        if(cal_flag.value != 0xFF)
        {
            cal_flag.time_seg = 1;
            fwrite_array(FILEID_RUN_DATA,FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);
            compute_xlost();
        }
        #endif
    }

    portContext->params.value[0] = 0;
    portContext->params.value[1] = 0;
    portContext->cur_plc_task_step = PLC_IDLE_CHECK_ERC_39;
    portContext->OnPortReadData = check_erc_39;
    readport_plc.OnPortReadData = check_erc_39;
    return 0;
}

#ifdef __COMPUTE_XLOST__
//data��ʽ����������4�ֽڣ�+���ȣ�1�ֽڣ�+����
INT16U read_spot_dayhold_data(INT16U meter_seq, INT32U phy, INT8U td[3], INT8U rec_datetime[5], INT8U *data)
{
    INT16U  data_len = 0, buf_len = 0;
    METER_DOCUMENT meter_doc;
    INT8U req[100], resp[100];

    fread_meter_params(meter_seq, PIM_METER_DOC, (INT8U *)&meter_doc, sizeof(METER_DOCUMENT));

    buf_len = sizeof(resp) / sizeof(INT8U);
    mem_set(req, buf_len, 0xEE);
    mem_set(resp, buf_len, 0xEE);
    data_len = read_his_phy(req, resp, phy, meter_seq, td, rec_datetime, buf_len, 0, &meter_doc);
    mem_cpy(data, resp, data_len);

    return data_len;
}

/*+++
���ܣ������ܱ�ķ��๩�����
������
    cal_flag            ��������һЩ��־
    z_supplay_amount    �ܱ�Ľ���Ĺ����������Ѿ�ʹ�ý�������죩
    meter_idx  ���ܱ����
���أ�
������
---*/
void compute_zmeter_supply_amount(tagXLOSTCALFLAG *cal_flag, INT32U z_supply_amount, INT16U meter_seq, METER_DOCUMENT meter_doc, INT32U  *supply_amount_phase)
{
    INT32U  amount_today_phase[3], amount_yestoday_phase[3];
    INT32U  ct;
    INT16U  meter_idx;
    SPOT_PTCT spot_ptct;
    INT8U  td[2][3], for_year, for_month, for_day, rec_datetime[5];
    INT8U  today_data[150], yestoday_data[150], tmp_data[20];
    INT8U  is_valid1, is_valid2, idx;

    //����Ѿ�ȫ����������ն������,ͳ�Ʊ������۵���(�������)
    supply_amount_phase[0] = 0;
    supply_amount_phase[1] = 0;
    supply_amount_phase[2] = 0;

    meter_idx = meter_seq;

    //��ȡCT
    fread_meter_params(meter_idx, PIM_METER_F25, spot_ptct.value, sizeof(SPOT_PTCT));

    get_yesterday(td[0]);
    for_day   = td[0][0];
    for_month = td[0][1];
    for_year  = td[0][2];
    td[0][0] = byte2BCD(for_day);
    td[0][1] = byte2BCD(for_month);
    td[0][2] = byte2BCD(for_year);

    date_minus_days(&for_year,&for_month,&for_day,1);
    td[1][0] = byte2BCD(for_day);
    td[1][1] = byte2BCD(for_month);
    td[1][2] = byte2BCD(for_year);

    //��ȡ����������ʾֵ                                         
    (void)read_spot_dayhold_data(meter_idx, A_ZXYG_DN, td[0], rec_datetime, tmp_data);    //A��0x00000480
    mem_cpy(today_data, tmp_data + 5, 4);
    (void)read_spot_dayhold_data(meter_idx, B_ZXYG_DN, td[0], rec_datetime, tmp_data);    //B��0x000008C0
    mem_cpy(today_data + 4, tmp_data + 5, 4);
    (void)read_spot_dayhold_data(meter_idx, C_ZXYG_DN, td[0], rec_datetime, tmp_data);    //C��0x00000D00
    mem_cpy(today_data + 8, tmp_data + 5, 4);

    //��ȡǰ��������ʾֵ                                         
    (void)read_spot_dayhold_data(meter_idx, A_ZXYG_DN, td[1], rec_datetime, tmp_data);   //0x00000480
    mem_cpy(yestoday_data, tmp_data + 5, 4);
    (void)read_spot_dayhold_data(meter_idx, B_ZXYG_DN, td[1], rec_datetime, tmp_data);   //0x000008C0
    mem_cpy(yestoday_data + 4, tmp_data + 5, 4);
    (void)read_spot_dayhold_data(meter_idx, C_ZXYG_DN, td[1], rec_datetime, tmp_data);   //0x00000D00
    mem_cpy(yestoday_data + 8, tmp_data + 5, 4);

    //�����ȡ�����ܱ�ķ������ʾֵ�����������������ô�ͽ��ܱ���ܵ���������3����Ϊ���������
    if(check_is_all_ch(today_data, 12, 0xEE) && (check_is_all_ch(yestoday_data, 12, 0xEE)))
    {
        //���ú����ط��ѽ��˹�CT�����ﲻ���ٳ�CT
        supply_amount_phase[0] = z_supply_amount / 3;
        supply_amount_phase[1] = z_supply_amount / 3;
        supply_amount_phase[2] = z_supply_amount / 3;
    }
    else
    {
        for(idx = 0; idx < 3; idx++)
        {
            amount_today_phase[idx] = bcd2u32(today_data + idx * 4, 4, &is_valid1);
            amount_yestoday_phase[idx] = bcd2u32(yestoday_data + idx * 4, 4, &is_valid2);

            if((is_valid1) && (is_valid2))
            {
                if(amount_yestoday_phase[idx] <= amount_today_phase[idx])
                {
                    amount_today_phase[idx] -= amount_yestoday_phase[idx];
                }
                else
                {
                    //���˱�����һȦ�������˱�����ֵ��Ȼ�����㣬��0��ʼ������
    //                meter_bit = 1;
    //                for(i=0;i<meter_doc.mbit_info.integer+4;i++)
    //                {
    //                    meter_bit *= 10;
    //                }
    //                meter_bit *= 10000;
    //                amount_today_phase[idx] = meter_bit - amount_yestoday_phase[idx] + amount_today_phase[idx];

                    //����0���㣬��Ϊ�б������
                    amount_today_phase[idx] = 0;
                }
            }
            else  //������Ч
            {
                //�޷�����
                if(cal_flag->time_seg == 0)  //����ʱ����
                {
                   return;
                }
                else
                {
                    amount_today_phase[idx] = 0;
                }
            }

            //ct
            if(spot_ptct.ct == 0xFFFF || spot_ptct.ct == 0x00) ct = 1;
            else ct = spot_ptct.ct;
            amount_today_phase[idx] *= ct;

            supply_amount_phase[idx] = amount_today_phase[idx];
        }
    }
}

/*+++
  ����:���㱾�ճ������,����е�����ܱ�Ļ�,ͬʱ���㱾�չ������
  ����:
        tagXLOSTCALFLAG *cal_flag  ��������־��
  ����:
        ��������־��
  ����:
---*/
INT8U compute_today_sale_amount(tagXLOSTCALFLAG *cal_flag)
{
    INT32U  sale_amount, sale_amount_A, sale_amount_B, sale_amount_C;
    INT32U  supply_amount, amount_today, amount_yestoday,supply_amount_phase[3];
    INT32U  ct;
    INT16U  meter_idx;
    SPOT_PTCT spot_ptct;
    //INT8U   rec_flag;
    INT8U  td[2][3], for_year, for_month, for_day, read_datetime[2][5];
    METER_DOCUMENT  meter_doc;
    METER_READ_INFO  C1_F170;
    INT8U  is_valid1,is_valid2,zmeter_idx;
    INT8U   today_data[150], yestoday_data[150];
    INT8U   tmp_buffer[30];

	#ifdef __PROVICE_ANHUI__
    INT8U  dh_td[2][3];//�������ļ�ϵͳ�еĵ����ʱ��
    //INT8U  dh_td_yesterday[3];
    INT32U dh_td_pos;//����ʱ��Ĵ洢λ��
    INT32U  td_pos_today;//����ʱ��Ĵ洢λ�ã��������⴦��
    INT32U  td_pos_yesterday;//
   	#endif
   	
    //���ȼ���Ƿ�ȫ������Ѿ��������
    //��һ֧���û�г����ɹ�,��Ҳ���ܽ��м���.
    if(cal_flag->time_seg == 0)
    {
        for(meter_idx=1;meter_idx<=MAX_METER_COUNT;meter_idx++)
        {
            //rec_flag = read_fmByte(FMADDR_METER_READFLAG+meter_idx);
            //if(rec_flag & READFLAG_DAYHOLD) return cal_flag->value;
            //if(meter_read_flag_flash_check_one_flag(meter_idx,READFLAG_DAYHOLD))  return cal_flag->value;
            if(get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx)) return cal_flag->value;
        }
    }

//    ���㱾���Լ����մ洢��,������ܼ���,�򷵻�.
//    if(FALSE == compute_today_and_yestoday_save_pos(&pos_today,&pos_yestoday))
//    {
//        return cal_flag->value;
//    }

    get_yesterday(td[0]);    //������
    for_day   = td[0][0];
    for_month = td[0][1];
    for_year  = td[0][2];
    td[0][0] = byte2BCD(for_day);
    td[0][1] = byte2BCD(for_month);
    td[0][2] = byte2BCD(for_year);

    date_minus_days(&for_year, &for_month, &for_day, 1);
    td[1][0] = byte2BCD(for_day);
    td[1][1] = byte2BCD(for_month);
    td[1][2] = byte2BCD(for_year);

    //����Ѿ�ȫ����������ն������,ͳ�Ʊ������۵���(�������)
    sale_amount = 0;
    supply_amount = 0;
    sale_amount_A = 0;
    sale_amount_B = 0;
    sale_amount_C = 0;

    for(meter_idx = 1; meter_idx <= MAX_METER_COUNT; meter_idx++)
    {
        //������Ƿ����
        if(file_exist(meter_idx) == FALSE) continue;

        fread_meter_params(meter_idx, PIM_METER_DOC, (INT8U *)&meter_doc, sizeof(METER_DOCUMENT));
        
        //ˮ���ȱ��ܲ����������
        if(check_is_sqr_protocol(meter_doc.protocol)) continue;
        //������ɲ����ܱ��Ͳ��ܲ����������
        if((meter_doc.baud_port.port == COMMPORT_485_CY) && (meter_doc.meter_class.user_class != 6)) continue;

        //�ж��Ƿ�Ϊ�ܱ�(�����ز���485�������ܱ�)
        if(meter_doc.meter_class.user_class == 6)
        {
            zmeter_idx = 1;    //̨���ܱ�
        }
        else
        {
            zmeter_idx = 0;
        }

        //��ȡCT
        fread_meter_params(meter_idx, PIM_METER_F25, spot_ptct.value, sizeof(SPOT_PTCT));

        //��ȡ�����Լ����յ������
        (void)read_spot_dayhold_data(meter_idx, S1C_RDJ_ZXYGDN_SJK, td[0], read_datetime[0], today_data);//0x00000040
        (void)read_spot_dayhold_data(meter_idx, S1C_RDJ_ZXYGDN_SJK, td[1], read_datetime[1], yestoday_data);//0x00000040

//	    //�ж������Ƿ����Ҫ��,���������,������������Ч
//	    if(FALSE == check_rec_date(read_datetime[0], td[0]))
//	    {
//	        mem_set(today_data, sizeof(today_data), REC_DATA_IS_DEFAULT);
//	    }
//
//        if(FALSE == check_rec_date(rec_datetime[1], td[1]))
//	    {
//	        mem_set(yestoday_data, sizeof(yestoday_data), REC_DATA_IS_DEFAULT);
//	    }

        amount_today    = bcd2u32(today_data + 5, 4, &is_valid1);   //5�ֽڣ�������4 + ����1��
        amount_yestoday = bcd2u32(yestoday_data + 5, 4, &is_valid2);

        if((is_valid1) && (is_valid2))
        {
            if(amount_yestoday <= amount_today)
            {
                amount_today -= amount_yestoday;
            }
            else
            {
                //���ܱ�����һȦ���
//                meter_bit = 1;
//                for(idx=0;idx<meter_doc.mbit_info.integer+4;idx++)
//                {
//                    meter_bit *= 10;
//                }
//                meter_bit *= 10000;
//                amount_today = meter_bit - amount_yestoday + amount_today;
                //���ܱ�����һȦ������٣����ܱ���
                amount_today = 0;
            }
        }
        else
        {
            //�޷�����
            if(cal_flag->time_seg == 0)  //����ʱ����
            {
               return cal_flag->value;
            }
            else
            {
                amount_today = 0;
            }
        }

		//ct
        if(spot_ptct.ct == 0xFFFF || spot_ptct.ct == 0x00) ct = 1;
        else ct = spot_ptct.ct;
        amount_today *= ct;

        if(zmeter_idx == 1)
        {
            //�ܱ�
            supply_amount += amount_today;

            compute_zmeter_supply_amount(cal_flag, amount_today, meter_idx, meter_doc, supply_amount_phase);
            
            fwrite_array(FILEID_RUN_DATA, FLADDR_A_SUPPLY_AMOUNT, (INT8U *)&(supply_amount_phase[0]), 4);
            fwrite_array(FILEID_RUN_DATA, FLADDR_B_SUPPLY_AMOUNT, (INT8U *)&(supply_amount_phase[1]), 4);
            fwrite_array(FILEID_RUN_DATA, FLADDR_C_SUPPLY_AMOUNT, (INT8U *)&(supply_amount_phase[2]), 4);
        }
        else   //���ܱ�
        {
            sale_amount += amount_today;

//            fread_array(meter_idx, PIM_REC_STATE, (INT8U *)&(C1_F170), sizeof(DC1_F170));
            app_readdata_cur_data(meter_idx, 0x0000AEC0, read_datetime[0], tmp_buffer, 30,TRUE);
            mem_cpy((void *)&C1_F170,(void *)&tmp_buffer,sizeof(METER_READ_INFO));

            /*
              ����������������ܱ�
              C1_F170.phase.reserved1:                �����ز���ͨ������ͨ����Ϳ���ȷ�ϣ�
              meter_doc.meter_class.meter_class == 2  ����485����һ��485�����
            */
            //485�����໧����ȡF153�����û����������/3
            if(((meter_doc.baud_port.port == COMMPORT_485_REC) || (meter_doc.baud_port.port == COMMPORT_485_CAS)) && (meter_doc.meter_class.meter_class == 2))
            {
                //����supply_amount_phase[3]������
                compute_zmeter_supply_amount(cal_flag, amount_today, meter_idx, meter_doc, supply_amount_phase);
                sale_amount_A += supply_amount_phase[0];
                sale_amount_B += supply_amount_phase[1];
                sale_amount_C += supply_amount_phase[2];
            }
            
            if(C1_F170.phase.reserved1)   //�ز����໧��
            {
                sale_amount_A += amount_today / 3;
                sale_amount_B += amount_today / 3;
                sale_amount_C += amount_today / 3;
            }
            else                         //�ز�����
            {
                if((C1_F170.phase.act_A == 1) && (C1_F170.phase.act_B == 0) && (C1_F170.phase.act_C == 0))
                {
                    sale_amount_A += amount_today;
                }
                else if((C1_F170.phase.act_B == 1) && (C1_F170.phase.act_A == 0) && (C1_F170.phase.act_C == 0))
                {
                    sale_amount_B += amount_today;
                }
                else if((C1_F170.phase.act_C == 1) && (C1_F170.phase.act_A == 0) && (C1_F170.phase.act_B == 0))
                {
                    sale_amount_C += amount_today;
                }
            }
        }
    }

    cal_flag->sum_amount = 1; //�����ܱ�����Ѽ����־
    //���ó������������ɱ�־
    cal_flag->sale_amount = 1;
    fwrite_array(FILEID_RUN_DATA, FLADDR_SALE_AMOUNT, (INT8U *)&sale_amount, 4);
    fwrite_array(FILEID_RUN_DATA, FLADDR_A_SALE_AMOUNT, (INT8U *)&sale_amount_A, 4);
    fwrite_array(FILEID_RUN_DATA, FLADDR_B_SALE_AMOUNT, (INT8U *)&sale_amount_B, 4);
    fwrite_array(FILEID_RUN_DATA, FLADDR_C_SALE_AMOUNT, (INT8U *)&sale_amount_C, 4);

    if(cal_flag->sum_amount)
    {
        fwrite_array(FILEID_RUN_DATA, FLADDR_SUPPLY_AMOUNT, (INT8U *)&supply_amount, 4);
    }

    //update xlost compute status flag.
    fwrite_array(FILEID_RUN_DATA, FLADDR_XLOST_CAL_FLAG, (INT8U*)cal_flag, sizeof(tagXLOSTCALFLAG));

    return cal_flag->value;
}


/*+++
  ���ܣ�ͨ������ն˼�����յĹ������
  ������
          tagXLOSTCALFLAG *cal_flag  ��������־��
  ���أ�
         ��������־��
  ������1)û���ܱ�ֻ��������ն˵��ն�����������㡣
        2)����ն�Ҳ�п��ܲ����ڡ�
        3)��Ҫ��ȡ����ն˵ı��ն�����������ն�������ſ��Լ��㡣
---*/
INT8U compute_today_supply_amount(tagXLOSTCALFLAG *cal_flag)
{
//    INT32U  today_amount,ct_times;
//    INT32U  yestoday_amount;
//    union{
//       INT8U value[4];
//       INT32U     ertu_id;
//    }ertu_addr;
//    INT8U yesterday[3];
//    INT8U dh_date[3];
    //INT8U rec_flag;
    //INT8U str_ct[2];

    /*
    //�������ն˵���Ч��
    read_fmArray(EEADDR_TRANS_ERTU,ertu_addr.value,4);
    if(0xFFFFFFFF == ertu_addr.ertu_id) return cal_flag->value;

    //��ȡ��������ն��ն������������־
    rec_flag = read_fmByte(FMADDR_LOCAL_TRANS_FLAG);
    if(rec_flag != 0xAA) return cal_flag->value;       //have not record success yet!.

    //����
    read_fmArray(FMADDR_LOCAL_TRANS_DHA0,dh_date,3);

     #ifdef __DEBUG_INFO__
     snprintf(info,100,"@compute_today_supply_amount: today [%d-%d-%d] [%d-%d-%d]\r\n",dh_date[0],dh_date[1],dh_date[2],GPLC.record_year,GPLC.record_month,GPLC.record_day);
     debug_print(info);
     #endif

    if(FALSE == is_hex_today(dh_date) ) return cal_flag->value;

    read_fmArray(FMADDR_LOCAL_TRANS_DHA0+3,(unsigned char *)&today_amount,4);

    //����
    get_former_day(yesterday);

    read_fmArray(FMADDR_LOCAL_TRANS_DHA1,dh_date,3);

    if(compare_string(yesterday,dh_date,3) != 0) return cal_flag->value;
    read_fmArray(FMADDR_LOCAL_TRANS_DHA1+3,(unsigned char *)&yestoday_amount,4);


    //���㹩�����
    today_amount -= yestoday_amount;

    //�ܵ�����Ҫ����̨��CT
    fread_ertu_params(EEADDR_TRANS_CT,str_ct,2);
    BCD2Ulong(str_ct,2,1,&ct_times);
    today_amount *= ct_times;

    write_fmArray(FMADDR_SUPPLY_AMOUNT,(unsigned char *)&today_amount,4);

    cal_flag->sum_amount = 1;       */

    return cal_flag->value;

}

void compute_phase_xlost(INT32U sale, INT32U supply, INT8U *xlost_rate_str, double *linelost, BOOLEAN *alarm)
{
    INT32U  sale_amount, supply_amount;
    double   xlost;
    INT16S   xlost_rate;
    BOOLEAN alarm_flag;

    sale_amount = sale;
    supply_amount = supply;
    alarm_flag = FALSE;
    xlost = 99.99;
    xlost_rate_str[2] = 0x01;  //������Ϊ��ֵ
    
    if(supply_amount > 0)
    {
        //�����������۵�������
        if(supply_amount >= sale_amount)
        {
            xlost = supply_amount - sale_amount;
            xlost_rate_str[2] = 0x00;  //������Ϊ��ֵ
        }
        else
        {
            xlost = sale_amount - supply_amount;
            xlost_rate_str[2] = 0x01;  //������Ϊ��ֵ
            alarm_flag = TRUE;
        }
        xlost /= supply_amount;
        xlost *= 100;

        if(xlost >= 100)  xlost = 99.99;

        *linelost = xlost;
    }
    
    xlost_rate = xlost*100;   //С�����1λ��

    xlost_rate_str[0] = byte2BCD(xlost_rate % 100);
    xlost_rate_str[1] = byte2BCD(xlost_rate / 100);
    *alarm = alarm_flag;
}

/*+++
  ���ܣ������㱾��������
  ������
         ��
  ���أ�
         ��
  ����: 1)�ڳ��������е���,���԰�ȫʹ��GPLC
        1)���㵱��������
        2)����Ƿ���������澯
        3)�ܱ������485������������ն�.
---*/
void compute_xlost(void)
{
    INT32U   offset;
    INT32U  sale_amount, supply_amount;
    double   xlost;
    tagXLOSTCALFLAG  cal_flag;
    INT8U xlost_a_day[15];
    INT8U tmp_str[2];
    INT8U xlost_rate_str[3];
    BOOLEAN      alarm_flag;
    INT8U yesterday[3];
    INT16U pass_day;

    mem_set(xlost_a_day, sizeof(xlost_a_day), 0xFF);

    //���ȼ����������־�ֽ�
    fread_array(FILEID_RUN_DATA, FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);

    //����������0xFF��,�򷵻�
    if(cal_flag.value == 0xFF) return;  //complete already .

    #ifdef __DEBUG_INFO__
    //snprintf(info,100,"@compute_xlost: flag=%02X\r\n",cal_flag.value);
    //debug_print(info);
    #endif

    //��鳭������Ƿ��Ѿ�����,���û�м���,��������Ҫ������ճ������
    if(! cal_flag.sale_amount)
    {
        //������ճ������
        cal_flag.value = compute_today_sale_amount(&cal_flag);
        
        //���û���㣬����
        if(!cal_flag.sale_amount)  return;      //fail to calculate today's sale amount.
    }

    //����ܱ�����Ƿ��Ѿ�����õ�
    if(! cal_flag.sum_amount)
    {
        //û�м���õ�,˵���ܱ�������ն�,������ն˼����ܱ����
        //cal_flag.value = compute_today_supply_amount(&cal_flag);
        //if(! cal_flag.sum_amount) return;     //fail to calculate today's supply amount.
        return;
    }

    //�������������
    //��ȡ���㵽�ı��յ��۵����͹�����
    fread_array(FILEID_RUN_DATA, FLADDR_SALE_AMOUNT, (INT8U *)&sale_amount, 4);
    fread_array(FILEID_RUN_DATA, FLADDR_SUPPLY_AMOUNT, (INT8U *)&supply_amount, 4);
    compute_phase_xlost(sale_amount, supply_amount, xlost_rate_str, &xlost, &alarm_flag);
    mem_cpy(xlost_a_day + 3, xlost_rate_str, 3);
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"@Z_compute_xlost: Z_sale=%ld, Z_supply=%ld  Z_xlost=%lf\r\n",sale_amount,supply_amount,xlost);
    debug_print(info);
    #endif

    //��ȡ���㵽��A�౾�յ��۵����͹�����
    fread_array(FILEID_RUN_DATA, FLADDR_A_SALE_AMOUNT, (INT8U *)&sale_amount, 4);
    fread_array(FILEID_RUN_DATA, FLADDR_A_SUPPLY_AMOUNT, (INT8U *)&supply_amount, 4);
    compute_phase_xlost(sale_amount, supply_amount, xlost_rate_str, &xlost, &alarm_flag);
    mem_cpy(xlost_a_day + 6, xlost_rate_str, 3);

    //��ȡ���㵽��B�౾�յ��۵����͹�����
    fread_array(FILEID_RUN_DATA, FLADDR_B_SALE_AMOUNT, (INT8U *)&sale_amount, 4);
    fread_array(FILEID_RUN_DATA, FLADDR_B_SUPPLY_AMOUNT, (INT8U *)&supply_amount, 4);
    compute_phase_xlost(sale_amount, supply_amount, xlost_rate_str, &xlost, &alarm_flag);
    mem_cpy(xlost_a_day + 9, xlost_rate_str, 3);

    //��ȡ���㵽��C�౾�յ��۵����͹�����
    fread_array(FILEID_RUN_DATA, FLADDR_C_SALE_AMOUNT, (INT8U *)&sale_amount, 4);
    fread_array(FILEID_RUN_DATA, FLADDR_C_SUPPLY_AMOUNT, (INT8U *)&supply_amount, 4);
    compute_phase_xlost(sale_amount, supply_amount, xlost_rate_str, &xlost, &alarm_flag);
    mem_cpy(xlost_a_day + 12, xlost_rate_str, 3);

    get_yesterday(yesterday);    //������
    pass_day = getPassedDays(yesterday[2],yesterday[1],yesterday[0]);
    xlost_a_day[0] = yesterday[0];
    xlost_a_day[1] = yesterday[1];
    xlost_a_day[2] = yesterday[2];
    offset = pass_day%30;
    offset *= sizeof(xlost_a_day);
    offset += PIM_ERTU_XLOST;
    fwrite_array(FILEID_RUN_DATA, offset, xlost_a_day, sizeof(xlost_a_day));

    //����������־��дΪFF���Ѽ���
    tmp_str[0] = 0xFF;
    fwrite_array(FILEID_RUN_DATA, FLADDR_XLOST_CAL_FLAG, tmp_str, 1);
}
#endif //#ifdef __COMPUTE_XLOST__

#ifdef __ENABLE_ESAM2__
/*+++
  ���ܣ���鳭�����
  ������
          INT8U *data
          INT8U datalen
          INT16U file_id
          INT32U offset
  ���أ�
          TRUE ����Ҫִ�е�����
  ������
         1)
---*/
BOOLEAN check_batch_meter_rec_ctrl(INT8U *data,INT8U datalen,INT16U file_id,INT32U offset)
{
    BATCH_TASK_TIME *task_time;
    INT8U ctrl_idx;

    task_time = (BATCH_TASK_TIME*)data;

    //��鳭�����
    for(ctrl_idx=0;ctrl_idx<4;ctrl_idx++)
    {
        if(task_time->task_ctrl[ctrl_idx] == 0x01)
        {
            task_time->task_ctrl[ctrl_idx] = 0x00;
            fwrite_array(file_id,offset,task_time->value,datalen);
            if(ctrl_idx != 3) return TRUE;
        }
    }

    if((task_time->task_ctrl[0] == 0x00) && (task_time->task_ctrl[1] == 0x00)
    && (task_time->task_ctrl[2] == 0x00) && (task_time->task_ctrl[3] == 0x00))
    {
        //�������ʹ�����ˣ�����ִ��ʧ�ܴ���
        task_time->task_state = 0x0A;
        fwrite_array(file_id,offset,task_time->value,datalen);
        return FALSE;
    }
    return TRUE;
}

/*+++
  ���ܣ�����Ƿ�������������� -- �����֤
  ������
          INT16U meter_idx
  ���أ�
          TRUE ����Ҫִ�е�����
  ������
         1)
---*/
BOOLEAN check_batch_meter_task_auth(INT16U meter_idx)
{
    INT32U offset;
    BATCH_TASK_HEADER_XIAO task_header_xiao;
    union{
        INT8U value[16];
        BATCH_TASK_AUTH task_auth;
        BATCH_TASK_TIME task_time;
    }var;
    //INT8U ctrl_idx;
    BOOLEAN has_task;

    has_task = FALSE;
    mem_set(task_header_xiao.value,sizeof(BATCH_TASK_HEADER_XIAO),0x00);
    fread_array(FILEID_METER_BATCH_TASK_0,0,task_header_xiao.value,sizeof(BATCH_TASK_HEADER_XIAO));
    offset = sizeof(BATCH_TASK_AUTH);
    offset *= meter_idx;
    offset += sizeof(BATCH_TASK_HEADER);
    fread_array(FILEID_METER_BATCH_TASK_0,offset,var.task_auth.value,sizeof(BATCH_TASK_AUTH));
    if(task_header_xiao.exec_type == 1) //���ֵ������
    {
        if(var.task_auth.task_state == 0xAA)
        {
            has_task = TRUE;
        }
    }
    else if(task_header_xiao.exec_type == 2) //ȫ���������
    {
        if((var.task_auth.task_state != 0x0A) && (var.task_auth.task_state != 0xA0))
        {
            has_task = TRUE;
        }
    }

    if(has_task)
    {
        return check_batch_meter_rec_ctrl(var.task_auth.value,sizeof(BATCH_TASK_AUTH),FILEID_METER_BATCH_TASK_0,offset);
    }
    return FALSE;
}

/*+++
  ���ܣ�����Ƿ�������������� -- ��ʱ
  ������
          INT16U meter_idx
          INT8U task_id
  ���أ�
          TRUE ����Ҫִ�е�����
  ������
         1)
---*/
BOOLEAN check_batch_meter_task_time(INT16U meter_idx)
{
    INT32U offset;
    union{
        INT8U value[16];
        BATCH_TASK_AUTH task_auth;
        BATCH_TASK_TIME task_time;
    }var;
    //INT8U ctrl_idx;

    //��������֤����û�гɹ�����������ִ��
    offset = sizeof(BATCH_TASK_AUTH);
    offset *= meter_idx;
    offset += sizeof(BATCH_TASK_HEADER);
    fread_array(FILEID_METER_BATCH_TASK_0,offset,var.task_auth.value,sizeof(BATCH_TASK_AUTH));
    if(var.task_auth.task_state != 0xA0) return FALSE;

    offset = sizeof(BATCH_TASK_TIME);
    offset *= meter_idx;
    offset += sizeof(BATCH_TASK_HEADER);
    fread_array(FILEID_METER_BATCH_TASK_1,offset,var.task_auth.value,sizeof(BATCH_TASK_TIME));
    if(var.task_auth.task_state == 0xAA)
    {
        return check_batch_meter_rec_ctrl(var.task_auth.value,sizeof(BATCH_TASK_TIME),FILEID_METER_BATCH_TASK_1,offset);
    }
    return FALSE;
}

/*+++
  ���ܣ�����Ƿ��������������
  ������
          INT16U meter_idx
          INT8U task_id
  ���أ�
          TRUE ����Ҫִ�е�����
  ������
         1)
---*/
BOOLEAN check_batch_meter_task(INT16U meter_idx)
{
    if(check_batch_meter_task_auth(meter_idx) == TRUE) return TRUE;
    if(check_batch_meter_task_time(meter_idx) == TRUE) return TRUE;
    return FALSE;
}

/*+++
  ���ܣ�����Ƿ��������������
  ������
          INT16U meter_idx
          INT8U task_id
  ���أ�
          TRUE ����Ҫִ�е�����
  ������
         1)
---*/
BOOLEAN update_batch_meter_task_ctrl(INT16U task_id,INT32U offset,INT8U* data)
{
    INT16U file_id;
    BATCH_TASK_AUTH *task_auth;
    INT8U ctrl_idx,datalen;

    if(task_id == 0)
    {
        file_id = FILEID_METER_BATCH_TASK_0;
        datalen = sizeof(BATCH_TASK_AUTH);
    }
    else if(task_id == 1)
    {
        file_id = FILEID_METER_BATCH_TASK_1;
        datalen = sizeof(BATCH_TASK_TIME);
    }
    else return FALSE;

    task_auth = (BATCH_TASK_AUTH*)data;

    //��鳭�����
    for(ctrl_idx=0;ctrl_idx<4;ctrl_idx++)
    {
        if(task_auth->task_ctrl[ctrl_idx] == 0x01)
        {
            return FALSE;  //�´�ִ��
        }
        else if(task_auth->task_ctrl[ctrl_idx] > 0x01)
        {
            task_auth->task_ctrl[ctrl_idx] = task_auth->task_ctrl[ctrl_idx]>>1;
            fwrite_array(file_id,offset,task_auth->value,datalen);
            break;
        }
    }
    if((task_auth->task_ctrl[0] == 0x00) && (task_auth->task_ctrl[1] == 0x00)
        && (task_auth->task_ctrl[2] == 0x00) && (task_auth->task_ctrl[3] == 0x00))
    {
        //�������ʹ�����ˣ�����ִ��ʧ�ܴ���
        task_auth->task_state = 0x0A;
        fwrite_array(file_id,offset,task_auth->value,datalen);
        return FALSE;
    }
    return TRUE;
}

/*+++
  ���ܣ������������--�����֤
  ������
          METER_DOCUMENT *meter_doc,
          INT8U *frame
          INT8U rec_flag  ������־
          INT32U *item     �������ݱ�ʶ
  ���أ�
          TRUE
  ������
         1)Ŀǰ���ڳ���������ֻ��1���������Ҫ���������Ҫ��������1���ֽ�ȥ������ǰ���
---*/
BOOLEAN prepare_exec_batch_meter_task_auth(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    BOOLEAN auth_identity_auth(INT8U P2,INT8U meter_no[8],INT8U meter_er[32],INT8U *task_data,INT8U *frame);
    INT32U offset;
    INT16U meter_idx;//task_data_len,
    union{
        INT8U value[16];
        BATCH_TASK_AUTH task_auth;
        BATCH_TASK_TIME task_time;
    }var;
    INT8U tmp[10]={0};//,frame_pos;
    //INT8U meter_rdn[4];
    //INT8U task_data[50];
    //INT8U task_auth_state;//,ctrl_idx;

    if (GB645_2007 != read_params->meter_doc.protocol) return FALSE;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    //��������֤����
    fread_array(FILEID_METER_BATCH_TASK_0,0,tmp,9);
    offset = sizeof(BATCH_TASK_AUTH);
    offset *= meter_idx;
    offset += sizeof(BATCH_TASK_HEADER);
    fread_array(FILEID_METER_BATCH_TASK_0,offset,var.task_auth.value,sizeof(BATCH_TASK_AUTH));
    //task_auth_state = var.task_auth.task_state; //�������͵�����Ҫ�������֤�ɹ������ִ��
    if(tmp[8] == 1)  //���ֵ������
    {
        if(var.task_auth.task_state == 0xAA)
        {
            //��鳭�����
            if(!update_batch_meter_task_ctrl(0,offset,var.task_auth.value)) return FALSE;
            //���������֤����
            if(auth_identity_auth(1,var.task_auth.meter_no,var.task_auth.meter_er,NULL,frame) == TRUE)
            {
                mem_cpy(var.value,frame,16);
                int32u2_bin(0x070000FF,read_params->item);
                read_params->resp_byte_num = 20;
                *frame_len = make_gb645_2007_esam_auth_frame(frame,read_params->meter_doc.meter_no,0x070000FF,var.value+8,var.value);

                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** prepare item esam auth : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                        meter_idx,0x070000FF,0xFFFFFFFF);
                debug_println_ext(info);
                #endif

                return TRUE;
            }
            else
            {
                //�ն�esam����ʧ�ܽ���������Ϊʧ��
                var.task_auth.task_state = 0x0A;
                fwrite_array(FILEID_METER_BATCH_TASK_0,offset,var.task_auth.value,sizeof(BATCH_TASK_AUTH));
                return FALSE;
            }
        }
    }
    else if(tmp[8] == 2)  //ȫ���������
    {
        if((var.task_auth.task_state != 0x0A) && (var.task_auth.task_state != 0xA0))
        {
            var.task_auth.task_state = 0xAA;
            fread_array(FILEID_METER_BATCH_TASK_0,9,var.task_auth.meter_er,32);
            fread_array(FILEID_METER_BATCH_TASK_0,41,tmp,2);    //��������
            mem_set(var.task_auth.meter_no,2,0x00);
            mem_cpy_reverse(var.task_auth.meter_no+2,read_params->meter_doc.meter_no,6);

            //��鳭�����
            if(!update_batch_meter_task_ctrl(0,offset,var.task_auth.value)) return FALSE;
            //���������֤����
            if(auth_identity_auth(2,var.task_auth.meter_no,var.task_auth.meter_er,tmp,frame) == TRUE)
            {
                mem_cpy(var.value,frame,16);
                int32u2_bin(0x070000FF,read_params->item);
                read_params->resp_byte_num = 20;
                *frame_len = make_gb645_2007_esam_auth_frame(frame,read_params->meter_doc.meter_no,0x070000FF,var.value+8,var.value);

                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** prepare item esam auth : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                        meter_idx,0x070000FF,0xFFFFFFFF);
                debug_println_ext(info);
                #endif
                
                return TRUE;
            }
            else
            {
                //�ն�esam����ʧ�ܽ���������Ϊʧ��
                var.task_auth.task_state = 0x0A;
                fwrite_array(FILEID_METER_BATCH_TASK_0,offset,var.task_auth.value,sizeof(BATCH_TASK_AUTH));
                return FALSE;
            }
        }
    }
    return FALSE;
}

/*+++
  ���ܣ������������--Уʱ
  ������
          INT16U meter_idx
          METER_DOCUMENT *meter_doc,
          INT8U *frame
          INT8U rec_flag  ������־
          INT32U *item     �������ݱ�ʶ
          INT16U delay    ��ʱʱ��
          BOOLEAN flag    true������F1���� false������F3����
  ���أ�
          TRUE
  ������
         1)Ŀǰ���ڳ���������ֻ��1���������Ҫ���������Ҫ��������1���ֽ�ȥ������ǰ���
---*/
BOOLEAN prepare_exec_batch_meter_task_time(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len,INT16U delay,BOOLEAN is_F3)
{
    BOOLEAN auth_set_time(INT8U task_data[40],INT8U meter_er[32],INT8U meter_rdn[4],INT16U delay,INT8U *frame);
    INT32U offset;
    INT16U task_data_len,meter_idx;
    BATCH_TASK_HEADER_XIAO task_header_xiao;
    union{
        INT8U value[56];//���������������Ա��ȣ�ռ��ͬ����С�ռ�Ļ���Ӧ����57�ֽ�
        BATCH_TASK_AUTH task_auth;
        BATCH_TASK_TIME task_time;
    }var;
    INT8U meter_rdn[4];
    INT8U task_data[50];
    //INT8U task_auth_state,ctrl_idx;

    if (GB645_2007 != read_params->meter_doc.protocol) return FALSE;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    //�����֤����û��ִ�гɹ�����ִ���������͵�����
    offset = sizeof(BATCH_TASK_AUTH);
    offset *= meter_idx;
    offset += sizeof(BATCH_TASK_HEADER);
    fread_array(FILEID_METER_BATCH_TASK_0,offset,var.task_auth.value,sizeof(BATCH_TASK_AUTH));
    if(var.task_auth.task_state != 0xA0) return FALSE;
    mem_cpy(meter_rdn,var.task_auth.meter_rondon,4);
    //
    fread_array(FILEID_METER_BATCH_TASK_1,0,task_header_xiao.value,sizeof(BATCH_TASK_HEADER_XIAO));
    offset = sizeof(BATCH_TASK_TIME);
    offset *= meter_idx;
    offset += sizeof(BATCH_TASK_HEADER);
    fread_array(FILEID_METER_BATCH_TASK_1,offset,var.task_time.value,sizeof(BATCH_TASK_TIME));
    if(var.task_time.task_state == 0xAA)
    {
        if((task_header_xiao.task_format[0] != 0x04) || (task_header_xiao.task_format[1] != 0x04)) return FALSE; //�����ʽ
        if(task_header_xiao.task_type != 0x01) return FALSE;    //��������
        task_data_len = bin2_int16u(task_header_xiao.task_data_len); //���񳤶�
        if(task_data_len != 40) return FALSE;
        fread_array(FILEID_METER_BATCH_TASK_1,41,task_data,task_data_len);
        //��鳭�����
        if(!update_batch_meter_task_ctrl(1,offset,var.task_time.value)) return FALSE;
        int32u2_bin(0x0400010C,read_params->item);
        read_params->resp_byte_num = 20;
        if ((is_F3) || (read_params->meter_doc.baud_port.port != COMMPORT_PLC))
        {
            //����Уʱ����
            if(auth_set_time(task_data,var.task_time.meter_er,meter_rdn,delay,frame) == TRUE)
            {
                mem_cpy(var.value,frame,24);
                *frame_len = make_gb645_2007_esam_set_time_frame(frame,read_params->meter_doc.meter_no,0x0400010C,var.value+4,var.value+20);

                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** prepare item esam time : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                        meter_idx,0x0400010C,0xFFFFFFFF);
                debug_println_ext(info);
                #endif

                return TRUE;
            }
            else
            {
                //�ն�esam����ʧ�ܽ���������Ϊʧ��
                var.task_time.task_state = 0x0A;
                fwrite_array(FILEID_METER_BATCH_TASK_1,offset,var.task_time.value,sizeof(BATCH_TASK_TIME));
                return FALSE;
            }
        }
        else
        {
            read_params->control.rec_delay_flag = 1;
//            #ifdef __376_2_2013__
            *frame_len = make_gb645_2007_esam_set_time_frame(frame,read_params->meter_doc.meter_no,0x0400010C,var.value+4,var.value+20);
            return TRUE;
//            #else
//            //����Уʱ����
//            if(auth_set_time(task_data,var.task_time.meter_er,meter_rdn,0,frame+2) == TRUE)
//            {
//                mem_cpy(var.value,frame+2,24);
//                frame[frame_pos] = make_gb645_2007_esam_set_time_frame(frame+frame_pos+1,GPLC.ADDR_DST,*item,var.value+4,var.value+20);
//                return TRUE;
//            }
//            else
//            {
//                //�ն�esam����ʧ�ܽ���������Ϊʧ��
//                var.task_time.task_state = 0x0A;
//                fwrite_array(FILEID_METER_BATCH_TASK_1,offset,var.task_time.value,sizeof(BATCH_TASK_TIME));
//                return FALSE;
//            }
//            #endif
        }
    }

    return FALSE;
}

BOOLEAN prepare_exec_batch_meter_task(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len)
{
    if (get_bit_value(read_meter_flag_esam,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx)) == FALSE) return FALSE;

    if (prepare_exec_batch_meter_task_auth(read_params,frame,frame_len)) return TRUE;
    if (prepare_exec_batch_meter_task_time(read_params,frame,frame_len,0,FALSE)) return TRUE;

    //���esam����
    clr_bit_value(read_meter_flag_esam,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
    return FALSE;
}
#endif  //#ifdef __ENABLE_ESAM2__

BOOLEAN prepare_read_item_patch_day_hold(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT16U day1,day2,day3;
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT16U meter_idx,mask_idx;
    INT8U idx,idx_sub,idx_day,day,item_idx;
    INT8U rec_td[3]={0};
    INT8U reserve_data[RESERVE_DATA]={0};
    #ifdef __PROVICE_HUNAN__
    INT32U offset;
	INT8U  f39[32];
	INT8U  user_class;
	INT8U  meter_class;
	#endif

    
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
	#ifdef __PROVICE_HUNAN__
    //����F39���ж��Ƿ���Ҫ��������ͷ���
    mem_set(f39,sizeof(f39),0x00);
	offset = EEADDR_SET_F39;
	user_class = read_params->meter_doc.meter_class.user_class;
	meter_class = read_params->meter_doc.meter_class.meter_class;
	offset += (512 + 3)*user_class;
	offset += 32*meter_class;
	fread_array(FILEID_ERTU,offset+1,f39,sizeof(f39));
	#endif
    if (get_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;

    if (read_params->meter_doc.protocol != GB645_2007)
    {
        clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }
CHECK_ITEM:
    if ((read_params->patch_ctrl_1.patch_day_num == 0) || (read_params->patch_ctrl_1.patch_day_num > DEFAULT_DAY_HOLD_PATCH_DAY_NUM))
    {
        clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }

    if (read_params->patch_ctrl_2.patch_count >= DEFAULT_DAY_HOLD_PATCH_COUNT)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** prepare item patch day hold : �������� = %d ������·�ɳ���ʧ�ܣ�",
                read_params->patch_ctrl_2.patch_count);
        debug_println_ext(info);
        #endif
        read_params->control.loop = 1;
        return FALSE;
    }

    if (read_params->control.day_hold_td_flag)
    {
        if (read_params->patch_ctrl_1.find_td)
        {
            get_yesterday(rec_td);
            day = read_params->patch_ctrl_1.patch_day_num - 1;
            item_idx = read_params->patch_ctrl_1.patch_day_num - 1;
            #ifdef __PROVICE_HUNAN__
			if(get_bit_value(f39+1,31,160))
            {
                set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_ZXYG_DH);
			}
			if(get_bit_value(f39+1,31,162))
			{
                set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_FXYG_DH);
			}
			#else
            set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_ZXYG_DH);
            set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_FXYG_DH);
            #endif
            while(day > 0)
            {
                if(day > 28)
                {
                    date_minus_days(rec_td+2,rec_td+1,rec_td+0,28);
                    day -= 28;
                }
                else
                {
                    date_minus_days(rec_td+2,rec_td+1,rec_td+0,day);
                    day -= day;
                }
            }

            for(idx_day=1;idx_day<read_params->patch_ctrl_1.patch_day_num;idx_day++)
            {
                mask_idx = 0;
                for(idx=0;idx<sizeof(CYCLE_DAY_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
                {
                    for(idx_sub=0;idx_sub<((CYCLE_DAY_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
                    {
                        if(get_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,mask_idx))
                        {
                            get_phy_data((READ_WRITE_DATA*)&(CYCLE_DAY_PHY_LIST[idx]),idx_sub,&phy);
                            if(readdata_cycle_day(meter_idx,&phy,rec_td,frame,frame+5,frame_len,reserve_data) == FALSE)
                            {
                                //�����ն���ʱ��
                                mem_set(read_params->day_hold_td,3,0x00);

                                goto FOR_END;
                            }
                            else
                            {
                                if ((reserve_data[0] > 0) && (reserve_data[0] <= 62))
                                {
                                    item_idx = reserve_data[0];
                                }
                                else item_idx = 1;

                                day1 = getPassedDays(2000+frame[4],frame[3],frame[2]);
                                day2 = getPassedDays(2000+datetime[YEAR],datetime[MONTH],datetime[DAY]);
                                day3 = getPassedDays(2000+rec_td[2],rec_td[1],rec_td[0]);

                                if(item_idx == 1)
                                {
                                    item_idx = day2 - day3 - 1;
                                }
                                else
                                {
                                    if(day1 <= day2) item_idx = item_idx + (day2 - day1) - 1;
                                }

                                if(item_idx > (DEFAULT_DAY_HOLD_PATCH_DAY_NUM-1)) item_idx = DEFAULT_DAY_HOLD_PATCH_DAY_NUM - 1;
                            }
                        }
                        mask_idx++;
                    }
                }
                date_add_days(rec_td+2,rec_td+1,rec_td+0,1);
            }

FOR_END:

            if(idx_day >= read_params->patch_ctrl_1.patch_day_num)   //û��Ҫ����
            {
                clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
                return FALSE;
            }

            read_params->patch_ctrl_1.find_td = 0;
            read_params->patch_ctrl_1.patch_day_num = item_idx;
        }
    }
    else
    {
        if (read_params->patch_ctrl_1.find_td)
        {
            read_params->patch_ctrl_1.patch_day_num--;
            read_params->patch_ctrl_1.find_td = 0;
        }
        #ifdef __PROVICE_HUNAN__
		if(get_bit_value(f39+1,31,160))
        {
            set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_ZXYG_DH);
		}
		if(get_bit_value(f39+1,31,162))
		{
            set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_FXYG_DH);
		}
		#else
        set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_ZXYG_DH);		
        set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_FXYG_DH);
		#endif
        //�����ն���ʱ��
//        get_yesterday(read_params->day_hold_td);
//        date_minus_days(read_params->day_hold_td+2,read_params->day_hold_td+1,read_params->day_hold_td,read_params->patch_ctrl.patch_day_num);


        get_yesterday(read_params->day_hold_td);
        day = read_params->patch_ctrl_1.patch_day_num;

        while(day > 0)
        {
            if(day > 28)
            {
                date_minus_days(read_params->day_hold_td+2,read_params->day_hold_td+1,read_params->day_hold_td,28);
                day -= 28;
            }
            else
            {
                date_minus_days(read_params->day_hold_td+2,read_params->day_hold_td+1,read_params->day_hold_td,day);
                day -= day;
            }
        }
    }

    if (read_params->patch_ctrl_1.patch_day_num > 0)
    {
        if(check_is_all_ch(read_params->day_hold_td,3,0x00))
        {
            if(get_data_item(0x00002C00,read_params->meter_doc.protocol,&library))
            {
                if(library.item != 0xFFFFFFFF)
                {
                    int32u2_bin(0x00002C00,read_params->phy);
                    int32u2_bin(library.item+read_params->patch_ctrl_1.patch_day_num,read_params->item);
                    read_params->resp_byte_num = 40;
                    read_params->read_type = READ_TYPE_DAY_HOLD_PATCH;

                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item+read_params->patch_ctrl_1.patch_day_num,NULL,0);

                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** prepare item patch day hold : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                            meter_idx,library.item+read_params->patch_ctrl_1.patch_day_num,0x00002C00);
                    debug_println_ext(info);
                    #endif

                    return TRUE;
                }
            }
        }

        mask_idx = 0;
        for(idx=0;idx<sizeof(CYCLE_DAY_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
        {
            for(idx_sub=0;idx_sub<((CYCLE_DAY_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
            {
                if(get_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,mask_idx))
                {
                    get_phy_data((READ_WRITE_DATA*)&(CYCLE_DAY_PHY_LIST[idx]),idx_sub,&phy);
                    if(readdata_cycle_day(meter_idx,&phy,read_params->day_hold_td,frame,frame+5,frame_len,NULL) == FALSE)
                    {
                        //��Ҫ����
                        //����Լ�⺯��
                        if(get_data_item(phy.phy,read_params->meter_doc.protocol,&library))
                        {
                            if(library.item != 0xFFFFFFFF)
                            {
                                int32u2_bin(phy.phy,read_params->phy);
                                int32u2_bin(library.item+read_params->patch_ctrl_1.patch_day_num,read_params->item);
                                read_params->resp_byte_num = 40;
                                read_params->read_type = READ_TYPE_DAY_HOLD_PATCH;

                                if (read_params->meter_doc.protocol == GB645_2007)
                                {
                                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item+read_params->patch_ctrl_1.patch_day_num,NULL,0);
                                }
                                else if ((read_params->meter_doc.protocol == GB645_1997) ||(read_params->meter_doc.protocol == GB645_1997_JINANGSU_4FL) 
                                        || (read_params->meter_doc.protocol == GB645_1997_JINANGSU_2FL) || (read_params->meter_doc.protocol  == 0) )
                                {
                                    *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item+read_params->patch_ctrl_1.patch_day_num,NULL,0);
                                }
                                else
                                 *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item+read_params->patch_ctrl_1.patch_day_num,NULL,0);
                                #ifdef __SOFT_SIMULATOR__
                                snprintf(info,100,"*** prepare item day hold patch : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                                        meter_idx,library.item+read_params->patch_ctrl_1.patch_day_num,phy.phy);
                                debug_println_ext(info);
                                #endif

                                return TRUE;
                            }
                            else
                            {
                                clr_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,mask_idx);
                            }
                        }
                        else
                        {
                            clr_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,mask_idx);
                        }
                    }
                    clr_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,mask_idx);
                }
                mask_idx++;
            }
        }

		#ifdef __PROVICE_HUNAN__
		if(get_bit_value(f39+1,31,160))
        {
            set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_ZXYG_DH);
		}
		if(get_bit_value(f39+1,31,162))
		{
            set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_FXYG_DH);
		}
		#else
        set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_ZXYG_DH);
		set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_FXYG_DH);
		#endif
        //�����ն���ʱ��
        mem_set(read_params->day_hold_td,3,0x00);
        read_params->patch_ctrl_1.patch_day_num--;
        goto CHECK_ITEM;
    }

    clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);

    return FALSE;
}

INT8U save_patch_day_hold(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    INT32U metertd,curtd,begintd,endtd;
    READ_WRITE_DATA phy;
    INT16U meter_idx;
    INT8U idx;
    INT8U reserve_data[RESERVE_DATA];
    INT8U begin_td[3]={0};
    INT8U end_td[3]={0};
	#ifdef __CHONGQING_DAY_HOLD_CTRL__
    INT8U td[3];
	#endif

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if (bin2_int32u(read_params->phy) == 0x00002C00)    //�ն���ʱ��
    {
        if(check_is_all_ch(frame,frame_len,0xEE)) //����
        {
            clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
            return 0;
        }

        read_params->day_hold_td[0] = BCD2byte(frame[2]);    //��
        read_params->day_hold_td[1] = BCD2byte(frame[3]);    //��
        read_params->day_hold_td[2] = BCD2byte(frame[4]);    //��

//        #ifdef __SOFT_SIMULATOR__
//        snprintf(info,100,"*** save patch day hold data td : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d-%02d",
//                bin2_int16u(read_params->meter_doc.meter_idx),
//                bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
//                read_params->day_hold_td[2],read_params->day_hold_td[1],read_params->day_hold_td[0]);
//        debug_println_ext(info);
//        #endif

        if ((read_params->day_hold_td[1] == 0) || (read_params->day_hold_td[1] > 12)
        || (read_params->day_hold_td[0] == 0) || (read_params->day_hold_td[0] > 31))
        {
            clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
            return 0;
        }

		#ifdef __CHONGQING_DAY_HOLD_CTRL__
        // 
        mem_cpy(td,datetime+DAY,3);
		date_minus_days(td+2,td+1,td,read_params->patch_ctrl_1.patch_day_num);
		if(compare_string(td,read_params->day_hold_td,3) != 0)
		{
		    //���ʱ�겻�ԣ��������ʹ洢�ˡ�
		    clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
            return 0;
		}
		#endif
        mem_cpy(begin_td,datetime+DAY,3);
        mem_cpy(end_td,datetime+DAY,3);
        date_add_days(begin_td+2,begin_td+1,begin_td+0,25);
        date_add_days(begin_td+2,begin_td+1,begin_td+0,25);
        date_add_days(begin_td+2,begin_td+1,begin_td+0,25);
        date_add_days(begin_td+2,begin_td+1,begin_td+0,15);
        begintd = begin_td[2]*10000+begin_td[1]*100+begin_td[0];

        date_minus_days(end_td+2,end_td+1,end_td+0,25);
        date_minus_days(end_td+2,end_td+1,end_td+0,25);
        date_minus_days(end_td+2,end_td+1,end_td+0,25);
        date_minus_days(end_td+2,end_td+1,end_td+0,15);
        endtd = end_td[2]*10000+end_td[1]*100+end_td[0];

        metertd = read_params->day_hold_td[2]*10000+read_params->day_hold_td[1]*100+read_params->day_hold_td[0];
        curtd = datetime[YEAR]*10000+datetime[MONTH]*100+datetime[DAY];

        //�ն���ʱ��ǰ�󳬹�90�죬�������ն�������
        if ((metertd > begintd) || (metertd < endtd))
        {
            mem_set(read_params->day_hold_td,3,0x00);
            if (read_params->patch_ctrl_1.patch_day_num > 0) read_params->patch_ctrl_1.patch_day_num--;
            else clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
            return 0;
        }

        if ((read_params->control.according_to_day_hold_td_save == 1) || (read_params->control.according_to_day_hold_td_save == 2))
        {
            //ʱ�䳬��������ʱ�� ����������
            if (metertd >= curtd)
            {
                clr_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
                return 0;
            }
        }

        date_minus_days(read_params->day_hold_td+2,read_params->day_hold_td+1,read_params->day_hold_td,1);
        set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_ZXYG_DH);
        set_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,READ_MASK_DAY_HOLD_FXYG_DH);

        return 0;
    }

    idx = get_phy_form_list_cycle_day(bin2_int32u(read_params->phy),&phy);
    if(idx == 0xFF) return 0;

    mem_set(reserve_data,RESERVE_DATA,0xFF);
    reserve_data[0] = read_params->item[0];
    writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,reserve_data,0xFF);
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,200,"*** save patch day hold data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d-%02d ",
            bin2_int16u(read_params->meter_doc.meter_idx),
            bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
            read_params->day_hold_td[2],read_params->day_hold_td[1],read_params->day_hold_td[0]);
    debug_println_ext(info);
    #endif

    clr_bit_value(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,idx);

    if(check_is_all_ch(read_params->read_mask.day_hold_patch,READ_MASK_BYTE_NUM_DAY_HOLD_PATCH,0x00))
    {
        read_params->patch_ctrl_2.patch_count++;
    }

    return 0;
}

void save_day_hold_power_on_off(READ_PARAMS *read_params)
{
    READ_WRITE_DATA phy;
    INT16U block_begin_idx,cur_idx;
    INT8U buffer[200];
    INT8U frame[200];
    INT8U idx,block_count;
	INT8U td[3];

    cur_idx = get_phy_form_list_cur_data(0x0000AF00,&phy,&block_begin_idx,&block_count);
    if (cur_idx == 0xFFFF) return;

    if(readdata_cur_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,frame,frame+6 ,frame+5,TRUE))
    {
        if(is_valid_bcd(frame+6,3) == TRUE)
        {
            idx = get_phy_form_list_cycle_day(phy.phy,&phy);
            if(idx == 0xFF) return;
            get_yesterday(td);			
            writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,td,frame+6,frame[5],buffer,NULL,0xFF);

            cur_idx = get_phy_form_list_cur_data(0x0000AF40,&phy,&block_begin_idx,&block_count);
            if (cur_idx == 0xFFFF) return;
            phy.flag = 0;
            phy.block_offset = 0;
            phy.block_len = 120;
            if(readdata_cur_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,frame,frame+6,frame+5,TRUE))
            {
                idx = get_phy_form_list_cycle_day(phy.phy,&phy);
                if(idx == 0xFF) return;
                get_yesterday(td);
                phy.flag = 0;
                phy.block_offset = 0;
                phy.block_len = 120;
                writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,td,frame+6,frame[5],buffer,NULL,0xFF);
            }
        }
    }
}

//#ifdef __ERC_39__
/*+++
  ���ܣ�����ն����Ƿ񳭵�,����3��û���ն���ķ���true
  ������
        INT16U meter_idx  ������
        INT8U day         ��鼸����ն�������
        BOOLEAN check_FX  �Ƿ��鷴��
  ���أ�
        
---*/
BOOLEAN check_day_hold_4_day(INT16U meter_idx)
{
    READ_WRITE_DATA phy;
    INT8U idx,datalen;
    INT8U td[3];
    INT8U rec_datetime[5];
    INT8U count;
    INT8U data[40];

    count = 0;

    td[0] = datetime[DAY];
    td[1] = datetime[MONTH];
    td[2] = datetime[YEAR];

    //date_minus_days(td+2,td+1,td,1);

//    td[0] = GPLC.record_day;
//    td[1] = GPLC.record_month;
//    td[2] = GPLC.record_year;

    idx = get_phy_form_list_cycle_day(0x00002C7F,&phy);
    if(idx == 0xFF) return FALSE;

    for(idx=0;idx<3;idx++)
    {
        //mem_cpy(rec_td,td,3);
        date_minus_days(td+2,td+1,td,1);

        if(readdata_cycle_day(meter_idx,&phy,td,rec_datetime,data,&datalen,NULL) == FALSE)
        {
            count++;
        }
        else break;
    }
    if (count == 3) return FALSE;
    else return TRUE;
}

INT8U check_erc_39(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT16U meter_idx;
	INT16U spot_idx = 0;//�˱�����ʹ��ʼ��0������ʱ����û�Դ˱�����ֵ������ʹ��ע������� ????
    METER_DOCUMENT meter_doc;

    portContext = (PLCPortContext*)readportcontext;

    if(!check_event_prop(ERC39))
    {
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = 0;
        portContext->OnPortReadData = router_check_urgent_timeout;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        return 0;
    }

//    #ifdef __SOFT_SIMULATOR__
//    INT32U start_time = os_get_systick_10ms();
//    snprintf(info,100,"*** check_erc_39 begin ***");
//    debug_println_ext(info);
//    #endif

    meter_idx = bin2_int16u(portContext->params.value);
    meter_idx++;
    int16u2_bin(meter_idx,portContext->params.value);
    if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
    {
//        #ifdef __SOFT_SIMULATOR__
//        snprintf(info,100,"*** check_erc_39 meter_idx = %d ***",meter_idx);
//        debug_println_ext(info);
//        #endif

        if (get_app_meter_doc(meter_idx,&meter_doc) == TRUE)
        {
            if(check_day_hold_4_day(meter_idx) == FALSE)
            {
                event_erc_39(meter_idx,spot_idx);
            }
        }
    }
    else
    {
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = 0;
        portContext->OnPortReadData = router_check_urgent_timeout;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
    }

//    #ifdef __SOFT_SIMULATOR__
//    snprintf(info,100,"*** check_erc_39 end time = %d ***",time_elapsed_10ms(start_time));
//    debug_println_ext(info);
//    #endif

    return 0;
}
//#endif //#ifdef __ERC_39__
#ifdef __CHECK_MONTH_HOLD_TD__
BOOLEAN prepare_read_wait_cycle_month(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    //READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT16U meter_idx;
    //INT8U idx,idx_sub;

    //mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    if (get_bit_value(read_meter_flag_wati_month_hold,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;

    if ((read_params->meter_doc.protocol == GB645_2007)&&((read_params->meter_doc.baud_port.port == COMMPORT_485_REC)||(read_params->meter_doc.baud_port.port == COMMPORT_485_CAS)))
    {
        if(get_data_item(0x00005880,read_params->meter_doc.protocol,&library))
        {
            if(library.item != 0xFFFFFFFF)
            {
//                rs232_debug_info("\xE0\xE0",2);
                int32u2_bin(0x00005880,read_params->phy);
                int32u2_bin(library.item,read_params->item);
                read_params->resp_byte_num = 40;
                read_params->read_type = READ_TYPE_MONTH_HOLD_WAIT_TD;

                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);

                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** prepare item day hold wait td : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                        meter_idx,library.item,0x00002C00);
                debug_println_ext(info);
                #endif

                return TRUE;
            }
        }
    }

    clr_bit_value(read_meter_flag_wati_month_hold,READ_FLAG_BYTE_NUM,meter_idx);

    return FALSE;
}
#endif
BOOLEAN prepare_read_wait_cycle_day(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    //READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT16U meter_idx;
    //INT8U idx,idx_sub;

    //mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    if (get_bit_value(read_meter_flag_wati_day_hold,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;

    if (read_params->meter_doc.protocol == GB645_2007)
    {
        if ((read_params->control.day_hold_td_flag) && (read_params->control.according_to_day_hold_td_save == 0) && (read_params->control.wait_day_hold_td))  //��Ҫ�����ն����ն���ʱ��
        {
            if (read_params->patch_ctrl_2.wait_day_hold_td_fail)
            {
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** prepare item day hold wait td : ����·��ʧ�ܣ������ȴ������� ");
                debug_println_ext(info);
                #endif
                read_params->control.loop = 1;
                return FALSE;
            }
            if(get_data_item(0x00002C00,read_params->meter_doc.protocol,&library))
            {
                if(library.item != 0xFFFFFFFF)
                {
                    int32u2_bin(0x00002C00,read_params->phy);
                    int32u2_bin(library.item,read_params->item);
                    read_params->resp_byte_num = 40;
                    read_params->read_type = READ_TYPE_DAY_HOLD_WAIT_TD;

                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);

                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** prepare item day hold wait td : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",
                            meter_idx,library.item,0x00002C00);
                    debug_println_ext(info);
                    #endif

                    return TRUE;
                }
            }
        }
    }

    clr_bit_value(read_meter_flag_wati_day_hold,READ_FLAG_BYTE_NUM,meter_idx);

    return FALSE;
}
#ifdef __CHECK_MONTH_HOLD_TD__
INT8U save_patch_month_hold_wait_td(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    INT16U meter_idx;
    INT8U rec_td[2],cur_td[2];
    INT8U	day_month_hold_td[13] = {0};//5�ֽڵ��ն�ʱ�� +3 �ֽڵ� �ն���ʱ�� + 2�ֽڵ��¶���ʱ��
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if (bin2_int32u(read_params->phy) == 0x00005880)    //�ն���ʱ��
    {
        if(check_is_all_ch(frame,frame_len,0xEE)) //����
        {
            clr_bit_value(read_meter_flag_wati_month_hold,READ_FLAG_BYTE_NUM,meter_idx);
            return 0;
        }
        else
        {
            
            read_params->month_hold_td[0] = BCD2byte(frame[2]);    //��
            read_params->month_hold_td[1] = BCD2byte(frame[3]);    //��
//            rs232_debug_info("\xE1\xE1",2);
//            rs232_debug_info(read_params->month_hold_td,2);
            if(compare_string(read_params->month_hold_td,datetime+MONTH,2) == 0)    //������ȷ
            {
//                rs232_debug_info("\xE2\xE2",2);
                fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                get_former_month(read_params->month_hold_td);
                mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                set_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                clr_bit_value(read_meter_flag_wati_month_hold,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                return 0;
            }
            else
            {
//                rs232_debug_info("\xE3\xE3",2);
                get_former_month(rec_td);
                cur_td[0] = BCD2byte(frame[2]);    //��
                cur_td[1] = BCD2byte(frame[3]);    //��
                if(compare_string(cur_td,rec_td,2) != 0)
                {
//                    rs232_debug_info("\xE4\xE4",2);
                    clr_bit_value(read_meter_flag_wati_month_hold,READ_FLAG_BYTE_NUM,meter_idx);
                    return 0;
                }
            }
        }
    }
    return 0;
}
#endif
INT8U save_patch_day_hold_wait_td(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    //INT32U metertd,curtd;
    //READ_WRITE_DATA phy;
    INT16U meter_idx;
    //INT8U idx;
    INT8U rec_td[3];
    INT8U	day_month_hold_td[13] = {0};//5�ֽڵ��ն�ʱ�� +3 �ֽڵ� �ն���ʱ�� + 2�ֽڵ��¶���ʱ��
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if (bin2_int32u(read_params->phy) == 0x00002C00)    //�ն���ʱ��
    {
        if(check_is_all_ch(frame,frame_len,0xEE)) //����
        {
            clr_bit_value(read_meter_flag_wati_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
            return 0;
        }
        else
        {
            read_params->day_hold_td[0] = BCD2byte(frame[2]);    //��
            read_params->day_hold_td[1] = BCD2byte(frame[3]);    //��
            read_params->day_hold_td[2] = BCD2byte(frame[4]);    //��
            mem_cpy(read_params->month_hold_td,read_params->day_hold_td+1,2);
            if(compare_string(read_params->day_hold_td,datetime+DAY,3) == 0)    //������ȷ
            {
                date_minus_days(read_params->day_hold_td+2,read_params->day_hold_td+1,read_params->day_hold_td,1);
                #ifdef __CHECK_MONTH_HOLD_TD__
                fread_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                #endif
                #ifndef __CHECK_MONTH_HOLD_TD__
                get_former_month_from_param(read_params->month_hold_td);
                #endif
                mem_cpy(day_month_hold_td,datetime+MINUTE,5);
                mem_cpy(day_month_hold_td+5,read_params->day_hold_td,3);
                #ifndef __CHECK_MONTH_HOLD_TD__
                mem_cpy(day_month_hold_td+8,read_params->month_hold_td,2);
                #endif
                fwrite_array(bin2_int16u(read_params->meter_doc.meter_idx),PIM_DAY_HOLD_DY_MONTH_TD,day_month_hold_td,sizeof(day_month_hold_td));
                set_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                clr_bit_value(read_meter_flag_wati_day_hold,READ_FLAG_BYTE_NUM,bin2_int16u(read_params->meter_doc.meter_idx));
                return 0;
            }
            else
            {
                get_yesterday(rec_td);
                if(compare_string(read_params->day_hold_td,rec_td,3) != 0)
                {
                    clr_bit_value(read_meter_flag_wati_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
                    return 0;
                }
            }
        }
    }

    read_params->patch_ctrl_2.wait_day_hold_td_fail = 1;

    return 0;
}



//read_meter_event_ctrl readMeterEventCtrl;

const MeterEventReadData meterEventDataList[]={

    //����
    {0x19000001,{DEFAULT_ZCS_RECORD_FLAG,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #if(defined __PROVICE_SHANGHAI__) //����19010002��1901FF01 �ۼ�ʱ������ݿ� 2015-11-25
    {0x19010001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},// A�����
    {0x19020001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},// B���������Ҫ�����ĺ���������
    {0x19030001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},// C�����
    #elif( (defined  __PROVICE_JIBEI__ ) || (defined __PROVICE_SHAANXI__) )//����1901FF01 �ۼ�ʱ������ݿ� 2016-03-24
    {0x19010001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},// A�����
    {0x19020001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},// B���������Ҫ�����ĺ���������
    {0x19030001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},// C�����
    #else
    {0x19010001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},// A�����
    {0x19020001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},// B���������Ҫ�����ĺ���������
    {0x19030001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},// C�����
    #endif
    //����
    {0x03110000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x02,0,FALSE,FALSE},
    
    
    #if( (defined  __PROVICE_JIBEI__) || (defined __PROVICE_SHAANXI__) )
    //��բ�ܴ��� 1D00FF01 1D00FF02  �������ݿ�
    {0x1D000001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x04,0,FALSE,FALSE},//�����⴦���� TODO
    //��բ�ܴ���
    {0x1E000001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x02,0,FALSE,FALSE},
    #else
    //��բ�ܴ���  ����Ҳ�����⣬�޸�Ϊ ֻ����1D000101 1D000102  �ȼ���������
    {0x1D000001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0001,0x04,0,FALSE,FALSE},//�����⴦���� TODO
    //��բ�ܴ���
    {0x1E000001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0001,0x02,0,FALSE,FALSE},
    #endif
    
    //ʧѹ
    {0x10000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    #if(defined __PROVICE_SHANGHAI__)
    {0x10010001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x10020001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x10030001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #elif( (defined  __PROVICE_JIBEI__ ) || (defined __PROVICE_SHAANXI__) )
    {0x10010001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x10020001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x10030001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    #else
    {0x10010001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x10020001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x10030001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    #endif
    
    //Ƿѹ
    {0x11000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    #if(defined __PROVICE_SHANGHAI__)
    {0x11010001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x11020001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x11030001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #elif( (defined  __PROVICE_JIBEI__ ) || (defined __PROVICE_SHAANXI__) )
    {0x11010001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x11020001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x11030001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    #else
    {0x11010001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x11020001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x11030001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    #endif
    
    //��ѹ
    {0x12000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    #if(defined __PROVICE_SHANGHAI__)
    {0x12010001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x12020001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x12030001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #elif( (defined  __PROVICE_JIBEI__ ) || (defined __PROVICE_SHAANXI__) )
    {0x12010001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x12020001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x12030001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    #else
    {0x12010001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x12020001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x12030001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    #endif
    //ʧ��
    {0x18000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    #if(defined __PROVICE_SHANGHAI__)
    {0x18010001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x18020001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x18030001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #elif( (defined  __PROVICE_JIBEI__ ) || (defined __PROVICE_SHAANXI__) )
    {0x18010001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x18020001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x18030001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    #else
    {0x18010001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x18020001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x18030001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    #endif
    
    //����
    //{0x1C000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    {0x1C010001,{DEFAULT_LOAD_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x1C020001,{DEFAULT_LOAD_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x1C030001,{DEFAULT_LOAD_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    
    //����
    {0x13000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    #if(defined __PROVICE_SHANGHAI__)
    {0x13010001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x13020001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x13030001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #elif( (defined  __PROVICE_JIBEI__ ) || (defined __PROVICE_SHAANXI__) )
    {0x13010001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x13020001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    {0x13030001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    #else
    {0x13010001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x13020001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x13030001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_VOLTAGE_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    #endif
    
    //����
    {0x1A000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    {0x1A010001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x1A020001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    {0x1A030001,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK|DEFAULT_CURRENT_EVENT_RECORD_FLAG_HIGH},0x0002,0x01,0,FALSE,FALSE},
    
    //���ʷ���
    //{0x1B000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    {0x1B010001,{DEFAULT_LOAD_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x1B020001,{DEFAULT_LOAD_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    {0x1B030001,{DEFAULT_LOAD_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    
    //��ѹ������
    //{0x1B000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    #if(defined __PROVICE_SHANGHAI__)
    {0x14000001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #elif( (defined  __PROVICE_JIBEI__ ) || (defined __PROVICE_SHAANXI__) )
    {0x14000001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},
    #else
    {0x14000001,{DEFAULT_LOAD_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #endif
    
    //����������
    {0x15000001,{DEFAULT_LOAD_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    
    //��ѹ��ƽ��
    //{0x1B000001,{DISABLE_READ_BLOCK,DEFAULT_ZCS_RECORD_FLAG},0x0002,0x01,0,FALSE,FALSE},//
    #if(defined __PROVICE_SHANGHAI__)
    {0x16000001,{DEFAULT_RECORD_FLAG_LOW,ENABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},	
    #elif( (defined  __PROVICE_JIBEI__ ) || (defined __PROVICE_SHAANXI__) )
    {0x16000001,{DISABLE_READ_BLOCK,ENABLE_READ_BLOCK},0x0001,0x01,0,FALSE,FALSE},	
    #else
    {0x16000001,{DEFAULT_UNBALANCE_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    #endif
    
    //������ƽ��
    {0x17000001,{DEFAULT_UNBALANCE_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    
    //��������������
    {0x1F000001,{DEFAULT_POWER_FACTOR_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    
    //ȫʧѹ
    {0x03050000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    
    //����ܴ���
    {0x03300000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},
    //��������ܴ���
    {0x03300100,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},//ֻ�������һ�ΰ�???
    //���������ܴ���
    {0x03300200,{DEFAULT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},//ֻ�������һ�ΰ�???
    //�¼������ܴ���
    {0x03300300,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},
    //Уʱ�ܴ���
    {0x03300400,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    //ʱ�α���
    {0x03300500,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},
    //ʱ������
    {0x03300600,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    //�����ձ��
    {0x03300700,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},
    //�ڼ��ձ��
    {0x03300800,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    //�й���Ϸ�ʽ���
    {0x03300900,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},
    //�޹���Ϸ�ʽ1���
    {0x03300A00,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    //�޹���Ϸ�ʽ2���
    {0x03300B00,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},
    //�����ձ��
    {0x03300C00,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},	
    //�����
    {0x03300D00,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x01,0,FALSE,FALSE},
    //����ť��
    {0x03300E00,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    
    //�������� ̫���� �Ȳ�����  TODO
    {0x03120000,{DEFAULT_XL_RECORD_FLAG,DISABLE_READ_BLOCK},0x0006,0x03,0,FALSE,FALSE},
    //�������ز�ƽ��
    {0x20000001,{DEFAULT_UNBALANCE_EVENT_RECORD_FLAG_LOW,DISABLE_READ_BLOCK},0x0002,0x01,0,FALSE,FALSE},
    //��������
    {0x21000000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    //���ʲ�������
    {0x03300F00,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x01,0,FALSE,FALSE},
    //���ݱ���
    {0x03301000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    //��Կ����
    {0x03301200,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x03,0,FALSE,FALSE},
    
    //�㶨�ų�����
    {0x03350000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x01,0,FALSE,FALSE},
    //���ɿ�������
    {0x03360000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x04,0,FALSE,FALSE},
    //��Դ�쳣
    {0x03370000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x02,0,FALSE,FALSE},
    
    //������Դʧ��
    {0x03060000,{DEFAULT_RECORD_FLAG,DISABLE_READ_BLOCK},0x000A,0x02,0,FALSE,FALSE},

	
	


	

};
#ifdef __SOFT_SIMULATOR__
#define EVENT_GRADE_INFO
#endif
#if 1
/*+++
  ���ܣ�׼������¼����ڳ�����飺F105���������Ƿ����ã���1-4������
  ������
         INT16U meter_idx,              �������
         METER_DOCUMENT *meter_doc,     �����
         INT8U *frame                   �������ݱ���
         INT32U *item                   ���������ݱ�ʶ
         INT8U event_level              1 2 3 4
  ���أ�
         TRUE  ��Ҫ����
         FALSE �������
  ������

---*/
BOOLEAN prepare_plc_cycle_meter_event_item(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    //read_meter_event_ctrl read_ctrl;
    INT16U 	meter_idx;
    //INT8U 	flag[4];

	meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
	
    if(get_read_mask_event_cycle_meter(meter_idx))
    {
    	if( read_params->control.loop == 1 )
        {
        if(prepare_plc_read_meter_event_level_item(read_params,frame,frame_len,1))return true;
        if(prepare_plc_read_meter_event_level_item(read_params,frame,frame_len,2))return true;
        if(prepare_plc_read_meter_event_level_item(read_params,frame,frame_len,3))return true;
        if(prepare_plc_read_meter_event_level_item(read_params,frame,frame_len,4))return true;
        }
        else
        {
        read_params->control.loop = 1;
        if(prepare_plc_read_meter_event_level_item(read_params,frame,frame_len,1))return true;
        if(prepare_plc_read_meter_event_level_item(read_params,frame,frame_len,2))return true;
        if(prepare_plc_read_meter_event_level_item(read_params,frame,frame_len,3))return true;
        if(prepare_plc_read_meter_event_level_item(read_params,frame,frame_len,4))return true;
        //����Ƿ��ӻ���Сʱ���ڣ���Ҫ��ʧ�ܣ�������ջ����£��óɹ���
        read_params->control.loop = check_read_meter_event_cycle(meter_idx);
        }

    }
	else
	{
		if(get_bit_value(read_meter_grade_flag_level_1,READ_FLAG_BYTE_NUM,meter_idx) )
			clr_bit_value(read_meter_grade_flag_level_1,READ_FLAG_BYTE_NUM,meter_idx);
		if(get_bit_value(read_meter_grade_flag_level_2,READ_FLAG_BYTE_NUM,meter_idx) )
			clr_bit_value(read_meter_grade_flag_level_2,READ_FLAG_BYTE_NUM,meter_idx);
		if(get_bit_value(read_meter_grade_flag_level_3,READ_FLAG_BYTE_NUM,meter_idx) )
			clr_bit_value(read_meter_grade_flag_level_3,READ_FLAG_BYTE_NUM,meter_idx);
		if(get_bit_value(read_meter_grade_flag_level_4,READ_FLAG_BYTE_NUM,meter_idx) )
			clr_bit_value(read_meter_grade_flag_level_4,READ_FLAG_BYTE_NUM,meter_idx);
	}
    return FALSE;
}
#else
/*+++
  ���ܣ�׼������¼����ڳ�����飺F105���������Ƿ����ã���1-4������
  ������
         INT16U meter_idx,              �������
         METER_DOCUMENT *meter_doc,     �����
         INT8U *frame                   �������ݱ���
         INT32U *item                   ���������ݱ�ʶ
         INT8U event_level              1 2 3 4
  ���أ�
         TRUE  ��Ҫ����
         FALSE �������
  ������

---*/
BOOLEAN prepare_plc_cycle_meter_event_item(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
	read_meter_event_ctrl read_ctrl;
    INT16U 	meter_idx;
	INT8U 	flag[4];
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
	if (get_bit_value(read_meter_grade_flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) 
	{
		#ifdef EVENT_GRADE_INFO
		if(meter_idx == 2)
		{
		    snprintf(info,100,"***��Ƴ�����־read_meter_grade_flag��ȫ00***");
		    debug_println_ext(info);               
		}
		#endif
		return FALSE;
	}

	if (check_is_all_ch(read_params->read_mask.meter_event_grade,READ_MASK_BYTE_NUM_METER_EVENT_GRADE,0x00))
    {
    	#ifdef EVENT_GRADE_INFO
		if(meter_idx == 2)
		{
		    snprintf(info,100,"***һ������meter_event_grade��ȫ00***");
		    debug_println_ext(info);               
		}
		#endif
        clr_bit_value(read_meter_grade_flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }
	
    if(get_read_mask_event_cycle_meter(meter_idx))
    {
    	
        read_params->control.loop = 1;
        if(prepare_plc_read_meter_event_item(read_params,frame,frame_len,1))  return true;
        if(prepare_plc_read_meter_event_item(read_params,frame,frame_len,2))return true;
        if(prepare_plc_read_meter_event_item(read_params,frame,frame_len,3))return true;
        if(prepare_plc_read_meter_event_item(read_params,frame,frame_len,4))return true;
    }
	//mem_set(flag,sizeof(flag),0xFF);
	//fwrite_array(meter_idx,PIM_READ_METER_GRADE_EVENT_CTRL_FROM_FILE_FLAG,flag,sizeof(flag));//���ٴӵ���ļ���ȡ����F106�����ļ���ȡ
	//fwrite_array(meter_idx,PIM_SAVE_METER_GRADE_EVENT_REPORT_TMP_DATA_FLAG,flag,1);
	#ifdef EVENT_GRADE_INFO
	if(meter_idx == 2)
	{
	    snprintf(info,100,"***��Ҫ���������־read_meter_grade_flag***");
	    debug_println_ext(info);               
	}
	#endif
	clr_bit_value(read_meter_grade_flag,READ_FLAG_BYTE_NUM,meter_idx);
    return FALSE;
}
#endif
/*
=======================================================
//����F106�ķ��������ɶ�Ӧ�ĳ����������ݣ���д�뵽������ȥ

========================================================
*/
void  process_plan_read_item(INT16U plan_id )
{
    INT32U  offset,processed_offset;
    INT32U  item_32u;
    INT32U  item_and_flag;
    INT8U   idx,count,num;
    INT8U   level,idx1;
    INT8U   shift_cnt;//�ƶ��Ĵ���
    INT8U   flag[MAX_METER_EVENT_ITEM_LEVEL_COUNT/8];
    PARAM_F106 f106[MAX_METER_EVENT_ITEM_LEVEL_COUNT];
    PARAM_F106 tmp_f106;
    MeterEventReadData readList[MAX_METER_EVENT_ITEM_LEVEL_COUNT];
    BOOLEAN is_find;
    INT8U   event_num;
    count = 0;
    offset = sizeof(PARAM_F106) * MAX_METER_EVENT_ITEM_COUNT * (plan_id-1);
    offset += PIM_PARAM_F106;
    
    for(level=0;level<MAX_METER_EVENT_LEVEL;level++)//ÿ����������ĸ��¼��ȼ�
    {		
        count = 0;
        num = 0;
        mem_set(f106[0].value,sizeof(f106),0x00);
        mem_set(readList,sizeof(readList),0x00);
        // 1 ��ȡlevel��Ӧ�ĳ�������
        for(idx=0;idx<MAX_METER_EVENT_ITEM_COUNT;idx++)
        {
            if(count>= MAX_METER_EVENT_ITEM_LEVEL_COUNT) break;
            fread_array(FILEID_METER_EVENT_PARAM,offset+idx*sizeof(PARAM_F106),tmp_f106.value,sizeof(PARAM_F106));
            if (tmp_f106.level != (level+1) ) continue;
            mem_cpy(f106[count].value,tmp_f106.value,sizeof(PARAM_F106));
            count++;
        }
        if(count == 0) goto write_data;//�� level �޳�������
        
        //level�г���������Ҫ����
        num = 0;
        event_num = MAX_METER_EVENT_ITEM_LEVEL_COUNT/8;
        mem_set(flag,event_num,0xFF);
        mem_set(readList,sizeof(readList),0x00);
        // 2 ���ڻ�ȡ��ͬһ�¼��ȼ������ݽ�������
        for(idx1 = 0;idx1<sizeof(meterEventDataList)/sizeof(MeterEventReadData);idx1++)
        {
            for(idx=0;idx<count;idx++)
            {
                if(get_bit_value(flag,event_num,idx) == 0) continue;
                item_32u = bin2_int32u(f106[idx].item);
                if(meterEventDataList[idx1].item == item_32u)
                {				
                    readList[num].item = meterEventDataList[idx1].item;
                    readList[num].record_primary_item = TRUE;
                    //readList[num].rec_flag[1] |= 0x80000000;
                    num++;
                    clr_bit_value(flag,event_num,idx);//�Ѿ������
                    break;
                }
            }
            if(idx >= count) continue;//δ�ҵ� meterEventDataList ����Ҫ����������
            
            is_find = FALSE;
            //�¼��ĳ����������Ƿ���Ҫ���ģ�			
            if( (((readList[num-1].item>>24)>=0x10) && ((readList[num-1].item>>24)<=0x20))
            || (readList[num-1].item == 0x03120000) )
            {
                for(idx=0;idx<count;idx++)
                {
                    if(get_bit_value(flag,event_num,idx) == 0) continue;
                    item_32u = bin2_int32u(f106[idx].item);
                    if( ((item_32u & 0xFFFF00FF) == readList[num-1].item) || ((item_32u & 0xFFFF00F0) == readList[num-1].item ) )
                    {
                        shift_cnt = (INT8U)((item_32u>>8)& 0x000000FF);
                        if( (shift_cnt == 0)||(shift_cnt >64) ) 
                        {
                            //clr_bit_value(flag,2,idx);//�˴��Ƿ���Ҫ������ ???
                            continue;//˵��const �б�֧������ ��Ҫ���� ????
                        }
                        is_find = TRUE;
                        readList[num-1].rec_flag[(shift_cnt-1)/32] |= 1<<((shift_cnt-1)%32);
                        readList[num-1].rec_item_cnt++;
                        clr_bit_value(flag,event_num,idx);//�Ѿ������				
                    }
                }
            
            }
            else
            {
                //
                for(idx=0;idx<count;idx++)
                {
                    if(get_bit_value(flag,event_num,idx) == 0) continue;
                    item_32u = bin2_int32u(f106[idx].item);
                    if( (item_32u & 0xFFFFFFF0) == readList[num-1].item )
                    {
                        shift_cnt = (INT8U)(item_32u & 0x000000FF);
                        if( (shift_cnt == 0)||(shift_cnt > 64) ) continue;
                        is_find = TRUE;
                        readList[num-1].rec_flag[(shift_cnt-1)/32] |= 1<<((shift_cnt-1)%32);
                        readList[num-1].rec_item_cnt++;
                        clr_bit_value(flag,event_num,idx);//�Ѿ������				
                    }
                }
            }
            //δ�ҵ���ʹ��Ĭ��
            if(is_find == FALSE)
            {
                mem_cpy((void*)readList[num-1].rec_flag,(void *)meterEventDataList[idx1].rec_flag,MAX_RECORD_ITEM/8);
                //readList[num-1].rec_flag[1] |= 0x80000000;
                readList[num-1].rec_item_cnt = meterEventDataList[idx1].rec_item_cnt;//����
            }
            
        }
        // 3 ������ɣ��Ƿ��в����б��е�
        for(idx=0;idx<count;idx++)
        {
            if(get_bit_value(flag,event_num,idx) == 0) continue;
            item_32u = bin2_int32u(f106[idx].item);
            is_find = FALSE;
            if( ((item_32u>>24)>=0x10) && ((item_32u>>24)<=0x20) )
            {
                item_and_flag = 0xFFFF00FF;
            }
            else if ((item_32u>>16) == 0x0312)
            {
                item_and_flag = 0xFFFF00F0;
            }
            else
                item_and_flag = 0xFFFFFFF0;
            
            for(idx1 = 0;idx1<sizeof(meterEventDataList)/sizeof(MeterEventReadData);idx1++)
            {
                if(meterEventDataList[idx1].item == (item_32u & item_and_flag))
                {
                    is_find = TRUE;
                    clr_bit_value(flag,event_num,idx);//clear
                    break;
                }
            }
            if(is_find) continue;
            readList[num].item = item_32u;
            readList[num].rec_item_cnt = 0;//���賭������������
            readList[num].report_enable = TRUE;
            readList[num].record_primary_item = TRUE;
            //readList[num].rec_flag[1] |= 0x80000000;
            num++;
            clr_bit_value(flag,event_num,idx);//�Ѿ������
            
        }
        // 4 д�뵽�ļ���
        //if( (num >0 ) && (num <= MAX_METER_EVENT_ITEM_LEVEL_COUNT) )
        write_data:
        {
            readList[0].total_cnt = num;
            processed_offset = sizeof(MeterEventReadData) * (MAX_METER_EVENT_ITEM_COUNT * (plan_id-1) + level * MAX_METER_EVENT_ITEM_LEVEL_COUNT);
            processed_offset += PIM_PARAM_F106_PROCESSED;
            fwrite_array(FILEID_METER_EVENT_PARAM,processed_offset,(INT8U *)readList,sizeof(MeterEventReadData)*MAX_METER_EVENT_ITEM_LEVEL_COUNT);
        }
    
    }
}


BOOLEAN get_item_mask(READ_PARAMS *read_params,INT8U plan_id,INT8U event_level,INT8U idx)
{
	INT32U 	offset;
	METER_EVENT_ITEM_CTRL item_ctrl;
	INT16U	meter_idx;
	MeterEventReadData  meter_event_item_read_ctrl;

    mem_set((void *)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL),0x00);
	mem_set((void *)&meter_event_item_read_ctrl,sizeof(MeterEventReadData),0x00);
	if(read_params->event_item_ctrl.valid_flag == 0xAA)//��Ҫ�����������ݣ����ô�F106���ߴӴ洢�ļ�����ȡ��������AA�������
		return TRUE;
	meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
	// 1 ���ļ�����ȡ����
	fread_array(FILEID_EVENT_GRADE_READ_ITEM_CTRL,PIM_READ_EVENT_GRADE_READ_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL));
	if(item_ctrl.valid_flag == 0x55)//���ļ�����ȡ��֮���´λ���д��֮ǰ���洢��������Ч
	{
		mem_cpy((INT8U *)&read_params->event_item_ctrl,(INT8U *)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL));
		read_params->event_item_ctrl.valid_flag = 0xAA;//��������������ĳ�����
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"***���ļ�����ȡ���ݴ����ݣ�׼����������,������=%02d***",
	            meter_idx);
	    debug_println_ext(info);               
		#endif
		return TRUE;
	}
	//  2 F106��ȡ����
	offset = sizeof(MeterEventReadData) * (MAX_METER_EVENT_ITEM_COUNT * (plan_id-1) + (event_level-1) * MAX_METER_EVENT_ITEM_LEVEL_COUNT+idx);
	offset += PIM_PARAM_F106_PROCESSED;
	fread_array(FILEID_METER_EVENT_PARAM,offset,(INT8U *)&meter_event_item_read_ctrl,sizeof(MeterEventReadData));

	if( (check_is_all_FF((INT8U *)&meter_event_item_read_ctrl,sizeof(MeterEventReadData)) == TRUE) )
	{
		mem_set((INT8U*)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL),0x00);
	}
	else
	{
		//
		mem_set((INT8U*)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL),0x00);
		item_ctrl.item = meter_event_item_read_ctrl.item;
		item_ctrl.report_flag = meter_event_item_read_ctrl.report_enable;
		item_ctrl.main_record = meter_event_item_read_ctrl.record_primary_item;//��Ҫ������
		item_ctrl.EventCtrl.rec_item_cnt = meter_event_item_read_ctrl.rec_item_cnt;//
		mem_cpy((INT8U *)item_ctrl.EventCtrl.secondary_record_flag,(INT8U *)meter_event_item_read_ctrl.rec_flag,8);
		mem_cpy((INT8U *)item_ctrl.EventCtrl.copy_flag,(INT8U *)item_ctrl.EventCtrl.secondary_record_flag,8);
	}
	mem_cpy((INT8U *)&read_params->event_item_ctrl,(INT8U *)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL));
	return TRUE;
}
#if 1
BOOLEAN prepare_plc_read_meter_event_level_item(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len,INT8U event_level)
{
	INT8U 	*meter_event_read_flag = NULL;
	INT16U 	meter_idx;
	
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
	switch(event_level)
	{
		case 1:
			meter_event_read_flag = read_meter_grade_flag_level_1;
			break;
		case 2:
			meter_event_read_flag = read_meter_grade_flag_level_2;
			break;
		case 3:
			meter_event_read_flag = read_meter_grade_flag_level_3;
			break;
		case 4:
			meter_event_read_flag = read_meter_grade_flag_level_4;
			break;
		default:
			return FALSE;
	}
	if (get_bit_value(meter_event_read_flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) 
	{
		return FALSE;
	}
	//if((event_level == 0) || (event_level > MAX_METER_EVENT_LEVEL) ) return FALSE;
	if (check_is_all_ch(read_params->read_mask.meter_event_grade+(event_level-1)*MAX_METER_EVENT_ITEM_FLAG_COUNT,MAX_METER_EVENT_ITEM_FLAG_COUNT,0x00))
    {   	
        clr_bit_value(meter_event_read_flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }
	if(prepare_plc_read_meter_event_item(read_params,frame,frame_len,event_level) ) return TRUE;

	clr_bit_value(meter_event_read_flag,READ_FLAG_BYTE_NUM,meter_idx);//�������
    return FALSE;
}
BOOLEAN prepare_plc_read_meter_event_item(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len,INT8U event_level)
{
    INT32U 	item_32u;
	//INT32U	cur_rec_flag[2];
	INT32U	tmp_item;//,save_item;
	//INT32U 	offset;
	
	//read_meter_event_ctrl readMeterEventCtrl;
	//MeterEventReadData  meter_event_read_data_ctrl;
    INT16U 	meter_idx;
    INT8U 	plan_id,idx;//item_count,
	INT8U	pos;
	//INT8U	flag[2];
	//INT8U	file_idx;
	
    plan_id = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

	if(read_params->meter_doc.protocol != GB645_2007) 
	{
		// ��ʱ��֧��97��
		return FALSE;
	}
    if((event_level == 0) ||(event_level > MAX_METER_EVENT_LEVEL)) 
	{
		return FALSE;
    }
	if(check_is_all_ch(read_params->read_mask.meter_event_grade+(event_level-1)*MAX_METER_EVENT_ITEM_FLAG_COUNT,MAX_METER_EVENT_ITEM_FLAG_COUNT,0x00) == TRUE)//�������� ȫ00��������
	{
		return FALSE;
	}
    //��ȡ�������
    fread_array(meter_idx,PIM_METER_F105,&plan_id,1);
    if( (plan_id == 0) || (plan_id > MAX_METER_EVENT_PLAN_COUNT) )
	{
		return FALSE;
    }

	if(read_params->meter_phase > 3 )
	{
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"=== read_params->meter_phase is wrong��PHASE= %02d ====",
	            read_params->meter_phase);
	    debug_println_ext(info);               
		#endif
		return FALSE;
	}
	#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"***׼����������****");
	    debug_println_ext(info);               
	#endif
	#ifdef EVENT_GRADE_INFO
    snprintf(info,100,"===�¼��ȼ�����λ��Ϣ : level = %02d,phase = %02d,meter_idx = %02d ====",
            event_level,read_params->meter_phase,meter_idx);
    debug_println_ext(info);               
	#endif	
	//�����read_params�У���������ϱ��Ļ������Ӷ���������������ˣ�������ȡ�������룬����������Ч�Ļ���
	//˵����Ҫ����������
	for(idx=0;idx<MAX_METER_EVENT_ITEM_LEVEL_COUNT;idx++)
	{
		if( get_bit_value(read_params->read_mask.meter_event_grade+(event_level-1)*MAX_METER_EVENT_ITEM_FLAG_COUNT,MAX_METER_EVENT_ITEM_FLAG_COUNT,idx) )
		{	
			
			//���ɳ������������־			
			get_item_mask(read_params,plan_id, event_level, idx);

			//��¼��Ϣ
			read_params->event_item_ctrl.event_idx = idx;
			read_params->event_item_ctrl.level= event_level;
			
			if(read_params->event_item_ctrl.main_record)
			{
				item_32u = read_params->event_item_ctrl.item;
				#ifdef EVENT_GRADE_INFO
			    snprintf(info,100,"***��Ҫ�����item = 0x%08X****",
			            item_32u);
			    debug_println_ext(info);               
				#endif
				goto prepare_record;
			}
			else//������¼�Ⱥ���������
			{
				if(check_is_all_ch((INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8,0x00) == FALSE)
				{
					tmp_item = read_params->event_item_ctrl.item;//
					for(pos=0;pos<(MAX_RECORD_ITEM);pos++)  //����������鳭�� 2015-11-24 �Ϻ� MAX_RECORD_ITEM-1
					{
						if( get_bit_value((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,8,pos) )
						{
							read_params->event_item_ctrl.secondary_item_idx = pos;
							if( ((tmp_item>>24)>=0x10) && ((tmp_item>>24)<=0x20) )	
							{
                                #if(defined __PROVICE_SHANGHAI__)  //2015-11-24 �Ϻ�����������
									if(pos == 0)
									{
										item_32u = (tmp_item+1);
									}
									else if(pos == 0x3F)
									{
										item_32u = (tmp_item | 0x0000FF00);
									}
									else
									{
										item_32u = (tmp_item | (pos+1)<<8) + (INT8U)(read_params->event_item_ctrl.EventCtrl.need_add_cnt & 0x0F);							
									}
								#elif ( (defined __PROVICE_JIBEI__) || (defined __PROVICE_SHAANXI__) )
									if(pos == 0x3F)
									{
										item_32u = (tmp_item | 0x0000FF00) + (INT8U)(read_params->event_item_ctrl.EventCtrl.need_add_cnt & 0x0F);
									}
									else
									{
										item_32u = (tmp_item | (pos+1)<<8) + (INT8U)(read_params->event_item_ctrl.EventCtrl.need_add_cnt & 0x0F);							
									}
								#else
								//��������
                                     item_32u = (tmp_item | (pos+1)<<8) + (INT8U)(read_params->event_item_ctrl.EventCtrl.need_add_cnt & 0x0F);	
								//(*rec_item) = (tmp_item | (pos+1)<<8) + (INT8U)(item_ctrl.EventCtrl.need_add_cnt & 0x0F);							
								#endif
								//��������
														
							}
							else if( (tmp_item>>16)== 0x0312 ) //��������
							{
								item_32u = ((tmp_item+1) | (pos+1)<<8) + (INT8U)(read_params->event_item_ctrl.EventCtrl.xl_add_cnt[pos] & 0x0F);	
							}
							else
							{
								item_32u = tmp_item | (pos+1);
							}
							#ifdef EVENT_GRADE_INFO
						    snprintf(info,100,"***���������item = 0x%08X need_add_cnt= %02d****",
						            item_32u,read_params->event_item_ctrl.EventCtrl.need_add_cnt);
						    debug_println_ext(info);               
							#endif
							goto prepare_record;
						}
					}				
				}
			}
			clr_bit_value(read_params->read_mask.meter_event_grade+(event_level-1)*MAX_METER_EVENT_ITEM_FLAG_COUNT,MAX_METER_EVENT_ITEM_FLAG_COUNT,idx);
		}
	}
	return FALSE;					
prepare_record:
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"***�������ݽ���****",
	            read_params->meter_phase);
	    debug_println_ext(info);               
		#endif
		read_params->resp_byte_num = 40;
        read_params->read_type = READ_TYPE_CYCLE_METER_EVENT;
        mem_cpy(read_params->item,(INT8U *)&item_32u,4);
       	if(read_params->meter_doc.protocol == GB645_2007)
       	{
       		*frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,item_32u,NULL,0);
       	}
       	else// TODO 97 ��δ����
       	{
       		*frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,item_32u,NULL,0);
       	}	
        return TRUE;

}
#else
BOOLEAN prepare_plc_read_meter_event_item(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len,INT8U event_level)
{
    INT32U 	item_32u;
	INT32U	cur_rec_flag[2];
	INT32U	tmp_item,save_item;
	INT32U 	offset;
	//read_meter_event_ctrl readMeterEventCtrl;
	MeterEventReadData  meter_event_read_data_ctrl;
    INT16U 	meter_idx;
    INT8U 	plan_id,item_count,idx;
	INT8U	pos;
	INT8U	flag[2];
	INT8U	file_idx;


    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);


    if((event_level == 0) ||(event_level > MAX_METER_EVENT_LEVEL))
	{
		return FALSE;
    }
	if(check_is_all_ch(read_params->read_mask.meter_event_grade+(event_level-1)*2,2,0x00) == TRUE)//�������� ȫ00��������
	{
		return FALSE;
	}
    //��ȡ�������
    fread_array(meter_idx,PIM_METER_F105,&plan_id,1);
    if( (plan_id == 0) || (plan_id > MAX_METER_EVENT_PLAN_COUNT) )
	{
		return FALSE;
    }

	if(read_params->meter_phase > 2 )
	{
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"=== read_params->meter_phase is wrong��PHASE= %02d ====",
	            read_params->meter_phase);
	    debug_println_ext(info);               
		#endif
		return FALSE;
	}
	#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"***׼����������****");
	    debug_println_ext(info);               
	#endif
	#ifdef EVENT_GRADE_INFO
    snprintf(info,100,"===�¼��ȼ�����λ��Ϣ : level = %02d,phase = %02d,meter_idx = %02d ====",
            event_level,read_params->meter_phase,meter_idx);
    debug_println_ext(info);               
	#endif	
	//�����read_params�У���������ϱ��Ļ������Ӷ���������������ˣ�������ȡ�������룬����������Ч�Ļ���
	//˵����Ҫ����������
	for(idx=0;idx<MAX_METER_EVENT_ITEM_LEVEL_COUNT;idx++)
	{
		if( get_bit_value(read_params->read_mask.meter_event_grade+(event_level-1)*2,2,idx) )
		{	
			
			//���ɳ������������־			
			get_item_mask(read_params, plan_id, event_level, idx);

			//��¼��Ϣ
			read_params->event_item_ctrl.event_idx = idx;
			read_params->event_item_ctrl.level= event_level;
			
			if(read_params->event_item_ctrl.main_record)
			{
				item_32u = read_params->event_item_ctrl.item;
				#ifdef EVENT_GRADE_INFO
			    snprintf(info,100,"***��Ҫ�����item = 0x%08X****",
			            item_32u);
			    debug_println_ext(info);               
				#endif
				goto prepare_record;
			}
			else//������¼�Ⱥ���������
			{
				if(check_is_all_ch((INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8,0x00) == FALSE)
				{
					tmp_item = read_params->event_item_ctrl.item;//
					for(pos=0;pos<(MAX_RECORD_ITEM-1);pos++)
					{
						if( get_bit_value((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,8,pos) )
						{
							read_params->event_item_ctrl.secondary_item_idx = pos;
							if( ((tmp_item>>24)>=0x10) && ((tmp_item>>24)<=0x1B) )	
							{
								//��������
								item_32u = (tmp_item | (pos+1)<<8) + (INT8U)(read_params->event_item_ctrl.EventCtrl.need_add_cnt & 0x0F);							
							}
							else
							{
								item_32u = tmp_item | (pos+1);
							}
							#ifdef EVENT_GRADE_INFO
						    snprintf(info,100,"***���������item = 0x%08X need_add_cnt= %02d****",
						            item_32u,read_params->event_item_ctrl.EventCtrl.need_add_cnt);
						    debug_println_ext(info);               
							#endif
							goto prepare_record;
						}
					}				
				}
			}
			clr_bit_value(read_params->read_mask.meter_event_grade+(event_level-1)*2,2,idx);
		}
	}
	return FALSE;					
prepare_record:
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"***�������ݽ���****",
	            read_params->meter_phase);
	    debug_println_ext(info);               
		#endif
		read_params->resp_byte_num = 40;
        read_params->read_type = READ_TYPE_CYCLE_METER_EVENT;
        mem_cpy(read_params->item,(INT8U *)&item_32u,4);
       	if(read_params->meter_doc.protocol == GB645_2007)
       	{
       		*frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,item_32u,NULL,0);
       	}
       	else// TODO 97 ��δ����
       	{
       		*frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,item_32u,NULL,0);
       	}	
        return TRUE;

}
#endif
/*+++
  ���ܣ���ñ��ȼ��¼���������
  ������
         INT8U event_level              1 2 3 4
  ���أ�
         TRUE  ��Ҫ����
         FALSE �������
  ������

---*/
INT8U get_plan_read_item(INT8U event_level,INT8U plan_id,INT8U *data)
{
    INT32U offset;
    INT8U idx,count;
    PARAM_F106 f106;

    count = 0;
    offset = sizeof(PARAM_F106) * MAX_METER_EVENT_ITEM_COUNT * (plan_id-1);
    offset += PIM_PARAM_F106;
    for(idx=0;idx<MAX_METER_EVENT_ITEM_COUNT;idx++)
    {
        if(count>= MAX_METER_EVENT_ITEM_LEVEL_COUNT) break;
        fread_array(FILEID_METER_EVENT_PARAM,offset+idx*sizeof(PARAM_F106),f106.value,sizeof(PARAM_F106));
        if (f106.level != event_level) continue;
        mem_cpy(data+count*sizeof(PARAM_F106),f106.value,sizeof(PARAM_F106));
        count++;
    }
    return count;
}
void save_event_grade_mask_flag(READ_PARAMS *read_params)
{
	INT16U 	meter_idx;
	INT8U	plan_id = 0;
	INT8U 	read_mask_meter_event_grade[PIM_READ_METER_GRADE_RECORD_MASK_METER_INFO_LEN];

	mem_set(read_mask_meter_event_grade,sizeof(read_mask_meter_event_grade),0x00);
	meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

	fread_array(meter_idx,PIM_METER_F105,&plan_id,1);
    if( (plan_id == 0) || (plan_id > MAX_METER_EVENT_PLAN_COUNT) )
	{
		return ;
    }
	fread_array(FILEID_METER_GRADE_RECORD_MASK,(meter_idx-1)*PIM_READ_METER_GRADE_RECORD_MASK_METER_INFO_LEN,read_mask_meter_event_grade,PIM_READ_METER_GRADE_RECORD_MASK_METER_INFO_LEN);
	if(check_is_all_ch(read_mask_meter_event_grade+2,PIM_READ_METER_GRADE_RECORD_MASK_INFO_SIZE,0x00) == FALSE )//���д��ȫ0���ˣ�����д��
	{
		
		if(compare_string(read_params->read_mask.meter_event_grade,read_mask_meter_event_grade+2,PIM_READ_METER_GRADE_RECORD_MASK_INFO_SIZE) !=0 )
		{
			#ifdef __SOFT_SIMULATOR__
	    	snprintf(info,100,"=== �����ұ��γ���δ��ɣ���Ҫд�� һ������ ====");
	    	debug_println_ext(info);               
			#endif
			mem_cpy(read_mask_meter_event_grade+2,read_params->read_mask.meter_event_grade,PIM_READ_METER_GRADE_RECORD_MASK_INFO_SIZE);
			fwrite_array(FILEID_METER_GRADE_RECORD_MASK,(meter_idx-1)*PIM_READ_METER_GRADE_RECORD_MASK_METER_INFO_LEN,read_mask_meter_event_grade,PIM_READ_METER_GRADE_RECORD_MASK_METER_INFO_LEN);			
		}
	}
	else
	{
		if(meter_idx == 2)
		{
			#ifdef __SOFT_SIMULATOR__		
			snprintf(info,100,"****����ֵ, flag =0x%02X,level1 = 0x%04X,level2 = 0x%04X,level3 = 0x%04X,level4 = 0x%04X ",
				read_mask_meter_event_grade[0],
				*((INT16U*)&read_mask_meter_event_grade[2]),*((INT16U*)&read_mask_meter_event_grade[6]),
				*((INT16U*)&read_mask_meter_event_grade[10]),*((INT16U*)&read_mask_meter_event_grade[14]));
	    	debug_println_ext(info);
			#endif
		}
	}
	if(read_params->event_item_ctrl.valid_flag == 0xAA)
	{
		//д��
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,150,"***����,д��������룬���=%d,flag=0x%02X,high=0x%08X,low=0x%08X,***",meter_idx,
	            read_params->event_item_ctrl.valid_flag,read_params->event_item_ctrl.EventCtrl.copy_flag[1],
	            read_params->event_item_ctrl.EventCtrl.copy_flag[0]);
	    debug_println_ext(info);               
		#endif
		read_params->event_item_ctrl.valid_flag = 0x55;
		fwrite_array(FILEID_EVENT_GRADE_READ_ITEM_CTRL,PIM_READ_EVENT_GRADE_READ_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&read_params->event_item_ctrl,sizeof(METER_EVENT_ITEM_CTRL));		
	}
	//�����㣬��ֹӰ���������
	mem_set(read_params->read_mask.meter_event_grade,READ_MASK_BYTE_NUM_METER_EVENT_GRADE,0x00);
	mem_set((INT8U *)&read_params->event_item_ctrl,sizeof(METER_EVENT_ITEM_CTRL),0x00);
				
}
INT32U get_meter_cjq_event_item(INT8U idx)
{
    switch(idx)
    {
    case 0:  return 0x04001501;
    case 1:  return 0x19000100;
    case 2:  return 0x03300D00;   //���
    case 3:  return 0x03300100;   //�������
    case 4:  return 0xB212;
    case 5:  return 0xB213;
    case 6:  return 0xB310;   //�������
    case 7:  return 0xC021;   //��

    }
    return 0;
}
/*+++
 ���ܣ�����DL/T645�����ɼ����ĵ���¼��ĳ�������
 ������
        INT8U *frame,     ���Ļ�����
        INT8U *meter_no   ����ַ    ����
        INT32U item,      ������Ŀ    DI3DI2DI1DI0
        INT8U *data,      ����������
        INT8U datalen     �������ݳ���
 ���أ�
        ���ĳ���
---*/
INT8U make_gb645_cjq_meter_event_state_frame(INT8U *frame,INT8U *meter_no,INT8U item,INT8U *data,INT8U datalen)
{
   INT8U idx,pos;
   INT8U cs;

   pos=0;
   frame[pos++] = 0x68;
   frame[pos++] = meter_no[0];
   frame[pos++] = meter_no[1];
   frame[pos++] = meter_no[2];
   frame[pos++] = meter_no[3];
   frame[pos++] = meter_no[4];
   frame[pos++] = meter_no[5];
   frame[pos++] = 0x68;
   frame[pos++] = 0x1E;
   frame[pos++] = 7+datalen;//6�ǵ���ַռ�ֽڣ���ʶҲ�㳤�����棿��
   frame[pos++] = item+0x33;
   frame[pos++] = meter_no[0]+0x33;
   frame[pos++] = meter_no[1]+0x33;
   frame[pos++] = meter_no[2]+0x33;
   frame[pos++] = meter_no[3]+0x33;
   frame[pos++] = meter_no[4]+0x33;
   frame[pos++] = meter_no[5]+0x33;
   for(idx=0;idx<datalen;idx++) frame[pos++]=(data[idx]+0x33);
 //  for(idx=10;idx<pos;idx++) frame[idx]+=0x33;

   cs = 0;
   for(idx=0;idx<pos;idx++) cs+=frame[idx];
   frame[pos++]=cs;
   frame[pos++]=0x16;
   return pos;
}

/*+++
  ���ܣ������ɼ����ڵ���¼�״̬
  ������
        INT16U spot_idx,    �������
        INT32U eeAddr,      ƫ�Ƶ�ַ
        INT8U *params,      ��������
        INT16U itemDataLen  ��������
  ���أ�
        INT16U ʵ�ʲ�������
  ������

---*/
BOOLEAN prepare_plc_read_cjq_event_state(INT16U meter_idx,METER_DOCUMENT *meter_doc,INT8U *frame,INT32U *item)
{
    INT8U frame_pos;
    INT8U data[2];

    frame_pos = 0;
    frame[frame_pos++] = 0x02; //���Գ���
    #ifdef __376_2_2013__
    if(GPLC.DL698_new_protocol_param.rec_delay_flag) frame[frame_pos++] = 0x00; //ͨ����ʱ����Ա�־
    #endif
 //   mem_cpy(GPLC.ADDR_DST,meter_doc->meter_no,6);
 //   GPLC.plc_resp_bytes = 40;

    *item = 0xC3;
    data[0] = 1; //�ӵ�һ����
    data[1] = 8; //��8��

    frame[frame_pos] = make_gb645_cjq_meter_event_state_frame(frame+frame_pos+1, meter_doc->meter_no,*item,data,2);
    return TRUE;

}

/*+++
  ���ܣ�׼��Ҫ�����ɼ����¼������ݱ�ʶ
  ������
        INT16U spot_idx,    �������
        METER_DOCUMENT *meter_doc,   ����
        INT8U *frame, ���Ļ�����
        INT32U *item  ���ݱ�ʶ
  ���أ�
        INT16U ʵ�ʲ�������
  ������

---*/
BOOLEAN prepare_plc_read_report_cjq_event_state(READ_PARAMS *read_params,INT8U *frame,INT32U *item)
{
    INT8U frame_pos;
    INT8U flag,idx,prot;
    INT8U data[12]={0};
    //INT8U pwd[4];
    INT16U meter_idx;

    *item = 0;
    prot = 0;
    flag = 0;    
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
 /*
    if(meter_doc->protocol != GB645_2007)
    {
        GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_report_meter_event = 0;
        return FALSE;
    }
 */       //�ɼ���ģʽ�����Խ���97��
    //flag = 0xFC;
    //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
    if ((flag == 0xFF) || (flag == 0x00))
    {
//        GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_report_cjq_event = 0;
        return FALSE;
    }

AGAIN:

 //   if (flag & 0x02)       //����״̬��Ϣ���ɼ��������ϱ�����״̬��,�ɲ�ȥ����������״̬�֣�ֱ�ӳ����ݱ�ʶ
 //   {
//        *item = 0xC2; //���Ӧ���ֽ�8*4
//        GPLC.plc_resp_bytes = 40;
 //   }
    if (flag & 0x04)   //����
    {
        *item = 0xC4;
    }
    else if (flag & 0x08)   //����
    {
        fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,data,2);//һ�������¼�״̬��ռ1���ֽڣ����и���Լռ1�ֽ�

        if (check_is_all_ch(data+1,1,0x00))
        {
            flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
            goto AGAIN;
        }

        for(idx=0;idx<4;idx++)//�ɼ���֧��4���¼���
        {
            if(get_bit_value(data+1,1,idx))
            {
                if(*data==1)
                {
                    *item = get_meter_cjq_event_item(idx+4);//97��״̬�֣�
                    prot = 1;
                }
                else
                    *item = get_meter_cjq_event_item(idx);
                clr_bit_value(data+1,1,idx);
                break;
            }
        }
        fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+2,data+1,1);

        if (idx >= 4)
        {
            flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
            goto AGAIN;
        }
    }
/*  ����˴����ǲ��ǲ������һ�ν����ж��ˣ�
    else if (flag & 0x10)
    {
        flag &= ~0x10;
        flag |= 0x02|0x04|0x08;
        fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
        *item = 0xC3; //���Ӧ���ֽ�96+12
        GPLC.plc_resp_bytes = 120;
    }
    */
    else
    {
        flag = 0xFF;
        fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
 //       GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_report_cjq_event = 0;//�ɼ����¼���־���
        return FALSE;
    }

    frame_pos = 0;
    frame[frame_pos++] = 0x02; //���Գ���


    if (*item == 0xC4) //������״̬�֣����˵���ٴ�2��״̬�֣�
    {
        fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+2,data,1); //�����ж�ȡһ������Լ��ǰ��������ȡһ��״̬��
            idx=0;
            data[idx] = ~data[idx];  //���״̬�֣���������д������
            data[idx] = data[idx]&0x0F;
        frame[frame_pos] = make_gb645_cjq_meter_event_state_frame(frame+frame_pos+1,read_params->meter_doc.meter_no,*item,data,1); //1���������ĳ���
    }
    else if (*item == 0xC3)
    {
     frame[frame_pos] = make_gb645_cjq_meter_event_state_frame(frame+frame_pos+1,read_params->meter_doc.meter_no,*item,data,1); //1���������ĳ���
    }
    else if (prot==1)
    {
     frame[frame_pos] = make_gb645_1997_frame(frame+frame_pos+1,read_params->meter_doc.meter_no,0x01,*item,NULL,0); //
    }
    else
    {
        frame[frame_pos] = make_gb645_2007_read_frame(frame+frame_pos+1,read_params->meter_doc.meter_no,*item,NULL,0);
    }

    //��ʶ�����ϱ�����¼�������
//    GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_report_cjq_event = 1;  //��1����ʾ�ɼ����ϱ�

    return TRUE;
}

#ifdef __MEXICO_GUIDE_RAIL__
INT32U get_meter_event_item(INT8U idx,INT8U mask_id)
{
    INT32U item = 0x04001600;

    return (item+idx);
}
#else
INT32U get_meter_event_item(INT8U idx,INT8U mask_id)
{
    switch(idx)
    {
    case 0:   //���ɿ�����
        if (mask_id == 0) return 0x03360001;
        else if (mask_id == 255) return 0x03360000;
        break;
    case 1:     //ESAM����
        if (mask_id == 0) return 0x040005FF;
        break;
    case 10:  //������¼�
        if (mask_id == 0) return 0x03300D01;
        else if (mask_id == 255) return 0x03300D00;
        break;
    case 11:  //�����ť���¼�
        if (mask_id == 0) return 0x03300E01;
        else if (mask_id == 255) return 0x03300E00;
        break;
    case 12:  //�㶨�ų�����
        if (mask_id == 0) return 0x03350001;
        else if (mask_id == 255) return 0x03350000;
        break;
    case 13:  //��Դ�쳣
        if (mask_id == 0) return 0x03370001; //��Դ�쳣����
        else if (mask_id == 255) return 0x03370000;
        break;
    case 14:  //��բ�ɹ�
        if (mask_id == 0) return 0x1d000101;
        else if (mask_id == 255) return 0x1d000001;
        break;
    case 15:  //��բ�ɹ�
        if (mask_id == 0) return 0x1e000101;
        else if (mask_id == 255) return 0x1e000001;
        break;
    case 16:   //A��ʧѹ
        #ifdef __EVENT_RECORD_BLOCK__   //2015-11-10
        if (mask_id == 0) return 0x1001FF01;
        #else
        if (mask_id == 0) return 0x10010101;
        else if (mask_id == 1) return 0x10012501;
        #endif
        else if (mask_id == 255) return 0x10010001;
        break;
    case 17:   //A��Ƿѹ
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1101FF01;
        #else
        if (mask_id == 0) return 0x11010101;
        else if (mask_id == 1) return 0x11012501;
        #endif
        else if (mask_id == 255) return 0x11010001;
        break;
    case 18:   //A���ѹ
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1201FF01;
        #else
        if (mask_id == 0) return 0x12010101;
        else if (mask_id == 1) return 0x12012501;
        #endif
        else if (mask_id == 255) return 0x12010001;
        break;
    case 19:   //A��ʧ��
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1801FF01;
        #else
        if (mask_id == 0) return 0x18010101;
        else if (mask_id == 1) return 0x18012101;
        #endif
        else if (mask_id == 255) return 0x18010001;
        break;
    case 20:  //A�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1901FF01;
        #else
        if (mask_id == 0) return 0x19010101;
        else if (mask_id == 1) return 0x19012101;
        #endif
        else if (mask_id == 255) return 0x19010001;
        break;
    case 21:  //A�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1C01FF01;
        #else
        if (mask_id == 0) return 0x1C010101;
        else if (mask_id == 1) return 0x1C011201;
        #endif
        else if (mask_id == 255) return 0x1C010001;
        break;
    case 22:  //A�๦�ʷ���
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1b01FF01;
        #else
        if (mask_id == 0) return 0x1b010101;
        else if (mask_id == 1) return 0x1b011201;
        #endif
        else if (mask_id == 255) return 0x1b010001;
        break;
    case 23:  //A�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1301FF01;
        #else
        if (mask_id == 0) return 0x13010101;
        else if (mask_id == 1) return 0x13012501;
        #endif
        else if (mask_id == 255) return 0x13010001;
        break;
    case 24:  //A�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1A01FF01;
        #else
        if (mask_id == 0) return 0x1A010101;
        else if (mask_id == 1) return 0x1A012101;
        #endif
        else if (mask_id == 255) return 0x1A010001;
        break;
    case 32:   //B��ʧѹ
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1002FF01;
        #else
        if (mask_id == 0) return 0x10020101;
        else if (mask_id == 1) return 0x10022501;
        #endif
        else if (mask_id == 255) return 0x10020001;
        break;
    case 33:   //B��Ƿѹ
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1102FF01;
        #else
        if (mask_id == 0) return 0x11020101;
        else if (mask_id == 1) return 0x11022501;
        #endif
        else if (mask_id == 255) return 0x11020001;
        break;
    case 34:   //B���ѹ
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1202FF01;
        #else
        if (mask_id == 0) return 0x12020101;
        else if (mask_id == 1) return 0x12022501;
        #endif
        else if (mask_id == 255) return 0x12020001;
        break;
    case 35:   //B��ʧ��
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1802FF01;
        #else
        if (mask_id == 0) return 0x18020101;
        else if (mask_id == 1) return 0x18022101;
        #endif
        else if (mask_id == 255) return 0x18020001;
        break;
    case 36:   //B�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1902FF01;
        #else
        if (mask_id == 0) return 0x19020101;
        else if (mask_id == 1) return 0x19022101;
        #endif
        else if (mask_id == 255) return 0x19020001;
        break;
    case 37:   //B�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1C02FF01;
        #else
        if (mask_id == 0) return 0x1C020101;
        else if (mask_id == 1) return 0x1C021201;
        #endif
        else if (mask_id == 255) return 0x1C020001;
        break;
    case 38:   //B�๦�ʷ���
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1b02FF01;
        #else
        if (mask_id == 0) return 0x1b020101;
        else if (mask_id == 1) return 0x1b021201;
        #endif
        else if (mask_id == 255) return 0x1b020001;
        break;
    case 39:   //B�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1302FF01;
        #else
        if (mask_id == 0) return 0x13020101;
        else if (mask_id == 1) return 0x13022501;
        #endif
        else if (mask_id == 255) return 0x13020001;
        break;
    case 40:   //B�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1A02FF01;
        #else
        if (mask_id == 0) return 0x1A020101;
        else if (mask_id == 1) return 0x1A022101;
        #endif
        else if (mask_id == 255) return 0x1A020001;
        break;
    case 48:   //C��ʧѹ
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1003FF01;
        #else
        if (mask_id == 0) return 0x10030101;
        else if (mask_id == 1) return 0x10032501;
        #endif
        else if (mask_id == 255) return 0x10030001;
        break;
    case 49:   //C��Ƿѹ
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1103FF01;
        #else
        if (mask_id == 0) return 0x11030101;
        else if (mask_id == 1) return 0x11032501;
        #endif
        else if (mask_id == 255) return 0x11030001;
        break;
    case 50:   //C���ѹ
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1203FF01;
        #else
        if (mask_id == 0) return 0x12030101;
        else if (mask_id == 1) return 0x12032501;
        #endif
        else if (mask_id == 255) return 0x12030001;
        break;
    case 51:   //C��ʧ��
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1803FF01;
        #else
        if (mask_id == 0) return 0x18030101;
        else if (mask_id == 1) return 0x18032101;
        #endif
        else if (mask_id == 255) return 0x18030001;
        break;
    case 52:   //C�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1903FF01;
        #else
        if (mask_id == 0) return 0x19030101;
        else if (mask_id == 1) return 0x19032101;
        #endif
        else if (mask_id == 255) return 0x19030001;
        break;
    case 53:   //C�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1C03FF01;
        #else
        if (mask_id == 0) return 0x1C030101;
        else if (mask_id == 1) return 0x1C031201;
        #endif
        else if (mask_id == 255) return 0x1C030001;
        break;
    case 54:   //C�๦�ʷ���
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1b03FF01;
        #else
        if (mask_id == 0) return 0x1b030101;
        else if (mask_id == 1) return 0x1b031201;
        #endif
        else if (mask_id == 255) return 0x1b030001;
        break;
    case 55:   //C�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1303FF01;
        #else
        if (mask_id == 0) return 0x13030101;
        else if (mask_id == 1) return 0x13032501;
        #endif
        else if (mask_id == 255) return 0x13030001;
        break;
    case 56:   //C�����
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1A03FF01;
        #else
        if (mask_id == 0) return 0x1A030101;
        else if (mask_id == 1) return 0x1A032101;
        #endif
        else if (mask_id == 255) return 0x1A030001;
        break;
    case 64:  //��ѹ������
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1400FF01;
        #else
        if (mask_id == 0) return 0x14000101;
        else if (mask_id == 1) return 0x14001201;
        #endif
        else if (mask_id == 255) return 0x14000001;
        break;
    case 65:  //����������
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1500FF01;
        #else
        if (mask_id == 0) return 0x15000101;
        else if (mask_id == 1) return 0x15001201;
        #endif
        else if (mask_id == 255) return 0x15000001;
        break;
    case 66:  //��ѹ��ƽ��
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1600FF01;
        #else
        if (mask_id == 0) return 0x16000101;
        else if (mask_id == 1) return 0x16001301;
        #endif
        else if (mask_id == 255) return 0x16000001;
        break;
    case 67:  //������ƽ��
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1700FF01;
        #else
        if (mask_id == 0) return 0x17000101;
        else if (mask_id == 1) return 0x17001301;
        #endif
        else if (mask_id == 255) return 0x17000001;
        break;
    case 68:  //������Դ����
        if (mask_id == 0) return 0x03060001;
        else if (mask_id == 255) return 0x03060000;
        break;
    case 69:  //����
        if (mask_id == 0) return 0x03110001;
        else if (mask_id == 255) return 0x03110000;
        break;
    case 70: //��������
        if (mask_id == 0) return 0x03120101;
        else if (mask_id == 255) return 0x03120000;
        break;
    case 71: //�ܹ�������������
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x1F00FF01;
        #else
        if (mask_id == 0) return 0x1F000101;
        else if (mask_id == 1) return 0x1F000601;
        #endif
        else if (mask_id == 255) return 0x1F000001;
        break;
    case 72: //�������ز�ƽ��
        #ifdef __EVENT_RECORD_BLOCK__
        if (mask_id == 0) return 0x2000FF01;
        #else
        if (mask_id == 0) return 0x20000101;
        else if (mask_id == 1) return 0x20001301;
        #endif
        else if (mask_id == 255) return 0x20000001;
        break;
    case 73:  //��������
        if (mask_id == 0) return 0x21000001;
        else if (mask_id == 255) return 0x21000000;
        break;
    case 74:   //ȫʧѹ
        if (mask_id == 0) return 0x03050001;
        else if (mask_id == 255) return 0x03050000;
        break;
    case 80:
        if (mask_id == 0) return 0x03300001;   //���
        else if (mask_id == 255) return 0x03300000;
        break;
    case 81:
        if (mask_id == 0) return 0x03300101;   //�������
        else if (mask_id == 255) return 0x03300100;
        break;
    case 82:
        if (mask_id == 0) return 0x03300201;   //��������
        else if (mask_id == 255) return 0x03300200;
        break;
    case 83:
        if (mask_id == 0) return 0x03300301;   //�¼�����
        else if (mask_id == 255) return 0x03300300;
        break;
    case 84:
        if (mask_id == 0) return 0x03300401;   //Уʱ
        else if (mask_id == 255) return 0x03300400;
        break;
    case 85:
        if (mask_id == 0) return 0x03300501;   //ʱ�α��� 
        else if (mask_id == 255) return 0x03300500;
        break;
    case 86:
        if (mask_id == 0) return 0x03300601;   //ʱ������ 
        else if (mask_id == 255) return 0x03300600;
        break;
    case 87:
        if (mask_id == 0) return 0x03300701;   //�����ձ�� 
        else if (mask_id == 255) return 0x03300700;
        break;
    case 88:
        if (mask_id == 0) return 0x03300801;   //�ڼ��ձ��
        else if (mask_id == 255) return 0x03300800;
        break;
    case 89:
        if (mask_id == 0) return 0x03300901;   //�й���Ϸ�ʽ��� 
        else if (mask_id == 255) return 0x03300900;
        break;
    case 90:
        if (mask_id == 0) return 0x03300A01;   //�޹���Ϸ�ʽ1��� 
        else if (mask_id == 255) return 0x03300A00;
        break;
    case 91:
        if (mask_id == 0) return 0x03300B01;   //�޹���Ϸ�ʽ2��� 
        else if (mask_id == 255) return 0x03300B00;
        break;
    case 92:
        if (mask_id == 0) return 0x03300C01;   //�����ձ�� 
        else if (mask_id == 255) return 0x03300C00;
        break;
    case 93:
        if (mask_id == 0) return 0x03300F01;   //���ʱ��� 
        else if (mask_id == 255) return 0x03300F00;
        break;
    case 94:
        if (mask_id == 0) return 0x03301001;   //���ݱ��� 
        else if (mask_id == 255) return 0x03301000;
        break;
    case 95:
        if (mask_id == 0) return 0x03301201;   //��Կ����
        else if (mask_id == 255) return 0x03301200;
        break;
    }
    return 0;
}
#endif

#ifdef __PROVICE_JIANGXI__
void save_report_event_data(INT16U meter_idx,INT8U idx_status,EVENT_READ_CTRL* read_ctrl,DATA_04001501* data_040001501)
{
    //
    INT32U offset = 0;
	INT16U event_len = 0;
	INT16U tmp_len = 0;
	report_ctrl reportCtrl;
	union{
        INT8U value[20];
        struct{
            INT8U length;                  //���ݳ���+4
            INT8U item[4];                 //������
            INT8U meter_state_word[15];   //��Լ���� 12+1+2
        };
    }var;

	reportCtrl.value = 0;

    //ֻ���ö�Ӧ״̬��
    mem_set(var.value,sizeof(var),0x00);
	set_bit_value(var.meter_state_word,12,idx_status);
	var.length = 19;
	//04001501
	var.item[0] = 0x01;
	var.item[1] = 0x15;
	var.item[2] = 0x00;
	var.item[3] = 0x04;
	//
	var.meter_state_word[12] = 0xAA;
	var.meter_state_word[13] = read_ctrl->rec_event_cnt;//�仯����
	var.meter_state_word[14] = 0xAA;
	offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001501);
    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,(INT8U*)&event_len,2);
	if  (event_len < 850)
    {
        #ifdef __EVENT_04001501_BEFORE__
        tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
        fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
        reportCtrl.read_tmp_and_save = 1;
        save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
        tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
        #else
		#ifdef __PROVICE_JIANGXI__ // ״̬������
        if ((event_len + var.length + 1) < 850)
        {
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2+event_len,var.value,var.length + 1);
            event_len = var.length + 1;
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.value,11);
            //var.data[0] = var.data[0] + event_len;
            tmp_len = bin2_int16u(var.value)+event_len;
            var.value[0] = (INT8U)(tmp_len&0x00FF);
            var.value[1] = (INT8U)(tmp_len >> 8);//����2�ֽ�
            var.value[10]++;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.value,11);
        }
        else
        {
            //�ȱ�һ��
            tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
            reportCtrl.read_tmp_and_save = 1;
            save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
            tpos_mutexFree(&SIGNAL_TEMP_BUFFER);

			//
            event_len = 9;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2+event_len,var.value,var.length + 1);
            event_len = var.length + 1;
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.value,11);
            tmp_len = event_len + 9;
            var.value[0] = (INT8U)(tmp_len & 0x00FF);
            var.value[1] = (INT8U)(tmp_len >> 8);
            var.value[10] = 1;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.value,11);
        }
		#else
        if ((event_len + var.data_04001501.length + 1) < 850)
        {
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2+event_len,var.data,var.data_04001501.length + 1);
            event_len = var.data_04001501.length + 1;
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.data,11);
            //var.data[0] = var.data[0] + event_len;
            tmp_len = bin2_int16u(var.data)+event_len;
            var.data[0] = (INT8U)(tmp_len&0x00FF);
            var.data[1] = (INT8U)(tmp_len >> 8);//����2�ֽ�
            var.data[10]++;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.data,11);
        }
        else
        {
            tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
            reportCtrl.read_tmp_and_save = 1;
            save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
            tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
            
            event_len = 9;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2+event_len,var.data,var.data_04001501.length + 1);
            event_len = var.data_04001501.length + 1;
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.data,11);
            tmp_len = event_len + 9;
            var.data[0] = (INT8U)(tmp_len & 0x00FF);
            var.data[1] = (INT8U)(tmp_len >> 8);
            var.data[10] = 1;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.data,11);
        }
		#endif
        event_len = tmp_len;
        tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
        fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
        reportCtrl.read_tmp_and_save = 1;
        save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
        tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
        #endif

        //copy �������ǰ��Ȼ��ȫ00 �����ݳ��������FF������11���ֽ����¸���
        mem_cpy(var.meter_state_word,read_ctrl->meter_state_word,12);
		clr_bit_value(var.meter_state_word,12,idx_status);
		if(TRUE == check_is_all_ch(var.meter_state_word,12,0x00))
		{
    		//���ݳ��������ȫFFFF
            event_len = 0xFFFF;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,(INT8U *)&event_len,2);
		}
		else
		{
		    //�˿� ��ַ �����ʶ����Ϣ���ú� ��һ�λ����¼�����
		    offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001501);
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.value,11);
			var.value[0] = 0x09;
			var.value[1] = 0x00;
			var.value[10] = 0x00;//���� 0x00
			fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.value,11);
		}
    }
}
#endif
/*
read_ctrl���Ƴ������Σ�ÿ�γ�����������

*/
#ifdef __MEXICO_GUIDE_RAIL__
INT32U get_meter_event_read_item(EVENT_READ_CTRL* read_ctrl,DATA_04001501* data_040001501,INT16U meter_idx)
{
    INT32U item;
    INT8U idx,idx1;

    item = 0;

    if (read_ctrl->event_count.value == 0)
    {
        return 0;
    }

    for(idx=0;idx<8*MEXICO_RAIL_EVENT_BYTE_CNT;idx++)
    {
        if(get_bit_value(read_ctrl->meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx))
        {
            if(read_ctrl->event_count.mask)
            {
                item = get_meter_event_item(idx,255);
                if (item == 0)
                {
                    /* �����Ϊ0 */
                    read_ctrl->event_count.value = 0;
                    clr_bit_value(read_ctrl->meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx);
                    return 0;
                }
                else
                {
                    return item;
                }
            }
            else
            {
                clr_bit_value(read_ctrl->meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx);
                read_ctrl->event_count.count = 0;
                return 0;
            }
        }
    }
    return item;
}
#else
INT32U get_meter_event_read_item(EVENT_READ_CTRL* read_ctrl,DATA_04001501* data_040001501,INT16U meter_idx)
{
    INT32U item;
    INT8U idx,idx1;

    item = 0;

    if (read_ctrl->event_count.value == 0) return 0;

    for(idx=0;idx<96;idx++)
    {
        if(get_bit_value(read_ctrl->meter_state_word,12,idx))
        {
AGAIN:
            if(read_ctrl->event_count.mask)
            {
                item = get_meter_event_item(idx,255);
                if (item == 0)
                {
                    read_ctrl->event_count.mask = 0;
                }
                else return item;
            }
            
            for(idx1=0;idx1<8;idx1++)
            {
                if(get_bit_value(read_ctrl->event_mask,1,idx1))
                {
                    item = get_meter_event_item(idx,idx1);
                    if (item == 0)
                    {
                        if(read_ctrl->event_count.count > 0)
                        {
                            read_ctrl->event_count.count--;
		                    read_ctrl->rec_event_cnt++;// ++
                        }
                        if(read_ctrl->event_count.count > 0)
                        {
                            //׼������
                            read_ctrl->event_mask[0] = 0xFF;
                            goto AGAIN;
                        }
                        else
                        {
                            //������Ҫ�������ݡ�TODO ??
                            #ifdef __PROVICE_JIANGXI__
                            save_report_event_data(meter_idx,idx,read_ctrl,data_040001501);
							#endif
                            read_ctrl->rec_event_cnt = 0;//Ҳ����
                            clr_bit_value(read_ctrl->meter_state_word,12,idx);
							
                            return 0;
                        }
                    }
                    else
                    {
                        item += read_ctrl->rec_event_cnt;//read_ctrl->event_count.count - 1U;
                        return item;
                    }
                }
            }
        }
    }
    return item;
}
#endif

/*
	input: 

	function:
	author:
	
*/
#ifdef __MEXICO_GUIDE_RAIL__
void get_event_mask_count(EVENT_READ_CTRL* read_ctrl,DATA_04001501* data_040001501)
{
    INT8U idx,count,pos;

    count = 0;
    for(idx=0;idx<8*MEXICO_RAIL_EVENT_BYTE_CNT;idx++)
    {
        if(get_bit_value(data_040001501->meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx))
        {
            count++;
        }
        if(get_bit_value(read_ctrl->meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx))
        {
            if (data_040001501->length > MEXICO_RAIL_EVENT_BYTE_CNT)
            {
                pos = (MEXICO_RAIL_EVENT_BYTE_CNT-1);
                if (data_040001501->meter_state_word[MEXICO_RAIL_EVENT_BYTE_CNT] == 0xAA)
                {
                    pos = MEXICO_RAIL_EVENT_BYTE_CNT;
                }
                if (((count + pos) < data_040001501->length) && (data_040001501->meter_state_word[count+pos] != 0xAA))
                {
                    if(data_040001501->meter_state_word[count+pos])
                    {
                        read_ctrl->event_count.count = 1;/* ֻ��һ�� */
                        read_ctrl->event_count.mask = 1;
                    }
                    else
                    {
                        read_ctrl->event_count.count = 0;
                        read_ctrl->event_count.mask = 0;
                    }
		            read_ctrl->rec_event_cnt = 0;//����
                    if (read_ctrl->event_count.count > 0)
                    {
                        //read_ctrl->event_mask[0] = 0xFF;
                        break;
                    }
                    else
                    {
                        clr_bit_value(read_ctrl->meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx);
                    }
                }
                else
                {
                    clr_bit_value(read_ctrl->meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx);
                }
            }
            else
            {
                clr_bit_value(read_ctrl->meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx);
            }
        }
    }
}
#else
void get_event_mask_count(EVENT_READ_CTRL* read_ctrl,DATA_04001501* data_040001501)
{
    INT8U idx,count,pos;

    count = 0;
    for(idx=0;idx<96;idx++)
    {
        if(get_bit_value(data_040001501->meter_state_word,12,idx)) count++;
        if(get_bit_value(read_ctrl->meter_state_word,12,idx))
        {
            if (data_040001501->length > 12)
            {
                pos = 11;
                if (data_040001501->meter_state_word[12] == 0xAA) pos = 12;
                if (((count + pos) < data_040001501->length) && (data_040001501->meter_state_word[count+pos] != 0xAA))
                {
                    if(data_040001501->meter_state_word[count+pos] == 0xFF)
                    {
                       read_ctrl->event_count.count = 0;
                    }
                    else
                    {
                        read_ctrl->event_count.count = (data_040001501->meter_state_word[count+pos] > 10) ? 10 : data_040001501->meter_state_word[count+pos];
                    }
		            read_ctrl->rec_event_cnt = 0;//����
                    read_ctrl->event_count.mask = 1;
                    if (read_ctrl->event_count.count > 0)
                    {
                        read_ctrl->event_mask[0] = 0xFF;
                        break;
                    }
                    else
                    {
                        clr_bit_value(read_ctrl->meter_state_word,12,idx);
                    }
                }
                else
                {
                    clr_bit_value(read_ctrl->meter_state_word,12,idx);
                }
            }
            else
            {
                clr_bit_value(read_ctrl->meter_state_word,12,idx);
            }
        }
    }
}
#endif

BOOLEAN prepare_plc_read_report_meter_event_state(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len)
{
    INT32U item;
	INT16U 	meter_idx,offset;
	INT16U 	event_len = 0;
	INT16U  tmp_len;
    //INT8U 	frame_pos;
    INT8U 	idx;//flag,
    //INT8U 	data[12];
    INT8U 	pwd[4];
	report_ctrl reportCtrl;
    EVENT_READ_CTRL read_ctrl;
    union{
        INT8U data[256];
        DATA_04001501 data_04001501;
    }var;
    #ifdef __MEXICO_GUIDE_RAIL__
    INT8U data_645[MEXICO_RAIL_EVENT_BYTE_CNT] = {0};
    #endif

    item = 0;
	
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    #ifdef __READ_OOP_METER__  /*���������զ������������Ҫ�����ϱ�*/
    if(read_params->meter_doc.protocol != GB645_2007)
    {
        return FALSE;
    }
    #endif

    //��·���ϱ���06HF5�У�flag����ֵ��FE���ڴ洢��ʱ��&02��
    //flag = 0xFC;
    //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
    //fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
	fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
    //if ((flag == 0xFF) || (flag == 0x00))
	if ((read_ctrl.ctrl_flag == 0xFF) || (read_ctrl.ctrl_flag == 0x00))
    {
//        GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_report_meter_event = 0;
        return FALSE;
    }
	//DelayNmSec(6000);
AGAIN:

    //if (flag & 0x02)       //����״̬��
    if (read_ctrl.ctrl_flag & 0x02)       //����״̬��
    {
        item = 0x04001501; //���Ӧ���ֽ�96+12
//        GPLC.plc_resp_bytes = 120;
        read_params->resp_byte_num = 120;
    }
//    #ifndef __QGDW_CHECK__
//    else if (read_ctrl.ctrl_flag & 0x04)   //����
//    {
//        item = 0x04001503;
        //mem_cpy(read_params->item,item,4);
//    }
//    #endif
    //else if (flag & 0x08)   //����
    else if (read_ctrl.ctrl_flag & 0x08)   //����
    {
    	mem_set(read_params->item,sizeof(read_params->item),0x00);
        //fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,data,12);
		fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
		
        //if (check_is_all_ch(data,12,0x00))
        #ifdef __MEXICO_GUIDE_RAIL__
        if (check_is_all_ch(read_ctrl.meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,0x00))
        #else
        if (check_is_all_ch(read_ctrl.meter_state_word,12,0x00))
        #endif
        {
            #ifndef __MEXICO_GUIDE_RAIL__ /* ���洢�ϱ�10F10 */
            //flag &= ~0x08;
            //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
            offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001501);
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,(INT8U*)&event_len,2);
            if ( (event_len < 850) && (event_len > 13))
            {
                tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
				reportCtrl.read_tmp_and_save = 1;
                save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
				event_len = 0xFFFF;
                fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,(INT8U*)&event_len,2);
            }
            #endif

            read_ctrl.ctrl_flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,1);
            goto AGAIN;
        }

		item = 0;
        fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL),var.data,sizeof(DATA_04001501));

        #ifdef __MEXICO_GUIDE_RAIL__
        while (check_is_all_ch(read_ctrl.meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,0x00) == FALSE)
        #else
		while (check_is_all_ch(read_ctrl.meter_state_word,12,0x00) == FALSE)
		#endif
        {
            if (read_ctrl.event_count.count == 0)
            {
                get_event_mask_count(&read_ctrl,&(var.data_04001501));
            }
            item = get_meter_event_read_item(&read_ctrl,&(var.data_04001501),meter_idx);
            if(item > 0)
            {
                break;
            }
        }

        #ifdef __MEXICO_GUIDE_RAIL__
        if (check_is_all_ch(read_ctrl.meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,0x00))
        #else
        if (check_is_all_ch(read_ctrl.meter_state_word,12,0x00))
        #endif
        {
            #ifndef __MEXICO_GUIDE_RAIL__ /* ��ͨ��10F10�ϱ� */
            offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001501);
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,(INT8U*)&event_len,2);
           // if ( (event_len < 255) && (event_len > 13))
            if  (event_len < 850)
            {
                #ifdef __EVENT_04001501_BEFORE__
				tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
				reportCtrl.read_tmp_and_save = 1;
                save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
				tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                #else
                if ((event_len + var.data_04001501.length + 1) < 850)
                {
                    fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2+event_len,var.data,var.data_04001501.length + 1);
                    event_len = var.data_04001501.length + 1;
                    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.data,11);
                    //var.data[0] = var.data[0] + event_len;
                    tmp_len = bin2_int16u(var.data)+event_len;
                    var.data[0] = (INT8U)(tmp_len&0x00FF);
					var.data[1] = (INT8U)(tmp_len >> 8);//����2�ֽ�
                    var.data[10]++;
                    fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.data,11);
                }
                else
                {
                    tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
					reportCtrl.read_tmp_and_save = 1;
                    save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
                    tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
        
                    event_len = 9;
                    fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2+event_len,var.data,var.data_04001501.length + 1);
                    event_len = var.data_04001501.length + 1;
                    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.data,11);
					tmp_len = event_len + 9;
                    var.data[0] = (INT8U)(tmp_len & 0x00FF);
					var.data[1] = (INT8U)(tmp_len >> 8);
                    var.data[10] = 1;
                    fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,var.data,11);
                }
                event_len = tmp_len;
				tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
				reportCtrl.read_tmp_and_save = 1;
                save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
				tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                #endif

                event_len = 0xFFFF;
                fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,(INT8U *)&event_len,2);
            }
            #endif

            read_ctrl.ctrl_flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
            goto AGAIN;
        }

        fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
		/*
        for(idx=0;idx<96;idx++)
        {
            if(get_bit_value(data,12,idx))
            {
                *item = get_meter_event_item(idx);
                mem_cpy(read_params->item,item,4);
                if (*item == 0)
                {
                    clr_bit_value(data,12,idx);
                }
                else break;
            }
        }
        fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,data,12);

        if (idx >= 96)
        {
            flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
            goto AGAIN;
        }
        */
    }
//    #ifdef __QGDW_CHECK__
    else if (read_ctrl.ctrl_flag & 0x04)   //����
    {
        item = 0x04001503;
        DelayNmSec(1000);//��ʱһ�£���������̨����ܼ�ⲻ��;
    }
//    #endif
    //else if (flag & 0x10)
	else if (read_ctrl.ctrl_flag & 0x10)
    {
        if(read_params->event_item_ctrl.read_04001501_time > 2)//������ͣ�£���Ҫ��ͣ�ĳ���
        read_ctrl.ctrl_flag &= ~0x10;
        read_ctrl.ctrl_flag |= 0x02|0x04|0x08;
        fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,1);
        item = 0x04001501; //���Ӧ���ֽ�96+12
        //mem_cpy(read_params->item,item,4);
        read_params->event_item_ctrl.read_04001501_time ++;
        read_params->resp_byte_num = 120;
    }
    else
    {
        //flag = 0xFF;
        //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
		read_ctrl.ctrl_flag = 0xFF;
        fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,1);
//        GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_report_meter_event = 0;
        return FALSE;
    }

    //frame_pos = 0;
    //frame[frame_pos++] = 0x02; //���Գ���
	mem_cpy(read_params->item,(INT8U*)&item,4);
    read_params->read_type = READ_TYPE_EVENT_TAIL_REPORT;
    if (item == 0x04001503)
    {
        //fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,data,12);
        //for(idx=0;idx<12;idx++)
        //{
        //    data[idx] = ~data[idx];
        //}
        fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL),var.data,sizeof(DATA_04001501));
        for(idx=0;idx<12;idx++)
        {
            //	#ifdef __PROVICE_HUNAN__
	    //		var.data_04001501.meter_state_word[idx] = 0xFF;//ȫ�����
            //    #else
            var.data_04001501.meter_state_word[idx] = ~var.data_04001501.meter_state_word[idx];
            //    #endif
            #ifdef __MEXICO_GUIDE_RAIL__/* ī���絼��� ȫ������ 180705 */
            var.data_04001501.meter_state_word[idx] = 0x00;
            #endif
        }
        mem_set(pwd,4,0x00);
        pwd[0] = 0x02;
        #ifdef __MEXICO_GUIDE_RAIL__ /* ī���絼���ֻ��2���ֽ� */
        *frame_len = make_gb645_2007_write_frame(frame,read_params->meter_doc.meter_no,item,pwd,var.data_04001501.meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT);
        #else
        *frame_len = make_gb645_2007_write_frame(frame,read_params->meter_doc.meter_no,item,pwd,var.data_04001501.meter_state_word,12);
        #endif
    }
    else
    {
        #ifdef __MEXICO_GUIDE_RAIL__ /* ī���絼���ֻ��2���ֽ� */
        data_645[0] = 0;
        if(item != 0x04001501)
        {
            *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,item,data_645,1);
        }
        else
        #endif
        {
            *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,item,NULL,0);
        }
    }

    //��ʶ�����ϱ�����¼�������
//    GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_report_meter_event = 1;

    return TRUE;
}
/*
	input: 

	function:
	author:
	
*/
void get_ext_event_mask_count(EVENT_READ_CTRL* read_ctrl,DATA_04001507* data_040001507)
{
    INT8U idx,count,pos;

    count = 0;
    for(idx=0;idx<56;idx++)
    {
        if(get_bit_value(data_040001507->meter_state_word,7,idx))
        {
            count++;
        }
        if(get_bit_value(read_ctrl->meter_state_word,7,idx))
        {
            if (data_040001507->length > 7)
            {
                pos = 6;
                if (((count + pos) < data_040001507->length))
                {
                   // read_ctrl->event_count.count = (data_040001507->meter_state_word[count+pos] > 10) ? 10 : data_040001507->meter_state_word[count+pos];
                    read_ctrl->event_count.count = 1;
		            read_ctrl->rec_event_cnt = 0;//����
                    read_ctrl->event_count.mask = 1;
                    if (read_ctrl->event_count.count > 0)
                    {
                        read_ctrl->event_mask[0] = 0xFF;
                        break;
                    }
                    else
                    {
                        clr_bit_value(read_ctrl->meter_state_word,7,idx);
                    }
                }
                else
                {
                    clr_bit_value(read_ctrl->meter_state_word,7,idx);
                }
            }
            else
            {
                clr_bit_value(read_ctrl->meter_state_word,7,idx);
            }
        }
    }
}

INT32U get_meter_ext_event_item(INT8U idx,INT8U mask_id)
{
    switch(idx)
    {
    case 1:   //������쳣
    case 0:
        if (mask_id == 0) return 0x03302000;
        else if (mask_id == 255) return 0x03302001;
        break;
    case 3:   //���෴����
        if (mask_id == 0) return 0x03302001;
        else if (mask_id == 255) return 0x03302000;
        break;
    case 4:   //��������
        if (mask_id == 0) return 0x03302101;
        else if (mask_id == 255) return 0x03302100;
        break;
    case 5:   //��·�迹�쳣
        if (mask_id == 0) return 0x03C30001;
        else if (mask_id == 255) return 0x03C30000;
        break;
    
    default:
        return 0;
    }
    return 0;
}
/*
read_ctrl���Ƴ������Σ�ÿ�γ�����������

*/
INT32U get_meter_ext_event_read_item(EVENT_READ_CTRL* read_ctrl,DATA_04001507* data_040001507,INT16U meter_idx)
{
    INT32U item;
    INT8U idx,idx1;

    item = 0;

    if (read_ctrl->event_count.value == 0) return 0;

    for(idx=0;idx<56;idx++)
    {
        if(get_bit_value(read_ctrl->meter_state_word,7,idx))
        {
AGAIN:
            if(read_ctrl->event_count.mask)
            {
                item = get_meter_ext_event_item(idx,255);
                if (item == 0)
                {
                    read_ctrl->event_count.mask = 0;
                }
                else return item;
            }

            for(idx1=0;idx1<8;idx1++)
            {
                if(get_bit_value(read_ctrl->event_mask,1,idx1))
                {
                    item = get_meter_ext_event_item(idx,idx1);
                    if (item == 0)
                    {
                        if(read_ctrl->event_count.count > 0)
                        {
                            read_ctrl->event_count.count--;
		                    read_ctrl->rec_event_cnt++;// ++ 
                        }
                        if(read_ctrl->event_count.count > 0)
                        {
                            //׼������
                            read_ctrl->event_mask[0] = 0xFF;
                            goto AGAIN;
                        }
                        else
                        {
                            read_ctrl->rec_event_cnt = 0;//Ҳ����
                            clr_bit_value(read_ctrl->meter_state_word,7,idx);
							
                            return 0;
                        }
                    }
                    else
                    {
                        item += read_ctrl->rec_event_cnt;
                        return item;
                    }
                }
            }
        }
    }
    return item;
}
BOOLEAN prepare_plc_read_report_meter_ext_event_state(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len)
{
    INT32U item;
	INT16U 	meter_idx,offset;
	INT16U 	event_len = 0;
	INT16U  tmp_len;
    INT8U 	idx;
    INT8U 	pwd[4];
	report_ctrl reportCtrl;
    EVENT_READ_CTRL read_ctrl;
    union{
        INT8U data[128];
        DATA_04001507 data_04001507;
    }var;

    item = 0;
	
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

	fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
	if ((read_ctrl.ctrl_flag == 0xFF) || (read_ctrl.ctrl_flag == 0x00))
    {
        return FALSE;
    }
AGAIN:
    if (read_ctrl.ctrl_flag & 0x02)       //����״̬��
    {
        item = 0x04001507; //���Ӧ���ֽ�56+7
        read_params->resp_byte_num = 80;
    }
    else if (read_ctrl.ctrl_flag & 0x08)   //����
    {
    	mem_set(read_params->item,sizeof(read_params->item),0x00);
		fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
		
        if (check_is_all_ch(read_ctrl.meter_state_word,7,0x00))
        {
            offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001507);
            fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,(INT8U*)&event_len,2);
            if ( (event_len < 850) && (event_len > 13))
            {
                tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
				reportCtrl.read_tmp_and_save = 1;
                save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
				event_len = 0xFFFF;
                fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,(INT8U*)&event_len,2);
            }

            read_ctrl.ctrl_flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,1);
            goto AGAIN;
        }

		item = 0;
        fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL),var.data,sizeof(DATA_04001507));

		while (check_is_all_ch(read_ctrl.meter_state_word,7,0x00) == FALSE)
        {
            if (read_ctrl.event_count.count == 0)
            {
                get_ext_event_mask_count(&read_ctrl,&(var.data_04001507));
            }
            item = get_meter_ext_event_read_item(&read_ctrl,&(var.data_04001507),meter_idx);
            if(item > 0) break;
        }

        if (check_is_all_ch(read_ctrl.meter_state_word,7,0x00))
        {
            offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001507);
            fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,(INT8U*)&event_len,2);
           // if ( (event_len < 255) && (event_len > 13))
            if  (event_len < 850)
            {
                #ifdef __EVENT_04001501_BEFORE__
				tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
				reportCtrl.read_tmp_and_save = 1;
                save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
				tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                #else
                if ((event_len + var.data_04001507.length + 1) < 850)
                {
                    fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset+2+event_len,var.data,var.data_04001507.length + 1);
                    event_len = var.data_04001507.length + 1;
                    fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,var.data,11);
                    //var.data[0] = var.data[0] + event_len;
                    tmp_len = bin2_int16u(var.data)+event_len;
                    var.data[0] = (INT8U)(tmp_len&0x00FF);
					var.data[1] = (INT8U)(tmp_len >> 8);//����2�ֽ�
                    var.data[10]++;
                    fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,var.data,11);
                }
                else
                {
                    tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                    fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
					reportCtrl.read_tmp_and_save = 1;
                    save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
                    tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
        
                    event_len = 9;
                    fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset+2+event_len,var.data,var.data_04001507.length + 1);
                    event_len = var.data_04001507.length + 1;
                    fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,var.data,11);
					tmp_len = event_len + 9;
                    var.data[0] = (INT8U)(tmp_len & 0x00FF);
					var.data[1] = (INT8U)(tmp_len >> 8);
                    var.data[10] = 1;
                    fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,var.data,11);
                }
                event_len = tmp_len;
				tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
				reportCtrl.read_tmp_and_save = 1;
                save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
				tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                #endif

                event_len = 0xFFFF;
                fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,(INT8U *)&event_len,2);
            }

            read_ctrl.ctrl_flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
            goto AGAIN;
        }

        fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
    }
    else if (read_ctrl.ctrl_flag & 0x04)   //����
    {
        item = 0x04001508;
        DelayNmSec(1000);//��ʱһ�£���������̨����ܼ�ⲻ��;
    }
	else if (read_ctrl.ctrl_flag & 0x10)
    {
        if(read_params->event_item_ctrl.read_04001501_time > 2)//������ͣ�£���Ҫ��ͣ�ĳ���
        read_ctrl.ctrl_flag &= ~0x10;
        read_ctrl.ctrl_flag |= 0x02|0x04|0x08;
        fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,1);
        item = 0x04001507; //���Ӧ���ֽ�56+7
        read_params->event_item_ctrl.read_04001501_time ++;
        read_params->resp_byte_num = 80;
    }
    else
    {
		read_ctrl.ctrl_flag = 0xFF;
        fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,1);
        return FALSE;
    }

	mem_cpy(read_params->item,(INT8U*)&item,4);
    read_params->read_type = READ_TYPE_EVENT_TAIL_REPORT_EXT;
    if (item == 0x04001508)
    {
        fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL),var.data,sizeof(DATA_04001507));
        for(idx=0;idx<7;idx++)
        {
            var.data_04001507.meter_state_word[idx] = ~var.data_04001507.meter_state_word[idx];
        }
        mem_set(pwd,4,0x00);
        pwd[0] = 0x02;
        *frame_len = make_gb645_2007_write_frame(frame,read_params->meter_doc.meter_no,item,pwd,var.data_04001507.meter_state_word,7);
    }
    else
    {
        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,item,NULL,0);
    }

    return TRUE;
}
/*+++
 ���ܣ�����DL/T645-2007�����ò�������
 ������
        INT8U *frame,     ���Ļ�����
        INT8U *meter_no   ����ַ    ����
        INT32U item,      ������Ŀ    DI3DI2DI1DI0
        INT8U *data,      ����������
        INT8U datalen     �������ݳ���
 ���أ�
        ���ĳ���
---*/
INT8U make_gb645_2007_write_frame(INT8U *frame,INT8U *meter_no,INT32U item,INT8U *pwd,INT8U *data,INT8U datalen)
{
   INT8U idx,pos;
   INT8U cs;

   pos=0;
   frame[pos++] = 0x68;
   frame[pos++] = meter_no[0];
   frame[pos++] = meter_no[1];
   frame[pos++] = meter_no[2];
   frame[pos++] = meter_no[3];
   frame[pos++] = meter_no[4];
   frame[pos++] = meter_no[5];
   frame[pos++] = 0x68;
   frame[pos++] = 0x14;
   frame[pos++] = 4+4+4+datalen;// 4+4+4+datalen; //�����򳤶ȣ�L = 04H(������) + 04H�����룩+04H�������ߴ��룩+ m(���ݳ���)  ;
   frame[pos++] = item;
   frame[pos++] = item>>8;
   frame[pos++] = item>>16;
   frame[pos++] = item>>24;

   //����
   mem_cpy(frame+pos,pwd,4);
   pos += 4;

   //�����ߴ���  ʹ���ն˵�ַ��Ϊ�����ߴ���
//   mem_set(frame+pos,0,4);
   fread_ertu_params(EEADDR_CFG_DEVID,frame+pos,4);
   pos += 4;

   for(idx=0;idx<datalen;idx++) frame[pos++]=data[idx];
   for(idx=10;idx<pos;idx++) frame[idx]+=0x33;
   cs = 0;
   for(idx=0;idx<pos;idx++) cs+=frame[idx];
   frame[pos++]=cs;
   frame[pos++]=0x16;
   return pos;
}

BOOLEAN check_dayhold_data_right(INT8U *frame)
{
    INT32U z,fl,sum_fl;
    INT8U idx,pos,fl_count;
    BOOLEAN valid_data;

    z = 0;
    fl = 0;
    sum_fl = 0;
    pos =0;
    fl_count = 4;
//    fl_count = frame[pos];

//    if (fl_count > 4) return TRUE;
//    if (fl_count == 0) return TRUE;

    z = bcd2u32(frame,4,&valid_data);
    if(!valid_data) return TRUE;
    pos += 4;

    for(idx=0;idx<fl_count;idx++)
    {
        fl = bcd2u32(frame+pos,4,&valid_data);
        if (valid_data)
        {
            sum_fl += fl;
        }
        pos +=4;
    }

    if (z > sum_fl)
    {
        z = z - sum_fl;
    }
    else
    {
        z = sum_fl - z;
    }

    if (z > 40) return FALSE;
    else return TRUE;
}

INT32U check_settime_dayhold_data(INT32U item)
{
  INT32U return_item;
  switch(item)
  {
     case 0x05060101:
                   return_item = 0x05000101;
                   break;
     case 0x05060201:
                   return_item = 0x05000201;
                   break;
     case 0x05060301:
                   return_item = 0x05000301;
                   break;
     case 0x05060401:
                   return_item = 0x05000401;
                   break;
     case 0x05060501:
                   return_item = 0x05000501;
                   break;
     case 0x05060601:
                   return_item = 0x05000601;
                   break;
     case 0x05060701:
                   return_item = 0x05000701;
                   break;
     case 0x05060801:
                   return_item = 0x05000801;
                   break;
     case 0x05060901:
                   return_item = 0x05000901;
                   break;
     case 0x05060A01:
                   return_item = 0x05000A01;
                   break;
     case 0x01030000:
                   return_item = 0x0103FF00;
                   break;
     case 0x01040000:
                   return_item = 0x0104FF00;
                   break;
     default:
             return_item = item;
              break;
  }
   return return_item;

}
INT32U yunnan_check_dayhold_data(INT32U item)
{
  INT32U return_item;
  switch(item)
  {
     case 0x05060101:
                   return_item = 0x00010000;
                   break;
     case 0x05060201:
                   return_item = 0x05000201;
                   break;
     case 0x05060301:
                   return_item = 0x05000301;
                   break;
     case 0x05060401:
                   return_item = 0x05000401;
                   break;
     case 0x05060501:
                   return_item = 0x05000501;
                   break;

     default:
             return_item = item;
              break;
  }
   return return_item;

}
#ifdef __PLC_REC_VOLTMETER1__
/*+++
  ���ܣ������ѹ������
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
void plc_router_save_voltmeter_data(INT16U meter_idx,INT32U data_item,INT8U *data,INT8U datalen,BOOLEAN is_month)
{
    INT32U offset,ztime,stime,xtime;
    INT32U offset_tmp,offset_stat;
    SPOT_STAT day_stat;
    C2_F27F35_PLC dc2f27f35;
    INT8U idx,td[3],bin_td[3],mon[2];
    BOOLEAN valid_dataz,valid_data1,valid_data2;

    bin_td[0] = byte2BCD(datetime[DAY]);
    bin_td[1] = byte2BCD(datetime[MONTH]);
    bin_td[2] = byte2BCD(datetime[YEAR]);

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td,3);

    offset_tmp = getPassedDays(2000+td[2],td[1],td[0]);
    offset_tmp = (offset_tmp % SAVE_POINT_NUMBER_DAY_HOLD);

    if(is_month)
    {
      mon[0] = datetime[MONTH];
      mon[1] = datetime[YEAR];
    //�¶���ƫ��,�ñ�Ť������
    previous_monthhold_td_BCD(mon,mon);
//    previous_monthhold_td_BCD(mon,mon);

    offset = BCD2byte(mon[0])-1;
    offset *= sizeof(C2_F27F35);
    offset += PIM_C2_F35;
    }
    else
    {
    offset = offset_tmp;
    offset *= sizeof(C2_F27F35);
    offset += PIM_C2_F27;
    }

    fread_array(meter_idx,offset,dc2f27f35.c2f27f35.value,sizeof(C2_F27F35));

    if((data_item == DY_A_HGL) ||(data_item == Y_DY_A_HGL)) idx = 0;
    else if((data_item == DY_B_HGL) || (data_item == Y_DY_B_HGL))idx = 1;
    else if ((data_item == DY_C_HGL) || (data_item == Y_DY_C_HGL))idx = 2;
    //��ѹԽ���������ۼ�ʱ�� ��ѹԽ���������ۼ�ʱ��
    mem_set(dc2f27f35.c2f27f35.value+idx*10,4,0x00);
    //��ѹԽ�������ۼ�ʱ��
    stime = bcd2u32(data+7,3,&valid_data1);
    dc2f27f35.c2f27f35.value[idx*10+4] = stime;
    dc2f27f35.c2f27f35.value[idx*10+4+1] = stime>>8;
    //mem_cpy(dc2f27f35.c2f27f35.value+idx*10+4,data+7,2);
    //��ѹԽ�������ۼ�ʱ��
    xtime = bcd2u32(data+10,3,&valid_data2);
    dc2f27f35.c2f27f35.value[idx*10+6] = xtime;
    dc2f27f35.c2f27f35.value[idx*10+6+1] = xtime>>8;
    //mem_cpy(dc2f27f35.c2f27f35.value+idx*10+6,data+10,2);
    //��ѹ�ϸ����ۼ�ʱ�� = ��ѹ���ʱ�� - Խ����ʱ�� - Խ����ʱ��
    ztime = bcd2u32(data,3,&valid_dataz);
    if(valid_dataz && valid_data1 && valid_data2)
    {
        ztime = ztime - stime - xtime;
        dc2f27f35.c2f27f35.value[idx*10+8] = ztime;
        dc2f27f35.c2f27f35.value[idx*10+8+1] = ztime>>8;
        //ul2bcd(ztime,dc2f27f35.c2f27f35.value+idx*10+8,2);
    }
    else
    {
    mem_set(dc2f27f35.c2f27f35.value+idx*10+8,2,0x00);
    }
    //��ѹ���ֵ
    mem_cpy(dc2f27f35.c2f27f35.value+30+idx*10,data+13,2);
    //��ѹ���ֵ����ʱ��
    mem_cpy(dc2f27f35.c2f27f35.value+30+idx*10+2,data+15,3);
    //��ѹ��Сֵ
    mem_cpy(dc2f27f35.c2f27f35.value+30+idx*10+5,data+19,2);
    //��ѹ��Сֵ����ʱ��
    mem_cpy(dc2f27f35.c2f27f35.value+30+idx*10+7,data+21,3);
    //ƽ����ѹ
    mem_set(dc2f27f35.c2f27f35.value+60+idx*2,2,0x00);

    fwrite_array(meter_idx,offset,dc2f27f35.c2f27f35.value,sizeof(C2_F27F35));
}
/*+++
  ���ܣ������ѹ����������
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
void plc_router_save_vlotmeter_curve_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num)
{
    INT32U offset;
    C_VOLTMETER c_data;
    INT8U idx,midu,num,bin_td[3];
    INT8U td[5];

    td[2] = byte2BCD(datetime[DAY]);
    td[3] = byte2BCD(datetime[MONTH]);
    td[4] = byte2BCD(datetime[YEAR]);
    if(remain_num > 64)//û�����壡������ֻ��Ϊ�˴��������
    remain_num = 23;
    else
    td[1] = remain_num;
    td[0] = 0;
    midu = 15;

    bin_td[0] = td[2];
    bin_td[1] = td[3];
    bin_td[2] = td[4];

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td+2,3);

    phy->data_len = 23;   //һ����ѹ����ռ23�ֽڣ�������д���̶�ֵ��
    phy->offset = PIM_CURVE_V_I;

    offset = get_curve_save_offset(phy,td,midu);

    //�������������ݲ𿪴洢�������ѹ�������������ߵ�ѹ�Ķ�ȡ����һ����һ������У�ÿ���㶼�г���ʱ�ꡣ
    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.Voltage_A,data,2);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);

    td[0] = 0x0F;
    offset = get_curve_save_offset(phy,td,midu);
    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.Voltage_A,data+2,2);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);

    td[0] = 0x1E;
    offset = get_curve_save_offset(phy,td,midu);
    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.Voltage_A,data+4,2);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);

    td[0] = 0x2D;
    offset = get_curve_save_offset(phy,td,midu);
    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.Voltage_A,data+6,2);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);
}
/*+++
  ���ܣ���鱣��ĵ�ѹ����������
  ������
        INT16U meter_idx,    ������

---*/
INT8U check_plc_router_save_vlotmeter_curve_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U remain_num)
{
    INT32U offset;
    C_VOLTMETER c_data;
    INT8U idx,midu,num,tmp,i;
    INT8U td[5],bin_td[3];

    td[2] = byte2BCD(datetime[DAY]);
    td[3] = byte2BCD(datetime[MONTH]);
    td[4] = byte2BCD(datetime[YEAR]);
    if(remain_num > 64)//û�����壡������ֻ��Ϊ�˴��������
    remain_num = 23;
    else
    td[1] = remain_num;
    td[0] = 0;
    midu = 15;

    bin_td[0] = td[2];
    bin_td[1] = td[3];
    bin_td[2] = td[4];

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td+2,3);

    phy->data_len = 23;   //һ����ѹ����ռ23�ֽڣ�������д���̶�ֵ��
    phy->offset = PIM_CURVE_V_I;

    for(i=0;i<24;i++)
    {
      td[1] = remain_num;

      offset = get_curve_save_offset(phy,td,midu);
      fread_array(meter_idx,offset,c_data.value,7);

      if(compare_string(td,c_data.rec_date,5) == false)
      {
       remain_num ++;
      }
      else
      break;

    }

    return remain_num;
}
INT32U check_month_hold_dyhgl_data(INT32U item)
{
  INT32U return_item;
  switch(item)
  {
     case 0x03100100:
                   return_item = 0x03100101;
                   break;
     case 0x03100200:
                   return_item = 0x03100201;
                   break;
     case 0x03100300:
                   return_item = 0x03100301;
                   break;
     default:
             return_item = item;
              break;
  }
   return return_item;

}
#endif
/*+++
  ���ܣ����油����������
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
void plc_router_save_last_curve_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num)
{
    INT32U offset;
    C_CURVE_PATCH c_data;
    INT8U midu,bin_td[3]; // idx,num,
    INT8U td[5]={0};

    td[2] = byte2BCD(datetime[DAY]);
    td[3] = byte2BCD(datetime[MONTH]);
    td[4] = byte2BCD(datetime[YEAR]);
    if(remain_num > 64)//û�����壡������ֻ��Ϊ�˴��������
    {
        //remain_num = 23;//û���ã����ε�
    }
    else
    {
        td[1] = 24 - (remain_num );         //�ϼ��������й���Ҫ���Ŵ洢
    }
    td[0] = 0;
    midu = 60;

    bin_td[0] = td[2];
    bin_td[1] = td[3];
    bin_td[2] = td[4];

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td+2,3);

    phy->data_len = 0x15;   //һ����ѹ����ռ21�ֽڣ�������д���̶�ֵ��
    phy->offset = PIM_CURVE_ZFXYWG ;

    offset = get_curve_save_offset(phy,td,midu);

    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.CURVE_YWG,data,4);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);

}
/*+++
  ���ܣ���ѯ��������������
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
void check_plc_router_save_last_curve_data(INT16U meter_idx,READ_WRITE_DATA *phy,READ_PARAMS *read_params)
{
    INT32U offset;
    C_CURVE_PATCH c_data;
    INT8U midu,bin_td[3];
    INT8U i,remain_num,td[5];

    remain_num = read_params->patch_num; //д�Ĳ��ã���ͳһ��

    td[2] = byte2BCD(datetime[DAY]);
    td[3] = byte2BCD(datetime[MONTH]);
    td[4] = byte2BCD(datetime[YEAR]);
    if(remain_num > 64)//û�����壡������ֻ��Ϊ�˴��������
    {
        remain_num = 23;
    }
    else
    {
        td[1] = 24 - (remain_num );         //�ϼ��������й���Ҫ���Ŵ洢
    }
    td[0] = 0;
    midu = 60;

    bin_td[0] = td[2];
    bin_td[1] = td[3];
    bin_td[2] = td[4];

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td+2,3);

    phy->data_len = 0x15;   //һ����ѹ����ռ21�ֽڣ�������д���̶�ֵ��
    phy->offset = PIM_CURVE_ZFXYWG ;

    for(i=0;i<24;i++)
    {
        td[1] = 24 - remain_num;
  
        offset = get_curve_save_offset(phy,td,midu);
        fread_array(meter_idx,offset,c_data.value,9);
  
        if(compare_string(td,c_data.rec_date,5) == false)
        {
            remain_num ++;
            read_params->item_add_num ++;
        }
        else
        {
            break;
        }

    }
   read_params->patch_num = remain_num;

}
/*+++
  ���ܣ����油����������
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
void plc_router_save_last_gx_load_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num)
{
    INT32U offset;
    C_CURVE_PATCH c_data;
    INT8U midu,bin_td[3];//idx,num,
    INT8U td[5];

    td[2] = byte2BCD(datetime[DAY]);
    td[3] = byte2BCD(datetime[MONTH]);
    td[4] = byte2BCD(datetime[YEAR]);
    if(remain_num > 64)//û�����壡������ֻ��Ϊ�˴��������
    remain_num = 23;
    else
    td[1] = 4*remain_num;
    td[0] = 0;
    midu = 60;

    bin_td[0] = td[2];
    bin_td[1] = td[3];
    bin_td[2] = td[4];

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td+2,3);

    phy->data_len = 21;   //һ����ѹ����ռ23�ֽڣ�������д���̶�ֵ��
    phy->offset = PIM_CURVE_ZFXYWG;

    offset = get_curve_save_offset(phy,td,midu);

    //�������������ݲ𿪴洢�������ѹ�������������ߵ�ѹ�Ķ�ȡ����һ����һ������У�ÿ���㶼�г���ʱ�ꡣ
    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.CURVE_YWG,data,4);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);

    td[1] = 4*remain_num +1;
    offset = get_curve_save_offset(phy,td,midu);
    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.CURVE_YWG,data+4,4);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);

    td[1] = 4*remain_num +2;
    offset = get_curve_save_offset(phy,td,midu);
    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.CURVE_YWG,data+8,4);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);

    td[1] = 4*remain_num +3;
    offset = get_curve_save_offset(phy,td,midu);
    mem_cpy(c_data.rec_date,td,5);
    mem_cpy(c_data.CURVE_YWG,data+12,4);
    fwrite_array(meter_idx,offset,c_data.value,phy->data_len);

}
/*+++
  ���ܣ���鱣��Ĳ�����������
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
INT8U check_plc_router_save_last_gx_load_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U remain_num)
{
    INT32U offset;
    C_CURVE_PATCH c_data;
    INT8U midu,bin_td[3];//num,idx,
    INT8U i;
	INT8U td[5]={0};

    mem_set(c_data.value,sizeof(C_CURVE_PATCH),0x00);
    td[2] = byte2BCD(datetime[DAY]);
    td[3] = byte2BCD(datetime[MONTH]);
    td[4] = byte2BCD(datetime[YEAR]);
    if(remain_num > 64)//û�����壡������ֻ��Ϊ�˴��������
    {
        //���ֱ�������ţ�Ҫ��������Ķ����ر��
        //remain_num = 23;//�����θ�ֵû����
    }
    else
    {
        td[1] = 4*remain_num;
    }
    td[0] = 0;
    midu = 60;

    bin_td[0] = td[2];
    bin_td[1] = td[3];
    bin_td[2] = td[4];

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td+2,3);

    phy->data_len = 21;   //һ����ѹ����ռ23�ֽڣ�������д���̶�ֵ��
    phy->offset = PIM_CURVE_ZFXYWG;

    for(i=0;i<6;i++)
    {
        td[1] = remain_num*4;
        
        offset = get_curve_save_offset(phy,td,midu);
        fread_array(meter_idx,offset,c_data.value,9);
        
        if(compare_string(td,c_data.rec_date,5) == false)
        {
            remain_num ++;
        }
        else
        {
            break;
        }
    
    }

    return remain_num;

}
/*+++
  ���ܣ����油����������
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
void plc_router_save_last_load_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num)
{
    INT32U offset;
    C_VOLTMETER_BLOCK v_data;
    C_POWER_BLOCK gl_data;
    INT8U midu,bin_td[3];//idx,num,
    INT8U td[5];

    td[2] = byte2BCD(datetime[DAY]);
    td[3] = byte2BCD(datetime[MONTH]);
    td[4] = byte2BCD(datetime[YEAR]);
//    if(remain_num > 64)//û�����壡������ֻ��Ϊ�˴��������
//    remain_num = 23;
//    else
    td[1] = remain_num ;         //
    td[0] = 0;
    midu = 60;

    bin_td[0] = td[2];
    bin_td[1] = td[3];
    bin_td[2] = td[4];

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td+2,3);

    if(phy->phy  == FHJL1_DY_DL_PL)
    {
      phy->data_len = 23;   //�������˼�ǵ�ѹ������һ��ռ��23�Լ���8+15,8��3+5��15��6�ֽڵ�ѹ��9�ֽڵ�����
                           //�洢��offsetȷʵʹ����һ������ȡ��ʱ��ᴦ��
      phy->offset = PIM_CURVE_V_I ; //��
      mem_set(v_data.value,phy->data_len,REC_DATA_IS_DEFAULT);

      offset = get_curve_save_offset(phy,td,midu);
      mem_cpy(v_data.rec_date,td,5);
//      buffer_bcd_to_bin(data+5,data+5,3); //����ʱ���ʽ����bcd��ʽ
       buffer_bcd_to_bin(data+4,data+4,4);
       if(compare_string(td,data+3,5)==0)
       {
        mem_cpy(v_data.Block,data+8,15);
       }
     fwrite_array(meter_idx,offset,v_data.value,phy->data_len);

    }
    //�����ɼ�¼2
    if(phy->phy  == FHJL2_YG_WG_GL)
    {
      phy->data_len = 37;
      phy->offset = PIM_CURVE_GL ; //

      mem_set(gl_data.value,phy->data_len,REC_DATA_IS_DEFAULT);
      offset = get_curve_save_offset(phy,td,midu);
      mem_cpy(gl_data.rec_date,td,5);
     // buffer_bcd_to_bin(data+5,data+5,3);
      buffer_bcd_to_bin(data+4,data+4,4);
       if(compare_string(td,data+3,5)==0)
       {
        mem_cpy(gl_data.Block,data+9,24);
       }

      fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);

    }
     //�����ɼ�¼3
    if(phy->phy  == FHJL3_GL_YS)
    {
      phy->data_len = 37;
      phy->offset = PIM_CURVE_GL ; //

      mem_set(gl_data.value,phy->data_len,REC_DATA_IS_DEFAULT);
      offset = get_curve_save_offset(phy,td,midu);
      fread_array(meter_idx,offset,gl_data.value,phy->data_len);

//      mem_cpy(gl_data.rec_date,td,5);
      buffer_bcd_to_bin(data+4,data+4,4);

       if(compare_string(gl_data.rec_date,data+3,5)==0)
       {
         if((data[9] == 0xAA)&&(data[8] == 0xAA))
         mem_cpy(gl_data.glys,data+10,8);
       }

      fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);

    }

}
//#ifdef __ANHUI_485_PATCH_96_CURVE__
/*start_time��end_time��ΪBIN��ʽ*/
/*void calculat_curve_time(INT8U* start_time,INT8U* end_time,INT8U num,INT8U midu)
{
    INT8U td[5];
    CommandDate s_time;

    //����remain_num����ʱ�� ,����һ������
    s_time.year = start_time[4]+2000;
    s_time.month = start_time[3];
    s_time.day = start_time[2];
    s_time.hour = start_time[1];
    s_time.minute = start_time[0];
    s_time.minute -= (start_time[0]%15);

    commandDateMinusHour(&s_time,(num/4));
    commandDateMinusMinute(&s_time,(num%4)*midu);

    assign_td_value(&s_time,td,5);

    end_time[0] = BCD2byte(td[0]);
    end_time[1] = BCD2byte(td[1]);
    end_time[2] = BCD2byte(td[2]);
    end_time[3] = BCD2byte(td[3]);
    end_time[4] = BCD2byte(td[4]);
}
*/
/*+++
 * ���ܣ����油�����ɼ�¼����
 * ������
        INT16U meter_idx,    ������
        READ_WRITE_DATA *phy ������Լ������
        INT8U *data,         ���ݣ���ȥ��0x33,
        INT8U datalen        ���ݳ���
 * ˵����1.�����ϸ��ɼ�¼��ʽֱ�Ӵ洢0xEE��
---*/
void save_last_load_data_ah(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num,INT8U* start_time)
{
    INT32U offset;
    C_VOLTMETER_BLOCK v_data;
    C_POWER_BLOCK gl_data;
    C_ENERGY_BLOCK enegy_data;
    INT8U midu,rec_len;//num,
    INT8U td_bin[5] = {0};//td[5]
    INT8U rec_td[5]={0};
    //CommandDate s_time;

    midu = 15;
    /*
    //����remain_num����ʱ�� ,����һ������
    s_time.year = start_time[4]+2000;
    s_time.month = start_time[3];
    s_time.day = start_time[2];
    s_time.hour = start_time[1];
    s_time.minute = start_time[0];
    s_time.minute -= (start_time[0]%15);

    commandDateMinusHour(&s_time,(remain_num/4));
    commandDateMinusMinute(&s_time,(remain_num%4)*midu);

    assign_td_value(&s_time,td,5);

    td_bin[0] = BCD2byte(td[0]);
    td_bin[1] = BCD2byte(td[1]);
    td_bin[2] = BCD2byte(td[2]);
    td_bin[3] = BCD2byte(td[3]);
    td_bin[4] = BCD2byte(td[4]);
    */
    calculat_curve_time(start_time,td_bin,remain_num,midu);
    rec_len = data[2];
    buffer_bcd_to_bin(data+3,rec_td,5);//����ʱ���ʽ����bcd��ʽ
    //����һ�ฺ�ɼ�¼
    if(phy->phy  == FHJL1_DY_DL_PL)
    {
        phy->data_len = 23;
        phy->offset = PIM_CURVE_V_I ; 
        
        mem_set(v_data.value,phy->data_len,REC_DATA_IS_DEFAULT);
        offset = get_curve_save_offset(phy,td_bin,midu);
        mem_cpy(v_data.rec_date,td_bin,5);
        if((compare_string(td_bin,rec_td,5)==0) && (rec_len>=28)) //28=5�ֽ�ʱ��+17�ֽ�����+6�ֽڷָ���AA
        {
            if((data[0]==0xA0)&&(data[1]==0xA0)&&(is_valid_bcd(data+8,17)))
            {
                mem_cpy(v_data.Block,data+8,17);
            }
        }
        fwrite_array(meter_idx,offset,v_data.value,phy->data_len);
    }
    //������ฺ�ɼ�¼
    if(phy->phy  == FHJL2_YG_WG_GL)
    {
        phy->data_len = 37;
        phy->offset = PIM_CURVE_GL ;

        mem_set(gl_data.value,phy->data_len,REC_DATA_IS_DEFAULT);
        offset = get_curve_save_offset(phy,td_bin,midu);
        mem_cpy(gl_data.rec_date,td_bin,5);
        if((compare_string(td_bin,rec_td,5)==0) && (rec_len>=35))
        {
            if((data[0]==0xA0)&&(data[1]==0xA0)&&(is_valid_bcd(data+9,24)))
            {
                mem_cpy(gl_data.Block,data+9,24);
            }
        }
        fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
    }
    //�������ฺ�ɼ�¼
    if(phy->phy  == FHJL4_ZFXYG)
    {
        phy->data_len = 21;
        phy->offset = PIM_CURVE_ZFXYWG;

        mem_set(enegy_data.value,phy->data_len,REC_DATA_IS_DEFAULT);
        offset = get_curve_save_offset(phy,td_bin,midu);
        mem_cpy(enegy_data.rec_date,td_bin,5);
        if((compare_string(td_bin,rec_td,5)==0) && (rec_len>=27))
        {
            if((data[0]==0xA0)&&(data[1]==0xA0)&&(is_valid_bcd(data+11,16)))
            {
                mem_cpy(enegy_data.Block,data+11,16);
            }
        }
        fwrite_array(meter_idx,offset,enegy_data.value,phy->data_len);
    }
}
//#endif
/*+++
  ���ܣ���鲹����������
  ������
        INT16U meter_idx,    ������
        INT8U remain_num     Ҫ���ϵ�itemƫ��
---*/
INT8U check_plc_router_save_last_load_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U remain_num)
{
    INT32U offset;
    C_VOLTMETER_BLOCK v_data;
    C_POWER_BLOCK gl_data;
    INT8U midu,i;//idx,num,
    INT8U td[5],bin_td[3];

    td[2] = byte2BCD(datetime[DAY]);
    td[3] = byte2BCD(datetime[MONTH]);
    td[4] = byte2BCD(datetime[YEAR]);
    if(remain_num > 64)//û�����壡������ֻ��Ϊ�˴��������
    remain_num = 23;
    else
    td[1] = remain_num ;         //
    td[0] = 0;
    midu = 60;

    bin_td[0] = td[2];
    bin_td[1] = td[3];
    bin_td[2] = td[4];

    previous_dayhold_td(bin_td);
    buffer_bcd_to_bin(bin_td,td+2,3);

    if(phy->phy  == FHJL1_DY_DL_PL)
    {
      phy->data_len = 23;
      phy->offset = PIM_CURVE_V_I ; //��
      mem_set(v_data.value,phy->data_len,REC_DATA_IS_DEFAULT);

      for(i=0;i<24;i++)
      {
        td[1] = remain_num;

        offset = get_curve_save_offset(phy,td,midu);
        fread_array(meter_idx,offset,v_data.value,7);

        if(compare_string(td,v_data.rec_date,5) == false)
        {
         remain_num ++;
        }
        else
        break;

      }

     return remain_num;

    }
    //�����ɼ�¼2
    else if(phy->phy  == FHJL2_YG_WG_GL)
    {
      phy->data_len = 37;
      phy->offset = PIM_CURVE_GL ; //

      mem_set(gl_data.value,phy->data_len,REC_DATA_IS_DEFAULT);

      for(i=0;i<24;i++)
      {
        td[1] = remain_num;

        offset = get_curve_save_offset(phy,td,midu);
        fread_array(meter_idx,offset,gl_data.value,7);

        if(compare_string(td,gl_data.rec_date,5) == false)
        {
         remain_num ++;
        }
        else
        break;

      }

     return remain_num;

    }
     //�����ɼ�¼3   //g������������һ������ʱ�����Ǹ��ɼ�¼2�ģ�
    else if(phy->phy  == FHJL3_GL_YS)
    {
      phy->data_len = 37;
      phy->offset = PIM_CURVE_GL ; //

      mem_set(gl_data.value,phy->data_len,REC_DATA_IS_DEFAULT);
      for(i=0;i<24;i++)
      {
        td[1] = remain_num;

        offset = get_curve_save_offset(phy,td,midu);
        fread_array(meter_idx,offset,gl_data.value,37);

        if((compare_string(td,gl_data.rec_date,5) == false) &&(gl_data.glys[0] !=0xEE))
        {
         remain_num ++;
        }
        else
        break;

      }

     return remain_num;

    }
    else
    return 23;  //����Ҫ���������ݣ�ֱ�ӷ���23��������һ�����е㣬�˳�����

}

//#ifdef __ANHUI_485_PATCH_96_CURVE__
INT8U check_save_last_load_data_ah(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U remain_num, INT8U* start_time) //start_time�����15�ֵ���������byte��ʽ
{
    INT32U offset;
    C_VOLTMETER_BLOCK v_data;
    C_POWER_BLOCK gl_data;
    C_ENERGY_BLOCK energy_data;
    INT8U midu,i,r_num;//idx,num, minus_hour,minus_minute,
    INT8U td_bin[5] = {0}; //td[5],
    INT8U patch_start_time[5] = {0};

    midu = 15;
    mem_cpy(patch_start_time,start_time,5);
    if(phy->phy  == FHJL1_DY_DL_PL)
    {
        phy->data_len = 23;
        phy->offset = PIM_CURVE_V_I ; //��
        mem_set(v_data.value,phy->data_len,REC_DATA_IS_DEFAULT);

        r_num = remain_num;
        for(i=0;i<96-r_num;i++) //���ѭ��һ��Ҫ����һ��ʵ��ϵͳ�ϵ�ִ��ʱ�䣬��ֹ�����������񣡣�
        {
            calculat_curve_time(patch_start_time,td_bin,remain_num,midu);

            offset = get_curve_save_offset(phy,td_bin,midu);
            fread_array(meter_idx,offset,v_data.value,7);

            if(compare_string(td_bin,v_data.rec_date,5) == false)
            {
                remain_num ++;
                //commandDateMinusMinute(&s_time,midu);
            }
            else
            {
                break;
            }
        }
        return remain_num;
    }
    //�����ɼ�¼2
    else if(phy->phy  == FHJL2_YG_WG_GL)
    {
        phy->data_len = 37;
        phy->offset = PIM_CURVE_GL ; //

        mem_set(gl_data.value,phy->data_len,REC_DATA_IS_DEFAULT);

        r_num = remain_num;
        for(i=0;i<96-r_num;i++) //���ѭ��һ��Ҫ����һ��ʵ��ϵͳ�ϵ�ִ��ʱ�䣬��ֹ�����������񣡣�
        {
            calculat_curve_time(patch_start_time,td_bin,remain_num,midu);

            offset = get_curve_save_offset(phy,td_bin,midu);
            fread_array(meter_idx,offset,v_data.value,7);

            if(compare_string(td_bin,v_data.rec_date,5) == false)
            {
                remain_num ++;
                //commandDateMinusMinute(&s_time,midu);
            }
            else
                break;

        }
        return remain_num;
    }
    else if(phy->phy  == FHJL4_ZFXYG)
    {
        phy->data_len = 21;
        phy->offset = PIM_CURVE_ZFXYWG; //

        mem_set(energy_data.value,phy->data_len,REC_DATA_IS_DEFAULT);

        r_num = remain_num;
        for(i=0;i<96-r_num;i++) //���ѭ��һ��Ҫ����һ��ʵ��ϵͳ�ϵ�ִ��ʱ�䣬��ֹ�����������񣡣�
        {
            calculat_curve_time(patch_start_time,td_bin,remain_num,midu);

            offset = get_curve_save_offset(phy,td_bin,midu);
            fread_array(meter_idx,offset,energy_data.value,7);

            if(compare_string(td_bin,energy_data.rec_date,5) == false)
            {
                remain_num ++;
                //commandDateMinusMinute(&s_time,midu);
            }
            else
                break;

        }
        return remain_num;
    }
    else
    {
        return 23;  //����Ҫ���������ݣ�ֱ�ӷ���23��������һ�����е㣬�˳�����
    }
}
//#endif

READ_LOAD_RECORD_CTRL const load_record_ctrl_list[]=
{
    {FHJL1_DY_DL_PL,              2},
    {FHJL2_YG_WG_GL,              2},
    {FHJL3_GL_YS,                 2},
    {FHJL4_ZFXYG,                 2},
    {FHJL5_SXX_WG,                2},
    {FHJL6_XL,                    2},

    {FHJL1_DY_14,                 4},//
    //{(FHJL1_DY_14-0x3F),        48},// ��ʱע�͵�
    {(FHJL1_DY_14-0x3F+0),       12},// 2017-10-19��ʱʹ����� ̫���˿���Ӱ�쳭��
    {(FHJL1_DY_14-0x3F+1),       12},// 
    {(FHJL1_DY_14-0x3F+2),       12},// 
    //{FHJL1_DY_14,       16},//
    //{FHJL1_DY_14,       16},//
    {FHJL2_DL_14,                 4},
    //{(FHJL2_DL_14-0x3F),        32},//
    {(FHJL2_DL_14-0x3F+0),        8},// 2017-10-19��ʱʹ����� ̫���˿���Ӱ�쳭��
    {(FHJL2_DL_14-0x3F+1),        8},// B���������
    {(FHJL2_DL_14-0x3F+2),        8},// C���������
    {FHJL3_YGGL_14,               4},
    {(FHJL3_YGGL_14-0x3F+0),     16},// 16 OR 32  �й�����
    {(FHJL3_YGGL_14-0x3F+1),     16},// A�й�����
    {(FHJL3_YGGL_14-0x3F+2),     16},// B�й�����
    {(FHJL3_YGGL_14-0x3F+3),     16},// C�й�����
    {FHJL4_WGGL_14,               4},
    {(FHJL4_WGGL_14-0x3F+0),     16},// ���޹�����
    {(FHJL4_WGGL_14-0x3F+1),     16},// A �޹�����
    {(FHJL4_WGGL_14-0x3F+2),     16},// B �޹�����
    {(FHJL4_WGGL_14-0x3F+3),     16},// C �޹�����
    {FHJL5_GLYS_14,               4},
    {(FHJL5_GLYS_14-0x3F+0),     24},
    {(FHJL5_GLYS_14-0x3F+1),     24},// A ��������
    {(FHJL5_GLYS_14-0x3F+2),     24},// B ��������
    {(FHJL5_GLYS_14-0x3F+3),     24},// C ��������
    {FHJL6_YG_WG_ZDN_14,          4}, // 16*6= 96 ���������޹��ܵ���
    {(FHJL6_YG_WG_ZDN_14-0x3F+0), 6}, // 4*6= 24 �����й��ܵ���
    {(FHJL6_YG_WG_ZDN_14-0x3F+1), 6}, // 4*6= 24 �����й��ܵ���
    {(FHJL6_YG_WG_ZDN_14-0x3F+2), 6}, // 4*6= 24 ����޹�1�ܵ���
    {(FHJL6_YG_WG_ZDN_14-0x3F+3), 6}, // 4*6= 24 ����޹�2�ܵ���
    //{FHJL6_YG_WG_ZDN_14,         6}, // 16*6= 96
    {FHJL7_SXX_WG_ZDN_14,         4},
    {(FHJL7_SXX_WG_ZDN_14-0x3F+0),6}, // 4*6= 24 ��һ�����޹��ܵ���
    {(FHJL7_SXX_WG_ZDN_14-0x3F+1),6}, // 4*6= 24 �ڶ������޹��ܵ���
    {(FHJL7_SXX_WG_ZDN_14-0x3F+2),6}, // 4*6= 24 ���������޹��ܵ���
    {(FHJL7_SXX_WG_ZDN_14-0x3F+3),6}, // 4*6= 24 ���������޹��ܵ���
    {FHJL8_DQXL_14,    4},
};
INT8U load_record_read_cnt(INT32U phy)
{
    INT8U idx = 0;
    for(idx=0;idx<sizeof(load_record_ctrl_list)/sizeof(READ_LOAD_RECORD_CTRL);idx++)
    {
        if(load_record_ctrl_list[idx].phy == phy)
        {
            return load_record_ctrl_list[idx].BlockCnt;
        }
    }
    return 0xFF; // ��Ч
}
BOOLEAN check_valid_load_record_phy(INT32U phy)
{
    INT8U idx = 0;
    for(idx=0;idx<sizeof(load_record_ctrl_list)/sizeof(READ_LOAD_RECORD_CTRL);idx++)
    {
        if(load_record_ctrl_list[idx].phy == phy)
        {
            return TRUE;
        }
    }
    return FALSE; // ��Ч
}
/*start_time��end_time��ΪBIN��ʽ*/
void calculat_curve_time(INT8U* start_time,INT8U* end_time,INT8U num,INT8U midu)
{
    INT8U td[5];
    CommandDate s_time;
    INT8U idx = 0;
    INT8U cnt = 0;
    //����remain_num����ʱ�� ,����һ������
    s_time.year = start_time[4]+2000;
    s_time.month = start_time[3];
    s_time.day = start_time[2];
    s_time.hour = start_time[1];
    s_time.minute = start_time[0];
    s_time.minute -= (start_time[0]%15);

    // �ܶ�15���� 30 60 ������ �ٴ�ͳ���255 ע�⴫��
    cnt = num/4;
    for(idx=0;idx<cnt;idx++)
    {
        commandDateAddMinute(&s_time,4*midu);
    }
    commandDateAddMinute(&s_time,(num%4)*midu);

    assign_td_value(&s_time,td,5);

    end_time[0] = BCD2byte(td[0]);
    end_time[1] = BCD2byte(td[1]);
    end_time[2] = BCD2byte(td[2]);
    end_time[3] = BCD2byte(td[3]);
    end_time[4] = BCD2byte(td[4]);
}

/*
���ô���ʵ�֣�Ҳ�����ñ�ʵ��
*/
void get_phy_save_ctrl_info(READ_WRITE_DATA *phy,LOAD_SAVE_CTRL *load_save_ctrl)
{
    //
    switch(phy->phy)
    {
        case FHJL1_DY_DL_PL:
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len = 15;//
            break;
        case FHJL1_DY_14:
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len = 6;//
            load_save_ctrl->data_size = 2;//
            break;
        case FHJL2_DL_14:
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 6;//
            load_save_ctrl->save_len = 9;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL1_DY_14-0x3F+0):
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len = 2;//
            load_save_ctrl->data_size = 2;//
            break;
        case (FHJL1_DY_14-0x3F+1):
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 2;//
            load_save_ctrl->save_len = 2;//
            load_save_ctrl->data_size = 2;//
            break;
        case (FHJL1_DY_14-0x3F+2):
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 4;//
            load_save_ctrl->save_len = 2;//
            load_save_ctrl->data_size = 2;//
            break;
        case (FHJL2_DL_14-0x3F+0):
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 6;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL2_DL_14-0x3F+1):
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 9;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL2_DL_14-0x3F+2):
            phy->data_len = 23; // 5+6(��ѹ)+9+����+3(���ߵ���) = 23 
            phy->offset = PIM_CURVE_V_I ;
            load_save_ctrl->offset = 12;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;

        //
        case FHJL2_YG_WG_GL:
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len = 24;//
            load_save_ctrl->data_size = 3;//
            break;
        case FHJL3_YGGL_14:
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len = 12;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL3_YGGL_14-0x3F+0)://���й�����
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL3_YGGL_14-0x3F+1)://A �й�����
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 3;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL3_YGGL_14-0x3F+2)://B �й�����
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 6;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL3_YGGL_14-0x3F+3)://C �й�����
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 9;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case FHJL4_WGGL_14:// �޹������������ݿ�
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 12;//
            load_save_ctrl->save_len = 12;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL4_WGGL_14-0x3F+0): // ���޹�����
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 12;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL4_WGGL_14-0x3F+1): // A �޹�����
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 15;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL4_WGGL_14-0x3F+2): // B �޹�����
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 18;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case (FHJL4_WGGL_14-0x3F+3): // C �޹�����
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 21;//
            load_save_ctrl->save_len = 3;//
            load_save_ctrl->data_size = 3;//
            break;
        case FHJL5_GLYS_14:// �����������ݿ�
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 24;//
            load_save_ctrl->save_len = 8;//
            load_save_ctrl->data_size = 2;//
            break;
        case (FHJL5_GLYS_14-0x3F+0): // �ܹ�������
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 24;//
            load_save_ctrl->save_len = 2;//
            load_save_ctrl->data_size = 2;//
            break;
        case (FHJL5_GLYS_14-0x3F+1): // A ��������
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 26;//
            load_save_ctrl->save_len = 2;//
            load_save_ctrl->data_size = 2;//
            break;
        case (FHJL5_GLYS_14-0x3F+2): // B ��������
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 28;//
            load_save_ctrl->save_len = 2;//
            load_save_ctrl->data_size = 2;//
            break;
        case (FHJL5_GLYS_14-0x3F+3): // C ��������
            phy->data_len = 37; // 5+12(�й�����)+12(�޹�����)+8(��������) = 37
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 30;//
            load_save_ctrl->save_len = 2;//
            load_save_ctrl->data_size = 2;//
            break;

        // 12�� ���ɼ�¼3  
        case FHJL3_GL_YS:
            phy->data_len = 37;
            phy->offset = PIM_CURVE_GL ;
            load_save_ctrl->offset = 24;//
            load_save_ctrl->save_len = 8;//
            load_save_ctrl->data_size = 2;//
            break;
        case FHJL4_ZFXYG:// 12 ���ɼ�¼4 ���������޹��ܵ���
        case FHJL6_YG_WG_ZDN_14:
            phy->data_len = 21;
            phy->offset = PIM_CURVE_ZFXYWG;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len = 16;//
            load_save_ctrl->data_size = 4;//
            break;
        case (FHJL6_YG_WG_ZDN_14-0x3F+0):
            phy->data_len = 21;
            phy->offset = PIM_CURVE_ZFXYWG;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len =  4;//
            load_save_ctrl->data_size = 4;//
            break;
        case (FHJL6_YG_WG_ZDN_14-0x3F+1):// �����й��ܵ���
            phy->data_len = 21;
            phy->offset = PIM_CURVE_ZFXYWG;
            load_save_ctrl->offset = 4;//
            load_save_ctrl->save_len =  4;//
            load_save_ctrl->data_size = 4;//
            break;
        case (FHJL6_YG_WG_ZDN_14-0x3F+2):// ����޹�1�ܵ���
            phy->data_len = 21;
            phy->offset = PIM_CURVE_ZFXYWG;
            load_save_ctrl->offset = 8;//
            load_save_ctrl->save_len =  4;//
            load_save_ctrl->data_size = 4;//
            break;
        case (FHJL6_YG_WG_ZDN_14-0x3F+3):// ����޹�2 �ܵ���
            phy->data_len = 21;
            phy->offset = PIM_CURVE_ZFXYWG;
            load_save_ctrl->offset = 12;//
            load_save_ctrl->save_len =  4;//
            load_save_ctrl->data_size = 4;//
            break;
            
        case FHJL5_SXX_WG:// 12 ���ɼ�¼5 �������޹�
        case FHJL7_SXX_WG_ZDN_14:
            phy->data_len = 21;
            phy->offset = PIM_CURVE_WG1234;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len = 16;//
            load_save_ctrl->data_size = 4;//
            break;
        case (FHJL7_SXX_WG_ZDN_14-0x3F+0): //��һ�����޹��ܵ���
            phy->data_len = 21;
            phy->offset = PIM_CURVE_WG1234;
            load_save_ctrl->offset = 0;//
            load_save_ctrl->save_len =  4;//
            load_save_ctrl->data_size = 4;//
            break;
        case (FHJL7_SXX_WG_ZDN_14-0x3F+1): //�ڶ������޹��ܵ���
            phy->data_len = 21;
            phy->offset = PIM_CURVE_WG1234;
            load_save_ctrl->offset = 4;//
            load_save_ctrl->save_len =  4;//
            load_save_ctrl->data_size = 4;//
            break;
        case (FHJL7_SXX_WG_ZDN_14-0x3F+2): //���������޹��ܵ���
            phy->data_len = 21;
            phy->offset = PIM_CURVE_WG1234;
            load_save_ctrl->offset = 8;//
            load_save_ctrl->save_len =  4;//
            load_save_ctrl->data_size = 4;//
            break;
        case (FHJL7_SXX_WG_ZDN_14-0x3F+3): //���������޹��ܵ���
            phy->data_len = 21;
            phy->offset = PIM_CURVE_WG1234;
            load_save_ctrl->offset = 12;//
            load_save_ctrl->save_len =  4;//
            load_save_ctrl->data_size = 4;//
            break;
        
    }
}
void save_last_load_data_LiaoNing(READ_PARAMS *read_params,INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num,INT8U* start_time)//INT8U block_cnt,
{
    INT32U offset;
    C_VOLTMETER_BLOCK v_data;
    C_POWER_BLOCK gl_data;
    C_ENERGY_BLOCK energy_data;
    C_ELEC_ENERGY_BLOCK elec_energy_data;
    LOAD_SAVE_CTRL load_save_ctrl;
    INT16U pos = 0;
    INT16U read_pos = 0;
    INT8U midu;//idx,num,
    //INT8U td[5] = {0};
    INT8U td_bin[5] = {0};
    INT8U recv_td[5] = {0};
    //CommandDate s_time;
    INT8U block_cnt = 0;
    INT8U idx = 0;
    INT8U idx_sub = 0;
    INT8U record_len = 0;
    INT8U cnt = 0;
    BOOLEAN copy_flag = FALSE;
    BOOLEAN find_flag = FALSE;
    BOOLEAN curve_exist_flag = FALSE;
    INT8U   bit_mask = 0;
    INT8U   check_AA_len = 0;
    INT8U   tmp_flag = 0;
    //INT8U   idx_phase;
    union{
        INT8U load_VI[23];// 5(������ʱ��)+6(��ѹ)+9(����)+3(���ߵ���) 
        INT8U load_ywggl[37];// 5+12+12+8 �й� �޹�  �������� 
        INT8U load_dnsz[21]; // 5+16 ����ʾֵ 
    }var;
    midu = 15;
        
    
    block_cnt = load_record_read_cnt(phy->phy);

    if( (0xFF == block_cnt) || (0x00 == block_cnt) )
    {
        //  TODO ?????
        return;//����ʱ�������ˣ����ﻹ������������Ƿ��¼��־?�ڴ汻�����??
    }
    //#if ( (defined __PROVICE_SHANGHAI__) || (defined __PROVICE_SHANGHAI_FK__) )
    METER_DOCUMENT meter_doc;
    fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
    if( (COMMPORT_PLC == meter_doc.baud_port.port) || (COMMPORT_PLC_REC == meter_doc.baud_port.port) )
    {
        /* �ز����� �Ҳ��� 24�� ����һ�γ���һ���� */
        if( check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD))
        {
            if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_24_DOT) )
            {
                midu = 60; // 60����
                block_cnt = 1;
            }
            else
            {
                /* ���ܶ�洢���� */
                if(remain_num+block_cnt > read_params->patch_load_curve_cnt)
                {
                    block_cnt = read_params->patch_load_curve_cnt - remain_num;
                }
            }
        }

    }

    calculat_curve_time(start_time,td_bin,remain_num,midu);
    
    //#if ( (defined __PROVICE_SHANGHAI__) || (defined __PROVICE_SHANGHAI_FK__) )  
    READ_PARAMS *read_param = NULL;
    if( (COMMPORT_PLC != meter_doc.baud_port.port) && (COMMPORT_PLC_REC != meter_doc.baud_port.port) )
    {
        /* 485�������� ��Ҫ����洢���� ��ֹ���洢 */
        if( check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_485_PATCH_LOAD_RECORD)
         && check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PATCH_TODAY_LOAD_RECORD) )
        {
            switch(meter_doc.baud_port.port)
            {
                case COMMPORT_485_REC:
                    tmp_flag = get_readport_idx(COMMPORT_485_REC);  
                    if(tmp_flag != 0xFF)
                    {
                        read_param = (READ_PARAMS *)(&portContext_rs485[tmp_flag].read_params);
                        // ��ֹ��洢������Ҳ����˿ڶ�Ӧcontext������Ĭ��block_cnt�洢
                        if(remain_num+block_cnt > read_param->patch_load_curve_cnt)
                        {
                            block_cnt = read_param->patch_load_curve_cnt - remain_num;
                        }
                    }
                    break;
                case COMMPORT_485_CAS:
                    tmp_flag = get_readport_idx(COMMPORT_485_CAS);  
                    if(tmp_flag != 0xFF )
                    {
                        read_param = (READ_PARAMS *)(&portContext_rs485[tmp_flag].read_params);
                    
                        if(remain_num+block_cnt > read_param->patch_load_curve_cnt)
                        {
                            block_cnt = read_param->patch_load_curve_cnt - remain_num;
                        }
                    }
                    break;
            }
        }
    }    
    
    mem_set(load_save_ctrl.value,sizeof(LOAD_SAVE_CTRL),0x00);
    get_phy_save_ctrl_info(phy,&load_save_ctrl);
    switch(phy->phy)
    {
        case FHJL1_DY_DL_PL:
            // TODO ���⴦�� 
            pos = 0;          
            bit_mask = 0;
            while(pos < datalen)
            {
                record_len = data[2+pos];//���ɼ�¼�ֽ���
                buffer_bcd_to_bin(data+3+pos,recv_td,5);
                //mem_cpy(v_data.rec_date,td_bin,5);
                find_flag = FALSE;
                for(idx=0;idx<block_cnt;idx++)
                {
                    //
                    calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                    if(compare_string(td_bin,recv_td,5)==0)
                    {
                        mem_cpy(v_data.rec_date,td_bin,5);
                        offset = get_curve_save_offset(phy,td_bin,midu);
                        find_flag = TRUE;
                        set_bit_value(&bit_mask,1,idx);// �������䷶Χ�ڵ�ʱ��
                        break;
                    }
                }
                if(TRUE == find_flag)
                {
                    /* ��ȡ�洢���ݣ����ʱ�겻�ԣ���ִ�к������� */
                    #if 1
                    fread_array(meter_idx,offset,var.load_VI,phy->data_len);
                    if(0 != compare_string(v_data.rec_date,var.load_VI,5))
                    #endif
                    {
                        if( (data[0+pos] == 0xA0) && (data[1+pos] == 0xA0) && (record_len != 0)
                        && (TRUE == check_is_all_ch(data+3+pos+record_len-6,6,0xAA)) )
                        {
                            if(record_len >= 28)
                            {
                                //��ѹ ���� copy
                                mem_cpy(v_data.value+5,data+8+pos,15);
                                mem_set(v_data.value+5+15,3,REC_DATA_IS_DEFAULT);
                            }
                            else
                            {
                                // ����Ԥ�볤��ʱ���޷�ɸѡ��Ƶ�� ��ʱ��ô���� �����޸� TODO ?
                                //if(record_len > 11)// 5+17+6 
                                {
                                    //mem_cpy(v_data.value+5,data+8+pos,record_len-11);
                                    //mem_set(v_data.value+5+record_len-11,3,REC_DATA_IS_DEFAULT);
                                }
                                //else
                                {
                                    mem_set(v_data.value+5,18,REC_DATA_IS_DEFAULT);
                                }
                            }
                            fwrite_array(meter_idx,offset,v_data.value,phy->data_len);
                        }
                        else
                        {
                            mem_cpy(v_data.rec_date,td_bin,5);
                            mem_set(v_data.value+5,18,REC_DATA_IS_DEFAULT);
                            fwrite_array(meter_idx,offset,v_data.value,phy->data_len);
                        }
                    }
                }

                pos+= 5+record_len;//������һ��Ҫ�������ݿ�
            }
            // �ظ�������û�е�ʱ�꣬д��0xEE
            for(idx=0;idx<block_cnt;idx++)
            {
                //
                if(get_bit_value(&bit_mask,1,idx) == 0)
                {
                    calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                    offset = get_curve_save_offset(phy,td_bin,midu);
                    mem_cpy(v_data.rec_date,td_bin,5);
                    #if 1
                    fread_array(meter_idx,offset,var.load_VI,phy->data_len);
                    if(0 != compare_string(v_data.rec_date,var.load_VI,5))
                    #endif
                    {
                        mem_set(v_data.value+5,18,REC_DATA_IS_DEFAULT);
                        fwrite_array(meter_idx,offset,v_data.value,phy->data_len);
                    }
                }
            }            
            break;
        case FHJL1_DY_14://��ѹ 
        case (FHJL1_DY_14-0x3F+0):        
            if(datalen >= 5)//��Ҫ���� datalen ��ֹ�������
            {
                mem_cpy(recv_td,data,5);
                for(idx=0;idx<5;idx++)
                {
                    recv_td[idx] = BCD2byte(data[idx]);
                }
                if(compare_string(td_bin,recv_td,5)==0)
                {
                    copy_flag = TRUE;
                }
                read_pos = 5;
            }
            for(idx=0;idx<block_cnt;idx++)
            {
                calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                offset = get_curve_save_offset(phy,td_bin,midu);
                mem_cpy(v_data.rec_date,td_bin,5); 
                
                mem_set(v_data.value+5,18,0xFF);//REC_DATA_IS_DEFAULT
                if(TRUE == copy_flag)
                {
                    //
                    if(load_save_ctrl.data_size == 0) 
                    {
                        cnt = 1;
                    }
                    else
                    {
                        cnt = load_save_ctrl.save_len/load_save_ctrl.data_size;
                    }
                    //if(cnt > 1)
                    {
                        //�鳭�� cnt >1 ���೭��  cnt ���� 1
                        for(idx_sub=0;idx_sub<cnt;idx_sub++)
                        {
                            read_pos += load_save_ctrl.data_size;
                            // 5+idx*load_save_ctrl.save_len ����һ�鳤�� load_save_ctrl.data_size*idx_sub ���೤��
                            if( (FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size))
                               || (read_pos > datalen) )
                            {
                                //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                                mem_set(v_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size,REC_DATA_IS_DEFAULT);
                            }
                            else
                            {
                                mem_cpy(v_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size);
                            }
                        }
                    }
                    /*
                    else
                    {
                        if(FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len))
                        {
                            //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                            mem_set(v_data.Block+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                        }
                        else
                        {
                            mem_cpy(v_data.Block+load_save_ctrl.offset,data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len);
                        }
                    }
                    */
                }
                else
                {
                    mem_set(v_data.Block+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                }
                #if 1
                fread_array(meter_idx,offset,var.load_VI,phy->data_len);
                /*��ѹ�� ����A �� ���Ǵ洢��ʼλ�� ��� ʱ�겻�� �Ͳ�Ӧ�ô洢 */
                //if(0 != compare_string(v_data.rec_date,var.load_VI,5))
                /* ʱ�� ��ͬ  �� ���ݲ���ȫ FF ˵���洢���� ���ܴ洢���γ��� */
                curve_exist_flag = FALSE;
                if((compare_string(td_bin,var.load_VI,5) == 0) && (FALSE == check_is_all_FF(var.load_VI+5+load_save_ctrl.offset,load_save_ctrl.save_len)) )
                {
                    curve_exist_flag = TRUE;
                }
                if(FALSE == curve_exist_flag)
                #endif
                {
                    fwrite_array(meter_idx,offset,v_data.value,phy->data_len);               
                }
            }
            break;
        case (FHJL1_DY_14-0x3F+1):
        case (FHJL1_DY_14-0x3F+2):    
        case FHJL2_DL_14:// ����  ��ѹ ���� ���ߵ����洢��һ�� �����ʱ�ȶ�ȡ��ѹֵ
        case (FHJL2_DL_14-0x3F+0):
        case (FHJL2_DL_14-0x3F+1):
        case (FHJL2_DL_14-0x3F+2):
            if(datalen >= 5)//��Ҫ���� datalen ��ֹ�������
            {
                mem_cpy(recv_td,data,5);
                for(idx=0;idx<5;idx++)
                {
                    recv_td[idx] = BCD2byte(data[idx]);
                }
                if(compare_string(td_bin,recv_td,5)==0)
                {
                    copy_flag = TRUE;
                }
                read_pos = 5;
            }
            for(idx=0;idx<block_cnt;idx++)
            {
                calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                offset = get_curve_save_offset(phy,td_bin,midu);
                mem_cpy(v_data.rec_date,td_bin,5); 
                //��������
                fread_array(meter_idx,offset,var.load_VI,phy->data_len);
                mem_set(v_data.value+5,18,0xFF);
                if(compare_string(td_bin,var.load_VI,5)==0)
                {
                    mem_cpy(v_data.value+5,var.load_VI+5,18);
                }
                #if 1
                /* ʱ�� ��ͬ  �� ���ݲ���ȫ FF ˵���洢���� ���ܴ洢���γ��� */
                curve_exist_flag = FALSE;
                if((compare_string(td_bin,var.load_VI,5) == 0) && (FALSE == check_is_all_FF(var.load_VI+5+load_save_ctrl.offset,load_save_ctrl.save_len)) )
                {
                    curve_exist_flag = TRUE;
                    read_pos += load_save_ctrl.save_len;/*18-04-11 ������� ����洢������ */
                }
                if(FALSE == curve_exist_flag)
                #endif
                {
                    if(TRUE == copy_flag)
                    {
                        //
                        if(load_save_ctrl.data_size == 0) 
                        {
                            cnt = 1;
                        }
                        else
                        {
                            cnt = load_save_ctrl.save_len/load_save_ctrl.data_size;
                        }
                        //if(cnt > 1)
                        {
                            //�鳭�� cnt >1 ���೭��  cnt ���� 1
                            for(idx_sub=0;idx_sub<cnt;idx_sub++)
                            {
                                read_pos += load_save_ctrl.data_size;
                                // 5+idx*load_save_ctrl.save_len ����һ�鳤�� load_save_ctrl.data_size*idx_sub ���೤��
                                if( (FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size))
                                  || (read_pos > datalen) )
                                {
                                    //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                                    mem_set(v_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size,REC_DATA_IS_DEFAULT);
                                }
                                else
                                {
                                    mem_cpy(v_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size);
                                }
                            }
                        }
                        /*
                        if(FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len))
                        {
                            //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                            mem_set(v_data.Block+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                        }
                        else
                        {
                            mem_cpy(v_data.Block+load_save_ctrl.offset,data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len);
                        }
                        */
                    }
                    else
                    {
                        mem_set(v_data.Block+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                    }
                    fwrite_array(meter_idx,offset,v_data.value,phy->data_len);
                }
            }
            break;
        case FHJL2_YG_WG_GL:// �й� �޹� ���� ���� 
            
            pos = 0;
            bit_mask = 0;
            while(pos < datalen)
            {
                record_len = data[2+pos];//���ɼ�¼�ֽ���
                buffer_bcd_to_bin(data+3+pos,recv_td,5);
                //mem_cpy(gl_data.rec_date,td_bin,5);
                find_flag = FALSE;
                for(idx=0;idx<block_cnt;idx++)
                {
                    //
                    calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                    if(compare_string(td_bin,recv_td,5)==0)
                    {
                        mem_cpy(gl_data.rec_date,td_bin,5);
                        offset = get_curve_save_offset(phy,td_bin,midu);
                        find_flag = TRUE;
                        set_bit_value(&bit_mask,1,idx);// �������䷶Χ�ڵ�ʱ��
                        break;
                    }
                }
                if(TRUE == find_flag)
                {
                    #if 1
                    fread_array(meter_idx,offset,var.load_ywggl,phy->data_len);
                    if(0 != compare_string(gl_data.rec_date,var.load_ywggl,5))
                    #endif
                    {
                        if( (data[0+pos] == 0xA0) && (data[1+pos] == 0xA0) && (record_len != 0)
                        && (TRUE == check_is_all_ch(data+3+pos+record_len-5,5,0xAA)) )
                        {
                            if(record_len >= 35)
                            {
                                //�й��޹� copy 3+5+1�ֽ�(AA) = 9
                                mem_cpy(gl_data.value+5,data+9+pos,24);
                                mem_set(gl_data.value+5+24,8,0xFF);//REC_DATA_IS_DEFAULT //����������FF
                            }
                            else
                            {
                                // ����Ԥ�볤��ʱ���޷�ɸѡ��Ƶ�� ��ʱ��ô���� �����޸� TODO ?
                                //if(record_len > 11)// 5+17+6 
                                {
                                    //mem_cpy(v_data.value+5,data+8+pos,record_len-11);
                                    //mem_set(v_data.value+5+record_len-11,3,REC_DATA_IS_DEFAULT);
                                }
                                //else
                                {
                                    mem_set(gl_data.value+5,24,REC_DATA_IS_DEFAULT);
                                    mem_set(gl_data.value+5+24,8,0xFF); //����������FF
                                }
                            }
                            fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
                        }
                        else
                        {
                            mem_cpy(gl_data.rec_date,td_bin,5);
                            mem_set(gl_data.value+5,24,REC_DATA_IS_DEFAULT);// 32
                            mem_set(gl_data.value+5+24,8,0xFF);//����������FF
                            fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
                        }
                    }
                }

                pos+= 5+record_len;//������һ��Ҫ�������ݿ�
            }
            // �ظ�������û�е�ʱ�꣬д��0xEE
            for(idx=0;idx<block_cnt;idx++)
            {
                //
                if(get_bit_value(&bit_mask,1,idx) == 0)
                {
                    calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                    offset = get_curve_save_offset(phy,td_bin,midu);
                    mem_cpy(gl_data.rec_date,td_bin,5);
                    #if 1
                    fread_array(meter_idx,offset,var.load_ywggl,phy->data_len);
                    if(0 != compare_string(gl_data.rec_date,var.load_ywggl,5))
                    #endif
                    {
                        mem_set(gl_data.value+5,24,REC_DATA_IS_DEFAULT);
                        mem_set(gl_data.value+5+24,8,0xFF);//����������FF
                        fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
                    }
                }
            } 
            break;
        case FHJL3_YGGL_14: // �й�����
        case (FHJL3_YGGL_14-0x3F+0):       
            if(datalen >= 5)//��Ҫ���� datalen ��ֹ�������
            {
                mem_cpy(recv_td,data,5);
                for(idx=0;idx<5;idx++)
                {
                    recv_td[idx] = BCD2byte(data[idx]);
                }
                if(compare_string(td_bin,recv_td,5)==0)
                {
                    copy_flag = TRUE;
                }
                read_pos = 5;
            }
            for(idx=0;idx<block_cnt;idx++)
            {
                calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                offset = get_curve_save_offset(phy,td_bin,midu);
                mem_cpy(gl_data.rec_date,td_bin,5); 
                
                mem_set(gl_data.value+5,32,0xFF);//REC_DATA_IS_DEFAULT
                if(TRUE == copy_flag)
                {
                    //
                    if(load_save_ctrl.data_size == 0) 
                    {
                        cnt = 1;
                    }
                    else
                    {
                        cnt = load_save_ctrl.save_len/load_save_ctrl.data_size;
                    }
                    //if(cnt > 1)
                    {
                        //�鳭�� cnt >1 ���೭��  cnt ���� 1
                        for(idx_sub=0;idx_sub<cnt;idx_sub++)
                        {
                            read_pos += load_save_ctrl.data_size;
                            // 5+idx*load_save_ctrl.save_len ����һ�鳤�� load_save_ctrl.data_size*idx_sub ���೤��
                            if( (FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size))
                                || (read_pos > datalen) )
                            {
                                //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                                mem_set(gl_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size,REC_DATA_IS_DEFAULT);
                            }
                            else
                            {
                                mem_cpy(gl_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size);
                            }
                        }
                    }
                    /*
                    if(FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len))
                    {
                        //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                        mem_set(gl_data.value+5+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                    }
                    else
                    {
                        mem_cpy(gl_data.value+5+load_save_ctrl.offset,data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len);
                    }
                    */
                }
                else
                {
                    mem_set(gl_data.value+5+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                }
                #if 1
                fread_array(meter_idx,offset,var.load_ywggl,phy->data_len);
                //if(0 != compare_string(gl_data.rec_date,var.load_ywggl,5))
                /* ʱ�� ��ͬ  �� ���ݲ���ȫ FF ˵���洢���� ���ܴ洢���γ��� */
                curve_exist_flag = FALSE;
                if((compare_string(td_bin,var.load_ywggl,5) == 0) && (FALSE == check_is_all_FF(var.load_ywggl+5+load_save_ctrl.offset,load_save_ctrl.save_len)) )
                {
                    curve_exist_flag = TRUE;
                }
                if(FALSE == curve_exist_flag)
                #endif
                {
                    fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
                }
            }
            break;
        case (FHJL3_YGGL_14-0x3F+1):
        case (FHJL3_YGGL_14-0x3F+2):
        case (FHJL3_YGGL_14-0x3F+3):
        case FHJL4_WGGL_14: // �޹����� 
        case (FHJL4_WGGL_14-0x3F+0):
        case (FHJL4_WGGL_14-0x3F+1):
        case (FHJL4_WGGL_14-0x3F+2):
        case (FHJL4_WGGL_14-0x3F+3):
        case FHJL5_GLYS_14://��������
        case (FHJL5_GLYS_14-0x3F+0):
        case (FHJL5_GLYS_14-0x3F+1):
        case (FHJL5_GLYS_14-0x3F+2):
        case (FHJL5_GLYS_14-0x3F+3):
            if(datalen >= 5)//��Ҫ���� datalen ��ֹ�������
            {
                mem_cpy(recv_td,data,5);
                for(idx=0;idx<5;idx++)
                {
                    recv_td[idx] = BCD2byte(data[idx]);
                }
                if(compare_string(td_bin,recv_td,5)==0)
                {
                    copy_flag = TRUE;
                }
                read_pos = 5;
            }
            for(idx=0;idx<block_cnt;idx++)
            {
                calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                offset = get_curve_save_offset(phy,td_bin,midu);
                mem_cpy(gl_data.rec_date,td_bin,5); 
                //��������
                fread_array(meter_idx,offset,var.load_ywggl,phy->data_len);
                mem_set(gl_data.value+5,32,0xFF);
                if(compare_string(td_bin,var.load_ywggl,5)==0)
                {
                    mem_cpy(gl_data.value+5,var.load_ywggl+5,32);
                }

                #if 1
                /* ʱ�� ��ͬ  �� ���ݲ���ȫ FF ˵���洢���� ���ܴ洢���γ��� */
                curve_exist_flag = FALSE;
                if((compare_string(td_bin,var.load_ywggl,5) == 0) && (FALSE == check_is_all_FF(var.load_ywggl+5+load_save_ctrl.offset,load_save_ctrl.save_len)) )
                {
                    curve_exist_flag = TRUE;
                    read_pos += load_save_ctrl.save_len;/*18-04-11 ������� ����洢������ */
                }
                if(FALSE == curve_exist_flag)
                #endif
                {
                    if(TRUE == copy_flag)
                    {
                        //
                        if(load_save_ctrl.data_size == 0) 
                        {
                            cnt = 1;
                        }
                        else
                        {
                            cnt = load_save_ctrl.save_len/load_save_ctrl.data_size;
                        }
                        //if(cnt > 1)
                        {
                            //�鳭�� cnt >1 ���೭��  cnt ���� 1
                            for(idx_sub=0;idx_sub<cnt;idx_sub++)
                            {
                                read_pos += load_save_ctrl.data_size;
                                // 5+idx*load_save_ctrl.save_len ����һ�鳤�� load_save_ctrl.data_size*idx_sub ���೤��
                                if( (FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size))
                                    || (read_pos > datalen) )
                                {
                                    //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                                    mem_set(gl_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size,REC_DATA_IS_DEFAULT);
                                }
                                else
                                {
                                    mem_cpy(gl_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size);
                                }
                            }
                        }
                        /*
                        if(FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len))
                        {
                            //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                            mem_set(gl_data.value+5+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                        }
                        else
                        {
                            mem_cpy(gl_data.value+5+load_save_ctrl.offset,data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len);
                        }
                        */
                    }
                    else
                    {
                        mem_set(gl_data.value+5+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                    }
                    fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
                }
            }
            break;
        case FHJL3_GL_YS: // 12�� ���� ��������
            pos = 0;
            bit_mask = 0;
            check_AA_len = 4;
            
            while(pos < datalen)
            {
                record_len = data[2+pos];//���ɼ�¼�ֽ���
                buffer_bcd_to_bin(data+3+pos,recv_td,5);
                //mem_cpy(gl_data.rec_date,td_bin,5);
                find_flag = FALSE;
                for(idx=0;idx<block_cnt;idx++)
                {
                    //
                    calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                    if(compare_string(td_bin,recv_td,5)==0)
                    {
                        mem_cpy(gl_data.rec_date,td_bin,5);
                        offset = get_curve_save_offset(phy,td_bin,midu);
                        find_flag = TRUE;
                        set_bit_value(&bit_mask,1,idx);// �������䷶Χ�ڵ�ʱ��
                        break;
                    }
                }
                
                if(TRUE == find_flag)
                {
                    // �ȶ�ȡ֮ǰ�洢�����޹����ʺ͹�����������
                    fread_array(meter_idx,offset,var.load_ywggl,phy->data_len);
    
                    mem_cpy(gl_data.rec_date,td_bin,5);                
                    mem_set(gl_data.value+5,32,0xFF);
                    if(compare_string(td_bin,var.load_ywggl,5)==0)
                    {
                        mem_cpy(gl_data.value+5,var.load_ywggl+5,32);
                    }
                    #if 1
                    /* ʱ�� ��ͬ  �� ���ݲ���ȫ FF ˵���洢���� ���ܴ洢���γ��� */
                    curve_exist_flag = FALSE;
                    if((compare_string(td_bin,var.load_ywggl,5) == 0) && (FALSE == check_is_all_FF(var.load_ywggl+5+24,8)) )
                    {
                        curve_exist_flag = TRUE;
                    }
                    if(FALSE == curve_exist_flag)
                    #endif
                    {
                        if( (data[0+pos] == 0xA0) && (data[1+pos] == 0xA0) && (record_len != 0)
                        && (TRUE == check_is_all_ch(data+3+pos+record_len-check_AA_len,check_AA_len,0xAA)) )
                        {
                            if(record_len >= 19) // 5+6+8
                            {
                                //�������� 
                                mem_cpy(gl_data.value+5+24,data+pos+10,8);
                            }
                            else
                            {
                                // ����Ԥ�볤��ʱ���޷�ɸѡ��Ƶ�� ��ʱ��ô���� �����޸� TODO ?
                                //if(record_len > 11)// 5+17+6 
                                {
                                    //mem_cpy(v_data.value+5,data+8+pos,record_len-11);
                                    //mem_set(v_data.value+5+record_len-11,3,REC_DATA_IS_DEFAULT);
                                }
                                //else
                                {
                                    mem_set(gl_data.value+5+24,8,REC_DATA_IS_DEFAULT);
                                }
                            }
                            fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
                        }
                        else
                        {
                            mem_cpy(gl_data.rec_date,td_bin,5);
                            mem_set(gl_data.value+5+24,8,REC_DATA_IS_DEFAULT);
                            fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
                        }
                    }
                }

                pos+= 5+record_len;//������һ��Ҫ�������ݿ�
            }
            // �ظ�������û�е�ʱ�꣬д��0xEE
            for(idx=0;idx<block_cnt;idx++)
            {
                //
                if(get_bit_value(&bit_mask,1,idx) == 0)
                {
                    calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                    offset = get_curve_save_offset(phy,td_bin,midu);

                    // �ȶ�ȡ֮ǰ�洢�����޹����ʺ͹�����������
                    fread_array(meter_idx,offset,var.load_ywggl,phy->data_len);
                    #if 1
                    /* ʱ�� ��ͬ  �� ���ݲ���ȫ FF ˵���洢���� ���ܴ洢���γ��� */
                    curve_exist_flag = FALSE;
                    if((compare_string(td_bin,var.load_ywggl,5) == 0) && (FALSE == check_is_all_FF(var.load_ywggl+5+24,8)) )
                    {
                        curve_exist_flag = TRUE;
                    }
                    if(FALSE == curve_exist_flag)
                    #endif
                    {
                        mem_cpy(gl_data.rec_date,td_bin,5);               
                        mem_set(gl_data.value+5,32,0xFF);
                        if(compare_string(td_bin,var.load_ywggl,5)==0)
                        {
                            mem_cpy(gl_data.value+5,var.load_ywggl+5,32);
                        }
                        mem_set(gl_data.value+5+24,8,REC_DATA_IS_DEFAULT);
                        fwrite_array(meter_idx,offset,gl_data.value,phy->data_len);
                    }
                }
            }
            break;
        case FHJL4_ZFXYG:
        case FHJL5_SXX_WG:
            // TODO ���⴦�� 
            pos = 0;
            
            bit_mask = 0;
            if(FHJL4_ZFXYG == phy->phy)
            {
                check_AA_len = 3;
            }
            if(FHJL5_SXX_WG == phy->phy)
            {
                check_AA_len = 2;
            }
            while(pos < datalen)
            {
                record_len = data[2+pos];//���ɼ�¼�ֽ���
                buffer_bcd_to_bin(data+3+pos,recv_td,5);
                //mem_cpy(energy_data.rec_date,td_bin,5);
                find_flag = FALSE;
                for(idx=0;idx<block_cnt;idx++)
                {
                    //
                    calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                    if(compare_string(td_bin,recv_td,5)==0)
                    {
                        mem_cpy(energy_data.rec_date,td_bin,5);
                        offset = get_curve_save_offset(phy,td_bin,midu);
                        find_flag = TRUE;
                        set_bit_value(&bit_mask,1,idx);// �������䷶Χ�ڵ�ʱ��
                        break;
                    }
                }
                if(TRUE == find_flag)
                {
                    /* ��ȡ�洢���ݣ����ʱ�겻�ԣ���ִ�к������� */
                    #if 1
                    fread_array(meter_idx,offset,var.load_dnsz,phy->data_len);
                    if(0 != compare_string(energy_data.rec_date,var.load_dnsz,5))
                    #endif
                    {
                        if( (data[0+pos] == 0xA0) && (data[1+pos] == 0xA0) && (record_len != 0)
                        && (TRUE == check_is_all_ch(data+3+pos+record_len-check_AA_len,check_AA_len,0xAA)) )
                        {
                            if(record_len >= 27)
                            {
                                //����ʾֵ copy
                                mem_cpy(energy_data.value+5,data+8+pos+6-check_AA_len,16);
                                //mem_set(v_data.value+5+15,3,REC_DATA_IS_DEFAULT);
                            }
                            else
                            {
                                // ����Ԥ�볤��ʱ���޷�ɸѡ��Ƶ�� ��ʱ��ô���� �����޸� TODO ?
                                //if(record_len > 11)// 5+17+6 
                                {
                                    //mem_cpy(v_data.value+5,data+8+pos,record_len-11);
                                    //mem_set(v_data.value+5+record_len-11,3,REC_DATA_IS_DEFAULT);
                                }
                                //else
                                {
                                    mem_set(energy_data.value+5,16,REC_DATA_IS_DEFAULT);
                                }
                            }
                            fwrite_array(meter_idx,offset,energy_data.value,phy->data_len);
                        }
                        else
                        {
                            mem_cpy(energy_data.rec_date,td_bin,5);
                            mem_set(energy_data.value+5,16,REC_DATA_IS_DEFAULT);
                            fwrite_array(meter_idx,offset,energy_data.value,phy->data_len);
                        }
                    }
                }

                pos+= 5+record_len;//������һ��Ҫ�������ݿ�
            }
            // �ظ�������û�е�ʱ�꣬д��0xEE
            for(idx=0;idx<block_cnt;idx++)
            {
                //
                if(get_bit_value(&bit_mask,1,idx) == 0)
                {
                    calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                    offset = get_curve_save_offset(phy,td_bin,midu);
                    mem_cpy(energy_data.rec_date,td_bin,5);
                    #if 1
                    fread_array(meter_idx,offset,var.load_dnsz,phy->data_len);
                    if(0 != compare_string(energy_data.rec_date,var.load_dnsz,5))
                    #endif
                    {
                        mem_set(energy_data.value+5,16,REC_DATA_IS_DEFAULT);
                        fwrite_array(meter_idx,offset,energy_data.value,phy->data_len);
                    }
                }
            }           
            break;
        case FHJL6_YG_WG_ZDN_14: // ���������޹��ܵ���
        case (FHJL6_YG_WG_ZDN_14-0x3F+0):   
        case FHJL7_SXX_WG_ZDN_14: // �������޹��ܵ���
        case (FHJL7_SXX_WG_ZDN_14-0x3F+0):
            if(datalen >= 5)//��Ҫ���� datalen ��ֹ�������
            {
                mem_cpy(recv_td,data,5);
                for(idx=0;idx<5;idx++)
                {
                    recv_td[idx] = BCD2byte(data[idx]);
                }
                if(compare_string(td_bin,recv_td,5)==0)
                {
                    copy_flag = TRUE;
                }
                read_pos = 5;
            }
            for(idx=0;idx<block_cnt;idx++)
            {
                calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                offset = get_curve_save_offset(phy,td_bin,midu);
                mem_cpy(elec_energy_data.rec_date,td_bin,5); 
                
                mem_set(elec_energy_data.value+5,16,0xFF);//REC_DATA_IS_DEFAULT
                if(TRUE == copy_flag)
                {
                    //
                    if(load_save_ctrl.data_size == 0) 
                    {
                        cnt = 1;
                    }
                    else
                    {
                        cnt = load_save_ctrl.save_len/load_save_ctrl.data_size;
                    }
                    //if(cnt > 1)
                    {
                        //�鳭�� cnt >1 ���೭��  cnt ���� 1
                        for(idx_sub=0;idx_sub<cnt;idx_sub++)
                        {
                            read_pos += load_save_ctrl.data_size;
                            // 5+idx*load_save_ctrl.save_len ����һ�鳤�� load_save_ctrl.data_size*idx_sub ���೤��
                            if( (FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size))
                                || (read_pos > datalen) )
                            {
                                //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                                mem_set(elec_energy_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size,REC_DATA_IS_DEFAULT);
                            }
                            else
                            {
                                mem_cpy(elec_energy_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size);
                            }
                        }
                    }
                }
                else
                {
                    mem_set(elec_energy_data.value+5+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                }
                #if 1
                fread_array(meter_idx,offset,var.load_dnsz,phy->data_len);
                //if(0 != compare_string(elec_energy_data.rec_date,var.load_dnsz,5)) 
                curve_exist_flag = FALSE;
                if((compare_string(td_bin,var.load_dnsz,5) == 0) && (FALSE == check_is_all_FF(var.load_dnsz+5+load_save_ctrl.offset,load_save_ctrl.save_len)) )
                {
                    curve_exist_flag = TRUE;
                }
                if(FALSE == curve_exist_flag)
                #endif
                {
                    fwrite_array(meter_idx,offset,elec_energy_data.value,phy->data_len);
                }
            }
            break;
        case (FHJL6_YG_WG_ZDN_14-0x3F+1):
        case (FHJL6_YG_WG_ZDN_14-0x3F+2):
        case (FHJL6_YG_WG_ZDN_14-0x3F+3):
        case (FHJL7_SXX_WG_ZDN_14-0x3F+1):
        case (FHJL7_SXX_WG_ZDN_14-0x3F+2):
        case (FHJL7_SXX_WG_ZDN_14-0x3F+3):
            if(datalen >= 5)//��Ҫ���� datalen ��ֹ�������
            {
                mem_cpy(recv_td,data,5);
                for(idx=0;idx<5;idx++)
                {
                    recv_td[idx] = BCD2byte(data[idx]);
                }
                if(compare_string(td_bin,recv_td,5)==0)
                {
                    copy_flag = TRUE;
                }
                read_pos = 5;
            }
            for(idx=0;idx<block_cnt;idx++)
            {
                calculat_curve_time(start_time,td_bin,remain_num+idx,midu);
                offset = get_curve_save_offset(phy,td_bin,midu);
                mem_cpy(elec_energy_data.rec_date,td_bin,5); 
                //��������
                fread_array(meter_idx,offset,var.load_dnsz,phy->data_len);
                mem_set(elec_energy_data.value+5,16,0xFF);
                if(compare_string(td_bin,var.load_dnsz,5)==0)
                {
                    mem_cpy(elec_energy_data.value+5,var.load_dnsz+5,16);
                }

                #if 1
                /* ʱ�� ��ͬ  �� ���ݲ���ȫ FF ˵���洢���� ���ܴ洢���γ��� */
                curve_exist_flag = FALSE;
                if((compare_string(td_bin,var.load_dnsz,5) == 0) && (FALSE == check_is_all_FF(var.load_dnsz+5+load_save_ctrl.offset,load_save_ctrl.save_len)) )
                {
                    curve_exist_flag = TRUE;
                    read_pos += load_save_ctrl.save_len;/*18-04-11 ������� ����洢������ */
                }
                if(FALSE == curve_exist_flag)
                #endif
                {
                    if(TRUE == copy_flag)
                    {
                        //
                        if(load_save_ctrl.data_size == 0) 
                        {
                            cnt = 1;
                        }
                        else
                        {
                            cnt = load_save_ctrl.save_len/load_save_ctrl.data_size;
                        }
                        //if(cnt > 1)
                        {
                            //�鳭�� cnt >1 ���೭��  cnt ���� 1
                            for(idx_sub=0;idx_sub<cnt;idx_sub++)
                            {
                                read_pos += load_save_ctrl.data_size;
                                // 5+idx*load_save_ctrl.save_len ����һ�鳤�� load_save_ctrl.data_size*idx_sub ���೤��
                                if( (FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size))
                                    || (read_pos > datalen) )
                                {
                                    //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                                    mem_set(elec_energy_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size,REC_DATA_IS_DEFAULT);
                                }
                                else
                                {
                                    mem_cpy(elec_energy_data.Block+load_save_ctrl.offset+load_save_ctrl.data_size*idx_sub,data+5+idx*load_save_ctrl.save_len+load_save_ctrl.data_size*idx_sub,load_save_ctrl.data_size);
                                }
                            }
                        }
                        /*
                        if(FALSE == is_valid_bcd(data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len))
                        {
                            //���ó�EE ˵�����Ϊ���Ḻ�ɣ������ն˲��ٳ���
                            mem_set(gl_data.value+5+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                        }
                        else
                        {
                            mem_cpy(gl_data.value+5+load_save_ctrl.offset,data+5+idx*load_save_ctrl.save_len,load_save_ctrl.save_len);
                        }
                        */
                    }
                    else
                    {
                        mem_set(elec_energy_data.value+5+load_save_ctrl.offset,load_save_ctrl.save_len,REC_DATA_IS_DEFAULT);
                    }
                    fwrite_array(meter_idx,offset,elec_energy_data.value,phy->data_len);
                }
            }
            break;
    }    
}

INT8U check_save_last_load_data_LiaoNing(READ_PARAMS *read_params,READ_WRITE_DATA *phy,INT8U remain_num, INT8U *block_cnt,INT8U* start_time) //start_time�����15�ֵ���������byte��ʽ
{
    INT32U offset;
    //C_VOLTMETER_BLOCK v_data;
    //C_POWER_BLOCK gl_data;
    INT16U meter_idx = 0;
    union{
        INT8U value[40];
    struct{
        INT8U rec_date[5];
        INT8U data[35]; /* ע�����ռ�Ҫ�㹻 */ 
        };
    }var;
    INT8U midu,i,r_num;//idx,num,
    //INT8U td[5] = {0};
    INT8U td_bin[5] = {0};
    INT8U BlockCnt = 0;
    INT8U check_pos = 0;
    INT8U check_len = 0;
    INT8U idx_sub = 0;
    BOOLEAN flag =  FALSE;
    //INT8U load_cnt = 0;/* ���ݴ��ڵ����� ���� */
    midu = 15;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    
    BlockCnt = load_record_read_cnt(phy->phy);

    if( (0xFF == BlockCnt) || (0x00 == BlockCnt) )
    {
        return 96;// ����95 ������� ������
    }
    //#if ( (defined __PROVICE_SHANGHAI__) || (defined __PROVICE_SHANGHAI_FK__) )
    if( (COMMPORT_PLC == read_params->meter_doc.baud_port.port) || (COMMPORT_PLC_REC == read_params->meter_doc.baud_port.port) )
    {
        if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PATCH_TODAY_LOAD_RECORD))
        {
            if( check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD) )
            {
                /* 24 �� */
                if( check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_24_DOT) )
                {
                    *block_cnt = 1;//�ز� һ��һ���㲹�� Сʱ
                    midu = 60; // 60����
                    BlockCnt = 1;
                }
                else /* 96 �� */
                {
                    *block_cnt = BlockCnt;
                }
                
            }            
            else
            { 
                /*���ش����ֵ ���ٳ���*/
                return 96;
            }
        }
        else
        {
                        /* 24 �� */
                if( check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_24_DOT) )
                {
                    *block_cnt = 1;//�ز� һ��һ���㲹�� Сʱ
                    midu = 60; // 60����
                    BlockCnt = 1;
                }
                else
                {
                    *block_cnt = BlockCnt;//
                }
            
        }
    }
    else
    {
        *block_cnt = BlockCnt;//
    }
    switch(phy->phy)
    {
        case FHJL1_DY_DL_PL:
        case FHJL1_DY_14:
        case (FHJL1_DY_14-0x3F+0): // A ��ѹ
        case (FHJL1_DY_14-0x3F+1): // B ��ѹ
        case (FHJL1_DY_14-0x3F+2): // C ��ѹ
        case FHJL2_DL_14:
        case (FHJL2_DL_14-0x3F+0): // A ����
        case (FHJL2_DL_14-0x3F+1): // B ����
        case (FHJL2_DL_14-0x3F+2): // C ����
            phy->data_len = 23;
            phy->offset = PIM_CURVE_V_I ; //?
            if(FHJL1_DY_DL_PL == phy->phy)
            {
                check_pos = 5;
                check_len = 15;// 6+9 = 15
            }
            else if(FHJL1_DY_14 == phy->phy) //��ѹ�鸺��
            {
                //
                check_pos = 5;
                check_len = 6;
            }
            else if((FHJL1_DY_14-0x3F+0) == phy->phy) // A ���ѹ����
            {
                check_pos = 5;
                check_len = 2;
            }
            else if((FHJL1_DY_14-0x3F+1) == phy->phy) // B ���ѹ����
            {
                check_pos = 7;
                check_len = 2;
            }
            else if((FHJL1_DY_14-0x3F+2) == phy->phy) // C ���ѹ����
            {
                check_pos = 9;
                check_len = 2;
            }
            else if(FHJL2_DL_14 == phy->phy) //�����鸺��
            {
                check_pos = 11;
                check_len = 9;
            }
            else if((FHJL2_DL_14-0x3F+0) == phy->phy) // A ����� ����
            {
                check_pos = 11;
                check_len = 3;
            }
            else if((FHJL2_DL_14-0x3F+1) == phy->phy) // B ����� ����
            {
                check_pos = 14;
                check_len = 3;
            }
            else if((FHJL2_DL_14-0x3F+2) == phy->phy) // C ����� ����
            {
                check_pos = 17;
                check_len = 3;
            }
            else
            {
                check_pos = 5;
                check_len = 0;
            }
            break;
        case FHJL2_YG_WG_GL:
        case FHJL3_YGGL_14:
        case (FHJL3_YGGL_14-0x3F+0):
        case (FHJL3_YGGL_14-0x3F+1):
        case (FHJL3_YGGL_14-0x3F+2):
        case (FHJL3_YGGL_14-0x3F+3):
        case FHJL4_WGGL_14:
        case (FHJL4_WGGL_14-0x3F+0):
        case (FHJL4_WGGL_14-0x3F+1):
        case (FHJL4_WGGL_14-0x3F+2):
        case (FHJL4_WGGL_14-0x3F+3):
        case FHJL5_GLYS_14:
        case (FHJL5_GLYS_14-0x3F+0):
        case (FHJL5_GLYS_14-0x3F+1):
        case (FHJL5_GLYS_14-0x3F+2):
        case (FHJL5_GLYS_14-0x3F+3):
            phy->data_len = 37;
            phy->offset = PIM_CURVE_GL; //?
            if(FHJL2_YG_WG_GL == phy->phy)//�й��޹� ���ɼ�¼
            {
                check_pos = 5;
                check_len = 24;
            }
            else if(FHJL3_YGGL_14 == phy->phy) //�й�����
            {
                //
                check_pos = 5;
                check_len = 12;
            }
            else if((FHJL3_YGGL_14-0x3F+0) == phy->phy) //���й�����
            {
                //
                check_pos = 5;
                check_len = 3;
            }
            else if((FHJL3_YGGL_14-0x3F+1) == phy->phy) // A �й�����
            {
                //
                check_pos = 8;
                check_len = 3;
            }
            else if((FHJL3_YGGL_14-0x3F+2) == phy->phy) //B �й�����
            {
                //
                check_pos = 11;
                check_len = 3;
            }
            else if((FHJL3_YGGL_14-0x3F+3) == phy->phy) //C �й�����
            {
                //
                check_pos = 14;
                check_len = 3;
            }
            else if(FHJL4_WGGL_14 == phy->phy) //�޹�����
            {
                //
                check_pos = 17;
                check_len = 12;
            }
            else if((FHJL4_WGGL_14-0x3F+0) == phy->phy) //���޹�����
            {
                //
                check_pos = 17;
                check_len = 3;
            }
            else if((FHJL4_WGGL_14-0x3F+1) == phy->phy) //A �޹�����
            {
                //
                check_pos = 20;
                check_len = 3;
            }
            else if((FHJL4_WGGL_14-0x3F+2) == phy->phy) //B �޹�����
            {
                //
                check_pos = 23;
                check_len = 3;
            }
            else if((FHJL4_WGGL_14-0x3F+3) == phy->phy) //C �޹�����
            {
                //
                check_pos = 26;
                check_len = 3;
            }
            else if(FHJL5_GLYS_14 == phy->phy) //��������
            {
                //
                check_pos = 29;
                check_len = 8;
            }
            else if((FHJL5_GLYS_14-0x3F+0) == phy->phy) //�ܹ�������
            {
                //
                check_pos = 29;
                check_len = 2;
            }
            else if((FHJL5_GLYS_14-0x3F+1) == phy->phy) //A ��������
            {
                //
                check_pos = 31;
                check_len = 2;
            }
            else if((FHJL5_GLYS_14-0x3F+2) == phy->phy) //B ��������
            {
                //
                check_pos = 33;
                check_len = 2;
            }
            else if((FHJL5_GLYS_14-0x3F+3) == phy->phy) //C ��������
            {
                //
                check_pos = 35;
                check_len = 2;
            }
            else
            {
                check_pos = 5;
                check_len = 0;
            }
            break;
        //     
        case FHJL3_GL_YS:
            phy->data_len = 37;
            phy->offset = PIM_CURVE_GL; //?
            check_pos = 29;
            check_len = 8;
            break;
        case FHJL4_ZFXYG: // ���������޹�����ʾֵ
        case FHJL6_YG_WG_ZDN_14:
        case (FHJL6_YG_WG_ZDN_14-0x3F+0):// �����й�
        case (FHJL6_YG_WG_ZDN_14-0x3F+1):
        case (FHJL6_YG_WG_ZDN_14-0x3F+2):
        case (FHJL6_YG_WG_ZDN_14-0x3F+3):
            phy->data_len = 21;
            phy->offset = PIM_CURVE_ZFXYWG; //?
            if(FHJL4_ZFXYG == phy->phy)
            {
                check_pos = 5;
                check_len = 16;
            }
            else if(FHJL6_YG_WG_ZDN_14 == phy->phy)
            {
                check_pos = 5;
                check_len = 16;
            }
            else if((FHJL6_YG_WG_ZDN_14-0x3F+0) == phy->phy)
            {
                check_pos = 5;
                check_len = 4;
            }
            else if((FHJL6_YG_WG_ZDN_14-0x3F+1) == phy->phy)
            {
                check_pos = 9;
                check_len = 4;
            }
            else if((FHJL6_YG_WG_ZDN_14-0x3F+2) == phy->phy)
            {
                check_pos = 13;
                check_len = 4;
            }
            else if((FHJL6_YG_WG_ZDN_14-0x3F+3) == phy->phy)
            {
                check_pos = 17;
                check_len = 4;
            }
            else
            {
                check_pos = 5;
                check_len = 0;
            }
            break;
        case FHJL5_SXX_WG:
        case FHJL7_SXX_WG_ZDN_14:
        case (FHJL7_SXX_WG_ZDN_14-0x3F+0):// ��һ�����޹��ܵ���
        case (FHJL7_SXX_WG_ZDN_14-0x3F+1):
        case (FHJL7_SXX_WG_ZDN_14-0x3F+2):
        case (FHJL7_SXX_WG_ZDN_14-0x3F+3):
            phy->data_len = 21;
            phy->offset = PIM_CURVE_WG1234; //?
            if(FHJL5_SXX_WG == phy->phy)
            {
                check_pos = 5;
                check_len = 16;
            }
            else if(FHJL7_SXX_WG_ZDN_14 == phy->phy)
            {
                check_pos = 5;
                check_len = 16;
            }
            else if((FHJL7_SXX_WG_ZDN_14-0x3F+0) == phy->phy)// ��һ�����޹��ܵ���
            {
                check_pos = 5;
                check_len = 4;
            }
            else if((FHJL7_SXX_WG_ZDN_14-0x3F+1) == phy->phy)// �ڶ������޹��ܵ���
            {
                check_pos = 9;
                check_len = 4;
            }
            else if((FHJL7_SXX_WG_ZDN_14-0x3F+2) == phy->phy)// ���������޹��ܵ���
            {
                check_pos = 13;
                check_len = 4;
            }
            else if((FHJL7_SXX_WG_ZDN_14-0x3F+3) == phy->phy)// ���������޹��ܵ���
            {
                check_pos = 17;
                check_len = 4;
            }
            else
            {
                check_pos = 5;
                check_len = 0;
            }
            break;
    }

    r_num = remain_num;
    flag = FALSE;
    for(i=0;i<(read_params->patch_load_curve_cnt-r_num);i+=BlockCnt) //���ѭ��һ��Ҫ����һ��ʵ��ϵͳ�ϵ�ִ��ʱ�䣬��ֹ�����������񣡣�
    {
        #if 1
        //load_cnt = 0;
        /* ��鳭�� һ����һ������  */
        for(idx_sub=0;idx_sub< BlockCnt;idx_sub++)
        {
            
            if(remain_num + idx_sub >= read_params->patch_load_curve_cnt)
            {
                remain_num += idx_sub;
                return remain_num;
            }
            calculat_curve_time(start_time,td_bin,remain_num+idx_sub,midu);

            offset = get_curve_save_offset(phy,td_bin,midu);
            fread_array(meter_idx,offset,var.value,phy->data_len);
            /* 14���� ��ѹ�������������ٿ���???? TODO ?����ʵ�ʳ�������ٲ鿴�°�17-09-12
            
            */
            if( (FHJL1_DY_DL_PL == phy->phy) || (FHJL2_YG_WG_GL == phy->phy) 
              ||(FHJL4_ZFXYG == phy->phy) || (FHJL5_SXX_WG == phy->phy)
              ||(FHJL1_DY_14 == phy->phy) || (FHJL3_YGGL_14 == phy->phy)
              ||(FHJL6_YG_WG_ZDN_14 == phy->phy) || (FHJL7_SXX_WG_ZDN_14 == phy->phy) )
            {
                // ֻ���td,��Ϊ�洢��ʱ�򣬶���һ���Դ洢
                if (compare_string(td_bin,var.rec_date,5) != 0)
                {
                    flag = TRUE;/* 18-03-09 add */
                    break;
                }                
            }
            else
            {
                // ����ȫFF�ʹ���д�����
                if( (compare_string(td_bin,var.rec_date,5) == 0) && (FALSE == check_is_all_FF(var.value+check_pos,check_len)) )
                  //&& ( (TRUE == is_valid_bcd(var.value+check_pos,check_len)) || (check_is_all_ch(var.value+check_pos,check_len,0xEE)) ) )
                {
                    //remain_num += BlockCnt;
                    //load_cnt ++;
                }
                else
                {
                    flag = TRUE;
                    break;
                }
            }
        }
        if(TRUE == flag)
        {
            break;
        }
        remain_num += BlockCnt;
        #else
        calculat_curve_time(start_time,td_bin,remain_num,midu);

        offset = get_curve_save_offset(phy,td_bin,midu);
        fread_array(meter_idx,offset,var.value,phy->data_len);
        // 14���� ��ѹ�������������ٿ���???? TODO ?����ʵ�ʳ�������ٲ鿴�°�17-09-12
        if( (FHJL1_DY_DL_PL == phy->phy) || (FHJL2_YG_WG_GL == phy->phy) 
          ||(FHJL1_DY_14 == phy->phy) || (FHJL3_YGGL_14 == phy->phy) )
        {
            // ֻ���td,��Ϊ�洢��ʱ�򣬶���һ���Դ洢
            if (compare_string(td_bin,var.rec_date,5) == 0)
            {
                remain_num += BlockCnt;
            }
            else
            {
                break;
            }
        }
        else
        {
            // ����ȫFF�ʹ���д�����
            if( (compare_string(td_bin,var.rec_date,5) == 0)  && (FALSE == check_is_all_FF(var.value+check_pos,check_len)) )
              //&& ( (TRUE == is_valid_bcd(var.value+check_pos,check_len)) || (check_is_all_ch(var.value+check_pos,check_len,0xEE)) ) )
            {
                remain_num += BlockCnt;
            }
            else
            {
                break;
            }
        }
        #endif
    }
    return remain_num;   
}

void set_israel_di_item(INT8U *data,INT32U phy)
{

  switch(phy)
  {
    case FHJL1_DY_DL_PL :
        data[0] = 7;
        data[1] = 0;
        data[2] = 0x0C;
        data[3] = 0;
        break;
    case FHJL2_YG_WG_GL :
    case FHJL4_ZFXYG :
        data[0] = 0xF8;
        data[1] = 3;
        data[2] = 0;
        data[3] = 0;
        break;
    case FHJL3_GL_YS :
        data[0] = 0;
        data[1] = 0xFC;
        data[2] = 3;
        data[3] = 0;
        break;
  default :
        data[0] = 0xFF;
        data[1] = 0xFF;
        data[2] = 0x0F;
        data[3] = 0;
        break;
  }

}
#if defined(__PRECISE_TIME__)

BOOLEAN prepare_plc_read_meter_precise_time(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len,INT16U delay,BOOLEAN is_f3)
{
    INT32U offset;
    INT16U meter_idx;
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT8U frame_pos,had_read;
    INT8U ctrl_flag[2],td[3];
    SET_F297 F297;
    SET_F298 F298;
    PRECISE_TIME_FILE_CTRL file_ctrl;
    had_read = 0;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    #ifdef __READ_OOP_METER__ /*ֻ����07��97Э�飬*/
    if((read_params->meter_doc.protocol == GB645_2007) || (read_params->meter_doc.protocol == GB645_1997))
    {

    }
    else
    {
        return FALSE;
    }
    #endif
    td[0] = byte2BCD(datetime[DAY]);
    td[1] = byte2BCD(datetime[MONTH]);
    td[2] = byte2BCD(datetime[YEAR]);
    previous_dayhold_td(td);
    offset = td[0] - 1;
    offset += PIM_METER_PRECISE_TIME_HAD_READ;
    fread_array(meter_idx,offset,&had_read,1);
    //�鿴���룬�������ݵ�ʱ��Ӧ�ú�F297û�й�ϵ��������Ĵ���ע����

    fread_ertu_params(EEADDR_SET_F297,F297.value,sizeof(SET_F297));
    
    if (F297.flag != 1) return FALSE;
    if(F297.cycle == 0)F297.cycle = 1;
    if ((datetime[DAY] % F297.cycle) != 0) return FALSE;
    /*
    if (datetime[HOUR] < BCD2byte(F297.bcd_begin_hour)) return FALSE;
    else if(datetime[HOUR] == BCD2byte(F297.bcd_begin_hour))
    {
        if (datetime[MINUTE] < BCD2byte(F297.bcd_begin_minute)) return FALSE;
    }
    if (datetime[HOUR] > BCD2byte(F297.bcd_end_hour)) return FALSE;
    else if (datetime[HOUR] == BCD2byte(F297.bcd_end_hour))
    {
        if (datetime[MINUTE] > BCD2byte(F297.bcd_end_minute)) return FALSE;
    }
     */
    read_params->control.loop = TRUE;

    if( had_read == 0x55)return FALSE;

    fread_array(FILEID_PRECISE_TIME_DATA,PIM_PRECISE_TIME_CTRL,file_ctrl.value,sizeof(PRECISE_TIME_FILE_CTRL));
    if(file_ctrl.result != 0xAA) return FALSE;    //������Уʱû�гɹ��Ļ������Ե�����Уʱ
    fread_ertu_params(EEADDR_SET_F298,F298.value,sizeof(SET_F298));
    if (F298.flag == 0) return FALSE;
    if (F298.flag == 4) return FALSE; //4����ʾ����Ҫ�������ʱ��
    if (F298.flag > 4) return FALSE;
    if(F298.cycle == 0)F298.cycle = 1;
    if ((datetime[DAY] % F298.cycle) != 0) return FALSE;
    if (datetime[HOUR] < BCD2byte(F298.bcd_begin_hour)) return FALSE;
    else if(datetime[HOUR] == BCD2byte(F298.bcd_begin_hour))
    {
        if (datetime[MINUTE] < BCD2byte(F298.bcd_begin_minute)) return FALSE;
    }
    if (datetime[HOUR] > BCD2byte(F298.bcd_end_hour)) return FALSE;
    else if (datetime[HOUR] == BCD2byte(F298.bcd_end_hour))
    {
        if (datetime[MINUTE] > BCD2byte(F298.bcd_end_minute)) return FALSE;
    }

 //   if(system_flag & SYS_CAST_TIME_DONE)  return FALSE;

AGAIN:
    frame_pos = 0;
    if(!is_f3)
   {
    frame[frame_pos++] = 0x02; //���Գ���
    #ifdef __376_2_2013__
    if(GPLC.DL698_new_protocol_param.rec_delay_flag) frame[frame_pos++] = 0x00; //ͨ����ʱ����Ա�־
    #endif
   }
//    mem_cpy(portContext->router_work_info.ADDR_DST,read_params->meter_doc.meter_no,6);

    fread_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_FLAG,ctrl_flag,2); //������ô�洢�����Σ�



    if (ctrl_flag[0] & 0x01)
    {
        phy.phy = RQ_XQ;  //0x04000101;
        library.item = 0x04000101;

        int32u2_bin(phy.phy,read_params->phy);
        int32u2_bin(library.item,read_params->item);
        read_params->resp_byte_num = 40;
        read_params->read_type = READ_TYPE_CYCLE_DAY;

        if (read_params->meter_doc.protocol == GB645_2007)
        {
            *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
        }
        else if (read_params->meter_doc.protocol == GB645_1997)
        {
            *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
        }
        else
        {
            return FALSE;
        }

        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** prepare item prescise time : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
                meter_idx,library.item,phy.phy,0x55);
        debug_println_ext(info);
        #endif

      return TRUE;

    }
    else if (ctrl_flag[0] & 0x02)
    {
        phy.phy = SJ;  //0x04000102;

        int32u2_bin(phy.phy,read_params->phy);
        library.item = 0x04000102;
        int32u2_bin(library.item,read_params->item);
        read_params->resp_byte_num = 40;
        read_params->read_type = READ_TYPE_CYCLE_DAY;

        if (read_params->meter_doc.protocol == GB645_2007)
        {
            *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
        }
        else if (read_params->meter_doc.protocol == GB645_1997)
        {
            *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
        }
        else
        {
            return FALSE;
        }

        #ifdef __SOFT_SIMULATOR__
      //  snprintf(info,100,"*** prepare item day hold : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
      //          meter_idx,library.item,phy.phy,mask_idx);
      //  debug_println_ext(info);
        #endif

        return TRUE;
    }
    else if (ctrl_flag[0] & 0x04) //���㲥Уʱ
    {
       // phy.phy = JQDS_MN;
       // library.item = 0x04000100;   //����һ��������33343337
        if(is_f3)
        {
            if (F298.flag == 1) //1������ָ������ַ�Ĺ㲥��ʱ���
            {
                frame[0] = make_gb645_adj_time_frame(frame+1,portContext_plc.router_work_info.ADDR_DST,delay);
            }
            else if (F298.flag == 2) //2������ָ���ɼ����Ĺ㲥��ʱ���
            {
              frame[0] = make_gb645_precise_time_frame(frame+1,portContext_plc.router_work_info.ADDR_DST,read_params->meter_doc.protocol,delay);
            }
        }
        else
        {
            #ifdef __376_2_2013__
            if(GPLC.DL698_new_protocol_param.rec_delay_flag) frame[1] = 0x01; //ͨ����ʱ����Ա�־
            #endif
            //fread_array(FILEID_PLC_REC_TMP+GPLC.phase-1,PIM_METER_PRECISE_TIME_CYCLE,&cycle,1);
            if (ctrl_flag[1] & 0x01)
            {
                ctrl_flag[1] = ctrl_flag[1]<<1;
                fwrite_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_CYCLE,ctrl_flag+1,1);

                if (F298.flag == 1) //1������ָ������ַ�Ĺ㲥��ʱ���
                {
                    frame[frame_pos] = make_gb645_adj_time_frame(frame+frame_pos+1,NULL,delay);
                }
                else if (F298.flag == 2) //2������ָ���ɼ����Ĺ㲥��ʱ���
                {
                    frame[frame_pos] = make_gb645_precise_time_frame(frame+frame_pos+1,portContext_plc.router_work_info.ADDR_DST,read_params->meter_doc.protocol,delay);
                }
            }
            else
            {
                ctrl_flag[0] = 0x03;
                fwrite_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_FLAG,ctrl_flag,1);
                goto AGAIN;
            }
        }
        return TRUE;
    }

    return FALSE;
}
#endif

/* �������ûʹ�ã���ע�͵�
INT8U check_cjq_meter_more_data(INT16U meter_idx)
{
    BOOLEAN is_F35;
    INT16U meter_seq;
    INT8U params[2],tmp[2],value;
    INT8U idx;
    meter_seq = meter_idx&FAST_IDX_MASK;
    for(idx=0;idx<sizeof(CURVE_REC_FN)/sizeof(INT8U);idx++)
    {
        fread_array(meter_seq,PIM_METER_F104+idx*2,params,2);
        if(check_curve_fn_list(params[0]) >= sizeof(CURVE_REC_FN)/sizeof(INT8U))  continue;
        if(params[0] == 0xFF)
        {
            continue;
        }
        if (get_curve_midu(params[1]) == 0)
        {
            continue;
        }
        mem_cpy(tmp,params,2);
    }

 //   fread_array(meter_idx,PIM_METER_F104,params,20);
    if((tmp[0] != 0xFF)&&(tmp[1] != 0xFF))return true;

    fread_ertu_params(EEADDR_SET_F35+(meter_idx/8),&value,1);
    is_F35 = ((~value) & (0x01<<(meter_idx%8))) ? TRUE : FALSE;
    if(is_F35)return true;



    return false;
}
*/


INT8U get_phy_form_list_last_curve_cycle_day(INT32U phy,READ_WRITE_DATA* out)
{
    INT8U idx,idx_sub;
    INT8U mask_idx;

    mask_idx = 0;

    for(idx=0;idx<sizeof(LAST_CURVE_CYCLE_DAY_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((LAST_CURVE_CYCLE_DAY_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            get_phy_data((READ_WRITE_DATA*)&(LAST_CURVE_CYCLE_DAY_PHY_LIST[idx]),idx_sub,out);
            if(out->phy == phy)
            {
                return mask_idx;
            }
            mask_idx++;
        }
    }

    return 0xFF;
}

INT32U get_last_curve_cycle_day_save_offset(READ_WRITE_DATA *phy,INT8U td[3])
{
    INT32U offset;

    offset = getPassedDays(2000+td[2],td[1],td[0]);
    offset = offset % SAVE_POINT_NUMBER_DAY_HOLD;
    offset *= phy->data_len;
    offset += phy->offset;
    return offset;
}


INT8U writedata_last_curve_cycle_day(READ_PARAMS *read_params,INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[3],INT8U *data,INT8U datalen,INT8U* buffer,INT8U *reserve_data,INT8U remain_num,INT8U* start_time)
{
    INT32U offset;
    #ifdef __COUNTRY_ISRAEL__
    INT8U temp_data_len;
    #endif

    offset = get_last_curve_cycle_day_save_offset(phy,td);

    fread_array(meter_idx,offset,buffer,phy->data_len);

    //����ʱ��
    if(compare_string(buffer,td,3) != 0)
    {
        mem_set(buffer,phy->data_len,0xFF);
        mem_cpy(buffer,td,3);

        mem_cpy(buffer+3,datetime+MINUTE,5);

    }
    #ifdef __COUNTRY_ISRAEL__
    temp_data_len = datalen;
    #endif
    /*485 ���� �ز������˲������� */
    if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD)
      || check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_485_PATCH_LOAD_RECORD) )
    {
        if(FALSE == check_valid_load_record_phy(phy->phy))// �ฺ�ɿ飬������phy����ִ��copy        
        {
            datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
        }
    }
    else
    {
        datalen = (datalen > phy->block_len) ? phy->block_len : datalen;
    }
    mem_cpy(buffer+8+phy->block_offset,data,datalen);


    if(reserve_data)
    {
        mem_cpy(buffer+phy->data_len-RESERVE_DATA,reserve_data,RESERVE_DATA);
    }
//֮ǰ�����д洢�����ڲ��ô��ˣ����԰������Ҫ�󣬶�������ɣ�
   #ifdef __PLC_GUANGXI_VIP_CURVE__
    if((phy->phy == SYC_ZDDJ_YG) || (phy->phy == SYC_ZDDJ_WG) )
    plc_router_save_last_curve_data(meter_idx,phy,buffer+8,phy->data_len,remain_num);

    if(phy->phy == SYC_GXGY_YG )
    plc_router_save_last_gx_load_data(meter_idx,phy,buffer+8,phy->data_len,remain_num);
   #endif
    /*#ifdef __LOAD_RECORD_BLOCK__
    if((phy->phy == FHJL1_DY_DL_PL) || (phy->phy == FHJL2_YG_WG_GL)|| (phy->phy == FHJL3_GL_YS) )
    {
        #ifdef __ANHUI_485_PATCH_96_CURVE__
        save_last_load_data_ah(meter_idx,phy,buffer+8,phy->data_len,remain_num,start_time);
        #else
        plc_router_save_last_load_data(meter_idx,phy,buffer+8,phy->data_len,remain_num);
        #endif
    }
    #endif */
    #ifdef __COUNTRY_ISRAEL__
    else if(phy->phy == ISRAEL_LOAD_PROFILE)
    {
        save_last_load_profile_data_israel(meter_idx,phy,data,temp_data_len,remain_num,start_time); /*��data��������ȫ������ȥ*/
    }
    else if(phy->phy == ISRAEL_HOUR_DATA)
    {

        if(datalen == 37) //����ǵ������ѹ�͵�������ת�������򳤶Ȳ�һ��
        {
            single_meter_tran_three_meter(data+2);
            datalen = 45;
        }

       save_last_load_hour_data_israel(meter_idx,phy,data,datalen,remain_num,start_time); /*��data��������ȫ������ȥ*/
    }
    #endif
    if (check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_AH_485_PATCH_96_LOAD_RECORD))
    {
        //��ʱ����ʹ�õ� ����485 96�㸺��
        if((phy->phy == FHJL1_DY_DL_PL) || (phy->phy == FHJL2_YG_WG_GL)|| (phy->phy == FHJL3_GL_YS)
        || (phy->phy == FHJL4_ZFXYG) )
        {
            save_last_load_data_ah(meter_idx,phy,buffer+8,phy->data_len,remain_num,start_time);
        }
    }
    if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD)
      || check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_485_PATCH_LOAD_RECORD) )
    {
        if(TRUE == check_valid_load_record_phy(phy->phy))
        {
            save_last_load_data_LiaoNing(read_params,meter_idx,phy,buffer+8+phy->block_offset,datalen,remain_num,start_time);
        }
    }    
   #ifdef  __PLC_REC_VOLTMETER1__
    if((phy->phy == DY_A_HGL) || (phy->phy == DY_B_HGL) || (phy->phy == DY_C_HGL))
    plc_router_save_voltmeter_data(meter_idx,phy->phy,buffer+8,phy->data_len,0);
    if(phy->phy == DY_HOUR_HOLD)//��ѹ���������ݴ洢
    plc_router_save_vlotmeter_curve_data(meter_idx,phy,buffer+8,phy->data_len,remain_num);
   #endif
    return TRUE;
}

INT8U readdata_last_curve_cycle_day(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[3],INT8U rec_datetime[5],INT8U *data,INT8U *datalen,INT8U *reserve_data)
{
    INT32U offset;

    offset = get_last_curve_cycle_day_save_offset(phy,td);

    fread_array(meter_idx,offset,data,phy->data_len);
    if ((td[2] == data[2]) && (td[1] == data[1]) && (td[0] == data[0]))
    {
        mem_cpy(rec_datetime,data+3,5);
        if(check_is_all_ch(data+8+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+8+phy->block_offset,phy->block_len);
            if(reserve_data)
            {
                mem_cpy(reserve_data,data+phy->data_len-RESERVE_DATA,RESERVE_DATA);
            }

            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

INT16S app_readdata_last_curve_cycle_day(INT16U meter_idx,INT32U phy_id,INT8U td[3],INT8U rec_datetime[5],INT8U *data,INT8U max_datalen)
{
    READ_WRITE_DATA phy;
    INT8U idx,datalen;
    INT8U td_bin[3];

    datalen = 0;
    idx = get_phy_form_list_last_curve_cycle_day(phy_id,&phy);
    if(idx == 0xFF) return -1;
    if (phy.data_len > max_datalen) return -1;

    td_bin[0] = BCD2byte(td[0]);
    td_bin[1] = BCD2byte(td[1]);
    td_bin[2] = BCD2byte(td[2]);
    readdata_last_curve_cycle_day(meter_idx,&phy,td_bin,rec_datetime,data,&datalen,NULL);

    return datalen;
}

INT8U save_last_curve_cycle_day(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    INT8U idx;
    //INT8U begin_td[3],end_td[3];
    #ifdef __SICHUAN_FK_PATCH_CURVE_DATA__
    save_sichuan_patch_curve_data(read_params,frame,frame_len,buffer);
    return 0;
    #endif
    idx = get_phy_form_list_last_curve_cycle_day(bin2_int32u(read_params->phy),&phy);
    if(idx == 0xFF) return 0;
    if (phy.flag & SAVE_FLAG_DENY_NO_SAVE)
    {
        if((frame_len!=0)&&(check_is_all_ch(frame,frame_len,0xEE))) //����
        {
            //����msak
  //          clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_CYCLE_DAY,idx);
             if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_AH_485_PATCH_96_LOAD_RECORD))
             {
                 clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_LAST_CURVE_CYCLE_DAY,idx);
                 read_params->patch_num = 96;
             }
             if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD)
             || check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_485_PATCH_LOAD_RECORD) )
             {
                 clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                 read_params->patch_num = 0 ;
                 clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_LAST_CURVE_CYCLE_DAY,idx);
                 return 0;
             }

             #ifdef __COUNTRY_ISRAEL__
             clear_read_mask_from_meter_param(bin2_int16u(read_params->meter_doc.meter_idx),READ_TYPE_LAST_CURVE_CYCLE_DAY,idx);
             read_params->patch_num = 96;
             #endif

        }
        else
        {

            #ifdef __PLC_REC_VOLTMETER1__
            if (bin2_int32u(read_params->phy) == DY_HOUR_HOLD)
            writedata_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,NULL,read_params->patch_num);
            else
            #endif
              writedata_last_curve_cycle_day(read_params,bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,NULL,read_params->patch_num,read_params->patch_start_time);
//            writedata_last_curve_cycle_day(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,NULL,0xFF);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,150,"*** save patch hold curve data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , start_td = %02d-%02d-%02d-%02d-%02d, num = %2d",
                    bin2_int16u(read_params->meter_doc.meter_idx),
                    bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
                    (read_params->patch_start_time[4]),(read_params->patch_start_time[3]),(read_params->patch_start_time[2]),(read_params->patch_start_time[1]),(read_params->patch_start_time[0]),
                    read_params->patch_num);
            debug_println_ext(info);
            #endif
        }
    }
    else
    {
        writedata_last_curve_cycle_day(read_params,bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->day_hold_td,frame,frame_len,buffer,NULL,0xFF,NULL);
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** save day hold data : meter_idx = %d , item = 0x%08X , phy = 0x%08X , td = %02d-%02d-%02d",
                bin2_int16u(read_params->meter_doc.meter_idx),
                bin2_int32u(read_params->item),bin2_int32u(read_params->phy),
                read_params->day_hold_td[2],read_params->day_hold_td[1],read_params->day_hold_td[0]);
        debug_println_ext(info);
        #endif
    }
    #ifdef __PLC_REC_VOLTMETER1__
    if (bin2_int32u(read_params->phy) == DY_HOUR_HOLD) //��ѹ�����ݳ�����һ�յ�ѹ��¼������ to do
    {
         if(read_params->patch_num >= 23) //���ݳ�������23�㶳��
         {
         clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,idx);
         read_params->patch_num = 0 ;
         }
         else
         read_params->patch_num ++ ;

         return 0;
    }
    #endif
    #ifdef __PLC_GUANGXI_VIP_CURVE__
    if ((bin2_int32u(read_params->phy) == SYC_ZDDJ_YG) || (bin2_int32u(read_params->phy) == SYC_ZDDJ_WG))  //��ѹ�����ݳ�����һ�յ�ѹ��¼������ to do
    {
         if(read_params->patch_num >= 24) //���㶳�����ݣ�������24��
         {
         clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
         read_params->patch_num = 0 ;
         }
         else
         {
         read_params->patch_num ++ ;
         read_params->item_add_num ++ ;
         }
         return 0;
    }
    if ((bin2_int32u(read_params->phy) == SYC_GXGY_YG) )  //������Լ��������ɼ�¼
    {
         if(read_params->patch_num >= 5) //���㶳�����ݣ�������6��
         {
         clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_LAST_DAY_HOLD_CURVE_GXFH,idx);
         read_params->patch_num = 0 ;
         }
         else
         {
         read_params->patch_num ++ ;

         }
         return 0;
    }
    #endif

    /*#ifdef __LOAD_RECORD_BLOCK__
    if ((bin2_int32u(read_params->phy) == FHJL1_DY_DL_PL) || (bin2_int32u(read_params->phy) == FHJL2_YG_WG_GL)|| (bin2_int32u(read_params->phy) == FHJL3_GL_YS))  //��ѹ�����ݳ�����һ�յ�ѹ��¼������ to do
    {
        #ifdef __ANHUI_485_PATCH_96_CURVE__
        if(read_params->patch_num >= 95) //����96�����ߣ���������95��
        #else
         if(read_params->patch_num >= 23) //���㶳�����ݣ�������24��
        #endif
         {
         clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
         read_params->patch_num = 0 ;
         }
         else
         {
         read_params->patch_num ++ ;
         }
         return 0 ;
    }
    #endif*/
    if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_AH_485_PATCH_96_LOAD_RECORD))
    {
        if((bin2_int32u(read_params->phy) == FHJL1_DY_DL_PL) || (bin2_int32u(read_params->phy) == FHJL2_YG_WG_GL)|| (bin2_int32u(read_params->phy) == FHJL3_GL_YS)
        || (bin2_int32u(read_params->phy) == FHJL4_ZFXYG) )
        {
            if(read_params->patch_num >= 95) //����96�����ߣ���������95��
            {
                clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                read_params->patch_num = 0 ;
            }
            else
            {
                read_params->patch_num ++ ;
            }
            return 0 ;
        }
    }
    if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD)
      || check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_485_PATCH_LOAD_RECORD) )
    {
        if( TRUE == check_valid_load_record_phy(bin2_int32u(read_params->phy)) )
        {
            if(read_params->patch_num >= (read_params->patch_load_curve_cnt-1))
            {
                clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                read_params->patch_num = 0 ;
            }
            else
            {
                read_params->patch_num += read_params->block_cnt;
            }
            return 0 ;
        }
    }    
    #ifdef __COUNTRY_ISRAEL__
    if ((bin2_int32u(read_params->phy) == ISRAEL_LOAD_PROFILE))  //��ѹ�����ݳ�����һ�յ�ѹ��¼������ to do
    {
         if(read_params->patch_num >= 95) //���㶳�����ݣ�������96��
         {
             clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
             read_params->patch_num = 0 ;
         }
         else
         {
             read_params->patch_num += (read_params->multi_num & 0x0F) ; //���ֵ�ڽ��յ�ʱ����²�׼ȷ
         }
         return 0 ;
    }
    else if((bin2_int32u(read_params->phy) == ISRAEL_HOUR_DATA))
    {
         if(read_params->patch_num >= 95) //���㶳�����ݣ�������96��
         {
             clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
             read_params->patch_num = 0 ;
         }
        return 0 ;
    }
    #endif   
    clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);

    return 0;
}

//����frame��frame_lenΪNULLʱ��ֻ�Ǽ���Ƿ��г������ݣ������ɱ���
BOOLEAN prepare_read_item_last_curve_cycle_day(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT16U meter_idx,mask_idx;
	#ifdef __PROVICE_YUNNAN__
    INT16U meter_num;
	#endif
    INT8U idx,idx_sub;
	#ifdef __PLC_GUANGXI_VIP_CURVE__
    INT8U passedhour;
	#endif
	INT8U load_time[10] = {0};    
    INT8U load_end_time[6] = {0};    
    INT8U td_bin[5] = {0};
    INT8U patch_param[4] = {0};
    //INT32U tick;
    

    mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if (get_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;
    if (check_is_all_ch(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,0x00))
    {
        clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);

        return FALSE;
    }
#ifndef __PROVICE_SHAANXI_CHECK__
    if(GB645_2007 != read_params->meter_doc.protocol)// 07�� �Ų������ɼ�¼
    {
        clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);

        return FALSE;
    }
   #endif 
    if (check_is_all_ch(read_params->day_hold_td,3,0x00))
    {
        get_yesterday(read_params->day_hold_td);
        get_former_month(read_params->month_hold_td);
    }

    for(idx=0;idx<sizeof(LAST_CURVE_CYCLE_DAY_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((LAST_CURVE_CYCLE_DAY_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            if(get_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,mask_idx))
            {
                get_phy_data((READ_WRITE_DATA*)&(LAST_CURVE_CYCLE_DAY_PHY_LIST[idx]),idx_sub,&phy);
                if(readdata_last_curve_cycle_day(meter_idx,&phy,read_params->day_hold_td,frame,frame+5,frame_len,NULL) == FALSE)
                {
                    //��Ҫ����
                    //����Լ�⺯��
                    if(get_data_item(phy.phy,read_params->meter_doc.protocol,&library))
                    {
                        if(library.item != 0xFFFFFFFF)
                        {

                           #ifdef __PLC_REC_VOLTMETER1__ //
                           if(phy.phy == DY_HOUR_HOLD )
                           {
                           library.item += read_params->patch_num;
//                           read_params->patch_num ++ ;
                           }
                           #endif
                           #ifdef __PLC_GUANGXI_VIP_CURVE__
                           if((phy.phy == SYC_ZDDJ_YG) || (phy.phy == SYC_ZDDJ_WG))
                           {
//                             read_params->item_add_num = check_plc_router_save_last_curve_data(meter_idx,&phy,read_params->item_add_num,read_params->patch_num);
                             check_plc_router_save_last_curve_data(meter_idx,&phy,read_params);
                             if(read_params->patch_num > 24)
                             {
                              clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                              read_params->patch_num = 0 ;
                               return false;
                             }
                             passedhour = (datetime[HOUR] - read_params->last_item_get_time); //����ʱ��Сʱֵ����ȥ��һ�ε�ʱ��
                           library.item += (read_params->item_add_num + passedhour);
                           }
                           if((phy.phy == SYC_GXGY_YG))
                           {
                             read_params->patch_num = check_plc_router_save_last_gx_load_data(meter_idx,&phy,read_params->patch_num);
                             if(read_params->patch_num > 5)
                             {
                              clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_LAST_DAY_HOLD_CURVE_GXFH,idx);
                              read_params->patch_num = 0 ;
                               return false;
                             }
                              library.item += read_params->patch_num;
                           }
                           #endif

                            int32u2_bin(phy.phy,read_params->phy);
                            int32u2_bin(library.item,read_params->item);
                            read_params->resp_byte_num = 40;
                            read_params->read_type = READ_TYPE_LAST_CURVE_CYCLE_DAY;

                            if (read_params->meter_doc.protocol == GB645_2007)
                            {
                                if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_AH_485_PATCH_96_LOAD_RECORD))
                                {
                                    if((phy.phy == FHJL1_DY_DL_PL) ||(phy.phy == FHJL2_YG_WG_GL) || (phy.phy == FHJL3_GL_YS) 
                                    || (phy.phy == FHJL4_ZFXYG) )
                                    {                                                                           
                                        //����ʹ��patch_num��������������������ֵΪ96������ʱ���ǴӲ�����ʼʱ����ǰ��96���㣬����patch_num������Ҫ������ʱ��㣬��¼һ�²�����ʼʱ�̡�
                                        read_params->patch_num = check_save_last_load_data_ah(meter_idx,&phy,read_params->patch_num,read_params->patch_start_time);
                                        
                                        if(read_params->patch_num > 95)
                                        {
                                            clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                                            read_params->patch_num = 0 ;
                                            // return false;
                                            goto NO_FOUND;
                                        }
                                        
                                        /*s_time.year = read_params->patch_start_time[4]+2000;
                                        s_time.month = read_params->patch_start_time[3];
                                        s_time.day = read_params->patch_start_time[2];
                                        s_time.hour = read_params->patch_start_time[1];
                                        s_time.minute = read_params->patch_start_time[0];
                                        s_time.minute -= (read_params->patch_start_time[0]%15);
                                        commandDateMinusHour(&s_time,(read_params->patch_num/4));
                                        commandDateMinusMinute(&s_time,(read_params->patch_num%4)*15); */
                                        
                                        calculat_curve_time(read_params->patch_start_time,td_bin,read_params->patch_num,15);
                                        
                                        //assign_td_value(&s_time,load_time+1,5);
                                        load_time[0] = 1;
                                        load_time[1] = byte2BCD(td_bin[0]);
                                        load_time[2] = byte2BCD(td_bin[1]);
                                        load_time[3] = byte2BCD(td_bin[2]);
                                        load_time[4] = byte2BCD(td_bin[3]);
                                        load_time[5] = byte2BCD(td_bin[4]);
                                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,6);
                                    }
                                    else
                                    {
                                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                    }
                                }
                                else if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_LOAD_RECORD)
                                     || check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_485_PATCH_LOAD_RECORD) )
                                {
                                    /**/
                                    if(TRUE == check_valid_load_record_phy(phy.phy))
                                    {
                                        //����ʹ��patch_num��������������������ֵΪ96������ʱ���ǴӲ�����ʼʱ����ǰ��96���㣬����patch_num������Ҫ������ʱ��㣬��¼һ�²�����ʼʱ�̡�
                                        read_params->patch_num = check_save_last_load_data_LiaoNing(read_params,&phy,read_params->patch_num,&read_params->block_cnt,read_params->patch_start_time);
                                        #ifdef __SOFT_SIMULATOR__
                                        snprintf(info,100,"*** patch_num = %02d ",read_params->patch_num);
                                        debug_println_ext(info);
                                        #endif
                                        if(read_params->patch_num >= read_params->patch_load_curve_cnt)
                                        {
                                            clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,mask_idx);
                                            read_params->patch_num = 0 ;
                                            goto NO_FOUND;
                                        }                                    
                                        //#if ( (defined __PROVICE_SHANGHAI__) || (defined __PROVICE_SHANGHAI_FK__) )
                                        if( (COMMPORT_PLC == read_params->meter_doc.baud_port.port) || (COMMPORT_PLC_REC == read_params->meter_doc.baud_port.port) )
                                        {
                                            if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PLC_PATCH_24_DOT) )
                                            {
                                                calculat_curve_time(read_params->patch_start_time,td_bin,read_params->patch_num,60);
                                            }
                                            else
                                            {
                                                calculat_curve_time(read_params->patch_start_time,td_bin,read_params->patch_num,15);
                                            }
                                        }
                                        else
                                        {
                                            calculat_curve_time(read_params->patch_start_time,td_bin,read_params->patch_num,15);
                                        }                     
                                        //#if ( (defined __PROVICE_SHANGHAI__) || (defined __PROVICE_SHANGHAI_FK__) )
                                        /* �������� ��Ҫ�೭��  */
                                        if(check_const_ertu_plms_switch(CONST_ERTU_PLMS_SWITCH_PATCH_TODAY_LOAD_RECORD))
                                        {
                                            if(read_params->patch_load_curve_cnt - read_params->patch_num < read_params->block_cnt)
                                            {
                                                load_time[0] = byte2BCD(read_params->patch_load_curve_cnt - read_params->patch_num);
                                            }
                                            else
                                            {
                                                load_time[0] = byte2BCD(read_params->block_cnt);
                                            }
                                        }
                                        else
                                        {
                                            load_time[0] = byte2BCD(read_params->block_cnt);
                                        }
                                        load_time[1] = byte2BCD(td_bin[0]);
                                        load_time[2] = byte2BCD(td_bin[1]);
                                        load_time[3] = byte2BCD(td_bin[2]);
                                        load_time[4] = byte2BCD(td_bin[3]);
                                        load_time[5] = byte2BCD(td_bin[4]);
                                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,6);
                                    }
                                    else
                                    {
                                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                    }
                                }
                                else
                                {
                                    #ifdef __PROVICE_YUNNAN__
                                    meter_num = get_readport_meter_count_from_fast_index(COMMPORT_PLC);
                                    if(meter_num <16)
                                    {
                                    library.item = yunnan_check_dayhold_data(library.item);
                                      *frame_len = make_gb645_yunnan_mode3_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                    }
                                    else
                                    {
                                        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                    }
                                    #elif defined __LOAD_RECORD_BLOCK__                                
                                    if((phy.phy == FHJL1_DY_DL_PL) ||(phy.phy == FHJL2_YG_WG_GL) || (phy.phy == FHJL3_GL_YS) )
                                    {                                   
                                       read_params->patch_num = check_plc_router_save_last_load_data(meter_idx,&phy,read_params->patch_num);
                                       if(read_params->patch_num > 23)
                                       {
                                        clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                                        read_params->patch_num = 0 ;
                                        read_params->block_cnt = 0;
                                        // return false;
                                        goto NO_FOUND;
                                       }
        
                                       load_time[0] = 1;
                                       load_time[1] = 0;
                                       //load_time[2] = read_params->patch_num;
                                       load_time[2] = byte2BCD(read_params->patch_num);
                                       load_time[3] = byte2BCD(datetime[DAY]);
                                       load_time[4] = byte2BCD(datetime[MONTH]);
                                       load_time[5] = byte2BCD(datetime[YEAR]);
                                       previous_dayhold_td(load_time+3);
                                     
                                      *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,6);
                                   }
                                   else
                                   *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                   #elif defined __SH_LOAD_RECORD_BLOCK__
                                  if((phy.phy == FHJL1_DY_DL_PL) ||(phy.phy == FHJL2_YG_WG_GL) || (phy.phy == FHJL4_ZFXYG) )
                                  {
                                    //Ҫ��������ĸ��ɼ�¼��
                                   read_params->patch_num = check_plc_router_save_last_load_data(meter_idx,&phy,read_params->patch_num);
                                   if(read_params->patch_num > 23)
                                   {
                                    clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                                    read_params->patch_num = 0 ;
                                    // return false;
                                    goto NO_FOUND;
                                   }
    
                                  load_time[0] = 1;
                                  load_time[1] = 0;
                                  load_time[2] = byte2BCD(read_params->patch_num);
                                  load_time[3] = byte2BCD(datetime[DAY]);
                                  load_time[4] = byte2BCD(datetime[MONTH]);
                                  load_time[5] = byte2BCD(datetime[YEAR]);
    
                                  *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,6);
                                  }
                                  else
                                  *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                 #elif defined __COUNTRY_ISRAEL__
                                 if(phy.phy == ISRAEL_HOUR_DATA)
                                 {
                                     read_params->patch_num = check_plc_router_save_last_israel_hour_load_data(meter_idx,&phy,read_params->patch_load_curve_cnt);
                                     if(read_params->patch_num > 23)
                                    {
                                    clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                                    read_params->patch_num = 0 ;

                                    goto NO_FOUND;
                                    }

                                     load_time[0] = 0;
                                     load_time[1] = (read_params->patch_num);

                                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,2);

                                 }
                                 else if(phy.phy == ISRAEL_LOAD_PROFILE)
                                 {
                                   INT8U patch_num_temp,patch_num_temp2;
                                   
                                   read_params->patch_num = check_save_last_israel_load_profile_data(meter_idx,&phy,read_params->patch_num,read_params->patch_start_time);
                                   if(read_params->patch_num > 95) //д�ɺ궨���һЩ��
                                   {
                                    clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,idx);
                                    read_params->patch_num = 0 ;

                                    goto NO_FOUND;
                                   }

                                   library.item = 0x06000011;
                                   int32u2_bin(library.item,read_params->item);
                                  // calculat_curve_time(read_params->patch_start_time,td_bin,read_params->patch_num,15);

                                    load_time[0] = 1;
                                    if(read_params->meter_doc.meter_class.meter_class == 1)
                                    {

                                        patch_num_temp = read_params->patch_num;
                                        if(patch_num_temp <= 94)  /*����һ�£��������1�����ˣ���Ҫ������*/
                                        {
                                           patch_num_temp ++;
                                           patch_num_temp2 = check_save_last_israel_load_profile_data(meter_idx,&phy,patch_num_temp,read_params->patch_start_time);
                                           if(patch_num_temp2 - read_params->patch_num == 1) /*����һ�£��ǲ��������������ĵ㣬û�еĻ�����Ҫ��2��*/
                                           {
                                              load_time[0] = 2;
                                           }
                                        }
                                    }

                                    if(load_time[0] == 2)
                                    {
                                        read_params->resp_byte_num = 130;

                                        if(read_params->multi_num & 0xA0) //�״ν����ʱ�򣬼���ĳ���num�������µ�һ���㣬����2������ʱ��Ҫ�������һ���㡣��������������ǵ��ż����һ���㣬©���������ô���
                                        {
                                            read_params->patch_num ++;
                                            read_params->multi_num &= 0x0F;
                                        }
                                        calculat_curve_time(read_params->patch_start_time,td_bin,read_params->patch_num,15);
                                    }
                                    else
                                    {
                                        read_params->resp_byte_num = 110;

                                        calculat_curve_time(read_params->patch_start_time,td_bin,read_params->patch_num,15);
                                    }

                                    load_time[1] = byte2BCD(td_bin[0]);
                                    load_time[2] = byte2BCD(td_bin[1]);
                                    load_time[3] = byte2BCD(td_bin[2]);
                                    load_time[4] = byte2BCD(td_bin[3]);
                                    load_time[5] = byte2BCD(td_bin[4]);
                                    set_israel_di_item(load_time+6,phy.phy);
                                  *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_time,10);
                                 }
                                 else
                                 *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                  #else
                                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                                  #endif
                              }
                            }
                            else if (read_params->meter_doc.protocol == GB645_1997)
                            {
                                *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            else if (read_params->meter_doc.protocol == GUANGXI_V30)
                            {
                                *frame_len = make_gb645_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                            }
                            else if(read_params->meter_doc.protocol == GB_OOP)
                            {
                                read_params->patch_num = get_oop_last_load_data_patch_num(read_params,&phy,read_params->patch_num,read_params->patch_start_time);
                                        #ifdef __SOFT_SIMULATOR__
                                        snprintf(info,100,"*** patch_num = %02d ",read_params->patch_num);
                                        debug_println_ext(info);
                                        #endif
                                        if(read_params->patch_num >= 96)
                                        {
                                            clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,mask_idx);
                                            read_params->patch_num = 0 ;
                                            goto NO_FOUND;
                                        }                                    



                                        calculat_curve_time(read_params->patch_start_time,td_bin,read_params->patch_num,15);



                                             load_time[0] = 0;//�������룬��ʱ�ò���
                                             load_end_time[0] = 0;
                                             mem_cpy(load_time+1,td_bin,5);
                                             mem_cpy(read_params->patch_load_time,td_bin,5);
                                             calculat_curve_time(td_bin,td_bin,1,15);
                                             mem_cpy(load_end_time+1,td_bin,5);


                                             *frame_len = make_oop_meter_load_frame(frame,read_params->meter_doc.meter_no,(INT8U *)&library.item,1,load_time,load_end_time,1,15);
                                             if(*frame_len >0)
                                             {
                                                 return TRUE;
                                             }
                                             else
                                             {
                                                  goto NO_FOUND;
                                             }


                            }
                            else
                            {
                                clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
                                return FALSE;
                            }
                            #ifdef __SOFT_SIMULATOR__
                            snprintf(info,100,"*** prepare item day hold : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
                                    meter_idx,library.item,phy.phy,mask_idx);
                            debug_println_ext(info);
                            #endif

                            return TRUE;
                        }
                        else
                        {
                            //����msak
                            clear_read_mask_from_meter_param(meter_idx,READ_TYPE_LAST_CURVE_CYCLE_DAY,mask_idx);
                        }
                    }
                    else
                    {
                        //����msak
                        clear_read_mask_from_meter_param(meter_idx,READ_TYPE_LAST_CURVE_CYCLE_DAY,mask_idx);
                    }
                }
                clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,mask_idx);
            }            
NO_FOUND:
            mask_idx++;
        }
    }

    fread_array(meter_idx,PIM_PATCH_LOAD_DATE_AND_TIME,patch_param,4);
    if(compare_string(patch_param,datetime+DAY,3) == FALSE)
    {
         patch_param[3] ++;
         if(patch_param[3] >= DEFAULT_CURVE_HOLD_PATCH_DAY_COUNT)
         {
              clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,meter_idx);
              read_params->patch_load_tmp_loop = 0;
               fread_array(meter_idx,PIM_PATCH_LOAD_DATE_STATUS,patch_param,4);//����patch_param
               if(compare_string(patch_param,datetime+DAY,3) != FALSE)
               {
                  mem_cpy(patch_param,datetime+DAY,3);
                  patch_param[3] = 0xAA;
                  fwrite_array(meter_idx,PIM_PATCH_LOAD_DATE_STATUS,patch_param,4); //��ʶ������ռ�������߲��������
               }

              return FALSE;
         }

         fwrite_array(meter_idx,PIM_PATCH_LOAD_DATE_AND_TIME,patch_param,4);
         read_params->patch_load_tmp_loop = 1;
    }
    else
    {
        clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,meter_idx);

        fread_array(meter_idx,PIM_PATCH_LOAD_DATE_STATUS,patch_param,4);//����patch_param
        if(compare_string(patch_param,datetime+DAY,3) != FALSE)
        {
            mem_cpy(patch_param,datetime+DAY,3);
            patch_param[3] = 0xAA;
            fwrite_array(meter_idx,PIM_PATCH_LOAD_DATE_STATUS,patch_param,4); //��ʶ������ռ�������߲��������
        }
    }

    return FALSE;
}

#ifdef __BATCH_TRANSPARENT_METER_TASK__
BOOLEAN check_is_GB645(INT8U protocol)
{
    switch(protocol)
    {
    case GB645_1997:
    case GB645_CY:
    case GB645_1997_JINANGSU_4FL:
    case GB645_1997_JINANGSU_2FL:
    case GB645_2007:
    case GB645_YUNNAN:
    case GB645_1997_STAR_40:
        return TRUE;
    }
    return FALSE;
}

BOOLEAN prepare_batch_transparent_meter_task(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT32U ctrl_time,item;
    CommandDate cmddate,cmddate_cur;
    INT16U spot_idx,meter_idx;
    BATCH_TRANSPARENT_METER_TASK_HEADER header;
    INT8U data[6];
    INT8U idx; // frame_pos,
    INT8U gua_meter = 0;
    BOOLEAN create_erc;
    INT8U FE_count=0,idx_FE;
    INT8U tmp_frame_len;

    //frame_pos = 0;
    create_erc = FALSE;

    read_params->batch_ctrl.is_no_resp = 0;
    read_params->batch_ctrl.is_set_node_fail = 0;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    tpos_mutexPend(&SIGNAL_BATCH_SET);
    fread_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
    if (header.count > BATCH_TRANSPARENT_METER_TASK_FRAME_MAX_COUNT) 
    {
        tpos_mutexFree(&SIGNAL_BATCH_SET);
        return FALSE;
    }
    if (header.task_level >= 2)    //�����ն���
    {
        if (get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx))
        //if (meter_read_flag_check(record_flag,READFLAG_DAYHOLD) || meter_read_flag_check(record_flag,READFLAG_DAYHOLD_FX))
        {
            tpos_mutexFree(&SIGNAL_BATCH_SET);
            return FALSE;
        }
    }

//    if (GPLC.batch_meter_ctrl.is_set_router_fail)
//    {
//        GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_set_node_fail = 1;
//    }
//    if (header.ctrl_flag.task_end)
//    {
//        read_params->batch_ctrl.is_set_node_fail = 0;
//    }

    read_params->batch_ctrl.is_set_node_fail = header.ctrl_flag.task_end ? 0 : 1;
    read_params->control.loop = header.ctrl_flag.task_end ? 0 : 1;

    if (header.task_level == 0)  //�ѿ�����     //__BATCH_TRANSPARENT_METER_TASK_COST_CONTROL__
    {
        if (get_bit_value(COST_CONTROL_FAIL_FLAG,256,meter_idx))
        {
            read_params->control.loop = 1;
            clr_bit_value(COST_CONTROL_FAIL_FLAG,256,meter_idx);
        }
    }

    if (header.flag == 0xAA)
    {
        setCommandDate(&cmddate_cur,(INT8U*)(datetime+1));
        setCommandBCDDate(&cmddate,header.begin_time);
        #ifdef __BATCH_TRANSPARENT_METER_TASK_TMP1__
        if (((bin2_int16u(header.vaild_minute) > 0) && (commandDateCompare(&cmddate_cur,&cmddate) < 0)) || (bin2_int16u(header.vaild_minute) == 0))
        #else
        if (((header.vaild_minute > 0) && (commandDateCompare(&cmddate_cur,&cmddate) < 0)) || (header.vaild_minute == 0))
        #endif
        {
            for(idx=0;idx<header.count;idx++)
            {
                fread_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK),data,3);
                if ((data[0] != BATCH_TRANS_METER_TASK_FLAG_NO_EXEC) && (data[0] != BATCH_TRANS_METER_TASK_FLAG_EXEC_NO_RESP)) continue;
                if (data[2] > BATCH_TRANSPARENT_METER_TASK_FRAME_LEN)
                {
                    data[0] = BATCH_TRANS_METER_TASK_FLAG_CANT_EXEC;
                    fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK),data,1);
                    continue;
                }

                if (data[2] == 0) 
                {
                    data[0] = BATCH_TRANS_METER_TASK_FLAG_CANT_EXEC;
                    fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK),data,1);
                    continue;
                }

                if (data[0] != BATCH_TRANS_METER_TASK_FLAG_EXEC_NO_RESP)
                {
                    data[0] = BATCH_TRANS_METER_TASK_FLAG_EXEC_NO_RESP;
                    fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK),data,3);
                }

                //frame_pos = 0;
        
                item = 0xAA000000+(header.task_id[0]<<16)+(header.task_id[1]<<8);
                item += idx;
                int32u2_bin(item,read_params->item);
                read_params->resp_byte_num = 160;
                read_params->read_type = READ_TYPE_BATCH_TRANSPARENT_METER_TASK;

                *frame_len = data[2];
                tmp_frame_len = data[2]; /*���ڼ��fe����*/

                fread_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK)+3,frame,data[2]);

                if(check_is_GB645(read_params->meter_doc.protocol))
                {
                    for(idx_FE=0;idx_FE<tmp_frame_len;idx_FE++)
                    {
                        if(frame[idx_FE] == 0x68) /*�еĲ�һ����FEǰ����һֱ�鵽68��ֹ*/
                        {
                            break;
                        }
                        FE_count ++;
                    }

                    if ((compare_string(frame+1+FE_count,read_params->meter_doc.meter_no,6) != 0) && (compare_string(frame+1+FE_count,read_params->meter_doc.rtu_no,6) != 0))
                    {
                        data[0] = BATCH_TRANS_METER_TASK_FLAG_CANT_EXEC;
                        fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK),data,1);
                        continue;
                    }
                }

                tpos_mutexFree(&SIGNAL_BATCH_SET);
                return TRUE;
            }

            header.flag = 0x55;
            if (header.ctrl_flag.wait_read)
            {
                fread_array(FILEID_RUN_DATA,FMADDR_WAIT_READ_MINUTE,&gua_meter,1);
                if (gua_meter == 0x55) // 8��
                {
                    mem_set(header.begin_time,6,0);
                    mem_set(header.begin_time,1,0xFF);
                }
                else
                {
                    //mem_set(header.begin_time,6,0);
                    //mem_set(header.begin_time,4,0xFF);         //32��

                    //����ʱ��ȴ�
                    setCommandDate(&cmddate_cur,(INT8U*)(datetime+1));
                    commandDateAddMinute(&cmddate_cur,3);   //�ȴ�����
                    assign_td_value(&cmddate_cur,header.begin_time,5);
                }
            }
        }
        else
        {
            header.flag = 0x00;

            create_erc = FALSE;
            for(idx=0;idx<header.count;idx++)
            {
                fread_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK),data,3);
                if ((data[0] == BATCH_TRANS_METER_TASK_FLAG_NO_EXEC) || (data[0] == BATCH_TRANS_METER_TASK_FLAG_EXEC_NO_RESP))
                {
                    data[0] = BATCH_TRANS_METER_TASK_FLAG_TIME_OUT;
                    fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK),data,3);

                    if ((!create_erc) && (header.ctrl_flag.report))
                    {
                        data[5] = data[1];
                        data[1] = 0;
                        event_erc_64(read_params->meter_doc.spot_idx,header.task_id,data[5],data,NULL,0);
                        create_erc = TRUE;
                    }
                }
            }
        }

        set_DATAFMT_01((DATAFMT_01 *)header.end_time);
        check_A1_week(header.end_time);
        fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));

        spot_idx = bin2_int16u(read_params->meter_doc.spot_idx);
        if ((spot_idx > 0) && (spot_idx <= MAX_METER_COUNT))
        {
            fread_array(FILEID_RUN_DATA,FMDATA_F305+((spot_idx-1)/8),&idx,1);
            idx |= (0x01<<((spot_idx-1)%8));
            fwrite_array(FILEID_RUN_DATA,FMDATA_F305+((spot_idx-1)/8),&idx,1);
        }
    }

    if ((header.flag == 0x55) && (header.ctrl_flag.wait_read))
    {
        fread_array(FILEID_RUN_DATA,FMADDR_WAIT_READ_MINUTE,&gua_meter,1);
        if (gua_meter == 0x55) // 8��
        {
            ctrl_time = bin2_int32u(header.begin_time);
            if(ctrl_time > 0)
            {
                ctrl_time = (ctrl_time>>1);
                header.begin_time[0] = ctrl_time;
                header.begin_time[1] = ctrl_time>>8;
                header.begin_time[2] = ctrl_time>>16;
                header.begin_time[3] = ctrl_time>>24;
        
                fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
                read_params->batch_ctrl.is_no_resp = 1;

                tpos_mutexFree(&SIGNAL_BATCH_SET);
                return TRUE;
            }
            else
            {
                header.flag = 0x00;
                fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
            }
        }
        else
        {
            setCommandDate(&cmddate_cur,(INT8U*)(datetime+1));
            setCommandBCDDate(&cmddate,header.begin_time);
            if (commandDateCompare(&cmddate_cur,&cmddate) > 0)
            {
                header.flag = 0x00;
                fwrite_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
            }
            else
            {
                read_params->batch_ctrl.is_no_resp = 1;
                tpos_mutexFree(&SIGNAL_BATCH_SET);
                return TRUE;
            }
        }
    }

    tpos_mutexFree(&SIGNAL_BATCH_SET);
    return FALSE;
}

void save_batch_transparent_meter_task(INT16U meter_idx,READPORT_METER_DOCUMENT *meter_doc,INT32U item,INT8U *frame,INT16U frame_len)
{
    INT32U offset;
    INT8U data[10];
    INT8U idx,framelen;
    BATCH_TRANSPARENT_METER_TASK_HEADER header;
    BOOLEAN create_erc;

    fread_array(meter_idx,PIM_BATCH_TRANSPARENT_METER_TASK,header.value,sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER));
    data[4] = item>>16;
    data[5] = item>>8;

    if(compare_string(header.task_id,data+4,2) != 0) return;

    idx = item & 0xFF;

    if(idx >= header.count) return;
    if(header.flag != 0xAA) return;

    framelen = frame[POS_GB645_DLEN]+12;

    data[0] = BATCH_TRANS_METER_TASK_FLAG_FINISH;
    if(framelen > BATCH_TRANSPARENT_METER_TASK_FRAME_LEN)
    {
        framelen = BATCH_TRANSPARENT_METER_TASK_FRAME_LEN;
        data[0] = BATCH_TRANS_METER_TASK_FLAG_RESP_LONG;
    }
    data[1] = framelen+6;
    data[2] = byte2BCD(datetime[SECOND]);
    data[3] = byte2BCD(datetime[MINUTE]);
    data[4] = byte2BCD(datetime[HOUR]);
    data[5] = byte2BCD(datetime[DAY]);
    data[6] = byte2BCD(datetime[MONTH]);
    data[7] = byte2BCD(datetime[YEAR]);
    check_A1_week(data+2);

    offset = PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK);
    fwrite_array(meter_idx,offset+3+BATCH_TRANSPARENT_METER_TASK_FRAME_LEN+1+6,frame,framelen);
    fwrite_array(meter_idx,offset+3+BATCH_TRANSPARENT_METER_TASK_FRAME_LEN,data+1,7);
    fwrite_array(meter_idx,offset,data,1);

    if(header.ctrl_flag.report)
    {
        fread_array(meter_idx,offset,data+8,2);
        data[1] = framelen;
        event_erc_64(meter_doc->spot_idx,header.task_id,data[9],data,frame,framelen);
    }
    
    if (check_is_GB645(meter_doc->protocol) && check_frame_body_gb645(frame,frame_len) && (header.ctrl_flag.deny))
    {
        if (frame[POS_GB645_CTRL] & 0x40)
        {
            create_erc = FALSE;
            idx++;
            for(;idx<header.count;idx++)
            {
                offset = PIM_BATCH_TRANSPARENT_METER_TASK+sizeof(BATCH_TRANSPARENT_METER_TASK_HEADER)+idx*sizeof(BATCH_TRANSPARENT_METER_TASK);
                fread_array(meter_idx,offset,data,3);
                data[0] = BATCH_TRANS_METER_TASK_FLAG_BEFORE_EXCEPTION;
                fwrite_array(meter_idx,offset,data,1);

                if ((!create_erc) && (header.ctrl_flag.report))
                {
                    data[5] = data[1];
                    data[1] = 0;
                    event_erc_64(meter_doc->spot_idx,header.task_id,data[5],data,NULL,0);
                    create_erc = TRUE;
                }
            }
        }
    }
}

void set_gua_meter_minute(GB3762_VENDOR *vendor_info)
{
    INT32U h1;
    INT8U num;
	INT8U tmp = 0;

    num = 0xAA;
    if ((vendor_info->name[0]=='C')  && (vendor_info->name[1]=='T'))
    {
        h1 = vendor_info->bcd_year*0x10000+vendor_info->bcd_month*0x100+vendor_info->bcd_day;
        if(h1 < 0x00150403)
        {
            num = 0x55;    //8��
        }
    }
    fread_array(FILEID_RUN_DATA,FMADDR_WAIT_READ_MINUTE,&tmp,1);
    if (tmp != num)
    {
        fwrite_array(FILEID_RUN_DATA,FMADDR_WAIT_READ_MINUTE,&num,1);
    }
}
#endif

#ifdef __METER_DAY_FREEZE_EVENT__
#define CLEAR_READ_MASK                    0// �������
#define KEEP_ON_RECORDING                  1
#define SAVE_LAST_FREEZE_EVENT_RECORD      2//��ȡ��һ��Ķ����¼������洢������
DAY_FREEZE_EVENT_DATA const CYCLE_DAY_FREEZE_ITEM_LIST[] =
{
    //���ݱ�ʶ          ƫ��    ����ǿ�ʼ95666                    block_len                datalen record_offset,record_len
    {0x03300D00,   PIM_DAY_FREEZE_EVENT_OPEN_COVER,                3+5+129+RESERVE_DATA,     3+5+4, 14,             12},    //SAVE_FLAG_DENY_NO_SAVE|SAVE_FLAG_BLOCK|5,     0,    4,
    {0x03300E00,   PIM_DAY_FREEZE_EVENT_OPEN_BUTTON_COVER,         3+5+129+RESERVE_DATA,     3+5+4, 14,             12},
    {0x1D000001,   PIM_DAY_FREEZE_EVENT_METER_TRIP,                3+5+109+RESERVE_DATA,     3+5+4, 14,             10},
    {0x1E000001,   PIM_DAY_FREEZE_EVENT_METER_SWITCH,              3+5+109+RESERVE_DATA,     3+5+4, 14,             10},
    {0x03300100,   PIM_DAY_FREEZE_EVENT_METER_CLEAR,               3+5+669+RESERVE_DATA,     3+5+4, 14,             66},
}; 

INT32U get_cycle_day_event_save_offset(DAY_FREEZE_EVENT_DATA freeze_data,INT8U td[3])
{
    INT32U offset;
    offset = getPassedDays(2000+td[2],td[1],td[0]);
    offset = offset % SAVE_POINT_NUMBER_DAY_FREEZE_EVENT;
    offset *= freeze_data.block_len;
    offset += freeze_data.offset;
    return offset;
}
#endif
#ifdef __VOLTAGE_MONITOR__
void plc_router_save_valtage_data_standard(READ_PARAMS *read_params,INT8U *data,INT8U datalen)
{
    READ_WRITE_DATA phy;
    INT32U rec_v,max_v,min_v,list_v,eeAddr;
    INT32U cha,max_cha;//flAddr;
    INT16U meter_idx,idx_vip,vip_count;
    INT8U is_valid,idx,tmp_idx,itemcount,idx_list,idx_sub;
    INT8U buffer[256];
    INT8U mask_idx,midu;
  //  INT16U       pos;
  //  C_VIP_ZXYGZ  vip_zxygz;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    get_v_limit_value_standard(&max_v,&min_v);
    tmp_idx = 0xFF;
    max_cha = 0;

    rec_v = bcd2u32(data,2,&is_valid);
    if(bin2_int32u(read_params->item) == 0xB611) rec_v *= 10;

    if (is_valid)
    {
        idx = find_in_v_list(meter_idx);
        if(idx < MAX_V_MONITOR_COUNT*2)
        {
           // list_v = bcd2u32((INT8U*)(V_MONITOR[idx].voltage),2,&is_valid);
            if ((rec_v <= max_v) && (rec_v >= min_v))
            {

                V_MONITOR[idx].time[0] = byte2BCD(datetime[MINUTE]);
                V_MONITOR[idx].time[1] = byte2BCD(datetime[HOUR]);
                mem_cpy((INT8U*)(V_MONITOR[idx].voltage),data,2);

                //����
                vip_count = 0;
                fread_ertu_params(EEADDR_SET_F35,buffer,256);
                for(idx_vip=1;idx_vip<=MAX_METER_COUNT;idx_vip++)
                {
                    if (get_bit_value(buffer,256,idx_vip) == FALSE) vip_count++;
                }
                if(vip_count < 20)
                {
                    clr_bit_value(buffer,256,meter_idx);
                    fwrite_ertu_params(EEADDR_SET_F35,buffer,256);

                    mask_idx = 0;
                    for(idx_list=0;idx_list<sizeof(CRUVE_PHY_LIST)/sizeof(READ_WRITE_DATA);idx_list++)
                    {
                        for(idx_sub=0;idx_sub<((CRUVE_PHY_LIST[idx_list].flag&0x0F)+1);idx_sub++)
                        {
                            if (READ_MASK_CURVE_V_A == mask_idx)
                            {
                                get_phy_data((READ_WRITE_DATA*)&(CRUVE_PHY_LIST[idx_list]),idx_sub,&phy);

                                //��ȡ�ܶ�
                                fread_array(meter_idx,phy.offset,&midu,1);
                                switch(midu)
                                {
                                case 5:  break;
                                case 15: break;
                                case 30: break;
                                case 60: break;
                                default:
                                    if(read_params->meter_doc.baud_port.port == COMMPORT_PLC)
                                    {
                                        midu = 60;
                                        fwrite_array(meter_idx,phy.offset,&midu,1);
                                    }
                                    else
                                    {
                                        midu = 15;
                                        fwrite_array(meter_idx,phy.offset,&midu,1);
                                    }
                                    break;
                                }

                                if  (bin2_int32u(read_params->item) == 0xB611)
                                {
                                    v_97_to_format07(data);
                                }

                                writedata_curve(read_params,&phy,read_meter_flag_curve.cycle_60_minute,data,datalen,buffer,0xFF);
                                break;
                            }
                            mask_idx++;
                        }
                        if (READ_MASK_CURVE_V_A == mask_idx) break;
                    }
                }

                eeAddr = EEADDR_SET_F66 + 2*1029;
                fread_ertu_params(eeAddr+8,&itemcount,1);
                if (itemcount > 20)
                {
                    buffer[0] = 0x41;    //�ϱ�����

                    buffer[1] = 0x00;    //��׼ʱ��
                    buffer[2] = 0x30;
                    buffer[3] = 0x00;
                    buffer[4] = 0x01;
                    buffer[5] = 0xC1;
                    buffer[6] = 0x00;

                    buffer[7] = 0x01;    //��ȡ����
                    buffer[8] = 0x00;    //���ݵ�Ԫ��ʶ����

                    fwrite_ertu_params(eeAddr,buffer,9);

                    buffer[0] = 0x55;
                    fwrite_ertu_params(EEADDR_SET_F68 + 2,buffer,1);
                }

                if(itemcount < 20)
                {
                    buffer[0] = toDA(meter_idx);
                    buffer[1] = toDA(meter_idx)>>8;

                    buffer[2] = DT_F89 & 0xFF;
                    buffer[3] = DT_F89>>8;

                    fwrite_ertu_params(eeAddr+9+itemcount*4,buffer,4);
                    itemcount++;
                    fwrite_ertu_params(eeAddr+8,&itemcount,1);
                }
            }
            else
            {
                mem_set((INT8U*)(V_MONITOR[idx].value),sizeof(VOLTAGE_MONITOR),0x00);
            }
            V_MONITOR[idx].meter_idx.rec_task_flag = 0;
            return;
        }

        if ((rec_v <= max_v) && (rec_v >= min_v))
        {
            cha = (rec_v > 2200) ? (rec_v - 2200) : (2200 - rec_v);
            max_cha = 0;

            for(idx=0;idx<MAX_V_MONITOR_COUNT;idx++)
            {
                if (V_MONITOR[idx].meter_idx.meter_idx == 0)
                {
                    V_MONITOR[idx].meter_idx.meter_idx = meter_idx;
                    V_MONITOR[idx].time[0] = byte2BCD(datetime[MINUTE]);
                    V_MONITOR[idx].time[1] = byte2BCD(datetime[HOUR]);
                    mem_cpy((INT8U*)(V_MONITOR[idx].voltage),data,2);
                    tmp_idx = 0xFF;
                    break;
                }

                list_v = bcd2u32((INT8U*)(V_MONITOR[idx].voltage),2,&is_valid);
                if(is_valid)
                {
                    if (list_v == 0)
                    {
                        V_MONITOR[idx].meter_idx.meter_idx = meter_idx;
                        V_MONITOR[idx].time[0] = byte2BCD(datetime[MINUTE]);
                        V_MONITOR[idx].time[1] = byte2BCD(datetime[HOUR]);
                        mem_cpy((INT8U*)(V_MONITOR[idx].voltage),data,2);
                        tmp_idx = 0xFF;
                        break;
                    }
                    else
                    {
                        list_v = (list_v > 2200) ? (list_v - 2200) : (2200 - list_v);
                        if (cha < list_v)
                        {
                            if (max_cha < list_v)
                            {
                                tmp_idx = idx;
                                max_cha = list_v;
                            }
                        }
                    }
                }
            }
            if(tmp_idx < MAX_V_MONITOR_COUNT)
            {
                V_MONITOR[tmp_idx].meter_idx.meter_idx = meter_idx;
                V_MONITOR[tmp_idx].time[0] = byte2BCD(datetime[MINUTE]);
                V_MONITOR[tmp_idx].time[1] = byte2BCD(datetime[HOUR]);
                mem_cpy((INT8U*)(V_MONITOR[tmp_idx].voltage),data,2);
            }
        }

    }
    //���������ʶ
    clr_bit_value(read_meter_flag_voltage_moniter,READ_FLAG_BYTE_NUM,meter_idx);
    clr_bit_value(read_meter_flag_voltage_moniter_allow,READ_FLAG_BYTE_NUM,meter_idx);
}
#endif //#ifdef __VOLTAGE_MONITOR__
BOOLEAN prepare_israel_cast_set_meter_task(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT16U meter_idx;
    INT8U data_len;
    INT8U frame_pos,idx;
    INT8U cs,task_flag,tmp;

    frame_pos = 0;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if(GB645_2007 == read_params->meter_doc.protocol)
    {
       fread_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_FRAME,frame,200); //�Ӵ洢�ĵط�ȡ�����·��Ĺ㲥����

       if((frame[POS_GB645_FLAG1]==0x68) && (frame[POS_GB645_FLAG2]==0x68) )
       {
          *frame_len = frame[POS_GB645_DLEN]+12;
        //���㲥�����999999�ĳɵ���
          data_len = frame[POS_GB645_DLEN]+12;
          frame[frame_pos++] = 0x68;
          frame[frame_pos++] = read_params->meter_doc.meter_no[0];
          frame[frame_pos++] = read_params->meter_doc.meter_no[1];
          frame[frame_pos++] = read_params->meter_doc.meter_no[2];
          frame[frame_pos++] = read_params->meter_doc.meter_no[3];
          frame[frame_pos++] = read_params->meter_doc.meter_no[4];
          frame[frame_pos++] = read_params->meter_doc.meter_no[5];
          frame[frame_pos++] = 0x68;
//          mem_cpy(frame+2,meter_doc->meter_no,6);
          cs=0;

          for(idx=3;idx<data_len;idx++)
           cs+=frame[idx];
         frame[data_len+1] = cs;

            //D2D3��¼ִ�в���
               fread_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASK+meter_idx,&task_flag,1); //��¼�Ĵ���

            tmp = task_flag>>2;
            tmp &= 0x03;
            if(tmp >= 3)
            {
                //����Ϊʧ��!
                task_flag &= ~0x03;
                task_flag |= 0x02;
                fwrite_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASK+meter_idx,&task_flag,1);
                return FALSE;
            }
            else
            {
                //�����ۼ�һ��
                tmp++;
                tmp = tmp<<2;
                task_flag = (tmp & 0x0C) + (task_flag & 0x03);
                fwrite_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASK+meter_idx,&task_flag,1);
                mem_set(read_params->item,4,0xFF);
                read_params->item[0] = 0xAA;
               return TRUE;
            }
       }
       else
       {
          portContext_plc.plc_cast_task.value = 0;  //���Ǳ�׼645���Ͳ�ִ�й㲥���������ˣ�
       }
    }
    else
    {
       //����Ϊʧ��!
       task_flag &= ~0x03;
       task_flag |= 0x02;
       fwrite_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASK+meter_idx,&task_flag,1);
    }

    return FALSE;
}

/*+++
 ���ܣ���ȡ�㲥����״̬��Ϣ    AFN=0x0C  FN=DT_F13
 ������
       INT8U *statinfo  ͳ����Ϣ
 ���أ�
       INT8U  ���ݳ���
 ������
      1�����ݵ�ǰ����״̬ȷ���Ƿ���Ҫ����ִ��ͳ�ơ�
      2������485�����Ϊִ��OK��
---*/
INT16U  get_plc_cast_statinfo(INT8U *statinfo)
{
    INT16U meter_idx,count_ok,count_fail,count_left;
    INT8U  taskflag;
    INT8U  flag,buffer[16];

    fread_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASKFLAG,&taskflag,1);

    //�㲥��������ִ�й����У���Ҫִ��ͳ�ƣ����ص�ǰ����״̬
    if(PLCCASTTASK_EXEC == taskflag)
    {
       count_ok = 0;
       count_left = 0;
       count_fail = 0;
       for(meter_idx=0;meter_idx<MAX_METER_COUNT;meter_idx++)
       {
          //ע�⣺ʹ��ģ16���㣬ϣ��meter_idx��0��ʼ������ʵ�ʴ洢��1��ʼ��
          if(meter_idx % 16 == 0) fread_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASK+meter_idx+1,buffer,16);
          flag = buffer[meter_idx % 16] & 0x03;
          if(flag==1) count_ok++;
          else if(flag==2) count_fail++;
          else if(flag==0) count_left++;
       }
        if(count_left == 0)
        {
            taskflag = PLCCASTTASK_OK;
            portContext_plc.plc_cast_task.b.taskflag = PLCCASTTASK_OK;
        }
       fwrite_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASKFLAG,&taskflag,1);
       fwrite_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_STATINFO,(INT8U*)&count_ok,2);
       fwrite_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_STATINFO+2,(INT8U*)&count_fail,sizeof(INT16U));
       fwrite_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_STATINFO+4,(INT8U*)&count_left,sizeof(INT16U));
    }

    if(statinfo != NULL)
    {
    fread_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASKFLAG,statinfo,7);
    }
    return 7;

}

/*+++
 ���ܣ���ȡ�㲥����״̬��Ϣ    AFN=0x0C  FN=DT_F13
 ������
       INT8U *taskinfo,      ��������
       INT8U *param,         �������
       INT16U max_left_len   �����÷����ֽ���
 ���أ�
       INT8U  ���ݳ���
 ������
      1�����������
           ��ʼ�������	BIN	�ֽ�	2
           ���ζ�ȡ�Ĳ���������n(1?n?512)	BIN	�ֽ�	2

      2�����أ�
                 ��ʼ�������	        BIN	�ֽ�	2
       ���ζ�ȡ�Ĳ���������n(1?n?512)	BIN	�ֽ�	2
    ���ζ�ȡ�ĵ�1��������Ĺ㲥������Ϣ	BIN	�ֽ�	1
          ����	����	����	����

---*/
INT16U get_plc_cast_taskinfo(INT8U *taskinfo,INT8U *param,INT16U max_left_len)
{
   INT16U meter_seq;
   INT16U meter_count;
   INT16U pos,ans_count;

   //��ʼ���
   meter_seq = param[0] + param[1]*0x100;
   //������������ <= 512
   meter_count = param[2] + param[3]*0x100;

   mem_cpy(taskinfo,param,2);
   pos = 4;
   ans_count = 0;
   while(meter_count)
   {
      meter_count --;
      if(pos >= max_left_len) break;
      pos++;
//      taskinfo[pos++] = read_fmByte(FMADDR_PLC_CAST_TASK+meter_seq);
      fread_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASK+meter_seq,taskinfo+pos,1);

      ans_count ++;
      meter_seq ++;
   }

   mem_cpy(taskinfo+2,&ans_count,sizeof(INT16U));
   return pos;
}
#if defined(__PRECISE_TIME__)
//����������Уʱ��¼
void save_meter_precise_time_spot(INT16U meter_idx,INT32U before,INT32U after,INT8U flag,INT8U error_flag)
{
    INT32U offset;
    INT8U td[3];//,pos 
    PRECISE_TIME_RECORD time_data;
    PRECISE_TIME_CAST_RECORD time_cast_data;
 //   PRECISE_TIME_CAST_STAT time_cast_stat;

  //  pos = read_fmByte(FMADDR_METER_PRECISE_TIME_RECORD_SAVE_IDX);
 //   if(pos >= MAX_METER_PRECISE_TIME_RECORD_COUNT)
 //   {
 //       pos = 0;
 //       write_fmByte(FMADDR_METER_PRECISE_TIME_RECORD_SAVE_IDX,pos);
 //   }

    mem_set(time_data.value,sizeof(PRECISE_TIME_RECORD),0xFF);
    set_dayhold_recdate_bcd(time_data.rec_date);
    time_data.before_err[0] = before;
    time_data.before_err[1] = before>>8;
    time_data.after_err[0] = after;
    time_data.after_err[1] = after>>8;
    time_data.flag = flag;

    mem_set(time_cast_data.value,sizeof(PRECISE_TIME_CAST_RECORD),0xFF);
    set_dayhold_recdate_bcd(time_cast_data.rec_date);
 //   get_yesterday(time_cast_data.rec_date);
    if(error_flag ==0xAA)/*AA��ʾУʱ�ɹ����������ֵ��*/
    {
    time_cast_data.after_err[0] = after;
    time_cast_data.after_err[1] = after>>8;
    time_cast_data.after_err[2] = after>>16;
    }
    else
    {
    time_cast_data.after_err[0] = before;
    time_cast_data.after_err[1] = before>>8;
    time_cast_data.after_err[2] = before>>16;
    }
    time_cast_data.error_flag = error_flag;

    fwrite_array(meter_idx,PIM_METER_PRECISE_TIME_RECORD+sizeof(PRECISE_TIME_RECORD),time_data.value,sizeof(PRECISE_TIME_RECORD));

  //  fwrite_array(meter_idx,PIM_METER_PRECISE_TIME_RECORD+pos*sizeof(PRECISE_TIME_RECORD),time_data.value,sizeof(PRECISE_TIME_RECORD));

    fwrite_array(FILEID_PRECISE_TIME_STAT_DATA,meter_idx*sizeof(PRECISE_TIME_RECORD),time_data.value,sizeof(PRECISE_TIME_RECORD));

    td[0] = byte2BCD(datetime[DAY]);
    td[1] = byte2BCD(datetime[MONTH]);
    td[2] = byte2BCD(datetime[YEAR]);

    previous_dayhold_td(td);
//    offset = byte2BCD(datetime[DAY]) - 1;
    offset = td[0] - 1;
    offset *= sizeof(PRECISE_TIME_CAST_RECORD);
    offset += PIM_METER_PRECISE_TIME_CAST_RECORD;
    fwrite_array(meter_idx,offset,time_cast_data.value,sizeof(PRECISE_TIME_CAST_RECORD));//Уʱ������洢��
    //��Ҫר��Ū���ļ��洢ͳ���������������ͳ���ļ�
    fwrite_array(FILEID_PRECISE_TIME_CAST_STAT_DATA,meter_idx*sizeof(PRECISE_TIME_CAST_RECORD),time_cast_data.value,sizeof(PRECISE_TIME_CAST_RECORD));

}

void plc_router_save_meter_precise_time(INT16U meter_idx,INT32U phy,INT8U *data,INT8U datalen)
{
    INT32U meter_num,cur_num,offset;
    INT8U meter_datetime[7];
    INT8U cur_datetime[10],td[3];  //���ʱ��   ��-��-ʱ-��-��|��-��
    SET_F298 F298;
    INT8U ctrl_flag,flag;
    flag = 0;

    fread_ertu_params(EEADDR_SET_F298,F298.value,sizeof(SET_F298));

    if(phy == 0x04000100)
    {
        ctrl_flag = 0x03;
        fwrite_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_FLAG,&ctrl_flag,1);

        //˵����ʱû��Ӧ��ֱ�ӽ�������
        if ((datalen == 1) && (data[0] == REC_DATA_IS_DENY))
        {
            fread_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_FLAG,cur_datetime,10);
            cur_num = bin2_int32u(cur_datetime+PIM_METER_PRECISE_TIME_ERROR);
            save_meter_precise_time_spot(meter_idx,cur_num/1000,0,2,0xFF);
//            meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);
        }
    }
    else if(phy == RQ_XQ)  //�������   YYMMDDWW
    {
        fwrite_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_DATA,data,(datalen > 4) ? 4 : datalen);

        fread_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_FLAG,meter_datetime,2);
        meter_datetime[PIM_METER_PRECISE_TIME_FLAG] &= ~0x01;
        //if(meter_datetime[PIM_METER_PRECISE_TIME_CYCLE] > 3)  meter_datetime[1] = 0;
        fwrite_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_FLAG,meter_datetime,2);
        //clr_bit_value(read_params->read_mask.day_hold,READ_MASK_BYTE_NUM_DAY_HOLD,READ_MASK_DAY_HOLD_FYG_ZDXL_DH);
    }
    else if(phy == SJ)
    {
        mem_cpy(cur_datetime,(INT8U*)datetime,6);

        flag = 0x55;

        td[0] = byte2BCD(datetime[DAY]);
        td[1] = byte2BCD(datetime[MONTH]);
        td[2] = byte2BCD(datetime[YEAR]);

        previous_dayhold_td(td);
        offset = td[0] - 1;
        offset += PIM_METER_PRECISE_TIME_HAD_READ;
        fwrite_array(meter_idx,offset,&flag,1);


        fread_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_DATA+1,meter_datetime+DAY,3);

        meter_datetime[DAY] = BCD2byte(meter_datetime[DAY]);
        meter_datetime[MONTH] = BCD2byte(meter_datetime[MONTH]);
        meter_datetime[YEAR] = BCD2byte(meter_datetime[YEAR]);

        meter_datetime[SECOND] = BCD2byte(data[0]);
        meter_datetime[MINUTE] = BCD2byte(data[1]);
        meter_datetime[HOUR] = BCD2byte(data[2]);

     #ifdef __QGDW_376_2013_PROTOCOL__
        datetime_add_seconds(meter_datetime+YEAR,meter_datetime+MONTH,meter_datetime+DAY,
                    meter_datetime+HOUR,meter_datetime+MINUTE,meter_datetime+SECOND,portContext_plc.router_phase_work_info[0].comm_time);
     #endif

//        #ifdef __PRECISE_TIME_TEST__
//        mem_set(tmp_data.value,sizeof(PRECISE_TIME_TMP_DATA),0xFF);
//        mem_cpy(tmp_data.rtu_date,cur_datetime,6);
//        mem_cpy(tmp_data.meter_date,meter_datetime,6);
//        fwrite_array(FILEID_PRECISE_TIME_TMP_DATA,meter_idx*sizeof(PRECISE_TIME_TMP_DATA),tmp_data.value,sizeof(PRECISE_TIME_TMP_DATA));
//        #endif

        mem_cpy(cur_datetime,(INT8U*)datetime,6);
        //�������
        meter_num = get_millisecond_from_time(meter_datetime[HOUR],meter_datetime[MINUTE],meter_datetime[SECOND],0);
        cur_num = get_millisecond_from_time(cur_datetime[HOUR],cur_datetime[MINUTE],cur_datetime[SECOND],0);
        if(meter_num > cur_num) meter_num = meter_num - cur_num;
        else meter_num = cur_num - meter_num;

        if(compare_string(meter_datetime+DAY,cur_datetime+DAY,3) != 0)
        {
            cur_datetime[1] = byte2BCD(meter_datetime[DAY]);
            cur_datetime[2] = byte2BCD(meter_datetime[MONTH]);
            cur_datetime[3] = byte2BCD(meter_datetime[YEAR]);
            cur_datetime[4] = byte2BCD(meter_datetime[SECOND]);
            cur_datetime[5] = byte2BCD(meter_datetime[MINUTE]);
            cur_datetime[6] = byte2BCD(meter_datetime[HOUR]);

            //��-��|��-��- ��-��-ʱ  bcd
        //    check_meter_datetime(meter_idx,cur_datetime); //ʱ�䳬���ж�

//             meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);

            save_meter_precise_time_spot(meter_idx,meter_num/1000,0,2,0xFF);

            return;
        }

        if ((meter_num / 1000) > 300) //����5���ӣ�������Уʱ
        {
            cur_datetime[1] = byte2BCD(meter_datetime[DAY]);
            cur_datetime[2] = byte2BCD(meter_datetime[MONTH]);
            cur_datetime[3] = byte2BCD(meter_datetime[YEAR]);
            cur_datetime[4] = byte2BCD(meter_datetime[SECOND]);
            cur_datetime[5] = byte2BCD(meter_datetime[MINUTE]);
            cur_datetime[6] = byte2BCD(meter_datetime[HOUR]);

            //��-��|��-��- ��-��-ʱ  bcd
  //          check_meter_datetime(meter_idx,cur_datetime);  //���ܱ�ʱ�ӳ����¼��ж�
  /*
            if(F298.flag ==3)
            {
            if(GPLC.adjtime_flag.is_precise_time_ready ==2)
             meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);
            }
            else
            meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);
   */
            save_meter_precise_time_spot(meter_idx,meter_num/1000,0,2,0xF5);

            return;
        }

        fread_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_FLAG,cur_datetime,10);

        if (F298.flag == 3)
       {

          save_meter_precise_time_spot(meter_idx,cur_num/1000,0,0,0xAA);    //�㲥��ɹ��� �㲥û�в�ֵ��Ҫ���ɹ���Ҫ�����ɹ���û����ֵ��5��֮�������
       }
      else
      {

        if(meter_num <= F298.error_threshold*1000)  //�����ֵ
        {
          //  if(((cur_datetime[PIM_METER_PRECISE_TIME_FLAG] & 0xFC) == 0) || (GPLC.adjtime_flag.is_precise_time_ready ==2))
            if((cur_datetime[PIM_METER_PRECISE_TIME_FLAG] & 0xFC) == 0)
            {
                cur_num = bin2_int32u(cur_datetime+PIM_METER_PRECISE_TIME_ERROR);
               #ifndef __NO_SIMPLE_TIME_BEFORE_POWER_ON__
                event_erc_51_precise_time(meter_idx,cur_num/1000,meter_num/1000,1); //Уʱ�ɹ�
               #endif
                save_meter_precise_time_spot(meter_idx,cur_num/1000,meter_num/1000,1,0xAA);
            }
            else
            {
                save_meter_precise_time_spot(meter_idx,meter_num/1000,0,2,0xAA);
            }

            fwrite_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_ERROR,(INT8U*)&meter_num,4);
           /*
            if(F298.flag ==3)
            {
            if(GPLC.adjtime_flag.is_precise_time_ready ==2)
             meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);
            }
            else
            meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);
            */
        }
        else
        {
           // if ((F298.flag == 3) && (GPLC.adjtime_flag.is_precise_time_ready ==0)) //3������ȫ�ն˵Ĺ㲥��ʱ����㲥��ɺ��ٳ���ƫ��
            if (F298.flag == 3)
            {
                //������Ҫ�㲥Уʱ
             //   GPLC.adjtime_flag.is_precise_time_cast_flag = 1;
             //   if(GPLC.adjtime_flag.is_precise_time_ready ==0)
             //   GPLC.adjtime_flag.is_precise_time_ready = 1;
 //                save_meter_precise_time_spot(meter_idx,meter_num/1000,0,2);
//                meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);
                save_meter_precise_time_spot(meter_idx,cur_num/1000,meter_num/1000,0,0xAA);    //�㲥��ɹ���
            }
            else
            {
              //  if(((cur_datetime[PIM_METER_PRECISE_TIME_FLAG] & 0xFC) == 0) || (GPLC.adjtime_flag.is_precise_time_ready ==2))
                if((cur_datetime[PIM_METER_PRECISE_TIME_FLAG] & 0xFC) == 0)
                {
                    cur_num = bin2_int32u(cur_datetime+PIM_METER_PRECISE_TIME_ERROR);
                    #ifndef __NO_SIMPLE_TIME_BEFORE_POWER_ON__
                    event_erc_51_precise_time(meter_idx,cur_num/1000,meter_num/1000,0);  //Уʱʧ��
                    #endif
                //    meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);

                }
                      save_meter_precise_time_spot(meter_idx,cur_num/1000,meter_num/1000,0,0xBB);
            fwrite_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_ERROR,(INT8U*)&meter_num,4);

    //            if(cur_datetime[1] >= F298[6])
    //            {
    //                meter_read_flag_flash_clear_one_flag(meter_idx,READFLAG_METER_PRECISE_TIME);
    //            }
    //            else
                {
                    cur_datetime[0] &= ~0x02;
                    fwrite_array(FILEID_PLC_REC_TMP+portContext_plc.router_work_info.channel_id-1,PIM_METER_PRECISE_TIME_FLAG,cur_datetime,1);
                }
            }
        }
      }

    }
}
#endif
#if defined(__INSTANT_FREEZE__)
INT8U readdata_instant_freeze_curve(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[5],INT8U *data,INT8U *datalen)
{
    INT32U offset;
    INT8U midu = 0;

    if((instant_freeze.cycle & 0xC0) == 0)
    midu = instant_freeze.cycle;
    else if((instant_freeze.cycle & 0xC0) == 0x40)
    midu = 60;

    if(midu == 0) midu = 0xAA;

    offset = get_curve_save_offset(phy,td,midu);

    fread_array(meter_idx,offset,data,phy->data_len);
    if (compare_string(td,data,5) == 0)
    {
        if(check_is_all_ch(data+5+phy->block_offset,phy->block_len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {
            *datalen = phy->block_len;
            mem_cpy(data,data+5+phy->block_offset,phy->block_len);
            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

BOOLEAN prepare_plc_read_meter_instant_freeze(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
  //  READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT16U  meter_idx,mask_idx;
    INT16U  freeze_id;
    INT8U   td[6],tmp[4];
    INT8U   bplc_flg = 0;
    INT8U   bplc_buf[200] = {0};
    INT8U   bplc_len = 0;
    INT8U   cnt_id = 0;
    BOOLEAN flg = FALSE;


    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    #ifdef __READ_OOP_METER__
    if(read_params->meter_doc.protocol != GB645_2007)
    {
        clr_bit_value(read_meter_flag_instant_freeze.flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }
    #endif

    if (get_bit_value(read_meter_flag_instant_freeze.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;
    if (check_is_all_ch(read_params->read_mask.curve_instant_freeze,READ_MASK_BYTE_NUM_INSTANT_FREEZE,0x00))
    {
        clr_bit_value(read_meter_flag_instant_freeze.flag,READ_FLAG_BYTE_NUM,meter_idx);

        return FALSE;
    }



    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC) || (read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
    {
        if( (CYCLE_REC_MODE_PARALLEL == portContext_plc.plc_other_read_mode) 
        &&  (ROUTER_VENDOR_TOPSCOMM  == portContext_plc.router_base_info.router_vendor) )
        {
            /* ���ſ�� */
            bplc_flg = 0xAA;
            freeze_id = portContext_plc.freeze_id;
            bplc_len = make_bplc_instant_freeze_frame(0x0002,freeze_id,bplc_buf,read_params->meter_doc.meter_no,NULL,0);
        }
    }
    
    if (check_is_all_ch(read_params->day_hold_td,3,0x00))
    {
        get_yesterday(read_params->day_hold_td);
        get_former_month(read_params->month_hold_td);
    }

    for(mask_idx=0;mask_idx<5;mask_idx++)
    {
        if(get_bit_value(read_params->read_mask.curve_instant_freeze,READ_MASK_BYTE_NUM_INSTANT_FREEZE,mask_idx))
        {
            mem_cpy(td,(instant_freeze_control.instant_start_time)+1,5); //����ֵ,�˴���Ҫ�����15���ӱ����Ĺ�ϵ��������������

            if(read_instant_freeze_data(meter_idx,td,mask_idx) == FALSE)  //���һ�¹̶�λ�õ����ݱ�ʶ�Ƿ��Ѿ������ݣ�
            {
                //��Ҫ����
                if(mask_idx == 0)     mem_cpy(tmp,instant_freeze.item1,4);
                else if(mask_idx == 1)mem_cpy(tmp,instant_freeze.item2,4);
                else if(mask_idx == 2)mem_cpy(tmp,instant_freeze.item3,4);
                else if(mask_idx == 3)mem_cpy(tmp,instant_freeze.item4,4);
                else
                mem_cpy(tmp,instant_freeze.item5,4);
                
                
                library.item = bin2_int32u(tmp);
                if( (library.item != 0xFFFFFFFF) && (library.item != 0) ) 
                {
                    int32u2_bin(library.item,read_params->item);
                    read_params->resp_byte_num = 40;
                    read_params->read_type = READ_TYPE_INSTANT_FREEZE;
                    
                    if (read_params->meter_doc.protocol == GB645_2007)
                    {
                        if(0xAA == bplc_flg)
                        {
                            /*  */
                            cnt_id ++;
                            if(flg)
                            {
                                bplc_buf[bplc_len++] = 0xAA;
                            }
                            mem_cpy(bplc_buf + bplc_len,tmp,4);
                            bplc_len += 4;

                            if(FALSE == flg)
                            {
                                flg = TRUE;
                            }
                            continue;
                        }
                        else
                        {
                            *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                        }
                    }
                    else
                    {
                        clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
                        return FALSE;
                    }
                    
                    #ifdef __SOFT_SIMULATOR1__
                    snprintf(info,100,"*** prepare item day hold : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d",
                    meter_idx,library.item,phy.phy,mask_idx);
                    debug_println_ext(info);
                    #endif
                    
                    return TRUE;
                }
                //����msak
                //   clear_read_mask_from_meter_param(meter_idx,READ_TYPE_CUR_DATA,mask_idx);
                
            }
            clr_bit_value(read_params->read_mask.curve_instant_freeze,READ_MASK_BYTE_NUM_INSTANT_FREEZE,mask_idx);
        }

    }

    if( (0xAA == bplc_flg) && (cnt_id > 0 ) )
    {
        read_params->read_type = READ_TYPE_INSTANT_FREEZE;
        /* ���������� */
        bplc_buf[8] |= (cnt_id << 4);
        mem_cpy(frame,bplc_buf,bplc_len);
        *frame_len = bplc_len;
        return TRUE;
    }

    clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,meter_idx);

    return FALSE;
}

INT32U get_instant_freeze_save_offset(INT8U data_len,INT8U td[5],INT8U midu)
{
    INT32U offset;
    INT8U num;

    num = 60 / midu;
    offset = getPassedDays(2000+td[4],td[3],td[2]);
    offset *= num * 24;
    offset += td[1] * num;
    offset += td[0] / midu;
    offset = offset % SAVE_POINT_NUMBER_CURVE;
    offset *= data_len;
    offset += PIM_CURVE_INSTANT_FREEZE;
    offset++;     //�ܶ�

    return offset;
}
/*+++
  ���ܣ� ����˲ʱ��������
  ������
         INT8U *frame ����645����
  ���أ�
         BOOLEAN
  ������
       1) 68H	A0	����	A5	68H	96H	L    99 99 99 99  NN  L1 DI0~DI4 N1...Nm    L2������	CS	16H
       2) �����룺C=96H
       3) ��ַ�򣺵���ַ
       4) ���ݵ�Ԫ��ʶ��D0H
       5) ���ݵ�Ԫ���ݣ�PA0~ PA5Ϊ�����ز����ڵ��ַ
---*/
void save_instant_freeze_data(READ_PARAMS *read_params,INT8U *data,INT8U datalen)
{
    INT32U offset;
    INT16U meter_idx;
    INT8U midu,idx;;//data_len,
    INT8U td[5],buffer[200];
    INT8U item1_data_len,item2_data_len,item3_data_len,item4_data_len,item_all_data_len;
    INT8U offset_add = 0;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    //��ȡ�ܶ�

    if((instant_freeze.cycle & 0x3F) == 0)
    midu = 0xAA;
    else if((instant_freeze.cycle & 0xC0) == 0)
    midu = instant_freeze.cycle;
    else 
    midu = 60;


    mem_cpy(td,instant_freeze_control.instant_start_time+1,5);

    item1_data_len = get_item_datalen(instant_freeze.item1); //Ҫ����������ĳ���
    item2_data_len = get_item_datalen(instant_freeze.item2);
    item3_data_len = get_item_datalen(instant_freeze.item3);
    item4_data_len = get_item_datalen(instant_freeze.item4);
    item_all_data_len = item1_data_len+item2_data_len+item3_data_len+item4_data_len;

    idx = get_item_idx(data);
    if(midu ==0xAA)offset = INSTANT_FREEZE_DATA ; //ֻ����һ�ε����ݣ��ŵ���ǰ��������

     else
     offset = get_instant_freeze_save_offset(item_all_data_len+5,td,midu);

    fread_array(meter_idx,offset,buffer,item_all_data_len);

    //��֡��ʽ�� ʱ��5+���ݱ�ʶ4+���ݳ���1+����
    if(compare_string(buffer,td,5) != 0)   //���ʶ����ͬ��Ҫ����ÿһ��������洢��ʱ���Ƿ�Ҫ����ʱ�ꡣ
    {
        mem_set(buffer,item_all_data_len+5,0xFF);
        mem_cpy(buffer,td,5);

        if( (idx == 1) && (datalen>4) )
        {
        offset_add = 0;
        mem_cpy(buffer+5,data,4);
        buffer[9] = (item1_data_len-5);
        mem_cpy(buffer+10,data+4,(item1_data_len-5));
        fwrite_array(meter_idx,offset+offset_add,buffer,item1_data_len+5);
        }
        else if( (idx == 2) && (datalen>4) )
        {
        offset_add = item1_data_len;     //���ʱ��+����������ƫ��+���ݣ�����bufferд��ȥ
        mem_cpy(buffer+5+offset_add,data,4);
        buffer[9+offset_add] = (item2_data_len-5);
        mem_cpy(buffer+10+offset_add,data+4,(item2_data_len-5));
        fwrite_array(meter_idx,offset,buffer,item2_data_len+5+item1_data_len);
        }
        else if( (idx == 3) && (datalen>4) )
        {
        offset_add = item2_data_len+item1_data_len;     //���ʱ��+����������ƫ��+���ݣ�����bufferд��ȥ
        mem_cpy(buffer+5+offset_add,data,4);
        buffer[9+offset_add] = (item2_data_len-5);
        mem_cpy(buffer+10+offset_add,data+4,(item3_data_len-5));
        fwrite_array(meter_idx,offset,buffer,item3_data_len+5+item2_data_len+item1_data_len);
        }
        else if( (idx == 4)  && (datalen>4) )
        {
        offset_add = item2_data_len+item1_data_len+item3_data_len;     //���ʱ��+����������ƫ��+���ݣ�����bufferд��ȥ
        mem_cpy(buffer+5+offset_add,data,4);
        buffer[9+offset_add] = (item2_data_len-5);
        mem_cpy(buffer+10+offset_add,data+4,(item4_data_len-5));
        fwrite_array(meter_idx,offset,buffer,item4_data_len+5+item2_data_len+item1_data_len+item3_data_len);
        }
    }
    else
    {
        mem_cpy(buffer,data,4);

        if( (idx == 1) && (datalen>4) )
        {
        offset_add = 0;
        buffer[4] = (item1_data_len-5);
        mem_cpy(buffer+5,data+4,(item1_data_len-5));
        fwrite_array(meter_idx,offset+offset_add+5,buffer,item1_data_len);
        }
        else if( (idx == 2) && (datalen>4) )
        {
        offset_add = item1_data_len;
        buffer[4] = (item2_data_len-5);
        mem_cpy(buffer+5,data+4,(item2_data_len-5));
        fwrite_array(meter_idx,offset+offset_add+5,buffer,item2_data_len);
        }
        else if( (idx == 3) && (datalen>4) )
        {
        offset_add = item2_data_len+item1_data_len;
        buffer[4] = (item3_data_len-5);
        mem_cpy(buffer+5,data+4,(item3_data_len-5));
        fwrite_array(meter_idx,offset+offset_add+5,buffer,item3_data_len);
        }
        else if( (idx == 4)  && (datalen>4) )
        {
        offset_add = item3_data_len+item2_data_len+item1_data_len;
        buffer[4] = (item4_data_len-5);
        mem_cpy(buffer+5,data+4,(item4_data_len-5));
        fwrite_array(meter_idx,offset+offset_add+5,buffer,item4_data_len);
        }
    }
    clr_bit_value(read_params->read_mask.curve_instant_freeze,READ_MASK_BYTE_NUM_INSTANT_FREEZE,(idx-1));
}
BOOLEAN read_instant_freeze_data(INT16U meter_idx,INT8U td[5],INT8U mask_idx)
{
    INT32U offset;
    INT8U midu;//data_len,idx;
    INT8U buffer[200],tmp[5];
    INT8U item1_data_len,item2_data_len,item3_data_len,item4_data_len,item_all_data_len;
    INT8U offset_add=0;

    //��ȡ�ܶ�
    if((instant_freeze.cycle & 0x3F) == 0)
    midu = 0xAA;
    else if((instant_freeze.cycle & 0xC0) == 0)
    midu = instant_freeze.cycle;
    else
    midu = 60;


    item1_data_len = get_item_datalen(instant_freeze.item1); //Ҫ����������ĳ���


    item2_data_len = get_item_datalen(instant_freeze.item2);

    item3_data_len = get_item_datalen(instant_freeze.item3);

    item4_data_len = get_item_datalen(instant_freeze.item4);

    item_all_data_len = item1_data_len+item2_data_len+item3_data_len+item4_data_len;

    if(midu == 0xAA)offset = INSTANT_FREEZE_DATA ;
    else
    offset = get_instant_freeze_save_offset(item_all_data_len+5,td,midu);

    fread_array(meter_idx,offset,buffer,item_all_data_len+5);

    if((item1_data_len == 5) && (mask_idx == 0))
    {
        mem_cpy(tmp,instant_freeze.item1,4);
        tmp[4] = 0;
        fwrite_array(meter_idx,offset+5,tmp,5);
        return TRUE; //��֧�ֵ��������
    }
    if((item2_data_len == 5) && (mask_idx == 1))
    {
        mem_cpy(tmp,instant_freeze.item2,4);
        tmp[4] = 0;
        fwrite_array(meter_idx,offset+item1_data_len+5,tmp,5);
        return TRUE; //��֧�ֵ��������
    }
    if((item3_data_len == 5) && (mask_idx == 2))
    {
                mem_cpy(tmp,instant_freeze.item3,4);
        tmp[4] = 0;
        fwrite_array(meter_idx,offset+item1_data_len+item2_data_len+5,tmp,5);
        return TRUE; //��֧�ֵ��������
    }
    if((item4_data_len == 5) && (mask_idx == 3))
    {
                mem_cpy(tmp,instant_freeze.item4,4);
        tmp[4] = 0;
        fwrite_array(meter_idx,offset+item1_data_len+item2_data_len+item3_data_len+5,tmp,5);
        return TRUE; //��֧�ֵ��������
    }
    //����ʱ��
    if(compare_string(buffer,td,5) != 0)
    {
      mem_set(buffer,item_all_data_len+5,0xFF);
      fwrite_array(meter_idx,offset,buffer,item_all_data_len+5);
      return FALSE;
    }
    else
    {

        if(mask_idx == 0)offset_add = 0;
        else if(mask_idx == 1) offset_add = item1_data_len;
        else if(mask_idx == 2) offset_add = item2_data_len+item1_data_len;
        else if(mask_idx == 3) offset_add = item3_data_len+item1_data_len+item2_data_len;

        if(buffer[offset_add+5] == 0xFF)return FALSE; //�����FF����Ҫ����
    }

    return TRUE;

}
INT8U get_item_idx(INT8U item[4])
{
    if(compare_string(item,instant_freeze.item1,4)==0)return 1;
    if(compare_string(item,instant_freeze.item2,4)==0)return 2;
    if(compare_string(item,instant_freeze.item3,4)==0)return 3;
    if(compare_string(item,instant_freeze.item4,4)==0)return 4;
    return 0;
}

INT8U get_item_datalen(INT8U item[4])
{
  INT32U item_32;
  INT8U item_datalen;

  item_32 = bin2_int32u(item);
  switch(item_32)
  {
   case 0x0001FF00:
   case 0x0002FF00:
   case 0x0003FF00:
   case 0x0004FF00:
   case 0x05060101://������
       item_datalen = 25 ;//20���ݳ���+item+����
       break;
   case 0x0201FF00:
       item_datalen = 11;   // 3*2+4+1
       break;
   case 0x0202FF00:
       item_datalen = 14;  // 3*3+4+1
       break;
   case 0x0203FF00:
        item_datalen = 17 ;
        break;
   case 0x00000000:
   case 0xFFFFFFFF:
       item_datalen = 0;
       break;
   default:
       item_datalen =5;  // ��֧�ֵľ���5���ֽڣ�item+����0
       break;
  }


 return item_datalen ;
}

#endif


#ifdef __METER_DAY_FREEZE_EVENT__
INT8U  meter_freeze_event_read_ctrl(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,BOOLEAN append_item)
{
	INT32S	tmp_val,tmp_val1;
	INT32U 	offset;
	INT32U  tmp_data_item;
	INT16U 	meter_idx,data_len;
	INT8U 	item_data[PIM_FREEZE_EVENT_DATA_LEN];
	INT8U 	item_idx;
	INT8U	idx;
	BOOLEAN is_valid_bcd = FALSE;
	BOOLEAN valid_data_flag = FALSE;
	INT8U	td[3];
	INT8U	rec_datetime[5];
	INT8U	data[15];
	mem_set(item_data,sizeof(item_data),0x00);
	//item_idx = read_params->freeze_event_ctrl.item_ctrl.item;// ���������λ��
	meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
	item_idx = read_params->cur_mask_idx;//
	
	if( (read_params->meter_doc.protocol ==  GB645_2007))
	{
		//��ȡ������		
		offset = PIM_FREEZE_EVENT_ITEM_CNT_START;
		offset += item_idx*PIM_FREEZE_EVENT_DATA_LEN;
		fread_array(meter_idx,offset,item_data,PIM_FREEZE_EVENT_DATA_LEN);
		read_params->freeze_event_ctrl.item_ctrl.ctrl_flag.event_flag = 0;
		if(compare_string(item_data,read_params->item,4) == 0) //ͬһ��ʶ
		{
			mem_cpy(td,read_params->day_hold_td,3);
			date_minus_days(td+2,td+1,td,1);
			valid_data_flag = readdata_cycle_day_event(meter_idx,read_params->freeze_event_ctrl.freeze_data,td,rec_datetime,data,&data_len,FALSE);
			frame_len = (frame_len<=4)? frame_len:4;
			if( (compare_string(item_data+4,frame,frame_len)==0) && (valid_data_flag == TRUE) )//δ�����ı�
			{
				return SAVE_LAST_FREEZE_EVENT_RECORD;
			}
			else
			{
				//
                             
				tmp_val =  bcd2u32(frame,frame_len,&is_valid_bcd);
				if(compare_string(item_data+4,frame,frame_len)!=0)//�����б仯��ʱ��Ű��ձ仯���������������յ�ǰ��������
				{
					tmp_val1 = bcd2u32(item_data+4,frame_len,&is_valid_bcd); 
					tmp_val -=  tmp_val1;
				}
				mem_cpy(item_data+4,frame,frame_len);
                fwrite_array(meter_idx,offset,item_data,PIM_METER_EVENT_SAVE_DATA_07_PER_ITEM_LEN);	
				
				if(tmp_val < 0)//���������� ���յ�ǰ������������
				{		
					tmp_val =  bcd2u32(frame,frame_len,&is_valid_bcd);
					tmp_val = (tmp_val>=10)?10:tmp_val;
					read_params->freeze_event_ctrl.item_ctrl.record_cnt = tmp_val;
					read_params->freeze_event_ctrl.item_ctrl.read_record_ptr = 0;//
					read_params->freeze_event_ctrl.item_ctrl.flag = 0xAA;//�������ڳ�������������
					return KEEP_ON_RECORDING;
				}
				else
				{																
					tmp_val = (tmp_val>=10) ? 10:tmp_val;
					read_params->freeze_event_ctrl.item_ctrl.record_cnt = tmp_val;
					read_params->freeze_event_ctrl.item_ctrl.read_record_ptr = 0;//
					read_params->freeze_event_ctrl.item_ctrl.flag = 0xAA;//�������ڳ�������������
					#ifdef EVENT_GRADE_INFO
				    snprintf(info,100,"======== �¼��ܴ����仯 =%d,��������=%d***===== ",
				            tmp_val,read_params->freeze_event_ctrl.item_ctrl.record_cnt);
				    debug_println_ext(info);               
					#endif					
					return KEEP_ON_RECORDING;
					
				}
				
			}
		}
		else//��һ�δ洢�����ݴ�����ѡ�񳭶�����
		{
			mem_cpy(item_data,read_params->item,4);
			frame_len = (frame_len<=4)? frame_len:4;
			mem_cpy(item_data+4,frame,frame_len);
			fwrite_array(meter_idx,offset,item_data,PIM_FREEZE_EVENT_DATA_LEN);
			tmp_val = bcd2u32(frame,frame_len, &is_valid_bcd);
			if(tmp_val >0)
			{
				tmp_val = (tmp_val>=10)?10:tmp_val;
				#ifdef __SOFT_SIMULATOR__
				tmp_val = 3;// ��ʱ����ʹ��
				#endif
				read_params->freeze_event_ctrl.item_ctrl.record_cnt = tmp_val;
				read_params->freeze_event_ctrl.item_ctrl.read_record_ptr = 0;//
				read_params->freeze_event_ctrl.item_ctrl.flag = 0xAA;//�������ڳ�������������
				return  KEEP_ON_RECORDING;
			}
			else
			{
				return CLEAR_READ_MASK;//�������
			}
			
		}
	}
	else //97 or ����Э��
	{
		
	}
	//return TRUE;
}
/*
----------------------------------------------------------------------
�ȱ��������Ȼ����ݵ�ǰ�����ʹ洢�������ޱ仯���ж��Ƿ񳭶�����������

�洢����: 
2�ֽڵĳ���    2���ֽ�
item           4���ֽ�//������൱�ڴ洢����ͷ
��ǰ����       4���ֽ�
�仯����       1���ֽ� <=10
��¼1 

.........
��¼n   



----------------------------------------------------------------------
*/
void save_tmp_meter_event_data(READ_PARAMS *read_params,INT8U *data,INT16U	data_len,FREEZE_DATA_SAVE_CTRL save_ctrl)
{
	//��ʱ���浽�ļ���������ɺ󣬱��浽
	INT32U	item32u,tmp_item;
	INT16U 	meter_idx;
	INT16U	len;
	INT8U	head[12];
	meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);


	//
	mem_set(head,sizeof(head),0x00);
	if(save_ctrl.cnt_flag)//�洢����
	{
		mem_cpy(head+2,read_params->item,4);
		mem_cpy(head+6,data,data_len);
		head[10] = read_params->freeze_event_ctrl.item_ctrl.record_cnt;
		len = 9;//������len������ռ�ݵ�2���ֽ�
		head[0] = len;
		head[1] = len>>8;
		fwrite_array(meter_idx,PIM_FREEZE_EVENT_RECORD_TMP_DATA_START,head,11);
	}
	else
	{
		fread_array(meter_idx,PIM_FREEZE_EVENT_RECORD_TMP_DATA_START,head,11);
		item32u = bin2_int32u(head+2);
		len = bin2_int16u(head);
		tmp_item  = bin2_int32u(read_params->item);
		if((item32u&0xFFFF0000) == (tmp_item &0xFFFF0000) )
		{
			switch(read_params->freeze_event_ctrl.freeze_data.item)
			{
				case 0x03300D00:
				case 0x03300E00:
					if(data_len >=12)
					{
						data_len = 12;
					}
					else
					{
						mem_set(data+data_len,12-data_len,0xEE);
						data_len = 12;						
					}
					break;
				case 0x03300100:
					if(data_len >=66)
					{
						data_len = 66;
					}
					else
					{
						mem_set(data+data_len,66-data_len,0xEE);
						data_len = 66;					
					}
					break;
				case 0x1D000001:
				case 0x1E000001:
					if(data_len >=10)
					{
						data_len = 10;
					}
					else
					{
						mem_set(data+data_len,10-data_len,0xEE);
						data_len = 10;						
					}
					break;
			}
			//��д������
			fwrite_array(meter_idx,PIM_FREEZE_EVENT_RECORD_TMP_DATA_START+len+2,data,data_len);

			//д�볤����Ϣ
			len += data_len;
			head[0] = len;
			head[1] = len>>8;
			fwrite_array(meter_idx,PIM_FREEZE_EVENT_RECORD_TMP_DATA_START,head,11);

			
		}
	}
}

BOOLEAN save_freeze_event(READ_PARAMS *read_params,BOOLEAN read_tmp_save_flag)
{
	DAY_FREEZE_EVENT_DATA freeze_data;
	INT32U 	item,offset;
	INT16U	meter_idx;
	INT16U	len;
	INT8U	td[3];
	INT8U	idx;
	BOOLEAN flag = FALSE;
	INT8U *data;
	INT8U	cnt;
	meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

	//item = bin2_int32u(read_params->item);
	item = read_params->freeze_event_ctrl.freeze_data.item;
	mem_cpy(td,read_params->day_hold_td,3);

	if(read_tmp_save_flag == FALSE)
	{
		date_minus_days(td+2,td+1,td,1);
	}
	
	for(idx=0;idx<(sizeof(CYCLE_DAY_FREEZE_ITEM_LIST)/sizeof(DAY_FREEZE_EVENT_DATA));idx++)
	{
		if(CYCLE_DAY_FREEZE_ITEM_LIST[idx].item == item)
		{
			mem_cpy((void *)&freeze_data,(void*)&CYCLE_DAY_FREEZE_ITEM_LIST[idx],sizeof(DAY_FREEZE_EVENT_DATA));
			flag = TRUE;
			break;
		}
	}

	if(flag == TRUE)
	{
		offset = get_cycle_day_event_save_offset(freeze_data,td);
		tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
		if(read_tmp_save_flag == FALSE)
		{
			data = (INT8U*)g_temp_buffer;
			fread_array(meter_idx,offset,data,freeze_data.block_len);
			if((td[2] == data[2]) && (td[1] == data[1]) && (td[0] == data[0]))
			{
				//ȡ֮ǰһ�յĶ��ᣬ��д��
				mem_cpy(data,read_params->day_hold_td,3);
				offset = get_cycle_day_event_save_offset(freeze_data,read_params->day_hold_td);
				fwrite_array(meter_idx,offset,data,freeze_data.block_len);
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			mem_set(g_temp_buffer,freeze_data.block_len,0x00);
			fread_array(meter_idx,PIM_FREEZE_EVENT_RECORD_TMP_DATA_START,(INT8U*)&len,2);
			mem_cpy(g_temp_buffer,read_params->day_hold_td,3);
			for(cnt=0;cnt<5;cnt++)
			{
				g_temp_buffer[3+cnt]=byte2BCD(datetime[MINUTE+cnt]);
			}
			fread_array(meter_idx,PIM_FREEZE_EVENT_RECORD_TMP_DATA_START+2,g_temp_buffer+8,len);
			fwrite_array(meter_idx,offset,g_temp_buffer,freeze_data.block_len);
		}
		tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
	}
	return TRUE;
	
}
INT8U save_freeze_event_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    INT32U begintd,endtd,metertd,curtd;
    READ_WRITE_DATA phy;
    INT8U idx;
    INT8U begin_td[3],end_td[3];

	INT32U 	data_item;
	INT32U	bit_and;
	FREEZE_EVENT_CTRL item_ctrl;
	INT16U	meter_idx;
	INT16U	len;
	INT8U	flag;
	INT8U	cur_idx;
    FREEZE_DATA_SAVE_CTRL save_ctrl;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    data_item = bin2_int32u(read_params->item);// ���ݱ�ʶ
	cur_idx = read_params->cur_mask_idx;

	mem_set((void*)&save_ctrl,sizeof(save_ctrl),0x00);
	// ��Ч���ݵĴ��� TODO??
	if(frame[0] == REC_DATA_IS_DENY) //���ж�������Ч�ԣ����ϣ��������ʶ
	{
		goto CLEAR_FLAG;  //����¼�����
	}
	//�����¼�����
	if(read_params->freeze_event_ctrl.item_ctrl.ctrl_flag.event_flag)
	{
		//save_tmp_meter_event_data();
		flag = meter_freeze_event_read_ctrl(read_params,frame,frame_len,FALSE);
		if(flag == CLEAR_READ_MASK)
		{
			goto CLEAR_FLAG;
		}
		else if(flag == KEEP_ON_RECORDING)
		{	
			save_ctrl.cnt_flag = 1;
			save_tmp_meter_event_data(read_params,frame,frame_len,save_ctrl);
			return 0;
		}
		else if(flag == SAVE_LAST_FREEZE_EVENT_RECORD)
		{
			save_freeze_event(read_params,FALSE);
			goto CLEAR_FLAG;
		}
		
	}
	else
	{
		if( ((data_item>>24) == 0x1D) || ((data_item>>24) == 0x1E) )
		{
			bit_and = 0xFFFFF0F0;
		}
		else
		{
			bit_and = 0xFFFFFFF0;
		}
		if((data_item & bit_and) == (read_params->freeze_event_ctrl.freeze_data.item & bit_and))
		{

			//
			save_ctrl.record_flag = 1;
			save_tmp_meter_event_data(read_params,frame,frame_len,save_ctrl);
			read_params->freeze_event_ctrl.item_ctrl.read_record_ptr++;
			
			if(read_params->freeze_event_ctrl.item_ctrl.read_record_ptr == read_params->freeze_event_ctrl.item_ctrl.record_cnt)
			{
				//
				save_freeze_event(read_params,TRUE);
				goto CLEAR_FLAG;
			}
                        return 0;
		}
		//else
			//goto CLEAR_FLAG;
			
	}
	CLEAR_FLAG:
	fread_array(FILEID_FREEZE_EVENT_CTRL,PIM_FREEZE_EVENT_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&item_ctrl,sizeof(FREEZE_EVENT_CTRL));
	if(item_ctrl.flag == 0x55)
	{		
		item_ctrl.flag = 0xFF;
		fwrite_array(FILEID_FREEZE_EVENT_CTRL,PIM_FREEZE_EVENT_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&item_ctrl,sizeof(FREEZE_EVENT_CTRL));		
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"***�¼������������д����Ч��ֵ***��flag = 0x%02X",item_ctrl.flag);
	    debug_println_ext(info);               
		#endif
	}
	read_params->freeze_event_ctrl.item_ctrl.flag = 0xFF;//��Ч
    clr_bit_value(read_params->read_mask.day_freeze_event,READ_MASK_BYTE_NUM_DAY_FREEZE_EVENT,cur_idx);

    return 0;
}

//�鿴������Ч��
BOOLEAN readdata_cycle_day_event(INT16U meter_idx, DAY_FREEZE_EVENT_DATA freeze_data, INT8U td [ 3 ], INT8U rec_datetime [ 5 ], INT8U * data, INT16U * datalen,BOOLEAN block_flag)//, INT8U * reserve_data)
{
    INT32U  offset;
    INT16U  len;
    offset = get_cycle_day_event_save_offset(freeze_data,td);
    len = (block_flag == TRUE)? freeze_data.block_len:freeze_data.data_len;
    fread_array(meter_idx,offset,data,len);
    if ((td[2] == data[2]) && (td[1] == data[1]) && (td[0] == data[0]))
    {
        mem_cpy(rec_datetime,data+3,5);
        if(check_is_all_ch(data+8,len,0xFF))
        {
            *datalen = 0;
            return FALSE;
        }
        else
        {          
            if(block_flag == FALSE)
            {
            	*datalen = len-8;
            	mem_cpy(data,data+8,len-8);
            }
            else
            {
                *datalen = len-3-RESERVE_DATA;
                mem_cpy(data,data+3,len-3-RESERVE_DATA);
            }
            //if(reserve_data)
            {
                //mem_cpy(reserve_data,data+phy->data_len-RESERVE_DATA,RESERVE_DATA);
            }

            return TRUE;
        }
    }
    else
    {
        *datalen = 0;
        return FALSE;
    }
}

INT16U	app_read_cycle_day_event(INT16U meter_idx,INT16U fn,INT8U *data,INT8U td[3])
{
    DAY_FREEZE_EVENT_DATA freeze_data;
    INT16U  datalen,pos;
    INT8U   rec_datetime [5];
    INT8U   idx,cnt;
    switch(fn)
    {
        case DT_F230:
            idx = 0;
            break;
        case DT_F231:
            idx = 1;
            break;
        case DT_F232:
            idx = 2;
            break;
        case DT_F233:
            idx = 3;
            break;
        case DT_F234:
            idx = 4;
            break;
    }
	freeze_data = CYCLE_DAY_FREEZE_ITEM_LIST[idx];

	if(readdata_cycle_day_event(meter_idx,freeze_data,td,rec_datetime ,data,&datalen,TRUE))
	{		
		pos = freeze_data.record_offset;
		cnt = data[pos-1];//����	
		if(cnt<10)
			mem_set(data+pos+freeze_data.record_len*cnt,(10-cnt)*freeze_data.record_len,0xEE);
		return datalen;
	}
	else

	{
		mem_set(data,freeze_data.block_len-3-RESERVE_DATA,0xEE);
		return freeze_data.block_len-3-RESERVE_DATA;
	}
	
}

void get_meter_event_info_and_mask(INT16U	 meter_idx,READ_PARAMS *read_params,INT16U	mask_idx)
{
	FREEZE_EVENT_CTRL item_ctrl;
	// copy ���� ƫ��  ���ȵȻ�����Ϣ
	mem_cpy((void *)&read_params->freeze_event_ctrl.freeze_data,(void *)&CYCLE_DAY_FREEZE_ITEM_LIST[mask_idx],sizeof(DAY_FREEZE_EVENT_DATA));

	if(read_params->freeze_event_ctrl.item_ctrl.flag == 0xAA)
		return;
	// ���ݳ��������ļ��洢�����ݼ�����������
	fread_array(FILEID_FREEZE_EVENT_CTRL,PIM_FREEZE_EVENT_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&item_ctrl,sizeof(FREEZE_EVENT_CTRL));
	if(item_ctrl.flag == 0x55)//0x55˵��������������Ҫ��ȡ�洢�ĳ������ƣ���������
	{
		mem_cpy((INT8U *)&read_params->freeze_event_ctrl.item_ctrl,(INT8U *)&item_ctrl,sizeof(FREEZE_EVENT_CTRL));
		read_params->freeze_event_ctrl.item_ctrl.flag = 0xAA;//��������������ĳ�����
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,100,"***����ᣬ����л���������ȡ�ļ�����,������=%02d***",
	            meter_idx);
	    debug_println_ext(info);               
		#endif
		return;
	}
	//�տ�ʼ�����¼�����Ҫ�����¼�����
	mem_set((INT8U*)&item_ctrl,sizeof(FREEZE_EVENT_CTRL),0x00);
	item_ctrl.ctrl_flag.event_flag = 1;
	item_ctrl.item = read_params->freeze_event_ctrl.freeze_data.item;

	mem_cpy((INT8U *)&read_params->freeze_event_ctrl.item_ctrl,(INT8U *)&item_ctrl,sizeof(FREEZE_EVENT_CTRL));
	return;
}
BOOLEAN prepare_plc_meter_dayfreeze_event_item(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
	INT32U	item_32u;
    INT16U 	meter_idx,len;
    INT8U 	idx;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

	if (read_params->meter_doc.protocol != GB645_2007)//���read_flag��־
	{
		clr_bit_value(read_meter_flag_day_freeze_event,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
	}
    if (get_bit_value(read_meter_flag_day_freeze_event,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) 
	{
		return FALSE;
    }
    if (check_is_all_ch(read_params->read_mask.day_freeze_event,READ_MASK_BYTE_NUM_DAY_FREEZE_EVENT,0x00))
    {
        clr_bit_value(read_meter_flag_day_freeze_event,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }

    if (check_is_all_ch(read_params->day_hold_td,3,0x00))
    {
        get_yesterday(read_params->day_hold_td);
        //get_former_month(read_params->month_hold_td);
    }

    for(idx=0;idx<5;idx++)
    {
        if(get_bit_value(read_params->read_mask.day_freeze_event,READ_MASK_BYTE_NUM_DAY_FREEZE_EVENT,idx))
        {
        	//��ȡ����		
            get_meter_event_info_and_mask(meter_idx,read_params,idx);
        	if(readdata_cycle_day_event(meter_idx,read_params->freeze_event_ctrl.freeze_data,read_params->day_hold_td,frame,frame+5,&len,FALSE) ==  FALSE)
        	{
        		//int32u2_bin(phy.phy,read_params->phy);
		        
	        	if(read_params->freeze_event_ctrl.item_ctrl.ctrl_flag.event_flag)//�����¼�����
	        	{
	        		item_32u = read_params->freeze_event_ctrl.freeze_data.item;
	        	}
				else
				{
					if(read_params->freeze_event_ctrl.item_ctrl.record_cnt)
					{
						item_32u = (read_params->freeze_event_ctrl.freeze_data.item &0xFFFFFFFE) | (read_params->freeze_event_ctrl.item_ctrl.read_record_ptr +1);
						if( ((item_32u & 0x1D000000)== 0x1D000000) || ((item_32u & 0x1E000000)== 0x1E000000) )
						{
							item_32u |= 0x00000100; 
						}
					}
					else
					{
						clr_bit_value(read_params->read_mask.day_freeze_event,READ_MASK_BYTE_NUM_DAY_FREEZE_EVENT,idx);	
					}
				}
				//int32u2_bin(read_params->freeze_event_ctrl.freeze_data.item,read_params->item);
				int32u2_bin(item_32u,read_params->item);
		        read_params->resp_byte_num = 40;
		        read_params->read_type = READ_TYPE_CYCLE_FREEZE_EVENT;
		        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,item_32u,NULL,0);
		        
		        #ifdef __SOFT_SIMULATOR__
		        snprintf(info,100,"*** prepare item day freeze event : meter_idx = %d , item = 0x%08X , mask_idx = %03d",
		                meter_idx,read_params->freeze_event_ctrl.freeze_data.item,idx);
		        debug_println_ext(info);
		        #endif

		        read_params->cur_mask_idx = idx;
		        return TRUE;
        	}
			clr_bit_value(read_params->read_mask.day_freeze_event,READ_MASK_BYTE_NUM_DAY_FREEZE_EVENT,idx);
        }
        
    }

    clr_bit_value(read_meter_flag_day_freeze_event,READ_FLAG_BYTE_NUM,meter_idx);

    return FALSE;
}

void save_freeze_event_mask_flag(READ_PARAMS *read_params)
{
	INT16U 	meter_idx;
	INT8U	plan_id;
	INT8U 	read_mask_meter_event_grade[PIM_READ_METER_GRADE_RECORD_MASK_METER_INFO_LEN];
	
	meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
	
	if(read_params->freeze_event_ctrl.item_ctrl.flag == 0xAA)
	{
		//д��
		#ifdef EVENT_GRADE_INFO
	    snprintf(info,150,"***����,���ڳ����¼�����������,��� = %12d,flag = 0x%02X,read_record_ptr = 0x%02X,***",meter_idx,
	            read_params->freeze_event_ctrl.item_ctrl.flag,read_params->freeze_event_ctrl.item_ctrl.read_record_ptr);
	    debug_println_ext(info);               
		#endif
		read_params->freeze_event_ctrl.item_ctrl.flag = 0x55;
		//fwrite_array(FILEID_EVENT_GRADE_READ_ITEM_CTRL,PIM_READ_EVENT_GRADE_READ_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&read_params->event_item_ctrl,sizeof(METER_EVENT_ITEM_CTRL));		
		fwrite_array(FILEID_FREEZE_EVENT_CTRL,PIM_FREEZE_EVENT_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&read_params->freeze_event_ctrl.item_ctrl,sizeof(FREEZE_EVENT_CTRL));
	}
	//�����㣬��ֹӰ���������
	mem_set(read_params->read_mask.day_freeze_event,READ_MASK_BYTE_NUM_CURVE_HUNAN,0x00);
	mem_set((INT8U *)&read_params->freeze_event_ctrl.item_ctrl,sizeof(FREEZE_EVENT_CTRL),0x00);
				
}
#endif
/*
   ���ݿ鳭��ʱ�������ظ�FF�����ݣ���Ҫ�޸ĳ�EE���������Ϊ��������û�г������������
*/
INT8U check_frame_blockFF_convert_EE(READ_WRITE_DATA *phy,INT8U *frame,INT8U datalen)
{
    if(phy->phy == 0x00001E3F)   //��ѹ��
    {
        if(datalen == 6)
        {
            if(check_is_all_ch(frame,2,0xFF))  mem_set(frame,2,0xEE);
            if(check_is_all_ch(frame+2,2,0xFF))  mem_set(frame+2,2,0xEE);
            if(check_is_all_ch(frame+4,2,0xFF))  mem_set(frame+4,2,0xEE);
        }
    }
    else if((phy->phy == 0x00001F7F) || (phy->phy == 0x00001FBF))  // �������������
    {
        if(datalen == 8)
        {
            if(check_is_all_ch(frame,2,0xFF))  mem_set(frame,2,0xEE);
            if(check_is_all_ch(frame+2,2,0xFF))  mem_set(frame+2,2,0xEE);
            if(check_is_all_ch(frame+4,2,0xFF))  mem_set(frame+4,2,0xEE);
            if(check_is_all_ch(frame+6,2,0xFF))  mem_set(frame+6,2,0xEE);
        }
    }
    else if(phy->phy == 0x00001E7F)    //����,
    {
        if(datalen == 9)
        {
            if(check_is_all_ch(frame,3,0xFF))  mem_set(frame,3,0xEE);
            if(check_is_all_ch(frame+3,3,0xFF))  mem_set(frame+3,3,0xEE);
            if(check_is_all_ch(frame+6,3,0xFF))  mem_set(frame+6,3,0xEE);
        }
    }
    else if((phy->phy == 0x00001EBF) || (phy->phy == 0x00001EFF))  //�й����ʡ��޹�����
    {
        if(datalen == 12)
        {
            if(check_is_all_ch(frame,3,0xFF))  mem_set(frame,3,0xEE);
            if(check_is_all_ch(frame+3,3,0xFF))  mem_set(frame+3,3,0xEE);
            if(check_is_all_ch(frame+6,3,0xFF))  mem_set(frame+6,3,0xEE);
            if(check_is_all_ch(frame+9,3,0xFF))  mem_set(frame+9,3,0xEE);
        }
    }
    else
  	{
        return datalen;
  	}

    return datalen;
}

//������¼������ڼ���������
BOOLEAN check_read_meter_event_cycle(INT16U meter_idx)
{
    INT32U offset;
    PARAM_F106 *f106;
    INT8U idx,idx1,idx2,plan_id;
    INT8U cycle[MAX_METER_EVENT_LEVEL*2];
    INT8U min_hour_event[MAX_METER_EVENT_LEVEL],cnt;

        mem_set(min_hour_event,MAX_METER_EVENT_LEVEL,0);
        cnt =0;
        plan_id = 0;

        fread_ertu_params(EEADDR_SET_F107,cycle,MAX_METER_EVENT_LEVEL*2);  //����F107���¼��ȼ��Ƿ���������

        for(idx=0;idx<MAX_METER_EVENT_LEVEL;idx++)
        {
           if((cycle[idx*2+1] == 1) || (cycle[idx*2+1] == 2)) //���ӻ���Сʱ
           {

             min_hour_event[cnt] = idx+1;
             cnt ++;
           }
        }
        if(cnt == 0) return FALSE;     //û������F107���ڣ�����

        fread_array( meter_idx,PIM_METER_F105,&plan_id,1);  //�����ò������Ӧ��F105�ּ����
        if( (plan_id < 1) || (plan_id > MAX_METER_EVENT_PLAN_COUNT) ) return FALSE;

        offset = sizeof(PARAM_F106) * MAX_METER_EVENT_ITEM_COUNT * (plan_id-1);
        offset += PIM_PARAM_F106;

        tpos_mutexPend(&SIGNAL_TEMP_BUFFER);  //ռ�ñȽϴ�����g_temp_buffer

        fread_array(FILEID_METER_EVENT_PARAM,offset,g_temp_buffer,sizeof(PARAM_F106) * MAX_METER_EVENT_ITEM_COUNT);//������Ӧ�ּ���ŵ����ݱ�ʶ���¼��ȼ�

        for(idx1=0;idx1<MAX_METER_EVENT_ITEM_COUNT;idx1++)
        {
          f106 = (PARAM_F106*)(g_temp_buffer+idx1*sizeof(PARAM_F106));
          if(f106->level > MAX_METER_EVENT_LEVEL) continue;

            for(idx2=0;idx2<cnt;idx2++)
            {
              if(f106->level == min_hour_event[idx2])  //���F106�����¼��ȼ���F107��Ӧ����Ҫ����True
              {
                #ifdef __DEBUG_RECINFO__
                snprintf(info,100,"****���ӻ���Сʱ���ڵ���¼���������!  idx = %d ",idx);
                println_info(info);
                #endif
                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                return TRUE;
              }
              else
              {

              }
            }
        }
        tpos_mutexFree(&SIGNAL_TEMP_BUFFER);

     return FALSE; //���F107��F106����Ӧ������
}
#ifdef __PROVICE_JIANGSU__
BOOLEAN prepare_plc_cjq_read_item(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
     JIANGSU_CJQ_INFO  *cjq_info;
     tagPROTOCOL_LIB library;
     INT8U cjq_info_num = 0;
     INT16U cjq_count,meter_idx;
     INT8U has_find,idx,idx1,idx2;
     INT8U need_read_time;

     if(read_params->meter_doc.baud_port.port != COMMPORT_PLC) return FALSE;

     cjq_info_num = (MAX_PAGE_SIZE-2)/sizeof(JIANGSU_CJQ_INFO); //һ�������������ٸ�
     has_find = 0;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    tpos_mutexPend(&SIGNAL_TEMP_BUFFER);

    fread_array(FILEID_CJQ_METER,0,g_temp_buffer,cjq_info_num*sizeof(JIANGSU_CJQ_INFO));
    cjq_count = bin2_int16u(g_temp_buffer);

    if(cjq_count>255)cjq_count = 0;
    need_read_time = cjq_count/cjq_info_num ;//��Ҫ���ٸ�ѭ�����ܽ��ļ��������в�������ѯһ�飬
    need_read_time ++ ;


    for(idx1=0;idx1 < need_read_time;idx1++)  //��һ��count��time˭��û��Ҫÿ�ζ���ѯ10��
    {
     fread_array(FILEID_CJQ_METER,2+idx1*cjq_info_num*sizeof(JIANGSU_CJQ_INFO),g_temp_buffer,(cjq_info_num*sizeof(JIANGSU_CJQ_INFO)));

     cjq_info = (JIANGSU_CJQ_INFO*)( g_temp_buffer );

      for(idx = 0;idx<cjq_info_num;idx++)
      {
        if(compare_string(cjq_info->cjq_num ,read_params->meter_doc.rtu_no,6) == 0)
        {
          if(compare_string(cjq_info->get_time+2,datetime+DAY,3) != 0)
          {
              mem_set(cjq_info->value,sizeof(JIANGSU_CJQ_INFO),0xFF);

              library.item = 0x04A00101;
              int32u2_bin(library.item,read_params->item);
              read_params->item_len = library.len;
              read_params->item_format = library.format;
              read_params->resp_byte_num = 40;
              read_params->read_type = READ_TYPE_CJQ_INFO;
                if(!check_is_sqr_protocol(read_params->meter_doc.protocol))
                {
                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                }
                else
                {
                    *frame_len=0;
                }
                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                return TRUE;
          }
          /*
          else
          {
            for(idx2 = 0;idx2<10;idx2++)
            {
              if(check_is_all_FF(cjq_info->power_off+idx2,10))
              {
                library.item = 0x05060201+idx2;
                int32u2_bin(library.item,read_params->item);
                read_params->item_len = library.len;
                read_params->item_format = library.format;
                read_params->resp_byte_num = 40;
                read_params->read_type = READ_TYPE_CJQ_INFO;


                *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);

               tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
               return TRUE;
              }
              else
              {
               tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
               return FALSE;
              }
            }
          }
          */
          has_find = TRUE;
          break;
        }
        else
        cjq_info ++;

     }
    }

    if(!has_find)
    {
              library.item = 0x04A00101;
              int32u2_bin(library.item,read_params->item);
              read_params->item_len = library.len;
              read_params->item_format = library.format;
              read_params->resp_byte_num = 40;
              read_params->read_type = READ_TYPE_CJQ_INFO;
                if(!check_is_sqr_protocol(read_params->meter_doc.protocol))
                {
                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,NULL,0);
                }
                else
                {
                    *frame_len=0;
                }
                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                return TRUE;
    }

    tpos_mutexFree(&SIGNAL_TEMP_BUFFER);


    return FALSE;
}

void save_cjq_info_data(READ_PARAMS *read_params,INT8U *data,INT8U datalen)
{
     JIANGSU_CJQ_INFO  *cjq_info;
     tagPROTOCOL_LIB library;
     INT8U cjq_info_num = 0;
     INT16U cjq_count,meter_idx;
     INT8U has_find,idx,idx1,idx2;
     INT8U need_read_time = 0;

     cjq_info_num = (MAX_PAGE_SIZE-2)/sizeof(JIANGSU_CJQ_INFO); //һ�ζ�ȡ�����Ĳɼ�������
     has_find = 0;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
    fread_array(FILEID_CJQ_METER,0,g_temp_buffer,cjq_info_num*sizeof(JIANGSU_CJQ_INFO));
    cjq_count = bin2_int16u(g_temp_buffer);

    if(cjq_count>255)cjq_count = 0;

    need_read_time = cjq_count/cjq_info_num ;//��Ҫ���ٸ�ѭ��������ѯһ�飬
    need_read_time ++ ;


    for(idx1=0;idx1 < need_read_time;idx1++)
    {
     fread_array(FILEID_CJQ_METER,2+idx1*cjq_info_num*sizeof(JIANGSU_CJQ_INFO),g_temp_buffer,(cjq_info_num*sizeof(JIANGSU_CJQ_INFO)));
     
     cjq_info = (JIANGSU_CJQ_INFO*)( g_temp_buffer );

      for(idx = 0;idx<cjq_info_num;idx++)
      {
        if(compare_string(cjq_info->cjq_num ,read_params->meter_doc.rtu_no,6) == 0) //����ɼ���֮ǰ�Ѿ����ȥ�ˣ�ֻ����ǰbuffer����
        {
          if(compare_string(cjq_info->get_time+2,datetime+DAY,3) != 0)
          {
              mem_set(cjq_info->value,sizeof(JIANGSU_CJQ_INFO),0xFF);
              mem_cpy(cjq_info->cjq_num,read_params->meter_doc.rtu_no,6);

              if(bin2_int32u(read_params->item) == 0x04A00101)  //���ɼ����汾
              {
                mem_cpy(cjq_info->vision,data+4,32);
                mem_cpy(cjq_info->get_time,datetime+MINUTE,5);
              }
              fwrite_array(FILEID_CJQ_METER,idx1*cjq_info_num*sizeof(JIANGSU_CJQ_INFO)+(idx)*sizeof(JIANGSU_CJQ_INFO)+2,cjq_info->value,sizeof(JIANGSU_CJQ_INFO));
             // fwrite_array(FILEID_CJQ_METER,idx1*cjq_info_num*sizeof(JIANGSU_CJQ_INFO),g_temp_buffer,cjq_info_num*sizeof(JIANGSU_CJQ_INFO)+2);
          }
          else
          {
            /*
            if((bin2_int32u(read_params->item) & 0x05060200) == 0x05060200)
            {
              idx2 = read_params->item[0]; //

                if(check_is_all_FF(cjq_info->power_off+idx2*10,10))
                {
                 mem_cpy(cjq_info->power_off+idx2*10,data+4,10);

                fwrite_array(FILEID_CJQ_METER,(cjq_count)*sizeof(JIANGSU_CJQ_INFO)+2,cjq_info->value,sizeof(JIANGSU_CJQ_INFO));

                }


            }
            */
          }
           has_find = TRUE;
           break;
        }
         else
         cjq_info ++;

      }
    }

    if(!has_find)
    {
      //û�ҵ���Ҫ��ӽ�ȥ
              mem_set(cjq_info->value,sizeof(JIANGSU_CJQ_INFO),0xFF);
              if(bin2_int32u(read_params->item) == 0x04A00101)
              {
                mem_cpy(cjq_info->cjq_num,read_params->meter_doc.rtu_no,6);
                mem_cpy(cjq_info->vision,data+4,32);
                mem_cpy(cjq_info->get_time,datetime+MINUTE,5);
                mem_cpy(cjq_info->cjq_num,read_params->meter_doc.rtu_no,6);
              }
              else //��������֡
              {
              //  mem_cpy(cjq_info->get_time,datetime+MINUTE,5);
              //  mem_cpy(cjq_info->cjq_num,read_params->meter_doc.rtu_no,6);
              }

              cjq_count++;
              int16u2_bin(cjq_count,g_temp_buffer);

              fwrite_array(FILEID_CJQ_METER,0,g_temp_buffer,2);   //�����и��£�д��

              fwrite_array(FILEID_CJQ_METER,(cjq_count-1)*sizeof(JIANGSU_CJQ_INFO)+2,cjq_info->value,sizeof(JIANGSU_CJQ_INFO));
    }

    tpos_mutexFree(&SIGNAL_TEMP_BUFFER);

}
#endif
#ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__

//#define __GW_CYCLE_TASK__
#ifdef __GW_CYCLE_TASK__

void check_sum_gb645(INT8U *frame)
{
    INT8U cs = 0;
    INT8U idx = 0;
    cs = 0;
    for(idx=0;idx<(frame[POS_GB645_DLEN]+10);idx++)
    {
        cs += frame[idx];
    }
    frame[frame[POS_GB645_DLEN]+10] = cs;

    return ;
}
void Modify_frame_addr(INT8U *frame,INT8U *addr)
{
    INT8U cs = 0;
    INT8U idx = 0;
    mem_cpy(frame+POS_GB645_METERNO,addr,6);
    return;
}
void Modify_load_data_td(INT8U *frame,READ_PARAMS *read_params,INT8U cycle_datetime[5])
{
    INT32U item;
    INT8U cs = 0;
    INT8U idx = 0;
    INT8U tmp_item[4] = {0};
    if(read_params->meter_doc.protocol != GB645_2007)
    {
        return;
    }
    for(idx=0;idx<4;idx++)
    {
        tmp_item[idx] = frame[POS_GB645_ITEM+idx]-0x33;
    }
    item = bin2_int32u(tmp_item);
    switch(item)
    {
        case 0x06010001:
        case 0x06020001:
        case 0x06030001:
        case 0x06040001:
        case 0x06050001:
        case 0x06060001:
            break;
        case 0x06100101:
        case 0x06100102:
        case 0x06100103:
        case 0x061001FF:
        
        case 0x06100201:
        case 0x06100202:
        case 0x06100203:
        case 0x061002FF:
        
        case 0x06100300:
        case 0x06100301:
        case 0x06100302:
        case 0x06100303:
        case 0x061003FF:
        
        case 0x06100400:
        case 0x06100401:
        case 0x06100402:
        case 0x06100403:
        case 0x061004FF:

        case 0x06100500:
        case 0x06100501:
        case 0x06100502:
        case 0x06100503:
        case 0x061005FF:

        case 0x06100601:
        case 0x06100602:
        case 0x06100603:
        case 0x06100604:
        case 0x061006FF:

        case 0x06100701:
        case 0x06100702:
        case 0x06100703:
        case 0x06100704:
        case 0x061007FF:

        case 0x06100801:
        case 0x06100802:
        case 0x061008FF:
            break;
        default:
            return;
    }
    /* ʱ���滻����Ҫ����������ʱ�� */
    for(idx=0;idx<5;idx++)
    {
        frame[POS_GB645_07_DATA+1+idx]= byte2BCD(cycle_datetime[idx]) + 0x33;
    }
    //mem_cpy(frame+POS_GB645_07_DATA+1,cycle_datetime,5);
    
}
void get_load_td(INT8U td[6],BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER task_header)
{
    INT32U secs_val = 0;
    INT8U unit = task_header.unit;
    INT8U intvl = task_header.frog;
    
    switch(unit)
    {
        case 1: /* �� */
            secs_val = intvl*60;
            datetime_minus_seconds(td+5,td+4,td+3,td+2,td+1,td+0,secs_val);
            break;
        case 2: /* ʱ */
            secs_val = intvl*60*60;
            datetime_minus_seconds(td+5,td+4,td+3,td+2,td+1,td+0,secs_val);
            break; 
        case 3: /* �� */
            date_minus_days(td+5,td+4,td+3,intvl);
            break;
        case 4: /* �� */
            date_minus_months(td+5,td+4,intvl);
            break;
        case 5: /* �� */
            return;
        default:
            return;
    }
    //datetime_minus_seconds();
}
void pre_process_frame(INT8U *frame,BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER task_header,READ_PARAMS *read_params,INT8U cycle_datetime[5],INT8U  protrol)
{
    INT16U opt = 0;
    INT16U len=0;
    INT8U  idx = 0;
    INT8U  td[6] = {0};
    opt = bin2_int16u(task_header.rule_opt);
    for(idx=0;idx<16;idx++)
    {
        if(get_bit_value(task_header.rule_opt,2,idx))
        {
            switch(idx)
            {
                case 0:
                    if(protrol == GB_OOP)
                    {
                        len = get_frame_length_gb_oop(frame);
                        Modify_frame_addr(frame+4,read_params->meter_doc.meter_no);

                        //����HCSУ��λ
                        fcs16(frame+1,11);
                        //����FCSУ��λ
                        fcs16(frame+1,len-1);
                    }
                    else
                    {
                        Modify_frame_addr(frame,read_params->meter_doc.meter_no);
                        check_sum_gb645(frame);
                    }
                    break;
                case 1:
                    mem_cpy(td+1,cycle_datetime,5);
                    get_load_td(td,task_header);
                    Modify_load_data_td(frame,read_params,td+1);
                    check_sum_gb645(frame);
                    break;
                default:
                    break;
            }
            
        }
    }
    
}

/* begin_time ��BCD��ʽ��  
 *
 */
BOOLEAN get_gw_cycle_datetime(INT8U* value,INT8U begin_time[5],INT8U unit,INT8U frog)
{
	INT8U bg_td[5] = {0};
	INT8U idx = 0;

	for(idx=0;idx<5;idx++)
	{
	    bg_td[idx] = BCD2byte(begin_time[idx]);
	}
    mem_cpy(value,(INT8U*)(datetime+MINUTE),5);
    switch(unit)
    {
    case 1: //��
        value[0] = (value[0]-bg_td[0])/ frog * frog;
        value[0] += bg_td[0];
        break;
    case 2: //ʱ
        value[0] = 0;
        value[1] = (value[1]-bg_td[1]) / frog * frog;
        value[1] += bg_td[1];
        break;
    case 3: //��
        value[0] = 0;
        value[1] = 0;
        value[2] = (value[2] - bg_td[2]) / frog * frog;
        value[2] += bg_td[2];
        break;
    case 4: //��
        value[0] = 0;
        value[1] = 0;
        value[2] = 1;
        value[3] = (value[3] - bg_td[3]) / frog * frog;
        value[3] += bg_td[3];
        break;
    case 5: //��
        value[0] = 0;
        value[1] = 0;
        value[2] = 1;
        value[3] = 1;
        value[4] = (value[4] - bg_td[4]) / frog * frog;
        value[4] += bg_td[4];
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
#endif
void check_batch_transparent_meter_cycle_task(void)
{
    INT16U hm;
    static INT8U last_minute = 0xFF;
    static INT8U last_day = 0xFF;

    if ((last_minute == datetime[MINUTE]) && (last_day == datetime[DAY])) return;
    last_minute = datetime[MINUTE];

    hm = datetime[HOUR]*60UL + datetime[MINUTE];

    if ((hm == 0) || (last_day != datetime[DAY]))
    {
        last_day = datetime[DAY];
        file_delete(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MINUTE);
        file_delete(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_HOUR);
        file_delete(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_DAY);
        file_delete(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MONTH);
        file_delete(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_YEAR);
    }
    else if ((hm % 60) == 0)
    {
        file_delete(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MINUTE);
        file_delete(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_HOUR);
    }
    else if ((hm % 5) == 0)
    {
        file_delete(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MINUTE);
    }
}

BOOLEAN get_cycle_datetime(INT8U* value,INT8U unit,INT8U frog)
{
    mem_cpy(value,(INT8U*)(datetime+MINUTE),5);
    switch(unit)
    {
    case 1: //��
        value[0] = value[0] / frog * frog;
        break;
    case 2: //ʱ
        value[0] = 0;
        value[1] = value[1] / frog * frog;
        break;
    case 3: //��
        value[0] = 0;
        value[1] = 0;
        value[2] = (value[2] - 1) / frog * frog + 1;
        break;
    case 4: //��
        value[0] = 0;
        value[1] = 0;
        value[2] = 1;
        value[3] = (value[3] - 1) / frog * frog + 1;
        break;
    case 5: //��
        value[0] = 0;
        value[1] = 0;
        value[2] = 1;
        value[3] = 1;
        value[4] = value[4] / frog * frog;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

INT8U get_cycle_task_save_idx(INT8U* value,INT8U unit,INT8U frog)
{
    INT32U idx;
    INT8U num;  

    if (frog == 0) return 0xFF;

    idx = getPassedDays(2000+value[4],value[3],value[2]);

    switch(unit)
    {
    case 1: //��
        num = 60 / frog;
        if ((24 % frog) > 0) num++;
        idx *= 24;
        idx += value[1];
        idx *= num;
        idx += value[0] / frog;
        idx %= BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP_FRAME_MAX_COUNT;
        break;
    case 2: //ʱ
        num = 24 / frog;
        if ((24 % frog) > 0) num++;
        idx *= num;
        idx += value[1] / frog;
        idx %= BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP_FRAME_MAX_COUNT;
        break;
    case 3: //��
        idx = (2000+value[4]-1900)*12;
        num = 31 / frog;
        if ((31 % frog) > 0) num++;
        idx *= num;
        idx += value[3] * num;
        idx += value[2] / frog;
        idx %= BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP_FRAME_MAX_COUNT;
        break;
    case 4: //��
        idx = (2000+value[4]-1900);
        num = 12 / frog;
        if ((12 % frog) > 0) num++;
        idx *= num;
        idx += value[3] / frog;
        idx %= BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP_FRAME_MAX_COUNT;
        break;
    case 5: //��
        idx = value[4] / frog;
        idx %= BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP_FRAME_MAX_COUNT;
        break;
    default:
        return 0xFF;
    }
    return idx;
}

BOOLEAN prepare_batch_transparent_meter_cycle_task(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    BOOLEAN check_cmd_is_GB645(INT8U *frame,BOOLEAN is_update_cy_meterNo_cs);
    INT32U offset,offset_resp,item;
    INT16U meter_idx;
    #ifdef __GW_CYCLE_TASK__
    INT16U fileid = 0;
    #endif
    CommandDate cmdDate_begin,cmdDate_end,cmdDate_cur;
    INT8U valid_flag[MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM];
    INT8U read_flag[MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM];
    INT8U hm_flag[MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM];
    INT8U cycle_datetime[5],tmp[5],idx_tmp;
    INT8U idx,save_idx;
    INT8U idx_td;
    BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER task_header;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_VALID,PIM_METER_CYCLE_TASK_VAILD_FLAG+meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
    bit_value_opt_inversion(valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);

    fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_VALID,PIM_METER_CYCLE_TASK_HM_FLAG+meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
    bit_value_opt_inversion(hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
    if (check_is_all_ch(hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,0x00) == FALSE)
    {
        read_params->batch_ctrl.is_set_node_fail = 1;
        #ifdef __PROVICE_JIBEI__
        /* Сʱ����������ڣ�������� ����ʧ�� �����걸 
         * �ȼ򵥴���������Ҫ�洢��С�ܶ� 
         * �����Ҳ��ʧ�ܷ�ʽ�� 
         */
        if( (read_params->meter_doc.meter_class.meter_class == 0X02)
        || (read_params->meter_doc.meter_class.user_class == 0x0F) )
        #endif
        {
            read_params->control.loop = 1;
        }
    }

    if (check_is_all_ch(valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,0)) return FALSE;

    tpos_mutexPend(&SIGNAL_BATCH_CYCLE);

    for(idx=0;idx<BATCH_TRANSPARENT_METER_CYCLE_TASK_FRAME_MAX_COUNT;idx++)
    {
        if (get_bit_value(valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx))
        {
            #ifndef __GW_CYCLE_TASK__
            offset = idx * sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_CMD);
            offset += PIM_BATCH_TRANSPARENT_METER_CYCLE_TASK;
            fread_array(meter_idx,offset,task_header.value,sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER));
            if (task_header.task_flag != 0x55) continue;
            #else
            offset = idx * sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_CMD);
            offset += (meter_idx-1)*PIM_METER_CYCLE_TASK_MSG_LEN;
            fileid = FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK;
            fileid += (meter_idx-1)/512;
            fread_array(fileid,offset,task_header.value,sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER));
            if (task_header.task_flag != 0x55) continue;
            
            /* 
             * �����ն��� ��ʱֻ֧�ֵ��ں͸����ն�����ж� ��ȫ�¼����ȼ��ݲ����� 
             */
            if (task_header.priority < 2)
            {
                if (get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx))
                {
                    continue;
                }
            }
            #endif
            switch(task_header.unit)
            {
            case 1: //��
                fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MINUTE,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                break;
            case 2: //ʱ
                fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_HOUR,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                break;
            case 3: //��
                fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_DAY,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                break;
            case 4: //��
                fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MONTH,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                break;
            case 5: //��
                fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_YEAR,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                break;
            default:
                continue;
            }
            if (get_bit_value(read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx) == FALSE) continue;
            #ifdef __GW_CYCLE_TASK__
            for(idx_td=0;idx_td<3;idx_td++) /* �� �� �� ��ͳ��ʱ  ������ */
            {
            	if(0 == task_header.begin_time[2+idx_td])
            	{
            	    task_header.begin_time[2+idx_td] = byte2BCD(datetime[DAY+idx_td]);
            	}
            	if(0 == task_header.end_time[2+idx_td])
            	{
            	    task_header.end_time[2+idx_td]   = byte2BCD(datetime[DAY+idx_td]);
            	}
            }
            #endif
            setCommandBCDDate(&cmdDate_begin,task_header.begin_time);
            setCommandBCDDate(&cmdDate_end,task_header.end_time);
            setCommandDate(&cmdDate_cur,(INT8U*)datetime+MINUTE);
            #ifdef __GW_CYCLE_TASK__
            if ((check_is_all_ch(task_header.begin_time,5,0x99) == FALSE) && (commandDateCompare(&cmdDate_cur,&cmdDate_begin) < 0)) continue;
            if ((check_is_all_ch(task_header.end_time,5,0x99) == FALSE) && (commandDateCompare(&cmdDate_cur,&cmdDate_end) > 0)) continue;
            if (get_gw_cycle_datetime(cycle_datetime,task_header.begin_time,task_header.unit,task_header.frog) == FALSE) continue;
            #else
            if ((check_is_all_ch(task_header.begin_time,5,0x00) == FALSE) && (commandDateCompare(&cmdDate_cur,&cmdDate_begin) < 0)) continue;
            if ((check_is_all_ch(task_header.end_time,5,0x00) == FALSE) && (commandDateCompare(&cmdDate_cur,&cmdDate_end) > 0)) continue;
            if (get_cycle_datetime(cycle_datetime,task_header.unit,task_header.frog) == FALSE) continue;
            #endif

            save_idx = get_cycle_task_save_idx(cycle_datetime,task_header.unit,task_header.frog);
            if (save_idx > BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP_FRAME_MAX_COUNT) continue;
            offset_resp = PIM_BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP + idx * ONE_CYCLE_TASK_RESP_MAX_LEN;
            offset_resp += save_idx * sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_REPS);
            fread_array(meter_idx,offset_resp,tmp,5);

            for(idx_tmp=0;idx_tmp<5;idx_tmp++) tmp[idx_tmp] = BCD2byte(tmp[idx_tmp]);

            /* ��ȡ���� ���ݲɼ�����ʱ��͵�ǰ����ʱ�� ��ͬ ������  */
            if (compare_string(tmp,cycle_datetime,5) == 0) continue;

            *frame_len = task_header.req_frame_len;
            #ifdef __GW_CYCLE_TASK__
            fread_array(fileid,offset+sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER),frame,task_header.req_frame_len);
            #else
            fread_array(meter_idx,offset+sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER),frame,task_header.req_frame_len);
            #endif
            if((read_params->meter_doc.protocol == GB645_2007) || (read_params->meter_doc.protocol == GB645_1997))
            {
                if (check_cmd_is_GB645(frame,FALSE))
                {
                #ifdef __GW_CYCLE_TASK__
                /* ��ҪԤ���������� */
                    pre_process_frame(frame,task_header,read_params,cycle_datetime,read_params->meter_doc.protocol);
                #endif
                if (compare_string(frame+1,read_params->meter_doc.meter_no,6) != 0)
                {
                    task_header.task_flag = 0x00;
                    fwrite_array(meter_idx,offset,task_header.value,sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER));

                    clr_bit_value(valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
                    bit_value_opt_inversion(valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                    fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_VALID,PIM_METER_CYCLE_TASK_VAILD_FLAG+meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);

                    clr_bit_value(hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
                    bit_value_opt_inversion(hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                    fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_VALID,PIM_METER_CYCLE_TASK_HM_FLAG+meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);

                    continue;
                }
                }
                else
                {
                   continue;
                }
            }
            else if(read_params->meter_doc.protocol == GB_OOP)
            {
                if(check_frame_body_gb_oop(frame,*frame_len))
                {
                    #ifdef __GW_CYCLE_TASK__
                    /* ��ҪԤ���������� */
                    pre_process_frame(frame,task_header,read_params,cycle_datetime,read_params->meter_doc.protocol);
                    #endif
                    if (compare_string(frame+5,read_params->meter_doc.meter_no,6) != 0)
                    {
                        task_header.task_flag = 0x00;
                        fwrite_array(meter_idx,offset,task_header.value,sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_HEADER));

                        clr_bit_value(valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
                        bit_value_opt_inversion(valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                        fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_VALID,PIM_METER_CYCLE_TASK_VAILD_FLAG+meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,valid_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);

                        clr_bit_value(hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
                        bit_value_opt_inversion(hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
                        fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_VALID,PIM_METER_CYCLE_TASK_HM_FLAG+meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,hm_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);

                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;  /*Э�鲻ʶ��ģ���Ҫִ��*/
            }

            item = 0xBB000000 + (save_idx<<8) + idx; /* Ψһ�� ?? */
            int32u2_bin(item,read_params->item);
            read_params->read_type = READ_TYPE_BATCH_TRANSPARENT_METER_CYCLE_TASK;

            mem_cpy(read_params->cycle_datetime,cycle_datetime,5);
            read_params->cycle_unit = task_header.unit;
            read_params->cycle_task_id = task_header.task_id;
            read_params->resp_byte_num = task_header.resp_byte_num;
            read_params->cycle_task_report_flag = task_header.report_flag;
            tpos_mutexFree(&SIGNAL_BATCH_CYCLE);

            return TRUE;
        }
    }

    tpos_mutexFree(&SIGNAL_BATCH_CYCLE);
    
    return FALSE;
}




//�洢��Ҫ�ϱ����������ݱ�־
void  save_trans_meter_report_data(CYCLE_TASK_DATA_REPORT_RETRIEVE *task_data)
{
    INT32U read_pos=0,write_pos=0;

    fread_array(FILEID_BATCH_TRANSPARENT_TASK_REPORT,PIM_TRANSPARENT_TASK_REPORT_WRITE,(INT8U*)&write_pos,4);
    if(write_pos == 0xFFFFFFFF)
    {
        write_pos = PIM_TRANSPARENT_TASK_DATA;
    }
    fwrite_array(FILEID_BATCH_TRANSPARENT_TASK_REPORT,write_pos,task_data->value,sizeof(CYCLE_TASK_DATA_REPORT_RETRIEVE));

    write_pos += sizeof(CYCLE_TASK_DATA_REPORT_RETRIEVE);
    //ѭ��д��Ҫ�ϱ�������Ϣ
    if(write_pos >= PIM_TRANSPARENT_TASK_DATA_END)
    {
        write_pos = PIM_TRANSPARENT_TASK_DATA;
    }
    fwrite_array(FILEID_BATCH_TRANSPARENT_TASK_REPORT,PIM_TRANSPARENT_TASK_REPORT_WRITE,(INT8U*)&write_pos,4);
    //rs232_debug_info("\xC1",1);
    //rs232_debug_info(&write_pos,4);
}

//��ȡ��Ҫ�ϱ����������ݱ�־
BOOLEAN  read_trans_meter_report_data(CYCLE_TASK_DATA_REPORT_RETRIEVE *task_data,INT32U *read_pos)
{
    INT32U write_pos=0;

    fread_array(FILEID_BATCH_TRANSPARENT_TASK_REPORT,PIM_TRANSPARENT_TASK_REPORT_WRITE,(INT8U*)&write_pos,4);

    if(*read_pos==write_pos)
    {
        return FALSE;
    }
    if(*read_pos == 0xFFFFFFFF)
    {
        *read_pos = PIM_TRANSPARENT_TASK_DATA;
    }
    fread_array(FILEID_BATCH_TRANSPARENT_TASK_REPORT,*read_pos,task_data->value,sizeof(CYCLE_TASK_DATA_REPORT_RETRIEVE));
    //rs232_debug_info("\xC2",1);
    //rs232_debug_info(read_pos,4);
    return TRUE;
}


void save_batch_transparent_meter_cycle_task(READ_PARAMS *read_params,INT8U *frame)
{
    INT32U offset,item;
    INT16U framelen,meter_idx;
    INT8U idx,save_idx;
    INT8U tmp[15],pos;
    INT8U read_flag[MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM];
    CYCLE_TASK_DATA_REPORT_RETRIEVE task_data;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    item = bin2_int32u(read_params->item);
    idx = item;
    save_idx = item>>8;
    if (idx > BATCH_TRANSPARENT_METER_CYCLE_TASK_FRAME_MAX_COUNT) return;
    if (save_idx > BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP_FRAME_MAX_COUNT) return;

    switch(read_params->cycle_unit)
    {
    case 1: //��
        fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MINUTE,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        clr_bit_value(read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
        fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MINUTE,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        break;
    case 2: //ʱ
        fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_HOUR,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        clr_bit_value(read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
        fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_HOUR,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        break;
    case 3: //��
        fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_DAY,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        clr_bit_value(read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
        fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_DAY,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        break;
    case 4: //��
        fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MONTH,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        clr_bit_value(read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
        fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_MONTH,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        break;
    case 5: //��
        fread_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_YEAR,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        clr_bit_value(read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,idx);
        fwrite_array(FILEID_BATCH_TRANSPARENT_METER_CYCLE_TASK_YEAR,meter_idx*MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM,read_flag,MAX_BATCH_METER_CYCLE_TASK_BYTE_NUM);
        break;
    default:
        return;
    }
    if(read_params->meter_doc.protocol == GB_OOP)
    {
       framelen = get_frame_length_gb_oop(frame);
    }
    else
    {
        framelen = frame[POS_GB645_DLEN]+12;
    }
    if(framelen > BATCH_TRANSPARENT_METER_CYCLE_TASK_FRAME_LEN) return;

    offset = PIM_BATCH_TRANSPARENT_METER_CYCLE_TASK_RESP + idx * ONE_CYCLE_TASK_RESP_MAX_LEN;
    offset += save_idx * sizeof(BATCH_TRANSPARENT_METER_CYCLE_TASK_REPS);
    pos = 0;
    tmp[pos++] = byte2BCD(read_params->cycle_datetime[0]);
    tmp[pos++] = byte2BCD(read_params->cycle_datetime[1]);
    tmp[pos++] = byte2BCD(read_params->cycle_datetime[2]);
    tmp[pos++] = byte2BCD(read_params->cycle_datetime[3]);
    tmp[pos++] = byte2BCD(read_params->cycle_datetime[4]);
    #ifdef __GW_CYCLE_TASK__
    tmp[pos++] = 0; /* �ϱ���־ ��ʱ��չ ���������� */
    #endif
    tmp[pos++] = framelen+5;
    tmp[pos++] = byte2BCD(datetime[MINUTE]);
    tmp[pos++] = byte2BCD(datetime[HOUR]);
    tmp[pos++] = byte2BCD(datetime[DAY]);
    tmp[pos++] = byte2BCD(datetime[MONTH]);
    tmp[pos++] = byte2BCD(datetime[YEAR]);

    fwrite_array(meter_idx,offset,tmp,pos);
    fwrite_array(meter_idx,offset+pos,frame,framelen);
    #ifdef __GW_CYCLE_TASK__
    //�������ϱ���������
    task_data.task_id =  read_params->cycle_task_id;//�����id��
    #ifdef __PROVICE_HUBEI__
    if(read_params->cycle_task_report_flag != 0)
    {
        task_data.rpt_flag = 1;
    }
    else
    {
        task_data.rpt_flag = 0;
    }
    #else
    task_data.rpt_flag = 0x01;
    #endif
    mem_cpy(task_data.meter_idx,read_params->meter_doc.meter_idx,2);
    mem_cpy(task_data.data_offset,&offset,4);
    save_trans_meter_report_data((CYCLE_TASK_DATA_REPORT_RETRIEVE *)&task_data);
   // #endif
    #endif
}
#endif //#ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__

/***************************************************************************
 * ���ܣ�
 *     �����ɼ�������״̬
 * ������
 *     �㲥�ɼ���������ɺ�ͨ����Ե��ȡ�ɼ�������Ϣ��ȷ���Ƿ������ɹ����Ƿ���Ҫ���������ļ���
 * ������
 *     READ_PARAMS *read_params
 *     INT8U* frame
 *     INT8U* frame_len
 *     BOOLEAN is_high
 **************************************************************************/
BOOLEAN prepare_read_cjq_update_task(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len,BOOLEAN is_high)
{
    //INT32U offset;
    INT16U idx,meter_idx;
    INT16U count;
    CJQ_UPDATE_CTRL cjq_update_ctrl;
    CJQ_UPDATE_METER_CTRL cjq_update_meter_ctrl;
    INT8U rtu_no[6]={0};
    BOOLEAN is_find = FALSE;  //���ڱ�ʾ�Ƿ��ҵ��ɼ�����ַ
    INT8U cycle = 1;  //���ڼ�¼���Ҳɼ�����ַ��Ȧ��

    if (isvalid_meter_addr(read_params->meter_doc.rtu_no,FALSE) ==  FALSE) return FALSE;
    fread_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    if (cjq_update_ctrl.flag != 0xCC) return FALSE;
    if (cjq_update_ctrl.day_count >= g_cjq_update_data.cjq_update_day_count)
    {
        cjq_update_ctrl.flag = 0x00;
        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
        file_delete(FILEID_UPDATE_CJQ);//ɾ���ɼ��������ļ�
        return FALSE;
    }
    fread_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
AGAIN:
    if (cjq_update_meter_ctrl.readflag & 0x01) //�����汾��Ϣ
    {
        read_params->plan_type = 222;  //�Զ���
        read_params->resp_byte_num = 40;

        //0x04A00102��������չ�Ķ�ȡ�ɼ����ĳ�������汾
        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,0x04A00102,NULL,0);
        return TRUE;
    }
    else if (cjq_update_meter_ctrl.readflag & 0x02) //�ۼҵĿ��·�ɣ��˴������·���Ե���������ļ����
    {
        fread_array(FILEID_CJQ_METER_TEMP,PIM_CJQ_UPDATE_COUNT,(INT8U *)&count,2);
        //���ɼ�����ַ
        if((count <= MAX_METER_COUNT) && (count != 0))
        {
            tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
            //һ���Զ�ȡ600���ɼ�����ַ��Ϣ
CHECK_AGAIN:
            fread_array(FILEID_CJQ_METER_TEMP,PIM_CJQ_UPDATE_METER_INFO+(cycle-1)*3600,g_temp_buffer,3600);
            for(idx = 0;idx < 600;idx++)
            {
                mem_cpy(rtu_no,g_temp_buffer+idx*6,6);
                if(compare_string(rtu_no,read_params->meter_doc.rtu_no,6) == 0)
                {
                    is_find = TRUE;  //��ʾ�ҵ��˶�Ӧ�Ĳɼ�����ַ����ô��������������ļ��Ĵ���
                    break;
                }
                else
                {
                    if((idx + 1 + (cycle - 1) * 600) == count) break;
                    else continue;
                }
            }
            if((idx == 600) && ((cycle * 600 ) < count))
            {
                //��ʱ��ʾһȦ������û���ҵ���Ӧ�Ĳɼ�����ַ���Ҳɼ�����������һȦ������(600)����ô��������
                cycle++;
                goto CHECK_AGAIN;                
            }
            tpos_mutexFree(&SIGNAL_TEMP_BUFFER);    
        }
        
        //���ſ��·����Ϊ��֧�ֹ㲥�������޷�ʹ�ù㲥�ɼ����������˴�����Ϊ����Ե��·�����ļ���������
        //�ɼ����յ�����ļ���������󣬺�����Ӧ�Ĵ����״̬�֣�����ȫ��δ����ɹ����ں����Ϳ��������·��������ģ�ʵ�ֵ�Ե�������
        if ((portContext_plc.router_base_info.router_info1.comm_mode == 2) && (portContext_plc.router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM) && (FALSE == is_find))
        {
            read_params->plan_type = 222;  //�Զ���
            read_params->resp_byte_num = 40;
            //0x04A01101���ļ���ʶ0����������ļ�
            *frame_len = make_cjq_update_file_frame(frame,read_params->meter_doc.meter_no,0x04A01101,&cjq_update_ctrl,0,TRUE);
            return TRUE;
        }
        else
        {
            cjq_update_meter_ctrl.readflag &= ~0x02;
            fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
            goto AGAIN;
        }
    }
    //��������ļ��ı�־��û�������block_status��1����ȥ������״̬�������������������ļ���־�Ѿ��������־�Ѿ��·��ļ���������ȥ������״̬��
    else if ((cjq_update_meter_ctrl.readflag & 0x04) || ((read_params->block_status == 1) && (cjq_update_meter_ctrl.readflag & 0x10)))//���������״̬��
    {
        read_params->plan_type = 222;  //�Զ���
        read_params->resp_byte_num = 160;

        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,0x04A01102,NULL,0);
        return TRUE;
    }
//    else if (cjq_update_meter_ctrl.readflag & 0x08) //�������䣨��֧�֣�
//    {
//        read_params->plan_type = 222;  //�Զ���
//        read_params->resp_byte_num = 40;
//
//        *frame_len = make_oop_cjq_start_update_frame(frame,read_params->meter_doc.rtu_no,&cjq_update_ctrl);
//        return TRUE;
//    }
    else if (cjq_update_meter_ctrl.readflag & 0x10) //���������ض�Ӧ����ļ����䣩
    {
        read_params->plan_type = 222;  //�Զ���
        read_params->resp_byte_num = 160;

        for(idx=0;idx<bin2_int16u(cjq_update_ctrl.block_count);idx++)
        {
            if (get_bit_value(cjq_update_meter_ctrl.block,128,idx))
            {
                //0x04A01101��������չ�Ĳɼ���Ӧ�ó�������
                *frame_len = make_cjq_update_file_frame(frame,read_params->meter_doc.meter_no,0x04A01101,&cjq_update_ctrl,idx,FALSE);
                return TRUE;
            }
        }
        cjq_update_meter_ctrl.readflag &= ~0x10;
        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
        goto AGAIN;
    }
    else if (cjq_update_meter_ctrl.readflag & 0x20) //�����汾��Ϣ
    {
        read_params->plan_type = 222;  //�Զ���
        read_params->resp_byte_num = 40;

        //0x04A00102��������չ�Ķ�ȡ�ɼ����ĳ�������汾
        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,0x04A00102,NULL,0);
        return TRUE;
    }
    return FALSE;
}

#ifdef __FUJIAN_SUPPLEMENT_SPECIFICATION__

//const 
#include "fun/tops_fun_date.h"
pfun_diff_time pfun_get_diff_time[6]={diff_sec_between_dt,diff_min_between_dt,diff_hour_between_dt,
	                                        diff_day_between_dt,diff_month_between_dt,diff_year_between_dt};
void save_dayhold_zxyg(INT8U *frame)
{
    READ_WRITE_DATA phy;
    INT32U  item_32u = 0;
	INT16U	pos = 0;
	INT16U  meter_idx = 0;
	METER_DOCUMENT meter_doc;
    INT8U	item[4];
	INT8U	buffer[300];
	INT8U	td[3];
	INT8U   idx = 0;   
	INT8U	len = 0;
	INT8U   FE_cnt = 0;
	INT8U   frame_len = 0;
	INT8U	router_protocol = 0;
	INT8U   user_class = 0;
	
    //����Ҫ�浽F161������ TODO?
    len = frame[24];
	for(idx=0;idx<len;idx++)
	{
		if(frame[25+idx] == 0xFE)
		{
			FE_cnt++;
		}
		else
		{
		    break;
		}
	}
    mem_cpy(item,frame+25+FE_cnt+POS_GB645_ITEM,4);
	frame_len = frame[25+FE_cnt+POS_GB645_DLEN];
    for(idx=0;idx<4;idx++)
    {
        item[idx] = item[idx]-0x33;
    }
    item_32u = bin2_int32u(item);
    if( (0x05060101 == item_32u) || (0x00009010 == (item_32u&0x0000FFFF)) ) 
    {
        memory_fast_index_find_node_no(READPORT_PLC,frame+25+FE_cnt+POS_GB645_METERNO,&meter_idx,&pos,&router_protocol,&user_class);
		meter_idx &= FAST_IDX_MASK;//
		if(!file_exist(meter_idx)) 
		{
		    return;
		}
		fread_array(meter_idx,PIM_METER_DOC,meter_doc.value,sizeof(METER_DOCUMENT));
		if(bin2_int16u(meter_doc.meter_idx) == meter_idx)
		{
			if(GB645_2007 == meter_doc.protocol )
			{
				pos = POS_GB645_07_DATA;
				frame_len -= 4;
			}
			else if(GB645_1997 == meter_doc.protocol)
			{
				pos = POS_GB645_97_DATA;
				frame_len -= 2;
			}
			else
			{
			    return;
			}
			//��ʱֻ�ж�������������
            idx = get_phy_form_list_cycle_day(0x00002C7F,&phy);
            if(idx != 0xFF) 
            {
                mem_cpy(td,portContext_plc.fujian_ctrl.cur_DA_TD+DAY,3);
                date_minus_days(td+2,td+1,td,1);
				for(idx=0;idx<frame_len;idx++)
				{
					frame[25+FE_cnt+pos+idx] -= 0x33;
				}
                writedata_cycle_day(meter_idx,&phy,td,frame+25+FE_cnt+pos,frame_len,buffer,NULL,0xFF);
            }
		}
        
    
    }
}

#if (defined __PLAN_TASK_MULTIFRAME_CTRL__)
void clear_mask_write_report_flag(READ_CTRL_WORK_INFO *read_ctrl_work_info,/* READ_PARAMS *read_params, */TASK_PLAN_EXEC_RESULT *task_plan_result)
{
    INT32U offset = 0;
    INT16U idx;
    INT16U file_flag_id = 0;
    INT16U  success_cnt = 0;
    INT8U  task_state = 0;
    INT8U  bit_pos = 0;
    INT8U  temp_obj_task[256] = {0};
    BOOLEAN flag = FALSE;
    
    file_flag_id = bin2_int16u(read_ctrl_work_info->file_flag_id);
    if(check_is_all_ch(read_ctrl_work_info->obj_task,256,0x00) )
    {
        flag = TRUE;
    }

    /* �ع���ȡ�������  */
    get_read_mask_task_seq(temp_obj_task,read_ctrl_work_info->obj_pos);

    
    for(idx=0;idx<2048;idx++)
    {
        if(get_bit_value(temp_obj_task,256,idx) )
        {           
            //��������־
            if(TRUE == flag)
            {
                offset = (read_ctrl_work_info->cur_plan_save_idx)*PIM_PLAN_TASK_STATE_PER_PLAN_SIZE;
                offset += PIM_PLAN_TASK_STATE_READ_STATE;  
        		offset += idx/8;
        		bit_pos = idx%8;
        		fread_array(FILEID_PLAN_TASK_STATE,offset,&task_state,1);
        		clr_bit_value(&task_state,1,bit_pos);
        		fwrite_array(FILEID_PLAN_TASK_STATE,offset,&task_state,1);
                #ifdef __SOFT_SIMULATOR__       
                snprintf(info,100,"*****��֡�����룬�������= %03d,ƫ�� = %08d,bit_pos = %02d ",(idx+1),offset,bit_pos );
                debug_println_ext(info);                       
                #endif
                success_cnt++;
            }

            // obj_task ������ģ����� tmp_obj_task û����� ˵�����������
            if(0 == get_bit_value(read_ctrl_work_info->obj_task,256,idx) )
            {
        		//�ϱ���־
        		offset = (MAX_PAGE_SIZE*512);
    			offset += read_ctrl_work_info->data_save_cnt*idx;
    			offset += read_ctrl_work_info->cur_rpt_flag_save_pos;
    			task_state = 0xAA;
    			fwrite_array(file_flag_id,offset,&task_state,1);
    			#ifdef __SOFT_SIMULATOR__       
                snprintf(info,100,"*****��֡д�ϱ���־AA��ƫ�� = %08d ",offset );
                debug_println_ext(info);                       
                #endif
            }
        }
    }
    /*��֡ 2018-03-22 �˱�Ƴ�����ɺ�һ����ͳ�Ƴ����ɹ������� */
    if(TRUE == flag)
    {
        success_cnt += bin2_int16u(task_plan_result->success_task_cnt);
        int16u2_bin(success_cnt, task_plan_result->success_task_cnt);
    }
}
#endif
/*
����ʧ�ܶ��������ʣ����������ݣ���д��AA��Ȼ��ȴ��ϱ�


*/
void save_fail_obj_reserve_task_data(READ_CTRL_WORK_INFO *read_ctrl_work_info, /*READ_PARAMS *read_params,*/ INT8U DA_time[6],INT8U mask[256],INT8U report_condition)
{
    INT32U offset = 0;
    INT32U diff_time = 0;
    INT16U file_id = 0;
    INT16U file_flag = 0;//�洢�ϱ���־��λ�� 
    INT16U idx_mask = 0;
    INT16U idx = 0;
    INT16U task_seq = 0;
    C4_F1_FAIL fail_report;
    TASK_PLAN_INFO task_plan_info;
    TASK_PLAN  plan_info;
    PLAN_TASK_SAVE_CTRL save_ctrl;
    INT8U  flag = 0;
    INT8U  unit = 0;
    INT8U  dt[6] = {0};
    INT8U  jzq_dt[6] = {0};
    INT8U  cnt = 0;
    INT8U  save_flag_idx = 0;
    INT8U  bit_pos = 0;
    INT8U  task_state = 0;
    BOOLEAN find_file_flag = FALSE;
    
    mem_set(task_plan_info.value,sizeof(TASK_PLAN_INFO),0x00);
    tpos_mutexPend(&SIGNAL_PLAN_LIST);// TODO
	mem_cpy(task_plan_info.value,plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].value,sizeof(TASK_PLAN_INFO));
	tpos_mutexFree(&SIGNAL_PLAN_LIST);


    mem_cpy(jzq_dt,datetime+SECOND,6);
    // ����������Ϣ 
	offset = read_ctrl_work_info->cur_plan_save_idx*PIM_PER_PLAN_SIZE;
	offset += PIM_PLAN_CONFIG_START;
	fread_array(FILEID_PLAN_TASK,offset,plan_info.value,25);

    //�洢������Ϣ���洢��������Ӧ���� ռ�õ��ļ�ID 
    offset += sizeof(TASK_PLAN);
	fread_array(FILEID_PLAN_TASK,offset,save_ctrl.value,sizeof(PLAN_TASK_SAVE_CTRL));
	
	if( 0 == compare_string(plan_info.plan_id,task_plan_info.plan_id,2) )
	{
        for(idx_mask = 0;idx_mask < 2048;idx_mask++)
        {
            if( get_bit_value(mask,256,idx_mask) )
            {
                //д��ʧ�ܵ�����
                task_seq = idx_mask;
                cnt = 0;
                
                // ȷ������洢������ λ�ڵڼ����ļ���
            	file_id = (plan_info.data_save_cnt * (task_seq))/FILE_SAVE_TASK_DATA_CNT;
            	offset = (plan_info.data_save_cnt * (task_seq))%FILE_SAVE_TASK_DATA_CNT;
            	offset *= 300;// һ���洢������300�ֽ�
            	for(idx=0;idx<256;idx++)
            	{
            		if(get_bit_value(save_ctrl.file_flag,32,idx) == 0)
            		{
            		    if(find_file_flag == FALSE)
            		    {
            		        //
            		        find_file_flag = TRUE;
            				file_flag = idx+FILEID_TASK_DATA;//Ϊʵ�ʴ洢��־��λ��
            		    }
            			cnt++;
            			if( (file_id+1) == cnt)
            			{
            				file_id = idx+FILEID_TASK_DATA;//Ϊʵ�ʴ洢���ݵ�λ��
            				break;
            			}
            		}
            	}
            	if(idx == 256)// û�ҵ���TODO ???
            	{
            		continue;
            	} 
                //�������ݲɼ�����ʱ�꣬��ȡ�洢λ�ã���Ϊ��ȡ��ʱ��Ҳ����ô����λ�õ�  TODO ????
    			unit = get_unit(plan_info.repeat_unit);
    			if((unit != 0) && (plan_info.repeat_val != 0) && (plan_info.data_save_cnt != 0) )
    			{
    			    mem_cpy(dt,task_plan_info.DA_time,6);
    				diff_time = pfun_get_diff_time[unit-1](dt,task_plan_info.task_start_time);
    				diff_time /= plan_info.repeat_val;//���ظ�����ȡ��
    				diff_time %= plan_info.data_save_cnt;//�洢���
    		    }
    		    else
    		    {
    		        // 2017-07-15 ���һ����������ִֻ��һ�� 
    		        if( (plan_info.data_save_cnt != 0) && ((unit == 0) || (plan_info.repeat_val == 0)) )
    		        {
    		            //
    		            diff_time = 0;//
    		        }
    		    }
    		    
                {
       				save_flag_idx = (INT8U )diff_time; // �ϱ���־��λ�� 

       				
    				//�ҵ�ѭ���洢��λ��
    				offset += diff_time*300;
    				if(offset >= FILE_SAVE_TASK_DATA_CNT*300)
    				{
    					file_id += offset/(3058*300);
    					offset = ((offset/300)%3058)*300;
    				}                        

                    // д�� ���� 
                    task_seq += 1;//
                    mem_set(fail_report.value,sizeof(C4_F1_FAIL),0x00);
                    mem_cpy(fail_report.plan_id,read_ctrl_work_info->msg_info.plan_id,2);// 2
                    mem_cpy(fail_report.task_seq,(INT8U *)&(task_seq),2);// 2
                    mem_cpy(fail_report.obj_addr,read_ctrl_work_info->msg_info.obj_addr,6);// 6
                    fail_report.task_result = 0;//ʧ��  // 1
                    for(idx=0;idx<6;idx++)
                    {
                        fail_report.DA_time[idx] = byte2BCD(DA_time[idx]); // 6
                    }

                    for(idx=0;idx<6;idx++)// ��дΪ��һ�ε��ն�ִ��ʱ�䣬��ʱ����ͬһ��ʱ��???
                    {
                        fail_report.exe_time[idx] = byte2BCD(jzq_dt[idx]); // 6
                    }
                    
                    //mem_set(fail_report.exe_time,6,0x99);  // 6
                    //2017-07-13 �������ڴ�������д 0  
                    fail_report.try_cnt = read_ctrl_work_info->retry_cnt;// 0 1  ??????
                    fail_report.msg_len = 1;  // 1
                    fail_report.data = 0x0C;  // 1�ֽ�   12  ����δִ�� 
        
                    fwrite_array(file_id,offset,fail_report.value,sizeof(C4_F1_FAIL));
                
    				//
    				offset = (MAX_PAGE_SIZE*512);
    				offset += plan_info.data_save_cnt*(task_seq-1);
    				offset += save_flag_idx;

    				// д��AA �����ϱ��� 
    				// 2017-07-12 �ϱ����Ϊ2��  ֻ�Ǵ洢���� �洢�ϱ���־ 
    				if(report_condition != 2)
    				{
                        flag = 0xAA;
                        fwrite_array(file_flag,offset,&flag,1);
                    }
    				
    			}   

    		    // �����֡�����룬����һ��������2017-07-15 
                if((unit == 0) || (plan_info.repeat_val == 0))
                {
                    offset = (read_ctrl_work_info->cur_plan_save_idx)*PIM_PLAN_TASK_STATE_PER_PLAN_SIZE;
                    offset += PIM_PLAN_TASK_STATE_READ_STATE;  
            		offset += idx_mask/8;
            		bit_pos = idx_mask%8;
            		fread_array(FILEID_PLAN_TASK_STATE,offset,&task_state,1);
            		clr_bit_value(&task_state,1,bit_pos);
            		fwrite_array(FILEID_PLAN_TASK_STATE,offset,&task_state,1);
                    #ifdef __SOFT_SIMULATOR__       
                    snprintf(info,100,"*****��֡�����룬�������= %03d,ƫ�� = %08d,bit_pos = %02d ",(idx_mask+1),offset,bit_pos );
                    debug_println_ext(info);                       
                    #endif
                }
            }
        }
    }
}
/*
·����Ӧ���ĸ�ʽ: 
plan_id     2 bytes  
task_seq    2 bytes
obj_type    1 byte
obj_addr    6 bytes
msg_len     1 byte
msg_content n bytes

376.1 msg format
plan_id     2 bytes  
task_seq    2 bytes
obj_addr    6 bytes
task_result 1 byte
DA_TD       6 bytes // need to get
exe_td      6 bytes //datetiem 
try_cnt     1 byte 
msg_len     1 byte
msg_content n bytes  
*/
INT8U save_resp_data(READ_CTRL_WORK_INFO *read_ctrl_work_info,INT8U* frame,INT16U* frame_len)
{	   
	INT32U	offset = 0;	
	READ_METER_FUJIAN_CTRL *fujian_ctrl = NULL;
	RESP_MSG_FMT *resp_msg_fmt = NULL;//���ر��ĵĸ�ʽ
	TASK_PLAN_EXEC_RESULT *task_plan_result = NULL;
	INT16U	len = 0;
	INT16U	file_id = 0;
	INT16U 	crc16 = 0;
	INT16U  success_task_cnt = 0;	
	INT8U	td[6];
	INT8U	idx = 0;
	INT8U	bit_pos = 0;
	INT8U	task_state = 0;
	INT8U   retry_cycle_cnt = 0;
	BOOLEAN flag = FALSE;
	INT8U   report_flag = 0;

	INT8U   old_crc_data[2]; //�Ϻ��жϱ�־λ
    INT8U   new_crc_data[2];
	INT8U   report_condition = 0;
	INT8U   port_idx = 0;
	
	#ifdef __SOFT_SIMULATOR__
	//if(bin2_int16u(resp_msg_fmt->plan_id) == 2)
	{
		//if(resp_msg_fmt->msg_len == 1)
		//return 0 ;
	}
	#endif
	
	//���ݶ˿���???
	switch(read_ctrl_work_info->port)
	{
	    //
	    case COMMPORT_485_REC:
	        port_idx = get_readport_idx(COMMPORT_485_REC);
	        if(port_idx != 0xFF)
	        {
    			fujian_ctrl = (READ_METER_FUJIAN_CTRL *)&portContext_rs485[port_idx].fujian_ctrl;
    			resp_msg_fmt = (RESP_MSG_FMT *)&(read_ctrl_work_info->msg_info);
    			task_plan_result = (TASK_PLAN_EXEC_RESULT *) &portContext_rs485[port_idx].task_plan_result;
    			crc16=CRC16_2(frame,*frame_len,0xFF,0xFF);
    	        new_crc_data[0] = crc16;
    	        new_crc_data[1] = crc16>>8;
	        }
	        else
	        {
	            /* ��ѭ��??? */ 
	            return 0;
	        }
			//return 0;
			break;		
			
	    case COMMPORT_PLC:
			fujian_ctrl = (READ_METER_FUJIAN_CTRL *)&portContext_plc.fujian_ctrl;
			resp_msg_fmt = (RESP_MSG_FMT *)frame;
			task_plan_result = (TASK_PLAN_EXEC_RESULT *) &portContext_plc.task_plan_result;
			crc16=CRC16_2(frame+12,*frame_len-12,0xFF,0xFF);
	        new_crc_data[0] = crc16;
	        new_crc_data[1] = crc16>>8;
			break;
		default:
			return 0;
			break;
	}	
	if(0 == get_bit_value(read_ctrl_work_info->obj_task,256,read_ctrl_work_info->cur_mask_idx))
	{
	    /*����������� ��ִ������жϲ��洢���ݡ������־�Ȳ��� ��������ϱ���֡
		  �����⴦������ᵼ������ͳ�Ʋ��ԡ����ݿ��ܶ���ϱ������� 2018-03-21 */
	    return 0;
	}
	if(compare_string(read_ctrl_work_info->msg_info.value,resp_msg_fmt->value,11) == 0)
	{
	    if(COMMPORT_PLC == read_ctrl_work_info->port)
	    {
    		// ��Ϣ��ͬ��ʱ��Ŵ洢��
    		mem_cpy((INT8U*)&resp_msg_fmt->obj_type,resp_msg_fmt->obj_addr,6);
    
    		offset = bin2_int32u(read_ctrl_work_info->task_data_offset); // 
            file_id = bin2_int16u(read_ctrl_work_info->file_id);
    	    fread_array(file_id,offset+23,&retry_cycle_cnt,1);
    		if(retry_cycle_cnt == 0xFF)
    		{
    		    retry_cycle_cnt = 0;
    		}
    		
    		if( (resp_msg_fmt->msg_len == 1) || (resp_msg_fmt->msg_len == 0)
    		 || (frame[12] == 0x0B ))// ʧ�� ���ſ�� ������ ������  18-03-21 
    		{
    			frame[10] = 0;
    			//ʧ���ˣ��ҳ��������Դ������������ڴ��� ++
    			if(read_ctrl_work_info->fail_cnt == 0)
        		{
        		    //
        		    retry_cycle_cnt++;//
        		}
    			read_ctrl_work_info->fail_cnt ++;

    			//���Դ���Ϊ0 ��ʱ��ִ��һ��  TODO ?????
    			// >= ��Ϊ>  ���� ����Ϊ1��ʱ��ִ������ ����һ�� ����һ�� 2017-07-05 lyc
    			if( (read_ctrl_work_info->fail_cnt > read_ctrl_work_info->retry_cnt) ) // && (0 != read_params->retry_cnt) )
    			{		
    				flag = TRUE;
    				frame[12] = 0x0B;// 11 ʧ��ԭ�� ��ʱ ��·�ɻظ���ֵ�� 0x00
    			}
    		}
    		else
    		{
    		    if(retry_cycle_cnt == 0)
    		    {
    		        //δʧ�ܹ����ɹ��� ���� 1
    		        retry_cycle_cnt = 1;//
    		    }
    		    else
    		    {
    		        //��ʧ�ܣ����������������ں󣬳ɹ��ˣ�Ӧ��+1 2017-07-17
    		        retry_cycle_cnt++;
    		    }
    			frame[10] = 1;//�ɹ�
    		}
    		// �ճ�13�ֽ� �洢 DA_TD(6) exe_td(6), try_cnt(1)
    		len = frame[11]+1;//
    		mem_cpy_right(frame+24,frame+11,len);
	    }
		else
		{
		    //
		    mem_cpy_right(frame+25,frame,*frame_len);
			frame[24] = *frame_len;//������Ϣ
			len = *frame_len+1;//���Ϸ��ر��ĳ���һ���ֽ�

			mem_cpy(frame,resp_msg_fmt->value,4);// 4����Ϣ

			mem_cpy(frame+4,resp_msg_fmt->obj_addr,6);// 6ͨ�ŵ�ַ
            //�ɹ���ʱ���־�Լ����Դ��� 
            offset = bin2_int32u(read_ctrl_work_info->task_data_offset); // 
            file_id = bin2_int16u(read_ctrl_work_info->file_id);
    	    fread_array(file_id,offset+23,&retry_cycle_cnt,1);
    		if(retry_cycle_cnt == 0xFF)
    		{
    		    retry_cycle_cnt = 0;
    		}
    		
    		if( (frame[25+8] == 0xD1) && (frame[25+10] == 0x80) )// ʧ�� *frame_len <= 13
    		{
    			frame[10] = 0;
    			// ���ظ�����ʱ�������������ˣ���һ�γ����ɹ�����1����һ���������� ����1��
    			// ��һ�γ��� Ҫ��Ҫ������ ???? TODO 
        		retry_cycle_cnt++;//       		
    			//retry_cycle_cnt = 1;		
    			flag = TRUE;
    			frame[24] = 1;//������Ϣ ʧ�� 
    			frame[25] = 11;//��ʱ����
			    len = 2;//���Ϸ��ر��ĳ���һ���ֽ�
    		}
    		else
    		{
    		    if(retry_cycle_cnt == 0)
    		    {
    		        //δʧ�ܹ����ɹ��� ���� 1
    		        retry_cycle_cnt = 1;//
    		    }
    		    else
    		    {
    		        //��ʧ�ܣ����������������ں󣬳ɹ��ˣ�Ӧ��+1 2017-07-17
    		        retry_cycle_cnt++;
    		    }
    			frame[10] = 1;//�ɹ�
    		}
			
		}
		// frame+11  DA_TD  TODO ?????????/ BCD  
		//���ݲɼ�����ʱ�꣬�����������޸��ˣ����³������ˡ�2017-07-11
		mem_cpy(td,fujian_ctrl->cur_DA_TD,6); //portContext_plc.fujian_ctrl.cur_DA_TD
		for(idx=0;idx<6;idx++)
		{
			td[idx] = byte2BCD(td[idx]);
		}
		mem_cpy(frame+11,td,6);
		// frame+17  exe_td
		mem_cpy(td,datetime,6);
		for(idx=0;idx<6;idx++)
		{
			td[idx] = byte2BCD(td[idx]);
		}
		mem_cpy(frame+17,td,6);

		// frame+23  try_cnt 
		// 2017-07-13  ���Դ�����дʵ�ʴ���
		// 2017-07-14  ��д����������ֵ�������˼������ھ���д���� read_params->retry_cnt
		frame[23] = retry_cycle_cnt;//retry_cycle_cnt;//   TODO ??????

        // 2017-07-06 ֻ�е������Դ������߳ɹ���ʱ�� ��д������
        if( (1 == frame[10]) || (TRUE == flag) )
        {
    		// ����������ȡ�Ĺ����Լ������ offset ��file_id
    		len += 24;//��д�볤��
    		offset = bin2_int32u(read_ctrl_work_info->task_data_offset); // 
    		file_id = bin2_int16u(read_ctrl_work_info->file_id);
    		fwrite_array(file_id,offset,frame,len);
        }
		
		// ����TD д�����ݰ�???
		//offset = (bin2_int16u(read_params->msg_info.plan_id)-1)*PIM_PLAN_TASK_STATE_PER_PLAN_SIZE;
		offset = (read_ctrl_work_info->cur_plan_save_idx)*PIM_PLAN_TASK_STATE_PER_PLAN_SIZE;
		fread_array(FILEID_PLAN_TASK_STATE,offset,td,6);
		
		if( (1 == frame[10]) || (TRUE == flag) )//�ɹ��˲������־ ʧ���� �ݶ� �������Դ����� ��������������������ڲ���  TODO
		{
    		if(compare_string(td,fujian_ctrl->cur_DA_TD,6) == 0)
    		{
    		    
    		    // ��ʱʧ���� �Ͳ������־ 170630 �������ܱ�֤��һ���������ڻ����Գ���
    		    // ���ʧ����������0����ʧ�����Ե�λ�����⡣�������־,���򳭶���ɺ��ٶ��ϵ絼��310�����޷�����Ԥ�档2017-07-07
    		    //���һ�����������ظ����ڵ�λ ������ֵΪ0 Ҳ������� ���ٳ��� 2017-07-15
    		    if( (FALSE == flag) || (0 == plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].fail_repeat_val) 
    		    || (0 == plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].fail_repeat_unit) )
    		    {
    		        #if (defined __PLAN_TASK_MULTIFRAME_CTRL__)
    		        if(0xAA != read_ctrl_work_info->multiframe_flag) // ��֡ ������
    		        #endif
    		        {
    		            #ifdef __SOFT_SIMULATOR__       
                        snprintf(info,100,"*****��֡�����룬�������= %03d ",(read_ctrl_work_info->cur_mask_idx+1) );
                        debug_println_ext(info);                       
                        #endif
            			offset += PIM_PLAN_TASK_STATE_READ_STATE;  
            			offset += read_ctrl_work_info->cur_mask_idx/8;
            			bit_pos = read_ctrl_work_info->cur_mask_idx%8;
            			fread_array(FILEID_PLAN_TASK_STATE,offset,&task_state,1);
            			clr_bit_value(&task_state,1,bit_pos);
            			fwrite_array(FILEID_PLAN_TASK_STATE,offset,&task_state,1);
        			}
                }
				if(plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].report_condition == 2)
				{
				    //
				    report_condition = plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].report_condition;
				    offset = PIM_TASK_DATA_REPORT_CRC_DATA_START;
					offset += (bin2_int16u(read_ctrl_work_info->msg_info.task_seq)-1)*2;
					file_id = bin2_int16u(read_ctrl_work_info->file_flag_id);
					fread_array(file_id,offset,old_crc_data,2);	
					#ifdef __SOFT_SIMULATOR__       		
            	    snprintf(info,100,"���=%04d,old crc=%04x;new crc=%04x.",bin2_int16u(read_ctrl_work_info->msg_info.task_seq),bin2_int16u(old_crc_data),
            			bin2_int16u(new_crc_data) );
            	    debug_println_ext(info);
            	    #endif
					if( ((old_crc_data[0] == 0xFF) && (old_crc_data[1] == 0xFF)) || (compare_string(old_crc_data,new_crc_data,2) != 0) )
					{		
					    // �������ʧ���ˣ�����֮ǰд���CRC��˵���й��ɹ��������д��CRC����ʱ������
					    // ����ɹ��ˣ���д��CRC���ϱ���־
					    // 2017-07-12 ��ʱ����ʧ�ܵĲ��ϱ� ��д��CRC���ϱ���־
                	    //if( ((old_crc_data[0] != 0xFF) && (old_crc_data[1] != 0xFF) && (TRUE == flag) )
                	    //    || (1 == frame[10]) )
                	    if(1 == frame[10])//��ʱ���ճɹ��Ĳ�д��
                	    {
                	        
					        fwrite_array(file_id,offset,new_crc_data,2);
					        #ifdef __SOFT_SIMULATOR__       		
                    	    snprintf(info,100,"д���µ�CRC16" );
                    	    debug_println_ext(info);
                    	    #endif

                    	    offset = bin2_int32u(read_ctrl_work_info->task_flag_offset); // 
                		    file_id = bin2_int16u(read_ctrl_work_info->file_flag_id);
                		    report_flag = 0xAA;
                		    fwrite_array(file_id,offset,&report_flag,1);
                		    #ifdef __SOFT_SIMULATOR__       		
                     	    snprintf(info,100,"д���µ�CRC16" );
                     	    debug_println_ext(info);
                     	    #endif
					    }
					    
					}				
				}
				else
				{
				    #if (defined __PLAN_TASK_MULTIFRAME_CTRL__)
    		        if(0xAA != read_ctrl_work_info->multiframe_flag) // ��֡ ���д���ϱ���־ 
    		        #endif
    		        {
                        //�����־��ʱ�򣬲ſ���д��AA˵�������ϱ�
        				offset = bin2_int32u(read_ctrl_work_info->task_flag_offset); // 
                		file_id = bin2_int16u(read_ctrl_work_info->file_flag_id);
                		report_flag = 0xAA;
                		fwrite_array(file_id,offset,&report_flag,1);
            		}
				}
    		}
		}
		// �������µĳ���ʱ�䣬�ȷ��ڴ�ɣ�����ʲôʱ����ڴ�洢�����������۰�??????
		tpos_mutexPend(&SIGNAL_PLAN_LIST);
		mem_cpy(plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].task_read_time,datetime,6);
		for(idx=0;idx<6;idx++)
		{
		    //bcd ��ʽ
		    task_plan_result->last_exec_time[idx] = byte2BCD(datetime[SECOND+idx]);
		}
		tpos_mutexFree(&SIGNAL_PLAN_LIST);
		//�����־ ��mask idx ��֮ǰһ����  where to get the bit pos 
		if(1 == frame[10])//�ɹ��˲������־ 
		{
		    /*����������� �Ÿ��³ɹ����� ���򲻸��� Ҫ�������ϱ���֡
		    �ᵼ������ͳ�Ʋ��� 2018-03-21 */
		    //if(get_bit_value(read_params->read_mask.obj_task,256,read_params->cur_mask_idx))
		    /*18-03-22 �����֡ ���������� ��Ƴ�����ɲŸ�������  �����Ͻ� ��ʱ���� */
		    #if (defined __PLAN_TASK_MULTIFRAME_CTRL__)
		    if(0xAA != read_ctrl_work_info->multiframe_flag)
		    #endif
		    {
    		    success_task_cnt = bin2_int16u(task_plan_result->success_task_cnt);
    			success_task_cnt++;//�ɹ�������������
    		    int16u2_bin(success_task_cnt, task_plan_result->success_task_cnt);
    		}
    		clr_bit_value(read_ctrl_work_info->obj_task,256,read_ctrl_work_info->cur_mask_idx);
    		/* 2018-03-27 */
    		if(COMMPORT_PLC == read_ctrl_work_info->port)
    		{
    		    clr_bit_value(portContext_plc.all_task_mask,256,read_ctrl_work_info->cur_mask_idx);
    		}
		}
		else
		{
		    //ʧ���� �������Դ�����ȫ��������룬��50F1�ظ�ȷ�ϡ��ز���ô���� 485��?  TODO ?????
		    
		}
		// ���浽F161
		//save_dayhold_zxyg(frame);
		
		// TODO �������ִ�����ˣ�ɾ����?
		if(check_is_all_ch(read_ctrl_work_info->obj_task,256,0x00) )
		{
			//�����ϣ������ã������ǰ�  TODO ???????  
			fujian_ctrl->sucess_obj_cnt++;//

			#if (defined __PLAN_TASK_MULTIFRAME_CTRL__)
            //
            if(0xAA == read_ctrl_work_info->multiframe_flag)
            {
                clear_mask_write_report_flag(read_ctrl_work_info,task_plan_result);
            }
			#endif
			
			// 170701 ʧ�������ͳɹ��������ܺ� �����ܶ�������ʱ���ó����ɹ���
			if(fujian_ctrl->fail_obj_cnt + fujian_ctrl->sucess_obj_cnt == fujian_ctrl->all_obj_cnt)
			{
				// �õ�ǰ����״̬Ϊ�������?? TODO ?
				fujian_ctrl->task_ctrl_cmd.cur_plan_read_sucess = 1;//��ǰ���񳭶��ɹ�
			}
		}
		else
		{
		    //
		    if(TRUE == flag)
		    {
		        #if (defined __PLAN_TASK_MULTIFRAME_CTRL__)
		        if(0xAA == read_ctrl_work_info->multiframe_flag) // ��֡ ʧ��ʱ��        
		        {
                    
    				offset = bin2_int32u(read_ctrl_work_info->task_flag_offset); // 
            		file_id = bin2_int16u(read_ctrl_work_info->file_flag_id);
            		report_flag = 0xAA;
            		fwrite_array(file_id,offset,&report_flag,1);
            		//clear_mask_write_report_flag(read_params);
            		//mem_set(read_params->read_mask.obj_task,256,0x00);
        		}
        		#endif
        		{
        		    #if (defined __PLAN_TASK_MULTIFRAME_CTRL__)
    		        if(0xAA == read_ctrl_work_info->multiframe_flag) // ��֡ ʧ��ʱ��        
    		        {
    		            //
    		            clear_mask_write_report_flag(read_ctrl_work_info,task_plan_result);
    		        }
    		        #endif
    		        //����������룬Ȼ�󳭶�����ʣ������洢ʧ�ܣ�����������δִ�У����������
    		        clr_bit_value(read_ctrl_work_info->obj_task,256,read_ctrl_work_info->cur_mask_idx);        
    		        save_fail_obj_reserve_task_data(read_ctrl_work_info,fujian_ctrl->cur_DA_TD,read_ctrl_work_info->obj_task,report_condition);
    		        /* 2018-03-27 �屾���������룬����������ȡ����Ȼ���������������밴λ�룬������������������ 
    		        �������� ����� */
    		        if(COMMPORT_PLC == read_ctrl_work_info->port)
    		        {
        		        clr_bit_value(portContext_plc.all_task_mask,256,read_ctrl_work_info->cur_mask_idx);
                        bit_value_opt_inversion(read_ctrl_work_info->obj_task,256);
                        bit_value_opt_and(portContext_plc.all_task_mask,read_ctrl_work_info->obj_task,256);
    		        }
    		        mem_set(read_ctrl_work_info->obj_task,256,0x00);   		        
		        }
		        fujian_ctrl->fail_obj_cnt++;
				if( (fujian_ctrl->fail_obj_cnt + fujian_ctrl->sucess_obj_cnt) == fujian_ctrl->all_obj_cnt)
				{
				    fujian_ctrl->task_ctrl_cmd.cur_plan_read_sucess = 1;//��ǰ���񳭶��ɹ�
				}
		    }
		}
		if( (fujian_ctrl->fail_obj_cnt != 0 ) && (1 == fujian_ctrl->task_ctrl_cmd.cur_plan_read_sucess) )
		{
		    //����ʧ�ܵģ��ҷ�������ˣ���Ҫ����Ϊʧ������Ҫ��һ��������������	    
		    tpos_mutexPend(&SIGNAL_PLAN_LIST);
    		plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].plan_exec_result = PLAN_EXEC_RESULT_FAIL_WAIT;	
    		tpos_mutexFree(&SIGNAL_PLAN_LIST);
		}
	}
	return 0;
}


//���ݷ����Ĵ洢��Ⱥ��������ڣ�����洢��ƫ����Ϣ
void get_save_info(INT8U save_idx,TASK_PLAN  *plan_info,PLAN_TASK_SAVE_CTRL *save_ctrl)
{
	INT32U	offset;

	offset = PIM_PLAN_CONFIG_START + (save_idx*PIM_PER_PLAN_SIZE);
	fread_array(FILEID_PLAN_TASK,offset,plan_info->value,sizeof(TASK_PLAN));

	offset += sizeof(TASK_PLAN);
	fread_array(FILEID_PLAN_TASK,offset,save_ctrl->value,sizeof(PLAN_TASK_SAVE_CTRL));	
}



INT8U	get_unit(INT8U	repeat_unit)
{	
	INT8U	unit;
	unit = repeat_unit &0x0F;
	switch(unit)
	{
		case UNIT_SECOND:
		case UNIT_MINUTE:
		case UNIT_HOUR:
		case UNIT_DAY:
		case UNIT_MONTH:
		case UNIT_YEAR:
			break;
		default:
			unit = 0;
			break;
	}
	return unit;
}

INT8U	make_request_msg(TASK_CONFIG_INFO task_config_info,INT8U* frame)
{
	INT32U	value = 0;
	INT8U 	*data = NULL;
	INT16U	idx;
	INT8U	pos = 0;
	INT8U	content_len=0;
	INT8U	len;
	INT8U	second = 0;
	INT8U   gb645_start_pos = 0;
	
	data = frame+10;

	//
	len = task_config_info.req_msg_len;

	if(task_config_info.msg_td.msg_regr == 0x01)
	{
	    switch(task_config_info.msg_fmt)
	    {
	        //case 1:
	        case 2:
	        case 3:
        		for(idx=0;idx<len;idx++)
        		{
        			if( (0x68 == data[idx]) && (0x68 == data[idx+7]) )
        			{
        				pos = idx;
        				gb645_start_pos = idx;
        				break;
        			}
        		}
        		content_len = data[pos+POS_GB645_DLEN];
        		break;
        	case 4:
        	    // 1�ֽڿ����� 1�ֽ������򳤶� +n���� 
        	    // �������� 4 FE+ 1 68 + 6 ��ַ + 1 68 + 1 ������ +1�ֽ������򳤶�+n����+1cs+1 0x16
        	    // 2017-07-18 TODO ????
        	    break;
        	default:
        	    break;
		}
		switch(task_config_info.msg_fmt)
		{
			case 2:
				pos += POS_GB645_ITEM+content_len-5;

				//0xCC ����0x99 Ȼ����� ����0xcc �� ��ȥ0x33 Ŀ����ʱ������
				//�����Ľ����BCD�� 
				for(idx=0;idx<5;idx++)
				{
					if(data[pos+idx] == 0xCC)// �������˼�0x33�Ĵ����� 
					{
						data[pos+idx] = byte2BCD(datetime[idx+MINUTE]);
					}
					else
					{
					    data[pos+idx] -= 0x33;//��ȥ0x33  BCD format
					}
				}
				if(task_config_info.msg_td.dly_amend)
				{
					for(idx=0;idx<5;idx++)
					{
						data[pos+idx] = BCD2byte(data[pos+idx]);
					}
					switch(task_config_info.td_cnt.unit)
					{
						case 0x00://�� 
							value = 60;//*task_config_info.td_cnt.cnt;
							break;
						case 0x01:// ʱ
							value = 3600;//*task_config_info.td_cnt.cnt;
							break;
						case 0x02:// ��
							value = 86400;//*task_config_info.td_cnt.cnt;
							break;
						case 0x03: //��
							value = 1;//task_config_info.td_cnt.cnt;
							break;
						default:
							break;
					}
					value *= task_config_info.td_cnt.cnt;
					switch(task_config_info.td_cnt.unit)
					{
						case 0x00://�� 
						case 0x01:// ʱ
						case 0x02:// ��
							if(task_config_info.msg_td.symbol)
							{
								datetime_minus_seconds(data+pos+4,data+pos+3,data+pos+2,data+pos+1,data+pos,&second,value);
							}
							else
							{
								datetime_add_seconds(data+pos+4,data+pos+3,data+pos+2,data+pos+1,data+pos,&second,value);
							}

							
							break;
						case 0x03: //��
							if(task_config_info.msg_td.symbol)
							{
								datetime_minus_months_years(data+pos+4,data+pos+3,value,FALSE);
							}
							else
							{
								datetime_add_Month_Year(data+pos+4,data+pos+3,value,FALSE);
							}

							break;
						default:
							break;
					}
					for(idx=0;idx<5;idx++)
					{
						data[pos+idx] = byte2BCD(data[pos+idx]);
					}
				}

				// ��33 ����
				for(idx=0;idx<5;idx++)
				{
					data[pos+idx] += 0x33;
				}
				//���¼���У�� ���ɱ���
				GB645_checksum(data+gb645_start_pos);
				break;
			case 3:// ���ĸ�ʽ��  03H
				pos += POS_GB645_ITEM+content_len-6;

				// BCD ��ʽ��
				for(idx=0;idx<6;idx++)
				{
					if(data[pos+idx] == 0xCC)
					{
						data[pos+idx] = byte2BCD(datetime[idx+SECOND]);
					}
					else
					{
					    data[pos+idx] -= 0x33;//��ȥ0x33  BCD format
					}
				}
				if(task_config_info.msg_td.dly_amend)
				{
					for(idx=0;idx<6;idx++)
					{
						data[pos+idx] = BCD2byte(data[pos+idx]);
					}
					switch(task_config_info.td_cnt.unit)
					{
						case 0x00://�� 
							value = 60;//*task_config_info.td_cnt.cnt;
							break;
						case 0x01:// ʱ
							value = 3600;//*task_config_info.td_cnt.cnt;
							break;
						case 0x02:// ��
							value = 86400;//*task_config_info.td_cnt.cnt;
							break;
						case 0x03: //��
							value = 1;//task_config_info.td_cnt.cnt;
							break;
						default:
							break;
					}
					value *= task_config_info.td_cnt.cnt;
					switch(task_config_info.td_cnt.unit)
					{
						case 0x00://�� 
						case 0x01:// ʱ
						case 0x02:// ��
							if(task_config_info.msg_td.symbol)
							{
								datetime_minus_seconds(data+pos+5,data+pos+4,data+pos+3,data+pos+2,data+pos+1,data+pos,value);
							}
							else
							{
								datetime_add_seconds(data+pos+5,data+pos+4,data+pos+3,data+pos+2,data+pos+1,data+pos,value);
							}

							
							break;
						case 0x03: //��
							if(task_config_info.msg_td.symbol)
							{
								datetime_minus_months_years(data+pos+5,data+pos+4,value,FALSE);
							}
							else
							{
								datetime_add_Month_Year(data+pos+5,data+pos+4,value,FALSE);
							}

							break;
						default:
							break;
					}
					for(idx=0;idx<6;idx++)
					{
						data[pos+idx] = byte2BCD(data[pos+idx]);
					}
				}

				// ��33 ����
				for(idx=0;idx<5;idx++)
				{
					data[pos+idx] += 0x33;
				}
				//���¼���У�� ���ɱ���
				GB645_checksum(data+gb645_start_pos);
				
				break;
			default:
				break;
		}
	}
	
	//�������
	mem_cpy(frame,frame+10,len);
	return len;
	
}

/*
�˺����ĵ���ǰ���Լ������� �����ڵ�ǰ�ظ��������ˣ����ʱ����Ҫ�ж���������
���������ڵģ��򲻳�����
���������ڣ�����Ҫ�ж�:
����ڵ�ǰ�ظ����ڵĿ�ʼʱ�䣬����ʱ��͵�ǰʱ�����ظ����ڿ�ʼʱ��Ĳ�ֵ�����ظ�������ֵȡ����
�Ƚϼ������Ƿ�һ�£�һ������ͬһ���������ڣ����ٳ�����
���ϣ���Ҫ����
*/
BOOLEAN check_fail_retry_obj_resp_data(TASK_PLAN_INFO task_plan_info,INT8U exec_time[6],INT8U cur_dt[6],INT8U *fail_flag)
{
    INT32U interval1 = 0;
    INT32U interval2 = 0;
    INT8U td[6] = {0};
    INT8U check_td[6] = {0};
    INT8U unit = 0;
    mem_cpy(td,exec_time,6);
    mem_cpy(check_td,cur_dt,6);

    //����� 2000-01-01 00:00:00
    
    unit = get_unit(task_plan_info.fail_repeat_unit);
    if( (task_plan_info.fail_repeat_val != 0) && (unit != 0) )
    {
        //             
        interval1 = pfun_get_diff_time[unit-1](td,task_plan_info.cur_round_start_time);
        interval1 = (interval1)/task_plan_info.fail_repeat_val;
        interval2 = pfun_get_diff_time[unit-1](check_td,task_plan_info.cur_round_start_time);
        interval2 = (interval2)/task_plan_info.fail_repeat_val;

        if(interval1 == interval2)
        {
            *fail_flag = 0xAA;
            return TRUE;//ͬһ������������
        }
        return FALSE;// ����ͬһ������������ ��Ҫ����
    }
    else
    {
        return TRUE;// ��ʧ���������ڵĻ������������Ҫ������
    }
    
}
/*
	INT16U task_seq :������� ������� ��ȡ�洢λ��
	INT8U plan_id_idx PLAN_ID �ڿ��������е�λ��
	INT8U *fail_flag  0xAA ������ʧ���ˣ���Ҫʧ������
*/
INT8U	readdata_obj_tsk(INT16U task_seq,READ_CTRL_WORK_INFO *read_ctrl_work_info,TASK_PLAN  *plan_info,PLAN_TASK_SAVE_CTRL *save_ctrl,INT8U plan_id_idx,INT8U *fail_flag)
{
	INT32U	offset = 0;
	INT32U  offset_task_data = 0;
	INT32U	diff_time = 0;
	INT16U	file = 0;
	INT16U  file_flag = 0;
	INT16U	plan;
	INT16U	idx;
	INT8U	cnt = 0;
	INT8U	dt[6] = {0};
	INT8U   tmp_td[6] = {0};
	//INT8U	execute_td[6];
	INT8U	unit = 0;
	TASK_PLAN_INFO task_plan_info;
	C4_F1 task_save_info;
	BOOLEAN find_file_flag = FALSE;
    INT8U   save_flag_idx = 0;

	// ȷ��λ�ڵڼ����ļ���
	file = (plan_info->data_save_cnt * (task_seq))/FILE_SAVE_TASK_DATA_CNT;
	offset = (plan_info->data_save_cnt * (task_seq))%FILE_SAVE_TASK_DATA_CNT;
	offset *= 300;// һ���洢������300�ֽ�
	for(idx=0;idx<256;idx++)
	{
		if(get_bit_value(save_ctrl->file_flag,32,idx) == 0)
		{
		    if(find_file_flag == FALSE)
		    {
		        //
		        find_file_flag = TRUE;
				file_flag = idx+FILEID_TASK_DATA;//Ϊʵ�ʴ洢��־��λ��
		    }
			cnt++;
			if( (file+1) == cnt)
			{
				file = idx+FILEID_TASK_DATA;//Ϊʵ�ʴ洢���ݵ�λ��
				break;
			}
		}
	}
	if(idx == 256)// û�ҵ���TODO ???
	{
		return TRUE;
	}
	else
	{
		// ���ݵ�ǰʱ�䣬��ȡʵ�ʵĴ洢λ��
		tpos_enterCriticalSection();
		mem_cpy(dt,datetime+SECOND,6);
		mem_cpy(tmp_td,dt,6);
		tpos_leaveCriticalSection();

		plan = bin2_int16u(read_ctrl_work_info->msg_info.plan_id);
		if( (plan == 0) || (plan == 0xFFFF) )
		{
			return TRUE;
		}
		tpos_mutexPend(&SIGNAL_PLAN_LIST);// TODO
		mem_cpy(task_plan_info.value,plan_list.plan_info[plan_id_idx].value,sizeof(TASK_PLAN_INFO));
		tpos_mutexFree(&SIGNAL_PLAN_LIST);

		//���ڵ�ǰ������
		if( (DateTimeCompare(dt,task_plan_info.cur_round_start_time) >= 0)
		     && (DateTimeCompare(dt,task_plan_info.cur_round_stop_time) <= 0) )
		{
		    //�������ݲɼ�����ʱ�꣬��ȡ�洢λ�ã���Ϊ��ȡ��ʱ��Ҳ����ô����λ�õ�  TODO ????
			unit = get_unit(plan_info->repeat_unit);
			if((unit != 0) && (plan_info->repeat_val != 0) && (plan_info->data_save_cnt != 0) )
			{
			    mem_cpy(dt,task_plan_info.DA_time,6);
				diff_time = pfun_get_diff_time[unit-1](dt,task_plan_info.task_start_time);
				diff_time /= plan_info->repeat_val;//���ظ�����ȡ��
				diff_time %= plan_info->data_save_cnt;//�洢���
		    }
		    else
		    {
		        // 2017-07-15 ���һ����������ִֻ��һ�� 
		        if( (plan_info->data_save_cnt != 0) && ((unit == 0) || (plan_info->repeat_val == 0)) )
		        {
		            //
		            diff_time = 0;//
		        }
		    }
            {
				save_flag_idx = (INT8U )diff_time;
				//�ҵ�ѭ���洢��λ��
				offset += diff_time*300;
				if(offset >= FILE_SAVE_TASK_DATA_CNT*300)
				{
					file += offset/(3058*300);
					offset = ((offset/300)%3058)*300;
				}

                offset_task_data = offset;//�洢�������ݵ�ƫ��λ��
				//
				fread_array(file,offset,task_save_info.value,sizeof(C4_F1));
				//fread_array(file,offset+PIM_TASK_DATA_EXECUTE_TD,execute_td,6);
				for(idx=0;idx<6;idx++)
				{
					task_save_info.exe_time[idx] = BCD2byte(task_save_info.exe_time[idx]);
				}
				// ������ݴ洢�� file�� offset
				mem_cpy(read_ctrl_work_info->task_data_offset,(INT8U*)&offset,4);
				mem_cpy(read_ctrl_work_info->file_id,(INT8U*)&file,2);

				//flag ��file ��offset 
				offset = (MAX_PAGE_SIZE*512);
				offset += plan_info->data_save_cnt*task_seq;
				offset += save_flag_idx;
				mem_cpy(read_ctrl_work_info->task_flag_offset,(INT8U*)&offset,4);
				mem_cpy(read_ctrl_work_info->file_flag_id,(INT8U*)&file_flag,2);
               
                #if (defined __PLAN_TASK_MULTIFRAME_CTRL__)
                read_ctrl_work_info->cur_rpt_flag_save_pos = save_flag_idx;
				if(0xAA == read_ctrl_work_info->multiframe_flag)
				{
				    return FALSE;// ��֡��־ �� ������ű�־δ����� ��Ӧ�ó���
				}
				#endif
				//δ�洢��ʱ���ն�ִ��ʱ����ȫFF
				if( (DateTimeCompare(task_save_info.exe_time,task_plan_info.cur_round_start_time) >= 0)
		            && (DateTimeCompare(task_save_info.exe_time,task_plan_info.cur_round_stop_time) <= 0) )
				{
				    if( (0 == task_save_info.task_result) && (1 == task_save_info.msg_len ) )
				    {
				        #if (defined __140_PLAN_OUTPUT__)
				        if(bin2_int16u(plan_info->plan_id) == 140)
                        {
                            //rs232_debug_info("\xDD\xDD",2);
                            //rs232_debug_info(task_save_info.exe_time,6);
                            //rs232_debug_info(tmp_td,6);
                            //rs232_debug_info(task_plan_info.cur_round_start_time,6);
                            //rs232_debug_info("\xDD\xDD",2);
                        }
                        #endif
				        // 2017-07-04 ����ʱ����Ҫ����??? ����������Ҫ���ǳ���ʱ��
				        #if (defined __ROUTER_PHASE_CHANGE__)
				        if(TRUE == check_fail_retry_obj_resp_data(task_plan_info,task_save_info.exe_time,tmp_td,fail_flag))
                        {
                            //
                            #if (defined __140_PLAN_OUTPUT__)
                            if(bin2_int16u(plan_info->plan_id) == 140)
                            {
                                //rs232_debug_info("\xE1\xE1",2);
                            }
                            #endif
                            return TRUE;
                        }
				        else
				        #endif
				        {
    				        // ʧ�� �� ����1  ���� ??
    				        return FALSE;
				        }
				    }			    
					return TRUE;
				}
				else
				{
				    // �����FF�������Ļ���֤���Դ��� 00 
				    mem_set(task_save_info.value,sizeof(C4_F1),0xFF);
				    fwrite_array(file,offset_task_data,task_save_info.value,sizeof(C4_F1));
					return FALSE;
				}
			}
		}
		return TRUE;
		
	}
}
BOOLEAN prepare_read_obj_content(READ_CTRL_WORK_INFO *read_ctrl_work_info,INT8U* frame,INT8U* frame_len)
{
	INT32U	offset = 0;
	TASK_PLAN  plan_info;
	PLAN_TASK_SAVE_CTRL save_ctrl;
	TASK_CONFIG_INFO task_config_info;
    INT16U idx;
	//INT16U	plan;
	INT16U	task_seq = 0;
	INT16U	task_file;
	INT8U   save_idx = 0;
	INT8U   fail_flag = 0x00;
	INT8U   port_idx = 0;
	BOOLEAN fail_add_flag = FALSE;
    if (check_is_all_ch(read_ctrl_work_info->obj_task,READ_MASK_BYTE_NUM_OBJ_TASK,0x00))
    {
        return FALSE;
    }
    //�Ȼ�ȡ��Ϣ
    get_save_info(read_ctrl_work_info->cur_plan_save_idx,&plan_info,&save_ctrl);
	for(idx=0;idx<READ_MASK_BYTE_NUM_OBJ_TASK*8;idx++)
	{
		if(get_bit_value(read_ctrl_work_info->obj_task,READ_MASK_BYTE_NUM_OBJ_TASK,idx) )
		{
			//�������������ķ�ʽ
			/*
			1����ȡplan�ʹ洢��ȵ���Ϣ
			2���ҵ�λ��
			*/
			//get_save_info(read_params->msg_info.plan_id,&plan_info,&save_ctrl);
			if(readdata_obj_tsk(idx,read_ctrl_work_info,&plan_info,&save_ctrl,read_ctrl_work_info->cur_plan_id_idx_in_fast_index,&fail_flag) == FALSE)
			{
				task_seq = idx+1;
				mem_cpy(read_ctrl_work_info->msg_info.task_seq,(INT8U *)&task_seq,2);
				//��֡
				//plan = bin2_int16u(read_params->msg_info.plan_id)-1;
				save_idx = read_ctrl_work_info->cur_plan_save_idx;//portContext->fujian_ctrl.cur_exec_plan_id_save_idx;
				task_file = FILEID_TASK_CONFIG + (save_idx)/3;
				offset = (save_idx%3)*PIM_PER_PLAN_TASK_REQUEST_SIZE+2;//�������ֽڵ���������
				offset += idx*PIM_PER_TASK_REQUEST_SIZE;
				fread_array(task_file,offset,task_config_info.value,sizeof(TASK_CONFIG_INFO));
				//������������
				// 2017-07-13 �ж�֡����  ͬʱ֡��� 
				if( (task_config_info.req_msg_len <=220) && (task_config_info.req_msg_len > 0)
				    && (bin2_int16u(task_config_info.task_seq) == task_seq) )
				{
					fread_array(task_file,offset+14,frame+10,task_config_info.req_msg_len);
					//
					read_ctrl_work_info->resp_byte_num = 0;//140;
					read_ctrl_work_info->cur_mask_idx = idx;
					*frame_len = make_request_msg(task_config_info,frame);
					/*
					#ifdef __SOFT_SIMULATOR__					
                    snprintf(info,100,"*** prepare item fujian:");
                    GetHexMessage(frame,*frame_len,info+24);
                    debug_println_ext(info);
                    #endif
                    */
					return TRUE;
				}
			}
			#if (defined __ROUTER_PHASE_CHANGE__)
			if(fail_flag == 0xAA)
			{
			    fail_add_flag = TRUE;
			}
			#endif

			clr_bit_value(read_ctrl_work_info->obj_task,READ_MASK_BYTE_NUM_OBJ_TASK,idx);
    		/*2018-03-27 add */
    		if(COMMPORT_PLC == read_ctrl_work_info->port)
    		{
    		    clr_bit_value(portContext_plc.all_task_mask,READ_MASK_BYTE_NUM_OBJ_TASK,idx);
    		}
		}	
	}
	#if (defined __ROUTER_PHASE_CHANGE__)
	if(TRUE == fail_add_flag)
	{
	    if(COMMPORT_PLC == read_ctrl_work_info->port )
	    {
	        tpos_enterCriticalSection();
	        portContext_plc.fujian_ctrl.fail_obj_cnt ++;
	        if((portContext_plc.fujian_ctrl.fail_obj_cnt + portContext_plc.fujian_ctrl.sucess_obj_cnt) >= portContext_plc.fujian_ctrl.all_obj_cnt)
	        {
	            //
	            portContext_plc.fujian_ctrl.task_ctrl_cmd.cur_plan_read_sucess = 1;
	            
    		    //����ʧ�ܵģ��ҷ�������ˣ���Ҫ����Ϊʧ������Ҫ��һ��������������	    
    		    
        		plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].plan_exec_result = PLAN_EXEC_RESULT_FAIL_WAIT;	      		
	        }
	        tpos_leaveCriticalSection();
	    }
	    else if(COMMPORT_485_REC == read_ctrl_work_info->port )
	    {
	        port_idx = get_readport_idx(COMMPORT_485_REC);
	        if(port_idx != 0xFF)
	        {
     	        tpos_enterCriticalSection();
     	        portContext_rs485[port_idx].fujian_ctrl.fail_obj_cnt ++;
     	        if((portContext_rs485[port_idx].fujian_ctrl.fail_obj_cnt + portContext_rs485[port_idx].fujian_ctrl.sucess_obj_cnt) >= portContext_rs485[port_idx].fujian_ctrl.all_obj_cnt)
     	        {
     	            //
     	            portContext_rs485[port_idx].fujian_ctrl.task_ctrl_cmd.cur_plan_read_sucess = 1;
     	            
         		    //����ʧ�ܵģ��ҷ�������ˣ���Ҫ����Ϊʧ������Ҫ��һ��������������	    
         		    
             		plan_list.plan_info[read_ctrl_work_info->cur_plan_id_idx_in_fast_index].plan_exec_result = PLAN_EXEC_RESULT_FAIL_WAIT;	      		
     	        }
     	        tpos_leaveCriticalSection();
	        }
	    }
	}
	#endif
	return FALSE; 
}
#endif
#ifdef __PROVICE_SHAANXI_CHECK__
BOOLEAN prepare_meter_type_read_item(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
   INT32U item;
   INT16U meter_idx;
   INT8U buffer[10];
   INT8U td_bin[3],load_time[6];
   static INT8U td[3] = {0};

    if(portContext_plc.shaanxi_meter_type != 0x55)//������Ч�������
    {
         return FALSE;
    }
    else
    {
        if(compare_string(td,datetime+DAY,3) != FALSE)   //�����ˣ�Ҳ��Ҫִ����
        {
            if(td[0] != 0)
            {
                mem_cpy(td,datetime+DAY,3);
                portContext_plc.shaanxi_meter_type = 0;
                return FALSE;
            }
            mem_cpy(td,datetime+DAY,3);
        }
    }
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    fread_array(meter_idx, PIM_METER_TYPE,buffer,10);      /*buffer�����¼�˳���ʱ��5�ֽ�+1�ֽ�09/13�ж�+1�ֽڽ��߷�ʽ*/

    if(compare_string(buffer+2,datetime+DAY,3) != FALSE) //ʱ��û�и��¹�
    {

            item = 0x06100101; //A���ѹ
            mem_cpy(td_bin,datetime+DAY,3);
            load_time[0] = 1;
            load_time[1] = 0;
            load_time[2] = 0;
            load_time[3] = byte2BCD(td_bin[0]);
            load_time[4] = byte2BCD(td_bin[1]);
            load_time[5] = byte2BCD(td_bin[2]);

            int32u2_bin(item,read_params->item);
            read_params->resp_byte_num = 40;
            read_params->read_type = READ_METER_TYPE_INFO;

            *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,item,load_time,6);
             return TRUE;

    }
    if(buffer[6] == 0xFF )
    {
        item = 0x0201FF00;
        int32u2_bin(item,read_params->item);
        read_params->resp_byte_num = 30;
        read_params->read_type = READ_METER_TYPE_INFO;
        *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,item,NULL,0);
         return TRUE;
    }

    return FALSE;
}
INT8U save_meter_type_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len)
{
    INT32U item;
    INT16U meter_idx;
    INT8U buffer[10] ={0};
    
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    fread_array(meter_idx, PIM_METER_TYPE,buffer,10);
    mem_cpy(buffer,datetime+MINUTE,5);
    item = bin2_int32u(read_params->item);

    if(item == 0x06100101)
    {
        if(frame[0] == 0xEE) //���ϱ�ʾ09�汾
        {
            buffer[5] = 1;

        }
        else  /*�л�Ӧ���жϲ������ǲ������࣬*/
        {
           buffer[5] = 2;
        }

        buffer[6] = 0xFF;
    }

    if(item == 0x0201FF00)
    {
       if(frame[0] == 0xEE)
       {
          buffer[6] = 1;
       }
       else if(frame[2] != 0xFF) /*b���ѹ����FF������Ϊ����������ж�*/
       {
          buffer[6] = 2;
       }
       else
       {
           buffer[6] = 1;
       }
    }
    fwrite_array(meter_idx, PIM_METER_TYPE,buffer,10);
    return 0;
}
#endif
#ifdef __COUNTRY_ISRAEL__
//�������Ĵ�����ʱ�ꣿ����ʱ�괫�����ɣ������Ϸǵı��޷�֧��
void single_meter_tran_three_meter(INT8U *frame)
{
  INT8U tmp[50];

  mem_cpy(tmp,frame,31);

  tmp[28] = 0xFF;
  tmp[29] = 0xFF;
  tmp[30] = 0xFF;
  tmp[31] = 0xFF;
  tmp[32] = frame[28];
  tmp[33] = frame[29];
  tmp[34] = frame[30];

  mem_set(tmp+35,6,0xFF);

  mem_cpy(tmp+41,frame+31,4);

  mem_cpy(frame,tmp,45);

}

void single_meter_loadprofile_tran_three_meter(INT8U *frame)
{
  INT8U tmp[99];

  mem_cpy(tmp,frame,100);
  mem_set(tmp+12,4,0xEE);
  mem_cpy(tmp+16,frame+12,3);
  mem_set(tmp+19,6,0xEE);
  mem_cpy(tmp+25,frame+15,5);
  mem_set(tmp+30,21,0xEE);
  mem_cpy(tmp+51,frame+20,16);
  mem_set(tmp+67,8,0xEE);
  mem_cpy(tmp+75,frame+36,22);

  mem_cpy(frame,tmp,99);

}

//�ж��Ƿ�����ERC37
void check_erc_37(INT16U spot_idx,INT8U *frame)
{
  INT16U meter_idx;
  INT8U cur_event_flag[2],event_flag[2];

    mem_cpy(cur_event_flag,frame+24,2);

    meter_idx = trans_spot_idx_2_meter_idx(spot_idx);

    fread_array(meter_idx,PIM_CUR_MEVENT,event_flag,2);

         if((event_flag[0] != 0xFF) && (event_flag[1] != 0xFF))
         {
             if(compare_string(event_flag,cur_event_flag,2) != 0)
             {
                if( (cur_event_flag[0] > 0) || (cur_event_flag[1] > 0) )
                {
                  event_erc37(spot_idx,event_flag,cur_event_flag);
                }
             }
         }

    fwrite_array(meter_idx,PIM_CUR_MEVENT,cur_event_flag,2);
}
//#endif


INT8U check_save_last_israel_load_profile_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U remain_num, INT8U* start_time) //start_time�����15�ֵ���������byte��ʽ
{
    INT32U offset;
    INT8U load_profile_data[10];
    INT8U midu,i,r_num;
    INT8U td[5],td_bin[5];
    CommandDate s_time;
    midu = 15;

    if(phy->phy  == ISRAEL_LOAD_PROFILE)
    {

        phy->offset = PIM_ISRAEL_LOAD_PRO ;
        phy->data_len = 96; //��
        mem_set(load_profile_data,10,REC_DATA_IS_DEFAULT);

        r_num = remain_num;
        for(i=0;i<96-r_num;i++) //���ѭ��һ��Ҫ����һ��ʵ��ϵͳ�ϵ�ִ��ʱ�䣬��ֹ�����������񣡣�
        {
            calculat_israel_curve_time(start_time,td_bin,remain_num,midu,1);

            offset = get_curve_save_offset(phy,td_bin,midu);
            fread_array(meter_idx,offset,load_profile_data,7);

            if(compare_string(td_bin,load_profile_data,5) == FALSE)
            {
                remain_num ++;
            }
            else
            {
                {
                    break;
                }
            }
        }
        return remain_num;
    }
}
/*start_time��end_time��ΪBIN��ʽ*/
void calculat_israel_curve_time(INT8U* start_time,INT8U* end_time,INT8U num,INT8U midu,INT8U muli_flag)
{
    INT8U td[5];
    CommandDate s_time;
    if(muli_flag == 2)
    {
        num ++;
    }
    //����remain_num����ʱ�� ,����һ������
    s_time.year = start_time[4]+2000;
    s_time.month = start_time[3];
    s_time.day = start_time[2];
    s_time.hour = start_time[1];
    s_time.minute = start_time[0];
    s_time.minute -= (start_time[0]%15);

    commandDateMinusHour(&s_time,(num/4));
    commandDateMinusMinute(&s_time,(num%4)*midu);

    assign_td_value(&s_time,td,5);

    end_time[0] = BCD2byte(td[0]);
    end_time[1] = BCD2byte(td[1]);
    end_time[2] = BCD2byte(td[2]);
    end_time[3] = BCD2byte(td[3]);
    end_time[4] = BCD2byte(td[4]);
}



void save_last_load_profile_data_israel(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U remain_num,INT8U* start_time)
{
    INT32U offset;
    INT8U midu;//idx,num,
    INT8U td[5],td_bin[5];
    INT8U load_profile_data[100] = {0};
    INT8U load_temp[100] = {0};
    INT8U hour_data[50];
    INT8U idx_temp;
    INT8U need_save_second_flag = 0;
    INT8U remain_num_temp;
   // INT8U load_pro_pos,data_pos,phase;
    CommandDate s_time;

    midu = 15;
    portContext_plc.router_phase_work_info[portContext_plc.router_work_info.phase].read_params.multi_num  = 1;
    if(datalen > 100) /*˵���ǵ���������������ɼ�¼��*/
    {

       mem_cpy(load_temp,data,5);
       mem_cpy(load_temp+5,data+58,53);

       need_save_second_flag = 1;

       portContext_plc.router_phase_work_info[portContext_plc.router_work_info.phase].read_params.multi_num  = 2;

    }

    if(phy->phy  == ISRAEL_LOAD_PROFILE)
    {
SAVE_SECOND:

        if(need_save_second_flag == 2)
        {
            mem_cpy(data,load_temp,58);
        }

        calculat_israel_curve_time(start_time,td_bin,remain_num,midu,1);

        phy->data_len = 96;   //96�ֽڣ�ȫ���������滮
        phy->offset = PIM_ISRAEL_LOAD_PRO ; //��
        mem_set(load_profile_data,phy->data_len,REC_DATA_IS_DEFAULT);
        offset = get_curve_save_offset(phy,td_bin,midu);
        mem_cpy(load_profile_data,td_bin,5);


        if(data[3] == 0)
        single_meter_loadprofile_tran_three_meter(data);

        data[6] = data[0];
        data[7] = data[1];
        data[8] = data[2];
        data[9] = data[3];

       // buffer_bcd_to_bin(data+3,data+3,5);     //����ʱ���ʽ����bcd��ʽ
       // if(compare_string(td_bin,data+3,5)==0)
        {
            mem_cpy(load_profile_data+5,data+6,91); /*��di*/
        }

        fwrite_array(meter_idx,offset,load_profile_data,phy->data_len);
       /*
        if((td_bin[0] == 0) && (td_bin[1] == 0)) /*0�������ת��һ��F96��ת��һ���ն��ᣬת�����������⣬��Ҫ�����޸�--20180927��
                 ������Ϊ����06000011�ĵ���ֵ�����ǵ�ǰֵ����������ĺ�����Ҫ�ˣ����Ҫִ�У�ƫ��λ����Ҫ�޸�*/
                 /*
        {
           idx_temp = get_phy_form_list_cycle_day(ISRAEL_HOUR_DATA,phy);
           if(idx_temp != 0xFF)
           {
              get_yesterday(td);
              hour_data[0] = 0;
              hour_data[1] = 0;
              mem_cpy(hour_data+2,load_profile_data+9+46,4); //�����й�������ǰ����5�ֽ�ʱ��+4�ֽ�DI
              mem_cpy(hour_data+6,load_profile_data+9+62,16); //�ַ��ʵ���
              mem_cpy(hour_data+22,load_profile_data+9+42,4); //ʣ�����
              mem_cpy(hour_data+26,load_profile_data+9+40,2);
              mem_cpy(hour_data+28,load_profile_data+9,15);  //��ѹ����
              mem_cpy(hour_data+43,load_profile_data+9+38,2);  //��������
              mem_cpy(hour_data+45,load_profile_data+9+15,2);  //Ƶ��
              
              writedata_cycle_day(meter_idx,phy,td,hour_data,47,load_profile_data,NULL,0xFF);
           }
        }
        */
        if(need_save_second_flag == 1)
        {
            need_save_second_flag = 2;  /*��Ҫ��������������ʱ����*/
            if(remain_num > 0)
            {
                remain_num -- ;
                goto
                 SAVE_SECOND;
            }
            else
            {

            }
        }
    }
}

/*+++
  ���ܣ���鲹����������
  ������
        INT16U meter_idx,    ������
        INT8U remain_num     Ҫ���ϵ�itemƫ��
---*/
INT8U check_plc_router_save_last_israel_hour_load_data(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U load_cnt)
{
    INT32U offset;
    INT8U  israel_data[100];
    INT8U idx,i;//idx,num,
    INT8U bin_td[5]={0};

     idx = load_cnt;
     mem_cpy(bin_td+2,datetime+DAY,3);

     get_phy_form_list_cruve(ISRAEL_HOUR_DATA,phy);

      mem_set(israel_data,phy->data_len,REC_DATA_IS_DEFAULT);

      for(i=0;i<=idx;i++)
      {
        bin_td[1] = i;
        bin_td[0] = 0;
        offset = get_curve_save_offset(phy,bin_td,60);
        fread_array(meter_idx,offset,israel_data,7);

        if(compare_string(bin_td,israel_data,5) == FALSE)
        {
            continue;
        }
        else
        {
            return i;
        }

      }

    return 255;  //����Ҫ���������ݣ�ֱ�ӷ���23��������һ�����е㣬�˳�����

}

void save_last_load_hour_data_israel(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U patch_num,INT8U* start_time)
{
    INT32U offset;
    INT8U td_tmp[5] = {0};
    INT8U buffer[100] = {0};

      mem_cpy(td_tmp+2,datetime+DAY,3);
      td_tmp[1] =  patch_num;

        get_phy_form_list_cruve(ISRAEL_HOUR_DATA,phy);
        offset = get_curve_save_offset(phy,td_tmp,60);

        fread_array(meter_idx,offset,buffer,phy->data_len);
               //����ʱ��
        if(compare_string(buffer,td_tmp,5) != 0)
        {
           mem_set(buffer,phy->data_len,0xFF);
           mem_cpy(buffer,td_tmp,5);

           mem_cpy(buffer+5+phy->block_offset,data+2,datalen);

           fwrite_array(meter_idx,offset,buffer,phy->data_len);
        }

}
#endif

#ifdef __DAYLIGHT_SAVING_TIME__
BOOLEAN is_in_reappear_time_seg(void)
{
	INT8U tmp_datetime[6]={0};
    if(check_is_all_ch(g_DST_params.end_time, 6, 0xFF))
    {
        return FALSE;
    }
	mem_cpy(tmp_datetime, g_DST_params.end_time, 6);
    tmp_datetime[MONTH] &= 0x1F;
    buffer_bcd_to_bin(tmp_datetime, tmp_datetime, 6);
    
	if((0xAA == g_DST_params.time_state)&&(diff_min_between_dt(tmp_datetime, datetime)<=60)&&(compare_string_reverse(datetime, tmp_datetime, 6)<=0))
	{
		return TRUE;
	}
	return FALSE;
}

BOOLEAN check_DST_delay(void)
{
    INT8U tmp_datetime_begin[6]={0};
    INT8U tmp_datetime_end[6]={0};
    
    if(0x55 == g_DST_params.time_state)//���������
    {
        /*�Ѿ��������붬��ʱ��㣬���ǻ�û�л���ȥ�����*/
        mem_cpy(tmp_datetime_end, g_DST_params.end_time, 6);
        tmp_datetime_end[MONTH] &= 0x1F;
        buffer_bcd_to_bin(tmp_datetime_end, tmp_datetime_end, 6);    
        if(0 == compare_string(tmp_datetime_end+MINUTE, datetime+MINUTE, 5))
        {
            return FALSE;
        }
    }
    else if(0xAA == g_DST_params.time_state)//����Ƕ���
    {
        /*�Ѿ����˶����ĵ�ʱ��㣬���ǻ�û�л���ȥ�����*/
        mem_cpy(tmp_datetime_begin, g_DST_params.begin_time, 6);
        tmp_datetime_begin[MONTH] &= 0x1F;
        buffer_bcd_to_bin(tmp_datetime_begin, tmp_datetime_begin, 6);
        if(0 == compare_string(tmp_datetime_begin+MINUTE, datetime+MINUTE, 5))
        {
            return FALSE;
        }
    }
    
    /*�Ѿ��������л��ɹ�ҲҪ�ȵ�10�룬�ȴ���������ʱ���л����*/
    if(0 == compare_string(datetime+MINUTE, g_DST_target_time.target_begin_time+MINUTE, 5))
    {
        if(datetime[SECOND] < 10)
        {
            return FALSE;
        }
    }
    /*�Ѿ����붬�л��ɹ�ҲҪ�ȵ�10�룬�ȴ���������ʱ���л����*/
    if(0 == compare_string(datetime+MINUTE, g_DST_target_time.target_end_time+MINUTE, 5))
    {
        if(datetime[SECOND] < 10)
        {
            return FALSE;
        }
    }
    return TRUE;
}
#endif
BOOLEAN is_month_hold_td_valid(INT8U *td)
{
    if(datetime[MONTH] == 1)
    {
        if( (td[0] != 12) || (td[1] != (datetime[YEAR]-1)) )
        {
            return FALSE;
        }
    }
    else
    {
        if( (td[0] != (datetime[MONTH]-1)) || (td[1] != datetime[YEAR]) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

INT16U make_oop_meter_load_frame(INT8U* frame,INT8U* meter_no,INT8U *item,INT8U item_count,INT8U load_start_time[6],INT8U load_end_time[6],INT8U unit,INT16U cycle)
{
    INT16U pos,year;
    INT8U start_time[7]={0},end_time[7]={0},idx,oad_count=0;
    INT32U item_32;
    
    
    if((frame==NULL)||(item==NULL)||(item_count==0)||(cycle==0))
    {
        return 0;
    }
    /*ת����ʼʱ��*/
    pos =0;
    year=datetime[CENTURY]*100+load_start_time[YEAR];
    start_time[pos++] = year>>8;
    start_time[pos++] = year;
    start_time[pos++] = load_start_time[MONTH];
    start_time[pos++] = load_start_time[DAY];
    start_time[pos++] = load_start_time[HOUR];
    start_time[pos++] = load_start_time[MINUTE];
    start_time[pos++] = load_start_time[SECOND];
    /*ת������ʱ��*/
    pos =0;
    year=datetime[CENTURY]*100+load_end_time[YEAR];
    end_time[pos++] = year>>8;
    end_time[pos++] = year;
    end_time[pos++] = load_end_time[MONTH];
    end_time[pos++] = load_end_time[DAY];
    end_time[pos++] = load_end_time[HOUR];
    end_time[pos++] = load_end_time[MINUTE];
    end_time[pos++] = load_end_time[SECOND];

    /*��֯����*/
    pos=0;
    frame[OOP_POS_BEGIN] = 0x68;
    frame[OOP_POS_LEN] = 0;
    frame[OOP_POS_LEN+1] = 0;
    frame[OOP_POS_CTRL] = CTRLFUNC_PRM_SET | CTRLFUNC_FC_REQ_RESP;
    frame[OOP_POS_ADDR] = 0x05;
    mem_cpy(frame+OOP_POS_ADDR+1,meter_no,6);
    pos = OOP_POS_ADDR + 7;
    frame[pos++] = 2; //�ͻ�����ַCA
    frame[pos++] = 0;  //HCS
    frame[pos++] = 0;
    frame[pos++] = CLIENT_APDU_GET;  //GET
    frame[pos++] = GET_REQUEST_RECORD;  //��¼
    frame[pos++] = 2;  //PIID
    mem_cpy(frame+pos,(INT8U*)"\x50\x02\x02\x00",4);
    pos+=4;
    frame[pos++] = 0x02; //choice
    mem_cpy(frame+pos,(INT8U*)"\x20\x21\x02\x00",4);
    pos+=4;
    frame[pos++] = 28; //��ʼʱ��
    mem_cpy(frame+pos,start_time,7);
    pos+=7;
    frame[pos++] = 28; //����ʱ��
    mem_cpy(frame+pos,end_time,7);
    pos+=7;
    frame[pos++] = DT_TI;
    frame[pos++] = unit;
    frame[pos++] = cycle>>8;
    frame[pos++] = cycle;
    frame[pos++] = 1;/*������ʱ��ֵ��0����������������*/
    for(idx=0;idx<item_count;idx++)
    {
        /*�򸺺ɼ�¼��������࣬�������򵥴���*/
        item_32 = bin2_int32u(item+idx*4);
        switch(item_32)
        {
            case 0x061001FF:
                mem_cpy(frame+pos,"\x00\x20\x00\x02\x00",5);
                pos += 5;
                oad_count++;
                break;
            case 0x061002FF:
                mem_cpy(frame+pos,"\x00\x20\x01\x02\x00",5);
                pos += 5;
                oad_count++;
                break;
            case 0x061003FF:
                mem_cpy(frame+pos,"\x00\x20\x04\x02\x00",5);
                pos += 5;
                oad_count++;
                break;
            case 0x061004FF:  //�޹�����
                mem_cpy(frame+pos,"\x00\x20\x05\x02\x00",5);
                pos += 5;
                oad_count++;
                break;
            case 0x061005FF: //��������
                mem_cpy(frame+pos,"\x00\x20\x0A\x02\x00",5);
                pos += 5;
                oad_count++;
                break;
            case 0x06100601:
                mem_cpy(frame+pos,"\x00\x00\x10\x02\x01",5);
                pos += 5;
                oad_count++;
                break;
            case 0x06100602:
                mem_cpy(frame+pos,"\x00\x00\x20\x02\x01",5);
                pos += 5;
                oad_count++;
                break;
            case 0x06100603:
                mem_cpy(frame+pos,"\x00\x00\x30\x02\x01",5);
                pos += 5;
                oad_count++;
                break;
            case 0x06100604:
                mem_cpy(frame+pos,"\x00\x00\x40\x02\x01",5);
                pos += 5;
                oad_count++;
                break;
            default:
                break;
        }
    }
    if(oad_count==0) return 0;

    //ʱ���ǩ
    frame[pos++] = 0;  //��ʱ����Ҫ��֤ʱ���ǩ
    //����
    frame[OOP_POS_LEN] = (pos+2-1);
    frame[OOP_POS_LEN+1] = (pos+2-1)>>8;
    //����HCSУ��λ
    fcs16(frame+OOP_POS_LEN,11);
    //����FCSУ��λ
    fcs16(frame+OOP_POS_LEN,pos-1);
    pos += 2;
    frame[pos++]=0x16;
    pos = encode_readmeter_frame(frame, pos);
    return pos;
    
}




#if 1

INT8U get_oop_last_load_data_patch_num(READ_PARAMS *read_params,READ_WRITE_DATA *phy,INT8U remain_num,INT8U* start_time) //start_time�����15�ֵ���������byte��ʽ
{
    INT32U offset;
    INT16U meter_idx = 0;
    union{
        INT8U value[40];
    struct{
        INT8U rec_date[5];
        INT8U data[35]; /* ע�����ռ�Ҫ�㹻 */
        };
    }var;
    INT8U midu,i,r_num;//idx,num,
    //INT8U td[5] = {0};
    INT8U td_bin[5] = {0};
    INT8U BlockCnt = 0;
    INT8U check_pos = 0;
    INT8U check_len = 0;
    INT8U idx_sub = 0;
    BOOLEAN flag =  FALSE;

    midu = 15;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    switch(phy->phy)
    {
        case FHJL1_DY_14:
        case (FHJL1_DY_14-0x3F+0): // A ��ѹ
        case (FHJL1_DY_14-0x3F+1): // B ��ѹ
        case (FHJL1_DY_14-0x3F+2): // C ��ѹ
        case FHJL2_DL_14:
        case (FHJL2_DL_14-0x3F+0): // A ����
        case (FHJL2_DL_14-0x3F+1): // B ����
        case (FHJL2_DL_14-0x3F+2): // C ����
            phy->data_len = 23;
            phy->offset = PIM_CURVE_V_I ; //?
            if(FHJL1_DY_DL_PL == phy->phy)
            {
                check_pos = 5;
                check_len = 15;// 6+9 = 15
            }
            else if(FHJL1_DY_14 == phy->phy) //��ѹ�鸺��
            {
                //
                check_pos = 5;
                check_len = 6;
            }
            else if((FHJL1_DY_14-0x3F+0) == phy->phy) // A ���ѹ����
            {
                check_pos = 5;
                check_len = 2;
            }
            else if((FHJL1_DY_14-0x3F+1) == phy->phy) // B ���ѹ����
            {
                check_pos = 7;
                check_len = 2;
            }
            else if((FHJL1_DY_14-0x3F+2) == phy->phy) // C ���ѹ����
            {
                check_pos = 9;
                check_len = 2;
            }
            else if(FHJL2_DL_14 == phy->phy) //�����鸺��
            {
                check_pos = 11;
                check_len = 9;
            }
            else if((FHJL2_DL_14-0x3F+0) == phy->phy) // A ����� ����
            {
                check_pos = 11;
                check_len = 3;
            }
            else if((FHJL2_DL_14-0x3F+1) == phy->phy) // B ����� ����
            {
                check_pos = 14;
                check_len = 3;
            }
            else if((FHJL2_DL_14-0x3F+2) == phy->phy) // C ����� ����
            {
                check_pos = 17;
                check_len = 3;
            }
            else
            {
                check_pos = 5;
                check_len = 0;
            }
            break;
        case FHJL3_YGGL_14:
        case (FHJL3_YGGL_14-0x3F+0):
        case (FHJL3_YGGL_14-0x3F+1):
        case (FHJL3_YGGL_14-0x3F+2):
        case (FHJL3_YGGL_14-0x3F+3):
        case FHJL4_WGGL_14:
        case (FHJL4_WGGL_14-0x3F+0):
        case (FHJL4_WGGL_14-0x3F+1):
        case (FHJL4_WGGL_14-0x3F+2):
        case (FHJL4_WGGL_14-0x3F+3):
        case FHJL5_GLYS_14:
        case (FHJL5_GLYS_14-0x3F+0):
        case (FHJL5_GLYS_14-0x3F+1):
        case (FHJL5_GLYS_14-0x3F+2):
        case (FHJL5_GLYS_14-0x3F+3):
            phy->data_len = 37;
            phy->offset = PIM_CURVE_GL; //?
            if(FHJL2_YG_WG_GL == phy->phy)//�й��޹� ���ɼ�¼
            {
                check_pos = 5;
                check_len = 24;
            }
            else if(FHJL3_YGGL_14 == phy->phy) //�й�����
            {
                //
                check_pos = 5;
                check_len = 12;
            }
            else if((FHJL3_YGGL_14-0x3F+0) == phy->phy) //���й�����
            {
                //
                check_pos = 5;
                check_len = 3;
            }
            else if((FHJL3_YGGL_14-0x3F+1) == phy->phy) // A �й�����
            {
                //
                check_pos = 8;
                check_len = 3;
            }
            else if((FHJL3_YGGL_14-0x3F+2) == phy->phy) //B �й�����
            {
                //
                check_pos = 11;
                check_len = 3;
            }
            else if((FHJL3_YGGL_14-0x3F+3) == phy->phy) //C �й�����
            {
                //
                check_pos = 14;
                check_len = 3;
            }
            else if(FHJL4_WGGL_14 == phy->phy) //�޹�����
            {
                //
                check_pos = 17;
                check_len = 12;
            }
            else if((FHJL4_WGGL_14-0x3F+0) == phy->phy) //���޹�����
            {
                //
                check_pos = 17;
                check_len = 3;
            }
            else if((FHJL4_WGGL_14-0x3F+1) == phy->phy) //A �޹�����
            {
                //
                check_pos = 20;
                check_len = 3;
            }
            else if((FHJL4_WGGL_14-0x3F+2) == phy->phy) //B �޹�����
            {
                //
                check_pos = 23;
                check_len = 3;
            }
            else if((FHJL4_WGGL_14-0x3F+3) == phy->phy) //C �޹�����
            {
                //
                check_pos = 26;
                check_len = 3;
            }
            else if(FHJL5_GLYS_14 == phy->phy) //��������
            {
                //
                check_pos = 29;
                check_len = 8;
            }
            else if((FHJL5_GLYS_14-0x3F+0) == phy->phy) //�ܹ�������
            {
                //
                check_pos = 29;
                check_len = 2;
            }
            else if((FHJL5_GLYS_14-0x3F+1) == phy->phy) //A ��������
            {
                //
                check_pos = 31;
                check_len = 2;
            }
            else if((FHJL5_GLYS_14-0x3F+2) == phy->phy) //B ��������
            {
                //
                check_pos = 33;
                check_len = 2;
            }
            else if((FHJL5_GLYS_14-0x3F+3) == phy->phy) //C ��������
            {
                //
                check_pos = 35;
                check_len = 2;
            }
            else
            {
                check_pos = 5;
                check_len = 0;
            }
            break;
        //
        case FHJL3_GL_YS:
            phy->data_len = 37;
            phy->offset = PIM_CURVE_GL; //?
            check_pos = 29;
            check_len = 8;
            break;
        case FHJL4_ZFXYG:
        case FHJL6_YG_WG_ZDN_14:  // ���������޹�����ʾֵ
        case (FHJL6_YG_WG_ZDN_14-0x3F+0):// �����й�
        case (FHJL6_YG_WG_ZDN_14-0x3F+1):
        case (FHJL6_YG_WG_ZDN_14-0x3F+2):
        case (FHJL6_YG_WG_ZDN_14-0x3F+3):
            phy->data_len = 21;
            phy->offset = PIM_CURVE_ZFXYWG; //?
            if(FHJL4_ZFXYG == phy->phy)
            {
                check_pos = 5;
                check_len = 16;
            }
            else if(FHJL6_YG_WG_ZDN_14 == phy->phy)
            {
                check_pos = 5;
                check_len = 16;
            }
            else if((FHJL6_YG_WG_ZDN_14-0x3F+0) == phy->phy)
            {
                check_pos = 5;
                check_len = 4;
            }
            else if((FHJL6_YG_WG_ZDN_14-0x3F+1) == phy->phy)
            {
                check_pos = 9;
                check_len = 4;
            }
            else if((FHJL6_YG_WG_ZDN_14-0x3F+2) == phy->phy)
            {
                check_pos = 13;
                check_len = 4;
            }
            else if((FHJL6_YG_WG_ZDN_14-0x3F+3) == phy->phy)
            {
                check_pos = 17;
                check_len = 4;
            }
            else
            {
                check_pos = 5;
                check_len = 0;
            }
            break;
        case FHJL5_SXX_WG:
        case FHJL7_SXX_WG_ZDN_14:
        case (FHJL7_SXX_WG_ZDN_14-0x3F+0):// ��һ�����޹��ܵ���
        case (FHJL7_SXX_WG_ZDN_14-0x3F+1):
        case (FHJL7_SXX_WG_ZDN_14-0x3F+2):
        case (FHJL7_SXX_WG_ZDN_14-0x3F+3):

            phy->data_len = 21;
            phy->offset = PIM_CURVE_WG1234; //?
            if(FHJL5_SXX_WG == phy->phy)
            {
                check_pos = 5;
                check_len = 16;
            }
            else if(FHJL7_SXX_WG_ZDN_14 == phy->phy)
            {
                check_pos = 5;
                check_len = 16;
            }
            else if((FHJL7_SXX_WG_ZDN_14-0x3F+0) == phy->phy)// ��һ�����޹��ܵ���
            {
                check_pos = 5;
                check_len = 4;
            }
            else if((FHJL7_SXX_WG_ZDN_14-0x3F+1) == phy->phy)// �ڶ������޹��ܵ���
            {
                check_pos = 9;
                check_len = 4;
            }
            else if((FHJL7_SXX_WG_ZDN_14-0x3F+2) == phy->phy)// ���������޹��ܵ���
            {
                check_pos = 13;
                check_len = 4;
            }
            else if((FHJL7_SXX_WG_ZDN_14-0x3F+3) == phy->phy)// ���������޹��ܵ���
            {
                check_pos = 17;
                check_len = 4;
            }
            else
            {
                check_pos = 5;
                check_len = 0;
            }
            break;
    }

    flag = FALSE;
   // for(i=0;i<(read_params->patch_load_curve_cnt-r_num);i+=BlockCnt) //���ѭ��һ��Ҫ����һ��ʵ��ϵͳ�ϵ�ִ��ʱ�䣬��ֹ�����������񣡣�
    {

        /* ��鳭�� һ����һ������  */
        for(idx_sub=0;idx_sub<= 96;idx_sub++)
        {

            if( idx_sub >= 96)
            {
                return 0xAA;
            }
            calculat_curve_time(start_time,td_bin,idx_sub,midu);

            offset = get_curve_save_offset(phy,td_bin,midu);
            fread_array(meter_idx,offset,var.value,phy->data_len);


                // ����ȫFF�ʹ���д�����
                if( (compare_string(td_bin,var.rec_date,5) == 0) && (FALSE == check_is_all_FF(var.value+check_pos,check_len)) )
                {

                }
                else
                {
                    flag = TRUE;
                    break;
                }
        }
    }
    return idx_sub;
}
#endif

void get_indonesia_month_hold_td(INT8U former_month[2])
{
  INT8U fmonth[2];
  INT8U minus_month = 0;

  if((datetime[DAY] < 14) || ((datetime[DAY] == 14) && (datetime[HOUR] < 10)))  //14��֮ǰ���¼�1����
  {
     get_former_month(fmonth);
  }
  else if((datetime[DAY] > 14 ) || ((datetime[DAY] == 14) && (datetime[HOUR] >= 10)))
  {
      mem_cpy(fmonth,datetime+MONTH,2);
  }

   mem_cpy(former_month,fmonth,2);

   former_month[0] &= 0x01F;
}
#ifdef __HUBEI_STEP_CONTROL__
/*------------------------------
˵����������������
          ��ǰ����
          ������ۻ�����

---------------------------*/
INT8U add_amount_curdata_diff_lastdata(INT8U yestoday_data[4],INT8U today_data[4],INT8U process_add_data[4],INT32U ct)
{
    INT32U  amount_today, amount_yestoday,amount_process;
    INT8U is_valid1,is_valid2,is_valid3,err_flag;
    err_flag = 0;

    amount_process =  bcd2u32(process_add_data, 4, &is_valid3);
    amount_today    = bcd2u32(today_data, 4, &is_valid1);   //4�ֽڣ���ʽ�� amount_process��һ����
    amount_yestoday = bcd2u32(yestoday_data, 4, &is_valid2);


    if((is_valid1) && (is_valid2))
    {
        amount_today *= 100;
        amount_yestoday *= 100;

        if(amount_yestoday <= amount_today)
        {
            amount_today -= amount_yestoday; //����
        }
        else
        {

            amount_today = 0;
        }
    }
    else
    {
        //�޷�����,
        amount_today = 0;

        err_flag = 1;

    }
    amount_today *= ct;
    amount_process += amount_today;

    ul2bcd(amount_process,process_add_data,4);
    return err_flag;
}
INT32U get_line_lost_curve_save_offset(INT8U td[5],INT8U midu)
{
    INT32U offset;
    INT8U num;

    num = 60 / midu;
    offset = getPassedDays(2000+td[4],td[3],td[2]);
    offset *= num * 24;
    offset += td[1] * num;
    offset += td[0] / midu;
    offset = offset % SAVE_POINT_NUMBER_CURVE;
    offset *= PIM_LINE_LOST_PER_SIZE;
    offset += PIM_LINE_LOST_CURVE_START;
    offset++;     //�ܶ�

    return offset;

}
void compute_phase_lost_rate(INT8U sale[4], INT8U supply[4], INT8U *xlost_rate_str)
{
    INT32U  sale_amount,supply_amount;
    double   xlost;
    INT16S   xlost_rate;
    INT8U is_valid1,is_valid2;

    sale_amount =  bcd2u32(sale, 4, &is_valid1);
    supply_amount    = bcd2u32(supply, 4, &is_valid2);   //5�ֽڣ�������4 + ����1��

    xlost = 99.99;
    xlost_rate_str[2] = 0x80;  //������Ϊ��ֵ

    if(supply_amount > 0)
    {
        //�����������۵�������
        if(supply_amount >= sale_amount)
        {
            xlost = supply_amount - sale_amount;
            xlost_rate_str[2] = 0x00;  //������Ϊ��ֵ
        }
        else
        {
            xlost = sale_amount - supply_amount;
            xlost_rate_str[2] = 0x80;  //������Ϊ��ֵ
        }
        xlost /= supply_amount;
        xlost *= 100;

        if(xlost >= 100)  xlost = 99.99;

    }

    xlost_rate = xlost*10;   //С�����1λ��

    xlost_rate_str[0] = byte2BCD(xlost_rate % 100);
    xlost_rate_str[1] = byte2BCD(xlost_rate / 100);
    xlost_rate_str[1] |= xlost_rate_str[2];

}
INT8U check_phy_cal_yes_or_no(INT32U phy)
{
   switch(phy)
   {
     case 0x00000040:
     case 0x0000007F:
     case 0x00002C7F:
     case 0x0000BB40:
     case 0x00000080:
     case 0x000000BF:
     case 0x00002CBF:
     case 0x0000BB80:
     case 0x00000480:
     case 0x000008C0:
     case 0x00000D00:
     return TRUE;
     default:
      return FALSE;
   }

}
void calculate_line_lost(READPORT_METER_DOCUMENT *meter_doc,READ_WRITE_DATA *phy,INT8U *data,INT8U datalen,INT8U data_type)
{
    INT32U offset;
    INT32U ct;
    INT16U meter_idx;

    INT8U  td[3];
    COMP_LINE_LOST  cal_data;
    LINE_LOST_F309 meter_flag;
    INT8U param[2],param_len;
    INT8U rec_datetime[5];
    INT8U td_curve[6],td_curve_bcd[6];
    INT8U yestoday_data[4];
    INT8U err_flag;
    INT8U tmp_buffer[100];
    INT8U *td_tmp;
    INT8U midu,meter_idx_byte[1]={0};


    meter_idx = bin2_int16u(meter_doc->meter_idx);

    if(check_phy_cal_yes_or_no(phy->phy)== FALSE)
    {
        return;
    }
    if(data_type == READ_TYPE_CYCLE_DAY)
    {

        offset  =  (datetime[MONTH] % 2) * 31 + datetime[DAY] -1;
        offset *=  PIM_LINE_LOST_PER_SIZE;
        offset +=  PIM_LINE_LOST_DAYHOLD_DATA;

        fread_array(FILEID_COMP_LINE_LOST,offset,cal_data.value,sizeof(COMP_LINE_LOST));//FILEID_COMP_LINE_LOST��ǰ����COMP_LINE_LOSTռ86�����������й���ʶ256������256

        if(compare_string(cal_data.save_date+2,datetime+DAY,3) != FALSE) //���ʱ����Աȶ��ϣ�˵�����ǵ�һ�μ��㣬����ȶԲ��ϣ���Ҫȫ������
        {
            mem_set(cal_data.value,sizeof(COMP_LINE_LOST),0x00);
            mem_cpy(cal_data.save_date,datetime+MINUTE,5);

            tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
            mem_cpy(g_temp_buffer,cal_data.value,sizeof(COMP_LINE_LOST)); //���ڱ仯�ˣ�������е�flag
            mem_set(g_temp_buffer+LINE_LOST_PER_ZXYG_MASK_START,512,0xFF);
            fwrite_array(FILEID_COMP_LINE_LOST,offset,g_temp_buffer,PIM_LINE_LOST_PER_SIZE);
            tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
        }

    }
    else
    {
        fread_array(meter_idx,phy->offset,&midu,1);
        switch(midu)
        {
            case 15:
                 td_curve[5] = 1;
                 td_tmp = read_meter_flag_curve.cycle_15_minute;
                 break;
            case 30:
                 td_curve[5] = 2;
                 td_tmp = read_meter_flag_curve.cycle_30_minute;
                 break;
            case 60:
                 td_curve[5] = 3;
                 td_tmp = read_meter_flag_curve.cycle_60_minute;
                 break;
            default:
                   td_curve[5] = 1;
                   td_tmp = read_meter_flag_curve.cycle_15_minute;
                   break;
        }
        mem_cpy(td_curve,td_tmp,5);
        offset = get_line_lost_curve_save_offset(td_curve,midu);

        fread_array(FILEID_COMP_LINE_LOST,offset,cal_data.value,sizeof(COMP_LINE_LOST)); //tmp_data�У�ǰ�������ݣ������ǲ������Ƿ��Ѿ���������flag

        if(compare_string(cal_data.save_date,td_curve,5) != FALSE) //���ʱ����Աȶ��ϣ�˵�����ǵ�һ�μ��㣬����ȶԲ��ϣ���Ҫȫ������
        {
            mem_set(cal_data.value,sizeof(COMP_LINE_LOST),0x00);
            mem_cpy(cal_data.save_date,td_curve,5);

            tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
            mem_cpy(g_temp_buffer,cal_data.value,sizeof(COMP_LINE_LOST));
            mem_set(g_temp_buffer+LINE_LOST_PER_ZXYG_MASK_START,512,0xFF);
            fwrite_array(FILEID_COMP_LINE_LOST,offset,g_temp_buffer,PIM_LINE_LOST_PER_SIZE);
            tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
        }
    }

    if((phy->phy == 0x00000040) || (phy->phy == 0x0000007F) || (phy->phy == 0x00002C7F)|| (phy->phy == 0x0000BB40))  //z�����й�
    {
        fread_array(FILEID_COMP_LINE_LOST,LINE_LOST_PER_ZXYG_MASK_START+(meter_idx/8),meter_idx_byte,1);
        if(get_bit_value(meter_idx_byte,1,meter_idx%8)) //�������Ƿ��Ѿ��������
        {
            param_len = get_spot_params(meter_idx, PIM_METER_F309, param, 2);
           // if(param_len == 2)//309û�����ã�Ҫ��Ҫ���㣿
            {
                mem_cpy((INT8U*)&meter_flag,param,2);

              //  meter_flag.ineternet_zxyg = 0;

                if(meter_flag.value  & 0x001F) //��Ҫ����
                {
                    if(data_type == READ_TYPE_CYCLE_DAY)
                    {
                        get_yesterday(td);
                        date_minus_days(td+2,td+1,td,1);
                        if(readdata_cycle_day(meter_idx,phy,td,rec_datetime,tmp_buffer,&datalen,NULL) ) //����һ�յ�����
                        {
                            mem_cpy(yestoday_data,tmp_buffer,4);

                            line_lost_cal(meter_idx,meter_doc,0x00000040,yestoday_data,data,meter_flag,&cal_data);

                        }
                    }
                    else if(data_type == READ_TYPE_CURVE)
                    {

                        buffer_bin_to_bcd(td_curve,td_curve_bcd,6);
                        get_former_curve_td(td_curve_bcd,1);
                        buffer_bcd_to_bin(td_curve_bcd,td_curve,6);
                        if(readdata_curve(meter_idx,phy,td_curve,tmp_buffer,&datalen) ) //����һ��������ߵ�����
                        {
                            mem_cpy(yestoday_data,tmp_buffer,4);

                            line_lost_cal(meter_idx,meter_doc,0x00000040,yestoday_data,data,meter_flag,&cal_data);

                        }
                    }
                }


                clr_bit_value(meter_idx_byte,1,meter_idx%8); //���������������й����ݴ�����ɣ��´β�Ҫ������
                fwrite_array(FILEID_COMP_LINE_LOST,LINE_LOST_PER_ZXYG_MASK_START+(meter_idx/8),meter_idx_byte,1);

                fwrite_array(FILEID_COMP_LINE_LOST,offset,cal_data.value,sizeof(COMP_LINE_LOST));
            }

        }

    }
    else if((phy->phy == 0x00000080) || (phy->phy == 0x000000BF) || (phy->phy == 0x00002CBF) || (phy->phy == 0x0000BB80))
    {
        fread_array(FILEID_COMP_LINE_LOST,LINE_LOST_PER_FXYG_MASK_START+(meter_idx/8),meter_idx_byte,1);
        if(get_bit_value(meter_idx_byte,1,meter_idx%8)) //�������Ƿ��Ѿ��������
        {
            param_len = get_spot_params(meter_idx, PIM_METER_F309, param, 2);
          //  if(param_len ==2)
            {
                mem_cpy((INT8U*)&meter_flag,param,2);

                if(param[0] & 0x1F) //�����й�����
                {

                    if(data_type == READ_TYPE_CYCLE_DAY)
                    {
                        get_yesterday(td);
                        date_minus_days(td+2,td+1,td,1);
                        if(readdata_cycle_day(meter_idx,phy,td,rec_datetime,tmp_buffer,&datalen,NULL) ) //����һ�յ�����
                        {
                             mem_cpy(yestoday_data,tmp_buffer,4);
                             line_lost_cal(meter_idx,meter_doc,0x00000080,yestoday_data,data,meter_flag,&cal_data);
                        }
                    }
                    else if(data_type == READ_TYPE_CURVE)
                    {
                        get_former_curve_td(td_curve,1);

                        if(readdata_curve(meter_idx,phy,td_curve,tmp_buffer,&datalen) ) //����һ��������ߵ�����
                        {
                            mem_cpy(yestoday_data,tmp_buffer,4);

                            line_lost_cal(meter_idx,meter_doc,0x00000080,yestoday_data,data,meter_flag,&cal_data);

                        }
                    }
                }


                clr_bit_value(meter_idx_byte,1,meter_idx%8); //���������������й����ݴ�����ɣ��´β�Ҫ������
                fwrite_array(FILEID_COMP_LINE_LOST,LINE_LOST_PER_FXYG_MASK_START+(meter_idx/8),meter_idx_byte,1);

                fwrite_array(FILEID_COMP_LINE_LOST,offset,cal_data.value,sizeof(COMP_LINE_LOST));
            }

        }
    }
    else if((data_type == READ_TYPE_CYCLE_DAY) && ((phy->phy == 0x00000480) || (phy->phy == 0x000008C0) || (phy->phy == 0x00000D00)))  /*���ߵ��ݲ�����*/
    {
        get_yesterday(td);
        date_minus_days(td+2,td+1,td,1);
        if(readdata_cycle_day(meter_idx,phy,td,rec_datetime,tmp_buffer,&datalen,NULL) ) //����һ�յ�����
        {
             mem_cpy(yestoday_data,tmp_buffer,4);

            if(phy->phy == 0x00000480)
            {
                line_lost_cal(meter_idx,meter_doc,0x00000480,yestoday_data,data,meter_flag,&cal_data);
            }
            else if(phy->phy == 0x000008C0)
            {
                line_lost_cal(meter_idx,meter_doc,0x000008C0,yestoday_data,data,meter_flag,&cal_data);
            }
            else if(phy->phy == 0x00000D00)
            {
                line_lost_cal(meter_idx,meter_doc,0x00000D00,yestoday_data,data,meter_flag,&cal_data);
            }

        }

         fwrite_array(FILEID_COMP_LINE_LOST,offset,cal_data.value,sizeof(COMP_LINE_LOST));

    }

}
void line_lost_cal(INT16U meter_idx,READPORT_METER_DOCUMENT *meter_doc,INT32U phy,INT8U last_data[4],INT8U cur_data[4], LINE_LOST_F309 meter_flag,COMP_LINE_LOST *cal_data)
{

    INT32U ct;
    INT32U supply_amount,sal_amount;
    SPOT_PTCT spot_ptct;

    METER_READ_INFO  C1_F170;
    INT8U err_flag;
    INT8U tmp_buffer[100];
    INT8U read_datetime[5];
    INT8U xlost_rate_str[3];
    INT8U phase_A,phase_B,phase_C;

    fread_meter_params(meter_idx, PIM_METER_F25, spot_ptct.value, sizeof(SPOT_PTCT));
    //ct
    if((spot_ptct.ct == 0xFFFF) || (spot_ptct.ct == 0x00))
    {
        ct = 1;
    }
    else
    {
        ct = spot_ptct.ct;
    }

    if((meter_doc->meter_class.user_class == 6) )
    {
        if(phy == 0x00000040)
        {

            //�ܱ�����������������㹩��
            err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->supply_amount,ct);

            if(meter_flag.ineternet_zxyg)  //����������������ر��ˣ���309����û�����ʹ�ã����ڿ������
            {
              //  err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->internet_amount,ct);
            }

            compute_phase_lost_rate(cal_data->sale_amount, cal_data->supply_amount, xlost_rate_str);
            mem_cpy(cal_data->lost_rate, xlost_rate_str,2);  //����������

        }
        else if(phy == 0x00000080)
        {
            //�ܱ���Ҫ���㵽���͵�����
            err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->charge_amount,ct);

            if(meter_flag.ineternet_fxyg)
            {
              //  err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->internet_amount,ct);
            }
        }
        else if(phy == 0x00000480)
        {
            err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->supply_amount_A,ct);

            compute_phase_lost_rate(cal_data->sale_amount_A, cal_data->supply_amount_A, xlost_rate_str);
            mem_cpy(cal_data->lost_rate_A, xlost_rate_str,2);
        }
        else if(phy == 0x000008C0)
        {
            err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->supply_amount_B,ct);
            compute_phase_lost_rate(cal_data->sale_amount_B, cal_data->supply_amount_B, xlost_rate_str);
            mem_cpy(cal_data->lost_rate_B, xlost_rate_str,2);
        }
        else if(phy == 0x00000D00)
        {
           err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->supply_amount_C,ct);

           compute_phase_lost_rate(cal_data->sale_amount_C, cal_data->supply_amount_C, xlost_rate_str);
           mem_cpy(cal_data->lost_rate_C, xlost_rate_str,2);
        }
    }
    else if(meter_doc->meter_class.meter_class == 2)
    {
        if((phy == 0x00000040) || (phy == 0x00000080)) //�������й��������������۵���
        {

            //������������㹩�磬
            err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->sale_amount,ct);
            compute_phase_lost_rate(cal_data->sale_amount,cal_data->supply_amount, xlost_rate_str);
            mem_cpy(cal_data->lost_rate, xlost_rate_str,2);

            if((meter_flag.ineternet_zxyg) || (meter_flag.ineternet_fxyg))
            {
              //  err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->internet_amount,ct);
            }

        }
        else
        {
            app_readdata_cur_data(meter_idx, 0x0000AEC0, read_datetime, tmp_buffer, 30,TRUE);
            mem_cpy((void *)&C1_F170,(void *)&tmp_buffer,sizeof(METER_READ_INFO));

            if(phy == 0x00000480)
            {
               // if(C1_F170.phase.reserved2) //������
                {
                    if(C1_F170.phase.act_A != 0)
                    {
                        phase_line_lost_cal(1,last_data,cur_data, meter_flag,cal_data,ct);
                    }
                    else if(C1_F170.phase.act_B != 0)
                    {

                        phase_line_lost_cal(2,last_data,cur_data, meter_flag,cal_data,ct);
                    }
                    else if(C1_F170.phase.act_C != 0)
                    {

                        phase_line_lost_cal(3,last_data,cur_data, meter_flag,cal_data,ct);
                    }

                }


            }
            else if(phy == 0x000008C0)
            {

                if(C1_F170.phase.reserved2) //������
                {
                    if(C1_F170.phase.act_A != 0)//C1_F170.phase.rec_A)
                    {

                        phase_line_lost_cal(3,last_data,cur_data, meter_flag,cal_data,ct);
                    }
                    else if(C1_F170.phase.act_B != 0)
                    {

                        phase_line_lost_cal(1,last_data,cur_data, meter_flag,cal_data,ct);
                    }
                    else if(C1_F170.phase.act_C != 0)
                    {

                        phase_line_lost_cal(2,last_data,cur_data, meter_flag,cal_data,ct);
                    }

                }
                else
                {
                    if(C1_F170.phase.act_A != 0)//C1_F170.phase.rec_A)//��ȫ��Ӧ����û����������û����λ��Ӧ��
                    {

                        phase_line_lost_cal(2,last_data,cur_data, meter_flag,cal_data,ct);
                    }
                    else if(C1_F170.phase.act_B != 0)//(C1_F170.phase.rec_A == C1_F170.phase.act_B)
                    {

                        phase_line_lost_cal(3,last_data,cur_data, meter_flag,cal_data,ct);

                    }
                    else if(C1_F170.phase.act_C != 0)//(C1_F170.phase.rec_A == C1_F170.phase.act_C)
                    {

                        phase_line_lost_cal(1,last_data,cur_data, meter_flag,cal_data,ct);
                    }

                }
            }
            else if(phy == 0x000000D00)
            {

                if(C1_F170.phase.reserved2) //������
                {
                    if(C1_F170.phase.act_A != 0)//(C1_F170.phase.act_A == C1_F170.phase.rec_A)
                    {

                        phase_line_lost_cal(2,last_data,cur_data, meter_flag,cal_data,ct);
                    }
                    else if(C1_F170.phase.act_B != 0)//(C1_F170.phase.rec_A == C1_F170.phase.act_B)
                    {

                        phase_line_lost_cal(3,last_data,cur_data, meter_flag,cal_data,ct);

                    }
                    else if(C1_F170.phase.act_C != 0)//(C1_F170.phase.rec_A == C1_F170.phase.act_C)
                    {

                        phase_line_lost_cal(1,last_data,cur_data, meter_flag,cal_data,ct);
                    }

                }
                else
                {
                    if(C1_F170.phase.act_A != 0)//(C1_F170.phase.act_A == C1_F170.phase.rec_A)//��ȫ��Ӧ����û����������û����λ��Ӧ��
                    {
                        phase_line_lost_cal(3,last_data,cur_data, meter_flag,cal_data,ct);
                        //
                    }
                    else if(C1_F170.phase.act_B != 0)//(C1_F170.phase.rec_A == C1_F170.phase.act_B)
                    {
                        phase_line_lost_cal(1,last_data,cur_data, meter_flag,cal_data,ct);
                        //

                    }
                    else if(C1_F170.phase.act_C != 0)//(C1_F170.phase.rec_A == C1_F170.phase.act_C)
                    {
                        phase_line_lost_cal(2,last_data,cur_data, meter_flag,cal_data,ct);
                        //
                    }

                }
            }
        }
    }
    else
    {
        err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->sale_amount,ct);

        compute_phase_lost_rate(cal_data->sale_amount, cal_data->supply_amount, xlost_rate_str);
        mem_cpy(cal_data->lost_rate, xlost_rate_str,2);

        app_readdata_cur_data(meter_idx, 0x0000AEC0, read_datetime, tmp_buffer, 30,TRUE);
        mem_cpy((void *)&C1_F170,(void *)&tmp_buffer,sizeof(METER_READ_INFO));

        if(portContext_plc.router_base_info.router_info1.comm_mode == 2) /*����ĸ�ʽ����*/
        {
            phase_A = C1_F170.phase.rec_A;
            phase_B = C1_F170.phase.rec_B;
            phase_C = C1_F170.phase.rec_C;
        }
        else
        {
            phase_A = C1_F170.phase.act_A;
            phase_B = C1_F170.phase.act_B;
            phase_C = C1_F170.phase.act_C;
        }

        if((phase_A == 1) && (phase_B == 0) && (phase_C == 0))
        {
            phase_line_lost_cal(1,last_data,cur_data, meter_flag,cal_data,ct);
        }
        else if((phase_B == 1) && (phase_A == 0) && (phase_C == 0))
        {
            phase_line_lost_cal(2,last_data,cur_data, meter_flag,cal_data,ct);

        }
        else if((phase_C == 1) && (phase_A == 0) && (phase_B == 0))
        {
            phase_line_lost_cal(3,last_data,cur_data, meter_flag,cal_data,ct);
        }

    }
}

void phase_line_lost_cal(INT8U phase,INT8U last_data[4],INT8U cur_data[4], LINE_LOST_F309 meter_flag,COMP_LINE_LOST *cal_data,INT32U ct)
{
    INT8U xlost_rate_str[3];
    INT8U  err_flag;

    if(phase == 1)
    {
        err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->sale_amount_A,ct);
        compute_phase_lost_rate(cal_data->sale_amount_A, cal_data->supply_amount_A, xlost_rate_str);
        mem_cpy(cal_data->lost_rate_A, xlost_rate_str,2);

        if((meter_flag.ineternet_zxyg) || (meter_flag.ineternet_fxyg)) /*�Ƿֲ�ʽ��Դ��Ҫ���㵽��������*/
        {
              //  err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->internet_amount_A,ct);
        }

    }
    else if(phase == 2)
    {
        err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->sale_amount_B,ct);
        compute_phase_lost_rate(cal_data->sale_amount_B, cal_data->supply_amount_B, xlost_rate_str);
        mem_cpy(cal_data->lost_rate_B, xlost_rate_str,2);

        if((meter_flag.ineternet_zxyg) || (meter_flag.ineternet_fxyg)) /*�Ƿֲ�ʽ��Դ��Ҫ���㵽��������*/
        {
              //  err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->internet_amount_A,ct);
        }

    }
    else if(phase == 3)
    {
        err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->sale_amount_C,ct);
        compute_phase_lost_rate(cal_data->sale_amount_C,cal_data->supply_amount_C, xlost_rate_str);
        mem_cpy(cal_data->lost_rate_C, xlost_rate_str,2);

        if((meter_flag.ineternet_zxyg) || (meter_flag.ineternet_fxyg)) /*�Ƿֲ�ʽ��Դ��Ҫ���㵽��������*/
        {
              //  err_flag =  add_amount_curdata_diff_lastdata(last_data,cur_data,cal_data->internet_amount_A,ct);
        }

    }
}
#endif

INT8U save_load_curve_data(READ_PARAMS *read_params,INT8U* data,INT8U data_num,INT8U* td)
{
    READ_WRITE_DATA phy;
    INT32U phy_u32;
    INT8U idx,curve_type,curve_save_flag=0;
    INT8U* read_mask;
    INT16U midu;
    INT8U buf[200]={0},data_len;
    INT8U* pdata;

   // if(data_num==0)
    //    return 0;

    pdata = data;
    for(idx=0; idx<data_num; idx++)
    {
        phy_u32 = bin2_int32u(pdata);
        data_len = pdata[4];
        if(0xFF == get_phy_form_list_cruve(phy_u32,&phy))
        {
            pdata += 5+data_len;
            continue;
        }

        writedata_curve(read_params,&phy,td,pdata+5,data_len,buf,curve_save_flag);
        pdata += 5+data_len;
    }

    return 1;
}

void writedata_precision_cycle_day_and_curve(INT16U meter_idx,READ_WRITE_DATA *phy,INT8U td[5],INT8U *data,INT8U datalen,INT8U* buffer,INT8U *reserve_data,INT8U flag)
{
    INT32U offset;
    INT32U  new_phy_id = 0 ;
    INT8U idx = 0xFF;
    INT8U midu = 0;
    READ_WRITE_DATA phy_new;

    extern INT32U get_curve_save_offset(READ_WRITE_DATA *phy,INT8U td[5],INT8U midu);


    if((phy->phy >= (ZXYG_DN_SJK-0x3F)) && (phy->phy <= D4XX_WG_DN_SJK))
    {
        new_phy_id =  ((HIGH_PRECISION_ZXYG_DN_SJK-0x3F) + (phy->phy  - (ZXYG_DN_SJK-0x3F)));
    }
    else
    {
        return;
    }


    if(flag == 0x55)
    {
        idx = get_phy_form_list_cycle_day(new_phy_id,&phy_new);
        if(idx == 0xFF) return;

        offset = get_cycle_day_save_offset(&phy_new,td);

        fread_array(meter_idx,offset,buffer,phy_new.data_len);

        //����ʱ��
        if(compare_string(buffer,td,3) != 0)
        {
            mem_set(buffer,phy_new.data_len,0xFF);
            mem_cpy(buffer,td,3);
            mem_cpy(buffer+3,datetime+MINUTE,5);
        }

        if(datalen == 5)//�߾�������
        {
            mem_cpy(buffer+8+phy_new.block_offset,data,datalen);
        }
        else if(datalen == 4)//��ͨ645���������
        {
            buffer[8+phy_new.block_offset] = 0;
            mem_cpy(buffer+9+phy_new.block_offset,data,datalen);
        }
        else if(datalen >= 20) //645�����ݿ�
        {
           for(idx=0;idx<5;idx++)
           {
                buffer[8+idx*5] = 0;
                mem_cpy(buffer+9+(idx*5),data+idx*4,4);
           }
        }

        if(reserve_data)
        {
            mem_cpy(buffer+phy_new.data_len-RESERVE_DATA,reserve_data,RESERVE_DATA);
        }

        fwrite_array(meter_idx,offset,buffer,phy_new.data_len);
    }
    else if(flag == 0xAA)
    {
        idx = get_phy_form_list_cruve(new_phy_id,&phy_new);
        if(idx == 0xFF) return;
              //��ȡ�ܶ�
        fread_array(meter_idx,phy_new.offset,&midu,1);
        
        if(midu != 15)
        {
            midu = 15;
            fwrite_array(meter_idx,phy_new.offset,&midu,1);
        }

        offset = get_curve_save_offset(&phy_new,td,midu);

        fread_array(meter_idx,offset,buffer,phy_new.data_len);

        //����ʱ��
        if(compare_string(buffer,td,5) != 0)
        {
            mem_set(buffer,phy_new.data_len,0xFF);
            mem_cpy(buffer,td,5);
        }

        {
            //��������
            if(datalen == 5)
            {
                mem_cpy(buffer+5+phy_new.block_offset,data,datalen);
            }
            else if(datalen == 4)
            {
                buffer[5+phy_new.block_offset] = 0;
                mem_cpy(buffer+6+phy_new.block_offset,data,datalen);
            }
            else if(datalen >= 20) //���ݿ飬���ߵ�ֻҪ��
            {
                buffer[5+phy_new.block_offset] = 0;
                mem_cpy(buffer+6+phy_new.block_offset,data,4);
            }

        }

        fwrite_array(meter_idx,offset,buffer,phy_new.data_len);

    }

}
#ifdef __SICHUAN_FK_PATCH_CURVE_DATA__
//����frame��frame_lenΪNULLʱ��ֻ�Ǽ���Ƿ��г������ݣ������ɱ���
BOOLEAN prepare_read_item_last_curve_edmi_iec(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    extern READ_FLAG_DAY_HOLD read_meter_flag_last_curve_cycle_day;
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB library;
    INT16U meter_idx,mask_idx,idx_td;
    INT8U idx,idx_sub;
    INT8U load_td[6] = {0};

    mask_idx = 0;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);

    if (get_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE) return FALSE;
    if (check_is_all_ch(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_CURVE,0x00))
    {
        if((read_params->meter_doc.protocol == METER_EDMI))
        {
            if(TRUE == edmi_to_exit_state(read_params))
            {
                *frame_len = edmi_read_ctrl(read_params,NULL,0,frame,0,NULL);
                return TRUE;
            }
            read_params->read_ctrl_state = 0;
            read_params->read_ctrl_step = 0;
        }
        #ifdef __READ_IEC1107_METER__
        if(read_params->meter_doc.protocol == IEC1107)
        {
            if(TRUE == iec1107_to_exit_state(read_params))
            {
                *frame_len = iec1107_read_ctrl(read_params,frame,0,NULL);
                if(*frame_len > 0)
                    return TRUE;
            }
        }
        #endif
        clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }

    if((GB645_2007 != read_params->meter_doc.protocol)&&(IEC1107 != read_params->meter_doc.protocol)&&(METER_EDMI != read_params->meter_doc.protocol))// 07,IEC1107,edmi��, �Ų������ɼ�¼
    {
        clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
        return FALSE;
    }
            
    for(idx=0;idx<sizeof(CRUVE_PHY_LIST)/sizeof(READ_WRITE_DATA);idx++)
    {
        for(idx_sub=0;idx_sub<((CRUVE_PHY_LIST[idx].flag&0x0F)+1);idx_sub++)
        {
            if(get_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,mask_idx))  //���48��
            {
                get_phy_data((READ_WRITE_DATA*)&(CRUVE_PHY_LIST[idx]),idx_sub,&phy);

//                for(idx_td=read_params->patch_num; idx_td<(96*DEFAULT_CURVE_HOLD_PATCH_DAY_COUNT); idx_td++) //��ѯ96������
                for(idx_td=read_params->patch_num; idx_td<(4*ONCE_PATCH_X_HOUR_CURVE); idx_td++) //��ѯ24������
                {
                    calculat_curve_time(read_params->patch_start_time,read_params->patch_cur_time,idx_td,15);
                    if(DateTimeCompareToMinute(read_params->patch_cur_time,datetime+MINUTE)>0)
                        goto NEXT_MASK;    
                    if((readdata_curve(meter_idx,&phy,read_params->patch_cur_time,frame,frame_len) == FALSE) && (read_params->patch_cur_time[2]!=0) && (read_params->patch_cur_time[3]!=0) && (!mem_all_is(read_params->patch_cur_time,5,0xFF)))
                    {
                        //��Ҫ����
                        if(get_data_item(phy.phy,read_params->meter_doc.protocol,&library))
                        {
                            if(library.item != 0xFFFFFFFF)
                            {
                                int32u2_bin(phy.phy,read_params->phy);
                                int32u2_bin(library.item,read_params->item);
                                read_params->item_len = library.len;
                                read_params->item_format = library.format;
                                read_params->resp_byte_num = 40;
                                read_params->read_type = READ_TYPE_LAST_CURVE_CYCLE_DAY;
                                read_params->patch_num = idx_td;
                                if (read_params->meter_doc.protocol == GB645_2007)
                                {
                                    load_td[0] = 1;//����
                                    load_td[1] = byte2BCD(read_params->patch_cur_time[0]);
                                    load_td[2] = byte2BCD(read_params->patch_cur_time[1]);
                                    load_td[3] = byte2BCD(read_params->patch_cur_time[2]);
                                    load_td[4] = byte2BCD(read_params->patch_cur_time[3]);
                                    load_td[5] = byte2BCD(read_params->patch_cur_time[4]);

                                    *frame_len = make_gb645_2007_read_frame(frame,read_params->meter_doc.meter_no,library.item,load_td,6);
                                }
                                #ifdef __READ_IEC1107_METER__
                                else if((read_params->meter_doc.protocol == IEC1107))
                                {
                                    *frame_len = iec1107_read_ctrl(read_params,frame,1,read_params->patch_cur_time);
                                }
                                #endif
								else if(read_params->meter_doc.protocol == METER_EDMI)
	                            {
                                    *frame_len = edmi_read_ctrl(read_params,&(library.item),1,frame,1,read_params->patch_cur_time);
	                            }
                                else
                                {
                                    clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
                                    return FALSE;
                                }
                                
                                if(*frame_len>0)
                                {
                                    #ifdef __SOFT_SIMULATOR__
                                    snprintf(info,200,"*** prepare item patch curve : meter_idx = %d , item = 0x%08X , phy = 0x%08X , mask_idx = %03d, time:%02d-%02d-%02d-%2d-%02d",
                                            meter_idx,library.item,phy.phy,mask_idx,read_params->patch_cur_time[4],read_params->patch_cur_time[3],read_params->patch_cur_time[2],
                                            read_params->patch_cur_time[1],read_params->patch_cur_time[0]);
                                    debug_println_ext(info);
                                    #endif
                                    read_params->cur_mask_idx = mask_idx;
                                    return TRUE;
                                }
                                else
                                {
                                }
                            }
                            else
                            {
                                //����msak
                                clear_read_mask_from_meter_param(meter_idx,READ_TYPE_LAST_CURVE_CYCLE_DAY,mask_idx);
                            }
                        }
                        else
                        {
                            //����msak
                            clear_read_mask_from_meter_param(meter_idx,READ_TYPE_LAST_CURVE_CYCLE_DAY,mask_idx);
                        }
                    }
                    else if((read_params->patch_cur_time[2]==0) || (read_params->patch_cur_time[3]==0))   //����bug������������������
                    {
                        break;
                    }
                }
                #ifdef __READ_IEC1107_METER__
                //����������һ����������������������ݣ��ȼ�����룬��һ�β������������ݣ������е㲹��֮�������������
                if((read_params->meter_doc.protocol == IEC1107))
                {
                    if(idx_td >= 4 * ONCE_PATCH_X_HOUR_CURVE)
                    {
                        //һ֡�����������ݣ�����ȫ��������ɣ������ѭ���������
                        mem_set(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_LAST_DAY_HOLD_CURVE,0x00);
                    }
                }
                #endif
NEXT_MASK:                
                clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_CURVE,mask_idx);
                read_params->patch_num = 0;
            }            
            mask_idx++;
        }
    }
    
    if((read_params->meter_doc.protocol == METER_EDMI))
    {
        if(TRUE == edmi_to_exit_state(read_params))
        {
            *frame_len = edmi_read_ctrl(read_params,NULL,0,frame,0,NULL);
            return TRUE;
        }
        read_params->read_ctrl_state = 0;
        read_params->read_ctrl_step = 0;
    }
    #ifdef __READ_IEC1107_METER__
    else if(read_params->meter_doc.protocol == IEC1107)
    {
        if(TRUE == iec1107_to_exit_state(read_params))
        {
            *frame_len = iec1107_read_ctrl(read_params,frame,0,NULL);
            if(*frame_len > 0)
                return TRUE;
        }
    }
    #endif
    clr_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
    return FALSE;
}
#endif
INT8U save_sichuan_patch_curve_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,INT8U* buffer)
{
    READ_WRITE_DATA phy;
    INT32U phy_u32;
    INT8U  phy_num,i;
    INT8U idx;
    INT8U data[200]={0},td_load[5],data_len;
    INT8U* pdata;
    INT8U curve_save_flag = 0x55;

    phy_u32 = bin2_int32u(read_params->phy);
    idx = get_phy_form_list_cruve(phy_u32,&phy);
    if(idx == 0xFF) return 0;

    if ((phy.flag & SAVE_FLAG_DENY_NO_SAVE) && (frame[0] == 0xEE))
    {
        clr_bit_value(read_params->read_mask.last_day_curve_hold,READ_MASK_BYTE_NUM_CURVE,idx);
        read_params->patch_num = 0;
    }
    else if(frame_len==0)//�����ĸ��ɼ�¼������
    {
        read_params->patch_num++;
    }
    else 
    {
        pdata = data;
        mem_cpy(td_load,read_params->patch_cur_time,5);
        phy_num = extract_load_record_data_sichuan(phy_u32,td_load,frame,frame_len,pdata);
       
        if(phy_num <= 0)
        {
            read_params->patch_num++;
            return 0;
        }
        
        for(i=0; i<phy_num; i++)
        {
            phy_u32 = bin2_int32u(pdata);
            idx = get_phy_form_list_cruve(phy_u32,&phy);
            if(idx == 0xFF) return 0;

            data_len = pdata[4];
            writedata_curve(read_params,&phy,td_load,pdata+5,data_len,buffer,curve_save_flag);

            writedata_precision_cycle_day_and_curve(bin2_int16u(read_params->meter_doc.meter_idx),&phy,td_load,pdata+5,data_len,buffer,0,0xAA);

            pdata += 5+data_len;
        }
        if(read_params->delay_read_flag == 0xAA)    //���ɼ�¼����ʱ�겻��ȷ��������һ��
        {
            read_params->patch_num++;
        }
    }
    return 0;
}


