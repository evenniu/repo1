#include "app_dev_plat.h"
#include "math.h"
#include "protocol_library_hengtong.h"
#include "protocol_library_edmi.h"
#include "protocol_library_iec1107.h"
#ifdef __READ_OOP_METER__
#include "protocol_library_oop.h"
#endif
#include "protocol_library_dlms.h"
void plc_router_save_meter_event_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len);
void save_meter_event_record(INT16U meter_idx,READ_PARAMS *read_params,INT8U *frame,INT16U framelen,report_ctrl reportCtrl);
void plc_router_save_cjq_meter_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen,BOOLEAN is_catch);
void plc_router_save_report_meter_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen);
void plc_router_save_report_meter_ext_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen);
INT8U save_read_data_cjq_update_task(READ_PARAMS *read_params,INT8U* frame,INT8U *frame_len);
#ifdef __VOLTAGE_MONITOR__
void plc_router_save_valtage_data_standard(READ_PARAMS *read_params,INT8U *data,INT8U datalen);
void plc_router_save_valtage_data(READ_PARAMS *read_params,INT8U *data,INT8U datalen);
#endif
#ifdef __INSTANT_FREEZE__
void save_instant_freeze_data(READ_PARAMS *read_params,INT8U *data,INT8U datalen);
#endif
INT32U get_curve_normal_phy(INT32U oop_oad);
INT8U check_validity_curdata_to_holddata(INT8U port,INT8U phy_idx,INT8U read_type )
{
    INT8U idx;
    BOOLEAN cy_need_read_dh = FALSE;
    if(phy_idx == 0xFF) return FALSE;
    
    if(port == COMMPORT_485_CY)
    {
        if(TRUE == check_const_ertu_switch(CONST_ERTU_SWITCH_CY_READ_HOLD_ITEM))
        {
            cy_need_read_dh = TRUE;
        }
        else
        {
            return TRUE;
        }
    }

    
    if(read_type == READ_TYPE_CYCLE_DAY) idx = READ_MASK_DAY_HOLD_FWG_ZDXL_TIME_97_CUR_4;
    else if(read_type == READ_TYPE_CYCLE_MONTH) idx = READ_MASK_MONTH_HOLD_FWG_ZDXL_TIME_CUR_4;
    else return FALSE;        
    if(phy_idx > idx) return TRUE;

    if(TRUE == check_const_ertu_switch(CONST_ERTU_SWITCH_485_READ_DAYHOLD_ITEM)) return FALSE;

    if(cy_need_read_dh) return FALSE;
    
    if(FALSE == check_const_ertu_switch(CONST_ERTU_SWITCH_485_REC_IN_SEG)) return TRUE;
    if(check_is_in_rec_seg(port,datetime+MINUTE)) return TRUE;
    
    return FALSE;  
}
INT8U save_read_data_GB645_2007(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
/*
 * INT32U item;
    INT16U block_begin_idx = 0,idx_1;
    READ_WRITE_DATA phy;
    tagPROTOCOL_LIB pLib;
    //READ_WRITE_DATA *phy_ptr;
    READ_PARAMS read_params_tmp;
    //INT8U* td;
    INT8U idx;
	INT8U block_count = 0;
    INT8U curve_save_flag;//midu,
    #if ((defined __PLC_REC_VOLTMETER1__) || (defined __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__))
    INT8U i;
    #endif
    INT8U buffer[300]={0};
    curve_save_flag = 0;
    INT8U midu,valid_bcd;
    //BATCH_TRANSPARENT_METER_TASK_HEADER header;

    if (check_frame_body_gb645(frame,*frame_len) == FALSE) return 0;

    if (check_is_all_ch(read_params->meter_doc.meter_no,6,0xAA) == FALSE)
    {
        if (compare_string(read_params->meter_doc.meter_no,frame+1,6) != 0) return 0;

        {
           if(check_meter_idx_same_or_not(read_params,frame,frame_len,buffer) == FALSE)
           {
               return 0;
           }
        }
    }
      
    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    if (read_params->read_type == READ_TYPE_BATCH_TRANSPARENT_METER_TASK)
    {
        save_batch_transparent_meter_task(bin2_int16u(read_params->meter_doc.meter_idx),&(read_params->meter_doc),bin2_int32u(read_params->item),frame,*frame_len);
        return 0;
    }
    #endif
        #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
    if (read_params->read_type == READ_TYPE_BATCH_TRANSPARENT_METER_CYCLE_TASK)
    {
        save_batch_transparent_meter_cycle_task(read_params,frame);
        return 0;
    }
    #endif
    */
   #ifdef __COUNTRY_ISRAEL__
   //�㲥�����������Ӧ����
   if(bin2_int32u(read_params->item) == 0xFFFFFFAA)
   {
       //�㲥��������
       fread_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASK + bin2_int16u(read_params->meter_doc.meter_idx),&valid_bcd,1);
       valid_bcd &= ~0x03;
       if( (*frame) & 0x40)
       {
          //���Ӧ������ʧ�ܣ�   0x02
          valid_bcd |= 0x02;
       }
       else
       {
          //���Ӧ�����óɹ���  0x01
          valid_bcd |= 0x01;
       }
       fwrite_array(FILEID_PLC_CAST_SET_TASK,PIM_PLC_CAST_TASK + bin2_int16u(read_params->meter_doc.meter_idx),&valid_bcd,1);
       return 0;
   }
   #endif
    if ((frame[POS_GB645_CTRL] & 0x80) == 0) return 0;

    if (((frame[POS_GB645_CTRL] & 0x1F) == 0x11) || ((frame[POS_GB645_CTRL] & 0x1F) == 0x12)) //�����ݺͶ���������
    {
        if (frame[POS_GB645_CTRL] & 0x40) //�쳣Ӧ��
        {

            (void)get_data_item(bin2_int32u(read_params->phy),GB645_2007,&pLib);

            mem_set(frame+14,pLib.len,0xEE);

            frame[9] = pLib.len+4;

            switch(read_params->read_type)
            {
            case READ_TYPE_CYCLE_DAY:
                save_cycle_day(read_params,frame+14,frame[9]-4,buffer);
                #ifndef __CHECK_MONTH_HOLD_TD__
                if (read_params->control.month_hold_mode == 0)
                {
                    idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
					// �ж�idx  0xFF ˵����Ч����������δ��� ??�ݶ���ȥ�洢��
                    if(0xFF != idx)
                    {
                        if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                        {
                            save_cycle_month(read_params,frame+14,frame[9]-4,buffer);
                        }
                    }
                }
                #endif
                #ifdef __PROVICE_NEIMENG__
                /*�ն��᳭��ʵʱ����ʱת��ɵ�ǰ����*/
                if((read_params->meter_doc.baud_port.port == COMMPORT_PLC)||(read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
                {
                    //mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                    /*��ȡ������*/
                    idx_1 = get_phy_form_list_cur_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count);
                    if(idx_1 != 0xFFFF)
                    {
                        if(get_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx_1))
                        {
                            /*�����Ƿ��ϣ�����ֱ���������*/
                            save_cur_date(read_params,frame+14,frame[9]-4,buffer);
                            clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx_1);
                        }
                    }
                }
                #endif
                break;
            case READ_TYPE_CYCLE_MONTH:
                save_cycle_month(read_params,frame+14,frame[9]-4,buffer);
                break;
            case READ_TYPE_CURVE:
                save_curve_data(read_params,frame+14,frame[9]-4,buffer,curve_save_flag);
                break;
            case READ_TYPE_CUR_DATA:
                save_cur_date(read_params,frame+14,frame[9]-4,buffer);
                //485��������
                if((read_params->meter_doc.baud_port.port != COMMPORT_PLC) && (read_params->meter_doc.baud_port.port != COMMPORT_PLC_REC)
                    && (read_params->read_load_curve==0)) /*read_load_curve��ʶ�������ɼ�¼���������ݲ��ܴӵ�ǰ����ת��*/
                {
                    curve_save_flag = 0x55;
                    save_curve_data(read_params,frame+14,frame[9]-4,buffer,curve_save_flag);
                }
                break;
            case READ_TYPE_DAY_HOLD_PATCH:
                save_patch_day_hold(read_params,frame+14,frame[9]-4,buffer);
                break;
            case READ_TYPE_DAY_HOLD_WAIT_TD:
                save_patch_day_hold_wait_td(read_params,frame+14,frame[9]-4,buffer);
                break;
            #ifdef __CHECK_MONTH_HOLD_TD__
            case READ_TYPE_MONTH_HOLD_WAIT_TD:
                save_patch_month_hold_wait_td(read_params,frame+14,frame[9]-4,buffer);
                break;
            #endif
            case READ_TYPE_CYCLE_METER_EVENT:
                plc_router_save_meter_event_data(read_params,frame+POS_GB645_DLEN,frame[POS_GB645_DLEN]);//��ʶ+����
                break;
            #ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
            case READ_TYPE_CURVE_HUNAN:
                save_curve_data_hunan(read_params,frame+14,frame[9]-4,buffer);
                //#ifdef __ZhuZhou_CITY__
                save_cur_date(read_params,frame+14,frame[9]-4,buffer);
                //#endif
                break;
            #endif
            #ifdef __METER_EVENT_REPORT__
            case READ_TYPE_EVENT_TAIL_REPORT:
                plc_router_save_report_meter_event_data(bin2_int16u(read_params->meter_doc.meter_idx),
                        read_params->meter_doc.meter_no,bin2_int32u(read_params->item),frame+14,frame[9]-4);
                break;
            case READ_TYPE_EVENT_TAIL_REPORT_EXT:
                plc_router_save_report_meter_ext_event_data(bin2_int16u(read_params->meter_doc.meter_idx),
                        read_params->meter_doc.meter_no,bin2_int32u(read_params->item),frame+14,frame[9]-4);
                break;
            #endif
            case READ_TYPE_LAST_CURVE_CYCLE_DAY:
                save_last_curve_cycle_day(read_params,frame+14,frame[9]-4,buffer);
                break;

            #ifdef __VOLTAGE_MONITOR__
            case READ_TYPE_VOLTAGE_MONITOR_TASK:
                if (get_system_flag(SYS_VOLTAGE_MONITOR,SYS_FLAG_BASE))
                {
                    plc_router_save_valtage_data_standard(read_params,frame+14,frame[9]-4);
                }
                else
                {
                    plc_router_save_valtage_data(read_params,frame+14,frame[9]-4);
                }
                break;
            #endif
            #ifdef __INSTANT_FREEZE__
            case READ_TYPE_INSTANT_FREEZE:
                   save_instant_freeze_data(read_params,frame+10,frame[9]);
                   break;
            #endif

            #ifdef __METER_DAY_FREEZE_EVENT__
            case READ_TYPE_CYCLE_FREEZE_EVENT:
                save_freeze_event_data(read_params,frame+POS_GB645_07_DATA,frame[POS_GB645_DLEN]-4,buffer);
                break;
            #endif
            #ifdef __PROVICE_JIANGSU__
            case READ_TYPE_CJQ_INFO:
                save_cjq_info_data(read_params,frame+10,frame[9]);
               break;
            #endif
            #ifdef __PROVICE_SHAANXI_CHECK__
            case READ_METER_TYPE_INFO:/*�����жϵ��Э��ĳ�������*/
               save_meter_type_data(read_params,frame+14,frame[9]-4);
               break;
            #endif
            }
            return 1;
        }
        else
        {
            for(idx=0;idx<frame[9];idx++)
            {
                frame[10+idx] = frame[10+idx] - 0x33;
            }

            if (((compare_string(read_params->item,frame+10,4)) == 0)
                || ((check_is_sqr_protocol(read_params->meter_doc.protocol)) &&((compare_string(read_params->item,frame+10,2)) == 0)))
            {                
                switch(read_params->read_type)
                {
                case READ_TYPE_CYCLE_DAY:
//                    rs232_debug_info("\xA1\xA1",2);
//                    rs232_debug_info(frame+10,4);
                    save_cycle_day(read_params,frame+14,frame[9]-4,buffer);
                    #ifndef __CHECK_MONTH_HOLD_TD__
                    if (read_params->control.month_hold_mode == 0)
                    {
                        #ifdef __PROVICE_GANSU__
                        if(read_params->meter_doc.meter_class.meter_class == 6)   
                        {
                            if(bin2_int32u(read_params->phy)==DR_LD_DL) 
                            {
                                int32u2_bin(0x00000040,read_params->phy);
                            }
                            else if(bin2_int32u(read_params->phy)==0x0000263F)  
                            {
                                int32u2_bin(0x0000007F,read_params->phy);
                            }
                        }
                        #endif

                        idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
                        #if ((defined __PLC_REC_VOLTMETER1__) || (defined __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__))
                        if((phy.phy == DY_A_HGL) || (phy.phy == DY_B_HGL) || (phy.phy == DY_C_HGL) )
                        {
                          i++;
                        }
                        else
                        #endif
                        {
                            // �ж�idx  0xFF ˵����Ч����������δ��� ??�ݶ���ȥ�洢��
                            if(0xFF != idx)
                            {
                                if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                                {
//                                    rs232_debug_info("\xA2\xA2",2);
//                                    rs232_debug_info(frame+10,4);
                                    save_cycle_month(read_params,frame+14,frame[9]-4,buffer); 
                                }
                            }
                        }
                    }
                    #endif
                    #ifdef __PROVICE_NEIMENG__
                    /*�ն��᳭��ʵʱ����ʱת��ɵ�ǰ����*/
                    if((read_params->meter_doc.baud_port.port == COMMPORT_PLC)||(read_params->meter_doc.baud_port.port == COMMPORT_PLC_REC))
                    {
                        //mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                        idx_1 = get_phy_form_list_cur_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count);
                        if(idx_1 != 0xFFFF)
                        {
                            if(get_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx_1))
                            {
                                /*�洢ʵʱ����*/
                                save_cur_date(read_params,frame+14,frame[9]-4,buffer);
                                /*�������*/
                                clr_bit_value(read_params->read_mask.cur_data,READ_MASK_BYTE_NUM_CUR_DATA,idx_1);
                            }
                        }
                    }
                    #endif
                    break;
                case READ_TYPE_CYCLE_MONTH:
//                    rs232_debug_info("\xA3\xA3",2);
//                    rs232_debug_info(frame+10,4);
                    save_cycle_month(read_params,frame+14,frame[9]-4,buffer);
                    break;
                case READ_TYPE_CURVE:
                    save_curve_data(read_params,frame+14,frame[9]-4,buffer,curve_save_flag);
                    #ifdef __PROVICE_JIANGXI__
                    save_cur_date(read_params,frame+14,frame[9]-4,buffer);/*����תʵʱ*/
                    #endif
                    break;
                case READ_TYPE_CUR_DATA:
                    save_cur_date(read_params,frame+14,frame[9]-4,buffer);
                    //485��������
                    if((read_params->meter_doc.baud_port.port != COMMPORT_PLC) && (read_params->meter_doc.baud_port.port != COMMPORT_PLC_REC)
                        && (read_params->read_load_curve==0)) /*read_load_curve��ʶ�������ɼ�¼���������ݲ��ܴӵ�ǰ����ת��*/
                    {
                        curve_save_flag = 0x55;
                        save_curve_data(read_params,frame+14,frame[9]-4,buffer,curve_save_flag);
                    }

                    if((read_params->meter_doc.baud_port.port == COMMPORT_485_CY) || (read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS))
                    {
                        mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                        trans_phy_cur_2_cycle_day(read_params_tmp.phy);
                        //if (check_is_all_ch(read_params_tmp.day_hold_td,3,0x00)) get_yesterday(read_params_tmp.day_hold_td);
                        get_yesterday(read_params_tmp.day_hold_td);//��ʱ����̨����� 
                        idx = get_phy_form_list_cycle_day(bin2_int32u(read_params_tmp.phy),&phy); 
                        if(check_validity_curdata_to_holddata(read_params_tmp.meter_doc.baud_port.port,idx,READ_TYPE_CYCLE_DAY))
                        {
                            if(readdata_cycle_day(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.day_hold_td,buffer,buffer+5,frame_len,NULL) == FALSE)
                            {
//                                rs232_debug_info("\xA0\xA0",2);
//                                rs232_debug_info(frame+10,4);
                                save_cycle_day(&read_params_tmp,frame+14,frame[9]-4,buffer);
                            }
                        }
                        #ifdef __CHECK_MONTH_HOLD_TD__
                        if(read_params->meter_doc.baud_port.port == COMMPORT_485_CY)
                        {
                            if(check_is_all_ch(read_params->month_hold_td,2,0x00)) get_former_month(read_params_tmp.month_hold_td);

                            idx = get_phy_form_list_cycle_month(bin2_int32u(read_params_tmp.phy),&phy);
                            if(check_validity_curdata_to_holddata(read_params_tmp.meter_doc.baud_port.port,idx,READ_TYPE_CYCLE_MONTH))
                            {                            
                                if(readdata_cycle_month(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                                {                                          
                                    save_cycle_month(&read_params_tmp,frame+14,frame[9]-4,buffer);
                                }
                            }
                        }
                        #else
                        //�¶��ỹδ����
                        if(check_is_all_ch(read_params->month_hold_td,2,0x00)) get_former_month(read_params_tmp.month_hold_td);

                        idx = get_phy_form_list_cycle_month(bin2_int32u(read_params_tmp.phy),&phy);
                        if(check_validity_curdata_to_holddata(read_params_tmp.meter_doc.baud_port.port,idx,READ_TYPE_CYCLE_MONTH))
                        {                            
                            if(readdata_cycle_month(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                            {                                          
                                save_cycle_month(&read_params_tmp,frame+14,frame[9]-4,buffer);
                            }
                        }
                        #endif
                             //�����ն�������
                        //#ifndef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
                        // if(read_params->hold_flag.rec_day)
                        {
                            if(TRUE == is_recday_rs485_port(read_params->meter_doc.baud_port.port))
                            {
                                if(0xFF != get_phy_form_list_recday_data(bin2_int32u(read_params->phy),&phy))
                                {
                                    if(readdata_recday_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,datetime+DAY,buffer,buffer+5,frame_len) == FALSE)
                                    {
                                        save_recday_data(read_params,frame+14,frame[9]-4,buffer);
                                    }
                                }
                            }
                        }
                        //#endif

                        if(0xFF != get_phy_from_list_day_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                        {
                            readdata_day_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
//                            if(0 != compare_string(buffer+2,datetime+DAY,3))//rec_date
                            if(0 != compare_string(buffer+2,read_params->init_data_rec_time+2,3))
                            {
                                save_day_init_data(read_params,frame+14,frame[9]-4,buffer);
                            }
                        }
                        if(0xFF != get_phy_from_list_month_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                        {
                            readdata_month_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                            if(0 != compare_string(buffer+3,read_params->init_data_rec_time+3,2))
                            {
//                                rs232_debug_info("\xA4\xA4",2);
                                save_month_init_data(read_params,frame+14,frame[9]-4,buffer);
                            }
                        }
                    }
                    break;
                case READ_TYPE_DAY_HOLD_PATCH:
                    save_patch_day_hold(read_params,frame+14,frame[9]-4,buffer);
                    break;
                case READ_TYPE_DAY_HOLD_WAIT_TD:
                    save_patch_day_hold_wait_td(read_params,frame+14,frame[9]-4,buffer);
                    break;
                #ifdef __CHECK_MONTH_HOLD_TD__
                case READ_TYPE_MONTH_HOLD_WAIT_TD:
                    save_patch_month_hold_wait_td(read_params,frame+14,frame[9]-4,buffer);
                    break;
                #endif
                case READ_TYPE_CYCLE_METER_EVENT:
                plc_router_save_meter_event_data(read_params,frame+POS_GB645_DLEN,frame[POS_GB645_DLEN]);// ��ʶ+����
                break;
		#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
		case READ_TYPE_CURVE_HUNAN:
                    save_curve_data_hunan(read_params,frame+14,frame[9]-4,buffer);
                    //#ifdef __ZhuZhou_CITY__
                    save_cur_date(read_params,frame+14,frame[9]-4,buffer);
                    //#endif
                break;
		#endif
                #ifdef __METER_EVENT_REPORT__
                case READ_TYPE_EVENT_TAIL_REPORT:
                    plc_router_save_report_meter_event_data(bin2_int16u(read_params->meter_doc.meter_idx),
                    read_params->meter_doc.meter_no,bin2_int32u(read_params->item),frame+14,frame[9]-4);
                    break;
                case READ_TYPE_EVENT_TAIL_REPORT_EXT:
                    plc_router_save_report_meter_ext_event_data(bin2_int16u(read_params->meter_doc.meter_idx),
                    read_params->meter_doc.meter_no,bin2_int32u(read_params->item),frame+14,frame[9]-4);
                    break;
                #endif
                case READ_TYPE_LAST_CURVE_CYCLE_DAY:
                     save_last_curve_cycle_day(read_params,frame+14,frame[9]-4,buffer);
                     break;

                #ifdef __VOLTAGE_MONITOR__
                case READ_TYPE_VOLTAGE_MONITOR_TASK:
                    if (get_system_flag(SYS_VOLTAGE_MONITOR,SYS_FLAG_BASE))
                    {
                        plc_router_save_valtage_data_standard(read_params,frame+14,frame[9]-4);
                    }
                    else
                    {
                        plc_router_save_valtage_data(read_params,frame+14,frame[9]-4);
                    }
                    break;
                #endif
                #ifdef __INSTANT_FREEZE__
                case READ_TYPE_INSTANT_FREEZE:
                   save_instant_freeze_data(read_params,frame+10,frame[9]);
                   break;
                #endif

				#ifdef __METER_DAY_FREEZE_EVENT__
				case READ_TYPE_CYCLE_FREEZE_EVENT:
					save_freeze_event_data(read_params,frame+POS_GB645_07_DATA,frame[POS_GB645_DLEN]-4,buffer);
					break;
				#endif
#ifdef __PROVICE_JIANGSU__
                case READ_TYPE_CJQ_INFO:
                   save_cjq_info_data(read_params,frame+10,frame[9]);
                   break;
#endif
                #ifdef __PROVICE_SHAANXI_CHECK__
                case READ_METER_TYPE_INFO:
                   save_meter_type_data(read_params,frame+14,frame[9]-4);
                   break;
                #endif
                }
                return 1;
            }
        }
    }
    else if ((frame[POS_GB645_CTRL] & 0x1F) == 0x03)
    {
        if ((frame[POS_GB645_CTRL] & 0x40) == 0)
        {
            for(idx=0;idx<frame[POS_GB645_DLEN];idx++)
            {
                frame[POS_GB645_ITEM+idx] = frame[POS_GB645_ITEM+idx] - 0x33;
            }

            item = bin2_int32u(frame+POS_GB645_ITEM);
            if(item == 0x070000FF)
            {
#ifdef __ENABLE_ESAM2__
                save_batch_meter_task_auth(read_params,frame+POS_GB645_07_DATA,frame[POS_GB645_DLEN]-4);
#endif
            }
            return 1;
        }
    }
    else if ((frame[POS_GB645_CTRL] & 0x1F) == 0x14)
    {
        if (bin2_int32u(read_params->item) == 0x0400010C)  //esam��ʱ
        {
#ifdef __ENABLE_ESAM2__
            if ((frame[POS_GB645_CTRL] & 0x40) == 0) //ȷ��
            {
                save_batch_meter_task_time(read_params,TRUE);
            }
            else
            {
                save_batch_meter_task_time(read_params,FALSE);
            }
#endif
            return 1;
        }
		if(bin2_int32u(read_params->item) == 0x04001503)
		{
			#ifdef __METER_EVENT_REPORT__
			if ((frame[POS_GB645_CTRL] & 0x40) == 0) //ȷ��
            {
                plc_router_save_report_meter_event_data(bin2_int16u(read_params->meter_doc.meter_idx),
                            read_params->meter_doc.meter_no,bin2_int32u(read_params->item),frame+14,frame[9]-4);
            }
            else
            {
                mem_set(frame+14,20,0xEE);
            	frame[9] = 20+4;
				plc_router_save_report_meter_event_data(bin2_int16u(read_params->meter_doc.meter_idx),
                            read_params->meter_doc.meter_no,bin2_int32u(read_params->item),frame+14,frame[9]-4);
            }
			#endif
			return 1;
		}
		if(bin2_int32u(read_params->item) == 0x04001508)
		{
			#ifdef __METER_EVENT_REPORT__
			if ((frame[POS_GB645_CTRL] & 0x40) == 0) //ȷ��
            {
                plc_router_save_report_meter_ext_event_data(bin2_int16u(read_params->meter_doc.meter_idx),
                            read_params->meter_doc.meter_no,bin2_int32u(read_params->item),frame+14,frame[9]-4);
            }
            else
            {
                mem_set(frame+14,20,0xEE);
            	frame[9] = 20+4;
				plc_router_save_report_meter_ext_event_data(bin2_int16u(read_params->meter_doc.meter_idx),
                            read_params->meter_doc.meter_no,bin2_int32u(read_params->item),frame+14,frame[9]-4);
            }
			#endif
			return 1;
		}
    }
    return 0;
}

INT8U save_read_data_GB645_1997(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT32U phy_u32;
    READ_WRITE_DATA phy;
    //READ_WRITE_DATA *phy_ptr;
    tagPROTOCOL_LIB pLib_tmp;
    READ_PARAMS read_params_tmp;
    INT16U len,left_len,block_begin_idx;
    //INT8U* td;
    INT8U idx,block_count;
    INT8U curve_save_flag;//midu,
    INT8U buffer[300];
	#ifdef __POWER_CTRL__
	INT8U tmp_buf[50]={0};
	#endif
    METER_DOCUMENT pmeter_doc;
    mem_cpy(pmeter_doc.meter_idx, read_params->meter_doc.meter_idx,2);
    mem_cpy(pmeter_doc.spot_idx, read_params->meter_doc.spot_idx,2);
    mem_cpy(&(pmeter_doc.baud_port),&(read_params->meter_doc.baud_port),sizeof(BAUD_PORT));
    mem_cpy(pmeter_doc.meter_no, read_params->meter_doc.meter_no,6);
    mem_cpy(pmeter_doc.password, read_params->meter_doc.password,6);
    pmeter_doc.fl_count=read_params->meter_doc.fl_count;
    mem_cpy(&(pmeter_doc.mbit_info), &(read_params->meter_doc.mbit_info),sizeof(MBIT_INFO));
    mem_cpy(pmeter_doc.rtu_no, read_params->meter_doc.rtu_no,6);
    mem_cpy(&(pmeter_doc.meter_class),&(read_params->meter_doc.meter_class),sizeof(METER_CLASS));
    pmeter_doc.protocol = GB645_1997;

    curve_save_flag = 0;
	mem_set(buffer,sizeof(buffer),0x00);
    if (check_frame_body_gb645(frame,*frame_len) == FALSE) return 0;
    if (compare_string(read_params->meter_doc.meter_no,frame+1,6) != 0) return 0;
    if(read_params->meter_doc.baud_port.port ==  COMMPORT_PLC)/*ֻ���ز����ж�*/
    {
       if(check_meter_idx_same_or_not(read_params,frame,frame_len,buffer) == FALSE)
       {
           return 0;
       }
    }
    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    if (read_params->read_type == READ_TYPE_BATCH_TRANSPARENT_METER_TASK)
    {
        save_batch_transparent_meter_task(bin2_int16u(read_params->meter_doc.meter_idx),&(read_params->meter_doc),bin2_int32u(read_params->item),frame,*frame_len);
        return 0;
    }
    #endif

    if(frame[POS_GB645_CTRL] == 0xC0) goto SAVE_97;
    #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
    if (read_params->read_type == READ_TYPE_BATCH_TRANSPARENT_METER_CYCLE_TASK)
    {
        save_batch_transparent_meter_cycle_task(read_params,frame);
        return 0;
    }
    #endif
    if ((frame[POS_GB645_CTRL] & 0x80) == 0) return 0;

    if (((frame[8] & 0x1F) == 0x01) || ((frame[8] & 0x1F) == 0x02)) //�����ݺ��ض�����
    {
        if (frame[8] & 0x40) //�쳣Ӧ��
        {
            mem_set(frame+12,20,0xEE);
            frame[9] = 20+2;

            switch(read_params->read_type)
            {
            case READ_TYPE_CYCLE_DAY:
                save_cycle_day(read_params,frame+12,frame[9]-2,buffer);
                if (read_params->control.month_hold_mode == 0)
                {
                    idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
					// �ж�idx  0xFF ˵����Ч����������δ��� ??�ݶ���ȥ�洢��
                    if(0xFF != idx)
                    {
                        if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                        {
                            save_cycle_month(read_params,frame+12,frame[9]-2,buffer);
                        }
                    }
                }
                break;
            case READ_TYPE_CYCLE_MONTH:
                save_cycle_month(read_params,frame+12,frame[9]-2,buffer);
                break;
            case READ_TYPE_CURVE:
                save_curve_data(read_params,frame+12,frame[9]-2,buffer,curve_save_flag);
                break;
            case READ_TYPE_CUR_DATA:
                save_cur_date(read_params,frame+12,frame[9]-2,buffer);
                //485��������
                if((read_params->meter_doc.baud_port.port != COMMPORT_PLC) && (read_params->meter_doc.baud_port.port != COMMPORT_PLC_REC))
                {
                    curve_save_flag = 0x55;
                    save_curve_data(read_params,frame+12,frame[9]-2,buffer,curve_save_flag);
                }
                break;
            case READ_TYPE_CYCLE_METER_EVENT:
                plc_router_save_meter_event_data(read_params,frame+12,frame[9]-2);
                break;
			#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
			case READ_TYPE_CURVE_HUNAN:
                save_curve_data_hunan(read_params,frame+12,frame[9]-2,buffer);
                //#ifdef __ZhuZhou_CITY__
                save_cur_date(read_params,frame+12,frame[9]-2,buffer);
                //#endif
                break;
			#endif
            case READ_TYPE_LAST_CURVE_CYCLE_DAY:
                save_last_curve_cycle_day(read_params,frame+12,frame[9]-2,buffer);
                break;
            #ifdef __VOLTAGE_MONITOR__
            case READ_TYPE_VOLTAGE_MONITOR_TASK:
                if (get_system_flag(SYS_VOLTAGE_MONITOR,SYS_FLAG_BASE))
                {
                    plc_router_save_valtage_data_standard(read_params,frame+14,frame[9]-4);
                }
                else
                {
                    plc_router_save_valtage_data(read_params,frame+14,frame[9]-4);
                }
                break;
            #endif
            }
            return 1;
        }
        else
        {
 SAVE_97:
            for(idx=0;idx<2;idx++)
            {
                frame[10+idx] = frame[10+idx] - 0x33;
            }
                phy_u32 = bin2_int32u(read_params->phy);
            if (((compare_string(read_params->item,frame+10,2)) == 0) && (get_data_item(phy_u32,GB645_1997,&pLib_tmp)))
            {            
                for(idx=0;idx<2;idx++)
                {
                    frame[10+idx] = frame[10+idx] + 0x33;
                }

              
                left_len = READPORT_RS485_FRAME_SIZE - (*frame_len);
                len = unwrap_gb645_frame(phy_u32,frame,(*frame_len),1,left_len,&pLib_tmp,&pmeter_doc,pLib_tmp.phy_son, read_params->patch_num); //�����ʱ�������й�����20�ֽڣ�������������й����֣��������5�ֽڣ���ͨ����4�ֽڣ���ô������
                wrap_phy2block(phy_u32,frame, len);

                switch(read_params->read_type)
                {
                case READ_TYPE_CYCLE_DAY:
                    save_cycle_day(read_params,frame+5,frame[4],buffer);
                    if (read_params->control.month_hold_mode == 0)
                    {
                        idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
						// �ж�idx  0xFF ˵����Ч����������δ��� ??�ݶ���ȥ�洢��
                        if(0xFF != idx)
                    	{
                            if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                            {
                                save_cycle_month(read_params,frame+5,frame[4],buffer);
                            }
                    	}
                    }
                    break;
                case READ_TYPE_CYCLE_MONTH:
                    save_cycle_month(read_params,frame+5,frame[4],buffer);
                    break;
                case READ_TYPE_CURVE:
                    save_curve_data(read_params,frame+5,frame[4],buffer,curve_save_flag);
                    break;
                case READ_TYPE_CUR_DATA:
                    save_cur_date(read_params,frame+5,frame[4],buffer);
                    //485��������
                    if((read_params->meter_doc.baud_port.port != COMMPORT_PLC) && (read_params->meter_doc.baud_port.port != COMMPORT_PLC_REC))
                    {
                        curve_save_flag = 0x55;
                        save_curve_data(read_params,frame+5,frame[4],buffer,curve_save_flag);
                    }
                    if((read_params->meter_doc.baud_port.port == COMMPORT_485_CY) || (read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS))
                    {
                        mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                        trans_phy_cur_2_cycle_day(read_params_tmp.phy);
                        if (check_is_all_ch(read_params_tmp.day_hold_td,3,0x00)) get_yesterday(read_params_tmp.day_hold_td);
                        idx = get_phy_form_list_cycle_day(bin2_int32u(read_params_tmp.phy),&phy); 
                        if(check_validity_curdata_to_holddata(read_params_tmp.meter_doc.baud_port.port,idx,READ_TYPE_CYCLE_DAY))
                        {
                        if(readdata_cycle_day(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.day_hold_td,buffer,buffer+5,frame_len,NULL) == FALSE)
                        {
                                save_cycle_day(&read_params_tmp,frame+5,frame[4],buffer);
                        }
                        }
                        //�¶��ỹδ����
                        if(check_is_all_ch(read_params->month_hold_td,2,0x00)) get_former_month(read_params_tmp.month_hold_td);
                        idx = get_phy_form_list_cycle_month(bin2_int32u(read_params_tmp.phy),&phy);
                        if(check_validity_curdata_to_holddata(read_params_tmp.meter_doc.baud_port.port,idx,READ_TYPE_CYCLE_MONTH))
                        {
                        if(readdata_cycle_month(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                        {
                                save_cycle_month(&read_params_tmp,frame+5,frame[4],buffer);
                        }
                        }
                        
                        //�����ն�������
                        #ifndef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
                        //if(read_params->hold_flag.rec_day)
                        {
                            if(TRUE == is_recday_rs485_port(read_params->meter_doc.baud_port.port))
                            {
                                if(0xFF != get_phy_form_list_recday_data(bin2_int32u(read_params->phy),&phy))
                                {
                                if(readdata_recday_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,datetime+DAY,buffer,buffer+5,frame_len) == FALSE)
                                {
                                        save_recday_data(read_params,frame+5,frame[4],buffer);
                                }
                              }
                           }
                        }
                        #endif
                        if(0xFF != get_phy_from_list_day_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                        {
                            readdata_day_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                            
                            if(0 != compare_string(buffer+2,read_params->init_data_rec_time+2,3))
                            {
                                    #ifdef __POWER_CTRL__
                                if((phy_u32 == 0x0000007F) && (read_params->meter_doc.protocol == 0))//������ճ�ֵ����25�ֽڴ洢
                                {
                                    get_pulse_meter_F33(bin2_int16u(read_params->meter_doc.spot_idx),buffer);
                                    mem_cpy(tmp_buf,buffer+6,25);
                                    save_day_init_data(read_params,tmp_buf,25,buffer);
                                }
                                else
                                #endif
                                {
                                     save_day_init_data(read_params,frame+5,frame[4],buffer);
                                }
                            }
                       }
                        if(0xFF != get_phy_from_list_month_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                        {    
                            readdata_month_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                            if(0 != compare_string(buffer+3,read_params->init_data_rec_time+3,2))
                            {
                              #ifdef __POWER_CTRL__
                                if((phy_u32 == 0x0000007F) && (read_params->meter_doc.protocol == 0))//������³�ֵ����25�ֽڴ洢
                                {
                                    get_pulse_meter_F33(bin2_int16u(read_params->meter_doc.spot_idx),buffer);
                                    mem_cpy(tmp_buf,buffer+6,25);
                                    save_month_init_data(read_params,tmp_buf,25,buffer);
                                }
                                else
                                #endif
                                {
                                        save_month_init_data(read_params,frame+5,frame[4],buffer);
                                }
                            }
                        }
                    }
                    break;
				#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
				case READ_TYPE_CURVE_HUNAN:
                    save_curve_data_hunan(read_params,frame+5,frame[4],buffer);
                    break;
				#endif
                case READ_TYPE_LAST_CURVE_CYCLE_DAY:
                    save_last_curve_cycle_day(read_params,frame+5,frame[4],buffer);
                    //#ifdef __ZhuZhou_CITY__
                    save_cur_date(read_params,frame+5,frame[4],buffer);
                    //#endif
                    break;
                #ifdef __VOLTAGE_MONITOR__
                case READ_TYPE_VOLTAGE_MONITOR_TASK:
                    if (get_system_flag(SYS_VOLTAGE_MONITOR,SYS_FLAG_BASE))
                    {
                        plc_router_save_valtage_data_standard(read_params,frame+14,frame[9]-4);
                    }
                    else
                    {
                        plc_router_save_valtage_data(read_params,frame+14,frame[9]-4);
                    }
                    break;
                #endif
                }
                return 1;
            }

            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** save data : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",bin2_int16u(read_params->meter_doc.meter_idx),
                    bin2_int32u(read_params->item),bin2_int32u(read_params->phy));
            debug_println_ext(info);
            #endif
        }
    }
    return 0;
}

#ifdef __SH_2009_METER__
INT8U save_read_data_sh_2009(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT8U idx,len,phy_idx;
    INT8U pos;
    INT8U buffer[300];
    READ_WRITE_DATA phy;

    if (check_frame_body_gb645(frame,*frame_len) == FALSE) return 0;
    if (compare_string(read_params->meter_doc.meter_no,frame+POS_GB645_METERNO,4) != 0) return 0;
    frame[POS_GB645_METERNO+4] = read_params->meter_doc.meter_no[4];
    frame[POS_GB645_METERNO+5] = read_params->meter_doc.meter_no[5];

    if ((frame[POS_GB645_CTRL] == 0x1C) && (bin2_int32u(read_params->item) == 0x9C))    //����ƽ�ȵ���
    {
        pos = POS_GB645_ITEM;
        mem_cpy(buffer,frame+POS_GB645_ITEM,12);
        for(idx=0;idx<12;idx++) buffer[idx] -= 0x33;

        if (read_params->meter_doc.fl_count == 4)
        {
            mem_set(frame+pos,20,REC_DATA_IS_DEFAULT);
            mem_cpy(frame+pos,buffer,4);      //��--��
            mem_cpy(frame+pos+8,buffer+4,4);  //ƽ--��
            mem_cpy(frame+pos+16,buffer+8,4); //��--��
            len = 20;
        }
        else
        {
            mem_cpy(frame+pos,buffer,12);
            len = 12;
        }
        save_cycle_day(read_params,frame+POS_GB645_ITEM,len,buffer);
    }
//    else if ((frame[POS_GB645_CTRL] == 0x2E) && (bin2_int32u(read_params->item) == 0x9020))
//    {
//        mem_cpy(buffer,frame+POS_GB645_ITEM,4);
//        frame[POS_GB645_CTRL] = 0x81;
//        pos = POS_GB645_ITEM;
//        frame[pos++] = read_params->item[0] + 0x33;
//        frame[pos++] = read_params->item[1] + 0x33;
//        mem_cpy(frame+pos,buffer,4);
//        frame[POS_GB645_DLEN] = 6;
//    }
//    else if ((frame[POS_GB645_CTRL] == 0x1F) && (bin2_int32u(read_params->item) == 0x901F))      //�ܷ�ƽ��
//    {
//        mem_cpy(buffer,frame+POS_GB645_ITEM,16);
//        frame[POS_GB645_CTRL] = 0x81;
//        pos = POS_GB645_ITEM;
//        frame[pos++] = read_params->item[0] + 0x33;
//        frame[pos++] = read_params->item[1] + 0x33;
//        mem_cpy(frame+pos,buffer,16);
//        frame[POS_GB645_DLEN] = 18;
//    }
    else if ((frame[POS_GB645_CTRL] == 0x37) && (bin2_int32u(read_params->item) == 0xB7))     //�¶���
    {
        pos = POS_GB645_ITEM;
        mem_cpy(buffer,frame+POS_GB645_ITEM,51);
        for(idx=0;idx<51;idx++) buffer[idx] -= 0x33;

        for(idx=0;idx<3;idx++)
        {
            mem_cpy(buffer+298,read_params->month_hold_td,2);/*��Ҫ���ջظ������е�ʱ��洢���ȱ���read_params->month_hold_td��Ȼ��save_cycle_month���ٻָ�*/
            
            read_params->month_hold_td[0] = BCD2byte(buffer[idx*17+3]);/*��*/
            read_params->month_hold_td[1] = BCD2byte(buffer[idx*17+4]);/*��*/
            
        if (read_params->meter_doc.fl_count == 4)
        {
            mem_set(frame+pos,20,REC_DATA_IS_DEFAULT);
                mem_cpy(frame+pos,buffer+idx*17+5,4);    //��--��
                mem_cpy(frame+pos+8,buffer+idx*17+5+4,4);  //ƽ--��
                mem_cpy(frame+pos+16,buffer+idx*17+5+8,4); //��--��
            len = 20;
        }
        else
        {
                mem_cpy(frame+pos,buffer+idx*17+5,12);
            len = 12;
        }
            
            phy_idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
			
            if(0xFF != phy_idx)
            {
                if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer+100,buffer+105,buffer+200) == FALSE)
                {
        save_cycle_month(read_params,frame+pos,len,buffer);
    }
}
   
            mem_cpy(read_params->month_hold_td,buffer+298,2);
        } 
    }
}
#endif //#ifdef __SH_2009_METER__

#ifdef __READ_EDMI_METER__
INT8U save_read_data_METER_EDMI(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT32U phy_u32;
    READ_WRITE_DATA phy;
    READ_WRITE_DATA *phy_ptr;
    tagPROTOCOL_LIB pLib_tmp;
    READ_PARAMS read_params_tmp;
    INT16U len,left_len,block_begin_idx;
    INT8U td[5];
    INT8U idx,ret,data_num,block_count,curve_save_flag;
    INT8U realFrameLen;
    INT8U buffer[300],tmp_buf[50],frame_buf[255]={0};

    mem_cpy(frame_buf,frame,* frame_len);
    realFrameLen = * frame_len;
    if (check_edmi_frame(frame_buf,&realFrameLen) == FALSE) return 0;
   // rs232_debug_info("\xB1",1);
    ret = edmi_recv_ctrl(read_params,frame_buf,realFrameLen);
    if(ret == EDMI_SAVE_NO_DATA)
    {
        return 1;
    }
    else if(ret == EDMI_SAVE_MANY_DATA)
    {
        data_num = edmi_parse_load_record(read_params,frame_buf,realFrameLen,td,buffer);

        if(FALSE == save_load_curve_data(read_params,buffer,data_num,td))
        {
            edmi_to_exit_state(read_params);
        }
        else
        {
            mem_set(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,0x00);
        }

        return 1;
    }
    else if(ret == EDMI_SAVE_RECORD_NO_DATA)//���ɼ�¼û�У�
    {
       mem_cpy(td,read_params->protocolCtrl.edmi.td,5);
       phy_u32 = bin2_int32u(read_params->phy);
       get_phy_form_list_cruve(phy_u32,&phy);
       {
     //       mem_set(frame_buf,phy.block_len,0xEE);
       }
      // writedata_curve(read_params,&phy,td,frame_buf,phy.block_len,buffer,0);
       return 1;
    }

    {
        phy_u32 = bin2_int32u(read_params->phy);
        left_len = READPORT_RS485_FRAME_SIZE - (*frame_len);
        len=unwrap_emdi_frame(&phy_u32, 1, frame, *frame_len, left_len, (METER_DOCUMENT *)&(read_params->meter_doc), 1,0);
        if(0 == len) return 0;
        wrap_phy2block(phy_u32,frame, len);

//        if(((phy_u32>= 0x40) && (phy_u32<=0x7F))
//                ||((phy_u32>= 0x80) && (phy_u32<=0xBF))
//                ||((phy_u32>= 0xC0) && (phy_u32<=0xFF))
//                ||((phy_u32>= 0x100) && (phy_u32<=0x13F)))
//        {
//            frame[4] = 4;
//            mem_cpy(frame+5,frame+6,4);
//        }

        switch(read_params->read_type)
        {
            case READ_TYPE_CYCLE_DAY:
                save_cycle_day(read_params,frame+5,frame[4],buffer);
                if (read_params->control.month_hold_mode == 0)
                {
                    idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
                    if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                    {
                        save_cycle_month(read_params,frame+5,frame[4],buffer);
                    }
                }
                break;
            case READ_TYPE_CURVE:
                save_curve_data(read_params,frame+5,frame[4],buffer,0);
                break;
            case READ_TYPE_CUR_DATA:
                save_cur_date(read_params,frame+5,frame[4],buffer);
                #ifndef __EDMI_READ_LOAD__
                curve_save_flag = 0x55;
                save_curve_data(read_params,frame+5,frame[4],buffer,curve_save_flag);
                #endif
                if((read_params->meter_doc.baud_port.port == COMMPORT_485_CY) || (read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS))
                {
                    #ifndef __RS485_READ_DAYHOLD_ITEM__
                    mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                    trans_phy_cur_2_cycle_day(read_params_tmp.phy);
                    if (check_is_all_ch(read_params_tmp.day_hold_td,3,0x00)) get_yesterday(read_params_tmp.day_hold_td);
                    if(0xFF != get_phy_form_list_cycle_day(bin2_int32u(read_params_tmp.phy),&phy))
                    {
                        if(readdata_cycle_day(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.day_hold_td,buffer,buffer+5,frame_len,NULL) == FALSE)
                        {
                            save_cycle_day(&read_params_tmp,frame+5,frame[4],buffer);
                        }
                    }
                    //�¶��ỹδ����
                    if(check_is_all_ch(read_params->month_hold_td,2,0x00)) get_former_month(read_params_tmp.month_hold_td);
                    if(0xFF != get_phy_form_list_cycle_month(bin2_int32u(read_params_tmp.phy),&phy))
                    {
                        if(readdata_cycle_month(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                        {
                            save_cycle_month(&read_params_tmp,frame+5,frame[4],buffer);
                        }
                    }
                    #endif
                    //�����ն�������
                    #ifndef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
                    //if(read_params->hold_flag.rec_day)
                    {
                        if(TRUE == is_recday_rs485_port(read_params->meter_doc.baud_port.port))
                        {
                            if(0xFF != get_phy_form_list_recday_data(bin2_int32u(read_params->phy),&phy))
                            {
                                if(readdata_recday_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,datetime+DAY,buffer,buffer+5,frame_len) == FALSE)
                                {
                                    save_recday_data(read_params,frame+5,frame[4],buffer);
                                }
                            }
                        }
                    }
                    #endif
                    if(0xFF != get_phy_from_list_day_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                    {
                        readdata_day_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                            
                        if(0 != compare_string(buffer+2,datetime+DAY,3))
                        {
                            if((phy_u32 == 0x0000007F) && (read_params->meter_doc.protocol == 0))
                            {
                                save_day_init_data(read_params,tmp_buf,25,buffer);
                            }
                            else save_day_init_data(read_params,frame+5,frame[4],buffer);
                        }
                    }
                    if(0xFF != get_phy_from_list_month_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                    {    
                        readdata_month_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                        if(0 != compare_string(buffer+3,datetime+MONTH,2))
                        {
                            if((phy_u32 == 0x0000007F) && (read_params->meter_doc.protocol == 0))
                            {
                                save_month_init_data(read_params,tmp_buf,25,buffer);
                            }
                            else save_month_init_data(read_params,frame+5,frame[4],buffer);
                        }
                    }
                }
                break;
        }
        return 1;
    }
    
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** save data : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",bin2_int16u(read_params->meter_doc.meter_idx),
            bin2_int32u(read_params->item),bin2_int32u(read_params->phy));
    debug_println_ext(info);
    #endif
     
    return 0;
}
#endif

#ifdef __READ_HENGTONG_METER__
INT8U save_read_data_hengtong(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT32U phy_u32,data_addr;
    READ_WRITE_DATA phy;
    READ_WRITE_DATA *phy_ptr;
    tagPROTOCOL_LIB pLib_tmp;
    READ_PARAMS read_params_tmp;
    INT16U len,left_len,block_begin_idx;
    INT8U* td;
    INT8U idx,block_count;
    INT8U midu,result;
    INT8U buffer[300],tmp_buf[50];

    data_addr = bin2_int32u(read_params->item);
    result = check_hengt_resp_frame(read_params->meter_doc.meter_no[0],HENGTONG_QUERY,data_addr,frame,buffer,*frame_len); 
    if (result == FALSE) return 0;
    
    
    //if (((frame[8] & 0x1F) == 0x01) || ((frame[8] & 0x1F) == 0x02)) //�����ݺ��ض�����
    {
        if (result == 0xFD) //�쳣Ӧ��
        {
            mem_set(frame+12,20,0xEE);
            frame[9] = 20+2;

            switch(read_params->read_type)
            {
            case READ_TYPE_CYCLE_DAY:
                save_cycle_day(read_params,frame+12,frame[9]-2,buffer);
                if (read_params->control.month_hold_mode == 0)
                {
                    idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
                    if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                    {
                save_cycle_month(read_params,frame+12,frame[9]-2,buffer);
                    }
                }
                break;
            case READ_TYPE_CURVE:
                save_curve_data(read_params,frame+12,frame[9]-2,buffer,0);
                break;
            case READ_TYPE_CUR_DATA:
                save_cur_date(read_params,frame+12,frame[9]-2,buffer);
                break;  
            }
            return 1;
        }
        else if(result != 0xFE)
        {
            
            //if ((compare_string(read_params->item,frame+10,2)) == 0)
            {
                phy_u32 = bin2_int32u(read_params->phy);
                
                left_len = READPORT_RS485_FRAME_SIZE - (*frame_len);
                len = unwrap_hengtong_frame(&phy_u32,1, frame, *frame_len, left_len,PROTOCOL_HENGTONG_OLD,1);
                wrap_phy2block(phy_u32,frame, len);

                switch(read_params->read_type)
                {
                case READ_TYPE_CYCLE_DAY:
                    save_cycle_day(read_params,frame+5,frame[4],buffer);
                    if (read_params->control.month_hold_mode == 0)
                    {
                        idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
                        if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                        {
                            save_cycle_month(read_params,frame+5,frame[4],buffer);
                        }
                    }
                    break;
                case READ_TYPE_CURVE:
                    save_curve_data(read_params,frame+5,frame[4],buffer,0);
                    break;
                case READ_TYPE_CUR_DATA:
                    save_cur_date(read_params,frame+5,frame[4],buffer);
                    if((read_params->meter_doc.baud_port.port == COMMPORT_485_CY) || (read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS))
                    {
                        #ifndef __RS485_READ_DAYHOLD_ITEM__
                        mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                        trans_phy_cur_2_cycle_day(read_params_tmp.phy);
                        if (check_is_all_ch(read_params_tmp.day_hold_td,3,0x00)) get_yesterday(read_params_tmp.day_hold_td);
                        if(0xFF != get_phy_form_list_cycle_day(bin2_int32u(read_params_tmp.phy),&phy))
                        {
                        if(readdata_cycle_day(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.day_hold_td,buffer,buffer+5,frame_len,NULL) == FALSE)
                        {
                                save_cycle_day(&read_params_tmp,frame+5,frame[4],buffer);
                        }
                        }
                        //�¶��ỹδ����
                        if(check_is_all_ch(read_params->month_hold_td,2,0x00)) get_former_month(read_params_tmp.month_hold_td);
                        if(0xFF != get_phy_form_list_cycle_month(bin2_int32u(read_params_tmp.phy),&phy))
                        {
                        if(readdata_cycle_month(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                        {
                                save_cycle_month(&read_params_tmp,frame+5,frame[4],buffer);
                        }
                        }
                        #endif
                        //�����ն�������
                        #ifndef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
                        //if(read_params->hold_flag.rec_day)
                        {
                            if(TRUE == is_recday_rs485_port(read_params->meter_doc.baud_port.port))
                            {
                                if(0xFF != get_phy_form_list_recday_data(bin2_int32u(read_params->phy),&phy))
                                {
                                    if(readdata_recday_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,datetime+DAY,buffer,buffer+5,frame_len) == FALSE)
                                    {
                                            save_recday_data(read_params,frame+5,frame[4],buffer);
                                    }
                                }
                            }
                        }
                        #endif
                        if(0xFF != get_phy_from_list_day_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                        {
                            readdata_day_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                            
                            if(0 != compare_string(buffer+2,datetime+DAY,3))
                            {
                            if((phy_u32 == 0x0000007F) && (read_params->meter_doc.protocol == 0))
                            {
                                save_day_init_data(read_params,tmp_buf,25,buffer);
                            }
                            else save_day_init_data(read_params,frame+5,frame[4],buffer);
                            }
                        }
                        if(0xFF != get_phy_from_list_month_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                        {    
                            readdata_month_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                            if(0 != compare_string(buffer+3,datetime+MONTH,2))
                            {
                                if((phy_u32 == 0x0000007F) && (read_params->meter_doc.protocol == 0))
                                {
                                    save_month_init_data(read_params,tmp_buf,25,buffer);
                                }
                                else save_month_init_data(read_params,frame+5,frame[4],buffer);
                            }
                        }
                    }
                    break;
                }
                return 1;
            }

            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** save data : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",bin2_int16u(read_params->meter_doc.meter_idx),
                    bin2_int32u(read_params->item),bin2_int32u(read_params->phy));
            debug_println_ext(info);
            #endif
        }
    }
    return 0;
}
#endif

#ifdef __READ_IEC1107_METER__
INT8U save_read_data_iec1107(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT8U *item;
    INT32U phy_u32;
    READ_WRITE_DATA phy;
    READ_WRITE_DATA *phy_ptr;
    tagPROTOCOL_LIB pLib_tmp;
    READ_PARAMS read_params_tmp;
    INT16U len,left_len,block_begin_idx;
    INT8U block_count;
    INT8U phy_num,curve_save_flag=0;
    INT8U buffer[300],tmp_buf[50]={0},td[5],dataToSave[100];
    INT8U idx,dataLen;

    if(FALSE == iec1107_process_ctrl(read_params,frame,*frame_len))
    {
        return 0;
    }

    mem_cpy(buffer,frame,* frame_len);

    if(read_params->read_ctrl_state==5)
    {
        if(read_params->read_ctrl_step==0)
        {
            if(TRUE == parse_iec1107_load_config(read_params,frame,*frame_len))
            {
                read_params->read_ctrl_step = 1;
                return 1;
            }
            else
            {
                iec1107_to_exit_state(read_params);
                return 0;
            }
        }
        else
        {
            block_count = iec1107_parse_load_record(read_params,frame,*frame_len,td,buffer);

            save_load_curve_data(read_params,buffer,block_count,td);
            if(read_params->read_type==READ_TYPE_CURVE)
                mem_set(read_params->read_mask.curve,READ_MASK_BYTE_NUM_CURVE,0);
            else
                read_params->patch_num++;
            read_params->read_ctrl_step = 0;//�����ɳ���ͬһ���ͬʱ���Ķ�����ɼ�¼
            read_params->protocolCtrl.iec1107.param_count = 0;
            return 0;
        }
    }

    //item=read_params->item;

    left_len = READPORT_RS485_FRAME_SIZE - (*frame_len);
    phy_num=unwrap_iec1107_frame(frame, *frame_len, left_len, IEC1107, 1);
    if(0 == phy_num) return 1;
    for(idx=0; idx<phy_num; idx++)
    {
        dataLen = frame[4];
        mem_cpy(read_params->phy,frame,4);
        phy_u32 = bin2_int32u(read_params->phy);
        mem_cpy(dataToSave,frame+5,dataLen);
        frame += (5+dataLen);

        switch(read_params->read_type)
        {
            case READ_TYPE_CYCLE_MONTH:
            case READ_TYPE_CURVE:
            case READ_TYPE_CUR_DATA:
            case READ_TYPE_CYCLE_DAY:
                if((phy_u32<0x9B80)||(phy_u32>0x9D7F))//����ֻ��07��ʽ�ģ�97��ʽ�Ĳ��ٴ棬�ظ��洢��������
                    save_cur_data_iec1107(read_params,dataToSave,dataLen,buffer);
                #ifdef __IEC1107_READ_LOAD__
//                if(read_params->meter_doc.meter_class.user_class!=7)
                #endif
                {
                    #ifdef __CUR_TO_CURVE__
                    curve_save_flag = 0x55;
                    save_curve_data(read_params,dataToSave,dataLen,buffer,curve_save_flag);
                    #endif
                }
                if((read_params->meter_doc.baud_port.port == COMMPORT_485_CY) || (read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS))
                {
                    #ifndef __RS485_READ_DAYHOLD_ITEM__
                    mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                    trans_phy_cur_2_cycle_day(read_params_tmp.phy);
					
#ifndef   __CYCLE_DAY_MONTH_DATE_TODAY__
                    if (check_is_all_ch(read_params_tmp.day_hold_td,3,0x00)) get_yesterday(read_params_tmp.day_hold_td);
#else
					if (check_is_all_ch(read_params_tmp.day_hold_td,3,0x00))
						mem_cpy(read_params_tmp.day_hold_td,datetime+DAY,3);

#endif
                    if(0xFF != get_phy_form_list_cycle_day(bin2_int32u(read_params_tmp.phy),&phy))
                    {
                        if(readdata_cycle_day(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.day_hold_td,buffer,buffer+5,frame_len,NULL) == FALSE)
                        {
                            save_cycle_day(&read_params_tmp,dataToSave,dataLen,buffer);
                        }
                    }                    
					
#ifndef   __CYCLE_DAY_MONTH_DATE_TODAY__
											//�¶��ỹδ����
					if(check_is_all_ch(read_params->month_hold_td,2,0x00)) get_former_month(read_params_tmp.month_hold_td);
#else			
					if(check_is_all_ch(read_params->month_hold_td,2,0x00)) 
						mem_cpy(read_params_tmp.month_hold_td,datetime+MONTH,2) ;
					
#endif
                    if(0xFF != get_phy_form_list_cycle_month(bin2_int32u(read_params_tmp.phy),&phy))
                    {
                        if(readdata_cycle_month(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                        {
                            save_cycle_month(&read_params_tmp,dataToSave,dataLen,buffer);
                        }
                    }
                    #endif
                    //�����ն�������
                    #ifndef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
                    //if(read_params_tmp.hold_flag.rec_day)
                    {
                        if(TRUE == is_recday_rs485_port(read_params_tmp.meter_doc.baud_port.port/*-2 Ҫ��2�� */))
                        {
                            if(0xFF != get_phy_form_list_recday_data(bin2_int32u(read_params_tmp.phy),&phy))
                            {
                                if(readdata_recday_data(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,datetime+DAY,buffer,buffer+5,frame_len) == FALSE)
                                {
                                    save_recday_data(read_params,frame+5,frame[4],buffer);
                                }
                            }
                        }
                    }
                    #endif
                    if(0xFF != get_phy_from_list_day_init_data(bin2_int32u(read_params_tmp.phy),&phy,&block_begin_idx,&block_count))
                    {
                        readdata_day_init_data(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                            
                        if(0 != compare_string(buffer+2,datetime+DAY,3))
                        {
                            if((phy_u32 == 0x0000007F) && (read_params_tmp.meter_doc.protocol == 0))
                            {
                                save_day_init_data(read_params,tmp_buf,25,buffer);
                            }
                            else save_day_init_data(read_params,frame+5,frame[4],buffer);
                        }
                    }
                    if(0xFF != get_phy_from_list_month_init_data(bin2_int32u(read_params_tmp.phy),&phy,&block_begin_idx,&block_count))
                    {    
                        readdata_month_init_data(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                        if(0 != compare_string(buffer+3,datetime+MONTH,2))
                        {
                            if((phy_u32 == 0x0000007F) && (read_params_tmp.meter_doc.protocol == 0))
                            {
                                save_month_init_data(read_params,tmp_buf,25,buffer);
                            }
                            else save_month_init_data(read_params,frame+5,frame[4],buffer);
                        }
                    }
                }
                break;
        }
    }
    return 1;
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** save data : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",bin2_int16u(read_params->meter_doc.meter_idx),
            bin2_int32u(read_params->item),bin2_int32u(read_params->phy));
    debug_println_ext(info);
    #endif

    return 0;
}
#endif
INT8U save_read_data(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT8U result;

    result = 0;
	
	if (read_params->plan_type == 222)
    {
        result = save_read_data_cjq_update_task(read_params,frame,frame_len);
		read_params->plan_type = 0;  //����ɼ������������ı�־
		return result;
    }
    if(check_is_sqr_protocol(read_params->meter_doc.protocol))   //ˮ���ȱ�
    {
        result = save_read_dzc_data(read_params,frame,frame_len);
    }
    else
    {
    switch(read_params->meter_doc.protocol)
    {
        case GB645_CY:
    case GB645_2007:
#ifdef __JIANGSU_READ_CURRENT_MONITOR__
    case BREAKER_MONITOR:
#endif
#ifdef __FUJIAN_CURRENT_BREAK__
    case FUJIAN_BREAKER_MONITOR:
#endif
            result = save_read_data_GB645_2007(read_params,frame,frame_len);
        break;
        #ifdef __POWER_CTRL__
        case 0:
        #endif
    case GB645_1997:
    case GUANGXI_V30:
    case GB645_1997_JINANGSU_4FL:
    case GB645_1997_JINANGSU_2FL:
        result = save_read_data_GB645_1997(read_params,frame,frame_len);
        break;
    #ifdef __SH_2009_METER__
    case SHANGHAI_2009:
        result = save_read_data_sh_2009(read_params,frame,frame_len);
        break;
    #endif
    #ifdef __READ_EDMI_METER__
    case METER_EDMI:
            result = save_read_data_METER_EDMI(read_params,frame,frame_len);
        break;
    #endif
    #ifdef __READ_HENGTONG_METER__
    case PROTOCOL_HENGTONG_OLD:
    case PROTOCOL_HENGTONG_NEW:
            result = save_read_data_hengtong(read_params,frame,frame_len);
        break;
    #endif
    #ifdef __READ_OOP_METER__
    case GB_OOP:
        result = save_read_data_gb_oop(read_params,frame,frame_len);
        break;
    #endif
    #ifdef __READ_IEC1107_METER__
    case IEC1107:
            result = save_read_data_iec1107(read_params,frame,frame_len);
        break;
    #endif
    #ifdef __READ_DLMS_METER__
    case METER_DLMS:
        result = save_read_data_dlms(read_params,frame,frame_len);
        break;
    #endif
    default:
        break;
    }
    }
   //��¼�յ����Ĵ���
   #ifdef __MESSAGE_SEND_RECEIVE_RECORD__
    if(read_params->meter_doc.baud_port.port == READPORT_PLC)
    message_send_and_receive_num_record(bin2_int16u(read_params->meter_doc.meter_idx),0xAA);
   #endif

    return result;
}

void trans_phy_cur_2_cycle_day(INT8U phy[4])
{
     INT32U phy_32u;

     phy_32u = bin2_int32u(phy);

     switch(phy_32u)  
     {
           
         //#if defined (__PROVICE_JILIN__)
         //case 0x0000143F:
         //case 0x0000147F:
         //case 0x000014BF:
         //case 0x000014FF:
         //    phy_32u = 0xFFFFFFFF;
         //    break;
         //#else
         case 0x00004EBF:
         case 0x00004F3F:
         case 0x00004EFF:
         case 0x00004F7F:
         
         case 0x00003BBF:
         case 0x00003C3F:
         case 0x00003CBF:
         case 0x00003D7F:

         case 0x00003BFF:
         case 0x00003C7F:
         case 0x00003CFF:
         case 0x00003D3F:
             phy_32u = 0xFFFFFFFF;
             break;
          case 0x0000143F:
                phy_32u = 0x00002E7F;
                break;
         case 0x0000147F:
                phy_32u = 0x00002EBF;
                break;  
        #ifdef __HUNAN_NEW_RECORDING__
        case A_SY_ZLJ_SJ:
             phy_32u = 0x000085C0;
             break;
        case B_SY_ZLJ_SJ:
             phy_32u = 0x00008680;
             break;
        case C_SY_ZLJ_SJ:
             phy_32u = 0x00008740;
             break;
        case SY_ZLJ_SJ:
             phy_32u = 0x000084C0;
             break;     
        #endif
         //#endif
          default:
                break;
     }

     int32u2_bin(phy_32u,phy);
}

#ifdef __ENABLE_ESAM2__
/*+++
  ���ܣ�����������������֤����
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
void save_batch_meter_task_auth(READ_PARAMS *read_params,INT8U *data,INT8U datalen)
{
    INT32U offset;
    INT16U meter_idx;
    BATCH_TASK_AUTH task_auth;
    //INT8U idx;

    mem_set(task_auth.value,sizeof(task_auth),0x00);
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    offset = sizeof(BATCH_TASK_AUTH);
    offset *= meter_idx;
    offset += sizeof(BATCH_TASK_HEADER);
    fread_array(FILEID_METER_BATCH_TASK_0,offset,task_auth.value,sizeof(BATCH_TASK_AUTH));
    if(datalen == 12)
    {
        task_auth.task_state = 0xA0; //��֤�ɹ�
        mem_cpy_reverse(task_auth.meter_rondon,data,4);
        mem_cpy_reverse(task_auth.meter_esam_id,data+4,8);
    }
    else
    {
        task_auth.task_state = 0x0A;  //��֤ʧ��
    }

    fwrite_array(FILEID_METER_BATCH_TASK_0,offset,task_auth.value,sizeof(BATCH_TASK_AUTH));
}

/*+++
  ���ܣ�������������ʱ����
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ���ȥ��0x33,��Ȼ������      hhmmss
        INT8U datalen        ���ݳ���
---*/
void save_batch_meter_task_time(READ_PARAMS *read_params,BOOLEAN is_success)
{
    INT32U offset;
    INT16U meter_idx;
    BATCH_TASK_TIME task_time;
    //INT8U idx;

    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    
    offset = sizeof(BATCH_TASK_TIME);
    offset *= meter_idx;
    offset += sizeof(BATCH_TASK_HEADER);
    fread_array(FILEID_METER_BATCH_TASK_1,offset,task_time.value,sizeof(BATCH_TASK_TIME));
    if(is_success)
    {
        task_time.task_state = 0xA0; //��֤�ɹ�
    }
    else
    {
        task_time.task_state = 0x0A; //��֤ʧ��
    }

    fwrite_array(FILEID_METER_BATCH_TASK_1,offset,task_time.value,sizeof(BATCH_TASK_TIME));
}
#endif  //#ifdef __ENABLE_ESAM2__
/*
�˺�����δʹ�ã�������ܴ��ڵ������Ȳ�����
*/
void plc_router_save_cjq_meter_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen,BOOLEAN is_catch)
{

    //INT16U meter_seq,save_pos;
    //INT16U pos_idx;
    INT8U idx;//z_count,,read_flag_byte_num;count,
    //INT8U event_read_flag[MAX_METER_EVENT_ITEM_LEVEL_COUNT/8];

    INT8U flag[12]={0};
    INT8U ctrl_flag = 0;
    //INT8U pos;
    INT8U test[4]={0};

    //z_count = data[0];     //֮ǰ�Ĳɼ����¼��ϱ����������ϱ����¼��ĵ������������Ϊ1����������ȥ�������¼���������ֱ���ϱ�״̬�֣��洢�󣬰��ն�Ӧ��ȥ������
    //count = data[1];       //��Ҫ��ȡ������Լ���ͣ���״̬��һ��ֱ�Ӵ洢��
    //�˺�����Ҫ�Ѳɼ����µĵ����ȡ����

    //pos = 2;
    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1); //�ȶ�������Ҫ�����׸�ֵ0
    if(data[8]==0x9E)
    	goto CONFIRM_END;
	if(item == 0xC3)
	{
		//for(int i=0;i<count;i++) //�˴�ȷ��֡�Ļ��������ˣ�Ϊɶ��ѭ������

		if (compare_string(data,meter_no,6) == TRUE) //���Ǽ������ĵ��������洢��meter_idx���¸�ֵ��
		{
			if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY))
			{
			    ctrl_flag = 0x00;
			    fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
			}
			else if(check_is_all_ch(data,(datalen > 64) ? 64 : datalen,0x00))  //���֧��8�����ÿ�������¼���8���ֽڣ�
			{
			    ctrl_flag = 0x00;
			    fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
			}
			else
			{
			    datalen = 2; //�ɼ������ź�+1�ֽڹ�Լ+1�ֽ��¼�״̬��,�ӿ�ͷ���������ݣ�����ַ6����Ҫ�洢�ˣ�ֱ�Ӵ�2��
			    fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,data+6,datalen); //�ϱ�����¼���������C3����α���
			    //Ҫ�ѵ���Ū������

			//   save_meter_event_record(COMMPORT_PLC,meter_no,item,data,datalen); //save����¼���Ҫ�����ϱ��������ʱ����Ҫ�ɣ������Σ�
			    //�˴���save���ϱ��Ŀ����ְɣ�
			    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
			    ctrl_flag &= ~0x02; //����ı�ʶ��Ӧ��ȥ������Ҳ����������ɣ������¼�״̬���Ѿ��洢��
			    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,test,4);
			    fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
			    fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,test,4);
			//                                        pos += 8; //8�ֽ�

			}
		}

	}
    else if (item == 0xC4) //���״̬��
	{
		fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
		if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY))
		{
			ctrl_flag = ~0x04;
			fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
		}
		else
		{
			ctrl_flag &= ~0x04;
			fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
		}
	}
    else
	{
		if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY) == FALSE)
		{
		//save_meter_event_record(COMMPORT_PLC,meter_no,item,data,datalen);
		}

		fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,flag,2);
		for(idx=0;idx<4;idx++)
		{
			if(get_bit_value(flag+1,1,idx))
			{
				clr_bit_value(flag+1,1,idx);
				fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,flag,2);
				break;
			}
		}
		fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
		if(check_is_all_ch(flag+1,1,0x00))
		{
			ctrl_flag &= ~0x08;
			fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
		}
	}
CONFIRM_END:
           return;
}





/*+++
  ���ܣ�����¼��¼�������ڴ洢�е�λ��
  ������
         INT8U event_level              1 2 3 4
  ���أ�
         TRUE  ��Ҫ����
         FALSE �������
  ������

---*/
INT8U find_plan_read_item(INT8U plan_id,INT32U item,INT8U *level_id)
{
    INT32U offset;
    INT8U idx,item_idx,item_byte[4];
    PARAM_F106 f106;

    *level_id = MAX_METER_EVENT_LEVEL + 1;
    item_idx = MAX_METER_EVENT_ITEM_COUNT + 1;
    item_byte[0] = item;
    item_byte[1] = item>>8;
    item_byte[2] = item>>16;
    item_byte[3] = item>>24;

    offset = sizeof(PARAM_F106) * MAX_METER_EVENT_ITEM_COUNT * (plan_id-1);
    offset += PIM_PARAM_F106;
    for(idx=0;idx<MAX_METER_EVENT_ITEM_COUNT;idx++)
    {
        fread_array(FILEID_METER_EVENT_PARAM,offset+idx*sizeof(PARAM_F106),f106.value,sizeof(PARAM_F106));
        if (f106.level == 0) continue;
        if (f106.level > MAX_METER_EVENT_LEVEL) continue;
        if (compare_string(item_byte,f106.item,4) == 0)
        {
            item_idx = idx;
            *level_id = f106.level;
        }
    }
    return item_idx;
}
#define CLEAR_FLAG_WITHOUT_REPORT          0
#define SAVE_AND_CLEAR_FLAG                1
#define SAVE_AND_KEEP_ON_RECORDING         2

INT8U frame_check_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len,BOOLEAN append_item)
{
    INT32S  tmp_val,tmp_val1;
    INT32U  offset;
    INT32U  tmp_data_item;
    INT16U  meter_idx;
    INT8U   item_data[PIM_METER_EVENT_XL_CNT_SAVE_DATA_PER_ITEM_LEN]; //PIM_METER_EVENT_SAVE_DATA_07_PER_ITEM_LEN
    INT8U   item_idx;
    INT8U   idx;
    BOOLEAN valid_bcd_flag;
    //BOOLEAN valid_bcd_flag1;
    BOOLEAN flag = FALSE;
    mem_set(item_data,sizeof(item_data),0x00);
    item_idx = read_params->event_item_ctrl.event_idx;// ���������λ��
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    
    if( (read_params->meter_doc.protocol ==  GB645_2007))
    {
        if(bin2_int32u(read_params->item) != 0x03120000) //������������
        {
            //��ȡ������		
            offset = PIM_METER_EVENT_SAVE_DATA_07;
            offset += item_idx*PIM_METER_EVENT_SAVE_DATA_07_PER_ITEM_LEN+PIM_METER_EVENT_SAVE_DATA_07_LEN*(read_params->event_item_ctrl.level-1);
            fread_array(meter_idx,offset,item_data,PIM_METER_EVENT_SAVE_DATA_07_PER_ITEM_LEN);
            if(compare_string(item_data,read_params->item,4) == 0) //ͬһ��ʶ
            {
                frame_len = (frame_len<=3)? frame_len:3;
                if(compare_string(item_data+4,frame,frame_len)==0)//δ�����ı�
                {
                    return CLEAR_FLAG_WITHOUT_REPORT;
                }
                else
                {
                    //
                    
                    tmp_val = 0;
                    tmp_val1 = 0;
                    for(idx=0;idx<frame_len;idx++)
                    {
                        tmp_val  += BCD2byte(frame[idx])*pow(100,idx); 
                        tmp_val1 += BCD2byte(item_data[4+idx])*pow(100,idx);
                    }
                    tmp_val -=  tmp_val1;
                    mem_cpy(item_data+4,frame,frame_len);
                    fwrite_array(meter_idx,offset,item_data,PIM_METER_EVENT_SAVE_DATA_07_PER_ITEM_LEN);	
                    
                    if(tmp_val < 0)//���������� ������
                    {						
                        return CLEAR_FLAG_WITHOUT_REPORT;
                    }
                    else
                    {		
                        //tmp_data_item = bin2_int32u(read_params->item);
                        //�������úʹ����������ó�������
                        if(append_item == FALSE)//�޺�������������
                        {
                            return SAVE_AND_CLEAR_FLAG;// ����
                        }
                        else
                        {
                        
                            tmp_val = (tmp_val>=10) ? 10:tmp_val;
                            read_params->event_item_ctrl.EventCtrl.count = tmp_val;
                            read_params->event_item_ctrl.EventCtrl.need_add_cnt = 0;
                            read_params->event_item_ctrl.valid_flag = 0xAA;//�������ڳ�������������
                            #ifdef EVENT_GRADE_INFO
                            snprintf(info,100,"======== �ܴ����仯 =%d,��������=%d***===== ",
                            tmp_val,read_params->event_item_ctrl.EventCtrl.count);
                            debug_println_ext(info);               
                            #endif
                            tmp_data_item = bin2_int32u(read_params->item);
                            if( ((tmp_data_item>>24)<0x10) || ((tmp_data_item>>24)>0x20) )
                            {
                                //������¼��
                                for(idx = tmp_val;idx<read_params->event_item_ctrl.EventCtrl.rec_item_cnt;idx++)
                                {
                                    clr_bit_value((INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8,idx); 
                                }
                                mem_cpy((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,(INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8);
                                read_params->event_item_ctrl.EventCtrl.count = 1;// 1 ��
                            }
                            #ifdef __PROVICE_SHANGHAI__  //2015-11-24 �Ϻ� ֻ����һ��
                            //item_ctrl->EventCtrl.count = 1;// ֻ������ 1 �εļ�¼
                            read_params->event_item_ctrl.EventCtrl.count = 1;
                            #endif
                            return SAVE_AND_KEEP_ON_RECORDING;
                        }
                    }
                    
                }
            }
            else//����ͬһ��ʶ����ֻ�洢���Σ����ٳ���ʣ�����ݣ������־
            {
                mem_cpy(item_data,read_params->item,4);
                frame_len = (frame_len<=3)? frame_len:3;
                mem_cpy(item_data+4,frame,frame_len);
                fwrite_array(meter_idx,offset,item_data,PIM_METER_EVENT_SAVE_DATA_07_PER_ITEM_LEN);
                return CLEAR_FLAG_WITHOUT_REPORT;
            }
        }
        else
        {
            offset = PIM_METER_EVENT_XL_CNT_SAVE_DATA;
            offset += item_idx*PIM_METER_EVENT_XL_CNT_SAVE_DATA_PER_ITEM_LEN+PIM_METER_EVENT_XL_CNT_SAVE_DATA_LEN*(read_params->event_item_ctrl.level-1);
            fread_array(meter_idx,offset,item_data,PIM_METER_EVENT_XL_CNT_SAVE_DATA_PER_ITEM_LEN);
            if(compare_string(item_data,read_params->item,4) == 0) //ͬһ��ʶ
            {
                frame_len = (frame_len<=18)? frame_len:18;
                if(compare_string(item_data+4,frame,frame_len)==0)//δ�����ı�
                {
                    return CLEAR_FLAG_WITHOUT_REPORT;
                }
                else
                {
                
                    tmp_val = 0;
                    tmp_val1 = 0;
                    mem_set(read_params->event_item_ctrl.EventCtrl.xl_cnt,6,0x00);
                    for(idx=0;idx<frame_len;idx+=3)
                    {
                        tmp_val  = bcd2u32(frame+idx,3,&valid_bcd_flag); 
                        tmp_val1 = bcd2u32(item_data+4+idx,3,&valid_bcd_flag);
                        tmp_val -=  tmp_val1;
                        if(tmp_val>0)
                        {
                            flag =  TRUE;
                            tmp_val = (tmp_val>=10) ? 10:tmp_val;
                            read_params->event_item_ctrl.EventCtrl.xl_cnt[idx/3] = tmp_val;
                            read_params->event_item_ctrl.EventCtrl.xl_add_cnt[idx/3] = 0;
                        }
                    }					
                    for(idx=0;idx<6;idx++)
                    
                    {
                        if( read_params->event_item_ctrl.EventCtrl.xl_cnt[idx] == 0 )//�ޱ仯�������
                            clr_bit_value((INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8,idx); 
                    }
                    //����copy����
                    mem_cpy((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,(INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8);
                    //д�뱾�εĳ�����ֵ
                    mem_cpy(item_data+4,frame,frame_len);
                    fwrite_array(meter_idx,offset,item_data,PIM_METER_EVENT_XL_CNT_SAVE_DATA_PER_ITEM_LEN);	
                    
                    if(flag == FALSE) // ���Ǵ�����С�� ������
                    {						
                        return CLEAR_FLAG_WITHOUT_REPORT;
                    }
                    else
                    {		
                        //�������úʹ����������ó�������
                        if(append_item == FALSE)//�޺�������������
                        {
                            return SAVE_AND_CLEAR_FLAG;// ����
                        }
                        else
                        {	
                            read_params->event_item_ctrl.valid_flag = 0xAA;//�������ڳ�������������
                            return SAVE_AND_KEEP_ON_RECORDING;
                        }
                    }
                    
                }
            }
            else//����ͬһ��ʶ����ֻ�洢���Σ����ٳ���ʣ�����ݣ������־
            {
                mem_cpy(item_data,read_params->item,4);
                frame_len = (frame_len<=18)? frame_len:18;
                mem_cpy(item_data+4,frame,frame_len);
                fwrite_array(meter_idx,offset,item_data,PIM_METER_EVENT_XL_CNT_SAVE_DATA_PER_ITEM_LEN);
                return CLEAR_FLAG_WITHOUT_REPORT;
            }
        }
    }
    else //97 or ����Э��
    {
    
    }
    
    return CLEAR_FLAG_WITHOUT_REPORT;
}

/*
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
���ݸ�ʽ ����ͷ+��������  
head : item(4)+2(len)+frame_cnt(1)+res
frame
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
BOOLEAN  save_tmp_data_in_meter_file(READ_PARAMS *read_params,INT8U *frame,INT8U frame_len,report_ctrl reportCtrl,INT8U *tmp,INT16U buffer_len)
{
    INT32U  offset;
    INT32U  val_and;
    INT32U  item_32u,save_item32u;
    INT16U  len = 0;
    INT16U  meter_idx;
    INT16U  tmp_len;
    //INT8U 	tmp[20];
    INT8U   head[8];//
    
    mem_set(tmp,buffer_len,0x00);
    mem_set(head,sizeof(head),0x00);
    
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    // 4 (item)+2(�ֽ� д�����ݳ���)+1(����֡����)
    if(reportCtrl.main_item_flag == TRUE)
    {		
        mem_cpy(head,read_params->item,4);//record item
        
        len = frame_len+9;//ǰ�����ֽ��ǳ���,9(1~9����F10�Ķ˿ڵ���Ϣ)
        head[4]	= len & 0x00FF;
        head[5] = (len>>8) & 0x00FF;
        head[6]	= 1;//һ֡����
        head[7] = 0;
        
        
        // 1���ֽڵĳ���+����
        tmp[0] = (INT8U)(len & 0x00FF);
        tmp[1] = (INT8U)((len>>8) & 0x00FF);
        tmp[2] = read_params->meter_doc.baud_port.port;
        mem_cpy(tmp+3,read_params->meter_doc.meter_no,6);  //��ַ
        tmp[9] = 6;//6 TODO
        tmp[10] = 1;//���ݱ�ʶ����,��Ҫ�䶯��
        
        offset = PIM_SAVE_TMP_REPORT_DATA_HEAD_START;
        fwrite_array(meter_idx,offset,head,8);
        offset = PIM_SAVE_TMP_REPORT_DATA_START;
        fwrite_array(meter_idx,offset,tmp,11);
        fwrite_array(meter_idx,offset+11,frame,frame_len);
        return TRUE;
        
    }
    else
    {
        item_32u = bin2_int32u(read_params->item);
        offset = PIM_SAVE_TMP_REPORT_DATA_HEAD_START;
        fread_array(meter_idx,offset,head,8);
        save_item32u = bin2_int32u(head);
        
        if( (((save_item32u>>24) >= 0x10) && ((save_item32u>>24) <= 0x20)) || ((save_item32u>>16) == 0x0312) )
        {
            val_and = 0xFFFF00F0;
        }
        else
        {
            val_and = 0xFFFFFFF0;
        }
        if( (item_32u & val_and) == (save_item32u & val_and) )
        {
            offset = PIM_SAVE_TMP_REPORT_DATA_START;
            fread_array(meter_idx,offset,tmp,11);
            
            tmp_len = bin2_int16u(tmp) + frame_len;
            if(tmp_len >= 850)
            {
                offset = PIM_SAVE_TMP_REPORT_DATA_START;
                fread_array(meter_idx,offset,(INT8U *)&len,2);
                
                //�洢�� 
                tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,offset+2,g_temp_buffer,len);
                reportCtrl.read_tmp_and_save = 1;
                save_meter_event_record(0,NULL,g_temp_buffer,len,reportCtrl);
                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                
                //���´洢λ��
                offset = PIM_SAVE_TMP_REPORT_DATA_START;
                fread_array(meter_idx,offset,tmp,11);
                tmp_len = 9+frame_len;
                tmp[0] = tmp_len&0x00FF;
                tmp[1] = (tmp_len>>8)&0x00FF;
                tmp[10] = 1;
                fwrite_array(meter_idx,offset,tmp,11);
            }
            else
            {
                tmp[0] = tmp_len&0x00FF;
                tmp[1] = (tmp_len>>8)&0x00FF;
                tmp[10]++;
                fwrite_array(meter_idx,offset,tmp,11);
            }
            
            // ? 2�ֽڳ�����ָ Ԥ����2�ֽڴ洢������Ϣ������+2-frame_len���͵õ�������Ҫ�洢��λ��
            tmp_len += 2;// 2�ֽڳ���
            tmp_len -= frame_len;//��һ��д���λ��
            offset = PIM_SAVE_TMP_REPORT_DATA_START+tmp_len;
            fwrite_array(meter_idx,offset,frame,frame_len);
            return TRUE;
        }
        else
            return FALSE;
        
    }
}
#if 1
/*+++
  ���ܣ��������¼�����
  ������
        INT16U meter_idx,    ������
        INT8U  *meter_no      ����ַ
        INT8U *data,         ���ݣ�len+ ���ݱ�ʶ+ data(data_pos =5 or 3)
        INT8U datalen        ���ݳ���
---*/
void plc_router_save_meter_event_data(READ_PARAMS *read_params,INT8U* frame,INT8U frame_len)
{
    INT32U  data_item;
    INT32U  offset;
    INT32U  val_and;
    METER_EVENT_ITEM_CTRL  item_ctrl;
    INT16U  meter_idx;
    INT16U  frm_len = 0;
    //INT16U 	crc16;
    INT8U   item_idx;
    INT8U   plan_id,event_level;
    INT8U   flag_report;
    report_ctrl reportCtrl;	
    //INT8U 	port;
    INT8U   frame_data_pos;
    BOOLEAN save_success = FALSE;
    INT8U   value[256];
    
    #ifdef __PROVICE_SHANGHAI__
    INT16U  crc16;
    INT8U   old_crc_data[2]; //�Ϻ��жϱ�־λ
    INT8U new_crc_data[2];
    INT8U crc_idx;
    #endif
    INT8U secondary_item_idx;
    
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    data_item = bin2_int32u(read_params->item);// ���ݱ�ʶ
    plan_id = 0;
    mem_set(value,sizeof(value),0x00);
    mem_set((void *)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL),0x00);
    
    fread_array(meter_idx,PIM_METER_F105,&plan_id,1);
    if((plan_id == 0)||(plan_id > MAX_METER_EVENT_PLAN_COUNT)) 
    {
        return;
    }
    
    if(read_params->meter_doc.protocol == GB645_2007)
        frame_data_pos = 5;
    else if(read_params->meter_doc.protocol == GB645_1997)
        frame_data_pos = 3;
    else if(read_params->meter_doc.protocol == GUANGXI_V30)
        frame_data_pos = 3;
    else 
        return;
    
    item_idx = read_params->event_item_ctrl.event_idx;// ���������λ��			
    event_level = read_params->event_item_ctrl.level;
    if( (event_level == 0) || (event_level > MAX_METER_EVENT_LEVEL) )
    {
        return;
    }
    
    if(frame[frame_data_pos] == REC_DATA_IS_DENY) //���ж�������Ч�ԣ����ϣ��������ʶ
    {
        goto flag_clear;  //���ϲ������¼�,��Ҫ���
    }
    //
    #ifdef EVENT_GRADE_INFO
    snprintf(info,100,"***׼���洢����***����λ=%d",read_params->meter_phase);
    debug_println_ext(info);               
    #endif
    
    reportCtrl.value = 0;
    //port = read_params->meter_doc.baud_port.port;//�˿ں�
    
    if(	read_params->event_item_ctrl.main_record)
    {
        read_params->event_item_ctrl.main_record = FALSE;
        if(read_params->event_item_ctrl.report_flag)
        {
            #ifdef __PROVICE_SHANGHAI__  //2015-11-24 �Ϻ����ӱ�־λ�ж�
            if( (data_item == 0x04000501) || (data_item == 0x04001501) )
            {
                if(data_item == 0x04000501) crc_idx = 0;
                if(data_item == 0x04001501) crc_idx = 1;
                
                //����CRC���жϣ���ͬ���ϱ�״̬��
                // read old crc 
                offset = PIM_METER_EVENT_F106_CRC_DATA;
                offset += crc_idx*2;
                fread_array(meter_idx,offset,old_crc_data,2);
                
                // compute new crc
                crc16=CRC16_2(frame,frame_len,0xFF,0xFF);
                new_crc_data[0] = crc16;
                new_crc_data[1] = crc16>>8;
                
                if( (old_crc_data[0] == 0xFF) && (old_crc_data[1] == 0xFF)  )    //����жϾɵ�У��λΪ��ʼֵFF,���������Ϊ��3��4λ״̬�֡�
                {         
                    fwrite_array(meter_idx,offset,new_crc_data,2);      //д���µ�У�飬�����ϱ�
                
                }
                else
                {
                    if(compare_string(old_crc_data,new_crc_data,2) != 0)
                    {
                        //���汾��CRC����
                        fwrite_array(meter_idx,offset,new_crc_data,2);
                        if(crc_idx == 0)
                        {
                            //��������ж�
                            if ( (frame[5] & 0x0C ) == 0x00 )
                            {
                                goto flag_clear;
                            }
                        }
                        // д�뵽�ϱ��ļ�
                        save_meter_event_record(meter_idx,read_params,frame,frame_len+1,reportCtrl);
                    }
                }
            }
            #else
            //�����ϱ��ļ� TODO
            save_meter_event_record(meter_idx,read_params,frame,frame_len+1,reportCtrl);
            #endif
            //������¼�������־
            goto flag_clear;
        }
        else
        {
            if(check_is_all_ch((INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8,0x00))
            {
                // TODO 
                flag_report = frame_check_data(read_params,frame+5,frame_len-4,FALSE);
                if(flag_report == SAVE_AND_CLEAR_FLAG)
                {
                    save_meter_event_record(meter_idx,read_params,frame,frame_len+1,reportCtrl);
                }
                goto flag_clear;
            }
            else
            {
                //
                flag_report = frame_check_data(read_params,frame+5,frame_len-4,TRUE);
                if(flag_report == SAVE_AND_KEEP_ON_RECORDING) 
                {
                    reportCtrl.report_flag = FALSE;//�����ϱ�
                    reportCtrl.main_item_flag = TRUE;//��Ҫ������
                    // TODO  �ݴ浽���ļ� ?????
                    save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                    return;//
                }
                else goto flag_clear;//ֻ��� ���ϱ�
            }
        }
    }
    else
    {
        if( (((data_item>>24)>=0x10) && ((data_item>>24)<=0x20)) || ((data_item>>16)==0x0312) )
        {
            val_and = 0xFFFF00F0;
        }
        else
        {
            val_and = 0xFFFFFFF0;
        }
        
        if( (read_params->event_item_ctrl.item &val_and) == (data_item & val_and) )//����������
        {
            //save 		
            if(read_params->meter_doc.protocol ==  GB645_2007)
            {	
                if(read_params->event_item_ctrl.item != 0x03120000) // ����������
                {
                    //clear 
                    clr_bit_value((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,8,read_params->event_item_ctrl.secondary_item_idx);
                    #ifdef EVENT_GRADE_INFO
                    snprintf(info,100,"***��������λ��= %d, cur_rec_flag_High=0x%08X.cur_rec_flag_Low=0x%08X*** ",
                    read_params->event_item_ctrl.secondary_item_idx,read_params->event_item_ctrl.EventCtrl.copy_flag[1],
                    read_params->event_item_ctrl.EventCtrl.copy_flag[0]);
                    debug_println_ext(info);               
                    #endif
                    reportCtrl.appendix_item = 1;//����������
                    if(check_is_all_ch((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,8,0x00))
                    {					
                        read_params->event_item_ctrl.EventCtrl.count --;
                        read_params->event_item_ctrl.EventCtrl.need_add_cnt ++;
                        #ifdef EVENT_GRADE_INFO
                        snprintf(info,100,"***�����ĳ��������͸������� change_cnt=%d,rec_add_cnt=%d*** ",
                        read_params->event_item_ctrl.EventCtrl.count,read_params->event_item_ctrl.EventCtrl.need_add_cnt);
                        debug_println_ext(info);               
                        #endif
                        if(read_params->event_item_ctrl.EventCtrl.count == 0)
                        {	
                            read_params->event_item_ctrl.EventCtrl.need_add_cnt = 0;
                            reportCtrl.read_tmp_and_save = 1;	
                            // TODO ���� ?????
                            save_success = save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                            if(save_success == TRUE)
                            {			
                                offset = PIM_SAVE_TMP_REPORT_DATA_START;
                                fread_array(meter_idx,offset,(INT8U *)&frm_len,2);
                                //value[0] = frm_len;							
                                tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                                fread_array(meter_idx,offset+2,g_temp_buffer,frm_len);
                                save_meter_event_record(0,NULL,g_temp_buffer,frm_len,reportCtrl);
                                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                            
                            }						
                            goto flag_clear;
                        }
                        else
                        {
                            // TODO  �ݴ�����
                            save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                            mem_cpy((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,(INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8);
                            return;
                        }
                    }
                    else
                    {
                        #ifdef EVENT_GRADE_INFO
                        snprintf(info,100,"***δ�����������͸������� change_cnt=%d,rec_add_cnt=%d*** ",
                        read_params->event_item_ctrl.EventCtrl.count,read_params->event_item_ctrl.EventCtrl.need_add_cnt);
                        debug_println_ext(info);               
                        #endif
                        save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                    }
                }
                else //��������
                {
                    secondary_item_idx = read_params->event_item_ctrl.secondary_item_idx;
                    read_params->event_item_ctrl.EventCtrl.xl_cnt[secondary_item_idx]--;
                    read_params->event_item_ctrl.EventCtrl.xl_add_cnt[secondary_item_idx]++;
                    if(read_params->event_item_ctrl.EventCtrl.xl_cnt[secondary_item_idx] == 0)
                    {
                        read_params->event_item_ctrl.EventCtrl.xl_add_cnt[secondary_item_idx] = 0; //clear
                        //�����������ĳ�����־
                        clr_bit_value((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,8,read_params->event_item_ctrl.secondary_item_idx);
                        reportCtrl.read_tmp_and_save = 1;	
                        // TODO ���� ?????
                        save_success = save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                        if(save_success == TRUE)
                        {			
                            offset = PIM_SAVE_TMP_REPORT_DATA_START;						
                            fread_array(meter_idx,offset,(INT8U *)&frm_len,2);
                            //value[0] = frm_len;
                            tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                            fread_array(meter_idx,offset+2,g_temp_buffer,frm_len);
                            save_meter_event_record(0,NULL,g_temp_buffer,frm_len,reportCtrl);
                            tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                        
                        }
                        //�޳����������ˣ����
                        if(check_is_all_ch(read_params->event_item_ctrl.EventCtrl.xl_cnt,6,0x00) )
                            goto flag_clear;
                        return;// �����ݳ���������
                    }
                    else
                    {			
                        //���浽��ʱ�ļ���
                        save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                        return;
                    }
                    #ifdef __TEST____
                    //clear 
                    clr_bit_value((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,8,read_params->event_item_ctrl.secondary_item_idx);
                    #ifdef EVENT_GRADE_INFO
                    snprintf(info,100,"***��������λ��= %d, cur_rec_flag_High=0x%08X.cur_rec_flag_Low=0x%08X*** ",
                    read_params->event_item_ctrl.secondary_item_idx,read_params->event_item_ctrl.EventCtrl.copy_flag[1],
                    read_params->event_item_ctrl.EventCtrl.copy_flag[0]);
                    debug_println_ext(info);               
                    #endif
                    reportCtrl.appendix_item = 1;//����������
                    if(check_is_all_ch((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,8,0x00))
                    {					
                        read_params->event_item_ctrl.EventCtrl.count --;
                        read_params->event_item_ctrl.EventCtrl.need_add_cnt ++;
                        #ifdef EVENT_GRADE_INFO
                        snprintf(info,100,"***�����ĳ��������͸������� change_cnt=%d,rec_add_cnt=%d*** ",
                        read_params->event_item_ctrl.EventCtrl.count,read_params->event_item_ctrl.EventCtrl.need_add_cnt);
                        debug_println_ext(info);               
                        #endif
                        if(read_params->event_item_ctrl.EventCtrl.count == 0)
                        {	
                            read_params->event_item_ctrl.EventCtrl.need_add_cnt = 0;
                            reportCtrl.read_tmp_and_save = 1;	
                            // TODO ���� ?????
                            save_success = save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                            if(save_success == TRUE)
                            {			
                                offset = PIM_SAVE_TMP_REPORT_DATA_START;
                                fread_array(meter_idx,offset,(INT8U *)&frm_len,2);
                                //value[0] = frm_len;							
                                tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                                fread_array(meter_idx,offset+2,g_temp_buffer,frm_len);
                                save_meter_event_record(0,NULL,g_temp_buffer,frm_len,reportCtrl);
                                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
                            
                            }						
                            goto flag_clear;
                        }
                        else
                        {
                            // TODO  �ݴ�����
                            save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                            mem_cpy((INT8U *)read_params->event_item_ctrl.EventCtrl.copy_flag,(INT8U *)read_params->event_item_ctrl.EventCtrl.secondary_record_flag,8);
                            return;
                        }
                    }
                    else
                    {
                        #ifdef EVENT_GRADE_INFO
                        snprintf(info,100,"***δ�����������͸������� change_cnt=%d,rec_add_cnt=%d*** ",
                        read_params->event_item_ctrl.EventCtrl.count,read_params->event_item_ctrl.EventCtrl.need_add_cnt);
                        debug_println_ext(info);               
                        #endif
                        save_tmp_data_in_meter_file(read_params,frame,frame_len+1,reportCtrl,value,sizeof(value));
                    }
                    #endif
                }
            }
            else//TODO
            {
            
            }
            
        }
        #ifdef EVENT_GRADE_INFO
        snprintf(info,100,"***�����洢���ݣ����ϱ�***����λ=%d",read_params->meter_phase);
        debug_println_ext(info);               
        #endif
        return ;//����
    }
    flag_clear:
    // TODO ��������Ĵ�����?????
    fread_array(FILEID_EVENT_GRADE_READ_ITEM_CTRL,PIM_READ_EVENT_GRADE_READ_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL));
    if(item_ctrl.valid_flag == 0x55)
    {		
        item_ctrl.valid_flag = 0xFF;
        fwrite_array(FILEID_EVENT_GRADE_READ_ITEM_CTRL,PIM_READ_EVENT_GRADE_READ_ITEM_CTRL_SIZE*(meter_idx-1),(INT8U *)&item_ctrl,sizeof(METER_EVENT_ITEM_CTRL));		
        #ifdef EVENT_GRADE_INFO
        snprintf(info,100,"***��������д����Ч��ֵ***����λ=%d��valid_flag = 0x%02X",item_ctrl.valid_flag);
        debug_println_ext(info);               
        #endif
    }
    read_params->event_item_ctrl.valid_flag = 0xFF;//���FF�����������ʱ��֪���˵�ǰ�¼�����Ҫ����
    if(item_idx >= MAX_METER_EVENT_ITEM_LEVEL_COUNT)
    {
        mem_set(read_params->read_mask.meter_event_grade+(event_level-1)*MAX_METER_EVENT_ITEM_FLAG_COUNT,MAX_METER_EVENT_ITEM_FLAG_COUNT,0x00);// �쳣����
    }
    else
    {
        clr_bit_value(read_params->read_mask.meter_event_grade+(event_level-1)*MAX_METER_EVENT_ITEM_FLAG_COUNT,MAX_METER_EVENT_ITEM_FLAG_COUNT,item_idx);//
    }
    #ifdef EVENT_GRADE_INFO
    snprintf(info,100,"***�����洢���ݣ��ϱ�or���ϱ�***����λ=%d��report_flag = %d",read_params->meter_phase,reportCtrl.report_flag);
    debug_println_ext(info);               
    #endif
    return;   
}
#endif

#if 1
/*+++
  ���ܣ� �������¼���¼
  ������
         INT32U item
         INT8U* data
         INT16U datalen
  ���أ�
         TRUE  ��Ҫ����
         FALSE �������
  ������

---*/
void save_meter_event_record(INT16U meter_idx,READ_PARAMS *read_params,INT8U *frame,INT16U framelen,report_ctrl reportCtrl)
{
    INT32U 	offset;
	//INT32U 	tmp_offset;	
    METER_EVENT_REC_HEAD rec_head[8],rec_head_last;
    INT16U 	count,head_pos;
	INT16U 	tmp_len;
    INT8U 	idx,file_idx;//,last_datalen;
    INT8U 	read_count,tmp[20];

    file_idx = 0;
	mem_set(tmp,sizeof(tmp),0x00);
	mem_set(rec_head,sizeof(rec_head),0x00);
    fread_array(FILEID_RUN_PARAM,FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,&file_idx,1);

    if(file_idx >= FILEID_EVENT_GRADE_REPORT_RECORD_COUNT)
    {
        file_idx = 0;
        file_delete(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx);

        fwrite_array(FILEID_RUN_PARAM,FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,&file_idx,1);
        #ifdef __DEBUG_RECINFO__
        snprintf(info,100,"****use file_id = %d",file_idx);
        debug_print(info);
        #endif
    }

FIND_SAVE_POS:

    read_count = 8;
    count = MAX_METER_EVENT_RECORD_COUNT;
    head_pos = 0;
    rec_head_last.value = 0xFFFF;
    while(count > 0)
    {
        if(count < read_count) read_count = count;
        count -= read_count;
        offset = PIM_METER_EVENT_RECORD_HEAD + head_pos*sizeof(METER_EVENT_REC_HEAD)*read_count;
        fread_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,(INT8U*)&(rec_head[0].value),sizeof(METER_EVENT_REC_HEAD)*read_count);

        for(idx=0;idx<read_count;idx++)
        {
            if( (rec_head[idx].value == 0xFFFF) )//�ҵ��� ��Ҫ������ ��Ҫ���µ�λ�� && (reportCtrl.main_item_flag)
            {
                if(rec_head_last.value == 0xFFFF)  //˵�����ļ��ļ��ĵ�һ��λ��
                {
                	
            		//�����ϱ��Ļ���д��ֵ
            		rec_head_last.is_report = 1;
                    rec_head_last.offset = 0;
                    offset = PIM_METER_EVENT_RECORD_HEAD + head_pos*sizeof(METER_EVENT_REC_HEAD)*read_count;
                    offset += sizeof(METER_EVENT_REC_HEAD)*idx;
                    fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,(INT8U*)&rec_head_last,sizeof(METER_EVENT_REC_HEAD));
						
                    // ���Ҵ洢λ��
                    if(reportCtrl.read_tmp_and_save)// 
                    {
                    	//tmp[0] = framelen;
                    	tmp_len = framelen;
                    	offset = PIM_METER_EVENT_RECORD_DATA + rec_head_last.offset;  
						fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,(INT8U *)&tmp_len,2);
                    	fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset+2,frame,framelen);
                    }
					else
					{
					    // read_params������������NULL�����ϱ���
					    if(NULL == read_params)
					    {
					        return;
					    }
						// 2�ֽڳ��� ����9�ֽڵ��ϱ���Ϣ = 11
						tmp_len = framelen+9;
						tmp[0] = tmp_len & 0x00FF;
						tmp[1] = (tmp_len>>8) & 0x00FF; // 2���ֽ�
						tmp[2] = read_params->meter_doc.baud_port.port;
						mem_cpy(tmp+3,read_params->meter_doc.meter_no,6);  //��ַ
						tmp[9] = 6;//6 TODO
						tmp[10] = 1;
                        offset = PIM_METER_EVENT_RECORD_DATA + rec_head_last.offset;
                        fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,tmp,11);
                        fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset+11,frame,framelen);					
					}
               		
						
                    
                    #ifdef __DEBUG_RECINFO__
                    snprintf(info,100,"****file_id = %d, idx = %03d, offset = %05d, datalen = %02d ,next_offset = %05d",
                            file_idx,head_pos*read_count+idx,offset,tmp[1]<<8+tmp[0],offset+tmp[1]<<8+tmp[0]+1);
                    debug_print(info);
                    #endif
                    				
					//�����ϱ���־
                    gSendMeterEvent.begin_idx.report_flag = 1;//
                    return;
                }
                else
                {
                    offset = PIM_METER_EVENT_RECORD_DATA + rec_head_last.offset;
                    fread_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,tmp,2);
					tmp_len = bin2_int16u(tmp);
                    offset = rec_head_last.offset + tmp_len + 2;
                    if(offset >= 0x7FFF)
                    {
                        //��Ҫ��һ���ļ��洢
                        file_idx++;
                        if(file_idx >= FILEID_EVENT_GRADE_REPORT_RECORD_COUNT) file_idx = 0;
                        file_delete(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx);
                   //     fwrite_array(FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,0,&file_idx,2);
                        fwrite_array(FILEID_RUN_PARAM,FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,&file_idx,1);
                        #ifdef __DEBUG_RECINFO__
                        snprintf(info,100,"****use file_id = %d",file_idx);
                        debug_print(info);
                        #endif
                        goto FIND_SAVE_POS;
                    }
                    else
                    {
                        //tmp_offset = rec_head_last.offset+tmp[1]*256 + tmp[0] + 2;

                        rec_head_last.offset += tmp_len + 2;
                        rec_head_last.is_report = 1;
                        offset = PIM_METER_EVENT_RECORD_HEAD + head_pos*sizeof(METER_EVENT_REC_HEAD)*read_count;
                        offset += sizeof(METER_EVENT_REC_HEAD)*idx;
                        fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,(INT8U*)&rec_head_last,sizeof(METER_EVENT_REC_HEAD));

						if(reportCtrl.read_tmp_and_save)// 
						{
							tmp_len = framelen;
							offset = PIM_METER_EVENT_RECORD_DATA + rec_head_last.offset;
							fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,(INT8U *)&tmp_len,2);
							fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset+2,frame,framelen);
						}
						else
						{
						    // read_params������������NULL�����ϱ���
    					    if(NULL == read_params)
    					    {
    					        return;
    					    }
							//tmp[0] [1] ռ���ֽڲ����㵽������
							tmp_len = framelen+9;
							tmp[0] = tmp_len & 0x00FF;
							tmp[1] = (tmp_len>>8) & 0x00FF; 
							tmp[2] = read_params->meter_doc.baud_port.port;
							mem_cpy(tmp+3,read_params->meter_doc.meter_no,6);  //��ַ
							tmp[9] = 6;//6 TODO
							tmp[10] = 1;
	                        offset = PIM_METER_EVENT_RECORD_DATA + rec_head_last.offset;
	                        fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,tmp,11);
	                        fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset+11,frame,framelen);

	                        #ifdef __DEBUG_RECINFO__
	                        snprintf(info,100,"****file_id = %d, idx = %03d, offset = %05d, datalen = %02d ,next_offset = %05d",
	                                file_idx,head_pos*read_count+idx,offset,tmp[0],offset+tmp[0]+2);
	                        debug_print(info);
	                        #endif
						}					
                        //�����ϱ���־
                        gSendMeterEvent.begin_idx.report_flag = 1;
                        return;
                    }
                }
            }
            rec_head_last.value = rec_head[idx].value;
        }
        head_pos ++;
    }

    //�����˵�����ļ������ˣ�Ҫ���ļ���
    file_idx++;
    if(file_idx >= FILEID_EVENT_GRADE_REPORT_RECORD_COUNT) file_idx = 0;
    file_delete(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx);
//    fwrite_array(FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,0,&file_idx,2);
    fwrite_array(FILEID_RUN_PARAM,FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,&file_idx,1);
    #ifdef __DEBUG_RECINFO__
    snprintf(info,100,"****use file_id = %d",file_idx);
    debug_print(info);
    #endif
    goto FIND_SAVE_POS;
}
#else
/*+++
  ���ܣ� �������¼���¼
  ������
         INT32U item
         INT8U* data
         INT16U datalen
  ���أ�
         TRUE  ��Ҫ����
         FALSE �������
  ������

---*/
void save_meter_event_record(INT8U port,INT8U *meter_no,INT32U item,INT8U* data,INT16U datalen)
{
    INT32U offset;
    METER_EVENT_REC_HEAD rec_head[8],rec_head_last;
    INT16U count,head_pos;
    INT8U idx,file_idx,last_datalen;
    INT8U read_count,tmp[20];
    PARAM_F106 f106;
    INT8U* buffer;


    INT8U tmp_data[100];

     fread_array(FILEID_RUN_PARAM,FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,&file_idx,2);

    if(file_idx >= FILEID_EVENT_GRADE_REPORT_RECORD_COUNT)
    {
        file_idx = 0;
        file_delete(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx);

        fwrite_array(FILEID_RUN_PARAM,FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,&file_idx,2);
        #ifdef __DEBUG_RECINFO__
        snprintf(info,100,"****use file_id = %d",file_idx);
        debug_print(info);
        #endif
    }

FIND_SAVE_POS:

    read_count = 8;
    count = MAX_METER_EVENT_RECORD_COUNT;
    head_pos = 0;
    rec_head_last.value = 0xFFFF;
    while(count > 0)
    {
        if(count < read_count) read_count = count;
        count -= read_count;
        offset = PIM_METER_EVENT_RECORD_HEAD + head_pos*sizeof(METER_EVENT_REC_HEAD)*read_count;
        fread_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,(INT8U*)&(rec_head[0].value),sizeof(METER_EVENT_REC_HEAD)*read_count);

        for(idx=0;idx<read_count;idx++)
        {
            if(rec_head[idx].value == 0xFFFF) //�ҵ���
            {
                if(rec_head_last.value == 0xFFFF)  //˵�����ļ��ļ��ĵ�һ��λ��
                {
                    rec_head_last.is_report = 1;
                    rec_head_last.offset = 0;
                    offset = PIM_METER_EVENT_RECORD_HEAD + head_pos*sizeof(METER_EVENT_REC_HEAD)*read_count;
                    offset += sizeof(METER_EVENT_REC_HEAD)*idx;
                    fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,(INT8U*)&rec_head_last,sizeof(METER_EVENT_REC_HEAD));
                    tmp[0] = 14+datalen;
                    tmp[1] = port;              //�˿�
                    tmp[2] = 0;                 //�м̼���
                    mem_cpy(tmp+3,meter_no,6);  //��ַ
                    tmp[9] = 5;                 //ת�������ʶ.�ĳ�1���з�D2-D7����
                    tmp[10] = 1;                 //ת�����ݱ�ʶ����
                    tmp[11] = 4 + datalen;
                    tmp[12] = item;
                    tmp[13] = item>>8;
                    tmp[14] = item>>16;
                    tmp[15] = item>>24;
                    offset = PIM_METER_EVENT_RECORD_DATA + rec_head_last.offset;
                    int test_no = FILEID_EVENT_GRADE_REPORT_RECORD+file_idx;
                    fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,tmp,15);
                    fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset+15,data,datalen);
                    #ifdef __DEBUG_RECINFO__
                    snprintf(info,100,"****file_id = %d, idx = %03d, offset = %05d, datalen = %02d ,next_offset = %05d",
                            file_idx,head_pos*read_count+idx,offset,tmp[0],offset+tmp[0]+1);
                    debug_print(info);
                    #endif
                    //�����ϱ���־
                    gSendMeterEvent.begin_idx.report_flag = 1;//
                    return;
                }
                else
                {
                    offset = PIM_METER_EVENT_RECORD_DATA + rec_head_last.offset;
                    fread_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,tmp,1);
                    offset = rec_head_last.offset + tmp[0] + 1;
                    if(offset >= 0x7FFF)
                    {
                        //��Ҫ��һ���ļ��洢
                        file_idx++;
                        if(file_idx >= FILEID_EVENT_GRADE_REPORT_RECORD_COUNT) file_idx = 0;
                        file_delete(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx);
                   //     fwrite_array(FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,0,&file_idx,2);
                        fwrite_array(FILEID_RUN_PARAM,FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,&file_idx,2);
                        #ifdef __DEBUG_RECINFO__
                        snprintf(info,100,"****use file_id = %d",file_idx);
                        debug_print(info);
                        #endif
                        goto FIND_SAVE_POS;
                    }
                    else
                    {
                        rec_head_last.offset += tmp[0] + 1;
                        rec_head_last.is_report = 1;
                        offset = PIM_METER_EVENT_RECORD_HEAD + head_pos*sizeof(METER_EVENT_REC_HEAD)*read_count;
                        offset += sizeof(METER_EVENT_REC_HEAD)*idx;
                        fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,(INT8U*)&rec_head_last,sizeof(METER_EVENT_REC_HEAD));
                        tmp[0] = 14+datalen;
                        tmp[1] = port;              //�˿�
                        mem_cpy(tmp+2,meter_no,6);  //��ַ
                        tmp[8] = 5;                 //ת�������ʶ
                        tmp[9] = 1;                 //ת�����ݱ�ʶ����
                        tmp[10] = 4 + datalen;
                        tmp[11] = item;
                        tmp[12] = item>>8;
                        tmp[13] = item>>16;
                        tmp[14] = item>>24;
                        offset = PIM_METER_EVENT_RECORD_DATA + rec_head_last.offset;
                        fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset,tmp,15);
                        fwrite_array(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx,offset+15,data,datalen);

                        #ifdef __DEBUG_RECINFO__
                        snprintf(info,100,"****file_id = %d, idx = %03d, offset = %05d, datalen = %02d ,next_offset = %05d",
                                file_idx,head_pos*read_count+idx,offset,tmp[0],offset+tmp[0]+1);
                        debug_print(info);
                        #endif
                        //�����ϱ���־
                        gSendMeterEvent.begin_idx.report_flag = 1;
                        return;
                    }
                }
            }
            rec_head_last.value = rec_head[idx].value;
        }
        head_pos ++;
    }

    //�����˵�����ļ������ˣ�Ҫ���ļ���
    file_idx++;
    if(file_idx >= FILEID_EVENT_GRADE_REPORT_RECORD_COUNT) file_idx = 0;
    file_delete(FILEID_EVENT_GRADE_REPORT_RECORD+file_idx);
//    fwrite_array(FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,0,&file_idx,2);
    fwrite_array(FILEID_RUN_PARAM,FLADDR_METER_EVENT_RECORD_USE_FILE_IDX,&file_idx,2);
    #ifdef __DEBUG_RECINFO__
    snprintf(info,100,"****use file_id = %d",file_idx);
    debug_print(info);
    #endif
    goto FIND_SAVE_POS;
}
#endif

void plc_router_save_report_meter_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen)
{
	INT16U 	offset;
	INT16U	event_len = 0; //���ȱ�Ϊ2�ֽ� ����������� ����ʡ��Ҳ�ĳ�2�ֽ���
    //INT8U 	flag[12];
    INT16U  idx = 0;
	INT8U	idx1,len;
	
    //INT8U 	ctrl_flag;
	INT8U 	tmp[256]={0};
	report_ctrl reportCtrl;
	EVENT_READ_CTRL read_ctrl;
	METER_DOCUMENT meter_doc;
	#ifdef __PROVICE_JIANGXI__ //������ֻ��Ӧ�����¼� ��Ҫ�¼����������ϱ���ֻ�����
	//�����¼�  ����� bit10 bit12 �㶨�ų�����  bit20 bit36 bit52 A B C ���� bit 81 �������
	INT8U level1_status[12] = {0x00,0x14,0x10,0x00,0x10,0x00,0x10,0x00,0x00,0x00,0x02,0x00};				
	#endif
	#ifdef __MEXICO_GUIDE_RAIL__
    INT8U erc_no;
    INT16U spot_idx = 0;
    INT8U event_set[32] = {0};
    INT8U read_sta[2] = {0};
	#endif
    #ifdef __PROVICE_HUBEI__
    INT8U   event[30+EVENT_HEAD_LEN] = {0};
    INT8U   pos = 0;
    #endif
	
    //fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
	fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
	fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
	if( (read_ctrl.ctrl_flag == 0x00) || (read_ctrl.ctrl_flag == 0xFF) )
	{
		return ;
	}
    if(item == 0x04001501)
    {
        if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY))
        {
            //ctrl_flag = 0x00;
            //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
			read_ctrl.ctrl_flag = 0x00;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
        }
        #ifdef __MEXICO_GUIDE_RAIL__
        else if(check_is_all_ch(data,(datalen > MEXICO_RAIL_EVENT_BYTE_CNT) ? MEXICO_RAIL_EVENT_BYTE_CNT : datalen,0x00))
        #else
        else if(check_is_all_ch(data,(datalen > 12) ? 12 : datalen,0x00))
        #endif
        {
            if(meter_doc.protocol == 30)//GB645_2007
            {
				read_ctrl.ctrl_flag = 0x00;
            	fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));

            }
        }
        else
        {
            //datalen = datalen > 108 ? 108 : datalen;
            //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,data,datalen);
            //save_meter_event_record(COMMPORT_PLC,meter_no,item,data,datalen);
            //ctrl_flag &= ~0x02;
            //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
             
            datalen = (datalen > (sizeof(DATA_04001501) - 5U)) ? (sizeof(DATA_04001501) - 5U) : datalen;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL)+1,(INT8U*)&item,4);
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL)+1+4,data,datalen);
            tmp[0] = datalen + 4U;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL),tmp,1);

            #ifndef __MEXICO_GUIDE_RAIL__ /* ī���絼��� ��ͨ��10F10�ϱ� */
            offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001501);
            idx = 0;
            #ifdef __EVENT_04001501_BEFORE__
            tmp[idx++] = 14 + datalen;      //�ܳ�
            tmp[idx++] = ((INT16U)(14 + datalen))>>8; // 2�ֽڳ���
            tmp[idx++] = meter_doc.baud_port.port;//COMMPORT_PLC;
            mem_cpy(tmp+idx,meter_no,6);
            idx += 6;
            tmp[idx++] = 6;  //ת�������ʶ
            tmp[idx++] = 1;  //ת��ֱ�ӳ��������ݱ�ʶ����n
            tmp[idx++] = 4 + datalen;
            mem_cpy(tmp+idx,(INT8U*)&item,4);
            idx += 4;
            mem_cpy(tmp+idx,data,datalen);
            idx += datalen;
            #else
            tmp[idx++] = 9;      //�ܳ�
            tmp[idx++] = 0;  //�ܳ��� 2�ֽ� (port 1+ 6 addr+ result 1+ count 1 = 9)
            tmp[idx++] = meter_doc.baud_port.port;//COMMPORT_PLC;
            mem_cpy(tmp+idx,meter_no,6);
            idx += 6;
            tmp[idx++] = 6;  //ת�������ʶ
            tmp[idx++] = 0;  //ת��ֱ�ӳ��������ݱ�ʶ����n
            #endif
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,tmp,idx);
            //save_meter_event_record(COMMPORT_PLC,meter_no,item,data,datalen);
            #endif  /* end of #ifndef __MEXICO_GUIDE_RAIL__ */

            read_ctrl.ctrl_flag &= ~0x02;
            len = (datalen > 12) ? 12 : datalen;
            mem_set(read_ctrl.meter_state_word,12,0x00);
			
			#ifdef __PROVICE_HUNAN__
			if(meter_doc.baud_port.port == READPORT_PLC)//�ز��ű����¼�,�����˿�д�����00
			#endif
			{
            	mem_cpy(read_ctrl.meter_state_word,data,len);
            	#ifdef __MEXICO_GUIDE_RAIL__
                /* ��ȡF9 ����ERC65��79 �Ƿ����� �����Ƿ���Ҫ���� 
                 * 2018-09-13 ������Ҫ������Ч �������������¼� 
                 */ 
                fread_ertu_params(EEADDR_SET_F9,event_set,32);
                bit_value_opt_or(event_set+24,event_set+8,MEXICO_RAIL_EVENT_BYTE_CNT);
                bit_value_opt_and(read_ctrl.meter_state_word,event_set+24,MEXICO_RAIL_EVENT_BYTE_CNT);
                #endif
				#ifdef __PROVICE_JIANGXI__ //������ֻ��Ӧ�����¼� ��Ҫ�¼����������ϱ���ֻ�����
				//�����¼�  ����� bit10 bit12 �㶨�ų�����  bit20 bit36 bit52 A B C ���� bit 81 �������
				//INT8U level1_status[12] = {0x00,0x14,0x10,0x00,0x10,0x00,0x10,0x00,0x00,0x00,0x02,0x00};
				bit_value_opt_and(read_ctrl.meter_state_word,level1_status,12);
				#endif
			}
            read_ctrl.event_count.value = 0;
            read_ctrl.event_mask[0] = 0;
			read_ctrl.rec_event_cnt = 0;//��������

			//�����ϱ����¼��У��޽����¼����Ƿ�ֻ���״̬����?״̬���Ƿ��ϱ�?
			#ifdef __PROVICE_JIANGXI__
            if( TRUE == check_is_all_ch(read_ctrl.meter_state_word,12,0x00) )
            {
                //�޽����¼�������������������¼���¼�ı�־
                read_ctrl.ctrl_flag &= ~0x08;
            }
			#endif
			
			#ifdef __MEXICO_GUIDE_RAIL__
            if( TRUE == check_is_all_ch(read_ctrl.meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,0x00) )
            {
                /*F9 δ������Ҫ�¼�������������������¼���¼�ı�־ */
                read_ctrl.ctrl_flag &= ~0x08;
            }
			#endif
			
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));

            #ifdef __PROVICE_HUBEI__ //·���ϱ�״̬�֣�ֱ������ERC12
            if(get_bit_value(read_ctrl.meter_state_word,12,3)) //hubei
            {
                           //2. ��¼�¼�����
                event[EVENT_POS_ERC_NO] = ERC12;

                event[EVENT_POS_ERC_LEN] = 7;

                //3.����ʱ��
                set_event_datetime(event+EVENT_POS_ERC_TIME);

                pos=EVENT_POS_ERC_CONTENT;
                //������ĵ�8λ
                event[pos++] = meter_idx;

                    //������ĸ�4λ
                event[pos++] = meter_idx >> 8;

                //��ֹ��־
                event[pos-1] |= 0x80;

                //3. ��¼�¼�
                save_event_record(event,VIPEVENT_FLAG);

            }
            #endif
        }
    }
    else if (item == 0x04001503)
    {
        if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY))
        {
            //ctrl_flag &= ~0x04;
            //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
			read_ctrl.ctrl_flag = 0x00;
			//read_ctrl.ctrl_flag &= ~0x04;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,1);
        }
        else
        {
            //ctrl_flag &= ~0x04;
            //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl_flag,1);
            #ifdef __PROVICE_HUNAN__
			read_ctrl.ctrl_flag &= ~0x0C;
			#else
			read_ctrl.ctrl_flag &= ~0x04;
			#endif
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,1);
        }
    }
    else
    {
        if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY) == FALSE)
        {
            #ifdef __MEXICO_GUIDE_RAIL__
            /*TODO  �����¼� ERC65~ERC79 */
            erc_no = ERC65;
            erc_no += (item - 0x04001600);
            spot_idx = bin2_int16u(meter_doc.spot_idx);
            /*�ϱ���ʱ�� ����һ���ֽ� ��040016XX�е�XX��һ���� �������� */
            event_guide_rail_erc(spot_idx,erc_no,data+1,datalen-1);
            #else
            //save_meter_event_record(COMMPORT_PLC,meter_no,item,data,datalen);
            offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001501);
            fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,(INT8U *)&event_len,2);
            if ((event_len + 1U + 4U + datalen) >= 850U)
            {
                //ʹ����ʱ���������粻���ʣ���������
                tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
				reportCtrl.read_tmp_and_save = 1;//д���ϱ��ļ���ȥ
                save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);

                idx = 0;
                tmp[idx++] = 14 + datalen;      //�ܳ�
                tmp[idx++] = ((INT16U)(14 + datalen))>>8; // 2�ֽڳ���
                tmp[idx++] = meter_doc.baud_port.port;//COMMPORT_PLC;
                mem_cpy(tmp+idx,meter_no,6);
                idx += 6;
                tmp[idx++] = 6;  //ת�������ʶ
                tmp[idx++] = 1;  //ת��ֱ�ӳ��������ݱ�ʶ����n
                tmp[idx++] = 4U + datalen;
                mem_cpy(tmp+idx,(INT8U*)&item,4);
                idx += 4U;
                mem_cpy(tmp+idx,data,datalen);
                idx += datalen;
                fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,tmp,idx);
            }
            else
            {
                idx = 0;
                tmp[idx++] = 4U + datalen;
                mem_cpy(tmp+idx,(INT8U*)&item,4);
                idx += 4U;
                mem_cpy(tmp+idx,data,datalen);
                idx += datalen;
                fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset+2+event_len,tmp,idx);
                fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,tmp,11);
                //tmp[0] = tmp[0] + 5U + datalen;
                event_len = bin2_int16u(tmp);
				event_len += 5U + datalen;
                tmp[0] = (INT8U )(event_len&0x00FF);//����
				tmp[1] = (INT8U )(event_len>>8);
                tmp[10]++;
                fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+offset,tmp,11);
            }
            #endif
        }

        #ifdef __MEXICO_GUIDE_RAIL__
        for(idx=0;idx<8*MEXICO_RAIL_EVENT_BYTE_CNT;idx++)
        {
            if(get_bit_value(read_ctrl.meter_state_word,MEXICO_RAIL_EVENT_BYTE_CNT,idx))
            {
                /**/
                if(read_ctrl.event_count.mask)
                {
                    read_ctrl.event_count.mask = 0;
                    break;
                }
            }
        }
        #else
        //fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,flag,12);
        for(idx=0;idx<96;idx++)
        {
        	/*
            if(get_bit_value(flag,12,idx))
            {
                clr_bit_value(flag,12,idx);
                fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+1,flag,12);
                break;
            }
            */
            if(get_bit_value(read_ctrl.meter_state_word,12,idx))
            {
            	if(read_ctrl.event_count.mask)
                {
                    read_ctrl.event_count.mask = 0;
                    break;
                }

                for(idx1=0;idx1<8;idx1++)
                {
                    if(get_bit_value(read_ctrl.event_mask,1,idx1))
                    {
                        clr_bit_value(read_ctrl.event_mask,1,idx1);
                        break;
                    }
                }
                break;
            }
        }

        if(check_is_all_ch(read_ctrl.event_mask,12,0x00))
        {
            read_ctrl.ctrl_flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,1);

        }
        #endif
        fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
    }
}

/*
void plc_router_save_cjq_meter_event_state_data(INT16U meter_idx1,INT8U *meter_no,INT8U *data,INT8U datalen,BOOLEAN is_catch)
{
    INT16U meter_seq,save_pos;
    INT8U z_count,count,idx,read_flag_byte_num;
    INT8U event_read_flag[MAX_METER_EVENT_ITEM_LEVEL_COUNT/8];

    z_count = data[0];     //֮ǰ�Ĳɼ����¼��ϱ����������ϱ����¼��ĵ������������Ϊ1����������ȥ�������¼���������ֱ���ϱ�״̬�֣��洢�󣬰��ն�Ӧ��ȥ������
    count = data[1];

    for(idx=0;idx<count;idx++)
    {
        if(TRUE == find_meter_no_from_fast_index(data+2+idx*8,&meter_seq,&save_pos))
        {
            meter_seq &= IDX_MASK;
            if((meter_seq > 0) && (meter_seq <= MAX_METER_COUNT))
            {
                //����1���¼���������
                read_flag_byte_num = MAX_METER_EVENT_ITEM_LEVEL_COUNT/8;
                fread_array(FILEID_EVENT_GRADE_READ_FALG,meter_seq*read_flag_byte_num,event_read_flag,read_flag_byte_num);
                event_read_flag[0] |= data[2+idx*8+7];
                fwrite_array(FILEID_EVENT_GRADE_READ_FALG,meter_seq*read_flag_byte_num,event_read_flag,read_flag_byte_num);
                //����ɼ�������
                meter_read_flag_flash_clear_one_flag(meter_seq,READFLAG_REPORT_CJQ_EVENT);
            }
        }
    }

}
 */

/*+++
  ���ܣ� ����̨��������Ϣ
  ������
         INT8U *frame ����645����
         INT8U meter_protocol �ӽڵ�Э��
  ���أ�
         BOOLEAN
  ������
       1) 68H	A0	����	A5	68H	9EH	07H	03H	PA0	����	PA5	CS	16H
       2) �����룺C=9EH
       3) ��ַ�򣺵���ַ
       4) ���ݵ�Ԫ��ʶ��D0H
       5) ���ݵ�Ԫ���ݣ�PA0~ PA5Ϊ�����ز����ڵ��ַ
       ע�⣺���������˿����չ�ļ��ݣ�����������һ��AREA.
       68H	A0	����	A5	68H	9EH	08H	03H	AREA	PA0	����	PA5	CS	16H
---*/
void save_area_different_info(INT8U *frame,INT8U meter_protocol)
{
    INT16U count,idx;
    INT16U meter_idx,spot_idx,pos_idx;
    AREA_DIFFERENT_DATA area_info[10];
    INT8U idx_33,router_protocol;
    BOOLEAN is_find,set_event_flag;
    INT8U type;

    is_find = FALSE;
    set_event_flag = FALSE; //�����¼���ʶ
    spot_idx = 0;
    count = 0;
	mem_set(area_info,sizeof(area_info),0x00);
	
    fread_array(FILEID_AREA_DIFFERENT_DATA,PIM_AREA_DIFFERENT_COUNT,(INT8U*)&count,2);
    if (count > MAX_METER_COUNT) count = 0;

    for (idx=0;idx<count;idx++)
    {
        if ((idx % 10) == 0)
        {
            fread_array(FILEID_AREA_DIFFERENT_DATA,PIM_AREA_DIFFERENT_DATA+idx*sizeof(AREA_DIFFERENT_DATA),area_info[0].value,10*sizeof(AREA_DIFFERENT_DATA));
        }

        if (compare_string(area_info[idx%10].meter_no,frame+POS_GB645_METERNO,6) == 0)
        {
            is_find = TRUE;
            break;
        }
    }

    area_info[0].seq[0] = (idx+1);
    area_info[0].seq[1] = (idx+1)>>8;
    mem_cpy(area_info[0].meter_no,frame+POS_GB645_METERNO,6);

    #ifdef __PROVICE_SICHUAN__
    area_info[0].meter_protocol =  meter_protocol;
    area_info[0].abnormal_type = frame[POS_GB645_ITEM];
    #endif

    if(frame[POS_GB645_DLEN] == 8)/*�����8�ֽڣ����������68H	A0	����	A5	68H	9EH	08H	03H	AREA	PA0	����	PA5	CS	16H*/
    {
        mem_cpy(area_info[0].main_node,frame+POS_GB645_ITEM+2,6);
    }
    else
    {
        mem_cpy(area_info[0].main_node,frame+POS_GB645_ITEM+1,6);
    }

    for (idx_33=0;idx_33<6;idx_33++)
    {
        area_info[0].main_node[idx_33] = area_info[0].main_node[idx_33] - 0x33;
    }
    #ifdef __PROVICE_JIBEI__
    set_event_datetime(area_info[0].alter_datetime);
    #else
    set_DATAFMT_01((DATAFMT_01*)area_info[0].alter_datetime);
    #endif


    if (memory_fast_index_find_node_no(COMMPORT_PLC,area_info[0].meter_no,&meter_idx,&pos_idx,&router_protocol,NULL))
    {
        meter_idx &= FAST_IDX_MASK;
        if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
        {
            fread_array(meter_idx,PIM_METER_DOC+2,(INT8U*)&spot_idx,2);
            if ((spot_idx == 0) || (spot_idx > MAX_METER_COUNT)) spot_idx = 0;
        }
        #ifdef __PROVICE_JIBEI__
        //�ڱ��������ĵ����У��ж����ڵ��Ƿ�һ�£���һ�µ������¼�90
        if(compare_string(area_info[0].main_node,portContext_plc.router_work_info.ADDR_SRC,6) != 0)
        {
           area_info[0].area_type = 3;
           set_event_flag = TRUE;
        }
        else
        {
           area_info[0].area_type = 1;
        }
        #endif

    }
    else //���ڱ������������У���������
    {
        #ifdef __PROVICE_JIBEI__
        //�ڱ��������ĵ����У��ж����ڵ��Ƿ�һ�£���һ�µ������¼�90
        if(compare_string(area_info[0].main_node,portContext_plc.router_work_info.ADDR_SRC,6) == 0)
        {
           area_info[0].area_type = 2;
        }
        else
        {
            //�������ǲ��ڱ�������Ҳ���ڱ�̨������ʱ������
        }
        #endif
    }

    if (is_find)
    {
        fwrite_array(FILEID_AREA_DIFFERENT_DATA,PIM_AREA_DIFFERENT_DATA+idx*sizeof(AREA_DIFFERENT_DATA),area_info[0].value,sizeof(AREA_DIFFERENT_DATA));
    }
    else
    {
        fwrite_array(FILEID_AREA_DIFFERENT_DATA,PIM_AREA_DIFFERENT_DATA+count*sizeof(AREA_DIFFERENT_DATA),area_info[0].value,sizeof(AREA_DIFFERENT_DATA));
        count++;
        fwrite_array(FILEID_AREA_DIFFERENT_DATA,PIM_AREA_DIFFERENT_COUNT,(INT8U*)&count,2);
    }
    //�����¼�
    event_erc_63((INT8U*)&spot_idx,&(area_info[0]));
    #ifdef __PROVICE_SHAANXI__
    type = frame[POS_GB645_ITEM];
    event_erc_57((INT8U*)&spot_idx,&(area_info[0]),type);
    #endif
    //������Ҫ�����ERC90,ֻ�ԷǱ�̨���ı����ɣ�
    #ifdef __PROVICE_JIBEI__
    if(set_event_flag)
    {
        bplc_jibei_erc_90((INT8U*)&spot_idx,&(area_info[0]));
    }
    #endif
    #ifdef __PROVICE_NINGXIA__
    ningxia_hplc_event_erc_52((INT8U*)&spot_idx,&(area_info[0]));
    #endif
}

/*********************************************************************
 * ���ܣ�
 *     �洢�ɼ�������״̬
 * ������
 *     ���ݳ����Ĳɼ�������״̬��Ϣ�����ж�Ӧ�Ĵ���
 * ������
 *     READ_PARAMS *read_params
 *     INT8U* frame
 *     INT8U* frame_len
 * ����ֵ��
 *     INT8U
 **********************************************************************/
INT8U save_read_data_cjq_update_task(READ_PARAMS *read_params,INT8U* frame,INT8U *frame_len)
{
    INT16U len,pos,idx,meter_idx,byte_num;
    INT16U count=0;
    INT8U temp[20];
    CJQ_UPDATE_CTRL cjq_update_ctrl;
    CJQ_UPDATE_METER_CTRL cjq_update_meter_ctrl;
    BOOLEAN is_exception;

    mem_set(temp,20,0x00);
    is_exception = TRUE;
    meter_idx = bin2_int16u(read_params->meter_doc.meter_idx);
    
    if (check_frame_body_gb645(frame,*frame_len) == FALSE) return 0;
    
    //���ɼ�����ַ��һ�£��򲻴���
    if (compare_string(read_params->meter_doc.meter_no,frame+1,6) != 0) return 0;
    
    //���������C��ʾ�����Ǵ�վ������Ӧ��֡��Ҳ������
    if ((frame[POS_GB645_CTRL] & 0x80) == 0) return 0;

    fread_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
    fread_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
    if (cjq_update_ctrl.flag != 0xCC) return 0;
    
    pos = POS_GB645_ITEM;
   /*
    rs232_debug_info("\xAA",1);
    rs232_debug_info("\xAA",1);
    rs232_debug_info("\xA1",1);
    rs232_debug_info(&cjq_update_meter_ctrl.readflag,1);
    */
    if (((frame[POS_GB645_CTRL] & 0x1F) == 0x11) || ((frame[POS_GB645_CTRL] & 0x1F) == 0x12)) //�����ݺͶ���������
    {
        if (frame[POS_GB645_CTRL] & 0x40) //�쳣Ӧ��
        {
            //���������ж�����Ϊ���쳣ֹͣ���ͻ�ֹͣ�Ըòɼ����ĳ���
            is_exception = TRUE;
        }
        else  //����Ӧ��
        {
            //645���ݽ���-33����
            for(idx=0;idx<frame[POS_GB645_DLEN];idx++)
            {
                frame[POS_GB645_ITEM+idx] = frame[POS_GB645_ITEM+idx] - 0x33;
            }
            
           // rs232_debug_info("\xA2",1);
           // rs232_debug_info(frame,*frame_len);
            
            if (cjq_update_meter_ctrl.readflag & 0x01) //�����汾��Ϣ
            {
                if (bin2_int32u(frame+pos) == 0x04A00102)   //�����汾��Ϣ����Ӧ���
                {
                    pos += 4;
                    
                    //�汾��Ϣ��Ӧ���ݵĹ��ɣ���32�ֽ�
                    //�ɼ�������汾��6�ֽ�ASCII��ǰ2���ֽڣ�C1��ʾI�Ͳɼ�����C2��ʾII�Ͳɼ���
                    //���̴��룺2�ֽ�ASCII���ۼҵĳ��̴����õ���TC
                    //����汾����-�գ�1�ֽ�BCD
                    //����汾����-�£�1�ֽ�BCD
                    //����汾����-�꣺1�ֽ�BCD
                    //ģ��汾��Ϣ��6�ֽ�BIN
                    //���ã�15�ֽ�ASCII
                    //��֤�汾��Ϣ��ֻ��֤���̴��������汾���ڵ������꼴��
                    pos += 6;

                    if (compare_string(frame+pos,"TC",2) == 0)
                    {
                        pos += 2;
                        mem_cpy(cjq_update_meter_ctrl.meter_idx,read_params->meter_doc.meter_idx,2);
                        mem_cpy(cjq_update_meter_ctrl.rtu_no,read_params->meter_doc.rtu_no,6);
                        set_cosem_datetime_s(cjq_update_meter_ctrl.start_datetime,datetime);
                        cjq_update_meter_ctrl.patch_yijing_block[0] = 0;
                        cjq_update_meter_ctrl.patch_yijing_block[1] = 0;

                       // rs232_debug_info("\xA3",1);
                       // rs232_debug_info(cjq_update_ctrl.ver_date+9,3);
                       // rs232_debug_info(frame+pos,3);
                        
                        if (compare_string(cjq_update_ctrl.ver_date+9,frame+pos,3) != 0)  //�ɼ����汾�������ļ��汾��һ�£���δ�����ɹ�
                        {                            
                            mem_cpy(cjq_update_meter_ctrl.before_ver,frame+pos,50);
                            cjq_update_meter_ctrl.readflag &= ~0x01;
                            
                         //   rs232_debug_info("\xA4",1);
                         //   rs232_debug_info(&cjq_update_meter_ctrl.readflag,1);
                        }
                        else //�ɼ����汾�������ļ��汾һ�£������ɹ�
                        {
                            cjq_update_meter_ctrl.readflag = 0;
                            set_cosem_datetime_s(cjq_update_meter_ctrl.finish_datetime,datetime);
                            mem_cpy(cjq_update_meter_ctrl.after_ver,frame+pos,50);
                            cjq_update_meter_ctrl.cast_success_block[0] = cjq_update_ctrl.block_count[0];
                            cjq_update_meter_ctrl.cast_success_block[1] = cjq_update_ctrl.block_count[1];
                            cjq_update_meter_ctrl.patch_success_block[0] = 0;
                            cjq_update_meter_ctrl.patch_success_block[1] = 0;
                            cjq_update_meter_ctrl.result = 0xAA; //�㲥�ɹ�
                        }
                        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
                        is_exception = FALSE;

                        file_delete(FILEID_CJQ_METER);//����ɼ����汾��Ϣ�����³��µİ汾��Ϣ�����ܻ����һ������ε������
                    }
                }
            }
//                else if (cjq_update_meter_ctrl.readflag & 0x02) //�����ļ���Ϣ����֧�֣�
//                {
//                    if(compare_string(oad->value,"\xF0\x1\x2\x0",sizeof(OBJECT_ATTR_DESC)) == 0)
//                    {
//                        len = get_attribute_inbuffer(4,frame+pos,temp,40,TRUE,0xFF);
//                        if (compare_string(cjq_update_ctrl.ver_date,temp,15) != 0)
//                        {
//                            cjq_update_meter_ctrl.readflag &= ~0x02;
//                            cjq_update_meter_ctrl.readflag &= ~0x04;    //���������״̬��
//                            cjq_update_meter_ctrl.cast_success_block[0] = 0;
//                            cjq_update_meter_ctrl.cast_success_block[1] = 0;
//                            cjq_update_meter_ctrl.patch_success_block[0] = cjq_update_ctrl.block_count[0];
//                            cjq_update_meter_ctrl.patch_success_block[1] = cjq_update_ctrl.block_count[1];
//                        }
//                        else
//                        {
//                            cjq_update_meter_ctrl.readflag &= ~0x02;
//                            cjq_update_meter_ctrl.readflag &= ~0x08;    //��������
//                        }
//                        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
//                        is_exception = FALSE;
//                    }
//                }
            else if (cjq_update_meter_ctrl.readflag & 0x04) //���������״̬��
            {
                if (bin2_int32u(frame+pos) == 0x04A01102)   //���������״̬����Ӧ��飬��Ϊ�����Ǳ䳤��δ�Ӷ���Ӧ���ݳ��ȵ��ж�
                {
                    pos += 4;

                    len = bin2_int16u(frame+pos); //������
                    if(len == 0xFFFF) len = bin2_int16u(cjq_update_ctrl.block_count);
                    
                    pos += 2;
                    
                   // rs232_debug_info("\xA5",1);
                   // rs232_debug_info((INT8U *)&len,2);
                   // rs232_debug_info(cjq_update_ctrl.block_count,2);
                    
                    if ((len <= 128*8) && (bin2_int16u(cjq_update_ctrl.block_count) == len))
                    {
                        byte_num = len/8;
                        if ((len%8)&&(byte_num<128)) byte_num++;
                        mem_cpy(cjq_update_meter_ctrl.block,frame+pos,byte_num);
                        cjq_update_meter_ctrl.readflag &= ~0x04;
                        read_params->block_status = 0;
                        byte_num = 0;
                        for(idx=0;idx<len;idx++)
                        {
                            //ͳ��ʧ�ܵĿ���������1��ʾδ�ɹ�����
                            if (get_bit_value(cjq_update_meter_ctrl.block,128,idx)) byte_num++;
                        }
                        cjq_update_meter_ctrl.cast_success_block[0] = (len-byte_num);
                        cjq_update_meter_ctrl.cast_success_block[1] = (len-byte_num)>>8;
                        cjq_update_meter_ctrl.patch_success_block[0] = byte_num;
                        cjq_update_meter_ctrl.patch_success_block[1] = byte_num>>8;
                        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
                        is_exception = FALSE;
                    }
                }
            }
            else if (cjq_update_meter_ctrl.readflag & 0x20) //�����汾��Ϣ
            {
                if (bin2_int32u(frame+pos) == 0x04A00102)  //�����汾��Ϣ����Ӧ���
                {
                    pos +=4;
                    
                    //�汾��Ϣ��Ӧ���ݵĹ��ɣ���32�ֽ�
                    //�ɼ�������汾��6�ֽ�ASCII��ǰ2���ֽڣ�C1��ʾI�Ͳɼ�����C2��ʾII�Ͳɼ���
                    //���̴��룺2�ֽ�ASCII���ۼҵĳ��̴����õ���TC
                    //����汾����-�գ�1�ֽ�BCD
                    //����汾����-�£�1�ֽ�BCD
                    //����汾����-�꣺1�ֽ�BCD
                    //ģ��汾��Ϣ��6�ֽ�BIN
                    //���ã�15�ֽ�ASCII
                    //��֤�汾��Ϣ��ֻ��֤���̴��������汾���ڵ������꼴��
                    pos += 6;

                    if (compare_string(frame+pos,"TC",2) == 0)
                    {
                        pos += 2;
                        
                      //  rs232_debug_info("\xA6",1);
                      //  rs232_debug_info(cjq_update_ctrl.ver_date+9,3);
                      //  rs232_debug_info(frame+pos,3);
                        
                        if (compare_string(cjq_update_ctrl.ver_date+9,frame+pos,3) == 0)  //�ɼ����汾�������ļ��汾��Ϣһ��
                        {
                            cjq_update_meter_ctrl.result = 0x55; //�����ɹ�
                        }
                        else
                        {
                            if (cjq_update_meter_ctrl.result == 0xFF) //������0xFFʱ����ǰ��Ĳ�����д��ʧ��ԭ��
                            {
                                cjq_update_meter_ctrl.result = 0x00; //ʧ��
                            }
                        }
                        set_cosem_datetime_s(cjq_update_meter_ctrl.finish_datetime,datetime);
                        mem_cpy(cjq_update_meter_ctrl.after_ver,frame+pos,50);
                        cjq_update_meter_ctrl.readflag = 0;
                        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
                        is_exception = FALSE;
                    }
                }
            }
        }
    }
    else if ((frame[POS_GB645_CTRL] & 0x1F)  == 0x14)  //�ɼ����ļ�����
    {
        if (frame[POS_GB645_CTRL] & 0x40) //�쳣Ӧ��
        {
            //���������ж�����Ϊ���쳣���ͻ�ֹͣ�Ըòɼ����ĳ���
            is_exception = TRUE;
        }
        else  //����Ӧ��
        {
            //645���ݽ���-33����
            for(idx=0;idx<frame[POS_GB645_DLEN];idx++)
            {
                frame[POS_GB645_ITEM+idx] = frame[POS_GB645_ITEM+idx] - 0x33;
            }
//            if (cjq_update_meter_ctrl.readflag & 0x08) //��������
//            {
//                if (compare_string(oad->value,"\xF0\x1\x7\x0",sizeof(OBJECT_ATTR_DESC)) == 0)
//                {
//                    if (frame[pos++] == 0) //dar
//                    {
//                        cjq_update_meter_ctrl.readflag &= ~0x08;    //��������
//                        mem_set(cjq_update_meter_ctrl.block,128,0x00);
//                    }
//                    else
//                    {
//                        cjq_update_meter_ctrl.readflag = 0;  //���������־
//
//                        cjq_update_meter_ctrl.result = 0x01; //��������ʧ��
//                        set_cosem_datetime_s(cjq_update_meter_ctrl.finish_datetime,datetime);
//                        mem_cpy(cjq_update_meter_ctrl.after_ver,cjq_update_meter_ctrl.before_ver,50);
//                    }
//                    fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
//                    is_exception = FALSE;
//                }
//            }
//            else 
            if (cjq_update_meter_ctrl.readflag & 0x02) //���ſ��·���£���readflag��Ӧ��������ļ���������
            {
                //����·�������ļ��������˵���ڼ����Ĺ����У��Ѿ�ȷ����Ҫ�·��������˿���ֱ�ӽ��ɼ�����ַ��ӵ��洢�У�ͬʱ�����������
                //���ִ���˶Բɼ����Ĳ�����˵���������Ѿ���֤���ɼ�����ַ����Ч�ԣ���˿���ֱ�ӽ��ɼ�����ַд��洢�Ĳɼ�����ַ������
                fread_array(FILEID_CJQ_METER_TEMP,PIM_CJQ_UPDATE_COUNT,(INT8U *)&count,2);
                if(count > MAX_METER_COUNT) count = 0;
                fwrite_array(FILEID_CJQ_METER_TEMP,PIM_CJQ_UPDATE_METER_INFO+count*6,read_params->meter_doc.rtu_no,6);
                count++;
                fwrite_array(FILEID_CJQ_METER_TEMP,PIM_CJQ_UPDATE_COUNT,(INT8U *)&count,2);
                //���������Ӧ֡�������ݱ�ʶ����Ӧȷ��֡�ˣ�ֱ����Ϊִ�гɹ�
                cjq_update_meter_ctrl.readflag &= ~0x02;
                fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
                is_exception = FALSE;
            }
            else if (cjq_update_meter_ctrl.readflag & 0x10) //�ļ����䲹��
            {
                //���������Ӧ֡�������ݱ�ʶ
                //if (bin2_int32u(frame+pos) == 0x04A01101)   //�ļ��������Ӧ���
                {               
                    byte_num = bin2_int16u(cjq_update_meter_ctrl.patch_yijing_block);
                    byte_num ++;
                    cjq_update_meter_ctrl.patch_yijing_block[0] = byte_num;
                    cjq_update_meter_ctrl.patch_yijing_block[1] = byte_num>>8;

                  //  rs232_debug_info("\xA7",1);
                  //  rs232_debug_info((INT8U *)&byte_num,1);
                  //  rs232_debug_info(frame+pos,2);
                    
                    //�����ɹ��������Ӧ���δ�ɹ������־
                    clr_bit_value(cjq_update_meter_ctrl.block,128,bin2_int16u(frame+pos));
                    byte_num = 0;
                    for(idx=0;idx<bin2_int16u(cjq_update_ctrl.block_count);idx++)
                    {
                        if (get_bit_value(cjq_update_meter_ctrl.block,128,idx)) byte_num++;
                    }
                    
                  //  rs232_debug_info("\xA8",1);
                  //  rs232_debug_info((INT8U *)&byte_num,1);
                  //  rs232_debug_info(cjq_update_meter_ctrl.block,128);
                    
                    if (byte_num == 0)
                    {
                        cjq_update_meter_ctrl.readflag &= ~0x10;
                    }
                }
                fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
                is_exception = FALSE;
            }
        }
    }

    if (is_exception)
    {
        cjq_update_meter_ctrl.readflag = 0;
        cjq_update_meter_ctrl.result = 0xEE;      //�쳣ֹͣ
        set_cosem_datetime_s(cjq_update_meter_ctrl.finish_datetime,datetime);
        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_METER_FLAG+meter_idx*sizeof(CJQ_UPDATE_METER_CTRL),cjq_update_meter_ctrl.value,sizeof(CJQ_UPDATE_METER_CTRL));
    }
	
	
    return 0;
}
INT8U check_meter_idx_same_or_not(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len,INT8U* buffer)
{
    INT16U meter_idx_in_fast_index,pos_idx,pos;
    INT8U router_protocol,doc_len,idx;

    if(memory_fast_index_find_node_no(COMMPORT_PLC,frame+1,&meter_idx_in_fast_index,&pos_idx,&router_protocol,NULL))
    {
        meter_idx_in_fast_index &= FAST_IDX_MASK;
        if(bin2_int16u(read_params->meter_doc.meter_idx) != meter_idx_in_fast_index) /*����յ���645��Ŷ�Ӧ�Ĳ������ͨ����¼�Ĳ�һ�£���Ҫ�洢*/
        {
            doc_len = 12;/*ֻҪ����š�������š��˿ڡ�Э��ͱ��*/

            pos = 0;
            mem_cpy(buffer+pos,frame,*frame_len);    /*�����쳣���Ĵ�һ�� */
            pos += *frame_len;

            if(pos >150)    /*bufferֻ��300����ֹframe_len̫�������������������ֳ��������������*/
            {
                pos = 150;
            }

            buffer[pos++] = 0xAA;/*��¼һ����ʼ��ʶ*/
            buffer[pos++] = meter_idx_in_fast_index;
            buffer[pos++] = meter_idx_in_fast_index >>8; /*���ݱ�ŷ���Ĳ������*/
            buffer[pos++] = portContext_plc.router_work_info.phase; /*��ǰ���ڴ����ͨ�����*/
            mem_cpy(buffer+pos,read_params->meter_doc.value,doc_len);/*��ǰͨ���ĵ�����Ϣ*/
            pos +=  doc_len;

            for(idx=0;idx<7;idx++)   /*�����쳣ʱ������ͨ���ĵ����Ϣ,���������Ƿ����ظ���ռ��72�ֽ�*/
            {
                if(idx == portContext_plc.router_work_info.phase)
                {
                    continue;
                }
                mem_cpy(buffer+pos,portContext_plc.router_phase_work_info[idx].read_params.meter_doc.value,doc_len);
                pos += doc_len;
            }

            record_log_code(LOG_SYS_METER_IDX_ERROR,buffer,pos,LOG_ALL);/*��¼��־��ͨ���쳣����*/
            return FALSE;
        }
        else
        {
            return TRUE;
        }

    }
    else
    {
        return FALSE;  /*�ز��ڵĿ��������Ҳ�����������ܴ洢 */
    }

    return TRUE;
}

/*********************************************************************
 * ���ܣ�
 *     �洢���ͣ�ϵ��ϱ���Ϣ
 * ������
 *     FILEID_METER_POWEROFF_DATA   ���ͣ��洢�ļ�
 *     FILEID_METER_POWEROFF_DATA   ����ϵ�洢�ļ�
 * ������
 *     READ_PARAMS *read_params
 *     INT8U* frame
 *     INT8U* frame_len
 * ����ֵ��
 *     INT8U
 **********************************************************************/
void save_meter_power_off_event(INT8U *frame,INT8U data_len,INT8U flag)
{
    INT8U idx,count,idx1,save_idx=0;
    INT16U ex_count,in_count,file_idx;
    INT8U *count_pos,*begin_time,*end_time,*meter_map;
    INT8U meter_no[6];
    BOOLEAN is_repeat,is_update;
    METER_POWEROFF_DATA meter_poweroff_data;
    INT32U num;

    INT16U meter_idx,save_pos;
    INT8U router_protocol,user_type;

    count=data_len/6;
    if(count == 0)  return;

    //file_delete(FILEID_METER_POWEROFF_DATA);

    if(flag == 0x55)
    {
        file_idx = FILEID_METER_POWEROFF_DATA;  //���ͣ��洢�ļ�
    }
    else if(flag == 0xAA)
    {
        file_idx = FILEID_METER_POWEROFF_DATA+1; //����ϵ�洢�ļ�
    }
    else    return;

    tpos_mutexPend_unique(&SIGNAL_TEMP_BUFFER);
    fread_array(file_idx,BASE_INFO_HEADER,g_temp_buffer,300);
    count_pos = g_temp_buffer+4;
    begin_time = g_temp_buffer+8;
    end_time = g_temp_buffer+14;
    meter_map = g_temp_buffer+20;

    if(compare_string(g_temp_buffer,"\xAA\xAA\xAA\xAA",4)==0)
    {
        in_count=bin2_int16u(count_pos);
        if(in_count>MAX_METER_COUNT)    in_count = 0;
        ex_count=bin2_int16u(count_pos+2);
        if(ex_count>MAX_EX_METER_COUNT) ex_count = 0;
        mem_cpy(end_time,datetime+SECOND,6);
        tpos_enterCriticalSection();
        if(flag == 0x55)    g_meter_power_flag[0] = 0x55;    //ͣ��
        if(flag == 0xAA)    g_meter_power_flag[1] = 0x55;    //�ϵ�
        tpos_leaveCriticalSection();
    }
    else
    {
        mem_set(g_temp_buffer,4,0xAA);
        in_count = 0;
        ex_count = 0;
        mem_set(count_pos,4,0x00);
        mem_cpy(begin_time,datetime+SECOND,6);
        mem_cpy(end_time,begin_time,6);
        mem_set(meter_map,256,0x00);
        tpos_enterCriticalSection();
        if(flag == 0x55)    g_meter_power_flag[0] = 0x55;        //ͣ��
        if(flag == 0xAA)    g_meter_power_flag[1] = 0x55;        //�ϵ�
        tpos_leaveCriticalSection();
    }
    fwrite_array(file_idx,BASE_INFO_HEADER,g_temp_buffer,300);
    for(idx=0;idx<count;idx++)
    {
        mem_cpy(meter_no,frame+idx*6,6);
        if(memory_fast_index_find_node_no(COMMPORT_PLC,meter_no,&meter_idx,&save_pos,&router_protocol,&user_type))
        {
            meter_idx &= FAST_IDX_MASK;
            is_update=FALSE;
            if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
            {
                if(get_bit_value(meter_map,256,meter_idx))
                {
                    fread_array(file_idx,IN_METER_EVENT_DATA+(meter_idx-1)*sizeof(METER_POWEROFF_DATA),meter_poweroff_data.value,sizeof(METER_POWEROFF_DATA));
                    num=diff_sec_between_dt(meter_poweroff_data.occur_datetime,end_time);
                    if(num>300)
                    {
                        clr_bit_value(meter_map,256,meter_idx);
                        is_update=TRUE;
                    }
                }
                
                if(!get_bit_value(meter_map,256,meter_idx))
                {
                    set_bit_value(meter_map,256,meter_idx);
                    meter_poweroff_data.is_report=0;
                    mem_cpy(meter_poweroff_data.meter_no,meter_no,6);
                    mem_cpy(meter_poweroff_data.occur_datetime,end_time,6);
                    if(!is_update)
                    {
                        in_count++;
                        (void)int16u2_bin(in_count,count_pos);
                        fwrite_array(file_idx,BASE_INFO_HEADER,g_temp_buffer,300);
                    }
                    fwrite_array(file_idx,IN_METER_EVENT_DATA+(meter_idx-1)*sizeof(METER_POWEROFF_DATA),meter_poweroff_data.value,sizeof(METER_POWEROFF_DATA));
                }
            }
        }
        else
        {
            is_repeat = FALSE;
            is_update = FALSE;
            for(idx1=0;idx1<ex_count;idx1++)
            {
                fread_array(file_idx,EX_METER_EVENT_DATA+idx1*sizeof(METER_POWEROFF_DATA),meter_poweroff_data.value,sizeof(METER_POWEROFF_DATA));
                if(compare_string(meter_no,meter_poweroff_data.meter_no,6)==0)
                {
                    is_repeat =TRUE;
                    break;
                }
            }
            if(is_repeat)
            {
                num=diff_sec_between_dt(meter_poweroff_data.occur_datetime,end_time);
                if(num>300)
                {
                    is_repeat=FALSE;
                    is_update=TRUE;
                    save_idx=idx1;
                }
            }
            if(!is_repeat)
            {
                meter_poweroff_data.is_report=0;
                mem_cpy(meter_poweroff_data.meter_no,meter_no,6);
                mem_cpy(meter_poweroff_data.occur_datetime,end_time,6);
                if(!is_update)
                {
                    save_idx=ex_count;
                    ex_count++;
                    (void)int16u2_bin(ex_count,count_pos+2);
                    fwrite_array(file_idx,BASE_INFO_HEADER,g_temp_buffer,300);
                }
                fwrite_array(file_idx,EX_METER_EVENT_DATA+save_idx*sizeof(METER_POWEROFF_DATA),meter_poweroff_data.value,sizeof(METER_POWEROFF_DATA));
            }
        }
    }
    tpos_mutexFree(&SIGNAL_TEMP_BUFFER);
}


/* ================================================================
 * ��չ�¼��ϱ�  04001507
 *
 *
 * ================================================================
 */
void plc_router_save_report_meter_ext_event_data(INT16U meter_idx,INT8U *meter_no,INT32U item,INT8U *data,INT8U datalen)
{
	INT16U 	offset;
	INT16U	event_len = 0; //���ȱ�Ϊ2�ֽ� ����������� ����ʡ��Ҳ�ĳ�2�ֽ���
    INT16U  idx = 0;
	INT8U	idx1,len;
	INT8U 	tmp[256]={0};
	report_ctrl reportCtrl;
	EVENT_READ_CTRL read_ctrl;
	METER_DOCUMENT meter_doc;
	
	fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
	fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
	if( (read_ctrl.ctrl_flag == 0x00) || (read_ctrl.ctrl_flag == 0xFF) )
	{
		return ;
	}
    if(item == 0x04001507)
    {
        if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY))
        {
			read_ctrl.ctrl_flag = 0x00;
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
        }
        else if(check_is_all_ch(data,(datalen > 7) ? 7 : datalen,0x00))
        {
			read_ctrl.ctrl_flag = 0x00;
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
        }
        else
        {
            datalen = (datalen > (sizeof(DATA_04001507) - 5U)) ? (sizeof(DATA_04001507) - 5U) : datalen;
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL)+1,(INT8U*)&item,4);
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL)+1+4,data,datalen);
            tmp[0] = datalen + 4U;
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+sizeof(EVENT_READ_CTRL),tmp,1);

            offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001507);
            idx = 0;
            #ifdef __EVENT_04001501_BEFORE__
            tmp[idx++] = 14 + datalen;      //�ܳ�
            tmp[idx++] = ((INT16U)(14 + datalen))>>8; // 2�ֽڳ���
            tmp[idx++] = meter_doc.baud_port.port;//COMMPORT_PLC;
            mem_cpy(tmp+idx,meter_no,6);
            idx += 6;
            tmp[idx++] = 6;  //ת�������ʶ
            tmp[idx++] = 1;  //ת��ֱ�ӳ��������ݱ�ʶ����n
            tmp[idx++] = 4 + datalen;
            mem_cpy(tmp+idx,(INT8U*)&item,4);
            idx += 4;
            mem_cpy(tmp+idx,data,datalen);
            idx += datalen;
            #else
            tmp[idx++] = 9;      //�ܳ�
            tmp[idx++] = 0;  //�ܳ��� 2�ֽ� (port 1+ 6 addr+ result 1+ count 1 = 9)
            tmp[idx++] = meter_doc.baud_port.port;//COMMPORT_PLC;
            mem_cpy(tmp+idx,meter_no,6);
            idx += 6;
            tmp[idx++] = 6;  //ת�������ʶ
            tmp[idx++] = 0;  //ת��ֱ�ӳ��������ݱ�ʶ����n
            #endif
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,tmp,idx);

            read_ctrl.ctrl_flag &= ~0x02;
            len = (datalen > 7) ? 7 : datalen;
            mem_set(read_ctrl.meter_state_word,7,0x00);
			
			#ifdef __PROVICE_HUNAN__		
			if(meter_doc.baud_port.port == READPORT_PLC)//�ز��ű����¼�,�����˿�д�����00
			#endif
			{
            	mem_cpy(read_ctrl.meter_state_word,data,len);
			}
            read_ctrl.event_count.value = 0;
            read_ctrl.event_mask[0] = 0;
			read_ctrl.rec_event_cnt = 0;//��������
			
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
        }
    }
    else if (item == 0x04001508)
    {
        if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY))
        {
			read_ctrl.ctrl_flag = 0x00;
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,1);
        }
        else
        {
            #ifdef __PROVICE_HUNAN__
			read_ctrl.ctrl_flag &= ~0x0C;
			#else
			read_ctrl.ctrl_flag &= ~0x04;
			#endif
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,1);
        }
    }
    else
    {
        if(check_is_all_ch(data,datalen,REC_DATA_IS_DENY) == FALSE)
        {
            offset = sizeof(EVENT_READ_CTRL)+sizeof(DATA_04001507);
            fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,(INT8U *)&event_len,2);
            if ((event_len + 1U + 4U + datalen) >= 850U)
            {
                //ʹ����ʱ���������粻���ʣ���������
                tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
                fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset+2,g_temp_buffer,event_len);
				reportCtrl.read_tmp_and_save = 1;//д���ϱ��ļ���ȥ
                save_meter_event_record(0,NULL,g_temp_buffer,event_len,reportCtrl);
                tpos_mutexFree(&SIGNAL_TEMP_BUFFER);

                idx = 0;
                tmp[idx++] = 14 + datalen;      //�ܳ�
                tmp[idx++] = ((INT16U)(14 + datalen))>>8; // 2�ֽڳ���
                tmp[idx++] = meter_doc.baud_port.port;//COMMPORT_PLC;
                mem_cpy(tmp+idx,meter_no,6);
                idx += 6;
                tmp[idx++] = 6;  //ת�������ʶ
                tmp[idx++] = 1;  //ת��ֱ�ӳ��������ݱ�ʶ����n
                tmp[idx++] = 4U + datalen;
                mem_cpy(tmp+idx,(INT8U*)&item,4);
                idx += 4U;
                mem_cpy(tmp+idx,data,datalen);
                idx += datalen;
                fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,tmp,idx);
            }
            else
            {
                idx = 0;
                tmp[idx++] = 4U + datalen;
                mem_cpy(tmp+idx,(INT8U*)&item,4);
                idx += 4U;
                mem_cpy(tmp+idx,data,datalen);
                idx += datalen;
                fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset+2+event_len,tmp,idx);
                fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,tmp,11);
                event_len = bin2_int16u(tmp);
				event_len += 5U + datalen;
                tmp[0] = (INT8U )(event_len&0x00FF);//����
				tmp[1] = (INT8U )(event_len>>8);
                tmp[10]++;
                fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE+offset,tmp,11);
            }
        }

        for(idx=0;idx<56;idx++)
        {
            if(get_bit_value(read_ctrl.meter_state_word,7,idx))
            {
            	if(read_ctrl.event_count.mask)
                {
                    read_ctrl.event_count.mask = 0;
                    break;
                }

                for(idx1=0;idx1<8;idx1++)
                {
                    if(get_bit_value(read_ctrl.event_mask,1,idx1))
                    {
                        clr_bit_value(read_ctrl.event_mask,1,idx1);
                        break;
                    }
                }
                break;
            }
        }

        if(check_is_all_ch(read_ctrl.event_mask,1,0x00))
        {
            read_ctrl.ctrl_flag &= ~0x08;
            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,1);
        }
        fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,read_ctrl.value,sizeof(EVENT_READ_CTRL));
    }
}
#ifdef __READ_OOP_METER__
INT8U save_read_data_gb_oop(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT32U item, phy_u32=0xFFFFFFFF;
    INT16U pos = 0, data_pos=0;
    INT16U data_len = 0;
    READ_WRITE_DATA phy;
    INT8U meter_no[6];
    INT8U idx,idx_1;
    INT8U oad_cnt;
    #if ((defined __PLC_REC_VOLTMETER1__) || (defined __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__))
    INT8U i;
	#endif
    INT8U buffer[300]={0};
    tagPROTOCOL_LIB pLib;
            
    *frame_len = decode_readmeter_frame(frame, *frame_len);
    if (check_frame_body_gb_oop(frame,*frame_len) == FALSE) return 0;
    if (get_oop_frame_meter_no(frame,meter_no) == FALSE) return 0;
    if (compare_string(read_params->meter_doc.meter_no,meter_no,6) != 0) return 0;
    #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
    if (read_params->read_type == READ_TYPE_BATCH_TRANSPARENT_METER_CYCLE_TASK)
    {
        save_batch_transparent_meter_cycle_task(read_params,frame);
        return 0;
    }
    #endif
    pos = get_frame_header_length_gb_oop(frame);
    if (frame[pos++] != 0x85) return 0;
    if (frame[pos] == 0x02) //GetRequestNormalList
    {
        pos++;
        pos++;//PIID
        oad_cnt = frame[pos++];//count
        for(idx=0;idx<oad_cnt;idx++)
        {
        item = frame[pos++]<<24;
        item += frame[pos++]<<16;
        item += frame[pos++]<<8;
        item += frame[pos++];

            for(idx_1=0; idx_1<read_params->oad_cnt; idx_1++)//���ǻظ�oad˳��Ͷ�ȡ��һ�������
            {
                if(item == cosem_bin2_int32u(read_params->oad+idx_1*4))
                {
                    phy_u32 = bin2_int32u(read_params->phy_bak+idx_1*4);
                    break;
                }
            }
            if(idx_1 == read_params->oad_cnt)
            {
                if (frame[pos++] != 0x01) 
                {
                    pos++;//CHOICE DAR
                    continue;
                }
                pos+=get_object_attribute_item_len(frame+pos, frame[pos]);
                continue;
            }

            int32u2_bin(phy_u32, read_params->phy);
            (void)get_data_item(phy_u32, GB_OOP, &pLib);
            
            if (frame[pos++] != 0x01) 
            {
                pos++;//CHOICE DAR
                data_len = pLib.len;
                mem_set(buffer, data_len, 0xEE);
                save_oop_cur_hold_data(read_params, buffer, data_len);
                if(RQ_XQ == phy_u32)
                {
                    (void)get_data_item(SJ, GB_OOP, &pLib);
                    int32u2_bin(SJ, read_params->phy);
                    data_len = pLib.len;
                    mem_set(buffer, data_len, 0xEE);
                    save_oop_cur_hold_data(read_params, buffer, data_len);
                }
                continue;
            }
            
            if(RQ_XQ == phy_u32)
        {
                (void)get_data_item(SJ, GB_OOP, &pLib);
                int32u2_bin(SJ, read_params->phy);
            data_len = transmit_oop_format_to_07_data(&pLib,frame+pos,buffer);
                if(0 == data_len) 
                {
                    data_len = pLib.len;
                    mem_set(buffer, data_len, 0xEE);
                }
                save_oop_cur_hold_data(read_params, buffer, data_len);
                
                (void)get_data_item(RQ_XQ, GB_OOP, &pLib);
                int32u2_bin(RQ_XQ, read_params->phy);
            }
            
            data_len = transmit_oop_format_to_07_data(&pLib, frame+pos, buffer);
            if(0 == data_len) 
            {
                data_len = pLib.len;
                mem_set(buffer, data_len, 0xEE);
            }
            save_oop_cur_hold_data(read_params, buffer, data_len);
            pos+=get_object_attribute_item_len(frame+pos, frame[pos]);
        }
        return 1;
    }
    else if (frame[pos] == 0x03) //GetRequestRecord
    {
        if(read_params->read_type == READ_TYPE_LAST_CURVE_CYCLE_DAY)
        {
            pos++;
            pos++;//PIID
            pos += 4; //50 02 02 00
            if (frame[pos++] != 0x01) return 0;//Ŀǰֻ��һ��oad
            pos++; //0ad
            item = frame[pos++]<<24;
            item += frame[pos++]<<16;
            item += frame[pos++]<<8;
            item += frame[pos++];

            item = get_curve_normal_phy(item);
            if(get_data_item(item,GB_OOP,&pLib))
            {
                data_len = 1;
                if (frame[pos++] != 0x01) data_len = 0; //û������
                if (frame[pos++] == 0x00) data_len = 0; //0������

                data_len = transmit_oop_format_to_07_data(&pLib,frame+pos,buffer);

                get_phy_form_list_cruve(item,&phy);

                if(data_len >0)
                {
                     mem_cpy(frame,buffer,data_len);
                }
                else
                {

                    mem_set(frame,phy.block_len,0xEE);
                    data_len = phy.block_len;

                }

                writedata_curve(read_params,&phy,read_params->patch_load_time,frame,data_len,buffer,0);
            }
            else
            {
              //�������idx����,ע�⣬�������ϸ�Ķ�Ӧ��ϵ���߲����˴���
            }
        }
        else  //READ_TYPE_CURVE READ_TYPE_CYCLE_DAY READ_TYPE_CYCLE_MONTH
        {
        pos++;
        pos++;//PIID
            pos += 4; //����OAD
            oad_cnt = frame[pos++];
            data_pos = pos+oad_cnt*5;
            if(frame[data_pos] == 0x00) //������Ϣ     [0]
            {
                return 0;
            }
            data_pos += 2;
            for(idx=0; idx<oad_cnt; idx++)
            {
                pos++; //OAD
        item = frame[pos++]<<24;
        item += frame[pos++]<<16;
        item += frame[pos++]<<8;
        item += frame[pos++];

                if(item != cosem_bin2_int32u(read_params->oad+idx*4))
                {
                    if (frame[data_pos] == 0x00) 
                    {
                        data_pos++;
                    }
                    else
                    {
                        data_pos+=get_object_attribute_item_len(frame+data_pos, frame[data_pos]);
                    }
                    continue;
                }
                phy_u32 = bin2_int32u(read_params->phy_bak+idx*4);
                int32u2_bin(phy_u32, read_params->phy);
                (void)get_data_item(phy_u32, GB_OOP, &pLib);
                if (frame[data_pos] == DT_NULL) 
                {
                    data_pos++;
                    data_len = pLib.len;
                    mem_set(buffer, data_len, 0xEE);
                    save_oop_cur_hold_data(read_params, buffer, data_len);
                    continue;
                }
            
                data_len = transmit_oop_format_to_07_data(&pLib, frame+data_pos, buffer);
                if(0 == data_len) 
        {
                    data_len = pLib.len;
                    mem_set(buffer, data_len, 0xEE);
                }
                save_oop_cur_hold_data(read_params, buffer, data_len);
                data_pos+=get_object_attribute_item_len(frame+data_pos, frame[data_pos]);
        }
            return 1;
        }
    }
    return 0;
    }

void save_oop_cur_hold_data(READ_PARAMS *read_params, INT8U *data, INT16U data_len)
{
    INT8U idx;
    INT8U curve_save_flag=0;
    INT8U buffer[256]={0};
    INT8U buffer_len=0;
    INT8U block_count = 0;
    INT16U block_begin_idx = 0;
    READ_WRITE_DATA phy;
    READ_PARAMS read_params_tmp;

    switch(read_params->read_type)
    {
        case READ_TYPE_CURVE:
            save_curve_data(read_params, data, data_len, buffer, curve_save_flag);
            #ifdef __PROVICE_JIANGXI__
            save_cur_date(read_params, data, data_len, buffer);
            #endif
            break;
        case READ_TYPE_CYCLE_DAY:
            save_cycle_day(read_params,data,data_len,buffer);
            if (read_params->control.month_hold_mode == 0)
            {
                idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
                if(0xFF != idx)// �ж�idx  0xFF ˵����Ч����������δ��� ??�ݶ���ȥ�洢��
                {
                    if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,&buffer_len) == FALSE)
                    {
                        save_cycle_month(read_params,data,data_len,buffer);
                    }
                }
            }
            break;
        case READ_TYPE_CYCLE_MONTH:
            save_cycle_month(read_params,data,data_len,buffer);
            break;
        #ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
        case READ_TYPE_CURVE_HUNAN:
            save_curve_data_hunan(read_params,data,data_len,buffer);
            save_cur_date(read_params,data,data_len,buffer);
            break;
        #endif
        case READ_TYPE_CUR_DATA:
            save_cur_date(read_params,data,data_len,buffer);
            //485��������
            if((read_params->meter_doc.baud_port.port != COMMPORT_PLC) && (read_params->meter_doc.baud_port.port != COMMPORT_PLC_REC))
            {
                curve_save_flag = 0x55;
                save_curve_data(read_params,data,data_len,buffer,curve_save_flag);
            }

            if((read_params->meter_doc.baud_port.port == COMMPORT_485_CY) || (read_params->meter_doc.baud_port.port == COMMPORT_485_REC) || (read_params->meter_doc.baud_port.port == COMMPORT_485_CAS))
            {
                mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                trans_phy_cur_2_cycle_day(read_params_tmp.phy);
                get_yesterday(read_params_tmp.day_hold_td);//��ʱ����̨����� 
                idx = get_phy_form_list_cycle_day(bin2_int32u(read_params_tmp.phy),&phy); 
                if(check_validity_curdata_to_holddata(read_params_tmp.meter_doc.baud_port.port,idx,READ_TYPE_CYCLE_DAY))
                {
                    if(readdata_cycle_day(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.day_hold_td,buffer,buffer+5,&buffer_len,NULL) == FALSE)
                    {
                        save_cycle_day(&read_params_tmp, data, data_len, buffer);
                    }
                }
                //�¶��ỹδ����
                if(check_is_all_ch(read_params->month_hold_td,2,0x00)) get_former_month(read_params_tmp.month_hold_td);

                idx = get_phy_form_list_cycle_month(bin2_int32u(read_params_tmp.phy),&phy);
                if(check_validity_curdata_to_holddata(read_params_tmp.meter_doc.baud_port.port,idx,READ_TYPE_CYCLE_MONTH))
                {                            
                    if(readdata_cycle_month(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.month_hold_td,buffer,buffer+5,&buffer_len) == FALSE)
                    {                                          
                        save_cycle_month(&read_params_tmp, data, data_len, buffer);
                    }
                }

                //�����ն�������
                //#ifndef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
                // if(read_params->hold_flag.rec_day)
                {
                    if(TRUE == is_recday_rs485_port(read_params->meter_doc.baud_port.port))
                    {
                        if(0xFF != get_phy_form_list_recday_data(bin2_int32u(read_params->phy),&phy))
                        {
                            if(readdata_recday_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,datetime+DAY,buffer,buffer+5,&buffer_len) == FALSE)
                            {
                                save_recday_data(read_params, data, data_len, buffer);
                            }
                        }
                    }
                }
                //#endif

                if(0xFF != get_phy_from_list_day_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                {
                    readdata_day_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,&buffer_len,TRUE);
                    if(0 != compare_string(buffer+2,read_params->init_data_rec_time+2,3))
                    {
                        save_day_init_data(read_params, data, data_len, buffer);
                    }
                }
                if(0xFF != get_phy_from_list_month_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                {
                    readdata_month_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,&buffer_len,TRUE);
                    if(0 != compare_string(buffer+3,read_params->init_data_rec_time+3,2))
                    {
                        save_month_init_data(read_params, data, data_len, buffer);
                    }
                }
            }
            break;
        default:
            break;
    }
}
#endif
INT8U save_read_data_dlms(READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT32U phy_u32;
    READ_WRITE_DATA phy;
    READ_WRITE_DATA *phy_ptr;
    tagPROTOCOL_LIB pLib_tmp;
    READ_PARAMS read_params_tmp;
    INT16U len,left_len,block_begin_idx;
    INT8U* td;
    INT8U idx,block_count;
    INT8U midu;
    INT8U buffer[300],tmp_buf[50];

    mem_cpy(buffer,frame,* frame_len);
    midu = * frame_len;
    //1.�ٴ���֤���ĸ�ʽ
    if (check_frame_body_dlms(buffer,*frame_len) == FALSE) return 0;

    //2.��֤���
    if(dlms_frame_meter_no_is_ok(buffer,*frame_len,read_params->meter_doc.meter_no)==FALSE)return 0;

    //�ز���Ҫ��֤�ǲ��ǵ�¼���ǲ���Э�����ӳɹ�
             if(read_params->read_ctrl_state==0)
            {
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"dlms SNRM process %d",read_params->read_ctrl_step);
                debug_println_ext(info);
                #endif
                if(read_params->read_ctrl_step==0)
                {
                    if(dlms_frame_snrm_is_ok(buffer,*frame_len))
                    {
                        read_params->read_ctrl_step++;
                        #ifdef __SOFT_SIMULATOR__
                        snprintf(info,100,"dlms SNRM process ok %d",read_params->read_ctrl_step);
                        debug_println_ext(info);
                        #endif
                    }
                    else
                    {
                        read_params->read_ctrl_step=0;//ʧ�ܺ��ͷ��ʼ
                        read_params->read_ctrl_state=0;
                    }
                }
                else  if(read_params->read_ctrl_step==1)
                {
                    if(dlms_frame_aare_is_ok(buffer,*frame_len))
                    {
                        read_params->read_ctrl_step ++ ;
                        read_params->read_ctrl_state=1;

                        #ifdef __SOFT_SIMULATOR__
                        snprintf(info,100,"dlms aarq process %d",read_params->read_ctrl_step);
                        debug_println_ext(info);
                        #endif
                        if(dlms_read_object_list.count==0)//�����û�л�ȡsnӳ�������Ҫ��ȡ
                        {


                        }
                        else
                        {
                            if(read_params->read_ctrl_step>=2)
                            {
                                read_params->read_ctrl_state=1;
                            }
                        }
                    }
                    else
                    {
                        read_params->read_ctrl_step=0;//ʧ�ܺ��ͷ��ʼ
                        read_params->read_ctrl_state=0;
                    }
                }
                else
                {

                    //��ȡSNӳ���������Ҫ��֡��ȡ
                 //   if(dlms_frame_parse_sn_list(buffer,*frame_len))
                    {

                 //       read_params->read_ctrl_more_frame=1;
                    }
                //    else
                    {
                        read_params->read_ctrl_more_frame=0;
                        read_params->read_ctrl_state=1;
                    }

                }
            }
            else if(read_params->read_ctrl_state==1)
            {
              /*
	            if(0 == save_data_rs485(context))
	            {
	                pRs485Context->resend_times++;
	                if(pRs485Context->resend_times > 2)
	                {
	                    pRs485Context->resend_times = 0;
	                }
	            }
	            else
                {
                     pRs485Context->resend_times = 0;
                }
	            if(pRs485Context->read_params.read_type == READ_TYPE_CUR_DATA)
	            {
	                pRs485Context->resp_no_data_count = 0;
	                if(pRs485Context->comm_ok_count < 0xFF) pRs485Context->comm_ok_count++;
	            }
				
				if(pRs485Context->comm_ok_count >= 2)
				{				
					set_bit_value(read_meter_flag_read_ok,READ_FLAG_BYTE_NUM,bin2_int16u(pRs485Context->read_params.meter_doc.meter_idx));
				}
                  */
            }
            else if(read_params->read_ctrl_state==2)
            {
                //�Ͽ�����
                if(dlms_frame_disc_is_ok(buffer,*frame_len))
                {
                    read_params->read_ctrl_state=0;
                    read_params->read_ctrl_step=0;
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"dlms disc ok ");
                    debug_println_ext(info);
                    #endif
                }
                else
                {
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"dlms disc error ");
                    debug_println_ext(info);
                    #endif
                }
            }


    //3.��֤�Ƿ�Ϊ����Ӧ��֡
    if(dlms_frame_read_is_ok(buffer,*frame_len))  //������
    {   
        phy_u32 = bin2_int32u(read_params->phy);
        left_len = READPORT_RS485_FRAME_SIZE - (*frame_len);
        //rs232_debug_info(frame,  *frame_len);
        len=unwrap_dlms_frame(&phy_u32, 1, frame, *frame_len, left_len, METER_DLMS, 1);
        if(0 == len) return 0;
        wrap_phy2block(phy_u32,frame, len);
        
        //rs232_debug_info(frame, len);
        switch(read_params->read_type)
        {
//            case READ_TYPE_CYCLE_DAY:
//                save_cycle_day(read_params,frame+5,frame[4],buffer);
//                if (read_params->control.day_hold_save_month_hold)
//                {
//                    idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
//                    if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
//                    {
//                        save_cycle_month(read_params,frame+5,frame[4],buffer);
//                    }
//                }
//                break;
            case READ_TYPE_CURVE:
            case READ_TYPE_CUR_DATA:
            case READ_TYPE_CYCLE_DAY:
                save_cur_date(read_params,frame+5,frame[4],buffer);
                save_curve_data(read_params,frame+5,frame[4],buffer,0);
				
#ifdef __ENABLE_DOWN_RS232_RECORD_METER__
				if((read_params->meter_doc.baud_port.port == COMMPORT_485_CY) || ((read_params->meter_doc.baud_port.port >= COMMPORT_485_REC) && (read_params->meter_doc.baud_port.port <= COMMPORT_DOWN_RS232_2)))
#else
                if((read_params->meter_doc.baud_port.port == COMMPORT_485_CY) || ((read_params->meter_doc.baud_port.port >= COMMPORT_485_REC)))
#endif
				{

                    #ifndef __RS485_READ_DAYHOLD_ITEM__
                    mem_cpy(&read_params_tmp,read_params,sizeof(READ_PARAMS));
                    trans_phy_cur_2_cycle_day(read_params_tmp.phy);
                    if (check_is_all_ch(read_params_tmp.day_hold_td,3,0x00)) get_yesterday(read_params_tmp.day_hold_td);
                    if(0xFF != get_phy_form_list_cycle_day(bin2_int32u(read_params_tmp.phy),&phy))
                    {
                        if(readdata_cycle_day(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.day_hold_td,buffer,buffer+5,frame_len,NULL) == FALSE)
                        {
                            save_cycle_day(&read_params_tmp,frame+5,frame[4],buffer);
                        }
                    }
                   #ifdef __INDONESIA_DLMS_TEST__  //ӡ��ĵ������洢������
                   return 1;
                   #endif
                    //�¶��ỹδ����
                    if(check_is_all_ch(read_params->month_hold_td,2,0x00)) get_former_month(read_params_tmp.month_hold_td);
                    if(0xFF != get_phy_form_list_cycle_month(bin2_int32u(read_params_tmp.phy),&phy))
                    {
                        if(readdata_cycle_month(bin2_int16u(read_params_tmp.meter_doc.meter_idx),&phy,read_params_tmp.month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                        {
                            save_cycle_month(&read_params_tmp,frame+5,frame[4],buffer);
                        }
                    }
                    #endif
                    //�����ն�������
                    #ifndef __REC_DAY_HOLD_READ_SETTLEMENT_DAY_DATA__
                    //if(read_params->hold_flag.rec_day)
                    {
                        if(TRUE == is_recday_rs485_port(read_params->meter_doc.baud_port.port-2))
                        {
                            if(0xFF != get_phy_form_list_recday_data(bin2_int32u(read_params->phy),&phy))
                            {
                                if(readdata_recday_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,datetime+DAY,buffer,buffer+5,frame_len) == FALSE)
                                {
                                    save_recday_data(read_params,frame+5,frame[4],buffer);
                                }
                            }
                        }
                    }
                    #endif
                    if(0xFF != get_phy_from_list_day_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                    {
                        readdata_day_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                            
                        if(0 != compare_string(buffer+2,datetime+DAY,3))
                        {
                            if((phy_u32 == 0x0000007F) && (read_params->meter_doc.protocol == 0))
                            {
                                save_day_init_data(read_params,tmp_buf,25,buffer);
                            }
                            else save_day_init_data(read_params,frame+5,frame[4],buffer);
                        }
                    }
                    if(0xFF != get_phy_from_list_month_init_data(bin2_int32u(read_params->phy),&phy,&block_begin_idx,&block_count))
                    {    
                        readdata_month_init_data(bin2_int16u(read_params->meter_doc.meter_idx),&phy,buffer,buffer+5,frame_len,TRUE);
                        if(0 != compare_string(buffer+3,datetime+MONTH,2))
                        {
                            if((phy_u32 == 0x0000007F) && (read_params->meter_doc.protocol == 0))
                            {
                                save_month_init_data(read_params,tmp_buf,25,buffer);
                            }
                            else save_month_init_data(read_params,frame+5,frame[4],buffer);
                        }
                    }
                }
                break;
                case READ_TYPE_CYCLE_MONTH:
                     save_cycle_month(read_params,frame+5,frame[4],buffer);
                     break;
        }
        return 1;
    }
    else
    {
//        if(dlms_frame_frmr(buffer,*frame_len))
//        {
//        }

            mem_set(frame+14,20,0xEE);
            frame[9] = 20+4;

            switch(read_params->read_type)
            {
            case READ_TYPE_CYCLE_DAY:
                save_cycle_day(read_params,frame+14,frame[9]-4,buffer);
                if (read_params->control.month_hold_mode == 0)
                {
                    idx = get_phy_form_list_cycle_month(bin2_int32u(read_params->phy),&phy);
                    if(readdata_cycle_month(bin2_int16u(read_params->meter_doc.meter_idx),&phy,read_params->month_hold_td,buffer,buffer+5,frame_len) == FALSE)
                    {
                        save_cycle_month(read_params,frame+14,frame[9]-4,buffer);
                    }
                }
                break;
            case READ_TYPE_CURVE:
                save_curve_data(read_params,frame+14,frame[9]-4,buffer,0);
                break;
            case READ_TYPE_CUR_DATA:
                save_cur_date(read_params,frame+14,frame[9]-4,buffer);
                break;
            case READ_TYPE_DAY_HOLD_PATCH:
                save_patch_day_hold(read_params,frame+14,frame[9]-4,buffer);
                break;
            case READ_TYPE_DAY_HOLD_WAIT_TD:
                save_patch_day_hold_wait_td(read_params,frame+14,frame[9]-4,buffer);
                break;
            }
            return 1;


    }
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** save data : meter_idx = %d , item = 0x%08X , phy = 0x%08X ",bin2_int16u(read_params->meter_doc.meter_idx),
            bin2_int32u(read_params->item),bin2_int32u(read_params->phy));
    debug_println_ext(info);
    #endif
     
    return 0;
}
INT32U get_curve_normal_phy(INT32U oop_oad)
{
  switch(oop_oad)
  {
      case 0x20000200:       //��ѹ
             return  DY_SJK;
      case 0x20010200:       //����
            return  DL_SJK;
      case 0x20040200:       //�й�����
             return  SS_YGGL_SJK;
      case 0x20050200:       //�޹�����
            return  SS_WGGL_SJK;
      case 0x20060200:       //���ڹ���
            return  SS_SZGL_SJK;
      case 0x200A0200:
            return  GLYS_SJK; //��������
      case 0x20010400:
           return LX_DL;
      case 0x00100201:   //�����й�
           return (ZXYG_DN_SJK-0x3F);
      case 0x00200201:   //�����й�
           return (FXYG_DN_SJK-0x3F);
      case 0x00300201:  //�����޹�,����޹�1
           return (ZHWG1_DN_SJK-0x3F);
      case 0x00400201:  //�����޹� ������޹�2
           return (ZHWG2_DN_SJK-0x3F);
      default:
            return 0xFFFFFFFF;
  }

}

