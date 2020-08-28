#include "app_dev_plat.h"
#include "config_readport.h"
#include "tpos_plc_friend.h"
#include "tpos_readport_plc.h"
#include "voltage_monitor.h"     //__VOLTAGE_MONITOR__
#include "tpos_plc_parall.h"
#include "drv_rs232.h"
#include "display/display_status_bar.h"
#include "protocol_library_oop.h"
UPDATE_ROUTER_CTRL update_router_ctrl;






#if (defined __FUJIAN_SUPPLEMENT_SPECIFICATION__)
#include "tpos_readport_fujian.h"
#endif
void set_plc_meter_event_task_flag(INT8U rec_year,INT8U rec_month,INT8U rec_day);
void get_plan_min_cycle(INT8U data[MAX_METER_EVENT_PLAN_COUNT]);
BOOLEAN prepare_plc_read_report_cjq_event_state(READ_PARAMS *read_params,INT8U *frame,INT32U *item);
BOOLEAN prepare_plc_read_report_meter_event_state(READ_PARAMS *read_params,INT8U *frame,INT8U* frame_len);
#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
void check_hunan_curve_recording(void);
#endif
#ifdef __INSTANT_FREEZE__
BOOLEAN check_plc_instant_freeze(void);
#endif
#ifdef __PLC_EVENT_READ__
void check_plc_event_task(void);
#endif
#ifdef __PROVICE_CHONGQING__
void  update_llvc_rec_state(void);
void plc_router_relay_stat(void);
#endif
void timer_plc_net(void);
BOOLEAN meter_cast_timing(objReadPortContext * readportcontext,INT8U port);
BOOLEAN exec_ctrl_cmd(PLCPortContext *portContext);
BOOLEAN check_plc_reset_router(PLCPortContext *portContext);

BOOLEAN  plc_router_vipmeter_recording(objReadPortContext * readportcontext);
INT16U get_ertu_vip_meters(INT8U *resp,INT8U max_vip_count);
INT8U get_read_vip_meter_info(objReadPortContext * readportcontext);
INT8U prepare_read_vip_item(objReadPortContext * readportcontext);
void start_read_vip_meter(PLCPortContext* portContext);
INT8U get_vip_meter_count(void);

#if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
BOOLEAN check_ctrl_switch_trip(objReadPortContext * readportcontext);
BOOLEAN prepare_plc_switch_trip_ctrl(objReadPortContext * readportcontext,INT8U *frame,INT8U *framelen);
#endif
#ifdef __MEXICO_GUIDE_RAIL__

#endif
/****************************************************************
 * ���ܣ�
 *     �㲥�ɼ�����������
 * ������
 *     �Թ㲥�ķ�ʽ�����вɼ�������������
 * ������
 *     ��
 ******************************************************************/
void plc_send_frame(INT8U afn,INT16U fn,INT8U *data,INT16U datalen)
{    
    //if (afn == DL69842_AFN_TRANS)
    //{
    //    router_376_2_set_aux_info(0,80,1,TRUE);
    //}
    //else
    {
        router_376_2_set_aux_info(0,0,0,TRUE);
    }
    portContext_plc.frame_send_Len = router_376_2_cmd_frame(afn,fn,data,datalen,&portContext_plc);
    channel_plc_send((objReadPortContext *)&portContext_plc);
}

INT8U plc_recv_frame_one_byte(void)
{
    PLCPortContext* portContext;
    DL69842_RRR RRR;   // ��Ϣ�����нṹ
    INT16U pos,frame_len;
    INT8U result;
    
    portContext = &portContext_plc;
    result = channel_plc_recv((objReadPortContext *)&portContext_plc);
    if (result  == READPORT_RECV_COMPLETE)
        {
            //��Ϣ��
            mem_cpy(RRR.value,portContext->frame_recv + POS_DL69842_INFO,sizeof(DL69842_RRR));
            //�ŵ�����λ
            portContext->router_work_info.channel_id = RRR.resp_relay_channel.channel;
            portContext->router_work_info.phase = portContext->router_work_info.channel_id - 1;
            if(portContext->router_work_info.channel_id == 0)portContext->router_work_info.phase = 3; //ʹ��ת��3��ȥ����δ�޸�channel_id-1�ķ�ʽ����Ϊ���õĵط��Ƚ϶࣬��+1�ĵط�
            if(portContext->router_work_info.phase > 3)      //������0,1,2,3,��4���ŵ�
            {
                portContext->router_work_info.phase=0;
            }
            portContext->router_work_info.frame_seq = RRR.reserved2;
        //    //ʵ�����

            //�м����
            portContext->router_work_info.relay = RRR.resp_relay_channel.relay;

            //��Ϣ���
            pos = POS_DL69842_ADDR;
            if(RRR.resp_relay_channel.modem)
            {
                frame_len = bin2_int16u(portContext->frame_recv+1);
                if (frame_len > (pos+12)) //��ֹĳЩ����·�ɻظ��Ĵ���ַ��־����ʵ���ֲ���
                {
                    pos +=12;
                }
            }

            portContext->afn = portContext->frame_recv[pos++];
            portContext->fn = portContext->frame_recv[pos++];
            portContext->fn += portContext->frame_recv[pos++]<<8;
            portContext->recv_data_pos = pos;
        }
    return result;
}

BOOLEAN plc_send_recv_frame(INT8U afn,INT16U fn,INT8U *data,INT16U datalen,INT8U resp_afn,INT16U resp_fn,INT32U time_out_10ms)
{
    PLCPortContext* portContext;
    INT32U start_time_10ms;

    portContext = &portContext_plc;
    //����
    plc_send_frame(afn,fn,data,datalen);
    //����
    portContext_plc.recv_status.recv_frame_type = 0;
    portContext_plc.uart_recv_byte_time_out = os_get_systick_10ms();
    start_time_10ms = os_get_systick_10ms();
    while(time_elapsed_10ms(start_time_10ms) < time_out_10ms)
    {
        if (plc_recv_frame_one_byte()  == READPORT_RECV_COMPLETE)
        {
            if (portContext->afn == resp_afn)
            {
                if (portContext->fn > 0) 
                {
                    if (portContext->fn == resp_fn) return TRUE;
                }
                else return TRUE;
            }
        }
    }
    return FALSE;
}

INT8U check_add_node_ver(INT8U count,INT8U *data,INT8U* ver)
{
    INT8U idx;
    for(idx=0;idx<count;idx++)
    {
        if(compare_string(data+idx*11,ver,11) == 0) return count;
    }
    if (count<10)
    {
        mem_cpy(data+count*11,ver,11);
        count++;
    }
    return count;
}
void exec_update_router(void)
{

    void unlight_LED_ERR(void);
    INT8U *node_ptr;
    INT32U progLen, progStart,filesize;
    INT32U start_time_10ms,start_time_query;
    INT32U wait_time_slave_node_update;
    INT32U frame_idx;
    INT16U i, len,pos;
    INT16U  prog_page_size;
    INT16U count,save_idx;
    INT8U k;
    //INT8U TOPS_ENCRYPT_1_STR[] = { 'm','o','c','.','m','m','o','c','s','p','o','t','.','w','w','w' };
    INT8U try_count;
    INT8U file_prop,file_flag;
    INT16U total_sec, sec_idx;
    INT8U data[550]={0};
    INT8U count_resp,count_query,count_ver;
    INT8U main_node[6]={0};
    BOOLEAN result;
    BPLC_SLAVE_NODE_VER node_ver;

    if (update_router_ctrl.readflag != 0xAA)  return;

    fread_array(FILEID_UPDATE_ROUTER, FLADDR_ROUTER_UPDATE_FILE_SIZE, (INT8U*)&filesize, 4);
    if ((filesize == 0xFFFFFFFF) || (filesize > 0x100000)) //���ܳ���1M
    {
        tpos_enterCriticalSection();
        update_router_ctrl.readflag = 0xFF;  //�����־
        tpos_leaveCriticalSection();
        return;
    }

    try_count = 0;

    LED_BG_Light(TRUE);
#ifdef __DISPLAY_IN_CHINESE__
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
    {
        Led_ShowInfor(1, "�ӽڵ㿪ʼ����");
    }
    else
    {
        Led_ShowInfor(1, "·�ɿ�ʼ����");
    }
#else
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
    {
        Led_ShowInfor(1, "slave node update ...");
    }
    else
    {
        Led_ShowInfor(1, "router update ...");
    }
#endif

    plc_send_recv_frame(DL69842_AFN_RCTRL,DT_F2,NULL,0,DL69842_AFN_CONFIRM,0,3000);
    plc_send_recv_frame(DL69842_AFN_RSET,DT_F6,NULL,0,DL69842_AFN_CONFIRM,0,3000);

    if (portContext_plc.router_base_info.router_vendor != ROUTER_VENDOR_TOPSCOMM)
    {
        try_count = 2;
    }

START:
    try_count++;
    if (try_count > 3)
    {
#ifdef __DISPLAY_IN_CHINESE__
        if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
            Led_ShowInfor(1, "�ӽڵ�����ʧ��");
        else
        Led_ShowInfor(1, "·������ʧ��");
#else
        if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
            Led_ShowInfor(1, "slave node update failed");
        else
            Led_ShowInfor(1, "router update failed");
#endif
        tpos_enterCriticalSection();
        update_router_ctrl.readflag = 0x55;
        tpos_leaveCriticalSection();

        portContext_plc.need_reset_router = 1;//��Ҫ��λ��������
        return;//���3��ʧ�ܣ����������
    }

    //��������ļ�
    make_router_update_info(data, 0x00, 0x00, 0x00, 0x0000, 0x0000, 0x0000);
    result = FALSE;
    if (plc_send_recv_frame(DL69842_AFN_FILE,DT_F1,data,11,DL69842_AFN_FILE,DT_F1,3000))
    {
        if ((portContext_plc.afn == DL69842_AFN_FILE) && (portContext_plc.fn == DT_F1))
        {
            if (bin2_int32u(portContext_plc.frame_recv+portContext_plc.recv_data_pos) == 0)
            {
                result = TRUE;
            }
        }
    }
    if (result == FALSE) goto START;

#ifdef __DISPLAY_IN_CHINESE__
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
        Led_ShowInfor(1, "�ӽڵ�������...");
    else
        Led_ShowInfor(1, "·��������...");
#else
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
        Led_ShowInfor(1, "slave node update ...");
    else
        Led_ShowInfor(1, "router update ...");
#endif

    //��������
    progLen = filesize;

    //ÿ�ζ�ȡ����ĳ���,��С���ܳ���512
    if (portContext_plc.router_base_info.router_info1.comm_mode == 2)//���
    {
        prog_page_size = 256;
    }
    else
    {
        prog_page_size = 224;
    }

    if(update_router_ctrl.file_type==0x00)
    {
        progStart = 512;
    }
    else
    {
        progStart = 0;
    }


    total_sec = progLen / prog_page_size;
    if (progLen % prog_page_size != 0)
    {
        total_sec++;
    }
    sec_idx = 0;
    while (progLen > 0)
    {
        tpos_clrTaskWdt(); //����
        if (update_router_ctrl.readflag != 0xAA)
        {
#ifdef __DISPLAY_IN_CHINESE__
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
        Led_ShowInfor(1, "�ӽڵ�������ֹ");
    else
        Led_ShowInfor(1, "·��������ֹ...");
#else
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
        Led_ShowInfor(1, "slave node update stop");
    else
        Led_ShowInfor(1, "router update stop ...");
#endif
            make_router_update_info(data, 0x00, 0x00, 0x00, 0x0000, 0x0000, 0x0000);
            plc_send_recv_frame(DL69842_AFN_FILE,DT_F1,data,11,DL69842_AFN_FILE,DT_F1,3000);
            portContext_plc.need_reset_router = 1;//��Ҫ��λ��������
            return;
        }

        if (progLen >= prog_page_size)
            len = prog_page_size;
        else
            len = progLen;
        fread_array(FILEID_UPDATE_ROUTER, FLADDR_ROUTER_UPDATE_FILE+progStart, data + 11, len);

#ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"total_sec = %d , sec_idx = %d",total_sec,sec_idx);
        debug_println(info);
        GetHexMessage(data + 11, len, info);
        debug_println(info);
#endif
        file_prop = (progLen == len) ? 0x01 : 0x00;
        file_flag = (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG) ? 0x08 : 0x03;
        make_router_update_info(data, file_flag, file_prop, 0x00, total_sec, sec_idx, len);
        for(i=0;i<3;i++)
        {
            if (plc_send_recv_frame(DL69842_AFN_FILE,DT_F1,data,11+len,DL69842_AFN_FILE,DT_F1,3000) == TRUE)
            {
                if ((portContext_plc.afn == DL69842_AFN_FILE) && (portContext_plc.fn == DT_F1))
                {
                    if (bin2_int32u(portContext_plc.frame_recv+portContext_plc.recv_data_pos) == sec_idx)
                    {
                        break;
                    }
                }
            }
        }
        if (i >= 3) goto START;
        //else DelayNmSec(500);
        sec_idx++;
        progStart += len;
        progLen -= len;
        //��˸�澯��
        toggle_LED_ERR();
    }
    unlight_LED_ERR();

    DelayNmSec(8000);

    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
    {
        wait_time_slave_node_update = 0;
        mem_set(main_node,6,0x00);
        //��ѯ03f10
        if (plc_send_recv_frame(DL69842_AFN_QUERY,DT_F10,NULL,0,DL69842_AFN_QUERY,DT_F10,3000))
        {
            if ((portContext_plc.afn == DL69842_AFN_QUERY) && (portContext_plc.fn == DT_F10))
            {
                wait_time_slave_node_update = portContext_plc.frame_recv[portContext_plc.recv_data_pos+13];
                mem_cpy(main_node,portContext_plc.frame_recv+portContext_plc.recv_data_pos+14,6);
            }
        }

        start_time_10ms = os_get_systick_10ms();
        start_time_query = os_get_systick_10ms();
        while(time_elapsed_10ms(start_time_10ms) < wait_time_slave_node_update*60*100)
        {
            tpos_clrTaskWdt(); //����
            if (update_router_ctrl.readflag != 0xAA)
            {
#ifdef __DISPLAY_IN_CHINESE__
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
        Led_ShowInfor(1, "�ӽڵ�������ֹ");
    else
        Led_ShowInfor(1, "·��������ֹ...");
#else
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
        Led_ShowInfor(1, "slave node update stop");
    else
        Led_ShowInfor(1, "router update stop ...");
#endif
                make_router_update_info(data, 0x00, 0x00, 0x00, 0x0000, 0x0000, 0x0000);
                plc_send_recv_frame(DL69842_AFN_FILE,DT_F1,data,11,DL69842_AFN_FILE,DT_F1,3000);
                portContext_plc.need_reset_router = 1;//��Ҫ��λ��������
                return;
            }
            
            if (time_elapsed_10ms(start_time_query) > 60*100) //60s
            {
#ifdef __DISPLAY_IN_CHINESE__
    Led_ShowInfor(1, "�ӽڵ�������");
#else
    Led_ShowInfor(1, "slave node update ...");
#endif
                start_time_query = os_get_systick_10ms();
                //��ѯ10f4
                if (plc_send_recv_frame(DL69842_AFN_RQUERY,DT_F4,NULL,0,DL69842_AFN_RQUERY,DT_F4,3000))
                {
                    if ((portContext_plc.afn == DL69842_AFN_RQUERY) && (portContext_plc.fn == DT_F4))
                    {
                        if ((portContext_plc.frame_recv[portContext_plc.recv_data_pos] & 0x02) == 0)
                        {
                            if (portContext_plc.frame_recv[portContext_plc.recv_data_pos] & 0x01) break;
                        }
                    }
                }
            }
        }

        /*
        //��ѯ�ӽڵ�
        save_idx = 0;
        idx = 1;
        count = 0;
        count_query = 40;
        while(TRUE)
        {
            data[0] = idx;
            data[1] = idx>>8;
            data[2] = count_query;
            if (plc_send_recv_frame(DL69842_AFN_QUERY,DT_F101,data,3,3000))
            {
                if ((portContext_plc.afn == DL69842_AFN_QUERY) && (portContext_plc.fn == DT_F101))
                {
                    count = bin2_int16u(portContext_plc.frame_recv+portContext_plc.recv_data_pos);
                    count_resp = portContext_plc.frame_recv[portContext_plc.recv_data_pos+2];
                    if (idx == 1) count_query = count_resp;
                    if (count_resp > 0)
                    {
                        fwrite_array(FILEID_UPDATE_ROUTER, FLADDR_ROUTER_UPDATE_PLC_NODE_VER+save_idx*11,portContext_plc.frame_recv+portContext_plc.recv_data_pos+3,count_resp+11);
                        save_idx += count_resp;
                    }
                }
                else break;
            }
            else break;
            idx += count_query;
            if (count == 0) break;
            if (idx > count) break;
        }
        if (save_idx > 0)
        {
            fwrite_array(FILEID_UPDATE_ROUTER, FLADDR_ROUTER_UPDATE_PLC_NODE_COUNT,(INT8U*)&save_idx,2);
        }
        */

        //��ѯ�ӽڵ�
        count = 20;
        count_query = 20;
        frame_idx = 0;
        count_ver = 0;
        for(k=0;k<5;k++)
        {
            result = TRUE;
            for(i=0;i<count;i++)
            {
                tpos_clrTaskWdt(); //����
                if (k == 0)
                {
                    if ((i % count_query) == 0)
                    {
                        data[0] = i+1;
                        data[1] = (i+1)>>8;
                        data[2] = count_query;
                        if (plc_send_recv_frame(DL69842_AFN_RQUERY,DT_F2,data,3,DL69842_AFN_RQUERY,DT_F2,3000))
                        {
                            if ((portContext_plc.afn == DL69842_AFN_RQUERY) && (portContext_plc.fn == DT_F2))
                            {
                                if (i == 0) count = bin2_int16u(portContext_plc.frame_recv+portContext_plc.recv_data_pos);
                                if (count > MAX_METER_COUNT) goto END_UPDATE;
                                count_resp = portContext_plc.frame_recv[portContext_plc.recv_data_pos+2];
                                if (count_resp <= count_query)
                                {
                                    //data��ʹ��ǰ��160��20���ڵ���Ϣ����200��ʼ�Ű汾ͳ����Ϣ����400��ʼ��ʱ��֯����ʹ��
                                    if ((i == 0) && (count_resp > 0)) count_query = count_resp;
                                    mem_cpy(data,portContext_plc.frame_recv+portContext_plc.recv_data_pos+3,count_resp*8);
                                }
                                else goto END_UPDATE;
                            }
                            else goto END_UPDATE;
                        }
                        else goto END_UPDATE;
                    }
                    node_ptr = data+(i % count_query)*8;
                }
                else
                {
                    fread_array(FILEID_UPDATE_ROUTER,FLADDR_ROUTER_UPDATE_PLC_NODE_VER+i*sizeof(BPLC_SLAVE_NODE_VER),node_ver.value,sizeof(BPLC_SLAVE_NODE_VER));
                    if (node_ver.device_type != 0xFF) continue;
                    mem_cpy(data,node_ver.node,6);
                    node_ptr = data;
                }

                pos = 400;
                data[pos++] = 0x04;
                data[pos++] = 0x18;

                data[pos++] = 0xFE;
                data[pos++] = 0xFE;
                data[pos++] = 0x0F;
                data[pos++] = 0x00;

                data[pos++] = 0x01;
                data[pos++] = 0x05;

                data[pos++] = 0x00;
                data[pos++] = 0x03;

                data[pos++] = frame_idx;
                data[pos++] = frame_idx>>8;
                data[pos++] = frame_idx>>16;
                data[pos++] = frame_idx>>24;

                data[pos++] = main_node[5];
                data[pos++] = main_node[4];
                data[pos++] = main_node[3];
                data[pos++] = main_node[2];
                data[pos++] = main_node[1];
                data[pos++] = main_node[0];
                            
                data[pos++] = node_ptr[5];
                data[pos++] = node_ptr[4];
                data[pos++] = node_ptr[3];
                data[pos++] = node_ptr[2];
                data[pos++] = node_ptr[1];
                data[pos++] = node_ptr[0];
                            
                mem_cpy(portContext_plc.router_work_info.ADDR_DST,node_ptr,6);
                            
                mem_set(node_ver.value,sizeof(BPLC_SLAVE_NODE_VER),0xFF);
                mem_cpy(node_ver.node,node_ptr,6);
                
                //����
                plc_send_frame(DL69842_AFN_TRANS,DT_F1,data+400,26);
                //���� ·�ɻ��ϱ�����Ӧ����
                portContext_plc.recv_status.recv_frame_type = 0;
                portContext_plc.uart_recv_byte_time_out = os_get_systick_10ms();
                start_time_10ms = os_get_systick_10ms();
                while(time_elapsed_10ms(start_time_10ms) < 1000) //10s
                {
                    if (plc_recv_frame_one_byte()  == READPORT_RECV_COMPLETE)
                    {
                        if ((portContext_plc.afn == DL69842_AFN_TRANS) && (portContext_plc.fn == DT_F1))
                        {
                            if ((portContext_plc.frame_recv[portContext_plc.recv_data_pos] == 0x04) && (portContext_plc.frame_recv[portContext_plc.recv_data_pos+1] == 0x36))
                            {
                                if ((portContext_plc.frame_recv[portContext_plc.recv_data_pos+14] == node_ptr[5])
                                 && (portContext_plc.frame_recv[portContext_plc.recv_data_pos+15] == node_ptr[4])
                                 && (portContext_plc.frame_recv[portContext_plc.recv_data_pos+16] == node_ptr[3])
                                 && (portContext_plc.frame_recv[portContext_plc.recv_data_pos+17] == node_ptr[2])
                                 && (portContext_plc.frame_recv[portContext_plc.recv_data_pos+18] == node_ptr[1])
                                 && (portContext_plc.frame_recv[portContext_plc.recv_data_pos+19] == node_ptr[0]))
                                {
                                    count_ver = check_add_node_ver(count_ver,data+201,portContext_plc.frame_recv+portContext_plc.recv_data_pos+31);
                                    node_ver.device_type = portContext_plc.frame_recv[portContext_plc.recv_data_pos+9];
                                    mem_cpy(&(node_ver.sw_ver_no_main),portContext_plc.frame_recv+portContext_plc.recv_data_pos+31,11);
                                    DelayNmSec(1000); //���ͼ��1s
                                    break;
                                }
                            }
                        }
                    }
                }
                
//                if (plc_send_recv_frame(DL69842_AFN_TRANS,DT_F1,data+400,26,DL69842_AFN_TRANS,DT_F1,1000))
//                {
//                    if ((portContext_plc.afn == DL69842_AFN_TRANS) && (portContext_plc.fn == DT_F1))
//                    {
//                        if ((portContext_plc.frame_recv[portContext_plc.recv_data_pos] == 0x04) && (portContext_plc.frame_recv[portContext_plc.recv_data_pos+1] == 0x36))
//                        {
//                            count_ver = check_add_node_ver(count_ver,data+201,portContext_plc.frame_recv+portContext_plc.recv_data_pos+31);
//                            node_ver.device_type = portContext_plc.frame_recv[portContext_plc.recv_data_pos+9];
//                            mem_cpy(&(node_ver.sw_ver_no_main),portContext_plc.frame_recv+portContext_plc.recv_data_pos+31,11);        
//                        }
//                    }
//                }
                if (node_ver.device_type == 0xFF) result = FALSE;
                fwrite_array(FILEID_UPDATE_ROUTER,FLADDR_ROUTER_UPDATE_PLC_NODE_VER+i*sizeof(BPLC_SLAVE_NODE_VER),node_ver.value,sizeof(BPLC_SLAVE_NODE_VER));
                frame_idx ++;
            }
            if (result) break;
        }
        if (count > 0)
        {
            fwrite_array(FILEID_UPDATE_ROUTER, FLADDR_ROUTER_UPDATE_PLC_NODE_COUNT,(INT8U*)&count,2);
            data[200] = count_ver;
            fwrite_array(FILEID_UPDATE_ROUTER, FlADDR_ROUTER_UPDATE_VER_COUNT,data+200,count_ver*11+1); 
        }
    }

    END_UPDATE:

#ifdef __DISPLAY_IN_CHINESE__
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
        Led_ShowInfor(1, "�ӽڵ������ɹ�");
    else
        Led_ShowInfor(1, "·�������ɹ�");
#else
    if (update_router_ctrl.update_type == FILEFLAG_PLC_NODE_PROG)
        Led_ShowInfor(1, "slave node update succeed");
    else
        Led_ShowInfor(1, "router update succeed");
#endif

    save_idx = 0x55;
    fwrite_array(FILEID_UPDATE_ROUTER, FLADDR_ROUTER_UPDATE_CTRL_INFO,(INT8U*)&save_idx,1);

    tpos_enterCriticalSection();
    update_router_ctrl.readflag = 0x55;
    tpos_leaveCriticalSection();

    portContext_plc.need_reset_router = 1;//��Ҫ��λ��������

    record_log_code(LOG_SYS_ROUTER_UPDATE_END, NULL, 0, LOG_ALL);//·�������ɹ�
}

void cjq_update_day_count(void)
{
    CJQ_UPDATE_CTRL cjq_update_ctrl;

    fread_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
    if ((cjq_update_ctrl.flag == 0xCC) && (cjq_update_ctrl.day_count < g_cjq_update_data.cjq_update_day_count))
    {
        cjq_update_ctrl.day_count++;
        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
    }
}

/****************************************************************
 * ���ܣ�
 *     �㲥�ɼ�����������
 * ������
 *     �Թ㲥�ķ�ʽ�����вɼ�������������
 * ������
 *     ��
 ******************************************************************/
void exec_cast_update_cjq(void)
{
    INT8U* frame;
    INT32U idx,time_out_10ms,wait_time_10ms;
    CJQ_UPDATE_CTRL cjq_update_ctrl;
    INT8U data[200];
    INT8U pos,flag;
    if( portContext_plc.read_status.plc_net == 1) return;
    if( portContext_plc.cur_plc_task == PLC_TASK_PLC_NET) return;
    if (portContext_plc.router_base_info.router_vendor == 0) return;
    if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) return;
    fread_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
    if ((cjq_update_ctrl.flag == 0xAA) || (cjq_update_ctrl.flag == 0x55))
    {
        //���ſ���ز�·�ɣ���ʱ��֧�ֹ㲥�����������Ĺ㲥��������
        if ((portContext_plc.router_base_info.router_info1.comm_mode == 2) && (portContext_plc.router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)) 
        {
            portContext_plc.ctrl_cmd.cmd_redo = 1;  //����
            cjq_update_ctrl.flag = 0xCC;
            cjq_update_ctrl.day_count = 0;
            set_cosem_datetime_s(cjq_update_ctrl.finish_datetime,datetime);
            fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
            portContext_plc.cjq_update_flag = 0x00;
            return;
        }
        router_376_2_set_aux_info(0,0,0,TRUE);
        portContext_plc.frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RCTRL,DT_F2,NULL,0,&portContext_plc);
        channel_plc_send((objReadPortContext *)&portContext_plc);
        portContext_plc.recv_status.recv_frame_type = 0;
        portContext_plc.uart_recv_byte_time_out = os_get_systick_10ms();
        time_out_10ms = os_get_systick_10ms();
        while(time_elapsed_10ms(time_out_10ms) < 1000)
        {
            if (channel_plc_recv((objReadPortContext *)&portContext_plc)  == READPORT_RECV_COMPLETE) break;
        }

        for(idx=0;idx<bin2_int16u(cjq_update_ctrl.block_count)+3;idx++)
        {
            if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) break;
            
            frame = data+2;
            pos = 0;
            if ((idx == 0) || (idx == 1) || (idx == 2))
            {
                if (cjq_update_ctrl.flag == 0xAA)
                {
                    //�㲥��������ļ���3�Σ�������֤ȫ�ɼ��������յ�����������˹㲥����
                    pos = make_cjq_update_file_frame(frame,NULL,0x04A01101,&cjq_update_ctrl,0,TRUE);
                    if (idx == 2)
                    {
                        cjq_update_ctrl.flag = 0x55;
                        set_cosem_datetime_s(cjq_update_ctrl.start_datetime,datetime);
                        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
                    }
                }
                else continue;
            }
            else
            {
                if (get_bit_value(cjq_update_ctrl.block,128,idx-3) == 0) continue;
                //�����ļ�����
                pos = make_cjq_update_file_frame(frame,NULL,0x04A01101,&cjq_update_ctrl,idx-3,FALSE);
                clr_bit_value(cjq_update_ctrl.block,128,idx-3);
                fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
            }
AGAIN_SEND:
            if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) break;

            router_376_2_set_aux_info(0,0,0,TRUE);
            data[0] = 0x00; //������
            data[1] = pos;
            portContext_plc.frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_CTRL,DT_F3,data,pos+2,&portContext_plc);
            channel_plc_send((objReadPortContext *)&portContext_plc);
            portContext_plc.recv_status.recv_frame_type = 0;
            portContext_plc.uart_recv_byte_time_out = os_get_systick_10ms();
            time_out_10ms = os_get_systick_10ms();
            while(time_elapsed_10ms(time_out_10ms) < 1000)
            {
                if (channel_plc_recv((objReadPortContext *)&portContext_plc) == READPORT_RECV_COMPLETE) break;
            }
            if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) break;
            if (portContext_plc.frame_recv_Len > 0)
            {
                if (portContext_plc.frame_recv_buffer[POS_DL69842_AFN] == DL69842_AFN_CONFIRM)
                {
                    flag = FALSE;
                    if (bin2_int16u(portContext_plc.frame_recv_buffer+POS_DL69842_FN) == DT_F1)
                    {
                        flag = TRUE;
                        wait_time_10ms = (bin2_int16u(portContext_plc.frame_recv_buffer+POS_DL69842_DATA+4)+2)*1000;
                        DelayNmSec(wait_time_10ms);
                    }

                    if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) break;

                    wait_time_10ms = os_get_systick_10ms();
                    while(time_elapsed_10ms(wait_time_10ms) < 300*100)
                    {
                        if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) break;
                        
                        router_376_2_set_aux_info(0,0,0,TRUE);
                        portContext_plc.frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F4,NULL,0,&portContext_plc);
                        channel_plc_send((objReadPortContext *)&portContext_plc);
                        time_out_10ms = os_get_systick_10ms();
                        portContext_plc.recv_status.recv_frame_type = 0;
                        portContext_plc.uart_recv_byte_time_out = os_get_systick_10ms();
                        while(time_elapsed_10ms(time_out_10ms) < 1000)
                        {
                            if (channel_plc_recv((objReadPortContext *)&portContext_plc) == READPORT_RECV_COMPLETE) break;
                        }
                        if ((portContext_plc.frame_recv_buffer[POS_DL69842_AFN] == DL69842_AFN_RQUERY) && (bin2_int16u(portContext_plc.frame_recv_buffer+POS_DL69842_FN) == DT_F4))
                        {
                            if( (portContext_plc.frame_recv_buffer[POS_DL69842_DATA+13]==8) && (portContext_plc.frame_recv_buffer[POS_DL69842_DATA+14]==8) && (portContext_plc.frame_recv_buffer[POS_DL69842_DATA+15]==8) )
                            {
                                if (flag) break;
                                else goto AGAIN_SEND;
                            }
                        }

                        if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) break;
                        
                        DelayNmSec(10000);
                    }
                    
                    if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) break;
                }
                else goto AGAIN_SEND;
            }
        }

        portContext_plc.ctrl_cmd.cmd_redo = 1;
        if ((portContext_plc.cjq_update_flag != 0xAA) && (portContext_plc.cjq_update_flag != 0x55)) return;
        cjq_update_ctrl.flag = 0xCC;
        cjq_update_ctrl.day_count = 0;
        set_cosem_datetime_s(cjq_update_ctrl.finish_datetime,datetime);
        fwrite_array(FILEID_UPDATE_CJQ_CTRL,FLADDR_UPDATE_CJQ_CTRL_CAST_FLAG,cjq_update_ctrl.value,sizeof(CJQ_UPDATE_CTRL));
        portContext_plc.cjq_update_flag = 0x00;
    }
}
//�����������ȴ���һ�����������״̬
INT8U urgent_task_in_wait_next_urgent_task(PLCPortContext* portContext)
{
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** PLC_TASK_URGENT_TASK - wait next urgent task ***");
    debug_println_ext(info);
    #endif

    //�л����ձ��ĸ�ʽΪ376.2
    if (portContext->urgent_task_step == PLC_URGENT_TASK_TRANS_SEND_TOPSCOMM)
    {
        //����״̬
        portContext->recv_status.recv_frame_type = 0;
        portContext->recv_status.recv_frame_byte_time_out_ms = 1000;  //��λ��s
    }

    if(portContext->urgent_task_id != RECMETER_TASK_NONE)/*����Ѿ�û�н��������ˣ���Ҫ�ظ���״̬*/
    {
        portContext->tick = os_get_systick_10ms();/*���ý��������˳��Ŀ�ʼ��ʱʱ��*/

        switch(portContext->urgent_task_id) /*�еĽ��������Ǽ���������ģ�����Ҫ�ȴ���ֱ���˳�����*/
        {
          case RECMETER_TASK_TRANS_CAST_TIME:
          case RECMETER_TASK_METER_EVENT_REPORT:
          case RECMETER_TASK_TRANS_NOISE:
          case RECMETER_TASK_TRANS_READ_MODULE_ID:
          case RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE:
          case RECMETER_TASK_TRANS_CAST_PRECISE_TIME:
          case RECMETER_TASK_BPLC_NETWORK_INFO:
          case BPLC_AREA_DIS_F0_F111:
          case BPLC_AREA_DIS_F0_F112:
          #ifdef __MEXICO_CIU__ /* ī����CIU �ϱ���ִ����ɺ����̽��� */
          case RECMETER_TASK_MEXICO_CIU_REPORT:
          #endif
          case RECMETER_TASK_GET_AGGREGATION:
               portContext->tick -= 6000;/*��λ10ms Ϊ���������˳���������,֮ǰ��ȴ�60s�ٻָ�*/
               break;
        }
       /*����ȴ������������״̬������˵�ǵȴ���һ����������ʼִ��*/

        portContext->urgent_task_id = RECMETER_TASK_NONE;
        portContext->urgent_task_step = PLC_URGENT_TASK_WAIT_NEXT;

        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }

    return 0;
}

/*+++
  ���ܣ�ִ�н����ز���·�����������
  ������
        ��
  ���أ�
       ��
  ������
       1���ɹ�ִ��һ������󣬵ȿ���60����ٷ��أ��������Է�����Ӧ��վ��������������
       2��ĳЩ�����ִ��ʱ���Ƚϳ�������ڵȴ���ʱ��Ҫ����ͬ�Ĵ���
---*/
INT8U exec_urgent_task(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT16U meter_idx,save_pos;
    INT8U router_protocol = 0;
    #ifdef __PLC_BPLC_AGG__
    INT16U max_node_idx = 0;
    INT8U agg_id = 0;
    #endif
    #ifdef __INSTANT_FREEZE__
    INT16U freeze_id;
    INT8U item[32] = {0};
    INT8U item_cnt = 0;
    #endif
    
    portContext = (PLCPortContext*)readportcontext;

    switch(portContext->urgent_task_id)
    {
    case RECMETER_TASK_TRANS_METER_FRAME:
        if (portContext->router_base_info.router_info4.monitor_afn_type)
        {
            readport_plc.OnPortReadData = router_send_afn_02_F1;
        }
        else
        {
            if((portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT) && (portContext->read_status.doc_chg))
            {
                if( memory_fast_index_find_node_no(COMMPORT_PLC,portContext->urgent_task_meter_doc->meter_no,&meter_idx,&save_pos,&router_protocol,NULL) )
                {
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count = 1;
                  mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[0].node,portContext->urgent_task_meter_doc->meter_no,6);
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[0].protocol = router_protocol;
                }
                portContext->urgent_task = PLC_TASK_URGENT_TASK;
                readport_plc.OnPortReadData = router_send_afn_11_F1;
                portContext->urgent_task_step = PLC_URGENT_TASK_ADD_NODE;
                return 1;
            }
            else
            {
                readport_plc.OnPortReadData = router_send_afn_13_F1;
            }
        }
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_TASK_MONIER_645_1;
        return 1;
    case RECMETER_TASK_TRANS_376_2_FRAME:
        readport_plc.OnPortReadData = router_send_trans_frame;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_TASK_TRANS_SEND_376_2;
        return 1;
    case RECMETER_TASK_TRANS_TOPSCOMM_FRAME:
        readport_plc.OnPortReadData = router_send_trans_frame;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_TASK_TRANS_SEND_TOPSCOMM;
        portContext->recv_status.recv_frame_type = 1;
        portContext->recv_status.recv_frame_byte_time_out_ms = 5000;
        return 1;
    case RECMETER_TASK_TRANS_CAST:
    //
        readport_plc.OnPortReadData = router_send_afn_05_F3;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_CAST_OPEN_05_F3;

        break;
    case RECMETER_TASK_TRANS_CAST_TIME:
        //���÷��ͻ��������ݱ���
        portContext->cast_content = portContext->frame_cast_buffer; // portContext->frame_send
            portContext->cast_content_len = make_gb645_adj_time_frame(portContext->cast_content,NULL,0); //�㲥Уʱ
        readport_plc.OnPortReadData = router_send_afn_03_F9;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_CAST_DELAY_03_F9;
        break;
    case RECMETER_TASK_TRANS_CAST_TIME_FROM_STATION:
        //����վ�·�Уʱ����
        readport_plc.OnPortReadData = router_send_afn_03_F9;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_CAST_DELAY_03_F9;
        break;
    case RECMETER_TASK_METER_EVENT_REPORT:
        readport_plc.OnPortReadData = router_send_afn_13_F1;                 //�¼��ϱ���ؿ�ʼ
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_METER_EVENT_REPORT_CTRL_06F5_3;
        break;
    #ifdef __READ_PLC_NOISE__
    case RECMETER_TASK_TRANS_NOISE:
        readport_plc.OnPortReadData = router_send_afn_03_F2;                 //������������
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_TASK_READ_NOISE;
        break;
    #endif
	#ifdef __READ_MODULE_ID_PLAN__
    case RECMETER_TASK_TRANS_READ_MODULE_ID:
        /* ��ģ��ID��խ���������Ҫ */
        {
    		portContext->task_read_module_id.node_start_seq[0] = 0x01;
    		portContext->task_read_module_id.node_start_seq[1] = 0x00;
    		portContext->task_read_module_id.query_node_cnt = ROUTER_OPT_NODE_COUNT;
            readport_plc.OnPortReadData = router_send_afn_10_F7;
    		portContext->urgent_task = PLC_TASK_URGENT_TASK;
    		portContext->urgent_task_step = PLC_URGENT_TASK_READ_MODULE_ID;

            portContext->modul_id_read_flag = 0xAA; /*Ҳ���������ڲ���ִ�У������ĸ�������˾Ͳ���Ҫ�ڲ�����ɺ�ִ����*/
		}
		
		break;
	#endif
   #ifdef __INSTANT_FREEZE__
    case RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE:  //˲ʱ��������̣���������ͣ-����-��ͣ-�㲥-�ָ�

        /* ���ֱ�ӷ���  */
        portContext->cast_content = portContext->frame_cast_buffer; // portContext->frame_send
        if(2 == portContext->router_base_info.router_info1.comm_mode)/* bplc */
        {
            item_cnt = instant_freeze.item_count;
            mem_cpy(item,instant_freeze.item1,4*item_cnt);
            /*  */
            fread_ertu_params(EEADDR_FREEZE_ID, (INT8U *)&freeze_id,2);
            if(freeze_id == 0xFFFF)
            {
                freeze_id = 0;
            }
            else
            {
                freeze_id ++;
            }
            fwrite_ertu_params(EEADDR_FREEZE_ID, (INT8U *)&freeze_id,2);
            portContext->freeze_id = freeze_id;
            portContext->cast_content_len = make_bplc_instant_freeze_frame(0x0001,portContext->freeze_id,portContext->cast_content,NULL,item,item_cnt); //�㲥˲ʱ��������
            readport_plc.OnPortReadData = router_send_afn_05_F3;
            portContext->urgent_task = PLC_TASK_URGENT_TASK;
            portContext->urgent_task_step = PLC_CAST_OPEN_05_F3;
            /* flg �� clear�󷢹㲥�� ��ֹ�չ㲥��ɣ�flg�����ڣ������еı����̾ͳ����� */
            clr_readport_read_meter_flag_from_fast_index(read_meter_flag_instant_freeze.flag,COMMPORT_PLC);
        }
        else
        {
            //���÷��ͻ��������ݱ���
            readport_plc.OnPortReadData = router_send_afn_12_F1;
            portContext->urgent_task = RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE;
            portContext->urgent_task_step = PLC_URGENT_INSTANT_FREEZE_PAUSE;
        }
        /*
        portContext->cast_content = portContext->frame_send;
        portContext->cast_content_len = make_instant_freeze_frame(portContext->cast_content,NULL,0); //�㲥˲ʱ��������
        readport_plc.OnPortReadData = router_send_afn_05_F3;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_CAST_OPEN_05_F3;
        */
        break;
    #endif
    case RECMETER_TASK_TRANS_CAST_PRECISE_TIME:
        portContext->cast_content = portContext->frame_cast_buffer; //portContext->frame_send
        portContext->cast_content_len = make_gb645_adj_time_frame(portContext->cast_content,NULL,0); //�㲥��ȷ��ʱ����
        readport_plc.OnPortReadData = router_send_afn_03_F9;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_CAST_DELAY_03_F9;
        break;
       #ifdef __COUNTRY_ISRAEL__
    case RECMETER_TASK_TRANS_CAST_POWER_CONTROL:
        //���÷��ͻ��������ݱ���
        portContext->cast_content = portContext->frame_send;
        portContext->cast_content_len = make_over_power_frame(portContext->cast_content,NULL,0); //�㲥��������
        israel_ac_power_control.over_last_time = 0;           //���ͺ󣬹��ʳ����ۼ�ʱ������
        readport_plc.OnPortReadData = router_send_afn_05_F3;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_CAST_OPEN_05_F3;
        break;
       #endif
        #ifdef __SHANXI_READ_BPLC_NETWORK_INFO__
    case RECMETER_TASK_BPLC_NETWORK_INFO:
        readport_plc.OnPortReadData = router_send_afn_F0_F100;                 //�������������Ϣ
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_BPLC_NETWORK_INFO_READ;
        break;
    case BPLC_AREA_DIS_F0_F111:
        readport_plc.OnPortReadData = router_send_afn_F0_F111;                 //̨����������
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_BPLC_AREA_DIS_ALLOW;
        break;
    case BPLC_AREA_DIS_F0_F112:
        readport_plc.OnPortReadData = router_send_afn_F0_F112;                 //̨�����ֽ�ֹ
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_BPLC_AREA_DIS_FORBID;
        break;
        #endif
    #if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
    case RECMETER_TASK_SWITCH_TRIP_CTRL:
        readport_plc.OnPortReadData = router_send_afn_13_F1;                 //�¼��ϱ���ؿ�ʼ
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_SWITCH_TRIP_CTRL;
        portContext->switch_idx = 0;
        portContext->switch_trip_try_cnt = 0;
        break;
    #endif
    #if (defined __MEXICO_CIU__)
    case RECMETER_TASK_MEXICO_CIU_REPORT:
        readport_plc.OnPortReadData = router_send_afn_13_F1;                 /* CIU �¼��ϱ���ؿ�ʼ */
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        portContext->urgent_task_step = PLC_URGENT_MEXICO_CIU_REPORT;
        portContext->snd_ciu_flag = 0; /*�ȳ�������� */
        //portContext->switch_trip_try_cnt = 0;
        break;
    #endif
    /* TODO:  
     * ��ִ�е��˴�  ˵������խ�������������²ſ���ִ�е�����ġ�
     */
    #ifdef __PLC_BPLC_AGG__
    case RECMETER_TASK_GET_AGGREGATION:
        fread_array(FILEID_RUN_PARAM,FLADDR_AGGREGATION_FILE_IDX,(INT8U*)&agg_id,1);
        if(agg_id > 1)
        {
            portContext->aggregate_id = 0;
        }
        else
        {
            portContext->aggregate_id = (agg_id == 0) ? 1 : 0;
        }
        file_delete(FILEID_ROUTER_AGGREGATION_INFO+portContext->aggregate_id);
        
		if( (1 == portContext->router_base_info.router_info1.comm_mode) )/* խ�� */
		{
    		portContext->agg_ctl.plc.start_seq[0] = 0x00;
    		portContext->agg_ctl.plc.start_seq[1] = 0x00;
    		portContext->agg_ctl.plc.grp_cnt[0]   = 0x00;
    		portContext->agg_ctl.plc.grp_cnt[1]   = 0x00;
            readport_plc.OnPortReadData   = router_send_afn_F0_F11;
    		portContext->urgent_task      = PLC_TASK_URGENT_TASK;
    		portContext->urgent_task_step = PLC_URGENT_TASK_AGGREGATION;
    		
		}     
		else if(2 == portContext->router_base_info.router_info1.comm_mode) /* ��� */
		{
    		portContext->agg_ctl.bplc.node_idx[0] = 0x00;
    		portContext->agg_ctl.bplc.node_idx[1] = 0x00;
    		max_node_idx = fast_index_list.count;
    		portContext->agg_ctl.bplc.max_node_idx[0] = max_node_idx; 
    		portContext->agg_ctl.bplc.max_node_idx[1] = max_node_idx >> 8;
            if(TRUE == get_node_addr_from_fast_index(COMMPORT_PLC,&(portContext->agg_ctl)) )
    		{
                readport_plc.OnPortReadData   = router_send_afn_F0_F16;
        		portContext->urgent_task      = PLC_TASK_URGENT_TASK;
        		portContext->urgent_task_step = PLC_URGENT_TASK_AGGREGATION;
    		}
    		else
    		{
    		    portContext->urgent_task = PLC_TASK_URGENT_TASK;
    		    urgent_task_in_wait_next_urgent_task(portContext);
    		}
		}     
		else
		{
		    portContext->urgent_task = PLC_TASK_URGENT_TASK;
            //�����������ȴ���һ�����������״̬
            urgent_task_in_wait_next_urgent_task(portContext);
		}
		break;
	#endif
    default:
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        //�����������ȴ���һ�����������״̬
        urgent_task_in_wait_next_urgent_task(portContext);
        break;
    }

    return 0;
}

INT8U plc_router_process_urgent_task(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;

    if(portContext->urgent_task_id)
    {
        //if(portContext->urgent_task_step == 0)
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** PLC_TASK_URGENT_TASK - begin ***");
            debug_println_ext(info);
            #endif

            portContext->urgent_task_state.is_send_pause = 0;
            if (portContext->router_work_info.status.pause == 0)
            {
                if (portContext->cur_plc_task == PLC_TASK_READ_METER) /*�ڳ�����ִ�н�������*/
                {
                    if(portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER) //
                    {
                        portContext->urgent_task_state.is_send_pause = 1;
                    }
                }
                else if ((portContext->cur_plc_task == PLC_TASK_PLC_NET) && (portContext->cur_plc_task_step == PLC_NET_WAIT_NODE_LOGON))
                {
                    portContext->urgent_task_state.is_send_pause = 1;     /*���ѱ���ִ�н�����������������Ҳ�ᷢ��ͣ���ָ���ʱ��ֻ�Ա����ָ����ǲ�����Ҫ���ƣ�*/
                }
            }

            if (portContext->urgent_task_state.is_send_pause)
            {
                readport_plc.OnPortReadData = router_send_afn_12_F2;
                portContext->urgent_task = PLC_TASK_URGENT_TASK;
                portContext->urgent_task_step = PLC_URGENT_TASK_PAUSE_ROUTER_12_F2;
                portContext->urgent_task_pause_or_resume_times = 0;/*��ʼ��������¼��ͣ�ķ��ʹ���*/
            }
            else
            {
                exec_urgent_task(readportcontext);
            }
            return 1;
        }
        //�����쳣 zylook
    }
    return 0;
}

INT8U router_urgent_task_send_idle(objReadPortContext * readportcontext)
{
    INT32U timeout;
    //����Ƿ��˳�������
    //����Ƿ��б����
    //�����ճ�ʱ

    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;

    //�����ճ�ʱ ,ֻ���ڼ�������������·��ʱ���Ӧ��ʱ
    if(portContext->urgent_task)
    {
        if(portContext->urgent_task_id)  //��鷢�ͳ�ʱ
        {
            if(portContext->urgent_task_step == PLC_URGENT_TASK_WAIT_NEXT)
            {
                exec_urgent_task(readportcontext);
            }
            else if((time_elapsed_10ms(portContext->router_resp_time_out) > (portContext->router_base_info.max_monitor_meter_timeout_s+5)*100)
                        && ((time_elapsed_10ms(portContext->router_resp_time_out) >(portContext->plc_wait_resp_long_time_10ms) )))
            {
                //��дӦ������
                portContext->urgent_task_resp_frame_len = 0;
                portContext->plc_wait_resp_long_time_10ms = 0;
                //��ʱ �����������
                urgent_task_in_wait_next_urgent_task(portContext);
            }
            else if ((portContext->urgent_task_step == PLC_URGENT_TASK_PAUSE_ROUTER_12_F2) && (time_elapsed_10ms(portContext->router_resp_time_out) > 500))  //5s
            {
                portContext->urgent_task_pause_or_resume_times ++;
                if( portContext->urgent_task_pause_or_resume_times >= URGENT_TASK_PAUSE_RESUME_TRYR_TIME_MAX)
                {
                    exec_urgent_task(readportcontext); /*ִ�н�������*/
                }
                else
                {
                    readport_plc.OnPortReadData = router_send_afn_12_F2;
                }
            }
        }
        else   //���ȴ���һ�������������ȴ�1����
        {
            //�ȴ���һ����������ʱ���˳���������
            //·��͸��ģʽ�£��ȴ�30min;��һֱû�н��������ˣ����˳�·��͸��ģʽ
            if(gAppInfo.router_trans_mode == 1)
            {
                timeout = 108000;  //30min
            }
            else
            {
                timeout = 6000;  //60s
            }
            if(time_elapsed_10ms(portContext->tick) > timeout)   //60s
            {
                gAppInfo.router_trans_mode = 0;

                if (portContext->urgent_task_state.is_send_pause)
                {
                    if(portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER) //ֻ�б����ŷ��ָ�
                    {
                        if ((portContext->urgent_task_step == PLC_URGENT_TASK_RESUME_ROUTER_12_F3))
                        {
                            if(time_elapsed_10ms(portContext->router_resp_time_out) > 500) /*5s����һ��*/
                            {
                                portContext->urgent_task_pause_or_resume_times ++;
                                if(portContext->urgent_task_pause_or_resume_times >= URGENT_TASK_PAUSE_RESUME_TRYR_TIME_MAX)
                                {
                                    portContext->urgent_task_state.is_send_pause = 0;

                                    portContext->urgent_task = PLC_TASK_IDLE;
                                    portContext->urgent_task_step = PLC_URGENT_TASK_IDLE;

                                    readport_plc.OnPortReadData = router_reset;
                                    portContext->OnPortReadData = router_reset;
                                    portContext->cur_plc_task = PLC_TASK_IDLE;
                                    portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;

                                    record_log_code(LOG_ROUTER_RESUME_FAIL_RESET,NULL,0,LOG_ALL);/*��¼�쳣��Ϣ,��ʾ�ָ������Ӧ��������*/
                                    #ifdef __SOFT_SIMULATOR__
                                    snprintf(info,100,"*** PLC_TASK_URGENT_TASK - 12HF3_no_response***");
                                    debug_println_ext(info);
                                    #endif
                                }
                                else
                                {
                                    readport_plc.OnPortReadData = router_send_afn_12_F3;
                                }
                            }
                        }
                        else
                        {
                            portContext->urgent_task_pause_or_resume_times = 0;/*���ʹ������㣬��ʼ��¼�ָ��ķ��ʹ���*/
                            readport_plc.OnPortReadData = router_send_afn_12_F3;
                            portContext->urgent_task_step = PLC_URGENT_TASK_RESUME_ROUTER_12_F3;
                            
                        }
                    }
                    else
                    {
                        portContext->urgent_task_state.is_send_pause = 0;
                      
                        portContext->urgent_task = PLC_TASK_IDLE;
                        portContext->urgent_task_step = PLC_URGENT_TASK_IDLE;
                        #ifdef __SOFT_SIMULATOR__
                        snprintf(info,100,"*** PLC_TASK_URGENT_TASK - end ***");
                        debug_println_ext(info);
                        #endif

                        readport_plc.OnPortReadData = portContext->OnPortReadData;
                    }
                }
                else
                {
                    portContext->urgent_task = PLC_TASK_IDLE;
                    portContext->urgent_task_step = PLC_URGENT_TASK_IDLE;
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** PLC_TASK_URGENT_TASK - end ***");
                    debug_println_ext(info);
                    #endif

                    readport_plc.OnPortReadData = portContext->OnPortReadData;
                }
            }
        }
    }
    else
    {
        portContext->urgent_task = PLC_TASK_IDLE;
        portContext->urgent_task_step = PLC_URGENT_TASK_IDLE;
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** PLC_TASK_URGENT_TASK - end ***");
        debug_println_ext(info);
        #endif

        readport_plc.OnPortReadData = portContext->OnPortReadData;
    }
    return 0;
}

void check_read_cycle_set_read_task(void)
{

    #ifdef __VOLTAGE_MONITOR__
    void update_valtage_list(INT8U is_clear,INT8U rec_flag);
    #endif

    #if defined(__COMPUTE_XLOST__) || defined(__DZC_RELAY_READ__)
    INT8U value;
    #endif
    #if defined(__CJQ_ORDER_MODE__)
    INT8U cjq_value;
    #endif
    BOOLEAN has_task;
    INT8U idx;
    #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
    INT8U buffer[256]={0};
    #endif
    #ifdef __PRECISE_TIME__
    PRECISE_TIME_CAST_CTRL cast_time_ctrl;
    #endif
	#ifdef __PROVICE_JIANGXI__
	INT8U cycle[2] = {0};
	BOOLEAN flag = FALSE;
	#endif
	#if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__ )
    METER_CLASS meter_class;
    #endif
    #ifdef __CHONGQING_LONGHU__
    INT8U f103_param[4] = {0};// ���� �ܶȵ�λ �ܶ���ֵ ���߸��� 
    BOOLEAN set_flag = FALSE;
	#endif
    BOOLEAN delay_set_cycle_flag = FALSE;
    INT8U hour_minute[2] = {0};
	
    has_task = FALSE;
    #ifdef __PROVICE_JIANGXI__
    if(TRUE == check_is_all_ch(read_meter_flag_cycle_day.cycle_day,3,0x00))
    {
        //
        flag = TRUE;
    }
	#endif

    #if defined (__PROVICE_BEIJING__) && defined (__POWER_CTRL__)
    if((datetime[HOUR] == 0) && (datetime[MINUTE] >= 5))
    #endif
    {
    if(compare_string(read_meter_flag_cycle_day.cycle_day,datetime+DAY,3) != 0)
    {
        #if( ( defined __PROVICE_GUANGXI_PB__ ) || ( defined __ALL_MONTH_DATA_FROM_JSR_DATA__ ) )
        if(compare_string(read_meter_flag_cycle_day.cycle_day+1,datetime+MONTH,2) != 0)
        {
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_month,COMMPORT_485_CAS);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_month,COMMPORT_485_REC);
            
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** �¶��᳭�����񣬹����� ***");
            debug_println(info);
            #endif
        }
        #endif

        //������������־�ָ�λ��
        #if defined(__COMPUTE_XLOST__)
        value = 0;
        fwrite_array(FILEID_RUN_DATA, FLADDR_XLOST_CAL_FLAG, &value, sizeof(tagXLOSTCALFLAG));
        #endif

        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_PLC);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_last_curve_cycle_day.flag,COMMPORT_PLC);
        #ifdef __SICHUAN_FK_PATCH_CURVE_DATA__
//        set_readport_read_meter_flag_from_fast_index(read_meter_flag_last_curve_cycle_day.flag,COMMPORT_485_CAS);
//        set_readport_read_meter_flag_from_fast_index(read_meter_flag_last_curve_cycle_day.flag,COMMPORT_485_REC);
		#endif
        #ifdef __METER_DAY_FREEZE_EVENT__
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_day_freeze_event,COMMPORT_PLC);
		#endif
        //#ifdef __ITEM_PRIORITY__
        for(idx=0;idx<ITEM_PRIORITY_CYCLE_DAY_COUNT;idx++)
        {
            set_readport_read_meter_flag_from_fast_index(read_priority_ctrl_item_cycle_day[idx],COMMPORT_PLC);
        }
        //#endif

        if(compare_string(read_meter_flag_cycle_day.cycle_day+1,datetime+MONTH,2) != 0)
        {
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_month,COMMPORT_PLC);
            #ifdef __CHECK_MONTH_HOLD_TD__
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_month,COMMPORT_485_CAS);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_month,COMMPORT_485_REC);
            #endif
        }
       #ifdef __PRECISE_TIME__
       fwrite_ertu_params(EEADDR_PRECISE_CAST_PARAM,cast_time_ctrl.value ,sizeof(PRECISE_TIME_CAST_CTRL));
       #endif
//         #ifndef __POWER_CTRL__ 
//        if(check_const_ertu_switch(CONST_ERTU_SWITCH_485_READ_DAYHOLD_ITEM))
//        {
//        //set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CY);
//        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CAS);
//        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_REC);
//        }
//        #endif
//         
//         #ifdef __PROVICE_SHANGHAI_FK__      //2015-12-15   �Ϻ�
//        if(check_const_ertu_switch(CONST_ERTU_SWITCH_485_READ_DAYHOLD_ITEM))
//        {
//        //set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CY);
//        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CAS);
//        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_REC);
//        }
//        #endif
        
        if(check_const_ertu_switch(CONST_ERTU_SWITCH_485_READ_DAYHOLD_ITEM))
        {
        //set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CY);
            
        #ifdef __PROVICE_SHANGHAI__
        /*
         * �Ϻ�������Ҫ�� 0:10���Ӻ�ſɳ����ն���  
         */
        tpos_enterCriticalSection();
        mem_cpy(hour_minute,datetime+MINUTE,2);
        tpos_leaveCriticalSection();
        if( (0x00 == hour_minute[1]) && (hour_minute[0] < 10) )
        {
            delay_set_cycle_flag = TRUE;
        }
        else
        {
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CAS);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_REC);
            delay_set_cycle_flag = FALSE;
        }
        
        #else
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CAS);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_REC);
        #endif
        }
        
        if(!delay_set_cycle_flag)
        {
            mem_cpy(read_meter_flag_cycle_day.cycle_day,datetime+DAY,3);
        }
        
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** �ն��᳭�����񣬹����� ***");
        debug_println(info);
        #endif

        if(check_const_ertu_switch(CONST_ERTU_SWITCH_PATCH_DAYHOLD_DATA))
        {
            //�����ն���
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_patch_day_hold,COMMPORT_PLC);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_patch_day_hold,COMMPORT_485_CY);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_patch_day_hold,COMMPORT_485_CAS);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_patch_day_hold,COMMPORT_485_REC);
            //�����ն������ȼ�����
            set_readport_read_meter_flag_from_fast_index(read_priority_ctrl_patch_day_hold,COMMPORT_PLC);
        }
        //��ǰ
       // #ifndef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
	   //	#endif
		set_readport_read_meter_flag_from_fast_index(read_priority_ctrl_cur_data,COMMPORT_PLC);


#ifdef __ENABLE_ESAM2__
        //esam
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_esam,COMMPORT_485_REC);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_esam,COMMPORT_485_CAS);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_esam,COMMPORT_PLC);
#endif
        #ifdef __PROVICE_JIANGXI__
    	//�����ϵ磬���볭��ʱ�Σ������¼�����������1�죬Ӧ�����,���������������������ᵼ�¶೭��һ�� TODO ????
    	fread_ertu_params(EEADDR_SET_F107+2,cycle,2);
		//��һ�μӵ��ʱ��ȫ00 ��Ҫ��������
		if(TRUE == flag)
		{
        	if((cycle[1] == 3) && (cycle[0] == 1) ) //��������Ϊ1��
            {
                set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[1],COMMPORT_PLC);
                //˵�������³�������Ҫ�����ʷ�ĳ������ã���ֹ�����ϴ�δ������ɵĿ�ʼ����
				file_delete(FILEID_METER_GRADE_RECORD_MASK);//һ����������
		        file_delete(FILEID_EVENT_GRADE_READ_ITEM_CTRL);// ����������������
        	}
		}
    	#endif
		
        has_task = TRUE;

        portContext_plc.router_phase_work_info[0].read_params.is_day_changed = 1;
        portContext_plc.router_phase_work_info[1].read_params.is_day_changed = 1;
        portContext_plc.router_phase_work_info[2].read_params.is_day_changed = 1;
        portContext_plc.router_phase_work_info[3].read_params.is_day_changed = 1;
        #ifdef __CJQ_ORDER_MODE__
        //ͬ���ɼ�����Ϣ �����ڵ�һ���ز���ʱ�ļ�����
        fread_array(FILEID_PLC_REC_TMP,PIM_CJQ_READ_INFO,(INT8U*)cjq_read_info,MAX_READ_INFO_CNT*(sizeof(CJQ_READ_INFO)));
        for(cjq_value=0;cjq_value<MAX_READ_INFO_CNT;cjq_value++)
        {
            cjq_read_info[cjq_value].meter_seq = 0x00;
            cjq_read_info[cjq_value].comm_ok = 0x00;
        }
        fwrite_array(FILEID_PLC_REC_TMP,PIM_CJQ_READ_INFO,(INT8U*)cjq_read_info,MAX_READ_INFO_CNT*(sizeof(CJQ_READ_INFO)));
        #endif
        #ifdef __DZC_RELAY_READ__
        fread_array(FILEID_PLC_REC_TMP,PIM_DZC_METER_READ_START_HOUR,&value,1);
        if(datetime[HOUR] < value) sqr_delay_read_reset_router = 1;
        #endif
    }
    }

    //����
    if ((datetime[MINUTE] == 0) && (compare_string(read_meter_flag_curve.cycle_60_minute,datetime+MINUTE,5) != 0))
    {
        #if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__ )
        if( (datetime[HOUR] >= 8) && (datetime[HOUR] < 20) && ((datetime[HOUR]-4)%4 == 0) ) 
        {
            //���ó�����ǰ����
            meter_class.meter_class = 0;
            meter_class.user_class = 14;//����� 14 TODO ???
            set_readport_specific_meter_class_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC,meter_class);
        }
        #endif
        
        #ifdef __FUJIAN_CURRENT_BREAK__
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
        #endif
        #ifdef __INDONESIA_DLMS_TEST__ //ӡ��ĵ��14��10���Ժ�����һ�γ����¶�������
        if((datetime[DAY] == 14) && (datetime[HOUR] == 10))
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_month,COMMPORT_PLC);
        //6Сʱ����һ�γ���ʵʱ���ݣ�Ҳ����ʱ���¼��Ϳ�����¼���0�㲻��
        if(((datetime[HOUR]%6) == 0) && (datetime[HOUR] != 0))
        {
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
        }
        #endif
        #ifdef __CHONGQING_LONGHU__
        fread_ertu_params(EEADDR_SET_F103,f103_param,4);
		//������ͬʱ �ܶ�2 Сʱ ��ͬʱ ��ֵ��Ч
		if( (f103_param[2] == 0) || (f103_param[2] >= 24) )
		{
		    f103_param[2] = 24;
		}
		set_flag = FALSE;
		if( (1 == f103_param[0]) && (2 == f103_param[1]) && (f103_param[2]< 24) )
		{
		    if( (datetime[HOUR]%f103_param[2]) == 0 ) //&& (0 != datetime[HOUR])
		    {
		        set_flag = TRUE;
		    }
		}
		else
		{
		    set_flag = TRUE;
		}
		if(TRUE == set_flag)
		#endif
		{
            //set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_PLC);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_CY);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_CAS);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_REC);
            #if defined (__SGRID_HARDWARE__) || defined(__NGRID_HARDWARE_II__) || defined (__ZHEJIANG_TTU__)
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_MIN);
            #endif
            mem_cpy(read_meter_flag_curve.cycle_60_minute,datetime+MINUTE,5);
            mem_cpy(read_meter_flag_curve.cycle_30_minute,datetime+MINUTE,5);
            mem_cpy(read_meter_flag_curve.cycle_15_minute,datetime+MINUTE,5);
            mem_cpy(read_meter_flag_curve.cycle_05_minute,datetime+MINUTE,5);
            #ifdef __NGRID_HARDWARE_II__
            get_cy_voltage();
            #endif
            tpos_enterCriticalSection();
            portContext_plc.router_phase_work_info[0].read_params.span_curve_flag = 1;
            portContext_plc.router_phase_work_info[1].read_params.span_curve_flag = 1;
            portContext_plc.router_phase_work_info[2].read_params.span_curve_flag = 1;
            portContext_plc.router_phase_work_info[3].read_params.span_curve_flag = 1;
            tpos_leaveCriticalSection();
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_last_curve_cycle_day.flag,COMMPORT_PLC);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** 60�������ڣ������� ***");
            debug_println(info);
            #endif
            #ifdef __HOUR_CURVE_READ_SELF_ADAPTION__
            portContext_plc.active_read_curve_one_cycle = 1;/* clear */
            #endif
            has_task = TRUE;
        }

        #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
        if((portContext_plc.router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)
         && (portContext_plc.vip_meter_read.vip_cmd_11F8 == 1))  //���ŷ�����֧��,���в����
        {
        fread_ertu_params(EEADDR_SET_F35,buffer,256);
        bit_value_opt_inversion(buffer,256);
        bit_value_opt_or(priority_node.map,buffer,256);
        }
        #endif
      

        #ifdef __VOLTAGE_MONITOR__
        //�����ص��ѹ���Ӳ�����ĳ�������
        if((datetime[HOUR] % VIP_V_MONITOR_CYCLE) == 0)
        {
            if (!get_system_flag(SYS_VOLTAGE_MONITOR,SYS_FLAG_BASE))
            {
                update_valtage_list(2,1);
            }
        }
        #endif

        #ifdef __BATCH_TRANSPARENT_METER_CYCLE_TASK__
        #ifdef __PROVICE_JIBEI__
        /* ��ʱֻ�ܼ򵥴��� �޷�ͨ�ô��� */
        if( (datetime[HOUR] >= 7) && (datetime[HOUR] < 23) && ((datetime[HOUR]-7)%5 == 0) )
        {
            if(portContext_plc.router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER)
            {
                portContext_plc.need_reset_router = 1;      /*����ģʽ����·��*/
            }
            else
            {
                //portContext_plc.need_reset_router = 3;    /*�����Ͳ���ģʽ�����³���*/
            }
        }
        #endif
        #endif

        #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
        #ifdef __GW_CYCLE_TASK__
        if(portContext_plc.router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)
        {
            priority_node.flag = 0xFF;/* invalid */
            mem_set(priority_node.map,sizeof(priority_node.map),0x00);
            creat_priority_map_jibei();
            priority_node.flag = 0x00;
        }
        #endif
        #endif
        
    }
    else if (((datetime[MINUTE] % 30) == 0) && (compare_string(read_meter_flag_curve.cycle_30_minute,datetime+MINUTE,5) != 0))
    {
        //set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_PLC);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_CY);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_CAS);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_REC);
        #if defined (__SGRID_HARDWARE__) || defined(__NGRID_HARDWARE_II__) || defined (__ZHEJIANG_TTU__)
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_MIN);
        #endif
        mem_cpy(read_meter_flag_curve.cycle_30_minute,datetime+MINUTE,5);
        mem_cpy(read_meter_flag_curve.cycle_15_minute,datetime+MINUTE,5);
        mem_cpy(read_meter_flag_curve.cycle_05_minute,datetime+MINUTE,5);
        #ifdef __NGRID_HARDWARE_II__
        get_cy_voltage();
        #endif
        tpos_enterCriticalSection();
        portContext_plc.router_phase_work_info[0].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[1].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[2].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[3].read_params.span_curve_flag = 1;
        tpos_leaveCriticalSection();
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_last_curve_cycle_day.flag,COMMPORT_PLC);        
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** 30�������ڣ������� ***");
        debug_println(info);
        #endif
        #ifdef __HOUR_CURVE_READ_SELF_ADAPTION__
        portContext_plc.active_read_curve_one_cycle = 1;/* clear */
        #endif

        has_task = TRUE;
    }
    else if (((datetime[MINUTE] % 15) == 0) && (compare_string(read_meter_flag_curve.cycle_15_minute,datetime+MINUTE,5) != 0))
    {
        #ifdef __PROVICE_JIANGXI__/*������Ҫ15�����ϱ�F25��ʵʱ����*/
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
        #endif
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_PLC);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_CY);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_CAS);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_REC);
        #if defined (__SGRID_HARDWARE__) || defined(__NGRID_HARDWARE_II__) || defined (__ZHEJIANG_TTU__)
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_MIN);
        #endif
        mem_cpy(read_meter_flag_curve.cycle_15_minute,datetime+MINUTE,5);
        mem_cpy(read_meter_flag_curve.cycle_05_minute,datetime+MINUTE,5);
        #ifdef __NGRID_HARDWARE_II__
        get_cy_voltage();
        #endif
        tpos_enterCriticalSection();
        portContext_plc.router_phase_work_info[0].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[1].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[2].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[3].read_params.span_curve_flag = 1;
        tpos_leaveCriticalSection();
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_last_curve_cycle_day.flag,COMMPORT_PLC);
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** 15�������ڣ������� ***");
        debug_println(info);
        #endif
        #ifdef __HOUR_CURVE_READ_SELF_ADAPTION__
        portContext_plc.active_read_curve_one_cycle = 1;/* clear */
        #endif

        has_task = TRUE;
    }
    else if (((datetime[MINUTE] % 5) == 0) && (compare_string(read_meter_flag_curve.cycle_05_minute,datetime+MINUTE,5) != 0))
    {
        //set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
        #ifndef __DLMS_AUTO_SET__//5���Ӳ�Ҫȥ��¼
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_PLC);
        #endif
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_CY);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_CAS);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_REC);
        #if defined (__SGRID_HARDWARE__) || defined(__NGRID_HARDWARE_II__) || defined (__ZHEJIANG_TTU__)
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve.flag,COMMPORT_485_MIN);
        #endif
        mem_cpy(read_meter_flag_curve.cycle_05_minute,datetime+MINUTE,5);
        #ifdef __NGRID_HARDWARE_II__
        get_cy_voltage();
        #endif
        tpos_enterCriticalSection();
        portContext_plc.router_phase_work_info[0].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[1].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[2].read_params.span_curve_flag = 1;
        portContext_plc.router_phase_work_info[3].read_params.span_curve_flag = 1;
        tpos_leaveCriticalSection();
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_last_curve_cycle_day.flag,COMMPORT_PLC);
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** 5�������ڣ������� ***");
        debug_println(info);
        #endif
        has_task = TRUE;
    }

    if (has_task)
    {
        //����ģʽ�£���������Ϣ
        if ((portContext_plc.cur_plc_task == PLC_TASK_READ_METER) && (portContext_plc.cur_plc_task_step == PLC_READ_METER_SLEEP))
        {
            portContext_plc.read_status.is_jzq_read_cycle = 1;
        }
    }
}
INT8U check_jzq_read_cycle(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;
    if(portContext->router_interactive_status.meter_doc_synchro_done == 0)
    {
        return FALSE;/*����û�бȶ���ɵ�ʱ�򣬲�Ҫ����*/
    }
    if (portContext->read_status.is_jzq_read_cycle == 1)
    {
        portContext->read_status.is_jzq_read_cycle = 0;
        
        if ((portContext->cur_plc_task == PLC_TASK_READ_METER) && (portContext->cur_plc_task_step == PLC_READ_METER_SLEEP))
        {


            portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
            portContext->OnPortReadData = get_read_meter_info;
            readport_plc.OnPortReadData = router_check_urgent_timeout;

            portContext->params.task_read_data.node_idx[0] = 0;
            portContext->params.task_read_data.node_idx[1] = 0;
            portContext->params.task_read_data.has_fail_meter = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}
#ifdef __SOFT_SIMULATOR__
#define __DEBUG_RECINFO__
#endif
//������¼������ڼ���������
void check_meter_event_cycle(void)
{
	//INT8U 	*meter_event_read_flag = NULL;
    INT16U cycle_minute_num;
    INT16U minute_num;
    INT16U 	idx;//,idx1;
    //INT8U 	num;
    //INT8U	tmp_val;
    INT8U cycle[MAX_METER_EVENT_LEVEL*2] = {0};
	BOOLEAN flag_delete = FALSE;
	INT8U	tmp_flag,event_level_mask_flag;
	#ifdef __PROVICE_HUNAN__
    PARAM_F106 tmp_f106;
    #endif
     minute_num = datetime[HOUR]*60UL + datetime[MINUTE];
    fread_ertu_params(EEADDR_SET_F107,cycle,MAX_METER_EVENT_LEVEL*2);

#ifdef __PROVICE_HUNAN__
    fread_array(FILEID_METER_EVENT_PARAM,PIM_PARAM_F106,tmp_f106.value ,sizeof(PARAM_F106));//ֻ����1���¼�������offsetû�м�������λ��
    if((cycle[0]==0xFF) && (cycle[1] ==0xFF) && (tmp_f106.level == 1))
    {
    cycle[0] =2;
    cycle[1] =1;    //�����1���¼�������F107û�����ó������ڣ�Ĭ��2���ӳ���
    fwrite_ertu_params(EEADDR_SET_F107,cycle,MAX_METER_EVENT_LEVEL*2);
    }
#endif
	tmp_flag = event_level_mask_ctrl.event_level_mask_flag;
	event_level_mask_flag = 0;
    for(idx=0;idx<MAX_METER_EVENT_LEVEL;idx++)
    {
        if(cycle[idx*2+1] == 2) //Сʱ
        {
            if(cycle[idx*2] < 24)
            {
                cycle_minute_num = cycle[idx*2]*60;
				//���������㣬Ϊ0��ִ��,����ᵼ���쳣����
				if(cycle_minute_num == 0)
				{
					continue;
				}
                if((minute_num % cycle_minute_num) == 0)
                {
                    file_delete(FILEID_EVENT_GRADE_READ_FALG+idx);
					flag_delete = TRUE;	            

					set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_CY);
					set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_REC);
					set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_CAS);
                 #ifdef __SUCCESS_RATE_OVER_95__
                if(((portContext_plc.success_rate >= 95) && (portContext_plc.success_rate != 0xFF))
                    || (datetime[HOUR] > 0x14) || (portContext_plc.success_rate == 1))
                #endif
					set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_PLC);	
									
					event_level_mask_flag |=(INT8U)(1<<idx);
					meter_event_cycle_cnt[idx]++;
                    #ifdef __DEBUG_RECINFO__
                    snprintf(info,100,"****���õ���¼���������!  idx = %d ",idx);
                    println_info(info);
                    #endif
                }
            }
        }

        if(cycle[idx*2+1] == 1) //����
        {
            cycle_minute_num = cycle[idx*2];
//           portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.control.loop =1;
            //���������㣬Ϊ0��ִ��,����ᵼ���쳣����
			if(cycle_minute_num == 0)
			{
				continue;
			}
			if((minute_num % cycle_minute_num) == 0)
            {
                file_delete(FILEID_EVENT_GRADE_READ_FALG+idx);
				flag_delete = TRUE;
				
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_CY);
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_REC);
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_CAS);
                 #ifdef __SUCCESS_RATE_OVER_95__
                if(((portContext_plc.success_rate >= 95) && (portContext_plc.success_rate != 0xFF))
                   || (datetime[HOUR] > 0x14) || (portContext_plc.success_rate == 1))
                #endif
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_PLC);	
				
				event_level_mask_flag |=(INT8U)(1<<idx);
//                read_params.control.loop = 0;
				meter_event_cycle_cnt[idx]++;
                #ifdef __DEBUG_RECINFO__
                snprintf(info,100,"****���õ���¼���������!  idx = %d ",idx);
                println_info(info);
                #endif
            }
        }

        if(cycle[idx*2+1] == 3) //��
        {
            cycle_minute_num = cycle[idx*2];
			//���������㣬Ϊ0��ִ��,����ᵼ���쳣����
			if(cycle_minute_num == 0)
			{
				continue;
			}
            if ((minute_num == 0) && ((datetime[DAY] % cycle_minute_num) == 0))
            {
                file_delete(FILEID_EVENT_GRADE_READ_FALG+idx);
				flag_delete = TRUE;
				
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_CY);
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_REC);
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_CAS);
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_PLC);	
				
				event_level_mask_flag |=(INT8U)(1<<idx);	
				meter_event_cycle_cnt[idx]++;
                #ifdef __DEBUG_RECINFO__
                snprintf(info,100,"****���õ���¼���������!  idx = %d ",idx);
                println_info(info);
                #endif
            }
        }
        if(cycle[idx*2+1] == 4) //��
        {
            cycle_minute_num = cycle[idx*2];
			//���������㣬Ϊ0��ִ��,����ᵼ���쳣����
			if(cycle_minute_num == 0)
			{
				continue;
			}
            if((minute_num == 0) && (datetime[DAY] == 1) && ((datetime[MONTH] % cycle_minute_num) == 0))
            {
                file_delete(FILEID_EVENT_GRADE_READ_FALG+idx);
				flag_delete = TRUE;
				
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_CY);
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_REC);
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_485_CAS);
				set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[idx],COMMPORT_PLC);
				
				event_level_mask_flag |=(INT8U)(1<<idx);
				meter_event_cycle_cnt[idx]++;
                #ifdef __DEBUG_RECINFO__
                snprintf(info,100,"****���õ���¼���������!  idx = %d ",idx);
                println_info(info);
                #endif
            }
        }
    }
    
	if(flag_delete == TRUE)
	{
		file_delete(FILEID_METER_GRADE_RECORD_MASK);//һ����������
		file_delete(FILEID_EVENT_GRADE_READ_ITEM_CTRL);// ����������������
	}
	if( (tmp_flag != event_level_mask_flag) && (event_level_mask_flag !=0) )
	{
		event_level_mask_ctrl.event_level_mask_flag = event_level_mask_flag;
	}
	#ifdef __DEBUG_RECINFO__
    snprintf(info,100,"****�ȼ���������,level_mask_flag = 0x%02X ",event_level_mask_ctrl.event_level_mask_flag);
    debug_println_ext(info);
    #endif
	
}

INT8U router_wait_resp_frame(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U test[26] = {0};
    static INT8U day = 0;
    portContext = (PLCPortContext*)readportcontext;
    //����������ʱ���Ӧ��ʱ�ط�

    if ((portContext->cur_plc_task == PLC_TASK_READ_METER) && (portContext->cur_plc_task_step == PLC_READ_METER_WAIT_RESP_13F1))
    {
        if(time_elapsed_10ms(portContext->router_resp_time_out) > (portContext->router_base_info.max_monitor_meter_timeout_s+5)*100)
        {
            //����
            portContext->cur_plc_task = PLC_TASK_READ_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
            portContext->OnPortReadData = get_read_meter_info;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
        }
    }
    else if ((portContext->cur_plc_task == PLC_TASK_READ_VIP_METER)&& (portContext->cur_plc_task_step == PLC_READ_METER_WAIT_RESP_13F1))
    {
        if(time_elapsed_10ms(portContext->router_resp_time_out) > (portContext->router_base_info.max_monitor_meter_timeout_s+5)*100)
        {
            //����
            portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
            portContext->OnPortReadData = get_read_vip_meter_info;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
        }
    }
    else if(portContext->cur_plc_task == PLC_TASK_CLERA_AREA_DIS)
    {
        if(time_elapsed_10ms(portContext->router_resp_time_out) > 120*100) //120��
        {
            //��ʼ��·��
            portContext->cur_plc_task_step = PLC_CLEAR_AREA_DIS_ROUTER_INIT;
            portContext->OnPortReadData = router_send_afn_01_F2;
            readport_plc.OnPortReadData = router_send_afn_01_F2;
        }

    }
    else
    {
        if(time_elapsed_10ms(portContext->router_resp_time_out) > 500)   /*���ִ�е����˵��5sû���յ�·��������Ӧ*/
        {

            portContext->repeated_sending_times ++ ;

            if(portContext->repeated_sending_times > 2)
            {
                switch(portContext->cur_plc_task)
                {
                 case PLC_TASK_QUERY_VERSION:
                      switch(portContext->cur_plc_task_step)
                      {
                         #ifdef __READ_MODULE_ID_PLAN__
                        case PLC_QUERY_VERSION_QUERY_03F12: /*խ����ѯģ��ID�����̨�岻��Ӧ��ֱ��������*/
                             portContext->task_read_module_id.node_start_seq[0] = 0x01;
                             portContext->task_read_module_id.node_start_seq[1] = 0x00;
                             portContext->task_read_module_id.query_node_cnt = 1;
                             portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_10F112_MAIN_NODE;
                             readport_plc.OnPortReadData = router_wait_send_frame_complete;
                             portContext->OnPortReadData = router_send_afn_10_F112;
                             break;
                        #endif
                        default:
                            break;
                      }
                      break;
                 case PLC_TASK_PLC_NET:
                      switch(portContext->cur_plc_task_step)
                      {
                        case PLC_CAST_OPEN_05_F3:/*���͹㲥�ɼ����ѱ��������Ӧ�����������ϱ�������������·�ɲ���Ӧ�����²����������*/
                             if((portContext_plc.router_base_info.router_info1.comm_mode == 2) && (portContext_plc.hplc_area_distinguish != 0))
                             {
                                 active_node_distinguish(portContext); /*�����05HF6����*/
                             }
                             else
                             {
                                  active_node_logon(portContext);
                             }
                            break;
                        default:
                            break;
                      }
                      break;
                 default:
                      break;
                }
            }
            /*��������־��¼�Ĵ���*/
            if(day == 0)/*Ϊ0��ʱ��˵���ǵ�һ�ν����ۼƣ���ֵ��������ݡ������������ͣ���������Ȳ�����*/
            {
                day = datetime[DAY];
                portContext->plc_abnormal_log_time = 0;
            }
            portContext->plc_abnormal_log_time ++;

            if(portContext->plc_abnormal_log_time > PLC_ABNOMAL_LOG_MAX)/*һ���¼���50�Σ���־��ѭ���ģ���ֹ����������־��Ϣ*/
            {
                portContext->plc_abnormal_log_time = PLC_ABNOMAL_LOG_MAX;/*��ֹ����255�󣬱��0�ٴ�ִ��*/

                if(day != datetime[DAY])/*ֻ���ս����жϣ������ʱ���µ��ղ��䣬�º��겻ͬ���ݲ�������*/
                {
                    day = datetime[DAY];
                    portContext->plc_abnormal_log_time = 0;/*���վ�Ҫ����*/
                }
            }
            else
            {
                
                test[0] = portContext->repeated_sending_times;
                if(readport_plc.context->frame_send != NULL)
                {
                    mem_cpy(test+1,readport_plc.context->frame_send,25);
                }
                record_log_code(LOG_ROUTER_REPEAT_RESET,test,26,LOG_ALL);/*��¼�쳣��Ϣ��26�ֽ����ټ�¼��fn*/
            }/*����ֻ�Ƕ���־��¼�Ĵ���*/
            
            if(portContext->repeated_sending_times > REPEAT_TIME_MAX)
            {
                    readport_plc.OnPortReadData = router_reset;
                    portContext->OnPortReadData = router_reset;
                    portContext->cur_plc_task = PLC_TASK_IDLE;
                    portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;  /*һ������ʱ���ڣ��ۼƵ�20���쳣�ط����ж�·�ɿ����쳣�߷���ֱ�����ܽţ���ͷ��ʼ������*/
            }
            else
            {
                if(portContext->OnPortReadData)
                {
                    readport_plc.OnPortReadData = portContext->OnPortReadData;
                }
            }

        }
    }
    return 0;
}

INT8U router_check_urgent_timeout(objReadPortContext * readportcontext)
{
    //����Ƿ��˳�������
    //����Ƿ��б����
    //�����ճ�ʱ

    PLCPortContext *portContext;
    static INT8U last_minute[5] = {0xFF};
    static INT8U last_second = 0xFF;
    static INT8U last_day    = 0xFF;
	//INT8U	flag;
//    #ifdef __SOFT_SIMULATOR__
//    snprintf(info,100,"*** router_check_urgent_timeout ...");
//    debug_println_ext(info);
//    #endif

    portContext = (PLCPortContext*)readportcontext;

    //��·��͸��ģʽ�£������޽������ĵļ��
    if(gAppInfo.router_trans_mode != 1)
    {
        if(check_plc_reset_router(portContext)) return 0; //���·���Ƿ���Ҫ������
    }

    #ifdef __FUJIAN_SUPPLEMENT_SPECIFICATION__
    //if(check_forecast_task_update(portContext)) return 0;
    if(portContext->fujian_ctrl.protocol_type == PROTOCOL_FUJIAN)
    {
        
        //��������ģʽ�£���Ҫ����ĳЩ����
        if(exec_task_exec_cmd(portContext)) return 0;      
        // ����������񣬲��͹���ʹ��һ����������������
        if (plc_router_process_urgent_task_fujian(readportcontext)) return 0;
        //���㲥����
        if(check_broadcast_task(readportcontext)) return 0;
        if(check_plc_node_reg(readportcontext) ) return 0;
			
        // ���ѱ�����У��ż�鷽���Ƿ���Ҫִ��
        if (portContext->cur_plc_task != PLC_TASK_PLC_NET)
        {
            
            //if(compare_string(last_minute,datetime+MINUTE,5) != 0)
            mem_cpy(last_minute,datetime+MINUTE,5);
			//�ѱ���
            timer_plc_node_reg(); 
			//��ⷽ���Ƿ���Ҫִ�У����ڽ����������ȽϺ� ?? �ѱ�����У���Ҫȥ����MARK  ?????
            if(check_forecast_task_update(readportcontext,READPORT_PLC)) return 0;
        }
        else
        {
            // 
            if (portContext->cur_plc_task_step == PLC_NET_WAIT_NODE_REGISTER)
            {
                if (((portContext->plc_net_wait_time_s > 0) && (time_elapsed_10ms(portContext->plc_net_time_out_10ms) > portContext->plc_net_wait_time_s*100)))
                {
                    portContext->params.task_plc_net.run_params.is_force_exit = 0;
                    //ʱ�䵽�ˣ������ѱ�
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** ʱ�䵽�������ѱ��ܽ����������ѱ�ͳ�ƣ�***");
                    debug_println_ext(info);
                    #endif
                    //�����ѱ�ͳ��
                    portContext->cur_plc_task = PLC_TASK_PLC_NET;
                    portContext->cur_plc_task_step = PLC_NET_STAT_NODE_INFO;
                    portContext->OnPortReadData = start_plc_node_reg_stat;
                    readport_plc.OnPortReadData = start_plc_node_reg_stat;
                    return 0;
                }             
            }
			if(portContext->params.task_plc_net.run_params.is_force_exit)
			{
			    portContext->params.task_plc_net.run_params.is_force_exit = 0;
				portContext->cur_plc_task = PLC_TASK_PLC_NET;
                portContext->cur_plc_task_step = PLC_NET_SET_55_F7_FORECE_EXIT;
                portContext->OnPortReadData = router_send_afn_55_F7;
                readport_plc.OnPortReadData = router_send_afn_55_F7;
			}
        }
        
        //if(check_plc_parall_read(readportcontext)) return 0; /*���ò���*/

    }
	else
    #endif
    {
        //���ִ��·�ɽ�������Ȩ�޸�
        if (plc_router_process_urgent_task(readportcontext)) return 0;
        
        //·��͸��ģʽ�£�ֻ����·�ɽ�������ļ�飬��Ϊת��376.2�����ڽ���������ִ��
        if(gAppInfo.router_trans_mode == 1)
        {
            return 0;
        }
    
        if (check_plc_net(readportcontext)) return 0;
    
        if (exec_ctrl_cmd(portContext)) return 0;
    
        if (check_jzq_read_cycle(readportcontext)) return 0;
    
        if (check_plc_distinguish(readportcontext)) return 0;

        #if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
        if(check_ctrl_switch_trip(readportcontext))
        {
            return 0;
        }
        #endif
        #ifdef __PLC_BPLC_AGG__
        if (check_plc_aggregation_task(readportcontext)) return 0;
        #endif

        #ifdef __INSTANT_FREEZE__
        if( 0xAA == portContext->bplc_instant_freeze_flg )
        {
            /* ��� 5����֮��ſ��Դ��־ ���� ����ʵ��ʱ���������ǽű����� ???  */
            #ifdef __SOFT_SIMULATOR__
            if( time_elapsed_10ms(portContext->cast_bplc_insfrz_time_10ms) >= 30*100 )
            #else
            if( time_elapsed_10ms(portContext->cast_bplc_insfrz_time_10ms) >= 300*100 )
            #endif
            {
                portContext->bplc_instant_freeze_flg = 0;
                set_readport_read_meter_flag_from_fast_index(read_meter_flag_instant_freeze.flag,COMMPORT_PLC);
            }
        }
        #endif
        if (portContext->router_work_info.status.pause)
        {
         #ifdef __QGDW_CHECK__
         if((last_day != datetime[DAY]) && (last_day != 0xFF))
         portContext_plc.ctrl_cmd.cmd_resume = 1;
         last_day = datetime[DAY];
         #else
         if((last_day != datetime[HOUR]) && (last_day != 0xFF)) /*ʵ��ʹ��ʱ����ͣ���������ͣ1Сʱ*/
         portContext_plc.ctrl_cmd.cmd_resume = 1;
         last_day = datetime[HOUR];
         #endif
         return 0;
        }
    
    	#if(defined __PROVICE_CHONGQING__)
    	INT8U	flag;
        if(COMMPORT_PLC_REC <= COUNT_OF_READPORT)
        {
            tpos_enterCriticalSection();
            flag = llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.c2f55_stat_flag;
            tpos_leaveCriticalSection();
            if(flag == 1)//������������˳���ʱ�䣬����·���ϱ�
            {
                //ͳ����Ϣ		
                update_llvc_rec_state();
                // �м���Ϣͳ�� 
                plc_router_relay_stat();
                llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.c2f55_stat_flag = 0;
            }
        }
    	#endif
        //���ѱ�����У���ִ���볭����صĻ
        if (portContext->cur_plc_task != PLC_TASK_PLC_NET)
        {
            #ifdef __FUJIAN_SUPPLEMENT_SPECIFICATION__
            if(portContext->fujian_ctrl.protocol_type != PROTOCOL_FUJIAN)// �߸���Э�� ��ִ��??
            #endif
            {
                //����ÿ�ν�����Ҫ����ÿ���Ӵ���һ��
                if(compare_string(last_minute,datetime+MINUTE,5) != 0)
                {
                    mem_cpy(last_minute,datetime+MINUTE,5);
                    //�����������߳�������  --------------���������Ƿ���485������ ���Ƿֶ˿ڽ���  zylook
                    check_read_cycle_set_read_task();
                    #ifdef __VIP_METER_13HF1_READ__
                    plc_router_vipmeter_recording(readportcontext);
                    #endif
                    //����¼�������
                    //check_meter_event_cycle();
                    //����ͳ��
                    stat_llvc_rec_state();
                    //��ʱ�����ѱ�
                    timer_plc_net();
        
                //    meter_cast_timing(readportcontext,COMMPORT_PLC_REC);
                }
        
                if(last_second != datetime[SECOND])
                {
                    last_second = datetime[SECOND];
                    #ifdef __PROVICE_CHONGQING__
                    if ( ((last_second % 10) == 0) || (portContext->ajust_time) )  //10s���һ��
                    #else
                    if ((last_second % 10) == 0)  //10s���һ��
                    #endif
                    {
                        #ifdef __PROVICE_CHONGQING__ 
                        /*
                         * ���� �����ݶ�������������ȫ�¼����Լ��ʱ��·�������ն˳���
                         * ���¼����ݶ��ˣ����̱���ϣ����²����ɲ��ϱ�ȫ�¼��ˡ�
                         */
                        portContext->ajust_time = 0;
                        #endif
                        //�������볭��ʱ��
                        if (check_recording_time(readportcontext)) return 0;
                    }
                }
        
                //���ڵ�Ԥ��
                #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
                if (check_priority_node(readportcontext)) return 0;
                #endif
        
                //��������Ƿ�仯
                if (check_meter_doc(readportcontext)) return 0;
        
                if(check_plc_parall_read(readportcontext))
                {
                    portContext->router_parall_need_send = 0xAA;
                    return 0;
                }
                else
                {
                    portContext->router_parall_need_send = 0;
                }
            }
    
        }
        else
        {
            if (portContext->cur_plc_task_step == PLC_NET_WAIT_CJQ_SEARCH_METER)
            {
                if (time_elapsed_10ms(portContext->plc_net_time_out_10ms) > portContext->plc_net_wait_time_s*100)
                {
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** �ȴ��ɼ����ѱ� %d s ������ ***" ,portContext->plc_net_wait_time_s);
                    debug_println_ext(info);
                    #endif
                     //#if (defined __PROVICE_SICHUAN__) || (defined __PROVICE_SHAANXI__)
                     if((portContext_plc.router_base_info.router_info1.comm_mode == 2) && (portContext_plc.hplc_area_distinguish != 0))
                     {
                         active_node_distinguish(portContext); /*�����05HF6����*/
                     }
                     else
                    // #endif
                     {
                          active_node_logon(portContext);
                     }
                }
            }
            else if (portContext->cur_plc_task_step == PLC_NET_WAIT_NODE_LOGON)
            {
                if (((portContext->plc_net_wait_time_s > 0) && (time_elapsed_10ms(portContext->plc_net_time_out_10ms) > portContext->plc_net_wait_time_s*100))
                || (portContext->params.task_plc_net.run_params.is_force_exit)
                || ((datetime[HOUR] == 23) && (datetime[MINUTE] > 30))
                )
                {
                    portContext->params.task_plc_net.run_params.is_force_exit = 0;
                    //ʱ�䵽�ˣ������ѱ�
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** ʱ�䵽�ˣ��ѱ�����������ѱ�ͳ�ƣ�***");
                    debug_println_ext(info);
                    #endif
                    //�����ѱ�ͳ��
                    portContext->cur_plc_task = PLC_TASK_PLC_NET;
                    portContext->cur_plc_task_step = PLC_NET_END_NODE_LOGON_11_F6;//PLC_NET_END_PAUSE_ROUTER_12_F2;
                    if(portContext_plc.hplc_area_distinguish ==0x55)
                    {
                        portContext->OnPortReadData = router_send_afn_05_F6;
                        readport_plc.OnPortReadData = router_send_afn_05_F6;
                    }
                    else
                    {
                        portContext->OnPortReadData = router_send_afn_11_F6;
                        readport_plc.OnPortReadData = router_send_afn_11_F6;
                    }
                    return 0;
                }
                else if (time_elapsed_10ms(portContext->plc_net_timer_10ms) > 1*60*100)
                {
                    //�ѱ�����У�·�ɿ��������ӣ���ѯһ��·��״̬
                    readport_plc.OnPortReadData = router_send_afn_10_F4;
                    portContext->plc_net_timer_10ms = os_get_systick_10ms();
                    return 0;
                }
            }
            else
            {
               if((datetime[HOUR] == 23) && (datetime[MINUTE] > 30))
              {
                       //�����ѱ�����
              portContext->cur_plc_task = PLC_TASK_IDLE;
              portContext->cur_plc_task_step = 0;
              portContext->OnPortReadData = router_check_urgent_timeout;
              readport_plc.OnPortReadData = router_check_urgent_timeout;
              portContext_plc.read_status.plc_net = 0;
              }
            }
        }
    }
    if(portContext->OnPortReadData)
    {
        readport_plc.OnPortReadData = portContext->OnPortReadData;
    }

    return 0;
}

INT8U router_reset(objReadPortContext * readportcontext)
{
    INT8U idx;
    PLCPortContext *portContext;
    portContext = (PLCPortContext*)readportcontext;

    switch(portContext->cur_plc_task_step)
    {
    case PLC_IDLE_ROUTER_RESET_LOW:
        if ((portContext->router_base_info.router_info1.comm_mode == 1))//�����Ҫ���ܽţ�
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** ����·�ɵ�reset ***");
            debug_println_ext(info);
            #else
            pin_router_rst(PIN_LOW);
            #endif
        }
        else
        {

        }
        #ifndef __FactoryTest__
        for(idx=0;idx<(PARALL_MAX_CHANNEL_COUNT+PLC_OTHER_CHANNEL_COUNT);idx++) /*�������ͨ���ϵ���Ϣ*/
        {
            mem_set((INT8U*)(&portContext->router_phase_work_info[idx].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
        }
        portContext->urgent_task = PLC_TASK_IDLE; /*�����������*/
        portContext->urgent_task_step = PLC_URGENT_TASK_IDLE;
        portContext->urgent_task_id = RECMETER_TASK_NONE;
        portContext->router_parall_need_send = 0;
        #endif
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_WAIT_1S;
        portContext->tick = os_get_systick_10ms();
        break;
    case PLC_IDLE_ROUTER_RESET_WAIT_1S:
        if (time_elapsed_10ms(portContext->tick) > 100)   //1s
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** ����·�ɵ�reset���ȴ�1s ***");
            debug_println_ext(info);
            #endif
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_HIGH;
        }
        break;
    case PLC_IDLE_ROUTER_RESET_HIGH:
        if ((portContext->router_base_info.router_info1.comm_mode == 1))//�����Ҫ���ܽţ�
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** ����·�ɵ�reset ***");
            debug_println_ext(info);
            #else
            pin_router_rst(PIN_HIGH);
            #endif
        }
        else
        {

        }

        #ifdef __FUJIAN_SUPPLEMENT_SPECIFICATION__
		if(portContext_plc.fujian_ctrl.protocol_type == PROTOCOL_FUJIAN)
		{
		    #if(defined __TASK_PLAN_UPADTE__)
		    portContext->fujian_ctrl.cur_exec_plan_id = 0;
			portContext->fujian_ctrl.cur_exec_plan_id_idx = 0;
			mem_set(portContext->fujian_ctrl.cur_DA_TD,6,0x00);
		    #else
			if(portContext->fujian_ctrl.cur_exec_plan_id)
			{
				//
				tpos_mutexPend(&SIGNAL_PLAN_LIST);
				if(plan_list.plan_info[portContext->fujian_ctrl.cur_exec_plan_id_idx].plan_state == PLAN_STATE_EXECUTEING)
				{
					plan_list.plan_info[portContext->fujian_ctrl.cur_exec_plan_id_idx].plan_state = PLAN_STATE_IDLE;
					plan_list.plan_info[portContext->fujian_ctrl.cur_exec_plan_id_idx].refresh_flag = 1;
				}
				tpos_mutexFree(&SIGNAL_PLAN_LIST);
				portContext->fujian_ctrl.cur_exec_plan_id = 0;
				portContext->fujian_ctrl.cur_exec_plan_id_idx = 0;
				mem_set(portContext->fujian_ctrl.cur_DA_TD,6,0x00);
			}
			#endif
			// ��Ҫ��� ������ʹ������Ҳ�����ˡ�
			tpos_mutexPend(&SIGNAL_OBJ_INDEX_LIST);
			mem_set(obj_index_list.value,sizeof(obj_index_list),0x00);
			tpos_mutexFree(&SIGNAL_OBJ_INDEX_LIST);
			//������ϢҲ��������� TODO ???
			mem_set((portContext->router_phase_work_info[0].read_params.value),sizeof(READ_PARAMS),0x00);
        	mem_set((portContext->router_phase_work_info[1].read_params.value),sizeof(READ_PARAMS),0x00);
        	mem_set((portContext->router_phase_work_info[2].read_params.value),sizeof(READ_PARAMS),0x00);
			readport_plc.OnPortReadData = router_query_router_mode;
        	portContext->OnPortReadData = router_query_router_mode;
        	portContext->cur_plc_task = PLC_TASK_CHECK_ROUTER_MODE;
        	portContext->cur_plc_task_step = 0;
		}
		else		
		#endif
		{
            readport_plc.OnPortReadData = router_query_version;
            portContext->OnPortReadData = router_query_version;
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = 0;
            portContext->router_interactive_status.meter_doc_synchro_done = 0; //����δ�ȶ�ǰ���������¼��ϱ���ֻ����һ��ȷ��֡��
        }        
        break;
    }

    return 0;
}

INT8U router_query_version(objReadPortContext * readportcontext)
{

    INT32U time;
    PLCPortContext *portContext;
    INT8U idx;

    portContext = (PLCPortContext*)readportcontext;

    portContext->repeated_sending_times = 0;  /*��ѯ�汾ʱ���쳣���ظ����ʹ�������*/
    portContext->router_interactive_status.meter_doc_synchro_done = 0; /*�����ȶ���ɱ�ʶ��0��������ܲ��߸�λ���������������һ��*/
    portContext->router_parall_need_send = 0;  /*��Ҫ���ƽ���*/

    #ifndef __FactoryTest__ /*����������Ч�ʣ�����գ�������������*/
    /*�������һ��ͨ����Ϣ������Ϊ���ŵĿ���������ܽš��������ܽŵ��������ɾ��������ʱ�����ط��������*/
    for(idx=0;idx<(PARALL_MAX_CHANNEL_COUNT+PLC_OTHER_CHANNEL_COUNT);idx++) /*�������ͨ���ϵ���Ϣ*/
    {
        mem_set((INT8U*)(&portContext->router_phase_work_info[idx].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
    }
    portContext->urgent_task = PLC_TASK_IDLE; /*�����������*/
    portContext->urgent_task_step = PLC_URGENT_TASK_IDLE;
    portContext->urgent_task_id = RECMETER_TASK_NONE;
    #endif

    if(portContext->cur_plc_task != PLC_TASK_QUERY_VERSION)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** PLC_TASK_QUERY_VERSION ***");
        debug_println_ext(info);
        #endif
        portContext->cur_plc_task = PLC_TASK_QUERY_VERSION;
        portContext->cur_plc_task_step = PLC_QUERY_VERSION_WAIT_REPORT_03F10;
        portContext->tick = os_get_systick_10ms();
    }
    #if defined(__SOFT_SIMULATOR__)
    if (time_elapsed_10ms(portContext->tick) > 10*100)  //�ȴ�10s
    #else
    #if defined(__SGRID_HARDWARE__) || defined(__SGRID_HARDWARE_II__)
    if (time_elapsed_10ms(portContext->tick) > 1*100)  //�ȴ�1s,·����09�淶�ģ������ϱ��汾��Ϣ     
    #elif defined(__NEIMENG_09_3762__)
    if (time_elapsed_10ms(portContext->tick) > 3*100)  //�ȴ�3s,·��TCS�ϵ縴λ����Լ2s
    #elif defined(__FactoryTest__)
    if (time_elapsed_10ms(portContext->tick) > 3*100)  //�ȴ�3s
    #else
    if ((portContext->router_base_info.router_info1.comm_mode == 2) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)) //���·�ɣ���ʱ�����ϱ�03F10
        time = 1*100;  //�ȴ�1s
    else
        time = 60*100;  //�ȴ�60s

    if (time_elapsed_10ms(portContext->tick) > time)  //�ȴ�60s
    #endif
    #endif
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** û�еȴ����ϱ�03H-F10 ***");
        debug_println_ext(info);
        #endif
        #ifdef __NEIMENG_09_3762__
        readport_plc.OnPortReadData = router_send_afn_03_F1;
        portContext->OnPortReadData = router_send_afn_03_F1;
        portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F1;
        #else
        portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F10;
        readport_plc.OnPortReadData = router_send_afn_03_F10;
        portContext->OnPortReadData = router_send_afn_03_F10;
        #endif
    }

    return 0;
}

INT8U router_wait_send_frame_complete(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT32U mm;

    portContext = (PLCPortContext*)readportcontext;

    mm = ((portContext->cur_plc_task_step == PLC_QUERY_VERSION_QUERY_03F10) && (portContext->cur_plc_task == PLC_TASK_QUERY_VERSION)) ? 500 : 10;      //��λ10ms


    #ifdef __SOFT_SIMULATOR__
    if(time_elapsed_10ms(portContext->router_resp_time_out) > mm)   //300ms
    #else
    if(drv_plc_is_idle() == TRUE)
    #endif
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** �ȴ�300ms ***");
        debug_println_ext(info);
        #endif
        if(mm == 500) //�ڲ�·�ɰ汾��ʱ�򣬲�Ҫ���������������
        {
          if(time_elapsed_10ms(portContext->router_resp_time_out) > mm)
          {
            if (portContext->OnPortReadData)
            {
                readport_plc.OnPortReadData = portContext->OnPortReadData;
            }
          }
        }
        else
        {
            #ifdef __FUJIAN_SUPPLEMENT_SPECIFICATION__
            if( (portContext->urgent_task_id == RECMETER_TASK_TRANS_CAST_TIME) && (portContext->urgent_task_step == PLC_CAST_FUJIAN_REPORT_56_F6) )
            {
            	//����״̬������ȴ���һ�����������״̬
                urgent_task_in_wait_next_urgent_task_fujian(portContext);
            }
    		else
    		{
                if (portContext->OnPortReadData)
                {
                    readport_plc.OnPortReadData = portContext->OnPortReadData;
                }
            }
		    #else        
            if (portContext->OnPortReadData)
            {
               readport_plc.OnPortReadData = portContext->OnPortReadData;
            }
            #endif
        }
    }

    return 0;
}

void start_read_meter(PLCPortContext* portContext)
{
    INT16U count_fast_index = 0;
    static INT8U date_day = 0;

    #if 0
    // ����������ط���⣬����Ҫ���ϳ���һ��ģ��ID�ȣ���ʱ����������Ҫ�Լ�����--20190514    
    if(tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))//���뵽�ź��� �ʹ��� ���򲻴���
    {
        tpos_enterCriticalSection();
        if( RECMETER_TASK_NONE == portContext->urgent_task_id )
    {
            portContext->urgent_task_id = RECMETER_TASK_TRANS_READ_MODULE_ID; //��������
    }
        tpos_leaveCriticalSection();
        tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
    }
    #endif

    portContext->router_interactive_status.meter_doc_synchro_done = 1;/*�����ȶ���� */

    if(portContext->read_status.is_in_read_cycle)
    {
        //���볭������
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** PLC_TASK_READ_METER ***");
        debug_println_ext(info);
        #endif
        //�Ȳ�һ���Ƿ�֧�ֲ�����
         if(portContext->plc_other_read_mode == CYCLE_REC_MODE_PARALLEL)
        {
            //���뼯���������Ĳ��г���ģʽ
             count_fast_index = memory_fast_index_stat_port_node_count(READPORT_PLC,portContext->router_base_info.router_info3.rtu_no_mode,portContext->router_base_info.router_info4.dzc_cvt_no_mode);
             if(count_fast_index == 0)  //0���������ʱ��Ҫ��״̬�ˣ����ſ��0��������ʱһֱ������
            {
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** ���������ڵ����� = 0 ��·�ɽڵ����� = 0 , �˳����� ***");
                debug_println_ext(info);
                #endif
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                portContext->cur_plc_task = PLC_TASK_IDLE;
                portContext->cur_plc_task_step = 0;

                 portContext->router_interactive_status.plc_net_not_allow = 0; //�����ѱ��ӱȶԵ������ƣ�ֻ�Զ��Ŵ���
            }
            else
            {
                /*��ʼ��鳭�����������ǰ���Ѿ���ѯ��*/
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_12_F2;
                portContext->cur_plc_task = PLC_TASK_PARALLEL_READ;
                portContext->cur_plc_task_step = PLC_PARALL_READ_12HF2;

                portContext->router_interactive_status.plc_net_not_allow = 0; //�����ѱ�
            }

        }
        else if ((portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_CONCENTRATOR)
                || (portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ALL))  //�����֧�ֲ�������ô��������
        {
            //���뼯��������ģʽ

                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = get_read_meter_info;
                portContext->cur_plc_task = PLC_TASK_READ_METER;
                portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;

                portContext->params.task_read_data.node_idx[0] = 0;
                portContext->params.task_read_data.node_idx[1] = 0;
                portContext->params.task_read_data.has_fail_meter = FALSE;
               
                if(date_day != datetime[DAY])
                {
                date_day = datetime[DAY];
                portContext->concentrator_read_cycle_no = 0; //��������һ��
            }
        }
        else
        {
             #ifdef __PROVICE_JIANGXI__
              //��ʼ����ǰ����ͳ���ز����������������������ж��Ƿ���Ҫ�ܿ�95%������ȫ�¼�����
              #ifdef __SUCCESS_RATE_OVER_95__
                count_fast_index = memory_fast_index_stat_port_node_count(READPORT_PLC,portContext->router_base_info.router_info3.rtu_no_mode,portContext->router_base_info.router_info4.dzc_cvt_no_mode);
              if(count_fast_index <10)
              {
                  //�����仯������С��10�����ܳɹ���95%���ƣ�����10��������������鿴
                  //95%�Ĵ����������⣬���������������޸ġ�
                  portContext_plc.success_rate = 1; //��ʼֵ���ز���������10
              }
              #endif
             #endif
                 readport_plc.OnPortReadData = router_check_urgent_timeout;
                 portContext->OnPortReadData = router_send_afn_12_F1;
                 portContext->cur_plc_task = PLC_TASK_READ_METER;
                 portContext->cur_plc_task_step = PLC_READ_METER_RESTART_ROUTER_12_F1;
             }
        }
    else
    {
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext->OnPortReadData = router_check_urgent_timeout;
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = 0;
    }
}

void start_check_meter_doc(PLCPortContext* portContext)
{
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** PLC_TASK_CHECK_DOC ***");
    debug_println_ext(info);
    #endif

    portContext->read_status.doc_chg = 0;
    portContext->router_interactive_status.meter_doc_synchro_done = 0;
    portContext_plc.parall_read_ready_ok = 0;
    portContext_plc.doc_cynchro_rev_deny_num = 0;
    
    if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC09)
    {
        //������Ѷ��09·���Լ��ıȶԵ�������
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext->OnPortReadData = router_send_friend_afn_05_F31;
        portContext->cur_plc_task = PLC_TASK_CHECK_DOC;
        portContext->cur_plc_task_step = PLC_CHECK_DOC_FRIENDCOM_05H_F31;
    }
    #ifdef __BATCH_TRANSPARENT_METER_TASK__
    else if ((portContext->batch_meter_ctrl.is_not_check_meter_doc)
        && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM))
    {
        portContext->batch_meter_ctrl.is_not_check_meter_doc = 0;
        start_read_meter(portContext);
    }
    #endif
    else if (portContext->router_base_info.router_info4.afn_12H_is_valid)
    {
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext->OnPortReadData = router_send_afn_12_F2;
        portContext->cur_plc_task = PLC_TASK_CHECK_DOC;
        portContext->cur_plc_task_step = PLC_CHECK_DOC_PAUSE_ROUTER;
    }
    else
    {
        portContext->cur_plc_task = PLC_TASK_CHECK_DOC;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext->OnPortReadData = router_send_afn_10_F1;
        portContext->cur_plc_task_step = PLC_CHECK_DOC_QUERY_NODE_COUNT_10F1_1;
    }
}
//���л�״̬��ȷ��
INT8U router_send_afn_00_F1_no_change_status(objReadPortContext * readportcontext)
{
    INT8U temp[6] = {0};

    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    mem_set(temp,4,0xFF);

    router_send_3762_frame_no_change_status(portContext,DL69842_AFN_CONFIRM,DT_F1,temp,6);

    #ifdef __MEXICO_GUIDE_RAIL__
    DelayNmSec(80); /* ��ʱһ�ᣬҪ�����ܴ��ڽ�������13F1������ȥ */
    #endif
    return 0;
}
INT8U router_send_afn_00_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U confirm_data[6];
    INT8U pos;

    portContext = (PLCPortContext*)readportcontext;

    pos = 0;
    //����ŵ�״̬
    confirm_data[pos++] = 0xFF;
    confirm_data[pos++] = 0xFF;
    //#ifdef __376_2_2013__
    confirm_data[pos++] = 0xFF;
    confirm_data[pos++] = 0xFF;
    //#endif

    //�ȴ�ʱ��
    confirm_data[pos++] = 0;
    confirm_data[pos++] = 0;

    router_376_2_set_aux_info(portContext->router_work_info.channel_id,40,0,FALSE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_CONFIRM,DT_F1,confirm_data,pos,portContext);

    readport_plc.OnPortReadData = router_wait_send_frame_complete;
    portContext->OnPortReadData = router_check_urgent_timeout;

    switch(portContext->cur_plc_task)
    {
    case PLC_TASK_QUERY_VERSION:
        switch(portContext->cur_plc_task_step)
        {
        case PLC_QUERY_VERSION_WAIT_REPORT_03F10:
			#ifdef __READ_MODULE_ID_PLAN__
            /* ��ģ��ID */
            {
                portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F12;
                readport_plc.OnPortReadData = router_wait_send_frame_complete;
                portContext->OnPortReadData = router_send_afn_03_F12;
            }
			#else
                {
                    portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F4;
                    readport_plc.OnPortReadData = router_wait_send_frame_complete;
                    portContext->OnPortReadData = router_send_afn_03_F4;
                }
            #endif
            break;
        }
        break;
    case PLC_TASK_PLC_NET:
        switch(portContext->cur_plc_task_step)
        {
        case PLC_NET_REPORT_06_F3:
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** �ϱ�������Ϣ���ѱ�����������ѱ�ͳ�ƣ�***");
            debug_println_ext(info);
            #endif
            //�����ѱ�ͳ��
            start_plc_net_stat(portContext);
            break;
        }
        break;
    case PLC_TASK_READ_METER:
        switch(portContext->cur_plc_task_step)
        {
        case PLC_READ_METER_WAIT_RESP_13F1:
        case PLC_READ_METER_FIND_METER:
        case PLC_READ_METER_PREPARE_ITEM:     //����������3��״̬������ȷ�Ϻ�Ҫ�л���������
             portContext->OnPortReadData = prepare_read_item_concentrator;
             readport_plc.OnPortReadData = router_wait_resp_frame;
             break;
        case PLC_READ_METER_RESTART_ROUTER_12_F1: //����ڱ����У��������ɹ���Ҫ�ط���
             readport_plc.OnPortReadData = router_wait_send_frame_complete;
             portContext->OnPortReadData = router_send_afn_12_F1;
             break;
        case PLC_READ_METER_ROUTER_MODE:      //�ڵ�·�����󳭱�Ĺ����У�����Ҫ�л�
        case PLC_READ_METER_SLEEP:           //�����ˣ�Ҳ����Ҫ������
             readport_plc.OnPortReadData = router_wait_send_frame_complete;
             portContext->OnPortReadData = router_check_urgent_timeout;
             break;
        default:
             portContext->need_reset_router = 1;
             break;
             //���������̣�����־

        }
        break;
    case PLC_TASK_CHECK_DOC:  //�ȶԵ�������ֻ���ڲ������Ƿ�����з�ȷ�ϵ�ʱ�򣬲�ת��10HF4�ϡ�
        switch(portContext->cur_plc_task_step)
        {
        case PLC_CHECK_WIRELESS_NET_READY:
             readport_plc.OnPortReadData = router_check_urgent_timeout;
             portContext->OnPortReadData = router_send_afn_10_F4;
             break;
        default:
             portContext->need_reset_router = 1;
             break;
        }
        break;
    case PLC_TASK_PARALLEL_READ:
        switch(portContext->cur_plc_task_step)
        {
        case PLC_PARALL_READ_WAIT_READY:   //����ǵ�������ɣ�Ҫ����������
             readport_plc.OnPortReadData = router_check_urgent_timeout;
             portContext->OnPortReadData = router_send_afn_10_F4;
             break;
        case PLC_PARALL_READ_12HF2:  //����ͣ�����û�յ�ȷ��,�ȴ��ط�
             readport_plc.OnPortReadData = router_check_urgent_timeout; //
             portContext->OnPortReadData = router_send_afn_12_F2;
             break;
        case PLC_PARALL_READ_FIND_METER:
        case PLC_PARALL_READ_SEND_F1HF1:
        case PLC_PARALL_READ_WAIT_F1HF1:
             readport_plc.OnPortReadData = router_wait_send_frame_complete;
             portContext->OnPortReadData = router_check_urgent_timeout;  //�ȳ�ʱ���������߲�������
             break;
        default:
             readport_plc.OnPortReadData = router_wait_send_frame_complete;
             portContext->OnPortReadData = router_check_urgent_timeout;
             break;
        }
        break;

    default:
             portContext->need_reset_router = 1;
       break;
             //���������̣�����־
    }


    return portContext->frame_send_Len;
}

INT8U router_recv_afn_00_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT16U tmp;
    INT8U phase;

    portContext = (PLCPortContext*)readportcontext;

    if (portContext->cur_plc_task_step == PLC_INIT_ROUTER_01F2)
    {
        portContext->read_status.is_in_read_cycle = 0;

        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = 0;
        portContext->OnPortReadData = router_check_urgent_timeout;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
    }
    else if ((portContext->cur_plc_task_step == PLC_CAST_OPEN_05_F3) || (portContext->urgent_task_step == PLC_CAST_OPEN_05_F3)
            || (portContext->urgent_task_step == PLC_CAST_OOP_OPEN_05_F3))
    {
        tmp = bin2_int16u(portContext->frame_recv+portContext->recv_data_pos+4);  //�ȴ�ʱ�� s
        if (tmp > 200) tmp = 200; //����200��
        tmp++; //���1s
        portContext->cast_timer_10ms = os_get_systick_10ms();
        portContext->cast_wait_time_s = tmp;

        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** �ȴ��㲥ָ��ִ��ʱ�� = %d s ***",tmp);
        debug_println_ext(info);
        #endif

        readport_plc.OnPortReadData = wait_cast_exec_end;

        if(portContext->urgent_task_step == PLC_CAST_OPEN_05_F3)//��վ�Ƚ�������Ĺ㲥
        {
            portContext->urgent_task_step = PLC_CAST_WAIT_EXEC_END;
        }
        else if(portContext->urgent_task_step == PLC_CAST_OOP_OPEN_05_F3) //�����������oop��ʱ
        {
            portContext->urgent_task_step = PLC_CAST_OOP_WAIT_EXEC_END;
        }
        else
        {
            portContext->OnPortReadData = wait_cast_exec_end; //������ѱ��У��㲥�ɼ����ѱ�
            portContext->cur_plc_task_step = PLC_CAST_WAIT_EXEC_END;
        }
    }
    else if(portContext->cur_plc_task_step == PLC_IDLE_EXIT_SEG_12_F2)
    {
        if (portContext->seg_end_time.is_lase_time_seg)
        {
            //#if defined(__COMPUTE_XLOST__)
            //ÿ������ʱ��ʱǿ�Ƽ�������û�г����ĵ�����Ϊ0����
            portContext->cur_plc_task_step = PLC_IDLE_COMPUTE_XLOST;
            portContext->OnPortReadData = exec_xlost;
            readport_plc.OnPortReadData = exec_xlost;
        }
        else
        {
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            portContext->OnPortReadData = router_check_urgent_timeout;
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = 0;
        }
    }
    else if (portContext->urgent_task)
    {
        switch(portContext->urgent_task_step)
        {
        case PLC_URGENT_TASK_PAUSE_ROUTER_12_F2:
            exec_urgent_task(readportcontext);
            break;
        case PLC_URGENT_TASK_RESUME_ROUTER_12_F3:
            portContext->urgent_task = PLC_TASK_IDLE;
            portContext->urgent_task_step = PLC_URGENT_TASK_IDLE;
            portContext->urgent_task_state.is_send_pause = 0;/*�ָ������յ�ȷ�ϻظ�*/
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** PLC_TASK_URGENT_TASK - end ***");
            debug_println_ext(info);
            #endif
            //readport_plc.OnPortReadData = router_urgent_task_send_idle;
            break;
        case PLC_URGENT_TASK_ADD_NODE:
            if (portContext->router_base_info.router_info4.monitor_afn_type)
            {
                readport_plc.OnPortReadData = router_send_afn_02_F1;
            }
            else
            {
            readport_plc.OnPortReadData = router_send_afn_13_F1;
            }
            portContext->urgent_task_step = PLC_URGENT_TASK_MONIER_645_2;
            break;
            #ifdef __INSTANT_FREEZE__
        case PLC_URGENT_INSTANT_FREEZE_PAUSE://˲ʱ��������̣���������ͣ-����-��ͣ-�㲥-�ָ�
            readport_plc.OnPortReadData = router_send_afn_12_F2;
            portContext->urgent_task = PLC_TASK_URGENT_TASK;
            portContext->urgent_task_step = PLC_URGENT_INSTANT_FREEZE_CAST;
            break;
        case PLC_URGENT_INSTANT_FREEZE_CAST:
            portContext->cast_content = portContext->frame_cast_buffer; // portContext->frame_send
            portContext->cast_content_len = make_instant_freeze_frame(portContext->cast_content,NULL,0); //�㲥˲ʱ��������
            readport_plc.OnPortReadData = router_send_afn_05_F3;
            portContext->urgent_task = PLC_TASK_URGENT_TASK;
            portContext->urgent_task_step = PLC_CAST_OPEN_05_F3;
            break;
            #endif
        default:
            urgent_task_in_wait_next_urgent_task(portContext);
            break;
        }
    }
    else
    {
        switch(portContext->cur_plc_task)
        {
        case PLC_TASK_QUERY_VERSION:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_QUERY_VERSION_CTRL_05F1:  //�������ڵ��ַ����ѯ·�ɰ汾��Ϣ��ɣ����뵵������
                //�������ڵ��ַ
                mem_cpy(gSystemInfo.plc_ver_info,portContext->params.task_check_main_node.main_node,6);
                mem_cpy(portContext->router_work_info.ADDR_SRC,portContext->params.task_check_main_node.main_node,6);

               // if (portContext->read_status.is_in_read_cycle)
                if(1)
                {
                    start_check_meter_doc(portContext);
//                    #ifdef __SOFT_SIMULATOR__
//                    snprintf(info,100,"*** PLC_TASK_CHECK_DOC ***");
//                    debug_println_ext(info);
//                    #endif
//                    readport_plc.OnPortReadData = router_check_urgent_timeout;
//                    portContext->OnPortReadData = router_send_afn_12_F2;
//                    portContext->cur_plc_task = PLC_TASK_CHECK_DOC;
//                    portContext->cur_plc_task_step = PLC_CHECK_DOC_PAUSE_ROUTER;
//                    portContext->read_status.doc_chg = 0;
                }
                else
                {
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_check_urgent_timeout;
                    portContext->cur_plc_task = PLC_TASK_IDLE;
                    portContext->cur_plc_task_step = 0;
                }
                break;
            case PLC_QUERY_VERSION_QUERY_03F10:
               // if (portContext->read_status.is_in_read_cycle)
                {
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** PLC_TASK_CHECK_DOC ***");
                    debug_println_ext(info);
                    #endif
                    /*
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_12_F2;
                    portContext->cur_plc_task = PLC_TASK_CHECK_DOC;
                     portContext->cur_plc_task_step = PLC_CHECK_DOC_PAUSE_ROUTER;
                    portContext->read_status.doc_chg = 0;
                    #ifdef __ROUTER_DEBUG_09__
                    portContext->router_base_info.router_13_or_09 = ROUTER_PROTOCOL_GB3762;// ����03HF10����Ӧ����09Э�鴦��
                    #endif
                    */
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_03_F1;
                    portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F1;

                    portContext->router_base_info.router_info1.cycle_rec_mode = CYCLE_REC_MODE_ROUTER;//�����ɶ��

                }
                break;
            }
            break;
        case PLC_TASK_CHECK_DOC:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_CHECK_DOC_PAUSE_ROUTER:

                if(portContext->router_base_info.router_info1.node_mode == 0)
                {
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    if (((portContext->router_base_info.router_info1.comm_mode == ROUTER_COMM_MODE_WIFI) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC13))
                     || ((portContext->router_base_info.router_info1.comm_mode == ROUTER_COMM_MODE_HPLC) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)))
                     /*��Ѷ�����ߺͶ��ſ����Ҫ��������Ϣ*/
                    {
                        readport_plc.OnPortReadData = router_check_urgent_timeout;
                        portContext->OnPortReadData = router_send_afn_10_F4;
                        portContext->cur_plc_task_step = PLC_CHECK_WIRELESS_NET_READY;
                    }
                    else
                    {
                        start_read_meter(portContext);
                    }
                }
                else if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_XC0010)
                {
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_11_F1;
                    portContext->cur_plc_task_step = PLC_CHECK_DOC_ADD_NODE_11F1_1;
                }
/*
                else if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC)
                {
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_friend_afn_09_F11;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_ADD_NODE_11F1_1;
                break;
                }
 */               
                else
                {
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_10_F1;
                    portContext->cur_plc_task_step = PLC_CHECK_DOC_QUERY_NODE_COUNT_10F1_1;
                }
                break;
            case PLC_CHECK_DOC_INIT_ROUTER_01F2_1:
                start_read_meter(portContext);
                break;
            case PLC_CHECK_DOC_DEL_NODE:
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_10_F2;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_QUERY_NODE_INFO;
                tmp = bin2_int16u(portContext->params.task_check_doc.strat_seq);
                tmp += portContext->params.task_check_doc.jump_count;
                int16u2_bin(tmp,portContext->params.task_check_doc.strat_seq);
                portContext->params.task_check_doc.query_count = ROUTER_OPT_NODE_COUNT;
                break;
            case PLC_CHECK_DOC_ADD_NODE_11F1_1:
                memory_fast_index_clear_add_flag(readportcontext);
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = memory_fast_index_add_router_nodes;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_CHECK_FAST_INDEX_1;
                break;
            case PLC_CHECK_DOC_INIT_ROUTER_01F2_2:    //����reinstall node
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = memory_fast_index_add_router_nodes;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_CHECK_FAST_INDEX_2;
                portContext->params.task_check_doc.reinstall = 1;
                memory_fast_index_set_reinstall(COMMPORT_PLC);
                break;
            case PLC_CHECK_DOC_ADD_NODE_11F1_2:
                memory_fast_index_clear_add_flag(readportcontext);
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = memory_fast_index_add_router_nodes;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_CHECK_FAST_INDEX_2;
                break;
            case WIRELESS_CHANEL_SET_05F4:
                 portContext->OnPortReadData = prepare_read_item_concentrator;
                 break;
            }
            break;
        case PLC_TASK_PLC_NET:  //�ѱ�
            switch(portContext->cur_plc_task_step)
            {
            case PLC_NET_PAUSE_ROUTER_12_F2:
                get_router_main_node_addr(portContext->params.task_check_main_node.main_node);
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_05_F1;
                portContext->cur_plc_task = PLC_TASK_PLC_NET;
                portContext->cur_plc_task_step = PLC_NET_SET_MAIN_NODE_05_F1;
                break;
            case PLC_NET_SET_MAIN_NODE_05_F1:
                if (portContext->router_base_info.router_info3.plc_net_is_cast)
                {
                    readport_plc.OnPortReadData = start_cast_cjq_485_search_meter;
                    portContext->OnPortReadData = start_cast_cjq_485_search_meter;
                    portContext->cur_plc_task_step = PLC_NET_CAST_CJQ_SEARCH_METER;
                }
                else
                {
                    // #if (defined __PROVICE_SICHUAN__) || (defined __PROVICE_SHAANXI__)
                     if((portContext_plc.router_base_info.router_info1.comm_mode == 2) && (portContext_plc.hplc_area_distinguish != 0))
                     {
                         active_node_distinguish(portContext); /*�����05HF6����*/
                     }
                     else
                   //  #endif
                     {
                          active_node_logon(portContext);
                     }
                }
                break;
            case PLC_NET_ACTIVE_NODE_LOGON_11_F5:
                portContext->cur_plc_task_step = PLC_NET_WAIT_NODE_LOGON;
                portContext->plc_net_timer_10ms = os_get_systick_10ms();
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                break;
            case PLC_NET_END_NODE_LOGON_11_F6:
                portContext->cur_plc_task_step = PLC_NET_END_PAUSE_ROUTER_12_F2;
                portContext->OnPortReadData = router_send_afn_12_F2;
                readport_plc.OnPortReadData = router_send_afn_12_F2;
                break;
            case PLC_NET_END_PAUSE_ROUTER_12_F2:
                start_plc_net_stat(portContext);
                break;
            }
            break;
        case PLC_TASK_READ_METER:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_READ_METER_RESTART_ROUTER_12_F1:
                portContext->cur_plc_task_step = PLC_READ_METER_ROUTER_MODE;
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
                portContext->batch_meter_ctrl.is_restart_node_yugao = 0;
                #endif
                break;
            case PLC_URGENT_TASK_RESUME_ROUTER_12_F3:  //�������ͨģʽ�£������˻ָ������Ĵ������и���������������
            case PLC_READ_METER_PRIOR_11F8: /*���ȶ��лظ���ȷ��֡��Ҫ�лس���ģʽ*/
                portContext->cur_plc_task_step = PLC_READ_METER_ROUTER_MODE;//���������˳���ģʽ
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                break;
            case WIRELESS_CHANEL_SET_05F4:
                 portContext->OnPortReadData = prepare_read_item_concentrator;
                 break;
            }
            break;

        case PLC_TASK_READ_VIP_METER:

            switch(portContext->cur_plc_task_step)
            {
            case PLC_VIP_METER_12HF2 :
                 start_read_vip_meter(portContext);
                 break;
            case PLC_TOPSCOMM_VIP_METER_12HF1 :
                portContext->cur_plc_task_step = PLC_TOPSCOMM_VIP_METER_11HF8;//���������
                readport_plc.OnPortReadData = router_send_afn_11_F8;
               // portContext->OnPortReadData = router_send_afn_11_F8; //�鿴-���͡��Ĳ��죬
                break;
            case PLC_TOPSCOMM_VIP_METER_11HF8 :
                portContext->cur_plc_task = PLC_TASK_READ_METER;
                portContext->cur_plc_task_step = PLC_READ_METER_ROUTER_MODE;//���������
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                break;
            case PLC_VIP_METER_12HF3:  //�������ͨģʽ�£������˻ָ������Ĵ������и���������������
                //portContext->cur_plc_task_step = PLC_READ_METER_ROUTER_MODE;//���������˳���ģʽ
                portContext->cur_plc_task = portContext->before_vip_task;
                portContext->cur_plc_task_step = portContext->before_vip_task_step;
                readport_plc.OnPortReadData = portContext->PLC_Before_OnPortReadData; //router_check_urgent_timeout;
                portContext->OnPortReadData = portContext->Before_OnPortReadData;  //router_check_urgent_timeout;
                break;
            default:
                 start_read_vip_meter(portContext);
                break;
            }
            break;
        case PLC_TASK_PARALLEL_READ:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_PARALL_READ_12HF2 :
                portContext->cur_plc_task = PLC_TASK_IDLE;
                portContext->cur_plc_task_step = 0;
                portContext->OnPortReadData = router_check_urgent_timeout;
                readport_plc.OnPortReadData = router_check_urgent_timeout;

                for(phase =1;phase<=(PARALL_MAX_CHANNEL_COUNT);phase++) /*������ͨ����ʼ��*/
                {
                  mem_set((INT8U*)(&portContext->router_phase_work_info[phase].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
                  portContext->router_phase_work_info[phase].paral_read.parall_status.parall_keep = 0;  //
                  portContext->router_phase_work_info[phase].paral_read.parall_status.parall_idle = 1;  //�����λ���Ϊidle
                }

                portContext_plc.params.task_read_data.node_idx[0] = 0; //��ʼ����00��ʼ
                portContext_plc.params.task_read_data.node_idx[1] = 0;
                
                portContext->params.task_read_data.has_fail_meter = FALSE;

                portContext_plc.concentrator_read_cycle_no = 0;//��һ���ִ�

                portContext->parall_read_ready_ok = 0xAA;

                 break;

            }
            break;
        case PLC_TASK_CLERA_AREA_DIS:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_CLEAR_AREA_DIS_PAUSE_12HF2:
                portContext->cur_plc_task_step = PLC_CLEAR_AREA_DIS_FRAME_SEND;//�����������
                readport_plc.OnPortReadData = router_send_afn_F0_F14;
                 break;
            case PLC_CLEAR_AREA_DIS_FRAME_SEND :
                portContext->cur_plc_task_step = PLC_CLEAR_AREA_DIS_ROUTER_INIT; //������ʼ��
                readport_plc.OnPortReadData = router_send_afn_01_F2;
                break;
            case PLC_CLEAR_AREA_DIS_ROUTER_INIT:
                portContext_plc.need_reset_router = 1 ;  //����·�ɣ�������ӵ�������
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                break;
            }

            break;

        default:
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            portContext->OnPortReadData = router_check_urgent_timeout;
            break;
        }
    }
	return 0;
}

INT8U router_recv_afn_00_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT16U meter_idx,save_pos;//,tmp;
    #if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
    INT16U count = 0;
    #endif
    INT8U router_protocol;
    INT8U err_type,idx;

    portContext = (PLCPortContext*)readportcontext;

    if (portContext->cur_plc_task_step == PLC_INIT_ROUTER_01F2)
    {
        portContext->read_status.is_in_read_cycle = 0;

        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = 0;
        portContext->OnPortReadData = router_check_urgent_timeout;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
    }
    else if(portContext->cur_plc_task_step == PLC_IDLE_EXIT_SEG_12_F2)
    {
        if (portContext->seg_end_time.is_lase_time_seg)
        {
            //#if defined(__COMPUTE_XLOST__)
            //ÿ������ʱ��ʱǿ�Ƽ�������û�г����ĵ�����Ϊ0����
            portContext->cur_plc_task_step = PLC_IDLE_COMPUTE_XLOST;
            portContext->OnPortReadData = exec_xlost;
            readport_plc.OnPortReadData = exec_xlost;
        }
        else
        {
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            portContext->OnPortReadData = router_check_urgent_timeout;
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = 0;
        }
    }
    else if(portContext->urgent_task)
    {
        switch(portContext->urgent_task_step)
        {
        case PLC_URGENT_TASK_MONIER_645_1:
            if((portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC09) || (portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC13))
            {
                //��������
                portContext->urgent_task_resp_frame_len = 0;
                //mem_cpy(portContext->urgent_task_resp_frame,portContext->frame_recv+portContext->recv_data_pos+4,portContext->urgent_task_resp_frame_len);
                //����ȴ���һ�����������״̬
                urgent_task_in_wait_next_urgent_task(portContext);
            }
            else if ((portContext->router_base_info.router_info1.node_mode)
            && (memory_fast_index_find_node_no(COMMPORT_PLC,portContext->router_work_info.ADDR_DST,&meter_idx,&save_pos,&router_protocol,NULL)))
            {
                if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
                {
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count = 1;
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[0].node,portContext->router_work_info.ADDR_DST,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[0].protocol = router_protocol;
                }
                else
                {
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count = 1;
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[0].node,portContext->router_work_info.ADDR_DST,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[0].protocol = router_protocol;
                }

                readport_plc.OnPortReadData = router_send_afn_11_F1;
                portContext->urgent_task_step = PLC_URGENT_TASK_ADD_NODE;
            }
            else
            {
                //��������
                portContext->urgent_task_resp_frame_len = 0;
                //mem_cpy(portContext->urgent_task_resp_frame,portContext->frame_recv+portContext->recv_data_pos+4,portContext->urgent_task_resp_frame_len);
                //����ȴ���һ�����������״̬
                urgent_task_in_wait_next_urgent_task(portContext);
            }
            break;
        case PLC_URGENT_TASK_ADD_NODE: //����ڽ�����������Ӵӽڵ㣬�ظ����ϣ����ؽ��м�ز���
            if (portContext->router_base_info.router_info4.monitor_afn_type)
            {
                readport_plc.OnPortReadData = router_send_afn_02_F1;
            }
            else
            {
            readport_plc.OnPortReadData = router_send_afn_13_F1;
            }
            portContext->urgent_task_step = PLC_URGENT_TASK_MONIER_645_2;
            break;
        case PLC_URGENT_TASK_MONIER_645_2://��������
            portContext->urgent_task_resp_frame_len = 0;
            //����״̬������ȴ���һ�����������״̬
            urgent_task_in_wait_next_urgent_task(portContext);
            break;
        #if defined(__READ_PLC_NOISE__)
        case PLC_URGENT_TASK_READ_NOISE:
            //����ȴ���һ�����������״̬
            urgent_task_in_wait_next_urgent_task(portContext);
            break;
        #endif
#ifdef __READ_MODULE_ID_PLAN__
        case PLC_URGENT_TASK_READ_MODULE_ID:

             portContext->task_read_module_id.node_start_seq[0] = 0x01;
             portContext->task_read_module_id.node_start_seq[1] = 0x00;
             portContext->task_read_module_id.query_node_cnt = ROUTER_OPT_NODE_COUNT;
             readport_plc.OnPortReadData = router_send_afn_10_F112;
             portContext->urgent_task = PLC_TASK_URGENT_TASK;
             portContext->urgent_task_step = PLC_URGENT_TASK_READ_CHIP_ID; /*ģ��ID���ϵ�ʱ��ҲҪ��оƬID*/
             break;
#endif
        #ifdef __INSTANT_FREEZE__
        case PLC_URGENT_INSTANT_FREEZE_PAUSE:
            urgent_task_in_wait_next_urgent_task(portContext);
            break;
        #endif
        case PLC_URGENT_TASK_RESUME_ROUTER_12_F3: /*������ָ������˷��ϣ���Ϊ·��������*/
            portContext->urgent_task = PLC_TASK_IDLE;
            portContext->urgent_task_step = PLC_URGENT_TASK_IDLE;
            portContext->urgent_task_state.is_send_pause = 0;

            readport_plc.OnPortReadData = router_reset;
            portContext->OnPortReadData = router_reset;
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;
            break;
        #if defined(__SHANXI_READ_BPLC_NETWORK_INFO__)
        case RECMETER_TASK_BPLC_NETWORK_INFO:
            //����ȴ���һ�����������״̬
            urgent_task_in_wait_next_urgent_task(portContext);
            break;
        #endif
            #if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
            case PLC_URGENT_SWITCH_TRIP_CTRL:
            
                portContext->switch_trip_try_cnt = 0;
                portContext->switch_idx++;            
                //tpos_mutexPend(&SIGNAL_FAST_IDX);
                count = fast_index_list.count;
                //tpos_mutexFree(&SIGNAL_FAST_IDX);
                if(portContext->switch_idx >= count)
                {
                    //����״̬������ȴ���һ�����������״̬
                    portContext->switch_trip_flag = 0x55;
                    fwrite_array(FILEID_RUN_DATA,PIM_SWITCH_TRIP_CTRL_FLAG,&(portContext->switch_trip_flag),1);
                    urgent_task_in_wait_next_urgent_task(portContext);
                }
                else
                {
                    readport_plc.OnPortReadData = router_send_afn_13_F1;
                    portContext->urgent_task_step = PLC_URGENT_SWITCH_TRIP_CTRL;
                }
                break;
            #endif   
        default:
            urgent_task_in_wait_next_urgent_task(portContext);
            break;
        }
    }
    else
    {
        switch(portContext->cur_plc_task)
        {
        case PLC_TASK_QUERY_VERSION:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_QUERY_VERSION_QUERY_03F10:       //03F10���ϵĻ����Ͳ�֧��13�淶��·��
                readport_plc.OnPortReadData = router_send_afn_03_F1;
                portContext->OnPortReadData = router_send_afn_03_F1;
                portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F1;
                break;
            #ifdef __READ_MODULE_ID_PLAN__
            case PLC_QUERY_VERSION_QUERY_03F12://��֧��03F12��ѯ·��ģ��ID��Ϣ�Ļ���ֱ����������ѯ·�����ڵ���Ϣ
                portContext->task_read_module_id.node_start_seq[0] = 0x01;
                portContext->task_read_module_id.node_start_seq[1] = 0x00;
                portContext->task_read_module_id.query_node_cnt = 1;
                portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_10F112_MAIN_NODE;
                readport_plc.OnPortReadData = router_wait_send_frame_complete;
                portContext->OnPortReadData = router_send_afn_10_F112;
                
                main_node_modul_id_deny_process();
                break;
            case PLC_QUERY_VERSION_QUERY_10F112_MAIN_NODE:
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_03_F4;
                portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F4;
                portContext->cur_plc_task = PLC_TASK_QUERY_VERSION;

                portContext->check_10HF112_avilid = 0x55;

                main_node_chip_id_deny_process();
                break; 
            #endif
            case PLC_QUERY_VERSION_QUERY_03F4:  //��ѯ03HF4���ϣ�ֱ������05HF1��
                 get_router_main_node_addr(portContext->params.task_check_main_node.main_node);
                 readport_plc.OnPortReadData = router_send_afn_05_F1;
                 portContext->OnPortReadData = router_send_afn_05_F1;
                 portContext->cur_plc_task_step = PLC_QUERY_VERSION_CTRL_05F1;
                 break;
            case PLC_QUERY_VERSION_CTRL_05F1:
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->need_reset_router = 1;//�������ڵ�����ط��ϣ�����һ��·�ɣ������������ϵ��ʱ�������������ڵ����
                break;
            case PLC_QUERY_VERSION_PARALL_OK_NO:
                readport_plc.OnPortReadData = router_send_afn_03_F4;
                portContext->OnPortReadData = router_send_afn_03_F4;
                portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F4;
                break;
            }
            break;
        case PLC_TASK_CHECK_DOC:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_CHECK_DOC_ADD_NODE_11F1_1:
                    //�����ӱ�Żظ����ϣ���ʼ��һ�¡����綫���·�ɣ���֧������ظ���š�
                 readport_plc.OnPortReadData = router_check_urgent_timeout;
                 if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)
                 {
                     portContext->need_reset_router = 1;/*ɾ��������ظ����ϣ����ŵĸ�·����һ�¹ܽţ����½�������ִ�в�����ʼ��*/
                 }
                 else
                 {
                    portContext->OnPortReadData = router_send_afn_01_F2;
                    portContext->cur_plc_task_step = PLC_CHECK_DOC_INIT_ROUTER_01F2_2;
                 }
                 break;
            case PLC_CHECK_DOC_DEL_NODE:
            case PLC_CHECK_DOC_INIT_ROUTER_01F2_1:
            case PLC_CHECK_DOC_INIT_ROUTER_01F2_2:
                 readport_plc.OnPortReadData = router_check_urgent_timeout;
                 portContext->need_reset_router = 1;/*ɾ��������ظ����ϣ����߲�����ʼ���ظ����ϣ�����·�����ܽţ����½���*/
                 break;
		    #if (defined __PROVICE_CHONGQING__)    
            case PLC_CHECK_NET_TOPOLOGY:
                 portContext->OnPortReadData = router_send_afn_10_F3;
                 break;
            #endif
            default:  //�����ȶԹ����У���������ʱ����10�Σ��ٲ�����ʼ�������±ȶԣ���ֹ��·��ż������ֱ�Ӹ���ʼ�������⡣
                portContext_plc.doc_cynchro_rev_deny_num ++;
                if(portContext_plc.doc_cynchro_rev_deny_num > 10)
                {
                    portContext_plc.doc_cynchro_rev_deny_num = 0;
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)
                    {
                         portContext->need_reset_router = 1;/*���ŵĸ�·����һ�¹ܽţ����½�������ִ�в�����ʼ��*/
                    }
                    else
                    {
                        portContext->OnPortReadData = router_send_afn_01_F2;
                        portContext->cur_plc_task_step = PLC_CHECK_DOC_INIT_ROUTER_01F2_2;
                    }
                    break;
                }
            }
            break;

        case PLC_TASK_READ_METER:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_READ_METER_WAIT_RESP_13F1:
           //     if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC09)
                {
                    //����
                    portContext->cur_plc_task = PLC_TASK_READ_METER;
                    portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
                    portContext->OnPortReadData = get_read_meter_info;
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                }
                break;
            case PLC_READ_METER_RESTART_ROUTER_12_F1: //�����������Ӧ������·��û�в�����ʱ������Ӧ
                  //��������ظ����ϣ���ͣ��
                     readport_plc.OnPortReadData = router_send_afn_12_F2;
                     portContext->OnPortReadData = router_send_afn_12_F2;
                 portContext->cur_plc_task = PLC_TASK_READ_METER;
                 portContext->cur_plc_task_step = PLC_READ_METER_RESTART_ROUTER_12_F1;
                #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
                portContext->batch_meter_ctrl.is_restart_node_yugao = 0;
                #endif
                 break;
            case PLC_READ_METER_PRIOR_11F8:   /*���ȶ�������ظ��˷��ϣ�ҲҪת�ɱ�������ģʽ���ȴ�·������*/
                 portContext->cur_plc_task_step = PLC_READ_METER_ROUTER_MODE;
                 readport_plc.OnPortReadData = router_check_urgent_timeout;
                 portContext->OnPortReadData = router_check_urgent_timeout;
                 break;
            }
            break;
        case PLC_TASK_READ_VIP_METER:
            switch(portContext->cur_plc_task_step)
            {
             case PLC_TOPSCOMM_VIP_METER_12HF1:
             case PLC_TOPSCOMM_VIP_METER_11HF8: //
                 readport_plc.OnPortReadData = router_check_urgent_timeout;
                 portContext->OnPortReadData = router_check_urgent_timeout;
                 portContext->cur_plc_task = PLC_TASK_READ_METER;
                 portContext->cur_plc_task_step = PLC_READ_METER_ROUTER_MODE;
                   break;
                case PLC_VIP_METER_12HF3:  //�������ͨģʽ�£������˻ָ������Ĵ������и���������������
                    //portContext->cur_plc_task_step = PLC_READ_METER_ROUTER_MODE;//���������˳���ģʽ
                    portContext->cur_plc_task = PLC_TASK_IDLE;//portContext->before_vip_task;
    				portContext->cur_plc_task_step = 0;//portContext->before_vip_task_step;
    				portContext->need_reset_router = 1;//��ͷ�� reset��ʼ
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_check_urgent_timeout;
					
                    break;				   
             default:
                  portContext->need_reset_router = 1; //�������������쳣��ֱ�����������¼���
                 break;
            }
            break;
        case PLC_TASK_PLC_NET:
               //�ѱ����̽���
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** �ѱ�������յ����󳭱������쳣�������ѱ�ͳ�ƣ�***");
            debug_println_ext(info);
            #endif
            //�����ѱ�ͳ��
            if(portContext->cur_plc_task_step == PLC_NET_END_PAUSE_ROUTER_12_F2)  	//��ͣ���ϣ�ֱ�������ѱ�ͳ��
                start_plc_net_stat(portContext);
            else
            {
              portContext->cur_plc_task = PLC_TASK_PLC_NET;
              portContext->cur_plc_task_step = PLC_NET_END_PAUSE_ROUTER_12_F2;
              portContext->OnPortReadData = router_check_urgent_timeout;
              readport_plc.OnPortReadData = router_send_afn_12_F2;
            }
            break;
        case PLC_TASK_CLERA_AREA_DIS:
             switch(portContext->cur_plc_task_step)
             {
             case PLC_CLEAR_AREA_DIS_FRAME_SEND:
                portContext->cur_plc_task_step = PLC_CLEAR_AREA_DIS_ROUTER_INIT; //������ʼ��
                readport_plc.OnPortReadData = router_send_afn_01_F2;
                break;
             case PLC_CLEAR_AREA_DIS_ROUTER_INIT:
               portContext_plc.need_reset_router = 1 ;  //�������ʶ������ظ����ϵ������ֹͣ��������·�ɣ�
               readport_plc.OnPortReadData = router_check_urgent_timeout;
               portContext->OnPortReadData = router_check_urgent_timeout;
               break;
             default:
               portContext_plc.need_reset_router = 1 ;  //�������ʶ������ظ����ϵ������ֹͣ��������·�ɣ�
               readport_plc.OnPortReadData = router_check_urgent_timeout;
               portContext->OnPortReadData = router_check_urgent_timeout;
               break;
             }
             break;
        case PLC_TASK_PARALLEL_READ:
             err_type = portContext->frame_recv[portContext->recv_data_pos];
             if(err_type == 12)/*��������12����ʾ�ӽڵ㲻����*/
             {
                 for(idx=1;idx<=PARALL_MAX_CHANNEL_COUNT;idx++)
                 {
                   if(portContext->frame_recv[POS_DL69842_SEQ] == portContext->router_phase_work_info[idx].frame_number)
                   {
                      // mem_set((INT8U*)(&portContext->router_phase_work_info[idx].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
                       portContext->router_phase_work_info[idx].paral_read.parall_status.parall_idle = 1;
                       break;
                   }
                 }
             }
             break;
        }
    }
	return 0;
}

//��ʼ����F2����������ʼ��
INT8U router_send_afn_01_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_INIT,DT_F2,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    #ifdef __CJQ_ORDER_MODE__
    //�ɼ����±�˳�򳭶���
    if((portContext->router_base_info.router_info3.rtu_no_mode) || (portContext->router_base_info.router_info4.dzc_cvt_no_mode))
    {
       mem_set(cjq_read_info,(sizeof(CJQ_READ_INFO))*MAX_READ_INFO_CNT,0xFF);
    }
    #endif
    return portContext->frame_send_Len;
}

INT8U router_send_afn_02_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U gb_645_frame[260];
    //INT16U meter_idx;
    //READ_PARAMS read_params;
    //INT32U item;

    portContext = (PLCPortContext*)readportcontext;

    if(portContext->urgent_task)
    {
        gb_645_frame[0] = meter_protocol_2_router_protocol(portContext->urgent_task_meter_doc->protocol);      //ͨ��Э������
        gb_645_frame[1] = portContext->urgent_task_req_frame_len;      //���ĳ���L
        mem_cpy(gb_645_frame+2,portContext->urgent_task_req_frame,portContext->urgent_task_req_frame_len);

        router_376_2_set_aux_info(0,40,1,TRUE);

        if ((portContext->router_base_info.router_info3.rtu_no_mode) && (isvalid_meter_addr(portContext->urgent_task_meter_doc->rtu_no,FALSE)))
        {
            mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_meter_doc->rtu_no,6);
        }
        else if((portContext->router_base_info.router_info4.dzc_cvt_no_mode) && (isvalid_meter_addr(portContext->urgent_task_meter_doc->rtu_no,FALSE)))
        {
            mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_meter_doc->rtu_no,6);
        }
        else
        {
            mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_meter_doc->meter_no,6);
        }

        portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_TRANS,DT_F1,gb_645_frame,gb_645_frame[1]+2,portContext);

        readport_plc.OnPortReadData = router_urgent_task_send_idle;
        return portContext->frame_send_Len;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
        return 0;
    }                    
}

INT8U router_recv_afn_02_F1(objReadPortContext * readportcontext,DL69842_RRR *RRR)
{
    PLCPortContext* portContext;
    INT8U *meter_no,*frame_ptr;
    INT8U idx,FE_count,data_len;

    portContext = (PLCPortContext*)readportcontext;

    if ((portContext->cur_plc_task == PLC_TASK_READ_METER) && (portContext->cur_plc_task_step == PLC_READ_METER_WAIT_RESP_13F1))
    {
        //���645ǰ���ǲ�����FE
        portContext->router_work_info.phase = 0;
        data_len = portContext->frame_recv[portContext->recv_data_pos+1];
        if(data_len == 0)
        {
            portContext->params.task_read_data.has_fail_meter = TRUE;
            portContext->cur_plc_task = PLC_TASK_READ_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
            portContext->OnPortReadData = get_read_meter_info;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            return 0;
        }

        FE_count = 0;
        for(idx=0;idx<data_len;idx++)
        {
            if (portContext->frame_recv[portContext->recv_data_pos+2+idx] == 0x68) break;
            FE_count++;
        }

        data_len -= FE_count;
        meter_no = portContext->frame_recv+portContext->recv_data_pos+2+FE_count+1;
        frame_ptr = portContext->frame_recv+portContext->recv_data_pos+2+FE_count;
        if (compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,6) == 0)
        {
            //�жϱ���Ƿ�һ��
            save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
        }
        else if ((portContext->router_base_info.router_info3.rtu_no_mode) && (portContext->router_base_info.router_info3.rtu_frame_format))
        {
            //�ɼ���ģʽ
            if (compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6) == 0)
            {
                //�޸Ĳɼ���ģʽ����
                portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info.try_count = 0;
                data_len = trans_read_frame_cjq_mode_to_standard_format(frame_ptr,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no);
                save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
            }
        }

        if( RRR != NULL)/*������λ������*/
        {
            #ifdef __F170_REC_TIME_IS_DAY_HOLD_READ_TIME__
            if(portContext->router_phase_work_info[0].read_params.read_type == READ_TYPE_CYCLE_DAY) /*ɽ��ֻ���ն������ݲŸ���F170�������*/
            {
                update_meter_recstate(bin2_int16u(portContext->router_phase_work_info[0].read_params.meter_doc.meter_idx),
                COMMPORT_PLC,0,(RRR->phase_q.spec == 4) ? (0x80|RRR->phase_q.phase) : RRR->phase_q.phase,RRR->resp_relay_channel.relay,RRR->phase_q.value[1],TRUE);
            }
            #else
            update_meter_recstate(bin2_int16u(portContext->router_phase_work_info[0].read_params.meter_doc.meter_idx),
            COMMPORT_PLC,0,(RRR->phase_q.spec == 4) ? (0x80|RRR->phase_q.phase) : RRR->phase_q.phase,RRR->resp_relay_channel.relay,RRR->phase_q.value[1],TRUE);
            #endif
        }

        portContext->cur_plc_task = PLC_TASK_READ_METER;
        portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
        portContext->OnPortReadData = prepare_read_item_concentrator;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        return 0;
    }
    else
    {
        //��������
        data_len = portContext->frame_recv[portContext->recv_data_pos+1];
        FE_count = 0;
        for(idx=0;idx<data_len;idx++)
        {
            if (portContext->frame_recv[portContext->recv_data_pos+2+idx] == 0x68) break;
            FE_count++;
        }
        data_len -= FE_count;

        tpos_enterCriticalSection();
        if((portContext->urgent_task_resp_frame!=NULL) && (portContext->urgent_task_resp_frame_len>=data_len))
        {    
            portContext->urgent_task_resp_frame_len = data_len;
            mem_cpy(portContext->urgent_task_resp_frame,portContext->frame_recv+portContext->recv_data_pos+2+FE_count,portContext->urgent_task_resp_frame_len);
        }
        else
        {
            portContext->urgent_task_resp_frame_len = 0;
        }
        tpos_leaveCriticalSection();
        
        //����״̬������ȴ���һ�����������״̬
        urgent_task_in_wait_next_urgent_task(portContext);
    }
    return 0;
}

INT8U router_send_afn_03_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_QUERY,DT_F1,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

#if defined(__READ_PLC_NOISE__)
INT8U router_send_afn_03_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_QUERY,DT_F2,NULL,0,portContext);

    //readport_plc.OnPortReadData = router_urgent_task_send_idle;   //�ڽ��������г���
    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}

INT8U router_recv_afn_03_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT32U offset;
    C2_PLC_NOISE plc_noise;
    INT8U pos,index;

    portContext = (PLCPortContext*)readportcontext;

    //��������
    plc_noise.read_date[0] = ((datetime[MINUTE]/15)*15);
    plc_noise.read_date[1] = (datetime[HOUR]);
    plc_noise.read_date[2] = (datetime[DAY]);
    plc_noise.read_date[3] = (datetime[MONTH]);
    plc_noise.read_date[4] = (datetime[YEAR]);

    pos = portContext->recv_data_pos;
    mem_set(plc_noise.white_noise,sizeof(C2_PLC_NOISE)-5,0xFF);
    //������
    mem_cpy(plc_noise.white_noise,portContext->frame_recv+pos,1);
//    pos += 3;
//    //ɫ����
//    for(index=0;index<3;index++)
//    {
//        plc_noise.color_noise[index].noise = portContext->frame_recv[pos + index];
//        mem_cpy(plc_noise.color_noise[index].node,portContext->frame_recv + pos + 3 + index*6,6);
//    }

    offset = getPassedDays(2000+plc_noise.read_date[4],plc_noise.read_date[3],plc_noise.read_date[2]);
    offset = offset % 10;
    offset *= sizeof(C2_PLC_NOISE)*96;
    offset += plc_noise.read_date[1]*4*sizeof(C2_PLC_NOISE);
    offset += (plc_noise.read_date[0]/15)*sizeof(C2_PLC_NOISE);
    fwrite_array(FILEID_PLC_NOISE,offset,plc_noise.value,sizeof(C2_PLC_NOISE));

    //����״̬������ȴ���һ�����������״̬
    urgent_task_in_wait_next_urgent_task(portContext);
}
#endif //#if defined(__READ_PLC_NOISE__)

INT8U router_send_afn_03_F4(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_QUERY,DT_F4,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

INT8U router_recv_afn_03_F4(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    //INT32U ertu_addr;

    portContext = (PLCPortContext*)readportcontext;

    get_router_main_node_addr(portContext->params.task_check_main_node.main_node);

    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** devid : %02X%02X%02X%02X%02X%02X ***",
            portContext->params.task_check_main_node.main_node[5],
            portContext->params.task_check_main_node.main_node[4],
            portContext->params.task_check_main_node.main_node[3],
            portContext->params.task_check_main_node.main_node[2],
            portContext->params.task_check_main_node.main_node[1],
            portContext->params.task_check_main_node.main_node[0]);
    debug_println_ext(info);
    snprintf(info,100,"*** r--id : %02X%02X%02X%02X%02X%02X ***",
            portContext->frame_recv[portContext->recv_data_pos+5],
            portContext->frame_recv[portContext->recv_data_pos+4],
            portContext->frame_recv[portContext->recv_data_pos+3],
            portContext->frame_recv[portContext->recv_data_pos+2],
            portContext->frame_recv[portContext->recv_data_pos+1],
            portContext->frame_recv[portContext->recv_data_pos+0]);
    debug_println_ext(info);
    #endif

    if (compare_string(portContext->params.task_check_main_node.main_node,portContext->frame_recv+portContext->recv_data_pos,6) == 0)
    {
        //����Ҫ�������ڵ��ַ�������鵵������
        mem_cpy(gSystemInfo.plc_ver_info,portContext->params.task_check_main_node.main_node,6);
        mem_cpy(portContext->router_work_info.ADDR_SRC,portContext->params.task_check_main_node.main_node,6);

       // if (portContext->read_status.is_in_read_cycle)
        if(1)
        {

            if(portContext->router_base_info.router_info1.node_mode == 0)
            {
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            portContext->read_status.doc_chg = 0;
            start_read_meter(portContext);
            }
            else
            {
                start_check_meter_doc(portContext);
            }
        }
        else
        {
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            portContext->OnPortReadData = router_check_urgent_timeout;
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = 0;
        }
    }
    else
    {
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext->OnPortReadData = router_send_afn_05_F1;
        portContext->cur_plc_task_step = PLC_QUERY_VERSION_CTRL_05F1;
    }
	return 0;
}
INT8U router_send_afn_03_F21(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_QUERY,DT_F21,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}
INT8U router_recv_afn_03_F21(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;
    if((portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT)
    || (portContext->router_base_info.router_vendor == ROUTER_VENDOR_HR)
    || (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM) )
    {
       portContext->plc_other_read_mode = CYCLE_REC_MODE_PARALLEL;
    }
 //   if(portContext->cur_plc_task_step == PLC_QUERY_VERSION_PARALL_OK_NO)
    {
     readport_plc.OnPortReadData = router_check_urgent_timeout;
     portContext->OnPortReadData = router_send_afn_03_F4;
     portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F4;
    }
    return 0;
}

INT8U router_send_afn_03_F9(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U gb_cast[30],FE_count,idx;
    INT8U send_cast_len;

    portContext = (PLCPortContext*)readportcontext;
    FE_count = 0;
    for(idx=0;idx<portContext->cast_content_len;idx++)
    {
        if (portContext->cast_content[idx] == 0x68) break;
        FE_count++;
    }

    router_376_2_set_aux_info(0,40,0,TRUE);
    send_cast_len = (portContext->cast_content_len - FE_count);
    if(send_cast_len >28)
    send_cast_len = 28;

    mem_cpy(gb_cast+2,portContext->cast_content+FE_count,send_cast_len);
    gb_cast[0] = 0x02;
    gb_cast[1] = send_cast_len;

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_QUERY,DT_F9,gb_cast,send_cast_len+2,portContext);

    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}

INT8U router_recv_afn_03_F9(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    //INT32U ertu_addr;
    INT16U delay;
    INT8U  FE_count,idx;//gb_cast[20],
    INT8U  urgent_id = 0;
    INT8U  amend_reason = 0;
    INT8U send_cast_len;
    portContext = (PLCPortContext*)readportcontext;

    portContext = (PLCPortContext*)readportcontext;
    FE_count = 0;

    if(( PLC_CAST_DELAY_03_F9 != portContext->urgent_task_step) && (PLC_CAST_OOP_DELAY_03_F9 != portContext->urgent_task_step))//
    {
        return 0;
    }
    for(idx=0;idx<portContext->cast_content_len;idx++)
    {
        if (portContext->cast_content[idx] == 0x68) break;
        FE_count++;
    }
    send_cast_len = (portContext->cast_content_len - FE_count);
    delay = portContext->frame_recv[13];
    delay += portContext->frame_recv[14] << 8; //16U��delay��
    portContext_plc.router_phase_work_info[0].comm_time = delay;   //ͨѶʱ����
//  portContext->cast_content = portContext->frame_send;
    mem_cpy(portContext->cast_content,portContext->cast_content+FE_count,send_cast_len);//�������FEȥ����
    //portContext->cast_content_len = make_gb645_adj_time_frame(portContext->cast_content,NULL,delay); //�㲥Уʱ

    urgent_id = portContext->urgent_task_id;
    switch(urgent_id)
    {
        /*��վ�·��㲥Уʱ������delay���������վ�·��Ĺ㲥Уʱ���ĵ�ʱ������������*/
        case RECMETER_TASK_TRANS_CAST_TIME_FROM_STATION:
            if(PLC_CAST_OOP_DELAY_03_F9 == portContext->urgent_task_step)
            {
                amend_reason = AMEND_OOP_DATETIME_WITH_RECV_DELAY_TIME; //����oop��ʱ����
            }
            else
            {
            amend_reason = AMEND_DATETIME_WITH_RECV_DELAY_TIME;
            }
            break;
        /*��ȷ��ʱ���㲥Уʱ�ı����е�ʱ�����ݣ������ն�ʱ�䣬Ȼ���ٸ���delay������*/
        case RECMETER_TASK_TRANS_CAST_TIME:
        case RECMETER_TASK_TRANS_CAST_PRECISE_TIME:
            if(PLC_CAST_OOP_DELAY_03_F9 == portContext->urgent_task_step)
            {
                amend_reason = USE_TERMINAL_DATETIME_AND_AMEND_OOP_WITH_DELAY_TIME;
            }
            else
            {
            amend_reason = USE_TERMINAL_DATETIME_AND_AMEND_WITH_DELAY_TIME;
            }
            break;
        default:
            amend_reason = MESSAGE_NOT_AMEND;
            break;
    }
    if((amend_reason == AMEND_OOP_DATETIME_WITH_RECV_DELAY_TIME) || (amend_reason == USE_TERMINAL_DATETIME_AND_AMEND_OOP_WITH_DELAY_TIME))
    {
      //  #ifdef __READ_OOP_METER__
       //����oop����֡��������Ҫ����
        portContext->cast_content_len = amend_oop_adj_time_frame(portContext->cast_content, delay, amend_reason);//�㲥Уʱ
      //  #endif
        portContext->urgent_task_step = PLC_CAST_OOP_OPEN_05_F3;
    }
    else
    {
        portContext->cast_content_len = amend_gb645_adj_time_frame(portContext->cast_content,delay,amend_reason); //�㲥Уʱ
        portContext->urgent_task_step = PLC_CAST_OPEN_05_F3;
    }
    //��conent�������滻��
    readport_plc.OnPortReadData = router_send_afn_05_F3;
    portContext->urgent_task = PLC_TASK_URGENT_TASK;
    //step�Ѿ���ֵ��
	return 0;
}

INT8U router_send_afn_03_F10(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_QUERY,DT_F10,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

INT8U router_recv_afn_03_F10(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    GB3762_VENDOR* vendor_info;
    //INT32U router_soft_date;
    INT8U pos;
    INT8U rec_mode = 0;
    INT8U num;
    portContext = (PLCPortContext*)readportcontext;
    
    portContext->plc_other_read_mode = 0;
    portContext->parall_max_phase = 0;
    portContext->modul_id_read_flag = 0; /*��·�����½���һ�Σ������»�ȡһ��ģ��ID��Ϣ��*/
    if((portContext->frame_recv_Len) < 0x20)
    {
       readport_plc.OnPortReadData = router_check_urgent_timeout;
       portContext->OnPortReadData = router_send_afn_03_F1;
       portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F1;
       portContext->router_base_info.router_info1.cycle_rec_mode = CYCLE_REC_MODE_ROUTER;//�����ɶ��
    }
    else
    {
        pos = portContext->recv_data_pos;
        //�ӽڵ���Ϣģʽ
        portContext->router_base_info.router_info1.value = portContext->frame_recv[pos++];
        // portContext->router_base_info.router_info1.value = 0x71;
        portContext->router_base_info.router_info2.value = portContext->frame_recv[pos++];
        pos += 4;
        //�ӽڵ������ʱʱ�䣨��λS��
        portContext->router_base_info.max_monitor_meter_timeout_s = portContext->frame_recv[pos++];
        //�㲥�������ʱʱ�䣨��λS��
        mem_cpy(portContext->router_base_info.max_cast_task_timeout_s,portContext->frame_recv+pos,2);
        
        if(bin2_int16u(portContext->router_base_info.max_cast_task_timeout_s) > 300)//���̫��Ҳ����
        {
            portContext->router_base_info.max_cast_task_timeout_s[0] = 0x2D;  //301s
            portContext->router_base_info.max_cast_task_timeout_s[1] = 0x01;
        }
        portContext_plc.params.task_plc_net.last_minute[0] = 0x78;
        portContext_plc.params.task_plc_net.last_minute[1] = 0x00;  //Ĭ��120����
        
        pos += 2;
        //���֧�ֵ�376.2���ĳ���
        pos += 2;
        //�ļ�����֧�ֵ���󵥰�����
        pos += 2;
        //���������ȴ�ʱ��
        pos += 1;
        //���ڵ��ַ
        //    #ifndef __QGDW_CHECK__
        //    dl69842_checkset_mainnode(GPLC.frame+pos);   //��������ز����ڵ�
        //    #endif //#ifdef __QGDW_CHECK__
        pos += 6;
        //֧�ֵ����ӽڵ�����
        pos += 2;
        //��ǰ�ӽڵ�����
        pos += 2;
        //ͨ��ģ��ʹ�õ�376.2Э�鷢�����ڣ�BCD��
        pos += 3;
        //ͨ��ģ��ʹ�õ�376.2Э����󱸰����ڣ�BCD��
        pos += 3;
        //ͨ��ģ�鳧�̴��뼰�汾��Ϣ
        vendor_info = (GB3762_VENDOR *)(portContext->frame_recv+pos);
        mem_cpy(gSystemInfo.plc_ver_info+6,vendor_info->value,9);
        //pos += 9; //coverity�澯��ע�͵�����Ч�� ?????����Ҳûʹ��pos�����ʹ�ã��ſ��˴�
        #ifdef __BATCH_TRANSPARENT_METER_TASK__
        set_gua_meter_minute(vendor_info);
        #endif
        
        portContext->router_base_info.router_info3.add_meter_seq = 0;     //���·��ʱ����������
        portContext->router_base_info.router_info3.rec_delay_flag = 1;    //ͨѶ��ʱ����
        portContext->router_base_info.router_info3.comm_module_flag = 1;  //������ӵ�ַ��
        portContext->router_base_info.router_info3.rtu_no_mode = check_const_ertu_switch(CONST_ERTU_SWITCH_RTU_NO_MODE);
        portContext->router_base_info.router_info3.rtu_frame_format = check_const_ertu_switch(CONST_ERTU_SWITCH_RTU_FRAME_FORMAT);
        portContext->router_base_info.router_info3.plc_net_is_cast = check_const_ertu_switch(CONST_ERTU_SWITCH_PLC_NET_CAST_CJQ_SEARCH);
        portContext->router_base_info.router_info3.plc_net_add_null_cjq_2_doc = check_const_ertu_switch(CONST_ERTU_SWITCH_PLC_NET_ADD_NULL_CJQ);
        portContext->router_base_info.router_info3.plc_net_clear_file = check_const_ertu_switch(CONST_ERTU_SWITCH_PLC_NET_CLEAR_FILE);
        portContext->router_base_info.router_info4.monitor_afn_type = 0;  //ʹ�ü�ص�afn��0��afn13H-F1��1��afn02H-F1
        portContext->router_base_info.router_info4.afn_12H_is_valid = 1;  //AFN = 12H �Ƿ�֧�� 0:��֧�֣�1��֧��
        portContext->router_base_info.router_info4.dzc_cvt_no_mode = check_const_ertu_switch(CONST_ERTU_SWITCH_DZC_CVT_NO_MODE);
        portContext->router_base_info.router_13_or_09 = ROUTER_PROTOCOL_GB13762; //Ĭ��֧��13Э�飬�ظ�03HF10
        portContext->router_base_info.router_protocol = ROUTER_PROTOCOL_GB13762;
        portContext->router_base_info.router_vendor = ROUTER_VENDOR_UNKNOW; //δ����� ROUTER_NONE��ʾ
        
        portContext->read_status.router_report_complete = 0;
        
        portContext->watermeter_read_num_control = 0;
        read_custom_param(CONST_WATERMETER_READ_NUM,&num);
        if(num != 0xFF)
        {
            portContext->watermeter_read_num_control = num; //ˮ���ȱ����������ƹ���
        }

        if((vendor_info->name[0]=='S')  && (vendor_info->name[1]=='E'))  //����·�������⴦��1����֧��ȫ0�Ĵӽڵ㡣2���޸ò�����ʱ����ز���Ӧ��3���޲�����ʱ����������Ӧ
        {
            portContext->router_base_info.router_vendor = ROUTER_VENDOR_EASTSOFT;

        }
        else if((vendor_info->name[0]=='1')  && (vendor_info->name[1]=='0'))  //��˹��·�������⴦�� ���ر��˼���ӽڵ��ϱ�����
        {
            portContext->router_base_info.router_vendor = ROUTER_VENDOR_RISECOMM;
        }
        else if((vendor_info->name[0]=='F')  && (vendor_info->name[1]=='C'))  //��Ѷ�﷽�����·���������Ҫ��״̬��������
        {
            portContext->router_base_info.router_vendor = ROUTER_VENDOR_FC13;
        }
        else if((vendor_info->name[0]=='7')  && (vendor_info->name[1]=='3'))  //������·�ɣ�������һ��
        {
            portContext->router_base_info.router_vendor = ROUTER_VENDOR_NTSR;
        }
        else if((vendor_info->name[0]=='C')  && (vendor_info->name[1]=='T'))  //���ŷ�����������·�ɲ�����������C
        {
            //portContext->router_base_info.router_vendor = ROUTER_VENDOR_EASTSOFT;
            portContext->router_base_info.router_vendor = ROUTER_VENDOR_TOPSCOMM; //
            #ifdef __PLC_BPLC_AGG__
            if(portContext->router_base_info.router_info1.comm_mode == 1)
            {
                portContext->aggregate_auth = 1;
            }
            #endif
            if(portContext->router_base_info.router_info1.comm_mode == 2)  //�Ȱ�����ͨģʽ�ܣ����߲������ֳ�Ҫ���ȶ�Ϊ��
            {

                portContext->router_interactive_status.plc_net_not_allow = 1; //·�����������У����ſ�����������ѱ�����������ʱֻ�����ţ�����������֤
                portContext->parall_max_phase = PARALL_MAX_CHANNEL_COUNT;/*���ɿ������ã���������и������󣬸Ķ���Ŵ󼴿�*/
              //  portContext->router_base_info.router_info1.value = 0xB2;
            }
          //  portContext->router_base_info.router_info1.value = 0x71;

        }
        else if(((vendor_info->name[0]=='H')  && (vendor_info->name[1]=='R'))
        || ((vendor_info->name[0]=='H')  && (vendor_info->name[1]=='S')))
                                                                  //����Ѹ��
        {
            portContext->router_base_info.router_vendor = ROUTER_VEBDOR_HMXL ; //
            
            if(portContext->router_base_info.router_info1.comm_mode != 2)//�е�дֻ֧��·���������е绪�������ǵķ�����֧�֡����л��ɼ���������
            {
                portContext->router_base_info.router_info1.value = 0x71;
            }
        }
        else if((vendor_info->name[0]=='C')  && (vendor_info->name[1]=='F'))  //��Ѷ�﷽�����·���������Ҫ��״̬��������
        {
            portContext->router_base_info.router_vendor = ROUTER_VENDOR_FC13;
        }

        if(portContext->router_base_info.router_info1.comm_mode == 2) //�����03HF10�����Ǳ���0xB2��ʵ��Ҫ����������������С�
        {
            fread_ertu_params(EEADDR_SET_REC_MODE,&rec_mode,1);

            if( rec_mode == 0x11) //����
            {
               portContext->router_base_info.router_info1.value = 0xB2;

               if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT) //����ֻ���������߲���
               {
                  portContext->router_base_info.router_info1.value = 0x72;
               }
            }
            else if (rec_mode == 0x22) //����
            {
               portContext->router_base_info.router_info1.value = 0x72;
            }
            else  //����
            {
               portContext->router_base_info.router_info1.value = 0xF2;
               portContext->plc_other_read_mode = CYCLE_REC_MODE_PARALLEL;
            }

            portContext->parall_read_ready_ok = 0 ; //����׼��������ó�0.

            if((portContext->router_base_info.router_info3.rtu_no_mode) || (portContext->router_base_info.router_info3.rtu_frame_format)
            || (portContext->router_base_info.router_info4.dzc_cvt_no_mode)) //ˮ���ȱ�
            {
                //�ɼ���ģʽ���߲�����������δ������ģʽ���д���,Ĭ������
              //  portContext->router_base_info.router_info1.value = 0x72;
            }
           #ifdef __KENENG_DLMS__
            #ifdef __DLMS_PARALL_READ__
            #else
            portContext->router_base_info.router_info1.value = 0x72;
            portContext->plc_other_read_mode = 0;
            #endif
           #endif
            #ifdef __PROVICE_SICHUAN__
            fread_ertu_params(EEADDR_SET_F199, &(portContext->parall_max_phase), 1);
            #endif
            if(portContext->parall_max_phase == 0)
            {
                portContext->parall_max_phase = 5;
            }
            else if(portContext->parall_max_phase > PARALL_MAX_CHANNEL_COUNT)
            {
                portContext->parall_max_phase = PARALL_MAX_CHANNEL_COUNT;
            }

            portContext_plc.parall_max_item = 3;/*���һ������֧��3֡645,*/
        }
        else
        {
            //200601zhaoxi�����±�ͳһ�����жϿ���
            //#ifdef __PROVICE_JIANGXI__/*�ϵ�Ĭ����3����Ƶ�ɼ��ϱ����������խ��������Ҫ�ϱ�*/
            //INT8U active_flag = 0xAA;
            //fwrite_ertu_params(EEADDR_SET_F67+63, &active_flag, 1);//д�رձ�־
            //fwrite_ertu_params(EEADDR_SET_F67+62, &active_flag, 1);//д�رձ�־
            //fwrite_ertu_params(EEADDR_SET_F67+61, &active_flag, 1);//д�رձ�־
           //#endif
        }
        
        //����·�����͸���F67����
        #ifdef __PROVICE_JIANGXI__
        update_F67(portContext->router_base_info.router_info1.comm_mode == 2);
        #endif

            #ifdef __SHAANXI_TEST__
            portContext->router_base_info.router_info1.value = 0x72;
            portContext->plc_other_read_mode = 0;
            #endif

        switch(portContext->cur_plc_task)
        {
            case PLC_TASK_QUERY_VERSION:
                if(portContext->cur_plc_task_step == PLC_QUERY_VERSION_WAIT_REPORT_03F10)
                {
                    readport_plc.OnPortReadData = router_send_afn_00_F1;
                    portContext->OnPortReadData = router_check_urgent_timeout;
                }
                else if(portContext->cur_plc_task_step == PLC_QUERY_VERSION_QUERY_03F10)
                {
                    #ifdef __READ_MODULE_ID_PLAN__
                    /*�ĳ�03HF12,10HF112���飬һ��ģ�飬һ��оƬ
                    ���ڵ��߼��ǣ�ģ��ID��оƬID�������ļ����������*/
                    {
                        portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F12;
                        readport_plc.OnPortReadData = router_check_urgent_timeout;
                        portContext->OnPortReadData = router_send_afn_03_F12;
                    }
                    #else
                    {
                        readport_plc.OnPortReadData = router_check_urgent_timeout;
                        portContext->OnPortReadData = router_send_afn_03_F4;
                        portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F4;
                    }
                    #endif
                }
                break;
            default: //����״̬���յ�˵��·�ɲ�������λ�����¿�ʼ�ز�����
                if ((portContext->router_base_info.router_info1.comm_mode == 2))//���
                {
                    #ifdef __READ_MODULE_ID_PLAN__/* �����ʱ��ѯ ����Ͳ���Ҫ�� TODO: */
                    portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F12;
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_03_F12;
                    portContext->cur_plc_task = PLC_TASK_QUERY_VERSION;
                    #else
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_03_F4;
                    portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F4;
                    portContext->cur_plc_task = PLC_TASK_QUERY_VERSION;

                    #endif
                }
                break;
        }
    }
    return 0;
}

#ifdef __READ_MODULE_ID_PLAN__
/*
��ѯ�������ڵ�ͨ��ģ��ID����Ϣ
��ʽ:
���̴���:2byte
ID�ų��� 1B (�50byte)
ID�Ÿ�ʽ:1B
ID��:  m byte
*/
INT8U router_send_afn_03_F12(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    
    portContext = (PLCPortContext*)readportcontext;
    
    router_376_2_set_aux_info(0,40,0,TRUE);
    
    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_QUERY,DT_F12,NULL,0,portContext);
    
    readport_plc.OnPortReadData = router_wait_resp_frame;
    
    return portContext->frame_send_Len;
}

INT8U router_recv_afn_03_F12(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    
    GB3762_VENDOR* vendor_info;
    INT32U router_soft_date;
	INT16U recv_info_len = 0;
	INT16U read_info_len = 0;
    INT8U  pos;
	INT8U  value[64] = {0};//
	INT8U  tmp_val[64] = {0};
	INT8U  id_len;
	INT8U  idx = 0;
    INT8U flag = 0x00;
    
    portContext = (PLCPortContext*)readportcontext;

	pos = portContext->recv_data_pos;

	// ���̴���
	mem_cpy(value,portContext->frame_recv+pos,2);
	pos += 2;

	//len+��ʽ+id ( 1+1+m(m<=50) )copy and save 
	id_len = portContext->frame_recv[pos];
	if(id_len > 50) 
	{
	    id_len = 50;
	}
	mem_cpy(value+2,portContext->frame_recv+pos,id_len+2);
    #ifdef __PROVICE_HUBEI__
    value[3] = 1;/*��ʽĬ��1��bcd��ʽ����������Ҫ��*/
    #endif
	recv_info_len = id_len+4;

    fread_array(FILEID_RUN_DATA,FLADDR_ROUTER_ID_INFO,tmp_val,4);
    fread_array(FILEID_RUN_DATA,FLADDR_MODULE_ID_FILE_ID_FLAG,&flag,1);
    /* ������ 0x55 ����ûд�� */
    if(flag != 0x55) /* && (TRUE == check_is_all_FF(tmp_val,4)) */
    {
        //��һ�Σ�ֱ��д��
        fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_ID_INFO,value,recv_info_len);
        flag = 0x55;
        fwrite_array(FILEID_RUN_DATA,FLADDR_MODULE_ID_FILE_ID_FLAG,&flag,1);
    }
	else
	{
	    //
	    tmp_val[2] = (tmp_val[2]>50)?50: tmp_val[2];
	    read_info_len = tmp_val[2]+4;
		fread_array(FILEID_RUN_DATA,FLADDR_ROUTER_ID_INFO+4,tmp_val+4,tmp_val[2]);
		#ifdef __SOFT_SIMULATOR__
		//for(idx=0;idx<8;idx++)
		{
		    //
		    //value[4+idx] = idx;
		}
        //event_erc_43(tmp_val,read_info_len,value,recv_info_len,02); // ����ģ����
		#endif
		if( (read_info_len != recv_info_len) || (compare_string(value,tmp_val,recv_info_len) != 0) )
		{
		    //����ERC43
		    fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_ID_INFO,value,recv_info_len);
            #ifdef __PROVICE_HUNAN__
            #else
		    event_erc_43(tmp_val,read_info_len,value,recv_info_len,02); // ����ģ����
            #endif
		}
	}   
	
    //
    switch(portContext->cur_plc_task)
    {
    case PLC_TASK_QUERY_VERSION:      
        if(portContext->cur_plc_task_step == PLC_QUERY_VERSION_QUERY_03F12)
        {

            portContext->task_read_module_id.node_start_seq[0] = 0x01;
            portContext->task_read_module_id.node_start_seq[1] = 0x00;
            portContext->task_read_module_id.query_node_cnt = 1;
            portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_10F112_MAIN_NODE;
            readport_plc.OnPortReadData = router_wait_send_frame_complete;
            portContext->OnPortReadData = router_send_afn_10_F112;

        }
        break;
    }
	
    return 0;  
}

/*
��ѯ����ز�оƬ��Ϣ
��ʽ:
�ڵ���ʼ���:2 �ֽ�
�ڵ�����: 1�ֽ� 
*/
INT8U router_send_afn_10_F112(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    
    portContext = (PLCPortContext*)readportcontext;
    
    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F112,portContext->task_read_module_id.node_start_seq,3,portContext);
    
    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }
    
    return portContext->frame_send_Len;
}

INT8U router_recv_afn_10_F112(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;   
    INT32U offset = 0;
	SLAVE_NODE_ID_INFO node_id_info;
	SLAVE_NODE_ID_INFO last_node_id_info;
    INT16U node_count = 0;
    INT16U start_seq = 0;
    INT16U meter_idx = 0;
    INT16U save_pos = 0;
    INT16U pos = 0;
    INT8U count = 0;
    INT8U idx = 0;
    INT8U router_protocol;
	INT8U user_type = 0;
	INT8U flag = 0;
    INT8U last_id[35] = {0};/*��֮ǰ��10F7 �ظ���ʽ���� ������ 11+24(24 ���µ�ģ��ID�� )*/
    INT8U cur_id[35]  = {0};
    
    portContext = (PLCPortContext*)readportcontext;

	pos = portContext->recv_data_pos;

    mem_cpy(portContext->task_read_module_id.slave_node_count,portContext->frame_recv+pos,2);
    pos += 2;
    /* �ӽڵ������� �������ڵ��� ?? TODO: */
    node_count = bin2_int16u(portContext->task_read_module_id.slave_node_count);
    /* ������ʼ��� */
    start_seq = bin2_int16u(portContext->frame_recv+pos);
    pos += 2;
    count = portContext->frame_recv[pos];       //����Ӧ��Ĵӽڵ�����
    pos++;

    if(portContext->urgent_task) /*�������� ��ѯ�ӽڵ�  */
    {
    	for (idx=0;idx<count;idx++)
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** query node_id : %02x%02x%02x%02x%02x%02x ***",
                    portContext->frame_recv[portContext->recv_data_pos+pos+5],
                    portContext->frame_recv[portContext->recv_data_pos+pos+4],
                    portContext->frame_recv[portContext->recv_data_pos+pos+3],
                    portContext->frame_recv[portContext->recv_data_pos+pos+2],
                    portContext->frame_recv[portContext->recv_data_pos+pos+1],
                    portContext->frame_recv[portContext->recv_data_pos+pos+0]);
            debug_println_ext(info);
            #endif
    
            //clear to all 0xFF
            mem_set(node_id_info.value,sizeof(SLAVE_NODE_ID_INFO),0xFF);
    
    		mem_cpy(node_id_info.value,portContext->frame_recv+pos,33);
    		/* ������һ���ڵ��ID��Ϣ �̶����� 33 */
    		pos += 33;
            if( (0 == idx) && (1 == start_seq) )/* ·�ɻظ�����ʼ��� 1 ��֮���ID��Ϣ�������ڵ�ID��Ϣ */
            {
    		
                    /*��ѯ ���ڵ� */
                fread_array(FILEID_RUN_DATA,FLADDR_CHIP_ID_FILE_ID_FLAG,&flag,1);
                if(flag != 0xAA)
                {
                    /*����AA ��˵��֮ǰ��֧�ֻ����״ζ�ȡ ֱ��д�� ������ERC�¼� */
                    fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,node_id_info.value,33);
                    flag = 0xAA;
                    fwrite_array(FILEID_RUN_DATA,FLADDR_CHIP_ID_FILE_ID_FLAG,&flag,1);
                }
                else
                {
                    fread_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,last_node_id_info.value,33);
                    if(0 != compare_string(node_id_info.node_ic_id_info.value,last_node_id_info.node_ic_id_info.value,24) )
                    {
                        fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,node_id_info.value,33);
                    }
                }
            }
            else
            {
            //todo�����������м����ڣ������ڵĻ����洢��
            if(TRUE == memory_fast_index_find_node_no(READPORT_PLC,node_id_info.addr,&meter_idx,&save_pos,&router_protocol,&user_type))      
            {
                //�ɼ�����ô���� ?  TODO ???
                meter_idx &= FAST_IDX_MASK;
                if( (meter_idx >0) && (meter_idx <= MAX_METER_COUNT) )
    			{
    			    
				    //��һ��д�룬���ۻظ��ı����Ƕ�ȡ���ˣ�����û��ȡ������ֱ��д�룬��������¼�
				    offset = PIM_SLAVE_NODE_ID_START+(meter_idx-1)*PIM_SLAVE_NODE_ID_PER_SIZE;
				    fread_array(FILEID_SLAVE_NODE_ID,offset,last_node_id_info.value,33);
    				if( (check_is_all_FF(last_node_id_info.value,33) == TRUE) )
    				{
    				    //��д��ɣ�33���Ⱥ�Ķ����ǳ�0xFF
    				    fwrite_array(FILEID_SLAVE_NODE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
    				}
					else
					{
					    if(compare_string(last_node_id_info.addr,node_id_info.addr,6) != 0)
    					{
    					    /*��ַ��һ���ˣ�ֱ��д�룬����ɾ��������ʱ���Ƿ�Ͳ��ô�����???TODO ��Ҫ����
    					      �������¼�  */
    					    fwrite_array(FILEID_SLAVE_NODE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
    					}				    
    					else 
    					{
    					    // ���κ��ϴ�ID ��Ϣ ��һ������Ҫ����ERC�����洢���ε�ID��Ϣ
    					    #ifdef __SOFT_SIMULATOR__
                            //event_erc_43(last_node_id_info.value,11+last_node_id_info.node_id_len,node_id_info.value,11+node_id_info.node_id_len,0x04);
    						#endif
    					    //if( (compare_string(last_node_id_info.value,node_id_info.value,33) != 0)
    						//	|| (compare_string(last_node_id_info.node_ic_id_info.value,node_id_info.node_ic_id_info.value,24) != 0) )
    						if(compare_string(last_node_id_info.node_ic_id_info.value,node_id_info.node_ic_id_info.value,24) != 0)
    					    {
    					    	fwrite_array(FILEID_SLAVE_NODE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
                                /* ����10F7�ظ���ʽ�������ݣ������¼� */
                                
                                mem_cpy(last_id,last_node_id_info.addr,6);
                                last_id[6] = 0;
                                    mem_cpy(last_id+7,last_node_id_info.node_ic_id_info.head_1+15,2);/* ���̴��� */
                                last_id[9]= 24;/*ID ���� */
                                last_id[10] = 0x00;/*��ʽ ��� */
                                mem_cpy(last_id+11,last_node_id_info.node_ic_id_info.value,24);

                                mem_cpy(cur_id,node_id_info.addr,6);
                                cur_id[6] = 0;
                                    mem_cpy(cur_id+7,node_id_info.node_ic_id_info.head_1+15,2);/* ���̴��� */
                                cur_id[9]= 24;/*ID ���� */
                                cur_id[10] = 0x00;/*��ʽ ��� */
                                mem_cpy(cur_id+11,node_id_info.node_ic_id_info.value,24);
                                   #ifdef __PROVICE_GANSU__
                                  //����ģ�鲻�������¼�
                                   #else
                                event_erc_43(last_id,35,cur_id,35,0x04);
                                    #endif
    							
    					    }
    					}						
					}                
    			}
            }     
        }
    }
  }
  else
  {
        /*��ѯ ���ڵ� */
        mem_cpy(node_id_info.value,portContext->frame_recv+pos,33);
        fread_array(FILEID_RUN_DATA,FLADDR_CHIP_ID_FILE_ID_FLAG,&flag,1);
        if(flag != 0xAA)
        {
            /*����AA ��˵��֮ǰû��д�� ֱ��д�� ������ERC�¼� */
            fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,node_id_info.value,33);
            flag = 0xAA;
            fwrite_array(FILEID_RUN_DATA,FLADDR_CHIP_ID_FILE_ID_FLAG,&flag,1);
        }
        else
        {
            /* ֻ�ж�ID ��Ϣ ��ַ����Ϣ���ܻ����ڷ����仯 ��ʱ������ TODO ???? */
            fread_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,last_node_id_info.value,33);
            if(0 != compare_string(node_id_info.node_ic_id_info.value,last_node_id_info.node_ic_id_info.value,24) )
            {
                fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,node_id_info.value,33);
                /* TODO: */
                /* ����10F7�ظ���ʽ�������ݣ������¼� */
                                
                mem_cpy(last_id,last_node_id_info.addr,6);
                last_id[6] = 0;
                mem_cpy(last_id+7,last_node_id_info.node_ic_id_info.head_1+15,2);/* ���̴��� */
                last_id[9]= 24;/*ID ���� */
                last_id[10] = 0x00;/*��ʽ ��� */
                mem_cpy(last_id+11,last_node_id_info.node_ic_id_info.value,24);

                mem_cpy(cur_id,node_id_info.addr,6);
                cur_id[6] = 0;
                mem_cpy(cur_id+7,node_id_info.node_ic_id_info.head_1+15,2);/* ���̴��� */
                cur_id[9]= 24;/*ID ���� */
                cur_id[10] = 0x00;/*��ʽ ��� */
                mem_cpy(cur_id+11,node_id_info.node_ic_id_info.value,24);
                #ifdef __PROVICE_GANSU__
//���಻Ҫģ�����¼��ˣ�ֻ����оƬ
                #else
                event_erc_43(last_id,35,cur_id,35,0x04);
                #endif
            }            
        }        
    }
    //֧��03HF112
    portContext->router_read_id_ctrl.afn_03H_F112_flag = 1;
    portContext->check_10HF112_avilid = 0xAA;
	//���ս�������ִ��
	if (portContext->urgent_task)
    {
        //
        switch(portContext->urgent_task_step)
        {
            case PLC_URGENT_TASK_READ_CHIP_ID:
				start_seq = bin2_int16u(portContext->task_read_module_id.node_start_seq);
				/*�����ѯ����������ʱ����ʼ��Ŵ�����������û��������ظ�,���Գ��������� ���ٲ�ѯ�� . 18-04-18 */
                if ((count == 0) || (start_seq >= node_count)|| ((start_seq + ROUTER_OPT_NODE_COUNT) > node_count) )   //��ѯ��ɣ�����ȴ���һ����������״̬
                {

                    //��ѯ10HF31��
                    portContext->task_read_module_id.node_start_seq[0] = 1;
                    portContext->task_read_module_id.node_start_seq[1] = 0;
                    portContext->task_read_module_id.query_node_cnt = ROUTER_OPT_NODE_COUNT;    //ÿ�β�ѯROUTER_OPT_NODE_COUNT��
                    readport_plc.OnPortReadData = router_send_afn_10_F31;

                }
                else   //������Ҫ��ѯ�Ľڵ���Ϣ
                {
                    node_count = bin2_int16u(portContext->task_read_module_id.node_start_seq);
                    node_count += ROUTER_OPT_NODE_COUNT;
                    portContext->task_read_module_id.node_start_seq[0] = node_count;
                    portContext->task_read_module_id.node_start_seq[1] = node_count>>8;
                    portContext->task_read_module_id.query_node_cnt = ROUTER_OPT_NODE_COUNT;    //ÿ�β�ѯROUTER_OPT_NODE_COUNT��
        
        			// �������Ͳ�ѯ����
                    readport_plc.OnPortReadData = router_send_afn_10_F112;
                }
				break;
        }        
    } 
    else
    {
        switch(portContext->cur_plc_task)
        {
            case PLC_TASK_QUERY_VERSION:      
                if(portContext->cur_plc_task_step == PLC_QUERY_VERSION_QUERY_10F112_MAIN_NODE)
                {
                    readport_plc.OnPortReadData = router_check_urgent_timeout;                    
                    portContext->OnPortReadData = router_send_afn_03_F4;
                    portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F4;

                }
                break;
        }
    }

	return 0;
}
#endif

INT8U router_recv_afn_03_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    GB3762_VENDOR* vendor_info;

    INT8U pos;//,tmp[2];

    portContext = (PLCPortContext*)readportcontext;

    pos = portContext->recv_data_pos;

    //ͨ��ģ�鳧�̴��뼰�汾��Ϣ
    vendor_info = (GB3762_VENDOR *)(portContext->frame_recv+pos);
    mem_cpy(gSystemInfo.plc_ver_info+6,vendor_info->value,9);
    //pos += 9; //coverity�澯��ע�͵�����Ч�� ?????����Ҳûʹ��pos�����ʹ�ã��ſ��˴�

    portContext->router_base_info.router_info1.comm_mode = 1;
    portContext->router_base_info.router_info1.router_mng_mode = 1;
    portContext->router_base_info.router_info1.node_mode = 1;
    portContext->router_base_info.router_info1.cycle_rec_mode = CYCLE_REC_MODE_ROUTER;
    portContext->router_base_info.router_info2.delay_param_cast = 0;
    portContext->router_base_info.router_info2.delay_param_monitor = 0;
    portContext->router_base_info.router_info2.delay_param_router_read = 0;
    portContext->router_base_info.router_info2.fail_node_switch_mode = 1;
    portContext->router_base_info.router_info2.cast_cmd_confirm_mode = 0;
    portContext->router_base_info.router_info2.cast_cmd_channel_exec_mode = 0;
    portContext->router_base_info.router_info3.add_meter_seq = 1;
    portContext->router_base_info.router_info3.rec_delay_flag = 0;   //��delay_param_router_read����������ͬ������Ҫ�ϲ�
    portContext->router_base_info.router_info3.comm_module_flag = 1;
    portContext->router_base_info.router_info3.rtu_no_mode = check_const_ertu_switch(CONST_ERTU_SWITCH_RTU_NO_MODE);
    portContext->router_base_info.router_info3.rtu_frame_format = check_const_ertu_switch(CONST_ERTU_SWITCH_RTU_FRAME_FORMAT);
    portContext->router_base_info.router_info3.plc_net_is_cast = check_const_ertu_switch(CONST_ERTU_SWITCH_PLC_NET_CAST_CJQ_SEARCH);
    portContext->router_base_info.router_info3.plc_net_add_null_cjq_2_doc = check_const_ertu_switch(CONST_ERTU_SWITCH_PLC_NET_ADD_NULL_CJQ);
    portContext->router_base_info.router_info3.plc_net_clear_file = check_const_ertu_switch(CONST_ERTU_SWITCH_PLC_NET_CLEAR_FILE);
    portContext->router_base_info.router_info4.monitor_afn_type = 0;
    portContext->router_base_info.router_info4.afn_12H_is_valid = 1;
    portContext->router_base_info.router_info4.dzc_cvt_no_mode = check_const_ertu_switch(CONST_ERTU_SWITCH_DZC_CVT_NO_MODE);
    //�ӽڵ������ʱʱ�䣨��λS��
    portContext->router_base_info.max_monitor_meter_timeout_s = 90;    //Ĭ��90s��������·������
    //�㲥�������ʱʱ�䣨��λS��
    portContext->router_base_info.max_cast_task_timeout_s[0] = 0xFF;
    portContext->router_base_info.max_cast_task_timeout_s[1] = 0;

    #ifdef __PROVICE_HUNAN__
    portContext->router_base_info.router_13_or_09 = ROUTER_PROTOCOL_GB13762; // ���϶�����13Э�鴦��о����п����ܵ�����ط���
    portContext->router_base_info.router_protocol = ROUTER_PROTOCOL_GB13762;
    #else
    portContext->router_base_info.router_13_or_09 = ROUTER_PROTOCOL_GB3762; // 09Э�鴦��
    portContext->router_base_info.router_protocol = ROUTER_PROTOCOL_GB3762;
    #endif
    portContext->router_base_info.router_vendor = ROUTER_VENDOR_TOPSCOMM;

    portContext->need_reset_router = 0;    //��;�����������Ĭ�ϲ�����·��
    portContext->plc_wait_resp_long_time_10ms = 0;  //�ȴ�·�ɻظ�ʱ�䣬����32U�����̨��ʶ��·��4-10���ӻظ�����
    if((vendor_info->name[0]=='1')  && (vendor_info->name[1]=='0'))  //��˹��·�ɶ��������̴����оƬ���붼��01��
    {
    portContext->router_base_info.router_info1.node_mode = 0;
        portContext->router_base_info.router_vendor = ROUTER_VENDOR_RISECOMM;
        portContext->router_base_info.router_info1.cycle_rec_mode = CYCLE_REC_MODE_CONCENTRATOR;
    }
    else if((vendor_info->name[0]=='7')  && (vendor_info->name[1]=='3'))  //������·�ɶ��������̴����оƬ���붼��37��
    {
    portContext->router_base_info.router_info1.node_mode = 0;
    portContext->router_base_info.router_vendor = ROUTER_VENDOR_NTSR;
        portContext->router_base_info.router_info1.cycle_rec_mode = CYCLE_REC_MODE_CONCENTRATOR;
    }
    else if((vendor_info->name[0]=='X')  && (vendor_info->name[1]=='C'))  //���̣�
    {
    portContext->router_base_info.router_info1.node_mode = 1;
        portContext->router_base_info.router_info1.cycle_rec_mode = CYCLE_REC_MODE_CONCENTRATOR;
    portContext->router_base_info.router_vendor = ROUTER_VENDOR_XC0010;
    }
    else if((vendor_info->name[0]=='F')  && (vendor_info->name[1]=='C'))  //��Ѷ�
    {
    portContext->router_base_info.router_vendor = ROUTER_VENDOR_FC09;
        portContext->router_base_info.router_info1.comm_mode = 3;  //΢��������ͨ��
        portContext->router_base_info.router_info1.node_mode = 1;
        portContext->router_base_info.router_info1.cycle_rec_mode = CYCLE_REC_MODE_CONCENTRATOR;
        portContext->router_base_info.router_info4.monitor_afn_type = 1;    //ʹ��02H-F1
        portContext->router_base_info.router_info4.afn_12H_is_valid = 0;    //��֧��
        //�ӽڵ������ʱʱ�䣨��λS��
        portContext->router_base_info.max_monitor_meter_timeout_s = 60;
        portContext->router_base_info.router_info3.rtu_no_mode = 1;
    }
    else if((vendor_info->name[0]=='R')  && (vendor_info->name[1]=='Z'))  //�����º��
    {
        portContext->router_base_info.router_vendor = ROUTER_VENDOR_ZT;
        portContext->router_base_info.router_info4.monitor_afn_type = 1;    //ʹ��02H-F1
        portContext->router_base_info.router_info4.afn_12H_is_valid = 0;    //��֧��
        readport_plc.OnPortReadData = router_check_urgent_timeout;

        portContext->OnPortReadData = router_send_afn_10_F1;
        portContext->cur_plc_task_step = PLC_CHECK_DOC_QUERY_NODE_COUNT_10F1_1;
        portContext->cur_plc_task = PLC_TASK_CHECK_DOC;

        return 0;
    }
    else if((vendor_info->name[0]=='X')  && (vendor_info->name[1]=='J'))  //�麣��Ѷ
    {
        //·������ģʽ�����ǲ����Զ�����
    portContext->router_base_info.router_vendor = ROUTER_VENDOR_JX;
    }
    else if((vendor_info->name[0]=='S')  && (vendor_info->name[1]=='E'))  //����·�ɣ�09Э���·����Щ�ر�
    {
    portContext->router_base_info.router_vendor = ROUTER_VENDOR_EASTSOFT;
    }

    switch(portContext->cur_plc_task)
    {
    case PLC_TASK_QUERY_VERSION:
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_03_F4;
                    portContext->cur_plc_task_step = PLC_QUERY_VERSION_QUERY_03F4;
        break;
    }
    return 0;
}


//�������F1���������ڵ��ַ
INT8U router_send_afn_05_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_CTRL,DT_F1,portContext->params.task_check_main_node.main_node,6,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

INT8U router_send_afn_05_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_CTRL,DT_F2,&(portContext->params.task_read_data.ctrl_event_report),1,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

//�������F3�������㲥
INT8U router_send_afn_05_F3(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U buffer[100];

    portContext = (PLCPortContext*)readportcontext;

    buffer[0] = 0;
    if (portContext->cast_content)
    {
        if (portContext->cast_content_len < 99)
        {
            #ifdef __INSTANT_FREEZE__
            if( RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE == portContext->urgent_task_id)
            {
                buffer[0] = 0x0D; /* ���˲ʱ�����޸����� */
            }
            else
            #endif
            {
                buffer[0] = 0x02;
            }
            buffer[1] = portContext->cast_content_len;
            mem_cpy(buffer+2,portContext->cast_content,portContext->cast_content_len);
        }
    }

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_CTRL,DT_F3,buffer,buffer[1]+2,portContext);

    if(portContext->urgent_task)
    {
        #ifdef __INSTANT_FREEZE__
        if( RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE == portContext->urgent_task_id)
        {
            portContext->bplc_instant_freeze_flg = 0xAA;
            urgent_task_in_wait_next_urgent_task(portContext);
            portContext->cast_bplc_insfrz_time_10ms = os_get_systick_10ms();
        }
        else
        #endif
        {
            readport_plc.OnPortReadData = router_urgent_task_send_idle;
        }
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}
//�������F6��̨��ʶ�������߹رգ�
INT8U router_send_afn_05_F6(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U   mode = 0;

    portContext = (PLCPortContext*)readportcontext;

  //  fread_array(FILEID_RUN_PARAM,FLADDR_AUTO_MNG_METER_SWITCH,&mode,1);

   // if(mode > 3) mode = 0;
   switch(portContext -> hplc_area_distinguish)
   {
       case 0x55:    /*�ر�̨������*/
            mode = 0;
            break;
       case 0xAA:
            mode = 1;
            break;
   }

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_CTRL,DT_F6,&mode,1,portContext);

    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}
//��չ���F14��̨��ʶ��
INT8U router_send_afn_F0_F14(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U frameData[16];

    portContext = (PLCPortContext*)readportcontext;
   //���ʶ������
    frameData[0] = 0x68;
    frameData[1] = 0x10;
    frameData[2] = 0x00;
    frameData[3] = 0x41;
    frameData[4] = 0x00;
    frameData[5] = 0x00;
    frameData[6] = 0x28;
    frameData[7] = 0x32;
    frameData[8] = 0x00;
    frameData[9] = 0x00;
    frameData[10] = 0xF0;
    frameData[11] = 0x20;
    frameData[12] = 0x01;
    frameData[13] = 0x03;
    frameData[14] = 0xAF;
    frameData[15] = 0x16;

    router_376_2_set_aux_info(0,40,0,TRUE);
    mem_cpy(portContext->frame_send,frameData,16);
    portContext->frame_send_Len = 16;

    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}
//�ϱ��ӽڵ���Ϣ
INT8U router_recv_afn_06_F1(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U idx,count;

    portContext = (PLCPortContext*)readportcontext;

    portContext->plc_net_timer_10ms = os_get_systick_10ms();

    count = portContext->frame_recv[portContext->recv_data_pos];

    switch(portContext->cur_plc_task)
    {
    case PLC_TASK_PLC_NET:
        for(idx=0;idx<count;idx++)
        {
            plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                    portContext->frame_recv+portContext->recv_data_pos+1+idx*9,
                    portContext->frame_recv[portContext->recv_data_pos+1+idx*9+6],NULL,0,
                    portContext->params.task_plc_net.run_params.save_mode,
                    portContext->params.task_plc_net.run_params.file_idx,
                    portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);
        }
        router_send_afn_00_F1_no_change_status(readportcontext);//����Ҫ��һ���������ò��л�״̬��ȷ��֡
        break;
    default:
        for(idx=0;idx<count;idx++)
        {        
            if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM) //���ŵ������Σ�������ʱ�ϱ������������еĲ�ͣ�ϱ�����ȷ��Ҳ�����ã�����ֻ������
            {
                 plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                         portContext->frame_recv+portContext->recv_data_pos+1+idx*9,
                         portContext->frame_recv[portContext->recv_data_pos+1+idx*9+6],NULL,0,
                         portContext->params.task_plc_net.run_params.save_mode,
                         portContext->params.task_plc_net.run_params.file_idx,
                         portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);
            }
        }
       router_send_afn_00_F1_no_change_status(readportcontext);  //��ȷ�ϣ���������
       break;
    }





    return 0;
}

INT8U router_recv_afn_06_F2(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U *meter_no,*frame_ptr;
    INT8U idx,FE_count,data_len,delay_flag;


    portContext = (PLCPortContext*)readportcontext;



    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
    delay_flag = 0;
    else
    delay_flag = 2;
    //���645ǰ���ǲ�����FE
    data_len = portContext->frame_recv[portContext->recv_data_pos+delay_flag+3];
    FE_count = 0;
    for(idx=0;idx<data_len;idx++)
    {
        if (portContext->frame_recv[portContext->recv_data_pos+delay_flag+4+idx] == 0x68) break;
        FE_count++;
    }
    data_len -= FE_count;
    frame_ptr = portContext->frame_recv+portContext->recv_data_pos+delay_flag+4+FE_count;
    #ifdef __READ_OOP_METER__
    /* 
     * ֻ����ʽ�Ƿ���oop,�������ݸ�ʽ������oop����645
     */
    if(TRUE == check_frame_body_gb_oop(frame_ptr,data_len)) 
    {
        meter_no = portContext->frame_recv+portContext->recv_data_pos+delay_flag+4+FE_count+5;
    }
    else
    #endif
    {
        meter_no = portContext->frame_recv+portContext->recv_data_pos+delay_flag+4+FE_count+1;
    }
    portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_phase = portContext->router_work_info.phase;

    if ((compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,6) == 0)
      || (check_other_phase_same_meter_no_after_receive(portContext,meter_no,0x55))) //��ͨ�ز���洢
    {
        //#ifdef __ROUTER_JXZB__
        //if(portContext->router_base_info.router_vendor != ROUTER_VENDOR_TOPSCOMM)
        portContext->router_phase_work_info[portContext->router_work_info.phase].try_count = 0;
        //#endif
        #ifdef __MEXICO_CIU__
        portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.snd_rail_2_ciu = 0;
        #endif
        //�жϱ���Ƿ�һ��
        portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_phase = portContext->router_work_info.phase;
        save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
    }
    else if((portContext->router_base_info.router_info3.rtu_no_mode) && (portContext->router_base_info.router_info3.rtu_frame_format)) /*�������ж�Э��*/
    {
        //�ɼ���ģʽ
        if ((compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6) == 0)
        || (check_other_phase_same_meter_no_after_receive(portContext,meter_no,0xAA)) ) //�ɼ���ģʽ�洢
        {
            //�޸Ĳɼ���ģʽ����
            portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info.try_count = 0;
            if(!check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol))
            {
            data_len = trans_read_frame_cjq_mode_to_standard_format(frame_ptr,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no);
            }
            save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
        }
    }
    else if((portContext->router_base_info.router_info3.rtu_no_mode) && (!check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol)))//���˳�򳭶�
    {
        if (compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6) == 0)
        {
            #ifdef __CJQ_ORDER_MODE__
            if(save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len))
            {
                for(idx=0;idx<MAX_READ_INFO_CNT;idx++)
                {
                    if(0==compare_string(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,cjq_read_info[idx].cjq_no,6) )
                    {
                         cjq_read_info[idx].comm_ok = 1;
                        fwrite_array(FILEID_PLC_REC_TMP,PIM_CJQ_READ_INFO+idx*(sizeof(CJQ_READ_INFO)),cjq_read_info[idx].value,sizeof(CJQ_READ_INFO));
                    }
                }
            }
            #else
            save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
            #endif
        }
    }
    else if ((portContext->router_base_info.router_info4.dzc_cvt_no_mode) && (check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol)))//ˮ���ȱ�˳�򳭶�
    {
        if (compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6) == 0)
        {
            #ifdef __CJQ_ORDER_MODE__
            if(save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len))
            {
                for(idx=0;idx<MAX_READ_INFO_CNT;idx++)
                {
                    if(0==compare_string(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,cjq_read_info[idx].cjq_no,6) )
                    {
                         cjq_read_info[idx].comm_ok = 1;
                        fwrite_array(FILEID_PLC_REC_TMP,PIM_CJQ_READ_INFO+idx*(sizeof(CJQ_READ_INFO)),cjq_read_info[idx].value,sizeof(CJQ_READ_INFO));
                    }
                }
            }
            #else
            save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
            #endif
        }
    }
    #ifdef __SH_2009_METER__
    else if (portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol == SHANGHAI_2009)
    {
        if (compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,5) == 0)
        {
            save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
        }
    }
    #endif

    if(portContext->urgent_task)
    {
         router_send_afn_00_F1_no_change_status(readportcontext);/*����Ҫ��һ���������ò��л�״̬��ȷ��֡*/
    }
    else
    {
        switch(portContext->cur_plc_task)
        {
         case PLC_TASK_READ_METER:
             switch(portContext->cur_plc_task_step)
             {
             case PLC_READ_METER_ROUTER_MODE:
                  readport_plc.OnPortReadData = router_send_afn_00_F1;
                  portContext->OnPortReadData = router_check_urgent_timeout;
                  break;
             }
             break;
         }
    }
    return 0;
}

//�ϱ�������Ϣ
INT8U router_recv_afn_06_F3(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;

    if (portContext->frame_recv[portContext->recv_data_pos] == 1)   //�������
    {

       router_send_afn_00_F1_no_change_status(readportcontext); //��ҪӰ��֮ǰ��״̬

        if(portContext->router_base_info.router_info1.cycle_rec_mode != CYCLE_REC_MODE_ROUTER) return 0; //��·������ģʽʱ���Ƿ���ɲ���·��Ϊ׼
       
        if(COMMPORT_PLC_REC > COUNT_OF_READPORT) return 0;

        if(bin2_int16u(llvc_rec_state[COMMPORT_PLC_REC-1].total_count) != 0)
       // if(0)
        {
            llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.value = 0;
            llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.finish = 1;
            llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.c2f55_stat_flag = 1;
            mem_cpy(llvc_rec_state[COMMPORT_PLC_REC-1].end_datetime,datetime,6);
            fwrite_array(FILEID_RUN_DATA,FLADDR_LLVC_READ_STATE+(COMMPORT_PLC_REC-1)*sizeof(LLVC_REC_STATE),
            llvc_rec_state[COMMPORT_PLC_REC-1].value,sizeof(LLVC_REC_STATE));
            if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM )

            portContext->read_status.router_report_complete = 1;
            #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
                portContext->batch_meter_ctrl.is_restart_node_yugao = 1;
                #endif
        }
    }
    else if (portContext->frame_recv[portContext->recv_data_pos] == 2) //�ѱ����
    {
        switch(portContext->cur_plc_task)
        {
        case PLC_TASK_PLC_NET:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_NET_WAIT_NODE_LOGON:
                readport_plc.OnPortReadData = router_send_afn_00_F1;
                portContext->OnPortReadData = router_check_urgent_timeout;
                portContext->cur_plc_task_step = PLC_NET_REPORT_06_F3;
                break;
            default:
               router_send_afn_00_F1_no_change_status(readportcontext);
               break;
            }
            break;
        default:
            router_send_afn_00_F1_no_change_status(readportcontext);
            break;
        }
    }


    return 0;
}

//�ϱ��ӽڵ���Ϣ
INT8U router_recv_afn_06_F4(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U *rtu_no;
    INT8U meter_no[6]={0};
    INT8U idx,idx1,count_z,count,cjq_type,pos,prop;

    portContext = (PLCPortContext*)readportcontext;

    portContext->plc_net_timer_10ms = os_get_systick_10ms();
    #ifdef  __NEIMENG_09_3762__
    if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_ZT)
    {
//       readport_plc.OnPortReadData = router_send_afn_00_F1; //����ȷ���ˣ�С���߻��ϱ�������ȷ��Ҳ�����Ƴ���
       return 0;//С���߲����������ϱ����
    }
    #endif
    pos = 0;
    count_z = portContext->frame_recv[portContext->recv_data_pos+pos];

    switch(portContext->cur_plc_task)
    {
        case PLC_TASK_PLC_NET:

            for(idx=0;idx<count_z;idx++)
            {
                pos ++;
                rtu_no = portContext->frame_recv+portContext->recv_data_pos+pos;
                pos += 6;
                //ͨ��Э������
                pos++;
                //�ӽڵ����
                pos += 2;
                //�豸����
                cjq_type = portContext->frame_recv[portContext->recv_data_pos+pos];
                pos++;
                //�½Ӵӽڵ�����
                count = portContext->frame_recv[portContext->recv_data_pos+pos];
                pos++;
                //�����Ĵ���Ľڵ�����
                count = portContext->frame_recv[portContext->recv_data_pos+pos];
                pos++;

                if(cjq_type == 0x01) //ֻ�ܴ���һ�����ڵ㣬�������Ҳû�дӽڵ㣬���������ı���
                {
                    if(count == 1) //������滹�е������·������,ȥ�¹ҵĵ���Э��
                    {
                        mem_cpy(meter_no,portContext->frame_recv+portContext->recv_data_pos+pos,6); //���
                        pos += 6;
                        prop = portContext->frame_recv[portContext->recv_data_pos+pos]; //Э��
                        pos++;
                        plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                        meter_no,prop,NULL,0,
                        portContext->params.task_plc_net.run_params.save_mode,
                        portContext->params.task_plc_net.run_params.file_idx,
                        portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);

                    }
                    else
                    {
                        plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                        portContext->frame_recv+portContext->recv_data_pos+1,
                        portContext->frame_recv[portContext->recv_data_pos+1+6],NULL,0,
                        portContext->params.task_plc_net.run_params.save_mode,
                        portContext->params.task_plc_net.run_params.file_idx,
                        portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);
                    }
                }
                else
                {
                    for(idx1=0;idx1<count;idx1++)
                    {
                        mem_cpy(meter_no,portContext->frame_recv+portContext->recv_data_pos+pos,6);
                        pos += 6;
                        prop = portContext->frame_recv[portContext->recv_data_pos+pos];
                        pos++;
                        plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                                meter_no,prop,rtu_no,cjq_type,portContext->params.task_plc_net.run_params.save_mode,
                                portContext->params.task_plc_net.run_params.file_idx,
                                portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);
                    }

                    if ((count == 0) && (portContext->router_base_info.router_info3.plc_net_add_null_cjq_2_doc))
                    {
                        mem_cpy(meter_no,rtu_no,6);
                        meter_no[5] |= 0xA0;
                        plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                                meter_no,0,rtu_no,cjq_type,portContext->params.task_plc_net.run_params.save_mode,
                                portContext->params.task_plc_net.run_params.file_idx,
                                portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);
                    }
                }
            }
            readport_plc.OnPortReadData = router_send_afn_00_F1;
            portContext->OnPortReadData = router_check_urgent_timeout;

            break;
        default:
         //  if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM) //�����ϱ���������ʱ�ϱ���
           {
              for(idx=0;idx<count_z;idx++)
              {
                  pos ++;
                  rtu_no = portContext->frame_recv+portContext->recv_data_pos+pos;
                  pos += 6;
                  //ͨ��Э������
                  pos++;
                  //�ӽڵ����
                  pos += 2;
                  //�豸����
                  cjq_type = portContext->frame_recv[portContext->recv_data_pos+pos];
                  pos++;
                  //�½Ӵӽڵ�����
                  count = portContext->frame_recv[portContext->recv_data_pos+pos];
                  pos++;
                  //�����Ĵ���Ľڵ�����
                  count = portContext->frame_recv[portContext->recv_data_pos+pos];
                  pos++;
                  if(cjq_type == 0x01) //ͨ��06HF4�ϱ������Ǳ����ǵ�����ŷ�����������������������ϱ�������
                  {
                      if(count == 1) //������滹�е������·������,ȥ�¹ҵĵ���Э��
                      {
                          mem_cpy(meter_no,portContext->frame_recv+portContext->recv_data_pos+pos,6); //���
                          pos += 6;
                          prop = portContext->frame_recv[portContext->recv_data_pos+pos]; //Э��
                          pos++;
                          plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                          meter_no,prop,NULL,0,
                          portContext->params.task_plc_net.run_params.save_mode,
                          portContext->params.task_plc_net.run_params.file_idx,
                          portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);

                      }
                      else
                      {
                          plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                          portContext->frame_recv+portContext->recv_data_pos+1,
                          portContext->frame_recv[portContext->recv_data_pos+1+6],NULL,0,
                          portContext->params.task_plc_net.run_params.save_mode,
                          portContext->params.task_plc_net.run_params.file_idx,
                          portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);
                      }
                  }
                  else
                  {
                      for(idx1=0;idx1<count;idx1++)
                      {
                          mem_cpy(meter_no,portContext->frame_recv+portContext->recv_data_pos+pos,6);
                          pos += 6;
                          prop = portContext->frame_recv[portContext->recv_data_pos+pos];
                          pos++;
                          plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                                  meter_no,prop,rtu_no,cjq_type,portContext->params.task_plc_net.run_params.save_mode,
                                  portContext->params.task_plc_net.run_params.file_idx,
                                  portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);
                      }

                      if ((count == 0) && (portContext->router_base_info.router_info3.plc_net_add_null_cjq_2_doc))
                      {
                          mem_cpy(meter_no,rtu_no,6);
                          meter_no[5] |= 0xA0;
                          plc_router_reg_node(portContext->router_work_info.phase+1,portContext->router_work_info.relay,
                                  meter_no,0,rtu_no,cjq_type,portContext->params.task_plc_net.run_params.save_mode,
                                  portContext->params.task_plc_net.run_params.file_idx,
                                  portContext->params.task_plc_net.stamp,portContext->plc_net_buffer);
                      }
                  }
              }
           }
           router_send_afn_00_F1_no_change_status(readportcontext);  //��ȷ�ϣ���������
           break;
    }


    return 0;
}
/*·���¼��ϱ�*/
INT8U router_recv_afn_06_F5(objReadPortContext * readportcontext)
{
    INT32U 	item;
	INT32U  resp_frm_item;
	INT32U  expect_item[] = {0x04001501,0x04001507};
    static INT32U tick = 0;
    PLCPortContext *portContext;
    INT8U *meter_no,*frame_ptr;
    INT8U *frame_event_ptr = NULL;// ���ڵ�ͣ���ϱ� ʹ�� �� 00 01 �ɼ��� ���ܱ����ֿ���ʹ�õ���ָ��
    INT16U meter_idx = 0;
    INT16U pos_idx;
    INT16U pos;
    INT8U idx,FE_count,data_len,event_data_len;
    INT8U router_protocol;
    INT8U node_type;
    INT8U comm_protocol = 0;
    INT8U buffer_meter[120]={0};
    #ifdef __MEXICO_CIU__/* �����ĵ�ַ */
    INT8U guide_rail_addr[6] = {0};
    #endif
    INT8U flag = 0;
    INT8U i;
    BOOLEAN exp_flg = FALSE;

    portContext = (PLCPortContext*)readportcontext;
    //    if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC13)//С���ߵ��ϱ����Ȳ�������
    //    return 0;
    
    pos = 0;
    FE_count = 0;
    
    node_type = portContext->frame_recv[portContext->recv_data_pos+pos];   //�ӽڵ��豸����
    pos++;
    comm_protocol = portContext->frame_recv[portContext->recv_data_pos+pos];  //ͨ��Э������
    pos++;
    data_len = portContext->frame_recv[portContext->recv_data_pos+pos];      //���ĳ���L
    event_data_len = portContext->frame_recv[portContext->recv_data_pos+pos];      //���ĳ���L  ���ڵ�ͣ���ϱ� ʹ�� 
    pos++;
    frame_event_ptr = portContext->frame_recv+portContext->recv_data_pos+pos;

    for(idx=0;idx<data_len;idx++)
    {
        if (portContext->frame_recv[portContext->recv_data_pos+pos+idx] == 0x68) break;
        FE_count++;
    }
    data_len -= FE_count;//ȥ��FE ����
    
    frame_ptr = portContext->frame_recv+portContext->recv_data_pos+pos+FE_count;// 645 ������ʼλ��
    if((data_len == 0)||(check_frame_body_gb645(frame_ptr,data_len)==FALSE))
    {
        frame_ptr = NULL;
        meter_no = NULL;
        data_len = 0;
    }
    else
    {
        meter_no = frame_ptr+1;
    }


	//copy ��λ��Ϣ
	portContext->report_meter_phase = METER_EVENT_REPORT_CHANNEL;/*��������¼��ϱ���ʹ�����ͨ��*/

    if ( (data_len > 0) || (event_data_len > 0) )
    {
        if(frame_ptr != NULL)    //�ϱ�Ϊ645���ģ���ͣ�ϵ籨��
        {
            if ((frame_ptr[POS_GB645_CTRL] == 0x9E) && ((frame_ptr[POS_GB645_ITEM] == 0x03) ||(frame_ptr[POS_GB645_ITEM] == 0x36))  ) //�ϱ�����̨����Ϣ
            {
                #if defined(__SHANXI_READ_BPLC_NETWORK_INFO__)
                if(portContext->router_base_info.router_info1.comm_mode  == 2)
                {
                    save_area_sta_change_info(frame_ptr);
                }
                else
                {
                    save_area_different_info(frame_ptr,comm_protocol);
                }
                #else
                    save_area_different_info(frame_ptr,comm_protocol);
                #endif
            }
            else if (frame_ptr[POS_GB645_CTRL] == 0x9F  ) //�ϱ����ʱ���쳣
            {
                  #ifdef __PROVICE_HUNAN__
                  if(memory_fast_index_find_node_no(COMMPORT_PLC,meter_no,&meter_idx,&pos_idx,&router_protocol,NULL))
                  {
                        meter_idx &= FAST_IDX_MASK;
                        cco_report_erc12(meter_idx);
                  }
                  #endif
            }
            else
            {
                if (node_type == 0) //�ɼ�����·���ϱ������ݣ�����Ҫ�ж��Ƿ�ɼ���ģʽ
                {
                    #ifdef __METER_EVENT_REPORT__
                    //<TODO: >     �¼��ϱ�
                    return 0;//�ɼ���ģʽ��ʱ������
                        
                    //is_event_report = TRUE;
                    /*
                    item = 0xC3;
                    mem_cpy(&(portContext->router_phase_work_info[0].read_params.item),&item,4);
                    //mem_cpy(&meter_idx_no,0,16);
                    num = 0;
                    //·��һ���ϱ����8������״̬��
                    count = ((buffer[15]-0x33) > 8) ? 8 : (buffer[15]-0x33);
                    //node_info.count = 0;
                    for( i=0;i<12*count;i++)
                        buffer_meter[i]=(buffer[i+16]-0x33); //����33��
                        for(idx=0;idx<count;idx++)//Ҫѭ�������ϱ��Ŀ�����
                        {
                        if(memory_fast_index_find_node_no(COMMPORT_PLC,buffer_meter+idx*8,&meter_idx,&rec_pos,&router_protocol,NULL))
                        //if (compare_string(buffer_meter+idx*8,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,6) == 0)
                        // if (find_meter_no_from_fast_index(buffer_meter+idx*8,&meter_idx,&pos_idx) == TRUE)//ȡ����ַ�Աȣ����ܴ�bufferȥ��λ�ò���
                        {                                             //�˴��õ���pos_idx����ʱ��֪������
                            mem_cpy(meter_no,buffer_meter,6);
                            meter_idx &= FAST_IDX_MASK;
                            //char  test[6];
                            //��ʱ�洢һ�²����㣬���ں���ļ�شӽڵ�
                            meter_idx_no[num] = meter_idx;
                            meter_idx_no[num+1]= meter_idx>>8;
                            num += 2;
                            if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
                            {
                                flag = 0xFE;
                                cjq_flag = 0xFC;//�ɼ����ı�ʶλ
                                fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
                                fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE+3,&cjq_flag,1);
                                /item = GPLC.router_info[GPLC.phase-1].item;
                                //GPLC.router_info[GPLC.phase-1].item = 0xC3;//�˴�����˼�ǣ������ݱ�ʶд��ȥ��Ȼ��ȥ�洢���͵��������ݵ�����ʹ�á��˴����ϱ��ģ�GPLC����û�г��������ݱ�ʶ
                                //GPLC.router_info[GPLC.phase-1].rec_ctrl_info.is_report_cjq_event = 1;
                                item = 0xC3;
                                plc_router_save_cjq_meter_event_data(meter_idx,meter_no,item,buffer_meter+idx*8,2,TRUE);
                                //GPLC.router_info[GPLC.phase-1].item = item; //buffer��buffer_meter����һ����save������-33����
                            }
                        }
                    }//forѭ���Ľ���
                    */
                    #endif
                }
                else if (node_type == 1) //���
                {
                    #ifdef __METER_EVENT_REPORT__
                    if(memory_fast_index_find_node_no(COMMPORT_PLC,meter_no,&meter_idx,&pos_idx,&router_protocol,NULL))
                    {
                        #ifdef __MEXICO_CIU__ /* �ϱ� �����е�����ϱ� Ҳ��CIU�ϱ� */
                        if(meter_idx & FAST_IDX_RTU_FLAG) /* �ɼ�����־����CIU */
                        {
                            #if 1
                            /* 1.�ҵ�CIU��Ӧ�ĵ�� */
                            tpos_mutexPend(&SIGNAL_FAST_IDX);
                            mem_cpy(guide_rail_addr,fast_index_list.fast_index[pos_idx].rtu_no,6);
                            tpos_mutexFree(&SIGNAL_FAST_IDX);
                            if(0 == compare_string(meter_no,guide_rail_addr,5))
                            {
                                /* ���ҵ�����ڿ��������е�λ�� */
                                if(memory_fast_index_find_node_no(COMMPORT_PLC,guide_rail_addr,&meter_idx,&pos_idx,&router_protocol,NULL))
                                {
                                    /*  */
                                    meter_idx &= FAST_IDX_MASK;
                                    if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT) )
                                    {
                                        if( (portContext->urgent_task_id != RECMETER_TASK_METER_EVENT_REPORT)
                                         && (portContext->urgent_task_id != RECMETER_TASK_MEXICO_CIU_REPORT) )//����Ѿ����¼��������񣬲�Ҫ���µ��ֻ���д洢����
                                        {
                                            (void)prepare_read_meter_param(meter_idx,&(portContext->router_phase_work_info[portContext->report_meter_phase].read_params));
                                            
                                        }
                                        item = 0x04001501;
                                        if( frame_ptr[POS_GB645_DLEN]>114)
                                        {
                                            frame_ptr[POS_GB645_DLEN] = 114;
                                        }
                                        if( frame_ptr[POS_GB645_CTRL] & 0x40 )
                                        {
                                            return 0;/* �쳣���� */
                                        }
                                        /*04001501 �����4�ֽڳ��������������� */
                                        if( frame_ptr[POS_GB645_DLEN] < 8) 
                                        {
                                            return 0;//û��״̬�֣�δ����04001501��ʽ�Ļ�������
                                        }
                                        mem_cpy(buffer_meter,frame_ptr+POS_GB645_ITEM,frame_ptr[POS_GB645_DLEN]);
                                        for(i=0;i<frame_ptr[POS_GB645_DLEN];i++)
                                        {
                                            buffer_meter[i]-= 0x33;  //����33��
                                        }
                                        if(compare_string(buffer_meter, (INT8U *)&item, 4) != 0)
                                        {
                                            return 0; //���� 04001501  ��ִ��
                                        }
                                        /* ������copy���� */
                                        mem_cpy(portContext->ciu_read_item,buffer_meter+4,4);
                                        if ((portContext->cur_plc_task == PLC_TASK_READ_METER)
                                        || ((portContext->read_status.is_in_read_cycle == 0) && (portContext->router_interactive_status.meter_doc_synchro_done == 1))
                                        || ((portContext->cur_plc_task == PLC_TASK_CHECK_DOC) && (portContext->cur_plc_task_step == PLC_CHECK_WIRELESS_NET_READY))
                                        || (portContext->cur_plc_task == PLC_TASK_PARALLEL_READ ))   //����ط���˼�ǳ�����ִ�У�ʱ����(ͬ��������)ִ�У��ȴ�������ִ�У�������ִ�С�����ʱ����ִ����Ҫ���¿���
                                        {
                                            if(tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))//���뵽�ź��� �ʹ��� ���򲻴���
                                            {
                                                tpos_enterCriticalSection();
                                                if( RECMETER_TASK_NONE == portContext->urgent_task_id )
                                                {
                                                    portContext->urgent_task_id = RECMETER_TASK_MEXICO_CIU_REPORT; //��������        
                                                }
                                                tpos_leaveCriticalSection();
                                                tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
                                            }
                                        }
                                    }
                                }
                            }
                            #endif
                        }
                        else
                        #endif
                        {
                            meter_idx &= FAST_IDX_MASK;
                            if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT) && (router_protocol != GB_OOP)) //__READ_OOP_METER__,��������Ҳ�ϱ������ڻ�δ����
                            {
                            
                                if(portContext->urgent_task_id != RECMETER_TASK_METER_EVENT_REPORT)//����Ѿ����¼��������񣬲�Ҫ���µ��ֻ���д洢����
                                {
                                    if(prepare_read_meter_param(meter_idx,&(portContext->router_phase_work_info[portContext->report_meter_phase].read_params)))
                                    portContext->router_phase_work_info[portContext->report_meter_phase].read_params.event_item_ctrl.read_04001501_time = 0; //������һ�κ�Ҫ
                                }
                                if( frame_ptr[POS_GB645_CTRL]&0x40)
                                {
                                    return 0;//�쳣���� 
                                }
                                /* ��ʱ04001501 ����� �������ȡ��󳤶� */
                                if( frame_ptr[POS_GB645_DLEN]>114)
                                {
                                    frame_ptr[POS_GB645_DLEN] = 114;
                                }
                                mem_cpy(buffer_meter,frame_ptr+POS_GB645_ITEM,frame_ptr[POS_GB645_DLEN]);
                                for(i=0;i<frame_ptr[POS_GB645_DLEN];i++)
                                {
                                    buffer_meter[i]-= 0x33;  //����33��
                                }
                                resp_frm_item = bin2_int32u(buffer_meter);
                                exp_flg = FALSE;
                                for(i=0;i< sizeof(expect_item)/sizeof(INT32U);i++)
                                {
                                    if(expect_item[i] == resp_frm_item)
                                    {
                                        exp_flg = TRUE;
                                        break;
                                    }
                                }
                                if( FALSE == exp_flg )
                                {
                                    return 0;
                                }
                                #ifdef __MEXICO_GUIDE_RAIL__
                                if( frame_ptr[POS_GB645_DLEN]< 6)
                                {
                                    return 0;/* û��״̬�֣�δ����04001501��ʽ�Ļ������� */
                                }
                                #else
                                switch(resp_frm_item)
                                {
                                    case 0x04001501:
                                        if( frame_ptr[POS_GB645_DLEN]< 16)
                                        {
                                            return 0;//û��״̬�֣�δ����04001501��ʽ�Ļ�������
                                        }
                                        break;
                                    case 0x04001507:
                                        if( frame_ptr[POS_GB645_DLEN] < 11)
                                        {
                                            return 0;/* ���������� */
                                        }
                                        break;
                                    default:
                                        return 0;
                                }
                                #endif
                                //  if((flag == 0x00) || (flag == 0xFF) )// û���¼���ʱ��ų���
                                {
                                    flag = 0xFE;
                                    //fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
                                }

                                item = resp_frm_item;
                                switch(item)
                                {
                                    case 0x04001501:
                                        fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
                                        if((flag == 0x00) || (flag == 0xFF) )// û���¼���ʱ��ų���
                                        {
                                            flag = 0xFE;
                                            fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&flag,1);
                                        }
                                        else
                                        {
                                            router_send_afn_00_F1_no_change_status(readportcontext); //ֱ�Ӵӵײ��һ��ȷ��֡������Ҫ�л�״̬
                                            return 0;
                                        }
                                        plc_router_save_report_meter_event_data(meter_idx,meter_no,item,buffer_meter+4,frame_ptr[POS_GB645_DLEN]-4);
                                        break;
                                    case 0x04001507:
                                        fread_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,&flag,1);
                                        if((flag == 0x00) || (flag == 0xFF) )// û���¼���ʱ��ų���
                                        {
                                            flag = 0xFE;
                                            fwrite_array(meter_idx,PIM_METER_EXT_REPORT_EVENT_STATE,&flag,1);
                                        }
                                        else
                                        {
                                            router_send_afn_00_F1_no_change_status(readportcontext); //ֱ�Ӵӵײ��һ��ȷ��֡������Ҫ�л�״̬
                                            return 0;
                                        }
                                        plc_router_save_report_meter_ext_event_data(meter_idx,meter_no,item,buffer_meter+4,frame_ptr[POS_GB645_DLEN]-4);
                                        break;
                                    default:
                                        return 0;
                                        //break;
                                }

//                                #ifdef __MEXICO_GUIDE_RAIL__
//                                if( frame_ptr[POS_GB645_DLEN]< 6)
//                                {
//                                    return 0;/* û��״̬�֣�δ����04001501��ʽ�Ļ������� */
//                                }
//                                #else
//                                if( frame_ptr[POS_GB645_DLEN]< 16)
//                                {
//                                    return 0;//û��״̬�֣�δ����04001501��ʽ�Ļ�������
//                                }
//                                #endif
//                                if(compare_string(buffer_meter, (INT8U *)&item, 4) != 0)
//                                {
//                                    return 0; //���� 04001501  ��ִ��
//                                }
//                                plc_router_save_report_meter_event_data(meter_idx,meter_no,item,buffer_meter+4,frame_ptr[POS_GB645_DLEN]-4);
                                //2�����ڣ���ͣ���յ��¼��ϱ����Ȳ�Ҫ��������
                                if((tick == 0) || (time_elapsed_10ms(tick) > 100*60*2))   //2����
                                {
                                    tick = os_get_systick_10ms();
                                    
                                    if ((portContext->cur_plc_task == PLC_TASK_READ_METER)
                                    || ((portContext->read_status.is_in_read_cycle == 0) && (portContext->router_interactive_status.meter_doc_synchro_done == 1))
                                    || ((portContext->cur_plc_task == PLC_TASK_CHECK_DOC) && (portContext->cur_plc_task_step == PLC_CHECK_WIRELESS_NET_READY))
                                    || (portContext->cur_plc_task == PLC_TASK_PARALLEL_READ ))   //����ط���˼�ǳ�����ִ�У�ʱ����(ͬ��������)ִ�У��ȴ�������ִ�У�������ִ�С�����ʱ����ִ����Ҫ���¿���
                                    {
                                        if(tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))//���뵽�ź��� �ʹ��� ���򲻴���
                                        {
                                            tpos_enterCriticalSection();
                                            if( RECMETER_TASK_NONE == portContext->urgent_task_id )
                                            {
                                                portContext->urgent_task_id = RECMETER_TASK_METER_EVENT_REPORT; //��������        
                                            }
                                            tpos_leaveCriticalSection();
                                            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    #endif
                }
            }
        }
        else
        {
            // ��һ���ֽ� 01 ����ģ��ͣ�磬���ǲ�����ERC56
            //  ͨ��Э������03 ����698   ��������0���ܲ��� ERC56
            //  ͨ��Э������02 ����645����   ��������0���ܲ���   ERC56
            //if((comm_protocol == 0x04)||((comm_protocol == 0xF1)&&(node_type != 0)))
            {
                if(0x01 == frame_event_ptr[0])
                {
                    #ifdef __METER_POWERONOFF_EVENT__
                    if(event_data_len>6)
                    {
                        data_len = (event_data_len -1)/6*6;
                        save_meter_power_off_event(frame_event_ptr+1,data_len,0x55);
                    }
                    #else
                    if(event_data_len>6)
                    {
                        // event_erc_49(frame_ptr+1,(data_len-1)/6,((data_len-1)/6)*6); //20170616 song
                        #ifndef __PROVICE_JIANGSU__
                        if(event_data_len<139)
                        {
                            event_erc_56(frame_event_ptr+1,(event_data_len-1)/6,((event_data_len-1)/6)*6); //20170802 song
                        }
                        else
                        {
                            event_erc_56(frame_event_ptr+1,22,132);
                            event_erc_56(frame_event_ptr+133,(event_data_len-133)/6,((event_data_len-133)/6)*6);
                        }
                        #endif
                    }
                    #endif
                }
                
                if(0x02 == frame_event_ptr[0])
                {
                    #ifdef __METER_POWERONOFF_EVENT__
                    if(event_data_len>6)
                    {
                        data_len = (event_data_len -1)/6*6;
                        save_meter_power_off_event(frame_event_ptr+1,data_len,0xAA);
                    }
                    #endif
                }

                if(0x05 == frame_event_ptr[0])/*�ܾ��ӽڵ������¼���2.7Э������*/
                {
                    if(event_data_len>6)
                    {
                        data_len = (event_data_len -1)/6*6;
                        event_erc_59(frame_event_ptr+1,data_len);
                    }

                }
            }
            //�Ϻ��ɼ���ͣ�ϵ�ģʽ
            router_send_afn_00_F1_no_change_status(readportcontext); //ֱ�Ӵӵײ��һ��ȷ��֡������Ҫ�л�״̬
            return 0;
        }
    }
    
    switch(portContext->cur_plc_task) //ͨ�����ı�״̬�ķ�ʽ�ظ�ȷ�ϣ�
    {
        case PLC_TASK_READ_METER:
            if (portContext->batch_meter_ctrl.check_close_event_report) //ִ��͸����������Ƚڵ�����ʱ������ʱ�ر��¼��ϱ�����
            {
                portContext->batch_meter_ctrl.check_close_event_report = 0;
                portContext->batch_meter_ctrl.ctrl_close_event_report = 1;
                portContext->params.task_read_data.ctrl_event_report = 0;
                readport_plc.OnPortReadData = router_send_afn_05_F2;
                portContext->OnPortReadData = router_check_urgent_timeout;
            }
            else
            {
                // readport_plc.OnPortReadData = router_send_afn_00_F1;
                // portContext->OnPortReadData = router_check_urgent_timeout;
                router_send_afn_00_F1_no_change_status(readportcontext);
            }
            break;
        /*
        case PLC_TASK_CHECK_DOC: //����Ĳ�����״̬�����ڼ�鵵�������У���û������
            switch(portContext->cur_plc_task_step)
            {
                case PLC_CHECK_WIRELESS_NET_READY:
                    readport_plc.OnPortReadData = router_send_afn_00_F1;
                    break;
            }
            break;
        case PLC_TASK_PARALLEL_READ:    //�������յ��¼��ϱ�
            readport_plc.OnPortReadData = router_send_afn_00_F1;
            break;
        */
        default:
            router_send_afn_00_F1_no_change_status(readportcontext);
            break;
    }
    
    return 0;
}

//·�ɲ�ѯ��F1���ӽڵ�����
INT8U router_send_afn_10_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F1,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

//·�ɲ�ѯ��F1���ӽڵ�����
INT8U router_recv_afn_10_F1(objReadPortContext * readportcontext)
{
    INT16U count_router,count_fast_index;
    PLCPortContext* portContext;
    //DL69842_RRR RRR;

    portContext = (PLCPortContext*)readportcontext;

    switch(portContext->cur_plc_task)
    {
    case PLC_TASK_CHECK_DOC:
        switch(portContext->cur_plc_task_step)
        {
        case PLC_CHECK_DOC_QUERY_NODE_COUNT_10F1_1:
            count_fast_index = memory_fast_index_stat_port_node_count(READPORT_PLC,portContext->router_base_info.router_info3.rtu_no_mode,portContext->router_base_info.router_info4.dzc_cvt_no_mode);
            count_router = bin2_int16u(portContext->frame_recv+portContext->recv_data_pos);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** ���������ڵ����� = %d , ·�ɽڵ����� = %d ***",count_fast_index,count_router);
            debug_println_ext(info);
            #endif

            if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_ZT)
            {
                                  //����ӿ�����������Ӵӽڵ���Ϣ
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = memory_fast_index_add_router_nodes;
                    portContext->cur_plc_task_step = PLC_CHECK_DOC_CHECK_FAST_INDEX_1;
                    portContext->params.task_check_doc.reinstall = 0;
              return 0;
            }
            if (count_fast_index == 0)
            {
                if (count_router > 0)
                {
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** ���������ڵ����� = 0 ��·�ɽڵ����� > 0 , ��ʼ��·�� ***");
                    debug_println_ext(info);
                    #endif
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_01_F2;
                    portContext->cur_plc_task_step = PLC_CHECK_DOC_INIT_ROUTER_01F2_1;
                }
                else
                {
                    start_read_meter(portContext);
                }
            }
            else
            {

                if (count_router > 0)
                {
                    int16u2_bin(count_router,portContext->params.task_check_doc.node_count);  //�ڵ�����
                    int16u2_bin(1,portContext->params.task_check_doc.strat_seq);  //��1��ʼ
                    portContext->params.task_check_doc.jump_count = 0;
                    portContext->params.task_check_doc.query_count = ROUTER_OPT_NODE_COUNT;    //ÿ�β�ѯROUTER_OPT_NODE_COUNT��

                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_afn_10_F2;
                    portContext->cur_plc_task_step = PLC_CHECK_DOC_QUERY_NODE_INFO;
                }
                else
                {
                    //����ӿ�����������Ӵӽڵ���Ϣ
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = memory_fast_index_add_router_nodes;
                    portContext->cur_plc_task_step = PLC_CHECK_DOC_CHECK_FAST_INDEX_1;
                    portContext->params.task_check_doc.reinstall = 0;
                    //portContext->params.task_check_doc.strat_seq[0] = 0;  //��0��ʼ
                    //portContext->params.task_check_doc.strat_seq[1] = 0;
                }
            }
            break;
        case PLC_CHECK_DOC_QUERY_NODE_COUNT_10F1_2:
            count_fast_index = memory_fast_index_stat_port_node_count(READPORT_PLC,portContext->router_base_info.router_info3.rtu_no_mode,portContext->router_base_info.router_info4.dzc_cvt_no_mode);
            count_router = bin2_int16u(portContext->frame_recv+portContext->recv_data_pos);
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** ���������ڵ����� = %d , ·�ɽڵ����� = %d ***",count_fast_index,count_router);
            debug_println_ext(info);
            #endif

            if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT)
            {
               if((count_fast_index - (portContext->params.task_check_doc.abnormal_node_count)) != count_router)
               {
                  readport_plc.OnPortReadData = router_check_urgent_timeout;
                  portContext->OnPortReadData = router_send_afn_01_F2;
                  portContext->cur_plc_task_step = PLC_CHECK_DOC_INIT_ROUTER_01F2_2;
               }
               else    //���볭������
               {
                  readport_plc.OnPortReadData = router_check_urgent_timeout;
                  portContext->OnPortReadData = router_send_afn_10_F2;
                  portContext->cur_plc_task_step = PLC_CHECK_DOC_ADD_NODE_10F2_END;
               }

            }
            else
            {
               if(count_fast_index != count_router)
              {
                  readport_plc.OnPortReadData = router_check_urgent_timeout;
                  portContext->OnPortReadData = router_send_afn_01_F2;
                  portContext->cur_plc_task_step = PLC_CHECK_DOC_INIT_ROUTER_01F2_2;
              }
              else    //���볭������
              {
                  readport_plc.OnPortReadData = router_check_urgent_timeout;
                  portContext->OnPortReadData = router_send_afn_10_F2;
                  portContext->cur_plc_task_step = PLC_CHECK_DOC_ADD_NODE_10F2_END;
                  
                  if( count_fast_index == 0)
                  {
                  portContext->router_interactive_status.plc_net_not_allow = 0; //�����ѱ�

                  }
                  //������޸���Ϊ�˹�̨�壬�Ƿ��б�Ҫ������
  //                start_read_meter(portContext);
              }
            }
            break;
        }
        break;
    }
    return 0;
}

//·�ɲ�ѯ��F2���ӽڵ���Ϣ
INT8U router_send_afn_10_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U tmp_check[3];

    portContext = (PLCPortContext*)readportcontext;
    tmp_check[0] = 1;
    tmp_check[1] = 0x00; //�����Լ���ʱ��Ҫ���ٲ�һ�δӽڵ���Ϣ���ô���ֻ��1�Σ���ʼ���1��ʼ��
    tmp_check[2] = 0x05;
    router_376_2_set_aux_info(0,40,0,TRUE);
    if(portContext->cur_plc_task_step == PLC_CHECK_DOC_ADD_NODE_10F2_END)
    {
        portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F2,tmp_check,3,portContext);
    }
    else
    {
        //
        tmp_check[0] = portContext->params.task_check_doc.strat_seq[0];
		tmp_check[1] = portContext->params.task_check_doc.strat_seq[1];
		tmp_check[2] = portContext->params.task_check_doc.query_count;
        portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F2,tmp_check,3,portContext);//portContext->params.task_check_doc.strat_seq
    }

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

//·�ɲ�ѯ��F2���ӽڵ���Ϣ
INT8U router_recv_afn_10_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    SLAVE_NODE_INFO node_info;
    INT16U node_count = 0;
    INT16U start_seq = 0;
    INT16U meter_idx = 0;
    INT16U save_pos = 0;
    INT8U pos = 0;
    INT8U count = 0;
    INT8U idx = 0;
    INT8U router_protocol;
    HPLC_PHASE phase_temp;
    INT8U phase;
    struct{
        INT8U seq[4];
        INT8U read_date[5];
        METER_READ_INFO meter_read_info;
        INT8U value[27];
    }var;
    //BOOLEAN del_node = FALSE;

    portContext = (PLCPortContext*)readportcontext;

    mem_cpy(portContext->params.task_check_doc.node_count,portContext->frame_recv+portContext->recv_data_pos,2);
    pos += 2;
    node_count = bin2_int16u(portContext->params.task_check_doc.node_count);    //����
    count = portContext->frame_recv[portContext->recv_data_pos+pos];       //���νڵ���
    pos++;

    portContext->params.task_check_doc.add_or_del_node.del_node.node_count = 0;
    for (idx=0;idx<count;idx++)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** query node : %02x%02x%02x%02x%02x%02x ***",
                portContext->frame_recv[portContext->recv_data_pos+pos+5],
                portContext->frame_recv[portContext->recv_data_pos+pos+4],
                portContext->frame_recv[portContext->recv_data_pos+pos+3],
                portContext->frame_recv[portContext->recv_data_pos+pos+2],
                portContext->frame_recv[portContext->recv_data_pos+pos+1],
                portContext->frame_recv[portContext->recv_data_pos+pos+0]);
        debug_println_ext(info);
        #endif

        mem_cpy(node_info.value,portContext->frame_recv+portContext->recv_data_pos+pos+6,2);

        //todo�����������м�����
        if(FALSE == memory_fast_index_find_node_no(READPORT_PLC,portContext->frame_recv+portContext->recv_data_pos+pos,&meter_idx,&save_pos,&router_protocol,NULL))
        {
            mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                    portContext->frame_recv+portContext->recv_data_pos+pos,6);
            portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
        }
        else
        {
            if ((portContext->router_base_info.router_info3.rtu_no_mode) && ((meter_idx & FAST_IDX_DZC_METER_FLAG) == 0))  //�ɼ����µĵ���ַҪɾ��
            {
                if (((meter_idx & FAST_IDX_RTU_FLAG) == 0) && (meter_idx & FAST_IDX_METER_FLAG))
                {
                    mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                            portContext->frame_recv+portContext->recv_data_pos+pos,6);
                    portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
                }
                else if (portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB13762)
                {
                    if (meter_idx & FAST_IDX_RTU_FLAG)
                    {
                        if (node_info.protocol != 1)
                        {
                            if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT)   //����·�ɲ�֧���ظ����
                            {
                                mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                                        portContext->frame_recv+portContext->recv_data_pos+pos,6);
                                portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
                            }
                            memory_fast_index_set_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                        }
                        else
                        {
                            memory_fast_index_clear_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                        }
                    }
                    else  //���
                    {
                        if (router_protocol != node_info.protocol)
                        {
                            if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT)   //����·�ɲ�֧���ظ����
                            {
                                mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                                        portContext->frame_recv+portContext->recv_data_pos+pos,6);
                                portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
                            }
                            memory_fast_index_set_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                        }
                        else
                        {
                            memory_fast_index_clear_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                        }
                    }
                }
                else if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)  //3762�Ĳ���Ҫ�ж�Э������
                {

                    memory_fast_index_clear_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);

                }
            }
            else if ((portContext->router_base_info.router_info4.dzc_cvt_no_mode) && ((meter_idx & FAST_IDX_METER_FLAG) == 0))
            {
                if (((meter_idx & FAST_IDX_RTU_FLAG) == 0) && (meter_idx & FAST_IDX_DZC_METER_FLAG))
                {
                    mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                            portContext->frame_recv+portContext->recv_data_pos+pos,6);
                    portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
                }
                else if (portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB13762)
                {
                    if (meter_idx & FAST_IDX_RTU_FLAG)
                    {
                        if (node_info.protocol != 0)
                        {
                            if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT)   //����·�ɲ�֧���ظ����
                            {
                                mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                                        portContext->frame_recv+portContext->recv_data_pos+pos,6);
                                portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
                            }
                            memory_fast_index_set_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                        }
                        else
                        {
                            memory_fast_index_clear_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                        }
                    }
                    else  //ˮ���ȱ�
                    {
                        if (router_protocol != node_info.protocol)
                        {
                            if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT)   //����·�ɲ�֧���ظ����
                            {
                                mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                                        portContext->frame_recv+portContext->recv_data_pos+pos,6);
                                portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
                            }
                            memory_fast_index_set_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                        }
                        else
                        {
                            memory_fast_index_clear_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                        }
                    }
                }
            }
            else
            {
                if (meter_idx & FAST_IDX_RTU_FLAG)
                {
                    #ifndef __MEXICO_CIU__ /* ī����CIU ��ɾ��  */
                    mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                            portContext->frame_recv+portContext->recv_data_pos+pos,6);
                    portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
                    #endif
                }
                else if (portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB13762)
                {
                    if (router_protocol != node_info.protocol)
                    {
                        if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT)   //����·�ɲ�֧���ظ����
                        {
                            mem_cpy(portContext->params.task_check_doc.add_or_del_node.del_node.node[portContext->params.task_check_doc.add_or_del_node.del_node.node_count],
                                    portContext->frame_recv+portContext->recv_data_pos+pos,6);
                            portContext->params.task_check_doc.add_or_del_node.del_node.node_count++;
                        }
                        memory_fast_index_set_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                    }
                    else
                    {
                        memory_fast_index_clear_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                    }
                }
                else if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)  //3762�Ĳ���Ҫ�ж�Э������
                {
                    memory_fast_index_clear_one_add_flag(portContext->frame_recv+portContext->recv_data_pos+pos);
                }

            }

            /*�����еı������ȸ�������λ*/
            phase = node_info.phase;
            if(phase != 0) /*�����0������Ҳû����*/
            {
                meter_idx &= FAST_IDX_MASK;
                fread_array(meter_idx,PIM_CUR_READ_INFO,var.seq,sizeof(var));
                /*10HF2ֻ�ܸ�����λ�����ܸı䵥���������*/
                var.meter_read_info.phase.rec_A  = 0;
                var.meter_read_info.phase.rec_B  = 0;
                var.meter_read_info.phase.rec_C  = 0;
                switch(phase) /*���µ�Э�飬F170������λ��λ�ñ����ʵ����λ*/
                {
                    case 1:    var.meter_read_info.phase.rec_A = 1;   break;
                    case 2:    var.meter_read_info.phase.rec_B = 1;   break;
                    case 4:    var.meter_read_info.phase.rec_C = 1;   break;  /*3762��Э�飬c����4*/
                }

                fwrite_array(meter_idx,PIM_CUR_READ_INFO,var.seq,sizeof(var));
            }


        }
        if(portContext->params.task_check_doc.add_or_del_node.del_node.node_count >= ROUTER_OPT_NODE_COUNT) break;

        pos += 8;
    }

    //�´β�ѯ��Ҫ�����ĸ���
    portContext->params.task_check_doc.jump_count = count - portContext->params.task_check_doc.add_or_del_node.del_node.node_count;

    switch(portContext->cur_plc_task)
    {
    case PLC_TASK_CHECK_DOC:
        switch(portContext->cur_plc_task_step)
        {
        case PLC_CHECK_DOC_QUERY_NODE_INFO:
            start_seq = bin2_int16u(portContext->params.task_check_doc.strat_seq);
            if (portContext->params.task_check_doc.add_or_del_node.del_node.node_count)  //��Ҫɾ���Ľڵ���Ϣ
            {
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_11_F2;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_DEL_NODE;
            }
            else if ((count == 0) || (start_seq >= node_count))   //��ѯ��ɣ�������ӽڵ�����
            {
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = memory_fast_index_add_router_nodes;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_CHECK_FAST_INDEX_1;
                portContext->params.task_check_doc.reinstall = 0;
                //portContext->params.task_check_doc.strat_seq[0] = 0;  //��0��ʼ
                //portContext->params.task_check_doc.strat_seq[1] = 0;
            }
            else if( (start_seq + ROUTER_OPT_NODE_COUNT) > node_count)
            {
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = memory_fast_index_add_router_nodes;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_CHECK_FAST_INDEX_1;
                portContext->params.task_check_doc.reinstall = 0;

            }
            else   //������Ҫ��ѯ�Ľڵ���Ϣ
            {
                node_count = bin2_int16u(portContext->params.task_check_doc.strat_seq);
                node_count += ROUTER_OPT_NODE_COUNT;
                portContext->params.task_check_doc.strat_seq[0] = node_count;
                portContext->params.task_check_doc.strat_seq[1] = node_count>>8;
                portContext->params.task_check_doc.query_count = ROUTER_OPT_NODE_COUNT;    //ÿ�β�ѯROUTER_OPT_NODE_COUNT��

                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_10_F2;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_QUERY_NODE_INFO;
            }
            break;
        case PLC_CHECK_DOC_ADD_NODE_10F2_END:
            if (((portContext->router_base_info.router_info1.comm_mode == ROUTER_COMM_MODE_WIFI) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC13))
                || ((portContext->router_base_info.router_info1.comm_mode == ROUTER_COMM_MODE_HPLC) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)))
                     /*��Ѷ�����ߺͶ��ſ����Ҫ��������Ϣ*/
                //��Ҫ�����������ſ������ǰ��Ҫ������״̬������������Ҫ�顣�������󣬲�����Ҫ��һ�¡��������ҵĶ�����Ҫ�ˡ�
            {
               // #ifdef __PROVICE_JIANGSU__
              //  start_read_meter(portContext);
                //#else
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_10_F4;
                portContext->cur_plc_task_step = PLC_CHECK_WIRELESS_NET_READY;
               // #endif
            }
            #if (defined __PROVICE_CHONGQING__)
            else if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)
            {
                file_delete(FILEID_ROUTER_INFO_TABLE);
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_10_F3;
                portContext->net_topology_start_idx[0] = 0;
                portContext->net_topology_start_idx[1] = 0;
                portContext->cur_plc_task_step = PLC_CHECK_NET_TOPOLOGY;
            }
            #endif
            else
            {
              start_read_meter(portContext);
            }
             break;
        }
        break;
    }
    return 0;
}

#if (defined __PROVICE_CHONGQING__)
//·�ɲ�ѯ��F3���ӽڵ���һ��
INT8U router_send_afn_10_F3(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT16U node_count;
    INT16U seq_idx = 0;
    INT16U meter_idx = 0;
    METER_DOCUMENT meter_doc;

    portContext = (PLCPortContext*)readportcontext;
    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    seq_idx = bin2_int16u(portContext->net_topology_start_idx);
    
    if(seq_idx >= node_count)
    {
        seq_idx = 0;
        portContext->net_topology_start_idx[0] = 0;
        portContext->net_topology_start_idx[1] = 0;
        start_read_meter(portContext);
        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return 0;
    }

    while(seq_idx < node_count)
    {

        //��ȡ�ز��ӽڵ��ַ,���
        if (fast_index_list.fast_index[seq_idx].port == READPORT_PLC)
        {
            meter_idx = bin2_int16u(fast_index_list.fast_index[seq_idx].seq_spec);
            meter_idx &= FAST_IDX_MASK;
            if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
            {
                fread_meter_params(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
                if(FALSE == check_is_sqr_protocol(meter_doc.protocol))
                {
                    break;
                }
            }
        }
        
        seq_idx ++ ;
        continue ;
        

    }

    if(seq_idx >= node_count)
    {
        seq_idx = 0;
        portContext->net_topology_start_idx[0] = 0;
        portContext->net_topology_start_idx[1] = 0;
        start_read_meter(portContext);
        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return 0;
    }
    
    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F3,fast_index_list.fast_index[seq_idx].node,6,portContext);
    seq_idx++;
    portContext->net_topology_start_idx[0] = (INT8U)(seq_idx & 0xFFFF);
    portContext->net_topology_start_idx[1] = (INT8U)(seq_idx >> 8);
    
    
    tpos_mutexFree(&SIGNAL_FAST_IDX);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}
//·�ɲ�ѯ��F3����һ����Ϣ
INT8U router_recv_afn_10_F3(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT32U offset = 0;
    INT16U idx = 0;
    INT16U meter_count;
    INT16U meter_idx,pos_idx;
    INT8U  meter_relay_info[12] = {0};
    INT8U router_protocol,user_class;
    BOOLEAN is_exist;
    INT8U meter_no[6] = {0};
    
    
    is_exist = FALSE;

    portContext = (PLCPortContext*)readportcontext;
    fread_array(FILEID_ROUTER_INFO_TABLE,PIM_AREA_METER_COUNT,(INT8U*)&meter_count,2);
    if (meter_count > MAX_METER_COUNT) meter_count = 0;

    idx = bin2_int16u(portContext->net_topology_start_idx);
    tpos_enterCriticalSection();
    mem_cpy(meter_no,fast_index_list.fast_index[idx].node,6);
    tpos_leaveCriticalSection();

    
    if (memory_fast_index_find_node_no(COMMPORT_PLC,meter_no,&meter_idx,&pos_idx,&router_protocol,&user_class))
    {
        meter_idx &= FAST_IDX_MASK;
        if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
        {
            is_exist = TRUE;
        }
    }
    else//�����Ǽ������������в�ѯ�����ֲ����ڵ����ʱ��·�ɿ��ܳ������⵼�·��ر���쳣
    {
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext->OnPortReadData = router_send_afn_10_F3;
        portContext->cur_plc_task_step = PLC_CHECK_NET_TOPOLOGY;
        return 0;
    }

    if(is_exist)
    {
        offset = (meter_idx-1)*PIM_AREA_ROUTER_TABLE_PER_METER_INFO_LEN+PIM_AREA_ROUTER_TABLE_DATA_START;
        mem_cpy(meter_relay_info,meter_no,6);
        mem_cpy(meter_relay_info+6,portContext->frame_recv+portContext->recv_data_pos+1,6);
        fwrite_array(FILEID_ROUTER_INFO_TABLE,offset,meter_relay_info,12);
    }
   

   // ����ʹ�� portContext->plc_topology_seq_idx  ��ȷ����ȡ���ǿ���������Ӧ�Ĳ������

    //idx++;
    //portContext->net_topology_start_idx[0] = idx;
    //portContext->net_topology_start_idx[1] = (idx>>8);
    readport_plc.OnPortReadData = router_check_urgent_timeout;
    portContext->OnPortReadData = router_send_afn_10_F3;
    portContext->cur_plc_task_step = PLC_CHECK_NET_TOPOLOGY;

    return 0;

}
#endif

//·�ɲ�ѯ��F4��·������״̬
INT8U router_send_afn_10_F4(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F4,NULL,0,portContext);

    if((portContext->urgent_task_step == PLC_CAST_WAIT_EXEC_END) || (portContext->urgent_task_step == PLC_CAST_OOP_WAIT_EXEC_END))
    {
        readport_plc.OnPortReadData = wait_cast_exec_end; //�㲥���񣬶�ռ·��
    }
    else if(portContext->cur_plc_task_step == PLC_CAST_WAIT_EXEC_END)
    {
        readport_plc.OnPortReadData = wait_cast_exec_end; //�㲥���񣬶�ռ·��
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}

//·�ɲ�ѯ��F4��·������״̬
INT8U router_recv_afn_10_F4(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    BOOLEAN router_states;

    portContext = (PLCPortContext*)readportcontext;

    if ((portContext->frame_recv[portContext->recv_data_pos+13] == 8)
        && (portContext->frame_recv[portContext->recv_data_pos+14] == 8)
        && (portContext->frame_recv[portContext->recv_data_pos+15] == 8)
        && ((portContext->router_base_info.router_info1.comm_mode == ROUTER_COMM_MODE_NARROW)
        || (portContext->router_base_info.router_info1.comm_mode == ROUTER_COMM_MODE_WIFI ))) /*խ��������������*/
    {
        router_states = TRUE;    //·�ɿ���
    }
    else if(portContext->router_base_info.router_info1.comm_mode == ROUTER_COMM_MODE_HPLC)
    {
       if((portContext->frame_recv[portContext->recv_data_pos] & 0x10) == 0x10)
       {
          router_states = TRUE;    //·�ɿ���,���Ͽ��Ҫ���ѯ����ģʽ�����ֹͣ�������ǿ���
       }
       else
       {
         router_states = FALSE;   //æ
       }
    }
    else
    {
        router_states = FALSE;   //æ
    }

    if((portContext->urgent_task_step == PLC_CAST_WAIT_EXEC_END) || (portContext->urgent_task_step == PLC_CAST_OOP_WAIT_EXEC_END)) //��������Ĺ㲥���������վ͸���㲥Уʱ
    {
        if (router_states)
        {

           if((portContext->urgent_task_id == RECMETER_TASK_TRANS_CAST_TIME)&& (portContext->urgent_task_step == PLC_CAST_WAIT_EXEC_END))
           {
               //ת�㲥OOPУʱ
               portContext->cast_content = portContext->frame_cast_buffer; // portContext->frame_send
               // #ifdef __READ_OOP_METER__
               portContext->cast_content_len =  make_oop_adj_time_frame(portContext->cast_content,0); //oop�Ĺ㲥Уʱ
               // #endif
               readport_plc.OnPortReadData = router_send_afn_03_F9;
               portContext->urgent_task = PLC_TASK_URGENT_TASK;
               portContext->urgent_task_step = PLC_CAST_OOP_DELAY_03_F9;

           }
           else
           {
                    //�㲥����ִ�����
                          urgent_task_in_wait_next_urgent_task(portContext);
           }

        }

    }
    else if(portContext->cur_plc_task_step == PLC_CAST_WAIT_EXEC_END) //�ѱ������ �㲥���
    {
        if (router_states)
        {
            if (portContext->cur_plc_task == PLC_TASK_PLC_NET)
            {
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** ·�ɿ��У��㲥���������***");
                debug_println_ext(info);
                #endif

                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                portContext->cur_plc_task_step = PLC_NET_WAIT_CJQ_SEARCH_METER;
                portContext->plc_net_time_out_10ms = os_get_systick_10ms();
            }
        }
    }
    else if ((portContext->cur_plc_task_step  == PLC_CHECK_WIRELESS_NET_READY)) //|| ((portContext->router_base_info.router_info1.comm_mode == 2) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)))
    {
         if ((portContext->frame_recv[portContext->recv_data_pos] & 0x01) == 1)
         {
             portContext->wireless_network_time = 0;
             portContext->router_interactive_status.plc_net_not_allow = 0; //�����ѱ��ӱȶԵ������ƣ�ֻ�Զ��Ŵ���
             //������������ʼ����
             start_read_meter(portContext);
         }
         else
        {
            #ifdef __SOFT_SIMULATOR__
            if(((portContext->wireless_network_time >= 1) && ((portContext->router_base_info.router_info1.comm_mode == 2) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)))
                || (portContext->wireless_network_time > 150)) //���ſ��������ʱ��20������������150 //����150��������·��
            #else    
            if(((portContext->wireless_network_time > 20) && ((portContext->router_base_info.router_info1.comm_mode == 2) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)))
                || (portContext->wireless_network_time > 150)) /*���ſ��������ʱ��20������������150 ,����150����ǿ�Ƴ���*/
            #endif
            {
                portContext->wireless_network_time = 0;
                portContext->router_interactive_status.plc_net_not_allow = 0; //�����ѱ�
                start_read_meter(portContext);

            }
            else
            {  
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_wait_1_minute_send_afn_10_F4;
                portContext->tick = os_get_systick_10ms();
            
                portContext->wireless_network_time ++;   //һ���Ӽ�һ����
            }
        }
    }
    else
    {
        switch(portContext->cur_plc_task)
        {
        case PLC_TASK_PLC_NET:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_NET_WAIT_NODE_LOGON:
                if (router_states)
                {
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** ·�ɿ��У��ѱ�����������ѱ�ͳ�ƣ�***");
                    debug_println_ext(info);
                    #endif
                    //�����ѱ�ͳ��
                    start_plc_net_stat(portContext);
                }
                else
                {
                    readport_plc.OnPortReadData = router_check_urgent_timeout;/*·��û�п��е�ʱ�򣬾���router_check_urgent_timeout�����в�ѯ��10HF4����Ҳ�ڸú�����*/
                }
                break;
            default:
               readport_plc.OnPortReadData = router_check_urgent_timeout;
               break;
            }
            break;
        case PLC_TASK_PARALLEL_READ: /*���ڿ������Ĭ�ϲ�������״̬�ˣ����ŵĲ�ѯ�ڽ��볭��ǰ��ɣ�*/
            switch(portContext->cur_plc_task_step)
            {
            case PLC_PARALL_READ_WAIT_READY :
                 portContext->wireless_network_time = 0;
                 portContext->router_interactive_status.plc_net_not_allow = 0; //�����ѱ�

                 /*��ʼ��鳭�����������ǰ���Ѿ���ѯ��*/
                 readport_plc.OnPortReadData = router_send_afn_12_F2; //
                 portContext->OnPortReadData = router_check_urgent_timeout;
                 portContext->cur_plc_task_step = PLC_PARALL_READ_12HF2;
                 break;
            default:  /*��������Ĭ��״̬�½����鲢������*/
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                break;
            }
            break;
        }
    }

    return 0;
}
//���������ר�õģ����ѱ�Ĳ�ѯ������

INT8U router_wait_1_minute_send_afn_10_F4(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    readport_plc.OnPortReadData = router_check_urgent_timeout; //Ҫȥ��һ�½�������

    if (time_elapsed_10ms(portContext->tick) > 60*100)   //60s
    {
        portContext->OnPortReadData = router_send_afn_10_F4;
    }

    return 0;
}

#ifdef __READ_MODULE_ID_PLAN__
/*
��ѯ�ӽڵ�ID��Ϣ
��ʽ:
�ӽڵ���ʼ��� 2�ֽ�
�ӽڵ�����   1�ֽ�
*/
INT8U router_send_afn_10_F7(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F7,portContext->task_read_module_id.node_start_seq,3,portContext);
    
    //readport_plc.OnPortReadData = router_wait_resp_frame;  
    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }
    return portContext->frame_send_Len;
}

/*
��ѯ�ӽڵ�ID��Ϣ
�ӽڵ������� 2
����Ӧ��Ĵӽڵ����� 1

�ӽڵ��ʽ:
�ӽڵ��ַ          6
�ӽڵ�����          1
�ӽڵ�ģ�鳧�̴���  2
�ӽڵ�ģ��ID����    1
�ӽڵ�ģ��ID��ʽ    1
�ӽڵ�ģ��ID ��     m

*/
INT8U router_recv_afn_10_F7(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT32U offset = 0;
	SLAVE_NODE_ID_INFO node_id_info;
	SLAVE_NODE_ID_INFO last_node_id_info;
    INT16U node_count = 0;
    INT16U start_seq = 0;
    INT16U meter_idx = 0;
    INT16U save_pos = 0;
    INT16U pos = 0;
	INT16U process_pos = 0;
    INT8U count = 0;
    INT8U idx = 0;
    INT8U router_protocol;
    #ifdef __PROVICE_HUBEI__
    INT8U tmp=0;
    #endif
    BOOLEAN del_node = FALSE;
	
    portContext = (PLCPortContext*)readportcontext;

    pos = portContext->recv_data_pos;//
    mem_cpy(portContext->task_read_module_id.slave_node_count,portContext->frame_recv+pos,2);
    pos += 2;
    node_count = bin2_int16u(portContext->task_read_module_id.slave_node_count);    //�ӽڵ�������
    count = portContext->frame_recv[pos];       //����Ӧ��Ĵӽڵ�����
    pos++;

	
    for (idx=0;idx<count;idx++)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** query node_id : %02x%02x%02x%02x%02x%02x ***",
                portContext->frame_recv[portContext->recv_data_pos+pos+5],
                portContext->frame_recv[portContext->recv_data_pos+pos+4],
                portContext->frame_recv[portContext->recv_data_pos+pos+3],
                portContext->frame_recv[portContext->recv_data_pos+pos+2],
                portContext->frame_recv[portContext->recv_data_pos+pos+1],
                portContext->frame_recv[portContext->recv_data_pos+pos+0]);
        debug_println_ext(info);
        #endif

        //clear to all 0xFF
        mem_set(node_id_info.value,sizeof(SLAVE_NODE_ID_INFO),0xFF);

		mem_cpy(node_id_info.value,portContext->frame_recv+pos,11);
		pos += 11;//����ID��λ��
        #ifdef __PROVICE_HUBEI__
        node_id_info.node_id_format = 1;//�����̶���ǰ����ģ���õ�02���ᵼ����վ�����������������ü���������20190725,���̴���������壬������Ҫ�󵹻�λ��
        tmp = node_id_info.vendor_code[0];
        node_id_info.vendor_code[0] = node_id_info.vendor_code[1];
        node_id_info.vendor_code[1] = tmp;
        #endif
        if(node_id_info.node_id_len <= 50)
			mem_cpy(node_id_info.node_id,portContext->frame_recv+pos,node_id_info.node_id_len);
        //��ת����һ��ID��Ϣ
		pos += node_id_info.node_id_len;
        //todo�����������м����ڣ������ڵĻ����洢��
        if(TRUE == memory_fast_index_find_node_no(READPORT_PLC,node_id_info.node_addr,&meter_idx,&save_pos,&router_protocol,NULL))
        {
            //�ɼ�����ô���� ?  TODO ???
            meter_idx &= FAST_IDX_MASK;
			if( (meter_idx >0) && (meter_idx <=MAX_METER_COUNT) )
			{

				{
				    //��һ��д�룬���ۻظ��ı����Ƕ�ȡ���ˣ�����û��ȡ������ֱ��д�룬��������¼�
				    offset = PIM_MODULE_ID_START+(meter_idx-1)*PIM_MODULE_ID_PER_SIZE;
				    fread_array(FILEID_MODULE_ID,offset,last_node_id_info.value,11);
    				if( (check_is_all_FF(last_node_id_info.value,11) == TRUE) )
    				{
    				    //��д��ɣ��������ģ��ID���ȱ仯�ˣ������ǳ�0xFF
    				    fwrite_array(FILEID_MODULE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
    				}
                    else if((node_id_info.node_id_len == 0x01) && (node_id_info.node_id[0] == 0x00)) /*��֧�ֵģ���Ҫ�����¼�*/
                    {
                        fwrite_array(FILEID_MODULE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
                    }
					else
					{
					    //�ڶ��λ���N�ζ�ȡ�����·��û��ȡ�����򲻴洢��
					    if( (node_id_info.node_id_len == 0x01) && (node_id_info.node_id[0] == 0xFF) )
					    {
					        //
					        continue;
					    }
						else
						{
						    //��ȡ���ˣ��űȽϺʹ洢
						    //��һ��д��ͱ�֤�˳��ȣ��Ƿ�Ҫ�ж�ID������???	
						    fread_array(FILEID_MODULE_ID,offset+11,last_node_id_info.node_id,last_node_id_info.node_id_len);
        					if(compare_string(last_node_id_info.node_addr,node_id_info.node_addr,6) != 0)
        					{
        					    //��ַ��һ���ˣ�ֱ��д�룬����ɾ��������ʱ���Ƿ�Ͳ��ô�����???TODO ��Ҫ����
        					    fwrite_array(FILEID_MODULE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
        					}
							else if((last_node_id_info.node_id_len == 0x01) && (last_node_id_info.node_id[0] == 0xFF))
							{
							    //��һ��û��������ζ����ˣ�Ҳ���ܲ����¼�,ֱ��д��
							    fwrite_array(FILEID_MODULE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
							}
        					else 
        					{
        					    // 11�����Ȳ�һ����˵��ID�ſ��ܱ䶯�� �����¼�;���߰��ձ��ζ�ȡ����Ϣ�ж�
        					    // ���κ��ϴβ�һ������Ҫ����ERC�����洢���ε�ID��Ϣ
        					    #ifdef __SOFT_SIMULATOR__
                                //event_erc_43(last_node_id_info.value,11+last_node_id_info.node_id_len,node_id_info.value,11+node_id_info.node_id_len,0x04);
        						#endif
        					    if(compare_string(last_node_id_info.vendor_code,node_id_info.vendor_code,2) != 0)
        					    {
        					    	fwrite_array(FILEID_MODULE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
        							//��ʽ 6 addr+5+m ERC43 ��ʽ  5+m+6 addr,erc43��Ҫת����
                                    #ifdef __PROVICE_HUNAN__/*���ϲ�Ҫģ�����¼���ֻҪоƬ���*/
                                    #else
        							event_erc_43(last_node_id_info.value,11+last_node_id_info.node_id_len,node_id_info.value,11+node_id_info.node_id_len,0x04);
                                    #endif
        					    }
                                else if(compare_string(last_node_id_info.node_id,node_id_info.node_id,node_id_info.node_id_len) != 0)
                                {
                                    fwrite_array(FILEID_MODULE_ID,offset,node_id_info.value,sizeof(SLAVE_NODE_ID_INFO));
                                    if(last_node_id_info.node_id_len >1) /*���ζ��������ȶ��ٲ����¼�*/
                                    {
        							//��ʽ 6 addr+5+m ERC43 ��ʽ  5+m+6 addr,erc43��Ҫת����
                                    #ifdef __PROVICE_HUNAN__
                                     #else
        							event_erc_43(last_node_id_info.value,11+last_node_id_info.node_id_len,node_id_info.value,11+node_id_info.node_id_len,0x04);
                                    #endif
                                    }
                                }
        					}
						}
					}
				    
				}
                
			}
        }     
    }

	//���ս�������ִ��
	if (portContext->urgent_task)
    {
        //
        switch(portContext->urgent_task_step)
        {
            case PLC_URGENT_TASK_READ_MODULE_ID:
				start_seq = bin2_int16u(portContext->task_read_module_id.node_start_seq);
                if ( (count == 0) || (start_seq >= node_count) || ((start_seq + ROUTER_OPT_NODE_COUNT) > node_count) )   //��ѯ��ɣ�����ȴ���һ����������״̬
                {
                    

                   /*������оƬID*/
                    portContext->task_read_module_id.node_start_seq[0] = 0x01;
                    portContext->task_read_module_id.node_start_seq[1] = 0x00;
                    portContext->task_read_module_id.query_node_cnt = ROUTER_OPT_NODE_COUNT;
                    readport_plc.OnPortReadData = router_send_afn_10_F112;
                    portContext->urgent_task = PLC_TASK_URGENT_TASK;
                    portContext->urgent_task_step = PLC_URGENT_TASK_READ_CHIP_ID;
                }
                else   //������Ҫ��ѯ�Ľڵ���Ϣ
                {
                    node_count = bin2_int16u(portContext->task_read_module_id.node_start_seq);
                    node_count += ROUTER_OPT_NODE_COUNT;
                    portContext->task_read_module_id.node_start_seq[0] = node_count;
                    portContext->task_read_module_id.node_start_seq[1] = node_count>>8;
                    portContext->task_read_module_id.query_node_cnt = ROUTER_OPT_NODE_COUNT;    //ÿ�β�ѯROUTER_OPT_NODE_COUNT��
        
        			// �������Ͳ�ѯ����		  
                    readport_plc.OnPortReadData = router_send_afn_10_F7;
                }
				break;
        }
        
        
    }   

	return 0;
}
#endif


//·�����ã�F1����Ӵӽڵ�
INT8U router_send_afn_11_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    #if defined __CJQ_ORDER_MODE__ || defined __zh_wireless__ || defined __SOFT_SIMULATOR__
    INT16U meter_idx,save_pos;
    INT8U idx,i,find_same,router_protocol;
    #endif
    #ifdef __SOFT_SIMULATOR__
    //INT8U idx;
    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
    {
      for(idx=0;idx<portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count;idx++)
      {
          snprintf(info,100,"*** add node : %02x%02x%02x%02x%02x%02x ***",
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[idx].node[5],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[idx].node[4],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[idx].node[3],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[idx].node[2],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[idx].node[1],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[idx].node[0]);
          debug_println_ext(info);
      }
    }
    else
    {
      for(idx=0;idx<portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count;idx++)
      {
          snprintf(info,100,"*** add node : %02x%02x%02x%02x%02x%02x ***",
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node[5],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node[4],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node[3],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node[2],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node[1],
                  portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node[0]);
          debug_println_ext(info);
      }
    }
    #endif

    #if defined __CJQ_ORDER_MODE__ || defined __zh_wireless__ 
    //�ɼ�����ַģʽ ���ɼ��Ǻ������豸ר�ã�
    if((portContext->router_base_info.router_info4.dzc_cvt_no_mode) || (portContext->router_base_info.router_info3.rtu_no_mode))
    {
        if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB13762)
        {
            for(idx=0;idx<portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count;idx++)
            {
                find_same = 0;
                find_same = memory_fast_index_find_node_no_nowait(READPORT_PLC,portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node,&meter_idx,&save_pos,&router_protocol,NULL);

                if((find_same == 1) && (meter_idx & FAST_IDX_RTU_FLAG))
                {
                    //for(idx=0;idx<portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count;idx++)
                    {
                        find_same = 0;
                        for(i=0;i<MAX_READ_INFO_CNT;i++)
                        {
                            if(0 == compare_string(portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node,cjq_read_info[i].cjq_no,6))
                            {
                                find_same = 1;
                                break;//�ҵ���ͬ�ļ�¼����ѭ��
                            }
                        }

                        if(!find_same)//û�з�����ͬ�ĵ�ַ����ӵ�����
                        {
                            for(i=0;i<MAX_READ_INFO_CNT;i++)
                            {
                                if((check_is_all_ch(cjq_read_info[i].cjq_no,6,0xFF)) || (check_is_all_ch(cjq_read_info[i].cjq_no,6,0x00)))
                                {
                                    mem_cpy(cjq_read_info[i].cjq_no,portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node,6);
                                    cjq_read_info[i].meter_seq = 0x00;
                                    cjq_read_info[i].comm_ok = 0x00;
                                    break;
                                }
                            }
                        }

                    }
                }
            }
        }
    }

    #endif

    router_376_2_set_aux_info(0,40,0,TRUE);
    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
    {
       portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RSET,DT_F1,
                                    portContext->params.task_check_doc.add_or_del_node.value,
                                    portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count*9+1,portContext);
    }
    else
    {
//     portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node->protocol = 0; //�������͸��Э�飬·���·���ʱ����λ��
    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RSET,DT_F1,
                                    portContext->params.task_check_doc.add_or_del_node.value,
                                    portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count*7+1,portContext);
    }
    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}

//·�����ã�F2��ɾ���ӽڵ�
INT8U router_send_afn_11_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;
    #if defined __CJQ_ORDER_MODE__  ||  defined  __SOFT_SIMULATOR__
    INT16U meter_idx,save_pos;
    INT8U idx,i,result,router_protocol;
    #endif
    #ifdef __SOFT_SIMULATOR__
    //INT8U idx;
    for(idx=0;idx<portContext->params.task_check_doc.add_or_del_node.del_node.node_count;idx++)
    {
        snprintf(info,100,"*** del node : %02x%02x%02x%02x%02x%02x ***",
                portContext->params.task_check_doc.add_or_del_node.del_node.node[idx][5],
                portContext->params.task_check_doc.add_or_del_node.del_node.node[idx][4],
                portContext->params.task_check_doc.add_or_del_node.del_node.node[idx][3],
                portContext->params.task_check_doc.add_or_del_node.del_node.node[idx][2],
                portContext->params.task_check_doc.add_or_del_node.del_node.node[idx][1],
                portContext->params.task_check_doc.add_or_del_node.del_node.node[idx][0]);
        debug_println_ext(info);
    }
    #endif

    #ifdef __CJQ_ORDER_MODE__
    //�ɼ�����ַģʽ ���ɼ�����ַ�±��˳�򳭶�
    if((portContext->router_base_info.router_info3.rtu_no_mode) || (portContext->router_base_info.router_info4.dzc_cvt_no_mode))
    {    
        for(idx=0;idx<portContext->params.task_check_doc.add_or_del_node.del_node.node_count;idx++)
        {
            for(i=0;i<MAX_READ_INFO_CNT;i++)
            {
                if(0 == compare_string((portContext->params.task_check_doc.add_or_del_node.del_node.value+idx*6+1),cjq_read_info[i].cjq_no,6))
                {
                    mem_set(cjq_read_info[i].value,sizeof(CJQ_READ_INFO),0xFF);
                    break;//�ҵ���ͬ�ļ�¼����ѭ��
                }
            }
        }
    }
    #endif
    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RSET,DT_F2,
                                        portContext->params.task_check_doc.add_or_del_node.value,
                                        portContext->params.task_check_doc.add_or_del_node.del_node.node_count*6+1,portContext);
    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

//·�����ã�F5������ӽڵ�ע��
INT8U router_send_afn_11_F5(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U tmp_send_data[10];
    
    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);
    mem_cpy(tmp_send_data,portContext->params.task_plc_net.start_time,6); //û�ж�������ȴ�����
    tmp_send_data[6] = portContext->params.task_plc_net.last_minute[0];
	tmp_send_data[7] = portContext->params.task_plc_net.last_minute[1];
	tmp_send_data[8] = portContext->params.task_plc_net.node_repeat;
    tmp_send_data[9] = 0x30;
    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RSET,DT_F5,tmp_send_data,10,portContext);
    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}
//·�����ã�F6����ֹ·�ɽڵ��ϱ�
INT8U router_send_afn_11_F6(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RSET,DT_F6,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}
//·�����ã�F8���ڵ��������Ԥ��
INT8U router_send_afn_11_F8(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
    if ((priority_node.count > 0) && (priority_node.count <= 20))
    {
        portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RSET,DT_F8,&(priority_node.count),1+6*priority_node.count,portContext);
        readport_plc.OnPortReadData = router_wait_resp_frame;
        priority_node.flag = 0xAA;

        return portContext->frame_send_Len;
    }
    else
    #endif
    {
        readport_plc.OnPortReadData = portContext->OnPortReadData;
        return 0;
    }
}

//·�ɿ��ƣ�F1������·��
INT8U router_send_afn_12_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RCTRL,DT_F1,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;
    #ifdef __CJQ_ORDER_MODE__
    //ͬ���ɼ�����Ϣ �����ڵ�һ���ز���ʱ�ļ�����
    fwrite_array(FILEID_PLC_REC_TMP,PIM_CJQ_READ_INFO,(INT8U*)cjq_read_info,MAX_READ_INFO_CNT*(sizeof(CJQ_READ_INFO)));
    #endif
    return portContext->frame_send_Len;
}

//·�ɿ��ƣ�F2����ͣ·��
INT8U router_send_afn_12_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RCTRL,DT_F2,NULL,0,portContext);

    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }
    
    #ifdef __CJQ_ORDER_MODE__
    //ͬ�����Ǻ�ɼ�����Ϣ �����ڵ�һ���ز���ʱ�ļ�����
    fread_array(FILEID_PLC_REC_TMP,PIM_CJQ_READ_INFO,(INT8U*)cjq_read_info,MAX_READ_INFO_CNT*(sizeof(CJQ_READ_INFO)));
    #endif

    return portContext->frame_send_Len;
}

//·�ɿ��ƣ�F3���ָ�·��
INT8U router_send_afn_12_F3(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RCTRL,DT_F3,NULL,0,portContext);

    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}


INT8U router_send_afn_13_F1(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U gb_645_frame[260],delay_flag,tmp_doc[6];
    //INT16U meter_idx;
    //READ_PARAMS read_params;
    //INT32U item;

    portContext = (PLCPortContext*)readportcontext;

    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
    delay_flag = 0;
    else
    delay_flag = 1;

    mem_set(tmp_doc,sizeof(tmp_doc),0x00);
    if( portContext->urgent_task_step ==PLC_METER_EVENT_REPORT_CTRL_06F5_3)
    {
         //item = 0;

      //�ж�һ���ǲ��ǲɼ���
   //   if(cjq_flag)
    //to do:�ɼ���ģʽ������
      int test1 = portContext->report_meter_phase;//portContext->router_work_info.phase;
    //  if(true == prepare_plc_read_report_cjq_event_state(&(portContext->router_phase_work_info[0].read_params),gb_645_frame+2,item))
        if (TRUE == prepare_plc_read_report_meter_event_state(&(portContext->router_phase_work_info[test1].read_params),gb_645_frame+4,gb_645_frame+3))
        {
            mem_cpy(tmp_doc,portContext->router_phase_work_info[test1].read_params.meter_doc.meter_no,6);

            mem_cpy(portContext->router_work_info.ADDR_DST,tmp_doc,6);

            router_376_2_set_aux_info(0,40,1,true);

            gb_645_frame[0] = 0x02;      //ͨ��Э������
            gb_645_frame[1] = 0x00;      //ͨ����ʱ����Ա�־
            gb_645_frame[2] = 0x00;      //�ӽڵ㸽���ڵ�����n
         //   gb_645_frame[3] = portContext->urgent_task_req_frame_len;      //���ĳ���L
          //  mem_cpy(gb_645_frame+4,portContext->urgent_task_req_frame,portContext->urgent_task_req_frame_len);

            portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RTRANS,DT_F1,gb_645_frame,gb_645_frame[3]+4,portContext);
        }
        else
        {
        	//����ʱ�������������
            mem_set((INT8U*)&(portContext->router_phase_work_info[test1].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
            //����ȴ���һ�����������״̬
            urgent_task_in_wait_next_urgent_task(portContext);
            return 0;
        }
    }
    #if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
    else if( portContext->urgent_task_step == PLC_URGENT_SWITCH_TRIP_CTRL)
    {
        if(gSystemInfo.terminal_manage == 0xAA)
        {
            //����ʱ�������������
            mem_set((INT8U*)&(portContext->router_phase_work_info[0].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
            //����ȴ���һ�����������״̬
            urgent_task_in_wait_next_urgent_task(portContext);
            portContext->switch_trip_flag = 0x55;
            fwrite_array(FILEID_RUN_DATA,PIM_SWITCH_TRIP_CTRL_FLAG,&(portContext->switch_trip_flag),1); 
            return 0;
        }
        else
        {
        
            if (TRUE == prepare_plc_switch_trip_ctrl(readportcontext,gb_645_frame+4,gb_645_frame+3))
            {
                mem_cpy(tmp_doc,portContext->router_phase_work_info[0].read_params.meter_doc.meter_no,6);
                
                mem_cpy(portContext->router_work_info.ADDR_DST,tmp_doc,6);
                
                router_376_2_set_aux_info(0,40,1,TRUE);
                
                gb_645_frame[0] = 0x01;      //ͨ��Э������ 97Э��
                gb_645_frame[1] = 0x00;      //ͨ����ʱ����Ա�־
                gb_645_frame[2] = 0x00;      //�ӽڵ㸽���ڵ�����n
                portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RTRANS,DT_F1,gb_645_frame,gb_645_frame[3]+4,portContext);
            }
            else
            {
                //����ʱ�������������
                mem_set((INT8U*)&(portContext->router_phase_work_info[0].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
                //����ȴ���һ�����������״̬
                urgent_task_in_wait_next_urgent_task(portContext);
                portContext->switch_trip_flag = 0x55;
                fwrite_array(FILEID_RUN_DATA,PIM_SWITCH_TRIP_CTRL_FLAG,&(portContext->switch_trip_flag),1); 
                return 0;
            }
        }
    }
    #endif
    #if (defined __MEXICO_CIU__)
    else if( portContext->urgent_task_step == PLC_URGENT_MEXICO_CIU_REPORT)
    {
        if(0xAA != portContext->snd_ciu_flag)
        {
            gb_645_frame[3] = make_gb645_2007_read_frame(gb_645_frame+4,portContext->router_phase_work_info[portContext->report_meter_phase].read_params.meter_doc.meter_no,bin2_int32u(portContext->ciu_read_item),NULL,0);
    
            mem_cpy(tmp_doc,portContext->router_phase_work_info[portContext->report_meter_phase].read_params.meter_doc.meter_no,6);
                
            mem_cpy(portContext->router_work_info.ADDR_DST,tmp_doc,6);
            
            router_376_2_set_aux_info(0,40,1,TRUE);
            
            gb_645_frame[0] = 0x02;      //ͨ��Э������ 07Э��
            gb_645_frame[1] = 0x00;      //ͨ����ʱ����Ա�־
            gb_645_frame[2] = 0x00;      //�ӽڵ㸽���ڵ�����n
            portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RTRANS,DT_F1,gb_645_frame,gb_645_frame[3]+4,portContext);
        }
        else
        {
            /* ���㷢�ͱ��ĳ��� copy����ʱ���� */
            gb_645_frame[3] = portContext->snd_ciu_frame[POS_GB645_DLEN]+12;
            mem_cpy(gb_645_frame+4,portContext->snd_ciu_frame,gb_645_frame[3]);

            /* Ŀ�ĵ�ַ CIU ��ַ */
            mem_set(tmp_doc,sizeof(tmp_doc),0x00);
            mem_cpy(tmp_doc,portContext->router_phase_work_info[portContext->report_meter_phase].read_params.meter_doc.meter_no,5);
            mem_cpy(portContext->router_work_info.ADDR_DST,tmp_doc,6);

            /* ��֡13762 ���� */
            router_376_2_set_aux_info(0,40,1,TRUE);
            
            gb_645_frame[0] = 0x02;      //ͨ��Э������ 07Э��
            gb_645_frame[1] = 0x00;      //ͨ����ʱ����Ա�־
            gb_645_frame[2] = 0x00;      //�ӽڵ㸽���ڵ�����n
            portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RTRANS,DT_F1,gb_645_frame,gb_645_frame[3]+4,portContext);
        }
    }
    #endif

    else
    {
        if (portContext->router_base_info.router_info3.rtu_no_mode)//���ﻹ�е����⣬zyҪ����
        {
            if(portContext->urgent_task)
            {
                if (isvalid_meter_addr(portContext->urgent_task_meter_doc->rtu_no,FALSE))
                {
                   mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_meter_doc->rtu_no,6);
                }
                else
                {
                    mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_meter_doc->meter_no,6);
                }
            }
            else
            {
                mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_req_frame+1,6);
            }
        }
        else if(portContext->router_base_info.router_info4.dzc_cvt_no_mode)
        {
            if(portContext->urgent_task)
            {
                if (isvalid_meter_addr(portContext->urgent_task_meter_doc->rtu_no,FALSE))
                {
                    mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_meter_doc->rtu_no,6);
                }
                else
                {
                    mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_meter_doc->meter_no,6);
                }
            }
            else
            {
                mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_req_frame+1,6);
            }
        }
        else
        {
            #ifdef __READ_OOP_METER__
            if(GB_OOP == portContext->urgent_task_meter_doc->protocol)
            {
                if((portContext->urgent_task_req_frame[4]&0x0F) >= 5)
                {
                    mem_cpy(portContext->router_work_info.ADDR_DST, portContext->urgent_task_req_frame+5, 6);//��࿽��6��cur_f57.p_zero_time[0] = value&0xFF;
                }
                else//���Ȳ���6�����ȿ���ʵ�ʵ�ַ�������Ĳ�0
                {
                    mem_cpy(portContext->router_work_info.ADDR_DST, portContext->urgent_task_req_frame+5, (portContext->urgent_task_req_frame[4]&0x0F)+1);
                    mem_set(portContext->router_work_info.ADDR_DST+(portContext->urgent_task_req_frame[4]&0xF)+1, 5-(portContext->urgent_task_req_frame[4]&0x0F), 0x00);
                }
            }
            else
            #endif
            #ifdef __READ_DLMS_METER__
            if(METER_DLMS == portContext->urgent_task_meter_doc->protocol)
            {
            //��Ҫ�ñ����еĵ�ַ���Ҳ���
                mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_meter_doc->meter_no,6);
            }
            else
            #endif
            {
                mem_cpy(portContext->router_work_info.ADDR_DST,portContext->urgent_task_req_frame+1,6);
            }
        }

        #ifdef __SH_2009_METER__
        if (portContext->router_work_info.ADDR_DST[5] == 0xFF) portContext->router_work_info.ADDR_DST[5] = 0x00;
        #endif
        router_376_2_set_aux_info(0,40,1,TRUE);

        gb_645_frame[0] = meter_protocol_2_router_protocol(portContext->urgent_task_meter_doc->protocol);      //ͨ��Э������

        if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
        {}
        else
        gb_645_frame[1] = 0x00;      //ͨ����ʱ����Ա�־

        gb_645_frame[delay_flag+1] = 0x00;      //�ӽڵ㸽���ڵ�����n
        gb_645_frame[delay_flag+2] = portContext->urgent_task_req_frame_len;      //���ĳ���L
        mem_cpy(gb_645_frame+delay_flag+3,portContext->urgent_task_req_frame,portContext->urgent_task_req_frame_len);

        #ifdef __MESSAGE_SEND_RECEIVE_RECORD__
        message_send_and_receive_num_record(bin2_int16u(portContext->router_phase_work_info[0].read_params.meter_doc.spot_idx),0x66);//��¼���ͱ��Ĵ���
        #endif

        portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RTRANS,DT_F1,gb_645_frame,gb_645_frame[delay_flag+2]+delay_flag+3,portContext);

    }

    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;

}

INT8U router_recv_afn_13_F1(objReadPortContext * readportcontext,DL69842_RRR *RRR)
{
    PLCPortContext* portContext;

    INT32U item = 0;
    INT16U meter_idx;
    INT8U *meter_no,*frame_ptr;
    #if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
    INT16U count = 0;
    #endif
    INT8U idx,FE_count,data_len,pos;

    portContext = (PLCPortContext*)readportcontext;

    FE_count = 0;
    pos = 0;

    //��ǰ���ı���ͨ������ʱ��
    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB13762) pos+= 2;
    //ͨ��Э������
    pos++;
    //���ĳ���L
    data_len = portContext->frame_recv[portContext->recv_data_pos+pos];
    pos++;
    //��������
    for(idx=0;idx<data_len;idx++)
    {
        if (portContext->frame_recv[portContext->recv_data_pos+pos+idx] == 0x68)
        {
            break;
        }
        FE_count++;
    }

    data_len -= FE_count;
    frame_ptr = portContext->frame_recv+portContext->recv_data_pos+pos+FE_count;


    if (portContext->urgent_task)
    {
        #ifdef __READ_OOP_METER__
        if(portContext->router_phase_work_info[portContext->report_meter_phase].read_params.meter_doc.protocol == GB_OOP)
        {
            meter_no = frame_ptr + 5;        /*���������*/
        }
        else
        #endif
        {
            meter_no = frame_ptr+1;
        }
        
        if (portContext->urgent_task_step == PLC_METER_EVENT_REPORT_CTRL_06F5_3) //����������
        {

            if (data_len > 0)
            {

                for(idx=0;idx<frame_ptr[POS_GB645_DLEN];idx++) frame_ptr[POS_GB645_ITEM+idx] -= 0x33;

                //�ȴ������쳣  D1
                if(frame_ptr[POS_GB645_CTRL] &0x40) //�쳣Ӧ��
                {
                    mem_set(frame_ptr+POS_GB645_07_DATA,20,0xEE);
                    frame_ptr[POS_GB645_DLEN] = 20+4;
                }
                data_len = frame_ptr[POS_GB645_DLEN]-4;//ֻ�����ݳ�����Ϣ
                if (compare_string(portContext->router_phase_work_info[portContext->report_meter_phase].read_params.meter_doc.meter_no,meter_no,6) == 0)
                {
                    meter_idx = bin2_int16u(portContext->router_phase_work_info[portContext->report_meter_phase].read_params.meter_doc.meter_idx);

                    mem_cpy(&item,&(portContext->router_phase_work_info[portContext->report_meter_phase].read_params.item),4);//�˴���һ���ܵõ�item
                    #ifdef __SOFT_SIMULATOR__
                    mem_cpy(info,frame_ptr+POS_GB645_07_DATA,data_len);
                    debug_println_ext(info);
                    #endif
                      //ֱ�Ӵ�������
                   // plc_router_save_cjq_meter_event_data(meter_idx,meter_no,item,frame_ptr,data_len,true);
                    if(0xAA == portContext->router_phase_work_info[portContext->report_meter_phase].read_params.iot_report)
                    {
                        plc_router_save_report_meter_ext_event_data(meter_idx,meter_no,item,frame_ptr+POS_GB645_07_DATA,data_len);
                    }
                    else
                    {
                        plc_router_save_report_meter_event_data(meter_idx,meter_no,item,frame_ptr+POS_GB645_07_DATA,data_len);
                    }
                
                    readport_plc.OnPortReadData = router_send_afn_13_F1;
                    portContext->urgent_task_step = PLC_METER_EVENT_REPORT_CTRL_06F5_3;
                }
                else
                {
                    //����״̬������ȴ���һ�����������״̬
                    urgent_task_in_wait_next_urgent_task(portContext);
                }
            }
            else
            {
                //����״̬������ȴ���һ�����������״̬
                urgent_task_in_wait_next_urgent_task(portContext);
            }
        }
        #if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
        else if(portContext->urgent_task_step == PLC_URGENT_SWITCH_TRIP_CTRL)
        {
            if( (data_len > 0) && (0x84 == frame_ptr[POS_GB645_CTRL]) ) // && () )
            {
                portContext->switch_trip_try_cnt = 0;
                portContext->switch_idx++;
                tpos_mutexPend(&SIGNAL_FAST_IDX);
                count = fast_index_list.count;
                tpos_mutexFree(&SIGNAL_FAST_IDX);
                if(portContext->switch_idx >= count)
                {
                    //����״̬������ȴ���һ�����������״̬
                    portContext->switch_trip_flag = 0x55;
                    fwrite_array(FILEID_RUN_DATA,PIM_SWITCH_TRIP_CTRL_FLAG,&(portContext->switch_trip_flag),1);
                    urgent_task_in_wait_next_urgent_task(portContext);
                }               
                else
                {
                    readport_plc.OnPortReadData = router_send_afn_13_F1;
                    portContext->urgent_task_step = PLC_URGENT_SWITCH_TRIP_CTRL;
                }
            }
            else
            {
                portContext->switch_trip_try_cnt++;
                if(portContext->switch_trip_try_cnt >= 3)
                {
                    portContext->switch_trip_try_cnt = 0;
                    portContext->switch_idx++;
                }
                tpos_mutexPend(&SIGNAL_FAST_IDX);
                count = fast_index_list.count;
                tpos_mutexFree(&SIGNAL_FAST_IDX);
                if(portContext->switch_idx >= count)
                {
                    //����״̬������ȴ���һ�����������״̬
                    portContext->switch_trip_flag = 0x55;
                    fwrite_array(FILEID_RUN_DATA,PIM_SWITCH_TRIP_CTRL_FLAG,&(portContext->switch_trip_flag),1);
                    urgent_task_in_wait_next_urgent_task(portContext);
                }
                else
                {
                    readport_plc.OnPortReadData = router_send_afn_13_F1;
                    portContext->urgent_task_step = PLC_URGENT_SWITCH_TRIP_CTRL;
                }
            }
        }
        #endif
        #if (defined __MEXICO_CIU__)
        else if(portContext->urgent_task_step == PLC_URGENT_MEXICO_CIU_REPORT)
        {
            /* ����0 �Ҳ����쳣֡ */
            if( (data_len > 0) && (0 == (frame_ptr[POS_GB645_CTRL] &0x40)) )
            {
                if(0xAA != portContext->snd_ciu_flag)
                {
                    if(compare_string(portContext->router_phase_work_info[portContext->report_meter_phase].read_params.meter_doc.meter_no,meter_no,6) == 0)
                    {
                        /* ��֡���͸�CIU ctrl�����⣬TODO::???? */
                        make_write_ciu_frame(portContext->snd_ciu_frame,meter_no,frame_ptr+POS_GB645_ITEM,frame_ptr[POS_GB645_DLEN]);
                        /* ������ط��� ��CIU �������� */
                        portContext->snd_ciu_flag = 0xAA;
                        readport_plc.OnPortReadData = router_send_afn_13_F1;
                        portContext->urgent_task_step = PLC_URGENT_MEXICO_CIU_REPORT;
                        return 0;
                    }
                    else
                    {
                        urgent_task_in_wait_next_urgent_task(portContext);
                    }
                }
                else
                {
                    /*  */
                    if(compare_string(portContext->router_phase_work_info[portContext->report_meter_phase].read_params.meter_doc.meter_no,meter_no,6) == 0)
                    {
                        /*  */
                    }
                    else
                    {
                        /*  */
                        
                    }
                    portContext->snd_ciu_flag = 0x00;
                    urgent_task_in_wait_next_urgent_task(portContext);
                    
                }           
            }
            else /* ��ʱ ����ִ�� */
            {
                portContext->snd_ciu_flag = 0x00;
                urgent_task_in_wait_next_urgent_task(portContext);
            }
        }
        #endif

        else
        {
         #ifdef __MESSAGE_SEND_RECEIVE_RECORD__
        message_send_and_receive_num_record(bin2_int16u(portContext->router_phase_work_info[0].read_params.meter_doc.spot_idx),0x77);//��¼���ܱ��Ĵ���
        #endif
            //��������
           #ifdef __READ_DLMS_METER__ //dlmsЭ�飬��Ҫȥfe
            if(METER_DLMS == portContext->urgent_task_meter_doc->protocol)
            {
                data_len += FE_count;
                frame_ptr = portContext->frame_recv+portContext->recv_data_pos+pos;
            }
            #endif
            tpos_enterCriticalSection();
            if((portContext->urgent_task_resp_frame!=NULL) && (portContext->urgent_task_resp_frame_len>=data_len))
            {
                portContext->urgent_task_resp_frame_len = data_len;
                mem_cpy(portContext->urgent_task_resp_frame,frame_ptr,portContext->urgent_task_resp_frame_len);
            }
            else
            {
                portContext->urgent_task_resp_frame_len = 0;
            }
            tpos_leaveCriticalSection();
             //����״̬������ȴ���һ�����������״̬
             urgent_task_in_wait_next_urgent_task(portContext);
        }

    }
    else if (((portContext->cur_plc_task == PLC_TASK_READ_METER) && (portContext->cur_plc_task_step == PLC_READ_METER_WAIT_RESP_13F1))
              || (portContext->cur_plc_task == PLC_TASK_READ_VIP_METER))
    {
        //���645ǰ���ǲ�����FE
        portContext->router_work_info.phase = 0;
        
         #ifdef __READ_OOP_METER__
        if(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol == GB_OOP)
        {
            meter_no = frame_ptr + 5;        /*���������*/
        }
        else
        #endif
        {
            meter_no = frame_ptr+1;
        }   
        #ifdef __READ_DLMS_METER__ //dlmsЭ�飬��Ҫȥfe
            if(METER_DLMS == portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol)
            {
                data_len += FE_count;
                frame_ptr = portContext->frame_recv+portContext->recv_data_pos+pos;
            }
        #endif
        if(data_len == 0)
        {
          if((portContext->cur_plc_task == PLC_TASK_READ_VIP_METER))
          {
            portContext->params.task_read_data.has_fail_meter = TRUE;
            portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
            portContext->OnPortReadData = get_read_vip_meter_info;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            return 0;
          }
          else
          {
            portContext->params.task_read_data.has_fail_meter = TRUE;
            portContext->cur_plc_task = PLC_TASK_READ_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
            portContext->OnPortReadData = get_read_meter_info;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            return 0;
          }
        }
        else
        {
            #ifdef __READ_DLMS_METER__ //dlmsЭ�飬ֱ�Ӵ洢�������ж��ˣ�
            if(METER_DLMS == portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol)
            {

                save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
            }
            else
            #endif

            {
            if (compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,6) == 0)
            {
                //�жϱ���Ƿ�һ��
                save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
            }
            else if ((portContext->router_base_info.router_info3.rtu_no_mode) && (portContext->router_base_info.router_info3.rtu_frame_format))
            {
                //�ɼ���ģʽ
                if (compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6) == 0)
                {
                    //�޸Ĳɼ���ģʽ����
                    portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info.try_count = 0;
                    data_len = trans_read_frame_cjq_mode_to_standard_format(frame_ptr,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no);
                    save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
                }
            }
            else if(portContext->router_base_info.router_info4.dzc_cvt_no_mode)  //�ı�����ģʽ����洢;
            {
                //�ɼ���ģʽ
                if (compare_string(meter_no,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6) == 0)
                {
                    save_read_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),frame_ptr,&data_len);
                }
            }
            }
            if( RRR != NULL)/*������λ������*/
            {
                #ifdef __F170_REC_TIME_IS_DAY_HOLD_READ_TIME__
                if(portContext->router_phase_work_info[0].read_params.read_type == READ_TYPE_CYCLE_DAY) /*ɽ��ֻ���ն������ݲŸ���F170�������*/
                {
                    update_meter_recstate(bin2_int16u(portContext->router_phase_work_info[0].read_params.meter_doc.meter_idx),
                    COMMPORT_PLC,0,(RRR->phase_q.spec == 4) ? (0x80|RRR->phase_q.phase) : RRR->phase_q.phase,RRR->resp_relay_channel.relay,RRR->phase_q.value[1],TRUE);
                }
                #else
                update_meter_recstate(bin2_int16u(portContext->router_phase_work_info[0].read_params.meter_doc.meter_idx),
                COMMPORT_PLC,0,(RRR->phase_q.spec == 4) ? (0x80|RRR->phase_q.phase) : RRR->phase_q.phase,RRR->resp_relay_channel.relay,RRR->phase_q.value[1],TRUE);
                #endif
            }
    
            if(portContext->cur_plc_task == PLC_TASK_READ_VIP_METER)
            {
                portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
                portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
                portContext->OnPortReadData = prepare_read_vip_item;
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                return 0;
            }
            else
            {
                portContext->cur_plc_task = PLC_TASK_READ_METER;
                portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
                #ifdef __PROVICE_JIANGSU__
                if(check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol))
                {
                    portContext->OnPortReadData = get_read_meter_info;
                }
                else
                #endif

                if(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_read_priority_ctrl) /*������ͨ�����з��ϻظ��ĵ�����ߣ���Ҫѭ��*/
                {
                    portContext->OnPortReadData = get_read_meter_info;
                }
                else
                {
                    portContext->OnPortReadData = prepare_read_item_concentrator;
                }
                readport_plc.OnPortReadData = router_check_urgent_timeout;
            }
        }
    }

    return 0;
}

INT8U router_send_afn_14_F1(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U *meter_no,single_result;
    INT16U meter_idx,save_pos;//,test_tmp;
    //INT16U meter_seq;
    INT8U gb_645_frame[260];
    INT8U router_protocol,dalay_flag;
    BOOLEAN result;
    //INT8U meter_event_plan[MAX_METER_EVENT_PLAN_COUNT],plan_id;//����¼����ڳ���
    void save_event_grade_mask_flag(READ_PARAMS *read_params);
	#ifdef __METER_DAY_FREEZE_EVENT__
	void save_freeze_event_mask_flag(READ_PARAMS *read_params);
	#endif
    INT8U need_check_other_phase;
    INT8U idx = 0;
    INT16U tmp_meter_idx = 0;
    INT16U pos = 0;

    portContext = (PLCPortContext*)readportcontext;
    need_check_other_phase = 0;

    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
    dalay_flag = 0;
    else
    dalay_flag = 1;

    meter_no = portContext->frame_recv + portContext->recv_data_pos + 1;

    mem_cpy(portContext->router_work_info.ADDR_DST,meter_no,6);

    single_result = memory_fast_index_find_node_no_nowait(READPORT_PLC,meter_no,&meter_idx,&save_pos,&router_protocol,NULL);

    if(single_result ==1)
    {
        if((portContext->router_base_info.router_info3.rtu_no_mode || portContext->router_base_info.router_info4.dzc_cvt_no_mode) && (meter_idx & FAST_IDX_RTU_FLAG))/*�����ж�645�Ͳɼ��������ɼ�����ַ�޷��ж�Э������*/
        {
            if ((compare_string(portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info.rtu_no,meter_no,6) != 0)
            || (portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_day_changed))
            {
                tmp_meter_idx = memory_fast_index_find_first_meter_idx_in_rtu(COMMPORT_PLC,portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info.rtu_no,&pos);
                if(tmp_meter_idx <= MAX_METER_COUNT)  //��Ϊ�ж�MAX_METER_COUNT
                {
                    fwrite_meter_params(tmp_meter_idx,PIM_CJQ_METER_NO,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,6);
                }
                for(idx = 0;idx < 4;idx++)
                {
                    if(idx == portContext->router_work_info.phase) continue;
                    if(compare_string(portContext->router_phase_work_info[idx].cjq_info.rtu_no,meter_no,6) == 0)
                    {
                        tmp_meter_idx = memory_fast_index_find_first_meter_idx_in_rtu(COMMPORT_PLC,meter_no,&pos);
                        if(tmp_meter_idx <= MAX_METER_COUNT)
                        {
                            fwrite_meter_params(tmp_meter_idx,PIM_CJQ_METER_NO,portContext->router_phase_work_info[idx].read_params.meter_doc.meter_no,6);
                        }
                    }
                }
                portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_day_changed = 0;
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,200,"****��ʼ���ɼ������� rtu_no=%02x%02x%02x%02x%02x%02x",meter_no[5],meter_no[4],meter_no[3],meter_no[2],meter_no[1],meter_no[0]);
                debug_println_ext(info);
                #endif
                memory_fast_index_init_cjq_info(&(portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info),meter_no,TRUE);

                need_check_other_phase = 0xAA; //�ɼ���ģʽ������
            }
    //�˴�Ҫ����09����13��ʽ��
            result = prepare_cjq_read_item(&(portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info),
                    &(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),
                    gb_645_frame+dalay_flag+2,gb_645_frame+dalay_flag+1);

            if(result)
            {
              
                if(!check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol))
                {
                    //����ɼ���ģʽ����
                    if(portContext->router_base_info.router_info3.rtu_frame_format)
                    {
                        //�޸ı���Ϊ�ɼ���ģʽ�� ����ַ�ӵ���������ײ���
                        gb_645_frame[dalay_flag+1] = trans_read_frame_to_cjq_mode(gb_645_frame+dalay_flag+2,meter_no);

                        #ifdef __SOFT_SIMULATOR__
                        snprintf(info,200,"*** rtu_no=%02x%02x%02x%02x%02x%02x  slave_idx=%d  phase=%d  meter_idx=%d",
                            meter_no[5],meter_no[4],meter_no[3],meter_no[2],meter_no[1],meter_no[0],
                            portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info.idx,
                            portContext->router_work_info.phase,
                            bin2_int16u(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_idx));
                        debug_println_ext(info);
                        #endif
                    }
                }
            }
        }
        else
        {
            meter_idx &= FAST_IDX_MASK;
            if ((compare_string(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,meter_no,6) != 0)
            || (portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_day_changed))
            {
                portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_day_changed = 0;
                portContext->router_phase_work_info[portContext->router_work_info.phase].try_count = 0; //���󳭶���������
                need_check_other_phase = 0x55;
                #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
                clear_priority_node(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc));
                #endif

                #ifdef __SOFT_SIMULATOR__
                snprintf(info,200,"*** �����ˣ�valid_flag = 0x%02X",
                    portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.event_item_ctrl.valid_flag);
                #endif
                save_event_grade_mask_flag(&portContext->router_phase_work_info[portContext->router_work_info.phase].read_params);

                #ifdef __METER_DAY_FREEZE_EVENT__
                save_freeze_event_mask_flag(&portContext->router_phase_work_info[portContext->router_work_info.phase].read_params);
                #endif

               if(((portContext->router_base_info.router_info3.rtu_no_mode) && (!check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol)))
                  || ((portContext->router_base_info.router_info4.dzc_cvt_no_mode) && (check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol))))/*�ɼ���/ת����ģʽ��������ʱ�����ͨ���Ĳɼ�����Ϣ��Ҫ����*/
                {
                    if(isvalid_meter_addr(portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info.rtu_no,FALSE))
                    {
                        tmp_meter_idx = memory_fast_index_find_first_meter_idx_in_rtu(COMMPORT_PLC,portContext->router_phase_work_info[portContext->router_work_info.phase].cjq_info.rtu_no,&pos);
                        if(tmp_meter_idx <= MAX_METER_COUNT)  //��Ϊ�ж�MAX_METER_COUNT
                        {
                            fwrite_meter_params(tmp_meter_idx,PIM_CJQ_METER_NO,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,6);
                        }
                    }

                }
                /*װ��ǰҪ������ͨ�����е���Ϣ*/
                mem_set((INT8U*)(&portContext->router_phase_work_info[portContext->router_work_info.phase].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);

                if(!prepare_read_meter_param(meter_idx,&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params)))
                {

                    goto GAME_OVER;
                }
            }
            else
            {
                if(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.span_curve_flag )
                {
                    portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.span_curve_flag = 0;
                    if(!prepare_read_meter_param(meter_idx,&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params)))
                    {                        
                        goto GAME_OVER;
                    }
                }
            }

            portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_phase = portContext->router_work_info.phase;

            //#ifdef __ROUTER_JXZB__
            if ((portContext->router_phase_work_info[portContext->router_work_info.phase].try_count >= 3)
                && (portContext->router_base_info.router_vendor != ROUTER_VENDOR_TOPSCOMM ))
            {
                //������ʱ������
                result = FALSE;
                portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.control.loop = 1;
                portContext->router_phase_work_info[portContext->router_work_info.phase].try_count = 0;
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,200,"*** �������Դ���������  phase=%d  try_count=%d ",
                        portContext->router_work_info.phase,
                        portContext->router_phase_work_info[portContext->router_work_info.phase].try_count);
                debug_println_ext(info);
                #endif
            }
            else
            //#endif
            {
             result = prepare_read_item(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),gb_645_frame+dalay_flag+2,gb_645_frame+dalay_flag+1);
            }
        }

        if ((result) && ((gb_645_frame+dalay_flag+1) >0))  //���ͳ��ȴ���0
        {
            if (portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.batch_ctrl.is_no_resp)
            {
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                return 0;
            }

            if ((portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_set_routor_node_fail)
            || (portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_read_priority_ctrl)
            || (portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_read_priority)) //__ITEM_PRIORITY__
            {
                gb_645_frame[0] = 0; //����ʧ��    ����Ҫ���������������������һ��ʧ��
                #ifdef __SOFT_SIMULATOR__
                if (portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.is_read_priority)
                {
                    snprintf(info,200,"*** �������ȼ����ƣ�����һ��ʧ�� ");
                    debug_println_ext(info);
                }
                #endif
            }
            else
            {
                gb_645_frame[0] = 2;   //������־
                portContext->router_phase_work_info[portContext->router_work_info.phase].try_count++;
            }
        }
        else
        {
            gb_645_frame[0] = portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.control.loop ? 0 : 1;
            if(gb_645_frame[0] == 1) //���Ҫ�óɹ����ȿ���������û��Ҫ��ʧ��
            {
              gb_645_frame[0] = portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.patch_load_tmp_loop ? 0 : 1;
            }
        }

        if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
        {}
        else
        gb_645_frame[dalay_flag] = portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.control.rec_delay_flag;   //ͨ����ʱ����Ա�־  //ͨ����ʱ����Ա�־

        if(gb_645_frame[0] != 2)
        {
            gb_645_frame[dalay_flag+1] = 0;                                             //·���������ݳ���
            #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
            clear_priority_node(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc));
            #endif                                           
            //����ʱ�������������
            mem_set((INT8U*)&(portContext->router_phase_work_info[portContext->router_work_info.phase].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);

        }
        gb_645_frame[dalay_flag+2+gb_645_frame[dalay_flag+1]] = 0;                                 //�ӽڵ㸽���ڵ�����n
    }
    else if(single_result ==2)
    {
      return FALSE;
    }
    else
    {
GAME_OVER:
        gb_645_frame[0] = 1;
        gb_645_frame[dalay_flag] = 0;
        gb_645_frame[dalay_flag+1] = 0;
        gb_645_frame[dalay_flag+2] = 0;
        //����ʱ�������������
        mem_set(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.value,sizeof(READ_PARAMS),0x00);
    }

    if( need_check_other_phase)
    {
    clear_other_phase_same_meter_no_before_send(portContext,need_check_other_phase,meter_no);
    }
    #ifdef __MESSAGE_SEND_RECEIVE_RECORD__
    if(gb_645_frame[0] == 2)//������Ҫ���Ͳż�¼
    message_send_and_receive_num_record(meter_idx,0x55);//��¼���ͱ��Ĵ���
    #endif

    router_376_2_set_aux_info(portContext->router_work_info.channel_id,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.resp_byte_num,1,FALSE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_REQUEST,DT_F1,gb_645_frame,dalay_flag+3+gb_645_frame[dalay_flag+1],portContext);

    readport_plc.OnPortReadData = router_wait_send_frame_complete;
    portContext->OnPortReadData = router_check_urgent_timeout;

    return portContext->frame_send_Len;
}

INT8U router_recv_afn_14_F1(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    portContext = (PLCPortContext*)readportcontext;


    switch(portContext->cur_plc_task)
    {
    case PLC_TASK_READ_METER:
        switch(portContext->cur_plc_task_step)
        {
        case PLC_READ_METER_ROUTER_MODE:
        case PLC_READ_METER_PRIOR_11F8: /*����ڷ���11HF8�У��յ�·������14HF1��ҲҪ��Ӧ��*/
            readport_plc.OnPortReadData = router_send_afn_14_F1;
            portContext->OnPortReadData = router_check_urgent_timeout;
            portContext->cur_plc_task_step = PLC_READ_METER_ROUTER_MODE; /*�����ֵ�Ƿ�ֹ����11HF8�����յ�14HF1���󣬲���ֵ����Ӱ��*/
            break;
        }
        break;
    case PLC_TASK_PLC_NET:
        //�ѱ����̽���
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** �ѱ�������յ����󳭱������쳣�������ѱ�ͳ�ƣ�***");
        debug_println_ext(info);
        #endif
        //�����ѱ�ͳ��
        portContext->cur_plc_task = PLC_TASK_PLC_NET;
        portContext->cur_plc_task_step = PLC_NET_END_PAUSE_ROUTER_12_F2;
        portContext->OnPortReadData = router_check_urgent_timeout;
        readport_plc.OnPortReadData = router_send_afn_12_F2;
        break;
    }
	return 0;
}

INT8U router_send_afn_14_F2(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U cur_datetime[6];

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

        cur_datetime[0] = byte2BCD(datetime[SECOND]);
        cur_datetime[1] = byte2BCD(datetime[MINUTE]);
        cur_datetime[2] = byte2BCD(datetime[HOUR]);
        cur_datetime[3] = byte2BCD(datetime[DAY]);
        cur_datetime[4] = byte2BCD(datetime[MONTH]);
        cur_datetime[5] = byte2BCD(datetime[YEAR]);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_REQUEST,DT_F2,cur_datetime,6,portContext);

//     readport_plc.OnPortReadData =  router_wait_send_frame_complete;
    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}
INT8U router_recv_afn_14_F2(objReadPortContext * readportcontext)
{
   // PLCPortContext *portContext;

    //portContext = (PLCPortContext*)readportcontext;
    //readport_plc.OnPortReadData = router_send_afn_14_F2;
    router_send_afn_14_F2_no_change_status(readportcontext);//����Ҫ��һ���������ò��л�״̬��ȷ��֡
	return 0;
}
INT8U router_send_afn_14_F3(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U *meter_no;
    INT16U delay;
    INT8U gb_645_frame[260];

    portContext = (PLCPortContext*)readportcontext;

    meter_no = portContext->frame_recv + portContext->recv_data_pos;

    delay  = portContext->frame_recv[portContext->recv_data_pos+6];
    delay += portContext->frame_recv[portContext->recv_data_pos+7]<<8;

    gb_645_frame[0] = 0;

    if ((compare_string(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,meter_no,6) == 0)
    || (compare_string(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,meter_no,6) == 0))
    {
        if (bin2_int32u(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.item) == 0x0400010C)
        {
#ifdef __ENABLE_ESAM2__
            if (prepare_exec_batch_meter_task_time(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),
                gb_645_frame+1,gb_645_frame,delay,TRUE) == FALSE)
            {
                gb_645_frame[0] = 0;
            }
#endif
        }
    }

    //����ɼ���ģʽ����
    if ((portContext->router_base_info.router_info3.rtu_frame_format)
    && (isvalid_meter_addr(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,FALSE))
    && (gb_645_frame[0] > 0))
    {
        //�޸ı���Ϊ�ɼ���ģʽ�� ����ַ�ӵ���������ײ���
        gb_645_frame[0] = trans_read_frame_to_cjq_mode(gb_645_frame+1,meter_no);
    }

    router_376_2_set_aux_info(portContext->router_work_info.channel_id,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.resp_byte_num,1,FALSE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_REQUEST,DT_F3,gb_645_frame,1+gb_645_frame[0],portContext);

    readport_plc.OnPortReadData = router_wait_send_frame_complete;
    portContext->OnPortReadData = router_check_urgent_timeout;

    return portContext->frame_send_Len;
}

INT8U router_recv_afn_14_F3(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;

    switch(portContext->cur_plc_task)
    {
    case PLC_TASK_READ_METER:
        switch(portContext->cur_plc_task_step)
        {
        case PLC_READ_METER_ROUTER_MODE:
            //readport_plc.OnPortReadData = router_send_afn_14_F3;
            portContext->OnPortReadData = router_send_afn_14_F3;
            break;
        }
        break;
    }
	return 0;
}
INT8U router_recv_afn_14_F4(objReadPortContext * readportcontext)
{

    router_send_afn_14_F4_no_change_status(readportcontext);//����Ҫ��һ���������ò��л�״̬��ȷ��֡
	return 0;
}
INT8U router_send_afn_14_F4_no_change_status(objReadPortContext * readportcontext)
{
    INT32U item;
    INT8U data[20] = {0},resp[50],frame[30];
    INT8U meter_no[6] = {0};
    INT8U datalen = 0;
    INT8U protocol = 0;
    INT8U  send_len = 0,len;
    INT8U oad_byte[4] ={0};
    INT8U error_code=0;
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;
    protocol = portContext->frame_recv[portContext->recv_data_pos];

    mem_set(meter_no,6,0xAA);
    item = bin2_int32u(portContext->frame_recv + portContext->recv_data_pos + 1);

    
        if(protocol == 1) /*645Э��*/
        {
            send_len = make_gb645_2007_read_frame(frame,meter_no,item,NULL,0);
        }
        else if(protocol == 2)
        {
            oad_byte[3] = item>>24;
            oad_byte[2] = item>>16;
            oad_byte[1] = item>>8;
            oad_byte[0] = item;
            send_len= make_oop_cur_frame(frame,meter_no,1,oad_byte);
        }
        len=do_read_meter_cur_data_rs485(frame,send_len,resp,0,50,100,COMMPORT_485_CY,GB645_2007);//��ʱʱ�䣬��λ10ms

        if((len >12) && (protocol ==1))
        {
            datalen = (len -12);
            if(datalen > 20)
            {
                datalen = 20;
            }
            data[0] = 1;
            mem_cpy(data+1,resp+10,datalen);
        }
        else if((len >20) && (protocol ==2))
        {
            datalen = (len -24);
            if(datalen > 20)
            {
                datalen = 20;
            }
            data[0] = 2; /*1�ֽ�Э��*/

            mem_cpy(data+1,oad_byte,4);

            mem_cpy(data+5,resp+23,datalen-4);
        }
    

    router_send_3762_frame_no_change_status(portContext,DL69842_AFN_REQUEST,DT_F4,data,datalen+1);

    return 0;
}
INT8U router_send_trans_frame(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    portContext->frame_send_Len = (portContext->urgent_task_req_frame_len > READPORT_PLC_FRAME_SIZE) ? READPORT_PLC_FRAME_SIZE : portContext->urgent_task_req_frame_len;

    mem_cpy(portContext->frame_send,portContext->urgent_task_req_frame,portContext->frame_send_Len);

    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_check_urgent_timeout;
    }

    return portContext->frame_send_Len;
}
//·�ɲ�ѯ��F31���ӽڵ���λ��Ϣ
INT8U router_send_afn_10_F31(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    {
        //
        portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F31,portContext->task_read_module_id.node_start_seq,3,portContext);
        //portContext->params.task_check_doc.strat_seq

    }

    if(portContext->urgent_task)
    {
        readport_plc.OnPortReadData = router_urgent_task_send_idle;
    }
    else
    {
        readport_plc.OnPortReadData = router_wait_resp_frame;
    }

    return portContext->frame_send_Len;
}
//·�ɲ�ѯ��F31���ӽڵ���λ��Ϣ
INT8U router_recv_afn_10_F31(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT16U node_count = 0;
    INT16U start_seq = 0;
    INT16U meter_idx = 0,save_pos=0;
    INT8U pos = 0;
    INT8U count = 0;
    INT8U idx = 0;
    INT8U phase,uclass,router_protocol;
    struct{
        INT8U seq[4];
        INT8U read_date[5];
        METER_READ_INFO meter_read_info;
        INT8U value[27];
    }var;
    HPLC_PHASE phase_temp;
    INT8U test;

    portContext = (PLCPortContext*)readportcontext;

    mem_cpy(portContext->task_read_module_id.slave_node_count,portContext->frame_recv+portContext->recv_data_pos,2);
    pos += 2;
    node_count = bin2_int16u(portContext->task_read_module_id.slave_node_count);    //����
    pos += 2; //���

    count = portContext->frame_recv[portContext->recv_data_pos+pos];       //���νڵ���
    pos++;

    for (idx=0;idx<count;idx++)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** query node : %02x%02x%02x%02x%02x%02x ***",
                portContext->frame_recv[portContext->recv_data_pos+pos+5],
                portContext->frame_recv[portContext->recv_data_pos+pos+4],
                portContext->frame_recv[portContext->recv_data_pos+pos+3],
                portContext->frame_recv[portContext->recv_data_pos+pos+2],
                portContext->frame_recv[portContext->recv_data_pos+pos+1],
                portContext->frame_recv[portContext->recv_data_pos+pos+0]);
        debug_println_ext(info);
        #endif

       // mem_cpy(node_info.value,portContext->frame_recv+portContext->recv_data_pos+pos+6,2);

        phase = portContext->frame_recv[portContext->recv_data_pos+pos+6];

        //todo�����������м�����
        if(TRUE == memory_fast_index_find_node_no(COMMPORT_PLC,portContext->frame_recv+portContext->recv_data_pos+pos,&meter_idx,&save_pos,&router_protocol,&uclass))
        {
            if(phase != 0) /*�����0������Ҳû����*/
            {
                meter_idx &= FAST_IDX_MASK;
                fread_array(meter_idx,PIM_CUR_READ_INFO,var.seq,sizeof(var));


                phase_temp.value = 0;

                phase_temp.phase_info = phase & 0x07;

                phase_temp.phase_type = (phase &0xE0) >> 5;

                if((phase & 0x10) == 0x10)
                {
                    phase_temp.line_abnormal = 1;
                }

                if((phase & 0x08) == 0x08)
                {
                    phase_temp.meter_type = 1;
                }

                var.meter_read_info.phase.value = phase_temp.value;

                fwrite_array(meter_idx,PIM_CUR_READ_INFO,var.seq,sizeof(var));

            }
        }
        pos += 8;
    }
    start_seq = bin2_int16u(portContext->task_read_module_id.node_start_seq);
    if ((count == 0) || (start_seq >= node_count) || ((start_seq + ROUTER_OPT_NODE_COUNT) > node_count))   //
    {
                  //����״̬������ȴ���һ�����������״̬

            urgent_task_in_wait_next_urgent_task(portContext);
    }
    else   //������Ҫ��ѯ�Ľڵ���Ϣ
    {

        node_count = bin2_int16u(portContext->task_read_module_id.node_start_seq);
        node_count += ROUTER_OPT_NODE_COUNT;
        portContext->task_read_module_id.node_start_seq[0] = node_count;
        portContext->task_read_module_id.node_start_seq[1] = node_count>>8;
        portContext->task_read_module_id.query_node_cnt = ROUTER_OPT_NODE_COUNT;    //ÿ�β�ѯROUTER_OPT_NODE_COUNT��

        readport_plc.OnPortReadData = router_send_afn_10_F31;
    }


    return 0;
}
INT8U router_process_recv_frame(objReadPortContext * readportcontext)
{

    PLCPortContext *portContext;
    DL69842_RRR RRR;   // ��Ϣ�����нṹ
    INT16U fn,pos,frame_len=0;
    INT8U afn;
    //INT8U tmp;

    portContext = (PLCPortContext*)readportcontext;
    
    if ((portContext->cur_plc_task_step == PLC_IDLE_ROUTER_RESET_LOW)
    || (portContext->cur_plc_task_step == PLC_IDLE_ROUTER_RESET_WAIT_1S)
    || (portContext->cur_plc_task_step == PLC_IDLE_ROUTER_RESET_HIGH)) return 0;//�ն����ڸ�λ·�ɣ��յ����Ĳ�����

    portContext->plc_no_interactive_time_10ms = os_get_systick_10ms(); //10����?�н�����?·��

    if ((portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC09) && (portContext->cur_plc_task == PLC_TASK_CHECK_DOC))
    {
        return router_process_recv_frame_friendcom(readportcontext);
    }

    #if (defined __FUJIAN_SUPPLEMENT_SPECIFICATION__)
    if(portContext->fujian_ctrl.protocol_type == PROTOCOL_FUJIAN)
	{
		return router_process_recv_frame_fujian(readportcontext);// 
	}
    #endif
    
    //��Ϣ��
    mem_cpy(RRR.value,portContext->frame_recv + POS_DL69842_INFO,sizeof(DL69842_RRR));
    //�ŵ�����λ
    portContext->router_work_info.channel_id = RRR.resp_relay_channel.channel;
    portContext->router_work_info.phase = portContext->router_work_info.channel_id - 1;
    if(portContext->router_work_info.channel_id == 0)portContext->router_work_info.phase = 3; //ʹ��ת��3��ȥ����δ�޸�channel_id-1�ķ�ʽ����Ϊ���õĵط��Ƚ϶࣬��+1�ĵط�
    if(portContext->router_work_info.phase > 3)      //������0,1,2,3,��4���ŵ�
    {
        portContext->router_work_info.phase=0;
    }
    portContext->router_work_info.frame_seq = RRR.reserved2;
//    //ʵ�����

    //�м����
    portContext->router_work_info.relay = RRR.resp_relay_channel.relay;

    //��Ϣ���
    pos = POS_DL69842_ADDR;
    if(RRR.resp_relay_channel.modem)
    {
        frame_len = bin2_int16u(portContext->frame_recv+1);

        if(frame_len > (pos+12)) //��ֹ�г���·�ɻظ��Ĵ���ַ��ʶ����ʵ���ֲ���
        {
            pos +=12;
        }
    }

    afn = portContext->frame_recv[pos++];
    fn = portContext->frame_recv[pos++];
    fn += portContext->frame_recv[pos++]<<8;
    portContext->recv_data_pos = pos;

    if(portContext->urgent_task)
    {
        if(portContext->urgent_task_step == PLC_URGENT_TASK_TRANS_SEND_376_2)
        {
            //�������� Ҫ�ж�һ��resp�ĳ���
            tpos_enterCriticalSection();
            if(portContext->urgent_task_resp_frame != NULL)
            {
                portContext->urgent_task_resp_frame_len = (portContext->frame_recv_Len <= portContext->urgent_task_resp_frame_len) ? portContext->frame_recv_Len : portContext->urgent_task_resp_frame_len;
                mem_cpy(portContext->urgent_task_resp_frame,portContext->frame_recv,portContext->urgent_task_resp_frame_len);
            }
            tpos_leaveCriticalSection();
            //����״̬������ȴ���һ�����������״̬
            urgent_task_in_wait_next_urgent_task(portContext);
        }
        else if (portContext->urgent_task_step == PLC_URGENT_TASK_TRANS_SEND_TOPSCOMM)
        {
            //�������� Ҫ�ж�һ��resp�ĳ���
            tpos_enterCriticalSection();
            if(portContext->urgent_task_resp_frame != NULL)
            {
                portContext->urgent_task_resp_frame_len = (portContext->frame_recv_Len <= portContext->urgent_task_resp_frame_len) ? portContext->frame_recv_Len : portContext->urgent_task_resp_frame_len;
                mem_cpy(portContext->urgent_task_resp_frame,portContext->frame_recv,portContext->urgent_task_resp_frame_len);
            }
            tpos_leaveCriticalSection();
            urgent_task_in_wait_next_urgent_task(portContext);
        }
        //�����·��͸��ģʽ�����ڷ�ת��376.2�Ͷ��ű��ĵĽ�������ʱ��ֱ����֯10F1����Ӧ֡����·�ɵı���
        else if(gAppInfo.router_trans_mode == 1)
        {
            objResponse *pResp;
            //��֯10F1����Ӧ���ģ�֡���Ĭ��0,��Э�鴦������pResponse
            //����Response����
            pResp = xList_RESPONSE_get();
            pos = 0;
            //���δ���뵽������ȴ�100ms��1s��δ���뵽���򷵻�ʧ��
            while(pResp==NULL)
            {
                DelayNmSec(100);
                pos++;
                if(pos == 10) break;
                if(pResp==NULL)
                {
                    pResp = xList_RESPONSE_get();
                }

            }
            if(pResp!=NULL)
            {
                //����֮ǰ�·���͸�����Ķ�Ӧ��channel������Ӧ����
                if(gAppInfo.trans_pChannel == NULL)
                {
                    //Ĭ����232ͨ��
                    pResp->pChannel = &RS232Channel;
                    pResp->channel = CHANNEL_RS232;
                }
                else
                {
                    pResp->pChannel = gAppInfo.trans_pChannel;
                    pResp->channel = gAppInfo.trans_channel;
                }
                //������Ӧ֡��������λ��
                pResp->max_reply_pos = MAX_SIZE_PLMSDTP-4;  //ACDλΪ1
                
                pResp->frame[0] = 0x68;
                pResp->frame[5] = 0x68;

                //�ն˵�ַ
                mem_cpy(pResp->frame+POS_RT,gSystemInfo.ertu_devid,CFG_DEVID_LEN);

                pResp->frame[POS_MSA]  = 0;
                pResp->frame[POS_CTRL] = 0;
                pResp->frame[POS_CTRL] = CTRLFUNC_SET_DIR | CTRLFUNC_USERDATA;
                //���������C��ACDλ
                pResp->frame[POS_CTRL] &= CTRLFUNC_CLR_ACD;
                pResp->frame[POS_CTRL] &= CTRLFUNC_CLR_PRM;

                //֡��ŵĴ���֡���ͳһ��0
                //���õ�֡��־,���ȷ��,��Ҫȷ�ϣ���TP��ǩ
                pResp->frame[POS_SEQ] = MASK_FIR | MASK_FIN | MASK_TpV;
                pResp->frame[POS_SEQ]  &= ~MASK_CON;
                pResp->frame[POS_SEQ] |= 0;
                
                pResp->frame[POS_AFN]  = AFN_RELAY;
                
                pResp->pos = POS_DATA;
                
                //����pn��fn
                set_pn_fn(pResp->frame, &pResp->pos, DA_P0, DT_F1);
                pResp->frame[pResp->pos++] = 0x1F;  //�˿ں�
                pResp->frame[pResp->pos++] = portContext->frame_recv[1];   //����
                pResp->frame[pResp->pos++] = portContext->frame_recv[2];
              //  frame_len = portContext->frame_recv[1];
              //  frame_len += portContext->frame_recv[2] >> 8;
                frame_len = bin2_int16u(portContext->frame_recv+1);
                mem_cpy(pResp->frame+pResp->pos,portContext->frame_recv,frame_len);
                pResp->pos += frame_len;
                
                //�¼�������EC
                pResp->frame[pResp->pos++] = g_event.ec1;
                pResp->frame[pResp->pos++] = g_event.ec2;

                //ʱ���ǩTP
                pResp->frame[pResp->pos++] = 0;
                pResp->frame[pResp->pos++] = byte2BCD(datetime[SECOND]);
                pResp->frame[pResp->pos++] = byte2BCD(datetime[MINUTE]);
                pResp->frame[pResp->pos++] = byte2BCD(datetime[HOUR]);
                pResp->frame[pResp->pos++] = byte2BCD(datetime[DAY]);
                pResp->frame[pResp->pos++] = 0x05;
                
                //��ȥ�̶�����֡����ͷ
                pResp->pos -= 6;

                //���ݳ�������2λ
                pResp->pos<<=2;
                pResp->protocol_type = QGDW_376_1;
                pResp->pos += pResp->protocol_type;
                
                //���ó���
                pResp->frame[1] =  pResp->pos;
                pResp->frame[2] =  pResp->pos >> 8;
                pResp->frame[3] =  pResp->frame[1];
                pResp->frame[4] =  pResp->frame[2];

                //����У��λ
                app_encodeFrame(pResp);

                //������Ӧ֡
                app_send_ReplyFrame(pResp);
            }
            //����״̬������ȴ���һ�����������״̬
            urgent_task_in_wait_next_urgent_task(portContext);
            return 0;
        }
        else
        {
            switch(afn)
            {
            case DL69842_AFN_CONFIRM:            //0x00ȷ�ϱ���
                switch(fn)
                {
                case DT_F1:
                    router_recv_afn_00_F1(readportcontext);
                    break;
                case DT_F2:
                    router_recv_afn_00_F2(readportcontext);
                    break;
                default:
                    urgent_task_in_wait_next_urgent_task(portContext); /*�յ��˷�Ԥ�ڵ�Ӧ�𣬴���һ��*/
                    break;
                }
                break;

            case DL69842_AFN_TRANS:              //0x02����ת��
                switch(fn)
                {
                case DT_F1:
                    router_recv_afn_02_F1(readportcontext,NULL);
                    break;
                default:
                    urgent_task_in_wait_next_urgent_task(portContext); /*�յ��˷�Ԥ�ڵ�Ӧ�𣬴���һ��*/
                    break;
                }
                break;
            case DL69842_AFN_QUERY:              //0x03��ѯ
                switch(fn)
                {
                #if defined(__READ_PLC_NOISE__)
                case DT_F2:
                    router_recv_afn_03_F2(readportcontext);
                    break;
                #endif
                case DT_F9:
                     router_recv_afn_03_F9(readportcontext);
                     break;
             //   case DT_F10:    /*��Ҫ�Ž�����ֻ�Կ�������������ж��ſ���*/
                default:
                     urgent_task_in_wait_next_urgent_task(portContext); /*�յ��˷�Ԥ�ڵ�Ӧ�𣬴���һ��*/
                     break;
                }
                break;
            case DL69842_AFN_REPORT:             //0x06�����ϱ�
                switch(fn)
                {
                case DT_F1:
                    router_recv_afn_06_F1(readportcontext);
                    break;
                case DT_F2:
                    router_recv_afn_06_F2(readportcontext);
                    break;
                case DT_F4:
                    router_recv_afn_06_F4(readportcontext);
                    break;
                case DT_F5:
                    router_recv_afn_06_F5(readportcontext);
                    break;
                  //  #ifdef  __SHANXI_READ_BPLC_NETWORK_INFO__
                case DT_F10:
                    router_recv_afn_06_F10(readportcontext);
                    break;
                 //   #endif
                default:
                    urgent_task_in_wait_next_urgent_task(portContext); /*�յ��˷�Ԥ�ڵ�Ӧ�𣬴���һ��*/
                    break;
                }
                break;
            case DL69842_AFN_RQUERY:             //0x10·������ѯ
                switch(fn)
               {
               case DT_F4:
                    router_recv_afn_10_F4(readportcontext);
                    break;
                #ifdef __READ_MODULE_ID_PLAN__
                case DT_F7:
                    router_recv_afn_10_F7(readportcontext);
                    break;
                case DT_F31:
                    router_recv_afn_10_F31(readportcontext);
                    break;
                case DT_F112:
                    router_recv_afn_10_F112(readportcontext);
                    break;
                #endif
               default:
                    urgent_task_in_wait_next_urgent_task(portContext); /*�յ��˷�Ԥ�ڵ�Ӧ�𣬴���һ��*/
                    break;
               }
               break;
            case DL69842_AFN_RTRANS:             //0x13·������ת��
                switch(fn)
                {
                case DT_F1:
                     router_recv_afn_13_F1(readportcontext,NULL);
                     break;
                default:
                     urgent_task_in_wait_next_urgent_task(portContext); /*�յ��˷�Ԥ�ڵ�Ӧ�𣬴���һ��*/
                     break;
                }
                 break;
            case DL69842_AFN_REQUEST: /*0x14·�����󳭶�*/
                switch(fn)
                {
                case DT_F2:
                    router_recv_afn_14_F2(readportcontext);
                    break;
                case DT_F4:
                    router_recv_afn_14_F4(readportcontext);
                    break;
                }
                break;
            case DL69842_AFN_PARALLEL:
                switch(fn)
                {
                case DT_F1:
                    router_recv_afn_F1_F1(readportcontext,&RRR);
                    break;
                default:
                    urgent_task_in_wait_next_urgent_task(portContext); /*�յ��˷�Ԥ�ڵ�Ӧ�𣬴���һ��*/
                    break;
                }
                break;
            case DL69842_AFN_DEBUG:
               switch(fn)
               {
               #ifdef __PLC_BPLC_AGG__
               case DT_F11:
                   router_recv_afn_F0_F11(readportcontext);
                    break;
               case DT_F16:
                   router_recv_afn_F0_F16(readportcontext);
                    break;
               #endif
               #if defined(__SHANXI_READ_BPLC_NETWORK_INFO__)
               case DT_F100:
                    router_recv_afn_F0_F100(readportcontext);
                    break;
               case DT_F102:
                    router_recv_afn_F0_F102(readportcontext);
                    break;
               case DT_F103:
                    router_recv_afn_F0_F103(readportcontext);
                    break;
               default:
                    urgent_task_in_wait_next_urgent_task(portContext); //����ȴ���һ�����������״̬
                    break;
               #endif
               }
               break;
               
            default:
                if(portContext->urgent_task_step == PLC_URGENT_TASK_PAUSE_ROUTER_12_F2)
                {
                    readport_plc.OnPortReadData = router_send_afn_12_F2;
                    #ifdef __SOFT_SIMULATOR__
                    snprintf(info,100,"*** ���ϱ������󳭱��ͻ�ˣ��ط���ͣ·�� ***");
                    debug_println_ext(info);
                    #endif
                }
                else
                {
                    urgent_task_in_wait_next_urgent_task(portContext); /*�յ��˷�Ԥ�ڵ�Ӧ�𣬴���һ��*/
                }
                break;
            }
        }
    }
    else
    {
//        if (portContext->router_work_info.status.pause) return 0;

        switch(afn)
        {
        case DL69842_AFN_CONFIRM:            //0x00ȷ�ϱ���
            switch(fn)
            {
            case DT_F1:
            router_recv_afn_00_F1(readportcontext);
            break;
            case DT_F2:
                router_recv_afn_00_F2(readportcontext);
                break;
            }
            break;
        case DL69842_AFN_INIT:               //0x01��ʼ��
            break;
        case DL69842_AFN_TRANS:              //0x02����ת��
            switch(fn)
            {
            case DT_F1:
                router_recv_afn_02_F1(readportcontext,&RRR);
            break;
            }
            break;
        case DL69842_AFN_QUERY:              //0x03��ѯ
            switch(fn)
            {
            case DT_F1:
                router_recv_afn_03_F1(readportcontext);
                break;
            case DT_F4:
                router_recv_afn_03_F4(readportcontext);
                break;
            case DT_F9:
                router_recv_afn_03_F9(readportcontext);
                break;
            case DT_F10:
                router_recv_afn_03_F10(readportcontext);
                break;
			#ifdef __READ_MODULE_ID_PLAN__
			case DT_F12:
				router_recv_afn_03_F12(readportcontext);
				break;
			#endif
            case DT_F21:
                router_recv_afn_03_F21(readportcontext);
                break;
            }
            break;
        case DL69842_AFN_TEST:               //0x04�ز���·�ӿڼ��
            break;
        case DL69842_AFN_CTRL:               //0x05��������
            break;
        case DL69842_AFN_REPORT:             //0x06�����ϱ�
            switch(fn)
            {
            case DT_F1:
                router_recv_afn_06_F1(readportcontext);
                break;
            case DT_F2:

                #ifdef __F170_REC_TIME_IS_DAY_HOLD_READ_TIME__
                if(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.read_type == READ_TYPE_CYCLE_DAY)
                                update_meter_recstate(bin2_int16u(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_idx),
                        COMMPORT_PLC,(RRR.rpt_flag & 0x02) ? (0x80|(portContext->router_work_info.phase+1)) : (portContext->router_work_info.phase+1),
                        (RRR.phase_q.spec == 4) ? (0x80|RRR.phase_q.phase) : RRR.phase_q.phase,RRR.resp_relay_channel.relay,RRR.phase_q.value[1],TRUE);
                #else
                {
                update_meter_recstate(bin2_int16u(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_idx),
                        COMMPORT_PLC,(RRR.rpt_flag & 0x02) ? (0x80|(portContext->router_work_info.phase+1)) : (portContext->router_work_info.phase+1),
                        (RRR.phase_q.spec == 4) ? (0x80|RRR.phase_q.phase) : RRR.phase_q.phase,RRR.resp_relay_channel.relay,RRR.phase_q.value[1],TRUE);
                }
                #endif

                router_recv_afn_06_F2(readportcontext);
				/*
                #ifdef __METER_EVENT_REPORT__
                if (RRR.rpt_flag & 0x01)
                {
                	fread_array(bin2_int16u(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_idx),PIM_METER_REPORT_EVENT_STATE,&tmp,1);
					if( (tmp == 0x00) || (tmp == 0xFF) )
					{
                    	//���¼�
                    	tmp = 0xFE;
                    	fwrite_array(bin2_int16u(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_idx),PIM_METER_REPORT_EVENT_STATE,&tmp,1);
					}
					//RRR.rpt_flag = 0;
                }
                #endif
                */
                break;
            case DT_F3:
                router_recv_afn_06_F3(readportcontext);
                break;
            case DT_F4:
                router_recv_afn_06_F4(readportcontext);
                break;
            case DT_F5:
                router_recv_afn_06_F5(readportcontext);
                break;
           // #ifdef  __SHANXI_READ_BPLC_NETWORK_INFO__
            case DT_F10:
                router_recv_afn_06_F10(readportcontext);
                break;
           // #endif
            }
            break;
        case DL69842_AFN_RQUERY:             //0x10·������ѯ
            switch(fn)
            {
            case DT_F1:
                router_recv_afn_10_F1(readportcontext);
                break;
            case DT_F2:
                router_recv_afn_10_F2(readportcontext);
                break;
            #if (defined __PROVICE_CHONGQING__)    
            case DT_F3:
                router_recv_afn_10_F3(readportcontext);
                break;
            #endif    
            case DT_F4:
                router_recv_afn_10_F4(readportcontext);
                break;
            #ifdef __READ_MODULE_ID_PLAN__
            case DT_F112:
                router_recv_afn_10_F112(readportcontext);
                break;
            case DT_F31:
                router_recv_afn_10_F31(readportcontext);
                break;
            #endif
            }
            break;
        case DL69842_AFN_RSET:               //0x11·��������
            break;
        case DL69842_AFN_RCTRL:              //0x12·�ɿ���
            break;
        case DL69842_AFN_RTRANS:             //0x13·������ת��
            router_recv_afn_13_F1(readportcontext,&RRR);
            break;
        case DL69842_AFN_REQUEST:            //0x14·�����ݳ���
            switch(fn)
            {
            case DT_F1:
                router_recv_afn_14_F1(readportcontext);
                break;
            case DT_F2:
                router_recv_afn_14_F2(readportcontext);
                break;
            case DT_F3:
                router_recv_afn_14_F3(readportcontext);
                break;
            case DT_F4:
                router_recv_afn_14_F4(readportcontext);
                break;
            }
            break;
        case DL69842_AFN_DEBUG:              //0xF0�ڲ�����
            break;
        case DL69842_AFN_PARALLEL:         //F1��������
            router_recv_afn_F1_F1(readportcontext,&RRR);
            break;
        default:
            break;
        }

    }

    return 0;
}

INT16U plc_trans_send_meter_frame(INT8U* req,INT16U req_len,INT8U* resp,INT16U max_resp_len,INT16U max_wait_time_10ms,READPORT_METER_DOCUMENT *meter_doc)
{
    INT32U wait_time_10ms = 0;
    INT16U resp_len = 0;

    if(req_len == 0) return 0;

    wait_time_10ms = system_get_tick10ms();

    /*��ȡ�ź��� �����ĵȴ�ʱ�䣬����������ȴ�ʱ�䣬�򷵻�0
    �˺����Ѿ����� */
    if(0 == tpos_mutexGet(&SIGNAL_PLC_METER_MONITOR,FALSE,max_wait_time_10ms))
    {
        return 0;
    }
    
    while(portContext_plc.urgent_task_id != RECMETER_TASK_NONE)
    {
        DelayNmSec(10);
        
        if(time_elapsed_10ms(wait_time_10ms) > max_wait_time_10ms)
        {
            //�ͷ�ʹ��Ȩ
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
            return 0;
        }
    }


    if(!check_is_sqr_protocol(meter_doc->protocol))
    {
        if ((isvalid_meter_addr(meter_doc->rtu_no,FALSE)) && (portContext_plc.router_base_info.router_info3.rtu_no_mode) && (portContext_plc.router_base_info.router_info3.rtu_frame_format))
        {
            //����ɼ���ģʽ����
            req_len = trans_read_frame_to_cjq_mode(req,meter_doc->rtu_no);
        }
    }

    tpos_enterCriticalSection();
    if (portContext_plc.urgent_task_id == RECMETER_TASK_NONE) 
    {
        portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_METER_FRAME;
        portContext_plc.urgent_task_req_frame = req;
        portContext_plc.urgent_task_resp_frame = resp;
        portContext_plc.urgent_task_req_frame_len = req_len;
        portContext_plc.urgent_task_resp_frame_len = max_resp_len;
        portContext_plc.urgent_task_meter_doc = meter_doc;
    }
    else
    {
        tpos_leaveCriticalSection();
        //�ͷ�ʹ��Ȩ
        tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
        return 0;
    }
    tpos_leaveCriticalSection();

    while(portContext_plc.urgent_task_id != RECMETER_TASK_NONE)
    {
        DelayNmSec(10);

        /*�����ĳɸ���max_wait_time_10ms ���жϣ�Ӧ�ò㱣֤max_wait_time_10ms ��ȷ��*/
		/*if(time_elapsed_10ms(wait_time_10ms) > max_wait_time_10ms)*/
        if(time_elapsed_10ms(wait_time_10ms) > 100*(portContext_plc.router_base_info.max_monitor_meter_timeout_s+10))
        {
            /* ����ֵ��� ���üӹؼ������� */
            portContext_plc.urgent_task_resp_frame = NULL;// ָ���ÿ�
            //�ͷ�ʹ��Ȩ
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
            return 0;
        }
    }

    if (portContext_plc.urgent_task_resp_frame_len > 0)
    {
        if(!check_is_sqr_protocol(meter_doc->protocol))
        {
        if ((isvalid_meter_addr(meter_doc->rtu_no,FALSE)) && (portContext_plc.router_base_info.router_info3.rtu_no_mode) && (portContext_plc.router_base_info.router_info3.rtu_frame_format))
        {
            portContext_plc.urgent_task_resp_frame_len = trans_read_frame_cjq_mode_to_standard_format(portContext_plc.urgent_task_resp_frame,meter_doc->meter_no);
        }
        }
    }
    /*�ͷ��ź���ǰ ��ȡ������ */
    //���������쳣�ϱ�������� resp_len�����ֵ�����⣬ʵ����û���յ����ġ���վ����ʾ1800����ֽڵ�0����
    resp_len = portContext_plc.urgent_task_resp_frame_len;
    //�ͷ�ʹ��Ȩ
    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);

    return resp_len;
}

INT16U plc_trans_send_376_2_frame(INT8U* req,INT16U req_len,INT8U* resp,INT16U max_resp_len,INT16U max_wait_time_10ms)
{
    INT32U wait_time_10ms = 0;
    INT16U resp_len = 0;
    
    if(req_len == 0) return 0;

    wait_time_10ms = system_get_tick10ms();
    
    /*��ȡ�ź��� �����ĵȴ�ʱ�䣬����������ȴ�ʱ�䣬�򷵻�0
    �˺����Ѿ����� */
    if(0 == tpos_mutexGet(&SIGNAL_PLC_METER_MONITOR,FALSE,max_wait_time_10ms))
    {
        return 0;
    }
    
    while(portContext_plc.urgent_task_id != RECMETER_TASK_NONE)
    {
        DelayNmSec(10);
        if(time_elapsed_10ms(wait_time_10ms) > max_wait_time_10ms)
        {
            //�ͷ�ʹ��Ȩ
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
            return 0;
        }
    }

    tpos_enterCriticalSection();
    if (portContext_plc.urgent_task_id == RECMETER_TASK_NONE) 
    {
        portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_376_2_FRAME;
        portContext_plc.urgent_task_req_frame = req;
        portContext_plc.urgent_task_resp_frame = resp;
        portContext_plc.urgent_task_req_frame_len = req_len;
        portContext_plc.urgent_task_resp_frame_len = max_resp_len;
        portContext_plc.plc_wait_resp_long_time_10ms = max_wait_time_10ms;
    }
    else
    {
        tpos_leaveCriticalSection();
        //�ͷ�ʹ��Ȩ
        tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
        return 0;
    }
    tpos_leaveCriticalSection();
   
    while(portContext_plc.urgent_task_id != RECMETER_TASK_NONE)
    {
        DelayNmSec(10);
        if(time_elapsed_10ms(wait_time_10ms) > max_wait_time_10ms)
        {
            /* ����ֵ��� ���üӹؼ������� */
            portContext_plc.urgent_task_resp_frame = NULL;// ָ���ÿ�
            //�ͷ�ʹ��Ȩ
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
            return 0;
        }
    }

    resp_len = portContext_plc.urgent_task_resp_frame_len;
    //�ͷ�ʹ��Ȩ
    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);

    return resp_len;
}

INT16U plc_trans_send_topscomm_frame(INT8U* req,INT16U req_len,INT8U* resp,INT16U max_resp_len,INT16U max_wait_time_10ms)
{
    INT32U wait_time_10ms = 0;
    INT16U resp_len = 0;

    if(req_len == 0) return 0;

    wait_time_10ms = system_get_tick10ms();
    /*��ȡ�ź��� �����ĵȴ�ʱ�䣬����������ȴ�ʱ�䣬�򷵻�0
    �˺����Ѿ����� */
    if(0 == tpos_mutexGet(&SIGNAL_PLC_METER_MONITOR,FALSE,max_wait_time_10ms))
    {
        return 0;
    }

    while(portContext_plc.urgent_task_id != RECMETER_TASK_NONE)
    {
        DelayNmSec(10);
        if(time_elapsed_10ms(wait_time_10ms) > max_wait_time_10ms)
        {
            //�ͷ�ʹ��Ȩ
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
            return 0;
        }
    }    

    

    tpos_enterCriticalSection();
    if (portContext_plc.urgent_task_id == RECMETER_TASK_NONE) 
    {
        portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_TOPSCOMM_FRAME;
        portContext_plc.urgent_task_req_frame = req;
        portContext_plc.urgent_task_resp_frame = resp;
        portContext_plc.urgent_task_req_frame_len = req_len;
        portContext_plc.urgent_task_resp_frame_len = max_resp_len;
    }
    else
    {
        tpos_leaveCriticalSection();
        //�ͷ�ʹ��Ȩ
        tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
        return 0;
    }
    tpos_leaveCriticalSection();    
    
    while(portContext_plc.urgent_task_id != RECMETER_TASK_NONE)
    {
        DelayNmSec(10);
        if(time_elapsed_10ms(wait_time_10ms) > max_wait_time_10ms)
        {
            /* ����ֵ��� ���üӹؼ������� */
            portContext_plc.urgent_task_resp_frame = NULL;// ָ���ÿ�
            //�ͷ�ʹ��Ȩ
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
            return 0;
        }
    }
    resp_len = portContext_plc.urgent_task_resp_frame_len;
    //�ͷ�ʹ��Ȩ
    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);

    return resp_len;
}

INT16U plc_trans_send_cast_frame(INT8U* req,INT16U req_len,INT16U max_wait_time_10ms,INT8U cast_flag)
{
    INT32U wait_time_10ms = 0;

    if(req_len == 0)
    {
        return 0;
    }

    if (portContext_plc.router_base_info.router_vendor == ROUTER_VENDOR_FC09) 
    {
        return 0; //��Ѷ��09·�ɲ�֧�ֹ㲥
    }
    if(portContext_plc.read_status.plc_net)
    {
        return 0;
    }
    if(portContext_plc.cur_plc_task == PLC_TASK_PLC_NET)
    {
        return 0;
    }

    wait_time_10ms = system_get_tick10ms();
    /*��ȡ�ź��� �����ĵȴ�ʱ�䣬����������ȴ�ʱ�䣬�򷵻�0
    �˺����Ѿ����� */
    if(0 == tpos_mutexGet(&SIGNAL_PLC_METER_MONITOR,FALSE,max_wait_time_10ms))
    {
        return 0;
    }
    
    while(portContext_plc.urgent_task_id != RECMETER_TASK_NONE)
    {
        DelayNmSec(10);
        if(time_elapsed_10ms(wait_time_10ms) > max_wait_time_10ms)
        {
            //�ͷ�ʹ��Ȩ
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
            return 0;
        }
    }

    tpos_enterCriticalSection();
    if (portContext_plc.urgent_task_id == RECMETER_TASK_NONE)
    {
        if(cast_flag == 1)
        {
            portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_CAST_TIME;
        }
        else if(cast_flag == 2)
        {
            portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_CAST_TIME_FROM_STATION;
        }
        else
        {
            portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_CAST;
        }
        portContext_plc.cast_content = req;
        portContext_plc.cast_content_len = req_len;
    }
    else
    {
        tpos_leaveCriticalSection();
        //�ͷ�ʹ��Ȩ
        tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
        return 0;
    }
    tpos_leaveCriticalSection();
    
    while(portContext_plc.urgent_task_id != RECMETER_TASK_NONE)
    {
        DelayNmSec(10);
        if(time_elapsed_10ms(wait_time_10ms) > max_wait_time_10ms)
        {
            //�ͷ�ʹ��Ȩ
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
            return 0;
        }
    }

    //�ͷ�ʹ��Ȩ
    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);

    return 0;
}

void check_plc_cast_time_task(void)
{
    #ifdef __PROVICE_JIANGXI__
    INT8U  weekofday;
    #endif

    static INT8U exec_flag = 0xAA;

    if (portContext_plc.router_base_info.router_vendor == ROUTER_VENDOR_FC09) return;  //��Ѷ��09·�ɲ�֧��
    if(portContext_plc.read_status.plc_net) return;
    if(portContext_plc.cur_plc_task == PLC_TASK_PLC_NET) return; //�ѱ��в�ҪУʱ��

    #ifdef __DAYLIGHT_SAVING_TIME__
    if ( (portContext_plc.run_param.adjtime_flag == 0) && (0 == g_DST_params.use_flag.meter_flag) )
    {
        return;   //��ҪУʱ
    }
    #else
    if (portContext_plc.run_param.adjtime_flag == 0) return;   //��ҪУʱ
    #endif
    //if (compare_string(exec_day,datetime+DAY,3) == 0) return;  //����Уʱ����
    if ((portContext_plc.run_param.adjtime_hour*100+portContext_plc.run_param.adjtime_minute) == (datetime[HOUR]*100+datetime[MINUTE]))
    {
        exec_flag = 0x55;
    }

    #ifdef __DAYLIGHT_SAVING_TIME__
    if(0xAA == gAppInfo.DST_BCAST_flag)
    {
        exec_flag = 0x55;
    }
    #endif
    #ifdef __PROVICE_JIANGXI1__ /*����ִ��һ�Σ�����19�������Ѿ�ȥ��*/
      weekofday = 0;
      weekofday=weekDay((datetime[YEAR]),(datetime[MONTH]),(datetime[DAY]));
      if(weekofday != 6) exec_flag = 0xAA;
    #endif

    if(exec_flag != 0x55) return;
    if(0 == tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))
    {
        return ;
    }

    tpos_enterCriticalSection();
    if (portContext_plc.urgent_task_id == RECMETER_TASK_NONE) portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_CAST_TIME;
    tpos_leaveCriticalSection();

    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
    if(portContext_plc.urgent_task_id != RECMETER_TASK_TRANS_CAST_TIME)
    {        
        return;  //��������������ռ���ˣ� ����ִ���ˡ�
    }
    //mem_cpy(exec_day,datetime+DAY,3);
    exec_flag = 0xAA;

    #ifdef __DAYLIGHT_SAVING_TIME__
    gAppInfo.DST_BCAST_flag = 0;/*clear */
    #endif
}
BOOLEAN check_gb3762_router_state(PLCPortContext *portContext)
{
    INT32U time_out_10ms = 0;
    INT8U idx;
    static INT8U no_requst_wait_time = 0;
    
    portContext_plc.need_reset_router = 0;
    portContext->plc_no_interactive_time_10ms = os_get_systick_10ms();
    
    if((portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER) && (portContext->read_status.is_in_read_cycle == 1)) /*��ʱ���ڵĲ�Ҫ�жϣ�ʱ���ⲻ����*/
    {
        /*��Ҫ��ѯһ��״̬������¼��־������Ĵ�����ֱ�Ӵӵײ㷢�͡� */
        for(idx=0;idx<3;idx++)
        {
            router_376_2_set_aux_info(0,0,0,TRUE);
            portContext_plc.frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RQUERY,DT_F4,NULL,0,&portContext_plc);
            channel_plc_send((objReadPortContext *)&portContext_plc); /*��������������͵ı��ģ����¼���ļ�����U��*/
            portContext_plc.recv_status.recv_frame_type = 0;
            portContext_plc.uart_recv_byte_time_out = os_get_systick_10ms();
            time_out_10ms = os_get_systick_10ms();
            while(time_elapsed_10ms(time_out_10ms) < 1000)
            {
                if(channel_plc_recv((objReadPortContext *)&portContext_plc)  == READPORT_RECV_COMPLETE)
                {
                    break;
                }
            }
            
            if (portContext_plc.frame_recv_Len > 0)
            {
                if((portContext_plc.frame_recv_buffer[POS_DL69842_AFN] == DL69842_AFN_RQUERY)
                && (portContext_plc.frame_recv_buffer[POS_DL69842_AFN+1] == 8))        /*���յ�10HF4*/
                {
                    record_log_code(LOG_ROUTER_NO_REQUST,portContext_plc.frame_recv_buffer+POS_DL69842_AFN,19,LOG_ALL);/*��¼��־��10HF4����*/
                    /*���·�ɿ��У���λ���½���*/
                    if ((portContext_plc.frame_recv_buffer[POS_DL69842_AFN+16] == 8)
                    && (portContext_plc.frame_recv_buffer[POS_DL69842_AFN+17] == 8)
                    && (portContext_plc.frame_recv_buffer[POS_DL69842_AFN+18] == 8))/*������08��ʾ0~3ͨ�����У�*/
                    {
                        readport_plc.OnPortReadData = router_reset;
                        portContext->OnPortReadData = router_reset;
                        portContext->cur_plc_task = PLC_TASK_IDLE;
                        portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;
                        
                        return TRUE;/*���ܽŵ�ʱ�򣬲���Ҫ��������ִ����������ˣ�ֱ�ӷ���true*/
                    }
                    else
                    {
                        /*���·��æ���ٵ�һ�����ڣ��������3�����ڻ���æ����λ���½���*/
                        
                        no_requst_wait_time ++ ;
                        
                        if(no_requst_wait_time >=3)
                        {
                            no_requst_wait_time = 0;
                            readport_plc.OnPortReadData = router_reset;
                            portContext->OnPortReadData = router_reset;
                            portContext->cur_plc_task = PLC_TASK_IDLE;
                            portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;
                            
                            return TRUE;
                        }
                    }
                    
                    return FALSE; /*��Ҫ�����ȴ�������flase�����Լ����������ִ��*/
                }
                else
                {
                    /*�յ���������Ӧ��������*/
                }
            }
        }
        
        /*��ѯ3��10HF4��ûͨѶ�ϣ��ȼ�¼��Ȼ�����ܽŸ�λ��*/
        record_log_code(LOG_ROUTER_NO_REQUST,0,0,LOG_ALL);/*��¼��־���������ݣ�˵���Ƕ�����10HF4*/
        
        readport_plc.OnPortReadData = router_reset;
        portContext->OnPortReadData = router_reset;
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;
        
        return TRUE;
        
    }
    return FALSE;
}
/*����Ƿ�Ҫ����·�ɡ�
��һ�֣�����Ҫ���³������ǲ������±ȶԵ����������·������ڳ���������
�ڶ��֣�����Ҫ��λ���еģ����罻���������쳣��
�����֣��Ǳ���ģʽ�£�1��Сʱû���յ�·�ɱ��ģ���ѯ·��״̬�Ƿ���������¼ */
BOOLEAN check_plc_reset_router(PLCPortContext *portContext)
{
   static INT32U tick = 0;
   INT32U time_out_10ms = 0;
   INT8U idx;
   static INT8U no_requst_wait_time = 0;


   if(portContext_plc.need_reset_router == 3)  /*��Ҫ���³�����������������Ҫ*/
   {
        if(tick == 0)/*��10s�����³�����ֹ��ͣ���·�����*/
        {
            tick = os_get_systick_10ms();
        }

        if (time_elapsed_10ms(tick) > 1000)   //10s
        {
             tick = 0;
             portContext_plc.need_reset_router = 0;
             start_read_meter(portContext);
             return TRUE;
        }
        else
        {
             return FALSE;
        }
   }
   else if(portContext_plc.need_reset_router == 1)   /*���ܽŸ�λ���е�����쳣�ˣ���Ҫ���¸�λ·��*/
   {

        readport_plc.OnPortReadData = router_reset;
        portContext->OnPortReadData = router_reset;
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;

        portContext_plc.need_reset_router = 0;
        return TRUE;

   }
    #ifdef __FUJIAN_SUPPLEMENT_SPECIFICATION__
    else if (time_elapsed_10ms(portContext->plc_no_interactive_time_10ms) > (PLC_REC_MODE_NO_INTERACTIVE_SECOND_MAX*100))
    {
        if(gAppInfo.acquire_mode == 1)
        {
            /*��ʱ ���з���ִ��  ��ʱ����ô����:���ع������б� ��·�ɸ�λ �������������� 
             18-04-17������� */
            if(portContext->fujian_ctrl.cur_exec_plan_id > 0)
            {
                /* ���´���plan_list�б� Ȼ�� �뼶���� ˢ������ִ��ʱ�� */
                /*
                memory_task_plan_list_creat();
                portContext->fujian_ctrl.task_ctrl_cmd.value= 0;
                readport_plc.OnPortReadData = router_reset;
                portContext->OnPortReadData = router_reset;
                portContext->cur_plc_task = PLC_TASK_IDLE;
                portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;
        
                portContext_plc.need_reset_router = 0;
                return TRUE;
                */
                tpos_enterCriticalSection();
                plan_list.plan_info[portContext->fujian_ctrl.cur_exec_plan_id_idx].plan_exec_result = PLAN_EXEC_RESULT_FAIL_WAIT;
                portContext->fujian_ctrl.task_ctrl_cmd.cur_plan_read_sucess = 1;
                tpos_leaveCriticalSection();
            }
        }
        else 
        {
            if (llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.finish == 0)
            {
                return check_gb3762_router_state(portContext);
            }
        }
    }
    #else
    else if((time_elapsed_10ms(portContext->plc_no_interactive_time_10ms) > (PLC_REC_MODE_NO_INTERACTIVE_SECOND_MAX*100)) && (llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.finish == 0))
    {
        return check_gb3762_router_state(portContext);
    }
    #endif
   
   return FALSE;
}
//---------------------------------------------------------------------------
/*+++
  ���ܣ� ������ӱ�־������½ڵ㵽·����
  ������
         BOOLEAN is_reinstall  true ȫ���������  false ֻ���FAST_IDX_ADD_ROUTER_FLAG��־�Ľڵ�
  ���أ�
         BOOLEAN true ����ӽڵ�

         ����û��ʹ��SIGNAL_FAST_IDX�ź�����ͨ�����ڵ����Ч�ԣ�ȫFFʱ���˳�
---*/
BOOLEAN memory_fast_index_add_router_nodes(objReadPortContext * readportcontext)
{
    INT16U node_count;//,file_id;
    INT16U meter_idx,idx,node_count_tmp;
    PLCPortContext *portContext;
    //INT8U protocol,router_tmp[6],router_tmp1[6];
    //FAST_INDEX fast_index;
    //BOOLEAN result;

    #ifdef __DEBUG_RECINFO__
    println_info("**** memory_fast_index_add_router_nodes...");
    #endif

    portContext = (PLCPortContext*)readportcontext;

    //result = FALSE;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
    portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count = 0;
    else
    portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count = 0;

    portContext->params.task_check_doc.abnormal_node_count = 0; //�쳣����������������������綫�����ȫ0��Ų����·�

    for(idx=0;idx<node_count;idx++)
    {
        //��ȡ�ز��ӽڵ��ַ,���
        if (fast_index_list.fast_index[idx].port != READPORT_PLC) continue;

        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);

        if ((meter_idx & FAST_IDX_ADD_ROUTER_FLAG) == 0)  continue;

        if (portContext->router_base_info.router_info3.plc_net_add_null_cjq_2_doc)
        {
            //����Ƿ��ǿղɼ���
            if (isvalid_meter_addr(fast_index_list.fast_index[idx].node,FALSE) == FALSE)
            {
                if ((fast_index_list.fast_index[idx].node[5] & 0xA0) == 0xA0) continue;
            }
        }

        if ((portContext->router_base_info.router_info3.rtu_no_mode) && (fast_index_list.fast_index[idx].router_protocol != 0))  //�ɼ���ģʽʱ���ɼ�����ַ���ز�����Ҫ���·��
        {
            if (meter_idx & FAST_IDX_RTU_FLAG)  //�ɼ����͵�����ʱ�����ղɼ������·��
            {
                //��Ӳɼ�����ַ
                if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
                {
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].node,fast_index_list.fast_index[idx].node,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].protocol = meter_protocol_2_router_protocol(GB645_1997);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[0] = meter_idx;
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[1] = meter_idx >>8;

                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count++;
                }
                else
                {
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].node,fast_index_list.fast_index[idx].node,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].protocol = meter_protocol_2_router_protocol(GB645_1997);
                if ((portContext->router_base_info.router_info1.comm_mode == 2) && (portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM))//���
                {
                    portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].protocol = meter_protocol_2_router_protocol(GB645_2007);
                }
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count++;
                }

            }
            else if ((meter_idx & FAST_IDX_METER_FLAG) == 0)
            {
                //�ز���
                meter_idx &= FAST_IDX_MASK;
                if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;
                if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
                {
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].node,fast_index_list.fast_index[idx].node,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].protocol = fast_index_list.fast_index[idx].router_protocol;
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[0] = meter_idx;
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[1] = meter_idx >>8;

                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count++;
                }
                else
                {
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].node,fast_index_list.fast_index[idx].node,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].protocol = fast_index_list.fast_index[idx].router_protocol;
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count++;
                }

            }
        }
        else if ((portContext->router_base_info.router_info4.dzc_cvt_no_mode) && (fast_index_list.fast_index[idx].router_protocol == 0))
        {
            if (meter_idx & FAST_IDX_RTU_FLAG)  //ת������ˮ���ȱ����ʱ�����ղɼ������·��
            {
                //���ת������ַ
                if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
                {
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].node,fast_index_list.fast_index[idx].node,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].protocol = meter_protocol_2_router_protocol(CJT188);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[0] = meter_idx;
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[1] = meter_idx >>8;

                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count++;
                }
                else
                {
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].node,fast_index_list.fast_index[idx].node,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].protocol = meter_protocol_2_router_protocol(CJT188);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count++;
                }

            }
            else if ((meter_idx & FAST_IDX_DZC_METER_FLAG) == 0)
            {
                //ˮ���ȱ�
                meter_idx &= FAST_IDX_MASK;
                if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;
                if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
                {
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].node,fast_index_list.fast_index[idx].node,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].protocol = fast_index_list.fast_index[idx].router_protocol;
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[0] = meter_idx;
                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[1] = meter_idx >>8;

                portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count++;
                }
                else
                {
                mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].node,fast_index_list.fast_index[idx].node,6);
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].protocol = fast_index_list.fast_index[idx].router_protocol;
                portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count++;
                }

            }
        }
        else
        {
            meter_idx &= FAST_IDX_MASK;
            #ifndef __MEXICO_CIU__ /* ī����CIU ���·�ɵ�����ȥ */
            if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;
            #endif
            if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_EASTSOFT)
            {
                //����·��ȫ0��Ų������·�ɣ����Ժ���
                if(check_is_all_ch(fast_index_list.fast_index[idx].node,6,0x00))
                {
                    portContext->params.task_check_doc.abnormal_node_count ++ ;
                    continue;
                }
            }
            if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
            {
            mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].node,fast_index_list.fast_index[idx].node,6);
            portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].protocol = fast_index_list.fast_index[idx].router_protocol;
            portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[0] = meter_idx;
            portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count].router_seq[1] = meter_idx >>8;
    //        mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.node[portContext->params.task_check_doc.add_or_del_node.add_node.node_count].router_seq,fast_index_list.fast_index[idx].seq_spec,2);
            portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count++;
            }
            else
            {
            mem_cpy(portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].node,fast_index_list.fast_index[idx].node,6);
            portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count].protocol = fast_index_list.fast_index[idx].router_protocol;
            portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count++;
            }

        }
        if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
        {
            if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)
            {
                if (portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count >= ROUTER_OPT_NODE_COUNT) break;
            }
            else
            {
                if (portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count >= 1) break;
            }
        }
        else
        {
            if (portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count >= ROUTER_OPT_NODE_COUNT) break;
        }
    }
    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)//��ʵû��ҪcountҲ�ֿ�������ֵ��һ����
    node_count_tmp = portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node_count;
    else
    node_count_tmp = portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count;
    if(node_count_tmp)
    {
        switch(portContext->cur_plc_task)
        {
        case PLC_TASK_CHECK_DOC:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_CHECK_DOC_CHECK_FAST_INDEX_1: //��Ѷ��09·����ӵ���ʱ����Ҫ����һ�£��·������AFN��ͬ
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_11_F1;
                
                portContext->cur_plc_task_step = PLC_CHECK_DOC_ADD_NODE_11F1_1;
                break;
            case PLC_CHECK_DOC_CHECK_FAST_INDEX_2: //�˴��Ƿ�ҲҪ����
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_11_F1;

                portContext->cur_plc_task_step = PLC_CHECK_DOC_ADD_NODE_11F1_2;
                break;
            }
            break;
        }
    }
    else //��ӽڵ����
    {
        //��ѯ·�ɴӽڵ�����
        switch(portContext->cur_plc_task)
        {
        case PLC_TASK_CHECK_DOC:
            switch(portContext->cur_plc_task_step)
            {
            case PLC_CHECK_DOC_CHECK_FAST_INDEX_1:    //��ӽڵ���ɣ�ת����һ��
                if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_XC0010)
                {
                  start_read_meter(portContext); //����·�ɲ�֧�ֲ�ѯ�ӽڵ���������Ϣ
                  break;
                }
                else if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_ZT)
                {
                    readport_plc.OnPortReadData = router_check_urgent_timeout;
                    portContext->OnPortReadData = router_send_friend_afn_05_F4;
                    portContext->cur_plc_task_step = WIRELESS_CHANEL_SET_05F4;

                    break;
                }
                else
                {
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_send_afn_10_F1;
                portContext->cur_plc_task_step = PLC_CHECK_DOC_QUERY_NODE_COUNT_10F1_2;
                break;
                }
            case PLC_CHECK_DOC_CHECK_FAST_INDEX_2:    //��װ�ڵ���ɣ�ת����һ��
                //todo������·��״̬����������ظ�����
                start_read_meter(portContext);
                break;
            }
            break;
        }
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);

    return 0;
}

BOOLEAN memory_fast_index_update_one_add_flag(INT8U* node,BOOLEAN set_clr)
{
    INT16U pos;
    INT16U meter_idx;//,idx,rec_no;
    INT8U router_protocol;
    //FAST_INDEX fast_index;
    BOOLEAN result;

    result = FALSE;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    if(memory_fast_index_find_node_no(COMMPORT_PLC,node,&meter_idx,&pos,&router_protocol,NULL))
    {
        if (set_clr)
        {
            if ((meter_idx & FAST_IDX_ADD_ROUTER_FLAG) == 0)
            {
                meter_idx |= FAST_IDX_ADD_ROUTER_FLAG;
                fast_index_list.fast_index[pos].seq_spec[0] = meter_idx & 0xFF;
                fast_index_list.fast_index[pos].seq_spec[1] = meter_idx >> 8;

                #ifdef __DEBUG_RECINFO__
                println_info("****memory_fast_index_set_add_flag...");
                #endif
            }
        }
        else
        {
            if(meter_idx & FAST_IDX_ADD_ROUTER_FLAG)
            {
                meter_idx &= ~FAST_IDX_ADD_ROUTER_FLAG;
                fast_index_list.fast_index[pos].seq_spec[0] = meter_idx & 0xFF;
                fast_index_list.fast_index[pos].seq_spec[1] = meter_idx >> 8;

                #ifdef __DEBUG_RECINFO__
                println_info("****memory_fast_index_clear_add_flag...");
                #endif
            }
        }
        result = TRUE;
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    
    return result;
}

BOOLEAN memory_fast_index_set_one_add_flag(INT8U* node)
{
    memory_fast_index_update_one_add_flag(node,TRUE);
    return TRUE;
}

BOOLEAN memory_fast_index_clear_one_add_flag(INT8U* node)
{
    memory_fast_index_update_one_add_flag(node,FALSE);
    return TRUE;
}

void memory_fast_index_clear_add_flag(objReadPortContext * readportcontext)
{
    INT16U pos = 0;
    INT16U meter_idx = 0;
	INT16U idx;//,rec_no;
    PLCPortContext *portContext;
    INT8U router_protocol,tmp;
    //FAST_INDEX fast_index;
    //BOOLEAN result;

    #ifdef __DEBUG_RECINFO__
    println_info("****memory_fast_index_clear_add_flag...");
    #endif

    portContext = (PLCPortContext*)readportcontext;

    //result = FALSE;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    for(idx=0;idx<portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node_count;idx++)//�˴����÷�13��09��
    {
    if(portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB3762)
            tmp = memory_fast_index_find_node_no(COMMPORT_PLC,portContext->params.task_check_doc.add_or_del_node.add_node.router_09.node[idx].node,&meter_idx,&pos,&router_protocol,NULL);
    else
            tmp = memory_fast_index_find_node_no(COMMPORT_PLC,portContext->params.task_check_doc.add_or_del_node.add_node.router_13.node[idx].node,&meter_idx,&pos,&router_protocol,NULL);
        if(tmp)
        {
            if(meter_idx & FAST_IDX_ADD_ROUTER_FLAG)
            {
                meter_idx &= ~FAST_IDX_ADD_ROUTER_FLAG;
                fast_index_list.fast_index[pos].seq_spec[0] = meter_idx & 0xFF;
                fast_index_list.fast_index[pos].seq_spec[1] = meter_idx >> 8;
            }
        }
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
}

/*+++
  ���ܣ��ڿ��������в��ҵ���ţ�ֻƥ���ŵĺ󼸸��ֽ�
  ������
       INT8U* meter_no        //�����
       INT8U byte_num         //ƥ���ŵĺ󼸸��ֽ� >0  <=6
       INT16U* pos            //�ҵ�ʱ���Ǹñ�ŵĴ洢λ�ã�δ�ҵ�ʱ���Ǹñ��Ҫ�����λ��
       INT16U* rec_no         //��Ӧ�ĵ�����
  ����:
       ��Ч�ĵ�����

---*/
INT16U find_meter_no_from_fast_index_fuzzy(INT8U* meter_no,INT8U byte_num)
{
//    INT16U idx,meter_idx,rec_no,meter_count,file_id;
//    INT16S result;
//    FAST_INDEX fast_index_list[10];
//    BOOLEAN find_flag;
//
//    find_flag = FALSE;
//    meter_idx = 0;
//
//    if(byte_num > 6) byte_num = 6;
//    if(byte_num == 0) byte_num = 1;
//
//    tpos_mutexPend(&SIGNAL_FAST_IDX);
//
//    meter_count = get_fast_index_count_and_file_offset(&file_id);
//
//    for(idx=0;idx<meter_count;idx++)
//    {
//        //ÿ��
//        if ((idx % 10) == 0)
//        {
//            //��ȡ��ǰλ�õĵ���ַ
//            fread_array(FILEID_METER_NO_FAST_INDEX + file_id,idx*sizeof(FAST_INDEX),fast_index_list[0].value,sizeof(FAST_INDEX)*10);
//        }
//
//        if (compare_string(meter_no,fast_index_list[idx % 10].node,byte_num) == 0)
//        {
//            rec_no = bin2_int16u(fast_index_list[idx%10].seq_spec);
//            meter_idx = rec_no & FAST_IDX_MASK;
//            break;
//        }
//    }
//
//    tpos_mutexFree(&SIGNAL_FAST_IDX);
//    return meter_idx;
    //coverity�澯������0  ����ʹ�ô˺����Ļ�����Ҫע�⴦��
    return 0;
}

INT8U meter_protocol_2_router_protocol(INT8U meter_protocol)
{
    switch(meter_protocol)
    {
    case GB645_1997:
    case TOPS_ROADLAMP:
    case GB645_1997_JINANGSU_2FL:
    case GB645_1997_JINANGSU_4FL:
    case GB645_1997_STAR_40:   //__STAR_EXT_METER_PROTOCOL_40__
    case GUANGXI_V30:
        return 1; //645_1997
    case GB645_2007:
    //case OPEN_PACKAGE_MONITOR://���ı�Э��ų�ͻ���ݹر�
        return 2; //645_2007
    case GB_OOP:
        if(portContext_plc.router_base_info.router_info1.comm_mode == 2) return 3;  //����Ķ�֧��oopЭ��3��խ����ʱ����
        else return 0;    
    default:
        return 0; //͸������
    }
}

void set_port_doc_chg_flag(INT8U port)
{
    INT8U port_idx;
    if(port != COMMPORT_PLC)
    {
        port_idx = get_readport_idx(port);
        if(port_idx == 0xFF) return;  
    }
    switch(port)
    {
    case COMMPORT_PLC:
        portContext_plc.doc_chg_time = os_get_systick_10ms();
        portContext_plc.read_status.doc_chg = 1;
        break;
    case COMMPORT_485_REC:
        portContext_rs485[port_idx].read_status.doc_chg = 1;
        break;
    case COMMPORT_485_CAS:
        portContext_rs485[port_idx].read_status.doc_chg = 1;
        break;
    case COMMPORT_485_CY:
        portContext_rs485[port_idx].read_status.doc_chg = 1;
        break;
    case COMMPORT_485_MIN:
        portContext_rs485[port_idx].read_status.doc_chg = 1;
        break;
    }
}

BOOLEAN insert_fast_index(INT8U port,INT8U* meter_no,INT16U meter_idx,INT8U meter_protocol,INT8U *rtu_no,INT8U user_class)   //__FAST_INDEX_RTU_NO__
{
    INT16U meter_idx_tmp1,save_pos;
    INT8U router_protocol,uclass;
    BOOLEAN result;

    result = FALSE;

    if (TRUE == memory_fast_index_find_node_no(port,meter_no,&meter_idx_tmp1,&save_pos,&router_protocol,&uclass)) //�ص��������
    {
        if (meter_idx == FAST_IDX_RTU_FLAG)  //��Ӳɼ�����
        {
            if ((meter_idx_tmp1 & FAST_IDX_RTU_FLAG) == 0)   //���Ͳɼ�������壬��ԼҪ���յ��Ĺ�Լ���
            {
                meter_idx_tmp1 |= FAST_IDX_RTU_FLAG;
                meter_idx_tmp1 |= FAST_IDX_ADD_ROUTER_FLAG;
                result = memory_fast_index_update_info(port,meter_no,meter_idx_tmp1,router_protocol,NULL,MODIFY,user_class);
                set_port_doc_chg_flag(port);
            }
        }
        else //��ӵ��
        {
            //���������ű仯���ǲɼ����µ���ʶ�仯���ߵ��Ĺ�Լ�仯��Ҫ�޸�
            #ifndef __FAST_INDEX_RTU_NO__
            if (((meter_idx_tmp1 & (FAST_IDX_METER_FLAG | meter_idx)) != meter_idx) || (meter_protocol_2_router_protocol(meter_protocol) != router_protocol)
                ||(uclass != user_class))
            #endif
            {
                if(check_is_sqr_protocol(meter_protocol))
                {
                    meter_idx_tmp1 &= ~(FAST_IDX_DZC_METER_FLAG|FAST_IDX_MASK);
                }
                else
                {
                meter_idx_tmp1 &= ~(FAST_IDX_METER_FLAG|FAST_IDX_MASK);
                }
                meter_idx_tmp1 |= meter_idx;
                meter_idx_tmp1 |= FAST_IDX_ADD_ROUTER_FLAG;
                result = memory_fast_index_update_info(port,meter_no,meter_idx_tmp1,meter_protocol_2_router_protocol(meter_protocol),rtu_no,MODIFY,user_class);
                set_port_doc_chg_flag(port);
            }
        }
    }
    else //û���ڿ����������ҵ�
    {
        if (meter_idx == FAST_IDX_RTU_FLAG)  //�ɼ���  ��Լ����97���
        {
            meter_idx_tmp1 = FAST_IDX_RTU_FLAG;
            meter_idx_tmp1 |= FAST_IDX_ADD_ROUTER_FLAG;
            if(check_is_sqr_protocol(meter_protocol)) //���������ǵ��ɼ�����ַ����ˮ���ȱ�ת������ַ
            {
                result = memory_fast_index_update_info(port,meter_no,meter_idx_tmp1,meter_protocol_2_router_protocol(meter_protocol),NULL,ADD,user_class);
            }
            else
            {
                #ifdef __MEXICO_CIU__
                /* Э�黹�� ���ݵ��Э���� ������97Э�� */
                result = memory_fast_index_update_info(port,meter_no,meter_idx_tmp1,meter_protocol_2_router_protocol(meter_protocol),rtu_no,ADD,user_class);
                #else
                result = memory_fast_index_update_info(port,meter_no,meter_idx_tmp1,meter_protocol_2_router_protocol(GB645_1997),NULL,ADD,user_class);
                #endif
            }
            set_port_doc_chg_flag(port);
        }
        else  //���
        {
            meter_idx_tmp1 = meter_idx;
            meter_idx_tmp1 |= FAST_IDX_ADD_ROUTER_FLAG;
            result = memory_fast_index_update_info(port,meter_no,meter_idx_tmp1,meter_protocol_2_router_protocol(meter_protocol),rtu_no,ADD,user_class);
            set_port_doc_chg_flag(port);
        }
    }

    return result;
}

BOOLEAN delete_fast_index(INT8U port,INT8U* meter_no,INT16U meter_idx)
{
    INT16U meter_idx_tmp1,save_pos;
    INT8U router_protocol,u_class,tmp_port;
    INT8U rtu_no[6];
    BOOLEAN result;

    result = FALSE;
    tmp_port = port;
    #ifdef __RS485_PORT_CHECK__
    if((port == COMMPORT_485_REC) || (port == COMMPORT_485_CAS))
    {
        memory_fast_index_find_node_no_no_port(meter_no,&meter_idx_tmp1,&save_pos,&router_protocol,&tmp_port,&u_class);
    }
    #endif
    if (TRUE == memory_fast_index_find_node_no(tmp_port,meter_no,&meter_idx_tmp1,&save_pos,&router_protocol,&u_class))
    {
        if (meter_idx == FAST_IDX_RTU_FLAG) //�ɼ���
        {
            if (meter_idx_tmp1 & FAST_IDX_RTU_FLAG)
            {
                if ((meter_idx_tmp1 & FAST_IDX_MASK) > 0)    //�ɼ����͵����壬ɾ���ɼ������������
                {
                    meter_idx_tmp1 &= ~FAST_IDX_RTU_FLAG;
                    meter_idx_tmp1 |= FAST_IDX_ADD_ROUTER_FLAG;
                    result = memory_fast_index_update_info(tmp_port,meter_no,meter_idx_tmp1,router_protocol,NULL,MODIFY,u_class);
                }
                else //���ֲɼ���
                {
                    result = memory_fast_index_update_info(tmp_port,meter_no,meter_idx_tmp1,0,NULL,DELETE,u_class);
                }
                set_port_doc_chg_flag(tmp_port);
            }
        }
        else //���
        {
            if (meter_idx_tmp1 & FAST_IDX_RTU_FLAG)  //�ɼ����͵�����
            {
                meter_idx_tmp1 &= ~(FAST_IDX_METER_FLAG|FAST_IDX_DZC_METER_FLAG|FAST_IDX_MASK);
                meter_idx_tmp1 |= FAST_IDX_ADD_ROUTER_FLAG;
                mem_set(rtu_no,6,0x00);
//                result = memory_fast_index_update_info(port,meter_no,meter_idx_tmp1,meter_protocol_2_router_protocol(GB645_1997),rtu_no,MODIFY,u_class);
                result = memory_fast_index_update_info(tmp_port,meter_no,meter_idx_tmp1,router_protocol,rtu_no,MODIFY,u_class);
            }
            else //���ֵ��
            {
                result = memory_fast_index_update_info(tmp_port,meter_no,meter_idx_tmp1,0,NULL,DELETE,u_class);
            }
            set_port_doc_chg_flag(tmp_port);
        }
    }

    return result;
}

/**********************************************************
 * ���ܣ�
 *     ���������в��Ҷ�Ӧ�ɼ����µĵ�һ��������ĵ�����
 * ������
 *     INT8U port  �˿�
 *     INT8U *rtu_no  �ɼ�����
 *     INT8U *pos  ��Ӧ�����������
 * ���أ�
 *     INT16U  �������
 * ������
 *     1����ʹ�òɼ������ڿ��������н��в��ң��޷�ʹ�ö��ַ����ֽ׶�ֻ��ʹ�ñ���
 * ���ߣ�
 *     zyl by 2017.12.05
 **********************************************************/
INT16U memory_fast_index_find_first_meter_idx_in_rtu(INT8U port,INT8U *rtu_no,INT16U *pos)
{
    INT16U meter_count,idx,meter_idx;
    
    meter_idx = 0xFFFF;
    
    tpos_mutexPend(&SIGNAL_FAST_IDX);
    
    meter_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
    
    for(idx = 0;idx < meter_count;idx++)
    {
        if(fast_index_list.fast_index[idx].port != port) continue;
        if(compare_string(rtu_no,fast_index_list.fast_index[idx].rtu_no,6) != 0) continue;
        //������
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);
        *pos = idx;
        break;
    }
    
    tpos_mutexFree(&SIGNAL_FAST_IDX);
    
    if(meter_idx != 0xFFFF)
    {
        meter_idx &= FAST_IDX_MASK;
    }
    
    return meter_idx;
}
/*+++
  ���ܣ�ͳ�ƿ��������е��ز��ڵ�����
  ������
         INT32 addr         ���������Ŀ�ʼ��ַ
         INT16U max_count   �ڵ����
  ���أ�
         INT16U ��Ч�ؽڵ�����
  ������
       1) ���ݿ�������ͳ��
       2) �㽭�пյĲɼ���ռ�ò�����,����ͳ��������
       3) ˳����ڵ����Ч�ԣ���ͳ�Ƹ���
---*/
INT16U memory_fast_index_stat_port_node_count(INT8U port,INT8U rtu_no_mode,INT8U dzc_cvt_no_mode)
{
    INT16U node_count,meter_count,idx,meter_idx;//file_id,

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    meter_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    node_count = 0;
    for(idx=0;idx<meter_count;idx++)
    {
        //������
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);

        if (meter_idx == 0)  continue;

        if (fast_index_list.fast_index[idx].port != port) continue;

        if (fast_index_list.fast_index[idx].port > port) break;

        if (isvalid_meter_addr(fast_index_list.fast_index[idx].node,FALSE) == FALSE) continue;

        if (meter_idx & FAST_IDX_RTU_FLAG) //�ɼ���
        {
            if ((meter_idx & FAST_IDX_MASK) == 0) //���ֲɼ���
            {
                #ifdef __MEXICO_CIU__
                node_count++; /*ī����CIU Ҳ�ۼ����� ��Ҫ��ӵ�·�ɵ�����ȥ */
                #else
                if ((rtu_no_mode) && (fast_index_list.fast_index[idx].router_protocol == 1))//ʹ�òɼ�����ַ����
                {
                    node_count++;
                }
                else if ((dzc_cvt_no_mode) && (fast_index_list.fast_index[idx].router_protocol == 0))//ת������·����Э���Ϊ0
                {
                    node_count++;
                }
                #endif
            }
            else if ((meter_idx & FAST_IDX_MASK) <= MAX_METER_COUNT) //�ɼ���������
            {
                node_count++;
            }
        }
        else if (((meter_idx & FAST_IDX_MASK) <= MAX_METER_COUNT) && ((meter_idx & FAST_IDX_MASK) > 0)) //��ͨ��
        {
            if (meter_idx & FAST_IDX_METER_FLAG)  //�ɼ����µĵ��
            {
                if (!rtu_no_mode)  //��ʹ�òɼ�����ַ����
                {
                    node_count++;
                }
            }
            else if (meter_idx & FAST_IDX_DZC_METER_FLAG) //ת�����µ�ˮ���ȱ�
            {
                if(!dzc_cvt_no_mode)
                {
                    node_count++;
                }
            }
            else  //�����ز���
            {
                node_count++;
            }
        }
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return node_count;
}

/*+++
  ���ܣ�ͳ�ƿ��������ж�Ӧ��С��ŵĵ������
  ������
         INT8U port               �˿ں�
         INT8U user_class         ��С���
  ���أ�
         INT16U ��Ч�ؽڵ�����
  ������
       1) ���ݿ�������ͳ��
       2) ��Ҫ�������0CF10�Ķ�ȡ
       3) ˳����ڵ����Ч�ԣ���ͳ�Ƹ���
---*/
INT16U memory_fast_index_stat_user_class_count(INT8U port,INT8U user_class)
{
    INT16U node_count,meter_count,idx,meter_idx;//file_id,

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    meter_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    node_count = 0;
    for(idx=0;idx<meter_count;idx++)
    {
        //������
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);

        if (meter_idx == 0)  continue;

        if (fast_index_list.fast_index[idx].port != port) continue;

        if (fast_index_list.fast_index[idx].user_class != user_class) continue;

        if (isvalid_meter_addr(fast_index_list.fast_index[idx].node,FALSE) == FALSE) continue;

        if (((meter_idx & FAST_IDX_MASK) <= MAX_METER_COUNT) && ((meter_idx & FAST_IDX_MASK) > 0)) //���
        {
            node_count++;
        }
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return node_count;
}

/*+++
  ���ܣ����ݶ˿�ͳ�ƿ��������е��Ƿ���
  ������

  ���أ�

  ������

---*/
BOOLEAN memory_fast_index_has_node(INT8U port)
{
    INT16U node_count,idx;//file_id, ,meter_idx
    //INT8U read_count;
    BOOLEAN has_node;

    has_node = FALSE;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port) break;
        if (port != fast_index_list.fast_index[idx].port) continue;
        has_node = TRUE;
        break;
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return has_node;
}

/*+++
  ���ܣ����ݶ˿�ͳ�ƿ��������е��Ƿ���
  ������

  ���أ�

  ������

---*/
void memory_fast_index_set_reinstall(INT8U port)
{
    INT16U node_count,idx,meter_idx;//file_id,
    //INT8U read_count;
    //BOOLEAN has_node;

    //has_node = FALSE;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port) break;
        if (port != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);
        meter_idx |= FAST_IDX_ADD_ROUTER_FLAG;
        int16u2_bin(meter_idx,fast_index_list.fast_index[idx].seq_spec);
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
}


/*+++
  ���ܣ�ͨ�������������þ���
  ������

  ���أ�

  ������ÿ�ζ�ȡ�˸�

---*/
void set_readport_specific_meter_class_read_meter_flag_from_fast_index(INT8U* read_meter_flag,INT8U port,METER_CLASS meter_class)
{
    INT16U node_count,idx,meter_idx;
    
    //���ʱ�Ӷ�ʧ�󣬲��ܳ���
    if(get_system_flag(SYS_CLOCK_LOST,SYS_FLAG_BASE))
    {
        return;
    }    
    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port) break;
        if (port != fast_index_list.fast_index[idx].port) continue;
        // ���Ҵ�����Ƿ���������  
        if(meter_class.user_class != ((fast_index_list.fast_index[idx].user_class & 0xF0)>>4) ) continue;
        
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;
        read_meter_flag[meter_idx/8] |= 0x01<<(meter_idx%8);
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
}

/*+++
  ���ܣ�ͨ�������������ó�������
  ������

  ���أ�

  ������ÿ�ζ�ȡ�˸�

---*/
void set_readport_read_meter_flag_from_fast_index(INT8U* read_meter_flag,INT8U port)
{
    INT16U node_count,idx,meter_idx;//file_id,
    //FAST_INDEX fast_index[8];
    #ifdef __485_METER_EVENT_REPORT__
    METER_DOCUMENT meter_doc;
	INT8U ctrl;
	#endif
    //INT8U read_count;	
	//INT8U	plan_id;

    //���ʱ�Ӷ�ʧ�󣬲��ܳ���
     if(get_system_flag(SYS_CLOCK_LOST,SYS_FLAG_BASE))
     {
         return;
     }    
    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port) break;
        if (port != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;
        
        read_meter_flag[meter_idx/8] |= 0x01<<(meter_idx%8);
        
		#ifdef __485_METER_EVENT_REPORT__
		if( (port == READPORT_RS485_1) && (read_meter_flag == read_meter_flag_cur_data) )//��ʱֻ֧��485_1
		{
			//δ������������
            //DelayNmSec(3000);
            #ifndef __PROVICE_HUNAN__
			INT8U	plan_id = 0;
			fread_array( meter_idx,PIM_METER_F105,&plan_id,1);
			if( (plan_id < 1) || (plan_id > MAX_METER_EVENT_PLAN_COUNT) )
			#endif	
		    {    	
		    	//if( portContext_rs485_1.read_status.is_in_read_cycle == 1)
                fread_array(meter_idx,PIM_METER_DOC, (INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
				if(meter_doc.protocol == GB645_2007)//ֻ��07��֧�֣�������֧��
		    	{
					fread_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl,1);
					if( (ctrl == 0x00) || (ctrl == 0xFF) )
					{
			 			ctrl = 0xFE;
						fwrite_array(meter_idx,PIM_METER_REPORT_EVENT_STATE,&ctrl,1);
					}
		    	}
			}
		}
		#endif
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
}

/*+++
  ���ܣ�ͨ���������������������
  ������

  ���أ�

  ������ÿ�ζ�ȡ�˸�

---*/
void clr_readport_read_meter_flag_from_fast_index(INT8U* read_meter_flag,INT8U port)
{
    INT16U node_count,idx,meter_idx;//file_id,
    //FAST_INDEX fast_index[8];
    //INT8U read_count;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port) break;
        if (port != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

        read_meter_flag[meter_idx/8] &= ~(0x01<<(meter_idx%8));
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
}
/*+++
  ���ܣ�ͨ������������ѯ��������
  ������

  ���أ�

  ������ÿ�ζ�ȡ�˸�

---*/
INT8U check_readport_read_meter_flag_from_fast_index(INT8U* read_meter_flag,INT8U port)
{
    INT16U node_count,idx,meter_idx;//file_id,
    //FAST_INDEX fast_index[8];
    INT8U flag;//read_count,

    flag = 0;

    if(FALSE == tpos_mutexRequest(&SIGNAL_FAST_IDX))  return FALSE;
 //   tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port)
        {
            flag = 0;
            break;
        }
        if (port != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

        //read_meter_flag[meter_idx/8] |= 0x01<<(meter_idx%8);
        if((read_meter_flag[meter_idx/8]) & (0x01<<(meter_idx%8)))
        {
            flag = 1;
            break;
        }
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);

    return flag;
}
#ifdef __RS485_PORT_CHECK__
INT16U get_readport_meter_count_from_fast_index_with_doc_port(INT8U port)
{
    INT16U node_count,idx,meter_idx;
    INT16U read_count;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    read_count = 0;
    for(idx=0;idx<node_count;idx++)
    {
        if (port != fast_index_list.fast_index[idx].doc_port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

        read_count ++;
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return read_count;
}
INT16U stat_readport_day_hold_complete_meter_count_from_fast_index_with_doc_port(INT8U port)
{
    INT16U node_count,idx,meter_idx;//file_id,
    INT16U read_count;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    read_count = 0;
    for(idx=0;idx<node_count;idx++)
    {
        if (port != fast_index_list.fast_index[idx].doc_port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

        if (get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE)
        {
            read_count ++;
        }
        if(port != COMMPORT_PLC)
        {
            if(get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE)
            {
                //485ÿ������ͳ��һ��F170��
                update_meter_recstate(meter_idx,port,0x00,0x00,0x00,0x00,true);
            }
        }
        if((get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) != FALSE) &&(datetime[HOUR]==23)&&(datetime[MINUTE]>55))
        {
            //��23��57�Ժ�ÿ����ͳ��һ�Σ�ͳ��3�Σ�
            update_meter_recstate(meter_idx,port,0x00,0x00,0x00,0x00,FALSE);
        }
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return read_count;
}
#endif

/*+++
  ���ܣ�ͨ�������������ó�������
  ������

  ���أ�

  ������ÿ�ζ�ȡ�˸�

---*/
INT16U get_readport_meter_count_from_fast_index(INT8U port)
{
    INT16U node_count,idx,meter_idx;
    INT16U read_count;
    //FAST_INDEX fast_index[8];

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    read_count = 0;
    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port) break;
        if (port != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

        read_count ++;
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return read_count;
}

/*+++
  ���ܣ�ͨ�������������ó�������
  ������

  ���أ�

  ������ÿ�ζ�ȡ�˸�

---*/
INT16U stat_readport_day_hold_complete_meter_count_from_fast_index(INT8U port)
{
    INT16U node_count,idx,meter_idx;//file_id,
    INT16U read_count;
    //FAST_INDEX fast_index[8];
    //int test;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    read_count = 0;
    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port) break;
        if (port != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

        if (get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE)  /*ֻͳ���ն���ɹ���*/
        {
            read_count ++;
        }

        if(port != COMMPORT_PLC)
        {
            if(get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) == FALSE)
            {
              //485ÿ������ͳ��һ��F170��
              update_meter_recstate(meter_idx,port,0x00,0x00,0x00,0x00,true);
            }
        }
        if((get_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx) != FALSE) &&(datetime[HOUR]==23)&&(datetime[MINUTE]>55))
        {
           //��23��57�Ժ�ÿ����ͳ��һ�Σ�ͳ��3�Σ�
           update_meter_recstate(meter_idx,port,0x00,0x00,0x00,0x00,FALSE);
        }
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return read_count;
}

/*+++
 *  ���ܣ� ׼���������ڵĲ���
 *
 *  ������
 *
---*/
void prepare_read_plc_cycle_param(SEG_PARAM* seg_param,READ_METER_RUN_PARAM* run_param)
{
    SET_F33 f33;
    INT8U BS8;
    //������µ������
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** prepare_read_plc_cycle_param ***");
    debug_println_ext(info);
    #endif

    fread_ertu_params(EEADDR_SET_F33 + (COMMPORT_PLC_REC-1) * sizeof(tagSET_F33),f33.value,sizeof(SET_F33));

    run_param->read_cycle = f33.cycle;
    mem_cpy(run_param->rec_days,f33.rec_days,4);
        #if  defined(__DL698_41_0928__)
        f33.cast_time[2] = f33.rec_timeseg[10];
        f33.cast_time[1] = f33.rec_timeseg[9];
        f33.cast_time[0] = f33.rec_timeseg[8];
     #endif
    run_param->adjtime_day = BCD2byte(f33.cast_time[2]);
    run_param->adjtime_hour = BCD2byte(f33.cast_time[1]);
    run_param->adjtime_minute = BCD2byte(f33.cast_time[0]);
    

    fread_array(FILEID_RUN_PARAM,FLADDR_METER_SET_CLOCK_FALG,&BS8,1);
    if( BS8 != 0xAA)
    run_param->adjtime_flag = f33.run_ctrl[0] & 0x08;
    else
    run_param->adjtime_flag = 0;  //�¹淶��չ��F30���ڹ㲥������ر�

    run_param->ctrl_char.value = f33.run_ctrl[0];

    seg_param->seg_count = f33.seg_count;
    mem_cpy(seg_param->rec_timeseg,f33.rec_timeseg,96);
    //����Ƿ����ĳ������ʱ�Σ����24������ʱ��
    if ((seg_param->seg_count > 24) || (seg_param->seg_count == 0))
    {
        //ȱʡ���ã�00:05-23:50
        seg_param->seg_count = 1;                                                                                          
        seg_param->rec_timeseg[0][0] = 0x05;
        seg_param->rec_timeseg[0][1] = 0x00;
        seg_param->rec_timeseg[0][2] = 0x50;
        seg_param->rec_timeseg[0][3] = 0x23;
    }
    #ifdef __DL698_41_0928__
        run_param->read_cycle = 0;
        run_param->ctrl_char.value = 0;
             //ȱʡ���ã�00:05-23:50
        seg_param->seg_count = 1;
        seg_param->rec_timeseg[0][0] = 0x15;
        seg_param->rec_timeseg[0][1] = 0x00;
        seg_param->rec_timeseg[0][2] = 0x50;
        seg_param->rec_timeseg[0][3] = 0x23;

    #endif
}

BOOLEAN check_is_in_autorec_time(SEG_PARAM *seg_param,REC_END_TIME* rec_end_time)
{
    INT16U cur_time,from_time,end_time,max_end_time;
    INT16U in_end_time;//in_from_time,
    INT8U idx;
    BOOLEAN flag;
	#if defined(__PROVICE_SHANGHAI__)
    INT8U time_minute;
	#endif

    flag = FALSE;
    //in_from_time = 0;
    in_end_time = 0;
    max_end_time = 0;

    //����Ƿ��Ѿ���23��59����,
    //����ִ�г����ԭ���Ǽ���������23��59:58��λ!
    if ((datetime[HOUR]==23) && (datetime[MINUTE]==59) )
    {
        return flag;
    }
    
    rec_end_time->is_lase_time_seg = 1;

    for(idx = 0; idx < seg_param->seg_count;idx++)
    {
        #if defined(__PROVICE_SHANGHAI__)
        if(seg_param->rec_timeseg[idx][1] == 0x00)
        {
            fread_ertu_params(EEADDR_SET_F242,&time_minute,1);
            if(time_minute == 0 || time_minute > 60) time_minute = 5;
            if(BCD2byte(seg_param->rec_timeseg[idx][0]) < time_minute)
            {
                if(time_minute == 60)
                {
                    seg_param->rec_timeseg[idx][1] = 1;
                }
                else
                {
                    seg_param->rec_timeseg[idx][0] = byte2BCD(time_minute);
                }
            }
        }
        #endif

        cur_time = 0x100 * datetime[HOUR] + datetime[MINUTE];
        from_time = 0x100 * BCD2byte(seg_param->rec_timeseg[idx][1]) + BCD2byte(seg_param->rec_timeseg[idx][0]);
        end_time = 0x100 *  BCD2byte(seg_param->rec_timeseg[idx][3]) + BCD2byte(seg_param->rec_timeseg[idx][2]);

        if((cur_time >= from_time) && (cur_time < end_time) && (flag == FALSE))
        {
            //
            rec_end_time->year   = datetime[YEAR];
            rec_end_time->month  = datetime[MONTH];
            rec_end_time->day    = datetime[DAY];
            rec_end_time->hour   = end_time>>8;
            rec_end_time->minute = end_time;

            flag = TRUE;

            //in_from_time = from_time;
            in_end_time = end_time;
        }
        else
        {
            max_end_time = (end_time > max_end_time) ? end_time : max_end_time;

            if(flag)
            {
                if(max_end_time > in_end_time) rec_end_time->is_lase_time_seg = 0;
            }
        }
    }

    return flag;
}

BOOLEAN check_recording_time_begin(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    SEG_PARAM seg_param;

    portContext = (PLCPortContext*)readportcontext;

    prepare_read_plc_cycle_param(&seg_param,&(portContext->run_param));

    return check_is_in_autorec_time(&seg_param,&(portContext->seg_end_time));
}

/*+++
  ���ܣ�����Ƿ��˱��������ڵĽ���ʱ��
  ������
         ��
  ����:
        TRUE   ����,����
        FALSE  ,û�е�,����;
  ������

     1��23��59��00���Ҳ�����������
---*/
BOOLEAN check_recording_time_end(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    //INT16U     cur_time;
    //INT16U     hour;
    //INT8U      rec_date[3];
    BOOLEAN    result;

    portContext = (PLCPortContext*)readportcontext;
    result = FALSE;

    //������Ƿ���ͬ����ͬ��������,�� �� ��һ����Ҳ������������ϳ����˲�������ʱ�����
    if ( (portContext->seg_end_time.day != datetime[DAY] ) || (portContext->seg_end_time.month != datetime[MONTH] )
		|| (portContext->seg_end_time.year != datetime[YEAR] ) )
    {
        //�������ڽ���,�Զ�ȡ����ͣ����
        //GPLC.status.timeout = 1;
        //GPLC.status.pause = 0;
        portContext->seg_end_time.is_lase_time_seg = 1;
        result = TRUE;
    }

    //����Ƿ��˱��յ�23��59��00֮��
    if ((datetime[HOUR]== 23) && (datetime[MINUTE]==59))
    {
        //GPLC.status.timeout = 1;
        //GPLC.status.pause = 0;
        portContext->seg_end_time.is_lase_time_seg = 1;
        result = TRUE;
    }

    if (datetime[HOUR] >= portContext->seg_end_time.hour)
    {
        if (datetime[MINUTE] >= portContext->seg_end_time.minute)
        {
            //GPLC.status.timeout = 1;
            //GPLC.status.pause = 0;
            result = TRUE;
        }
    }

    if(TRUE == result)
    {
        #ifdef __DEBUG_RECINFO__
        println_info("****�˳���������...");
        #endif

        //���ó���ʱ�ν�����.
        //write_fmByte(EEADDR_PLC_REC_TIMESEG,0xBB);
    }

    return result;
}

 void set_llvc_rec_state_in_seg(INT8U port)
{
    //LLVC_REC_STATE rec_state;
    //INT8U i;

    if(port > COUNT_OF_READPORT) return;
     
    fread_array(FILEID_RUN_DATA,FLADDR_LLVC_READ_STATE+(port-1)*sizeof(LLVC_REC_STATE),
            llvc_rec_state[port-1].value,sizeof(LLVC_REC_STATE));
    if(port == COMMPORT_PLC_REC)
    {
        llvc_rec_state[port-1].port = COMMPORT_PLC;
        int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_PLC),llvc_rec_state[COMMPORT_PLC_REC-1].total_count);
        if(bin2_int16u(llvc_rec_state[COMMPORT_PLC_REC-1].total_count) == 0)//���⵱�ز���Ϊ��ʱ�����Լ����Դ��ڳ���ʼʱ���Լ�����ʱ��
        {
            mem_set(llvc_rec_state[port-1].value,sizeof(LLVC_REC_STATE),0);/*�ز���ͳ����Ϣ�����*/
            llvc_rec_state[port-1].port = 0x1F;
            llvc_rec_state[port-1].begin_datetime[3] = 0x01;/*�е���վ������0�º�0�գ����Ը�ֵ*/
            llvc_rec_state[port-1].begin_datetime[4] = 0x01;
            llvc_rec_state[port-1].end_datetime[3] = 0x01;
            llvc_rec_state[port-1].end_datetime[4] = 0x01;
            return;
        }
    }
    else
    {
        llvc_rec_state[port-1].port = port;
        #ifdef __RS485_PORT_CHECK__
        int16u2_bin(get_readport_meter_count_from_fast_index_with_doc_port(port),llvc_rec_state[port-1].total_count);
        #else
        int16u2_bin(get_readport_meter_count_from_fast_index(port),llvc_rec_state[port-1].total_count);
        #endif
    }
    llvc_rec_state[port-1].currec_flag.value = 0;
    llvc_rec_state[port-1].currec_flag.recording = 1;
    llvc_rec_state[port-1].read_count[0] = 0;
    llvc_rec_state[port-1].read_count[1] = 0;
    llvc_rec_state[port-1].read_vip_count = 0;
    mem_cpy(llvc_rec_state[port-1].begin_datetime,datetime,6);
    mem_set(llvc_rec_state[port-1].end_datetime,6,0x00);
    llvc_rec_state[port-1].end_datetime[3] = 0x01;
    llvc_rec_state[port-1].end_datetime[4] = 0x01;   // 20160708   jiangsu  ��ȡʱ���ʽ����
            
    fwrite_array(FILEID_RUN_DATA,FLADDR_LLVC_READ_STATE+(port-1)*sizeof(LLVC_REC_STATE),
    llvc_rec_state[port-1].value,sizeof(LLVC_REC_STATE));

}

void set_llvc_rec_state_out_seg(INT8U port)
{
    //LLVC_REC_STATE rec_state;
    if(port > COUNT_OF_READPORT) return;
    
    fread_array(FILEID_RUN_DATA,FLADDR_LLVC_READ_STATE+(port-1)*sizeof(LLVC_REC_STATE),
            llvc_rec_state[port-1].value,sizeof(LLVC_REC_STATE));
        if(port == COMMPORT_PLC_REC)
        {
        llvc_rec_state[port-1].port = COMMPORT_PLC;
        int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_PLC),llvc_rec_state[COMMPORT_PLC_REC-1].total_count);
        if(bin2_int16u(llvc_rec_state[COMMPORT_PLC_REC-1].total_count) == 0)//���⵱�ز���Ϊ��ʱ�����Լ����Դ��ڳ���ʼʱ���Լ�����ʱ��
        {
            mem_set(llvc_rec_state[port-1].begin_datetime,6,0x00);
            llvc_rec_state[port-1].begin_datetime[3] = 0x01;
            llvc_rec_state[port-1].begin_datetime[4] = 0x01;
            mem_set(llvc_rec_state[port-1].end_datetime,6,0x00);
            llvc_rec_state[port-1].end_datetime[3] = 0x01;
            llvc_rec_state[port-1].end_datetime[4] = 0x01;
            return;
        }
        }
        else
        {
        llvc_rec_state[port-1].port = port;
        #ifdef __RS485_PORT_CHECK__
        int16u2_bin(stat_readport_day_hold_complete_meter_count_from_fast_index_with_doc_port(port),llvc_rec_state[port-1].read_count);
        int16u2_bin(get_readport_meter_count_from_fast_index_with_doc_port(port),llvc_rec_state[port-1].total_count);
        #else
        int16u2_bin(stat_readport_day_hold_complete_meter_count_from_fast_index(port),llvc_rec_state[port-1].read_count);
        int16u2_bin(get_readport_meter_count_from_fast_index(port),llvc_rec_state[port-1].total_count);
        #endif
        }
 //       if (compare_string(llvc_rec_state[port-1].read_count,llvc_rec_state[port-1].total_count,2) == 0)
        {
        llvc_rec_state[port-1].currec_flag.value = 0;
        llvc_rec_state[port-1].currec_flag.finish = 1;
		llvc_rec_state[port-1].currec_flag.c2f55_stat_flag = 1;
        llvc_rec_state[port-1].read_vip_count = 0;
        mem_cpy(llvc_rec_state[port-1].end_datetime,datetime,6);
        // rs232_debug_info("\xBB",1);
        }
        fwrite_array(FILEID_RUN_DATA,FLADDR_LLVC_READ_STATE+(port-1)*sizeof(LLVC_REC_STATE),
        llvc_rec_state[port-1].value,sizeof(LLVC_REC_STATE));

}

void stat_llvc_rec_state(void)
{
    #ifdef __COMPUTE_XLOST__
    tagXLOSTCALFLAG cal_flag;
	#endif
    if(COMMPORT_PLC_REC > COUNT_OF_READPORT) return;
    if(compare_string(llvc_rec_state[COMMPORT_PLC_REC-1].begin_datetime+DAY,datetime+DAY,3) == 0) /*���ڳ��ֱ仯���������˳�ʼֵ����ô�Ͳ������ˣ�*/
    {

        int16u2_bin(stat_readport_day_hold_complete_meter_count_from_fast_index(COMMPORT_PLC),llvc_rec_state[COMMPORT_PLC_REC-1].read_count); /*�ն���ɹ�����*/
        int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_PLC),llvc_rec_state[COMMPORT_PLC_REC-1].total_count);  /*������*/
         #if ((defined __SUCCESS_RATE_OVER_95__) || (defined __COUNTRY_ISRAEL__) )
        INT16U total_count,read_count;
         total_count = bin2_int16u(llvc_rec_state[COMMPORT_PLC_REC-1].total_count);
         read_count = bin2_int16u(llvc_rec_state[COMMPORT_PLC_REC-1].read_count);
        if(total_count >= 10)
        portContext_plc.success_rate = (read_count*100) / total_count ;
        else
            portContext_plc.success_rate = 1; //��ʼֵ���ز���������10
        #endif
        if (compare_string(llvc_rec_state[COMMPORT_PLC_REC-1].read_count,llvc_rec_state[COMMPORT_PLC_REC-1].total_count,2) == 0)
        {
            if(bin2_int16u(llvc_rec_state[COMMPORT_PLC_REC-1].total_count) == 0) /*���0��ʱ����һ�£�����������жϾͲ������ˣ�����Ļ���Ҳ�Ҳ���������Խ�����*/
            {
                mem_set(llvc_rec_state[COMMPORT_PLC_REC-1].value,sizeof(LLVC_REC_STATE),0);
                llvc_rec_state[COMMPORT_PLC_REC-1].port = 0x1F;
                llvc_rec_state[COMMPORT_PLC_REC-1].begin_datetime[3] = 0x01;
                llvc_rec_state[COMMPORT_PLC_REC-1].begin_datetime[4] = 0x01;
                llvc_rec_state[COMMPORT_PLC_REC-1].end_datetime[3] = 0x01;
                llvc_rec_state[COMMPORT_PLC_REC-1].end_datetime[4] = 0x01;
            }
            else if (llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.finish == 0)
            {

                llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.value = 0;
                llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.finish = 1;
                llvc_rec_state[COMMPORT_PLC_REC-1].currec_flag.c2f55_stat_flag = 1;
                mem_cpy(llvc_rec_state[COMMPORT_PLC_REC-1].end_datetime,datetime,6);

                fwrite_array(FILEID_RUN_DATA,FLADDR_LLVC_READ_STATE+(COMMPORT_PLC_REC-1)*sizeof(LLVC_REC_STATE),
                llvc_rec_state[COMMPORT_PLC_REC-1].value,sizeof(LLVC_REC_STATE));

                #if defined(__COMPUTE_XLOST__)
                //������ɣ���������û�г����ĵ�����Ϊ0����
                fread_array(FILEID_RUN_DATA,FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);
                if(cal_flag.value != 0xFF)
                {
                    cal_flag.time_seg = 1;
                    fwrite_array(FILEID_RUN_DATA,FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);
                    compute_xlost();
                }
                #endif
            }
            else
            {
                /*����Ѿ����ɹ��ˣ�����Ҫ������*/
            }
        }

    }
    else
    {
        /*�������ʼʱ���ʵ��ʱ�䲻��һ�죬�����ʱ������Ϊ����ĳ�������δ��������ʼ��ͳ����Ϣ*/
        mem_set(llvc_rec_state[COMMPORT_PLC_REC-1].value,sizeof(LLVC_REC_STATE),0);
        llvc_rec_state[COMMPORT_PLC_REC-1].port = 0x1F;
        llvc_rec_state[COMMPORT_PLC_REC-1].begin_datetime[3] = 0x01;
        llvc_rec_state[COMMPORT_PLC_REC-1].begin_datetime[4] = 0x01;
        llvc_rec_state[COMMPORT_PLC_REC-1].end_datetime[3] = 0x01;
        llvc_rec_state[COMMPORT_PLC_REC-1].end_datetime[4] = 0x01;

        int16u2_bin(get_readport_meter_count_from_fast_index(COMMPORT_PLC),llvc_rec_state[COMMPORT_PLC_REC-1].total_count);/*��������*/
    }
}

INT8U check_recording_time(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    if(portContext->read_status.is_in_read_cycle == 0)   //����������
    {
        //����Ƿ��˳�������
        if(check_recording_time_begin(readportcontext))
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** IN RECORDING TIME ***");
            debug_println_ext(info);
            #endif

            set_llvc_rec_state_in_seg(COMMPORT_PLC_REC);
            #if (defined __PROVICE_CHONGQING__)
			update_llvc_rec_state();
            #endif
            portContext->read_status.is_in_read_cycle = 1;

            if (portContext->router_base_info.router_info1.comm_mode == 2) //����ز�·��  ���볭��ʱ��ʱ�����ø�λ·��
            {
                readport_plc.OnPortReadData = router_query_version;
                portContext->OnPortReadData = router_query_version;
                portContext->cur_plc_task = PLC_TASK_IDLE;
                portContext->cur_plc_task_step = 0;
            }
            else
            {
            readport_plc.OnPortReadData = router_reset;
            portContext->OnPortReadData = router_reset;
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = PLC_IDLE_ROUTER_RESET_LOW;
            }

            //���ó�������
            set_plc_read_task();

            return TRUE;
        }
        else
        {
          //����ʱ���⣬F11����ͳ�����ݣ���ʱӦ���ǳ��������������ϳ�������ʱ�䡣
//            #ifdef __SOFT_SIMULATOR__
//            snprintf(info,100,"*** WAIT RECORDING TIME ***");
//            debug_println_ext(info);
//            #endif
        }
    }
    else
    {
        //����Ƿ��˳�����
        if(check_recording_time_end(readportcontext))
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** EXIT RECORDING TIME ***");
            debug_println_ext(info);
            #endif

            portContext->read_status.is_in_read_cycle = 0;

            if (portContext->router_base_info.router_info4.afn_12H_is_valid)
            {
            readport_plc.OnPortReadData = router_send_afn_12_F2;
            portContext->OnPortReadData = router_send_afn_12_F2;
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = PLC_IDLE_EXIT_SEG_12_F2;
            }
            else
            {
                portContext->cur_plc_task = PLC_TASK_IDLE;
                portContext->cur_plc_task_step = 0;
                portContext->OnPortReadData = router_check_urgent_timeout;
                readport_plc.OnPortReadData = router_check_urgent_timeout;
            }

            set_llvc_rec_state_out_seg(COMMPORT_PLC_REC);

//            if (portContext->seg_end_time.is_lase_time_seg)
//            {
//                #if defined(__COMPUTE_XLOST__)
//                //ÿ������ʱ��ʱǿ�Ƽ�������û�г����ĵ�����Ϊ0����
//                fread_array(FILEID_RUN_DATA,FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);
//                if(cal_flag.value != 0xFF)
//                {
//                    cal_flag.time_seg = 1;
//                    fwrite_array(FILEID_RUN_DATA,FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);
//                    compute_xlost();
//                }
//                #endif
//
//                check_erc_39();
//            }
            return TRUE;
        }

        //ʱ�β����Ƿ�仯���仯���¼��ʱ�β���
        if(portContext->seg_end_time.is_time_seg_param_chg)
        {
            portContext->seg_end_time.is_time_seg_param_chg = 0;
            if(check_recording_time_begin(readportcontext) == FALSE)
            {
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** EXIT RECORDING TIME ***");
                debug_println_ext(info);
                #endif

                set_llvc_rec_state_out_seg(COMMPORT_PLC_REC);

                portContext->read_status.is_in_read_cycle = 0;

                readport_plc.OnPortReadData = router_send_afn_12_F2;
                portContext->OnPortReadData = router_send_afn_12_F2;
                portContext->cur_plc_task = PLC_TASK_IDLE;
                portContext->cur_plc_task_step = PLC_IDLE_EXIT_SEG_12_F2;

//                if (portContext->seg_end_time.is_lase_time_seg)
//                {
//                    #if defined(__COMPUTE_XLOST__)
//                    //ÿ������ʱ��ʱǿ�Ƽ�������û�г����ĵ�����Ϊ0����
//                    fread_array(FILEID_RUN_DATA,FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);
//                    if(cal_flag.value != 0xFF)
//                    {
//                        cal_flag.time_seg = 1;
//                        fwrite_array(FILEID_RUN_DATA,FLADDR_XLOST_CAL_FLAG, &cal_flag.value, 1);
//                        compute_xlost();
//                    }
//                    #endif
//
//                    check_erc_39();
//                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

INT8U check_meter_doc(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
	#ifdef __PROVICE_JIANGXI__
    INT16U count_fast_index = 0;
	#endif
    
    portContext = (PLCPortContext*)readportcontext;

    if (portContext->read_status.doc_chg)
    {
        #ifdef __FactoryTest__
        if (time_elapsed_10ms(portContext->doc_chg_time) > 100)  /*1s,������⣬�����·�����   */
        #else
        if (time_elapsed_10ms(portContext->doc_chg_time) > 3000)  //30s
        #endif
        {
            //������µ������
            if((portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC09)
            || (portContext->router_base_info.router_vendor == ROUTER_VEBDOR_HMXL)
             || (portContext->router_base_info.router_vendor == ROUTER_VEBDOR_GYXC))    //��Ѷ���·�ɣ��·��굵����Ҫ�������޸ĵķ����ǣ������仯����ͷ��
            {
              portContext->read_status.doc_chg = 0;
              portContext_plc.need_reset_router = 1;
            }
            else
            {
                //
                #ifdef __PROVICE_JIANGXI__
                //
                    #ifdef __SUCCESS_RATE_OVER_95__
                    count_fast_index = memory_fast_index_stat_port_node_count(READPORT_PLC,portContext->router_base_info.router_info3.rtu_no_mode,portContext->router_base_info.router_info4.dzc_cvt_no_mode);
    				if(count_fast_index <10)
    				{
    				    //�����仯������С��10�����ܳɹ���95%���ƣ�����10��������������鿴
    				    //95%�Ĵ����������⣬���������������޸ġ�
    				    portContext_plc.success_rate = 1; //��ʼֵ���ز���������10
    				}
    				#endif
				#endif
                start_check_meter_doc(portContext);
            }
//            #ifdef __SOFT_SIMULATOR__
//            snprintf(info,100,"*** PLC_TASK_CHECK_DOC ***");
//            debug_println_ext(info);
//            #endif
//            if(portContext->router_base_info.router_vendor == ROUTER_VENDOR_ZT) //�º��·�ɣ���֧����ͣ
//            {
//            portContext->cur_plc_task = PLC_TASK_CHECK_DOC;
//            readport_plc.OnPortReadData = router_check_urgent_timeout;
//            portContext->OnPortReadData = router_send_afn_10_F1;
//            portContext->cur_plc_task_step = PLC_CHECK_DOC_QUERY_NODE_COUNT_10F1_1;
//            }
//            else
//            {
//            readport_plc.OnPortReadData = router_send_afn_12_F2;
//            portContext->OnPortReadData = router_send_afn_12_F2;
//            portContext->cur_plc_task = PLC_TASK_CHECK_DOC;
//            portContext->cur_plc_task_step = PLC_CHECK_DOC_PAUSE_ROUTER;
//            }

            set_llvc_rec_state_in_seg(COMMPORT_PLC_REC);
            return TRUE;
        }

        //ֹͣ����ͬ�����̣�һ���������ͬ��
        if (portContext->cur_plc_task == PLC_TASK_CHECK_DOC)
        {
            //������µ������
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** ·�ɱȶԹ����У������仯����ͣ�ȶԣ��Ժ���� ***");
            debug_println_ext(info);
            #endif
            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = 0;
            portContext->OnPortReadData = router_check_urgent_timeout;
            readport_plc.OnPortReadData = router_check_urgent_timeout;

            return TRUE;
        }
    }
    return FALSE;
}

void set_plc_read_task(void)
{
    #ifdef __VOLTAGE_MONITOR__
    void update_valtage_list(INT8U is_clear,INT8U rec_flag);
    #endif

	#if defined(__COMPUTE_XLOST__)
    INT8U value;
	#endif
	INT8U idx;
	#ifdef __PROVICE_JIANGXI__
	INT8U cycle[2] = {0};
	#endif
    //INT8U  meter_event_plan[MAX_METER_EVENT_PLAN_COUNT]={0};//,plan_id;
    INT8U read_date[3]={0};

    //û���ز������ó�������Ϊ�˼ӿ�·���������ٶ�
    if (memory_fast_index_has_node(COMMPORT_PLC) == FALSE) return;

    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** SET RECORDING TASK ***");
    debug_println_ext(info);
    #endif

    //��ȡ�ϴ��ز���������
    fread_array(FILEID_RUN_DATA,FLADDR_PLC_READ_DATE,read_date,3);
    if(compare_string(read_date,datetime+DAY,3) == 0)
    {

    }
    else
    {
        #ifdef __VOLTAGE_MONITOR__
        init_F35_F66();

        if (get_system_flag(SYS_V_MONITOR,SYS_FLAG_BASE))
        {
            save_C2_F250();  //�����ѹ����Խ������
            update_valtage_list(1,1);    //����ص��ѹ�����б��еĵ�ѹ���ݣ������ó�������
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_voltage_moniter,COMMPORT_PLC);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_voltage_moniter_allow,COMMPORT_PLC);
        }
        #endif

        //��������
        mem_cpy(read_date,datetime+DAY,3);
        fwrite_array(FILEID_RUN_DATA,FLADDR_PLC_READ_DATE,read_date,3);
    }

    if(compare_string(read_meter_flag_cycle_day.cycle_day,datetime+DAY,3) == 0)
    {

    }
    else
    {
        //������������־�ָ�λ��
        #if defined(__COMPUTE_XLOST__)
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** ��λ�����־***");
        debug_println_ext(info);
        #endif
        value = 0;
        fwrite_array(FILEID_RUN_DATA, FLADDR_XLOST_CAL_FLAG, &value, sizeof(tagXLOSTCALFLAG));
        #endif

        //�ն���
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_PLC);
        //set_readport_read_meter_flag_from_fast_index(read_priority_ctrl_day_hold_1,COMMPORT_PLC); //�������ȼ�����
        //set_readport_read_meter_flag_from_fast_index(read_priority_ctrl_day_hold_2,COMMPORT_PLC);
       
        //#ifdef __ITEM_PRIORITY__
        for(idx=0;idx<ITEM_PRIORITY_CYCLE_DAY_COUNT;idx++)
        {
            set_readport_read_meter_flag_from_fast_index(read_priority_ctrl_item_cycle_day[idx],COMMPORT_PLC);
        }
        //#endif

        //�¶���
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_month,COMMPORT_PLC);
        //��ǰ
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);

        mem_cpy(read_meter_flag_cycle_day.cycle_day,datetime+DAY,3);

		
        #ifdef __PROVICE_JIANGXI__
    	//�����ϵ磬���볭��ʱ�Σ������¼�����������1�죬Ӧ�����,���������������������ᵼ�¶೭��һ�� TODO ????
    	fread_ertu_params(EEADDR_SET_F107+2,cycle,2);
    	if((cycle[1] == 3) && (cycle[0] == 1) ) //��������Ϊ1��
        {
            set_readport_read_meter_flag_from_fast_index(read_meter_event_grade_flag[1],COMMPORT_PLC);
			file_delete(FILEID_METER_GRADE_RECORD_MASK);//һ����������
		    file_delete(FILEID_EVENT_GRADE_READ_ITEM_CTRL);// ����������������
    	}
    	#endif
		
        //��������������ʵ�����壬���ٵ��á�
		//set_plc_meter_event_task_flag(1,2,3);//���ڳ����¼�
        //get_plan_min_cycle(meter_event_plan);       
    }
}

void set_plc_read_task_2_meter_idx(INT16U meter_idx)
{
    INT8U idx;
    #ifdef __PROVICE_JIANGXI__
	INT8U cycle[2] = {0};
	#endif
    if (meter_idx == 0 || meter_idx > MAX_METER_COUNT) return;

    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"*** set_plc_read_task_2_meter_idx ***");
    debug_println_ext(info);
    #endif

    //�ն���
    set_bit_value(read_meter_flag_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);
    //#ifdef __ITEM_PRIORITY__
    for(idx=0;idx<ITEM_PRIORITY_CYCLE_DAY_COUNT;idx++)
    {
        set_bit_value(read_priority_ctrl_item_cycle_day[idx],READ_FLAG_BYTE_NUM,meter_idx);
    }
    //#endif
    //�¶���
    set_bit_value(read_meter_flag_cycle_month,READ_FLAG_BYTE_NUM,meter_idx);
    //��ǰ����
   // ������ǰflag
    set_bit_value(read_meter_flag_cur_data,READ_FLAG_BYTE_NUM,meter_idx);
  //
    //��������
    set_bit_value(read_meter_flag_curve.flag,READ_FLAG_BYTE_NUM,meter_idx);
    if(check_const_ertu_switch(CONST_ERTU_SWITCH_PATCH_DAYHOLD_DATA))
    {
        set_bit_value(read_meter_flag_patch_day_hold,READ_FLAG_BYTE_NUM,meter_idx);
    }
    //������������
    set_bit_value(read_meter_flag_last_curve_cycle_day.flag,READ_FLAG_BYTE_NUM,meter_idx);

	#ifdef __METER_DAY_FREEZE_EVENT__
	//�����ز��ĵ�����¼�
	set_bit_value(read_meter_flag_day_freeze_event,READ_FLAG_BYTE_NUM,meter_idx);
	#endif

	#ifdef __PROVICE_JIANGXI__
	//�����¼�����������1�죬Ӧ�����
	fread_ertu_params(EEADDR_SET_F107+2,cycle,2);
	if((cycle[1] == 3) && (cycle[0] == 1) ) //��������Ϊ1��
    {
        set_bit_value(read_meter_event_grade_flag[1],READ_FLAG_BYTE_NUM,meter_idx);
		file_delete(FILEID_METER_GRADE_RECORD_MASK);//һ����������
		file_delete(FILEID_EVENT_GRADE_READ_ITEM_CTRL);// ����������������
	}
	#endif

}

/*+++
  ���ܣ��ڿ��������в��ҵ����
  ������
       INT8U port
       INT8U* meter_no   //�����
       INT16U* pos            //�ҵ�ʱ���Ǹñ�ŵĴ洢λ�ã�δ�ҵ�ʱ���Ǹñ��Ҫ�����λ��
       INT16U* rec_no         //��Ӧ�ĵ�����
  ����:

---*/
INT8U memory_fast_index_find_node_no(INT8U port,INT8U* node_no,INT16U* rec_no,INT16U* pos,INT8U* router_protocol,INT8U* user_class)
{
    INT16U idx,min,max,meter_count;
    INT16S result;
    INT8U port_meter_no[7];

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    port_meter_no[0] = port;
    mem_cpy(port_meter_no+1,node_no,6);

    meter_count = fast_index_list.count;
    if(meter_count > MAX_METER_COUNT)
    {
        meter_count = 0;
        mem_set(fast_index_list.fast_index[0].value,MAX_METER_COUNT*sizeof(FAST_INDEX),0xFF);
    }

    min=0;
    if (meter_count == 0)
    {
        *rec_no = 0;
        *pos = 0;

        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return FALSE;
    }
    else
    {
        max = meter_count -1;
    }
    
    while(min<=max)
    {

        idx=(min+max)>>1;   //��2

        //�Ƚ�
        result = compare_string(port_meter_no,fast_index_list.fast_index[idx].value,7);
        if(result == 0)
        {
            //�ҵ���
            //��ȡ��¼λ��
            *pos = idx;
			*rec_no = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);
            *router_protocol = fast_index_list.fast_index[idx].router_protocol;
            if(user_class != NULL)
                *user_class = fast_index_list.fast_index[idx].user_class;

            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return TRUE;
        }
        else if(result > 0)
        {
            min = idx+1;
        }
        else
        {
            if(idx==0) break; // ����Сֵ��ҪС
            max = idx-1;
        }
    }

    //Ŀ�ĵ��Ӧ������λ��=MIN
    *rec_no = min;
    *pos = min;

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return FALSE;
}
/*+++
  ���ܣ��ڿ��������в��ҵ����
  ������
       INT8U port
       INT8U* meter_no   //�����
       INT16U* pos            //�ҵ�ʱ���Ǹñ�ŵĴ洢λ�ã�δ�ҵ�ʱ���Ǹñ��Ҫ�����λ��
       INT16U* rec_no         //��Ӧ�ĵ�����
  ����:
  ���벻���ź������ͷ���2������һ�μ�������

---*/
INT8U memory_fast_index_find_node_no_nowait(INT8U port,INT8U* node_no,INT16U* rec_no,INT16U* pos,INT8U* router_protocol,INT8U* user_class)
{
    INT16U idx,min,max,meter_count;
    INT16S result;
    INT8U port_meter_no[7];

//    tpos_mutexPend(&SIGNAL_FAST_IDX);
    if(FALSE == tpos_mutexRequest(&SIGNAL_FAST_IDX))
    return 2;

    port_meter_no[0] = port;
    mem_cpy(port_meter_no+1,node_no,6);

    meter_count = fast_index_list.count;
    if(meter_count > MAX_METER_COUNT)
    {
        meter_count = 0;
        mem_set(fast_index_list.fast_index[0].value,MAX_METER_COUNT*sizeof(FAST_INDEX),0xFF);
    }

    min=0;
    if (meter_count == 0)
    {
        *rec_no = 0;
        *pos = 0;

        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return FALSE;
    }
    else
    {
        max = meter_count -1;
    }
    
    while(min<=max)
    {

        idx=(min+max)>>1;   //��2

        //�Ƚ�
        result = compare_string(port_meter_no,fast_index_list.fast_index[idx].value,7);
        if(result == 0)
        {
            //�ҵ���
            //��ȡ��¼λ��
            *pos = idx;
			*rec_no = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);
            *router_protocol = fast_index_list.fast_index[idx].router_protocol;
            if(user_class != NULL)
                *user_class = fast_index_list.fast_index[idx].user_class;
            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return TRUE;
        }
        else if(result > 0)
        {
            min = idx+1;
        }
        else
        {
            if(idx==0) break; // ����Сֵ��ҪС
            max = idx-1;
        }
    }

    //Ŀ�ĵ��Ӧ������λ��=MIN
    *rec_no = min;
    *pos = min;
    
    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return FALSE;
}

BOOLEAN memory_fast_index_find_node_no_no_port(INT8U* node_no,INT16U* rec_no,INT16U* pos,INT8U* router_protocol,INT8U* port,INT8U* user_class)
{
    if (memory_fast_index_find_node_no(COMMPORT_PLC,node_no,rec_no,pos,router_protocol,user_class))
    {
        *port = COMMPORT_PLC;
        return TRUE;
    }
    if (memory_fast_index_find_node_no(COMMPORT_485_REC,node_no,rec_no,pos,router_protocol,user_class))
    {
        *port = COMMPORT_485_REC;
        return TRUE;
    }
    if (memory_fast_index_find_node_no(COMMPORT_485_CAS,node_no,rec_no,pos,router_protocol,user_class))
    {
        *port = COMMPORT_485_CAS;
        return TRUE;
    }
    if (memory_fast_index_find_node_no(COMMPORT_485_CY,node_no,rec_no,pos,router_protocol,user_class))
    {
        *port = COMMPORT_485_CY;
        return TRUE;
    }
    return FALSE;
}

BOOLEAN memory_fast_index_update_info(INT8U port,INT8U* meter_no,INT16U meter_idx,INT8U router_protocol,INT8U* rtu_no,INT8U opt,INT8U user_class)   //__FAST_INDEX_RTU_NO__
{
    //INT16U cur_file_id,new_file_id;
    INT16U pos=0;
	INT16U rec_no;//,idx;
    //INT16U meter_count,count,move_count;
    //INT16U crc16;
    INT8U router_protocol_tmp,u_class;

    #ifdef __SOFT_SIMULATOR__
    snprintf(info,200,"meter_no=%02x%02x%02x%02x%02x%02x,meter_idx=%d,opt=%d",meter_no[5],meter_no[4],meter_no[3],meter_no[2],meter_no[1],meter_no[0],meter_idx,opt);
    debug_println(info);
    #endif
    if((router_protocol == 0) && ((meter_idx & FAST_IDX_RTU_FLAG) || (port == COMMPORT_485_REC) || (port == COMMPORT_485_CAS)))
    {
        prepare_zhiheng_read_param(meter_no, opt);
    }

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    if(opt == ADD)  //���
    {
        //�ҵ��ˣ������ظ����
        if(TRUE == memory_fast_index_find_node_no(port,meter_no,&rec_no,&pos,&router_protocol_tmp,&u_class))
        {
            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return FALSE;
        }
    }
    else if((opt == MODIFY) || (opt == DELETE)) //ɾ��
    {
        //û�ҵ��������޸ġ�ɾ��
        if(FALSE == memory_fast_index_find_node_no(port,meter_no,&rec_no,&pos,&router_protocol_tmp,&u_class))
        {
            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return FALSE;
        }
    }
    else
    {
        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return FALSE;
    }

    if(opt == ADD)
    {
        if ((fast_index_list.count + 1) > MAX_METER_COUNT)
        {
            //���ˣ�û�ط���
            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return FALSE;
        }

        tpos_enterCriticalSection();
        mem_cpy_right(fast_index_list.fast_index[pos+1].value,fast_index_list.fast_index[pos].value,(fast_index_list.count-pos)*sizeof(FAST_INDEX));
        fast_index_list.fast_index[pos].port = port;
        mem_cpy(fast_index_list.fast_index[pos].node,meter_no,6);
        mem_cpy(fast_index_list.fast_index[pos].seq_spec,(INT8U*)&meter_idx,2);
        fast_index_list.fast_index[pos].router_protocol = router_protocol;
        fast_index_list.fast_index[pos].user_class = user_class;
        #ifdef __RS485_PORT_CHECK__
        fast_index_list.fast_index[pos].doc_port = port;
        #endif
        #ifdef __FAST_INDEX_RTU_NO__
        if (rtu_no)
        {
            mem_cpy(fast_index_list.fast_index[pos].rtu_no,rtu_no,6);
        }
        #endif
        fast_index_list.count++;
        tpos_leaveCriticalSection();
    }
    else if(opt == MODIFY)   //�ڽ�ɾ���ɼ�����Ϣ���������Ϣʱ������û�������Ҫ���������ú�����ʱ���޷���ȡuser_class��Ϣ�����Զ���һ������ı���0xff��������optΪmodifyʱ������user_class
    {
        tpos_enterCriticalSection();
        fast_index_list.fast_index[pos].port = port;
        mem_cpy(fast_index_list.fast_index[pos].node,meter_no,6);
        mem_cpy(fast_index_list.fast_index[pos].seq_spec,(INT8U*)&meter_idx,2);
        fast_index_list.fast_index[pos].router_protocol = router_protocol;
        fast_index_list.fast_index[pos].user_class = user_class;
        #ifdef __FAST_INDEX_RTU_NO__
        if (rtu_no)
        {
            mem_cpy(fast_index_list.fast_index[pos].rtu_no,rtu_no,6);
        }
        #endif
        tpos_leaveCriticalSection();
    }
    else
    {
        tpos_enterCriticalSection();
        mem_cpy(fast_index_list.fast_index[pos].value,fast_index_list.fast_index[pos+1].value,(fast_index_list.count-pos-1)*sizeof(FAST_INDEX));
        fast_index_list.count--;
        mem_set(fast_index_list.fast_index[fast_index_list.count].value,sizeof(FAST_INDEX),0x00);
        tpos_leaveCriticalSection();
    }
    mem_cpy(fast_index_list.last_change_time,datetime,6);

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return TRUE;
}
#ifdef __RS485_PORT_CHECK__
BOOLEAN memory_fast_index_update_info_doc_port(INT8U port,INT8U* meter_no,INT16U meter_idx,INT8U router_protocol,INT8U* rtu_no,INT8U opt,INT8U user_class,INT8U doc_port)   //__FAST_INDEX_RTU_NO__
{
    //INT16U cur_file_id,new_file_id;
    INT16U pos=0;
	INT16U rec_no;//,idx;
    //INT16U meter_count,count,move_count;
    //INT16U crc16;
    INT8U router_protocol_tmp,u_class;

    #ifdef __SOFT_SIMULATOR__
    snprintf(info,200,"meter_no=%02x%02x%02x%02x%02x%02x,meter_idx=%d,opt=%d",meter_no[5],meter_no[4],meter_no[3],meter_no[2],meter_no[1],meter_no[0],meter_idx,opt);
    debug_println(info);
    #endif
    if((router_protocol == 0) && ((meter_idx & FAST_IDX_RTU_FLAG) || (port == COMMPORT_485_REC) || (port == COMMPORT_485_CAS)))
    {
        prepare_zhiheng_read_param(meter_no, opt);
    }

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    if(opt == ADD)  //���
    {
        //�ҵ��ˣ������ظ����
        if(TRUE == memory_fast_index_find_node_no(port,meter_no,&rec_no,&pos,&router_protocol_tmp,&u_class))
        {
            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return FALSE;
        }
    }
    else if((opt == MODIFY) || (opt == DELETE)) //ɾ��
    {
        //û�ҵ��������޸ġ�ɾ��
        if(FALSE == memory_fast_index_find_node_no(port,meter_no,&rec_no,&pos,&router_protocol_tmp,&u_class))
        {
            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return FALSE;
        }
    }
    else
    {
        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return FALSE;
    }

    if(opt == ADD)
    {
        if ((fast_index_list.count + 1) > MAX_METER_COUNT)
        {
            //���ˣ�û�ط���
            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return FALSE;
        }

        tpos_enterCriticalSection();
        mem_cpy_right(fast_index_list.fast_index[pos+1].value,fast_index_list.fast_index[pos].value,(fast_index_list.count-pos)*sizeof(FAST_INDEX));
        fast_index_list.fast_index[pos].port = port;
        mem_cpy(fast_index_list.fast_index[pos].node,meter_no,6);
        mem_cpy(fast_index_list.fast_index[pos].seq_spec,(INT8U*)&meter_idx,2);
        fast_index_list.fast_index[pos].router_protocol = router_protocol;
        fast_index_list.fast_index[pos].user_class = user_class;
        fast_index_list.fast_index[pos].doc_port = doc_port;
        #ifdef __FAST_INDEX_RTU_NO__
        if (rtu_no)
        {
            mem_cpy(fast_index_list.fast_index[pos].rtu_no,rtu_no,6);
        }
        #endif
        fast_index_list.count++;
        tpos_leaveCriticalSection();
    }
    else if(opt == MODIFY)   //�ڽ�ɾ���ɼ�����Ϣ���������Ϣʱ������û�������Ҫ���������ú�����ʱ���޷���ȡuser_class��Ϣ�����Զ���һ������ı���0xff��������optΪmodifyʱ������user_class
    {
        tpos_enterCriticalSection();
        fast_index_list.fast_index[pos].port = port;
        mem_cpy(fast_index_list.fast_index[pos].node,meter_no,6);
        mem_cpy(fast_index_list.fast_index[pos].seq_spec,(INT8U*)&meter_idx,2);
        fast_index_list.fast_index[pos].router_protocol = router_protocol;
        fast_index_list.fast_index[pos].user_class = user_class;
        fast_index_list.fast_index[pos].doc_port = doc_port;
        #ifdef __FAST_INDEX_RTU_NO__
        if (rtu_no)
        {
            mem_cpy(fast_index_list.fast_index[pos].rtu_no,rtu_no,6);
        }
        #endif
        tpos_leaveCriticalSection();
    }
    else
    {
        tpos_enterCriticalSection();
        mem_cpy(fast_index_list.fast_index[pos].value,fast_index_list.fast_index[pos+1].value,(fast_index_list.count-pos-1)*sizeof(FAST_INDEX));
        fast_index_list.count--;
        mem_set(fast_index_list.fast_index[fast_index_list.count].value,sizeof(FAST_INDEX),0x00);
        tpos_leaveCriticalSection();
    }
    mem_cpy(fast_index_list.last_change_time,datetime,6);

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return TRUE;
}
#endif
void memory_fast_index_create_from_meter_doc(void)
{
    INT16U meter_idx,meter_idx_tmp;
    READPORT_METER_DOCUMENT meter_doc;
    BOOLEAN rtu_flag;
    #ifdef __IMPORTANT_PARAM_BACKUP_RESTORE__
    INT8U meter_map[256]={0};
    #endif
    #ifdef __MEXICO_GUIDE_RAIL__
    INT8U  CIU_addr[6] = {0};
    #endif

    mem_set(meter_spot_2_meter_seq_map,MAX_METER_COUNT1,0x00);

    #ifdef __IMPORTANT_PARAM_BACKUP_RESTORE__
    get_meter_doc_mask(meter_map);
    #endif

    for(meter_idx=1;meter_idx<=MAX_METER_COUNT;meter_idx++)
    {
        #ifdef __IMPORTANT_PARAM_BACKUP_RESTORE__
        if(!file_exist_of_refactor_fast_index(meter_idx,meter_map))     continue;
        #endif
        if(prepare_read_meter_doc(meter_idx,&meter_doc))
        {
            //TODO:�й�F150���������Ч��ʶ(�ѱ����)����������
            meter_spot_2_meter_seq_map[bin2_int16u(meter_doc.spot_idx)] = meter_idx | 0x8000;

            if(meter_doc.baud_port.port == COMMPORT_PLC)
            {
                rtu_flag = FALSE;
                #ifdef __MEXICO_CIU__
                /*ֻ�����⴦��CIU ���Ǳ��ַ������ֽ�����   �������òɼ���ģʽ ������ʱ���ܴ������� 
                rtu_no �洢����CIU��Ӧ�ı�� 
                180628-by-lyc */
                mem_cpy(CIU_addr,meter_doc.meter_no,6);
                CIU_addr[5] = 0;
                insert_fast_index(meter_doc.baud_port.port,CIU_addr,FAST_IDX_RTU_FLAG,meter_doc.protocol,meter_doc.meter_no,meter_doc.meter_class.value);
                #else
                if (isvalid_meter_addr(meter_doc.rtu_no,FALSE))
                {
                    rtu_flag = TRUE;
                    insert_fast_index(meter_doc.baud_port.port,meter_doc.rtu_no,FAST_IDX_RTU_FLAG,meter_doc.protocol,NULL,meter_doc.meter_class.value);
                }
                #endif

                if(check_is_sqr_protocol(meter_doc.protocol))
                {
                    meter_idx_tmp = (rtu_flag) ? FAST_IDX_DZC_METER_FLAG : 0;
                }
                else
                {
                    meter_idx_tmp = (rtu_flag) ? FAST_IDX_METER_FLAG : 0;
                }
                meter_idx_tmp |= meter_idx;
                insert_fast_index(meter_doc.baud_port.port,meter_doc.meter_no,meter_idx_tmp,meter_doc.protocol,meter_doc.rtu_no,meter_doc.meter_class.value);
            }
            else
            {
                insert_fast_index(meter_doc.baud_port.port,meter_doc.meter_no,meter_idx,meter_doc.protocol,meter_doc.rtu_no,meter_doc.meter_class.value);
                //if(portContext_rs485[RS485_REC_PORT_IDX].read_status.doc_chg == 1) portContext_rs485[RS485_REC_PORT_IDX].read_status.doc_chg = 0;

            }
        }
    }
    mem_cpy(fast_index_list.last_change_time,datetime,6);
    portContext_plc.read_status.doc_chg = 0;
}

void memory_fast_index_init_cjq_info(CJQ_INFO *cjq,INT8U *rtu_no,BOOLEAN init_flag)    //__FAST_INDEX_RTU_NO__
{
    INT16U meter_idx;//,meter_idx_tmp;
    INT16U node_count,idx;
    INT16U tmp_idx = 0;
    INT16U rec_pos = 0;
    INT16U cjq_idx = 0;
	#ifndef __FAST_INDEX_RTU_NO__//δ����˺궨�壬�Ŷ������
    READPORT_METER_DOCUMENT meter_doc;
	#endif
    INT8U slave_idx;
    #ifdef __CJQ_ORDER_MODE__    
    INT8U idx_chg;
    #endif
    //BOOLEAN rtu_flag;
    INT8U tmp = 0;
    INT8U tmp1 = 0;
    INT8U protocol = 0;
    INT8U meter_no[6] = {0};

    slave_idx=0;

    #ifdef __CJQ_ORDER_MODE__
    INT8U i,j,suiji=0;
    CJQ_READ_INFO* read_info;
    #endif
    tpos_mutexPend(&SIGNAL_FAST_IDX);

    //��ʼ���ɼ����ṹ
    //��init_flag��FALSE����ʾ����Ҫ��ʼ��
    //��ʱskip_count��more_count��begin_idx��Ϊ����һЩ�������жϣ��Ͳ��ܳ�ʼ����
    if(init_flag == FALSE)
    {
        tmp = cjq->skip_count;
        tmp1 = cjq->more_count;
        tmp_idx = cjq->begin_idx;
    }
    mem_set((INT8U *)cjq,sizeof(CJQ_INFO),0x00);
    if(init_flag == FALSE)
    {
        cjq->skip_count = tmp;
        cjq->more_count = tmp1;
        cjq->begin_idx = tmp_idx;
    }

    mem_cpy(cjq->rtu_no,rtu_no,6);
    mem_set(cjq->meter_no,6,0xFF);   //���ó���Ч���

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
    
    meter_idx = memory_fast_index_find_first_meter_idx_in_rtu(COMMPORT_PLC,cjq->rtu_no,&rec_pos);
    if(meter_idx > MAX_METER_COUNT)
    {
        //��ʱ���ݲɼ�����δ�ڿ����������ҵ���Ӧ��λ��
        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return;
    }
    
    fread_meter_params(meter_idx,PIM_CJQ_METER_NO,meter_no,6);
    
    if(check_is_all_FF(meter_no,6))
    {
        tmp_idx = rec_pos;
    }
    else
    {
        (void)memory_fast_index_find_node_no(COMMPORT_PLC,meter_no,&cjq_idx,&tmp_idx,&protocol,NULL);
        //�п��ܵ����ڵĲɼ����Ž����˱����Ҫע�⴦��
        if(compare_string(cjq->rtu_no,fast_index_list.fast_index[tmp_idx].rtu_no,6) != 0)
        {
            tmp_idx = rec_pos;
        }
        else
        {
            //�Ӷ�Ӧ����ŵ���һ��λ�ÿ�ʼ���ң���ֹ·�ɲ��ϻ��ർ��һֱ���ڳ���ĳһ�����
            tmp_idx += 1;
        }
    }
    
    //������״ι����б�����ʼ��Ž��м�¼������ȷ���Ƿ񽫿��������б���ѯһ��
    if(TRUE == init_flag)
    {
        cjq->begin_idx = tmp_idx;
    }
    
    for(idx=tmp_idx;idx<node_count;idx++)
    {
        //��more_count����1ʱ����ʾ���ٴ�ѭ����������ʱҪ�����Ƿ��Ѿ������������еĵ��
        if((cjq->more_count == 1) && (idx == cjq->begin_idx))
        {
            //�������������־�����ټ�������
            cjq->more_count = 0;
            tpos_mutexFree(&SIGNAL_FAST_IDX);
            return;
        }
        //��ȡ�ز��ӽڵ��ַ,���
        if (fast_index_list.fast_index[idx].port != READPORT_PLC) continue;

        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);

        if (((meter_idx & FAST_IDX_METER_FLAG) == 0) && ((meter_idx & FAST_IDX_DZC_METER_FLAG) == 0))  continue;

        meter_idx &= FAST_IDX_MASK;

        if (check_read_meter_flag(meter_idx) == FALSE) continue;
        #ifdef __FAST_INDEX_RTU_NO__
        if (compare_string(cjq->rtu_no,fast_index_list.fast_index[idx].rtu_no,6) != 0) continue;
        #else
        if (prepare_read_meter_doc(meter_idx,&meter_doc) == FALSE) continue;
        if (compare_string(cjq->rtu_no,meter_doc.rtu_no,6) != 0) continue;
        #endif

        cjq->salve_idx[slave_idx++] = meter_idx;

        if(slave_idx >= MAX_CJQ_METER_COUNT)
        {

            cjq->more_count = 1;   //���òɼ��������ڶ�Ӧ��λ����ʱ�����ڼ�����������б�
            break;
        }
    }
    
    //�����ַ�б�δ���������ڿ��������У���0��tmp_idx����һ�飬��֤���������Ǳ�����һ���
    if(slave_idx < MAX_CJQ_METER_COUNT)
    {
        for(idx=0;idx<tmp_idx;idx++)
        {
            //��more_count����1ʱ����ʾ���ٴ�ѭ����������ʱҪ�����Ƿ��Ѿ������������еĵ��
            if((cjq->more_count == 1) && (idx == cjq->begin_idx))
            {
                //�������������־�����ټ�������
                cjq->more_count = 0;
                tpos_mutexFree(&SIGNAL_FAST_IDX);
                return;
            }
            //��ȡ�ز��ӽڵ��ַ,���
            if (fast_index_list.fast_index[idx].port != READPORT_PLC) continue;

            meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);

            if (((meter_idx & FAST_IDX_METER_FLAG) == 0) && ((meter_idx & FAST_IDX_DZC_METER_FLAG) == 0))  continue;

            meter_idx &= FAST_IDX_MASK;

            if (check_read_meter_flag(meter_idx) == FALSE) continue;
            #ifdef __FAST_INDEX_RTU_NO__
            if (compare_string(cjq->rtu_no,fast_index_list.fast_index[idx].rtu_no,6) != 0) continue;
            #else
            if (prepare_read_meter_doc(meter_idx,&meter_doc) == FALSE) continue;
            if (compare_string(cjq->rtu_no,meter_doc.rtu_no,6) != 0) continue;
            #endif

            cjq->salve_idx[slave_idx++] = meter_idx;

            if(slave_idx >= MAX_CJQ_METER_COUNT)
            {
                cjq->more_count = 1;   //���òɼ��������ڶ�Ӧ��λ����ʱ�����ڼ�����������б�
                break;
            }
        }
    }
    #ifdef __CJQ_ORDER_MODE__
    //�������������������ı�񣬲��ǰ��ձ��������ġ��������Ÿ���
    for(i=0;i<slave_idx-1;i++ )
    {
        for(j=0;j<slave_idx-i-1;j++)
        {
            if(cjq->salve_idx[j] > cjq->salve_idx[j+1])
            {
                meter_idx = cjq->salve_idx[j];
                cjq->salve_idx[j] = cjq->salve_idx[j+1];
                cjq->salve_idx[j+1] = meter_idx;
            }
        }
    }
    #endif

    tpos_mutexFree(&SIGNAL_FAST_IDX);

    #ifdef __CJQ_ORDER_MODE__
    for(idx =0;idx<MAX_READ_INFO_CNT;idx++)   //cjq_read_info��ά��������·��ͬ������ʱ������ģʽ�������������
    {
        if(0 == compare_string(cjq_read_info[idx].cjq_no,cjq->rtu_no,6))
        {
            //read_info = &(cjq_read_info[idx]);
            break;
        }
    }
    if(idx >= MAX_READ_INFO_CNT)
    {
        for(idx =0;idx<MAX_READ_INFO_CNT;idx++)
        {
            if((check_is_all_ch(cjq_read_info[idx].cjq_no,6,0xFF))||(check_is_all_ch(cjq_read_info[idx].cjq_no,6,0)) )
            {
                mem_cpy(cjq_read_info[idx].cjq_no, cjq->rtu_no,6);
                cjq_read_info[idx].meter_seq = 0;
                cjq_read_info[idx].comm_ok = 0;
                break;
            }
        }
        if(idx >= MAX_READ_INFO_CNT)  //����20����¼�Ĳɼ������治���ˣ����������
        {
            //for(idx=0;idx<3;idx++)
            {
                if(slave_idx > 1)
                {
                    idx_chg = os_get_systick_10ms() % slave_idx;
                    suiji = 1;
                    //if(idx_chg != 0 )
                    //{
                    //    meter_idx = cjq->salve_idx[0];
                    //    cjq->salve_idx[0] = cjq->salve_idx[idx_chg];
                    //    cjq->salve_idx[idx_chg] = meter_idx;
                    //}
                }
                //else break;
            }
            //return;
        }
    }
    if(0 == suiji)
    {
        read_info = &(cjq_read_info[idx]);
        for(idx=0;idx<slave_idx;idx++)
        {
            if(cjq->salve_idx[idx] > read_info->meter_seq)
            {
                idx_chg = idx;
                break;
            }
            else if(cjq->salve_idx[idx] == read_info->meter_seq)
            {
                if(read_info->comm_ok)
                {
                    idx_chg = idx;
                    break;
                }
            }

        }
        if(idx >=  slave_idx)
        {
            idx_chg = 0;
            read_info->meter_seq = cjq->salve_idx[idx_chg];
            read_info->comm_ok = 0;
        }
        else
        {
            read_info->meter_seq = cjq->salve_idx[idx_chg];
            read_info->comm_ok = 0;
        }
        //����ʱ��������������ⲽ��ϵ�ķ�Ӧ������
        fwrite_array(FILEID_PLC_REC_TMP,PIM_CJQ_READ_INFO,(INT8U*)cjq_read_info,MAX_READ_INFO_CNT*(sizeof(CJQ_READ_INFO)));
    }
    if(idx_chg != 0 )
    {
        meter_idx = cjq->salve_idx[0];
        cjq->salve_idx[0] = cjq->salve_idx[idx_chg];
        cjq->salve_idx[idx_chg] = meter_idx;
    }
    #else
//    for(idx=0;idx<3;idx++)
//    {
//        if(slave_idx > 1)
//        {
//            idx_chg = os_get_systick_10ms() % slave_idx;
//
//            if(idx_chg != 0 )
//            {
//                meter_idx = cjq->salve_idx[0];
//                cjq->salve_idx[0] = cjq->salve_idx[idx_chg];
//                cjq->salve_idx[idx_chg] = meter_idx;
//            }
//        }
//        else break;
//    }
    #endif
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"\t %d  [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d] idx:%d  skip_count:%d  more_count:%d",slave_idx,
            cjq->salve_idx[0],cjq->salve_idx[1],cjq->salve_idx[2],cjq->salve_idx[3],
            cjq->salve_idx[4],cjq->salve_idx[5],cjq->salve_idx[6],cjq->salve_idx[7],
            cjq->salve_idx[8],cjq->salve_idx[9],cjq->salve_idx[10],cjq->salve_idx[11],
            cjq->salve_idx[12],cjq->salve_idx[13],cjq->salve_idx[14],cjq->salve_idx[15],
            cjq->idx,cjq->skip_count,cjq->more_count);
    debug_println_ext(info);
    #endif
}

INT8U prepare_cjq_read_item(CJQ_INFO *cjq,READ_PARAMS *read_params,INT8U* frame,INT8U* frame_len)
{
    INT16U meter_idx;
    INT16U pos = 0;
    INT8U rtu_no[6];

CJQ_READ:
    for(;cjq->idx<MAX_CJQ_METER_COUNT;cjq->idx++) // cjq->idx
    {
        meter_idx = cjq->salve_idx[cjq->idx];
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT))
        {
            mem_set(cjq->meter_no,6,0xFF);
            continue;
        }

        //����ַ��һ����Ҫ����׼������
        if ((compare_string(read_params->meter_doc.meter_no,cjq->meter_no,6) != 0) || (read_params->is_day_changed))
        {
            read_params->is_day_changed = 0;
            if (prepare_read_meter_param(meter_idx,read_params) == FALSE)
            {
                mem_set(cjq->meter_no,6,0xFF);
                continue;
            }
            mem_cpy(cjq->meter_no,read_params->meter_doc.meter_no,6);
            cjq->try_count = 0;
        }

        //һ���ɼ����µĵ����3�Σ�����������
        if(cjq->try_count > 3)
        {
            if(cjq->skip_count != 0xFF)
            {
                cjq->skip_count++;
            }
            mem_set(cjq->meter_no,6,0xFF);
            continue;
        }

        //����Ƿ��г���������
        if (prepare_read_item(read_params,frame,frame_len))
        {
            cjq->try_count++;
            return TRUE;
        }
        else
        {
            mem_set(cjq->meter_no,6,0xFF);
        }
    }

    //�������Ҫ������������б�ı�־��������������ټ�������
    if(cjq->more_count)
    {
        mem_cpy(rtu_no,cjq->rtu_no,6);
        meter_idx = memory_fast_index_find_first_meter_idx_in_rtu(COMMPORT_PLC,rtu_no,&pos);
        if(meter_idx > MAX_METER_COUNT)
        {
            //��ʱ���ݲɼ�����δ�ڿ����������ҵ���Ӧ�����ݣ���δ���
            return FALSE;
        }
        fwrite_meter_params(meter_idx,PIM_CJQ_METER_NO,read_params->meter_doc.meter_no,6);
        memory_fast_index_init_cjq_info(cjq,rtu_no,FALSE);
        goto CJQ_READ;
    }

    if(cjq->skip_count) read_params->control.loop = 1;    //���ýڵ㳭��ʧ��
//    read_params->control.loop = 1;    //���ýڵ㳭��ʧ��

    return FALSE;
}

//-----�ѱ�---------------------------------------------------------------------
void timer_plc_net(void)
{
    INT8U open_flag = 0;
	#ifdef __PLC_NET_JIANGSU__
    SET_F111 set_F111;
	#endif

    #ifdef __PLC_NET_JIANGSU__
    fread_ertu_params(EEADDR_SET_F111 + (COMMPORT_PLC - 1) * sizeof(SET_F111),set_F111.value,sizeof(SET_F111));
    if (set_F111.port != COMMPORT_PLC)
    {
        set_F111.port = COMMPORT_PLC;
        set_F111.flag = 1;
        set_F111.bcd_start_time[0] = 0x00;
        set_F111.bcd_start_time[1] = 0x20;
        set_F111.bcd_start_time[2] = 0x00;
        set_F111.exec_hour = 3;
    }
    if (set_F111.flag != 1) return;
    if ((set_F111.bcd_start_time[2] != 0) && ((datetime[DAY] % BCD2byte(set_F111.bcd_start_time[2])) != 0)) return;
    if ((datetime[HOUR] == BCD2byte(set_F111.bcd_start_time[1])) && (datetime[MINUTE] == BCD2byte(set_F111.bcd_start_time[0])) && (portContext_plc.cur_plc_task != PLC_TASK_PLC_NET))
    {
        fread_array(FILEID_RUN_PARAM,FLADDR_AUTO_MNG_METER_SWITCH,&open_flag,1);
        if (open_flag != 2)    //���ղ�Ҫ��ӵ���
        {
            open_flag = 2;
            fwrite_array(FILEID_RUN_PARAM,FLADDR_AUTO_MNG_METER_SWITCH,&open_flag,1);
        }

        portContext_plc.read_status.plc_net = 1;
        portContext_plc.plc_net_cast_485_time_s = 1 * 60;    
        portContext_plc.plc_net_wait_report_node_time_s = (set_F111.exec_hour > 12) ? 3 : set_F111.exec_hour;
        portContext_plc.plc_net_wait_report_node_time_s *= 60 * 60; //2Сʱ
    }
    #else
        #if (defined __PROVICE_SICHUAN__) || (defined __PROVICE_SHAANXI__)         
        if(portContext_plc.router_base_info.router_info1.comm_mode == 2)
        {
            return;  /*�Ĵ��������ִ�е���̨��ʶ���������Ҫÿ������*/
        }
        #endif

#ifdef __PROVICE_GANSU__ /*����˵��Ҫ������Ӱ������*/
return;
#endif
        if (portContext_plc.router_base_info.router_vendor == ROUTER_VENDOR_RISECOMM) return; //���������˹��·�ɣ�������һ�����ģ���ʱ��֧���ѱ�

        if(portContext_plc.router_base_info.router_info1.comm_mode == 2)  /*���Ҫ��12���18��Ĭ������һ�γ���30���ӵ���������ϱ�*/
        {
          if (((datetime[HOUR] == 18) && (datetime[MINUTE] == 1) && (portContext_plc.cur_plc_task != PLC_TASK_PLC_NET))
          || ((datetime[HOUR] == 12) && (datetime[MINUTE] == 1) && (portContext_plc.cur_plc_task != PLC_TASK_PLC_NET)))
          {
                fread_array(FILEID_RUN_PARAM,FLADDR_AUTO_MNG_METER_SWITCH,&open_flag,1);
                if ((open_flag != 1) && (open_flag != 2)) 
                {
                    return;  /*��ҪĬ�Ͽ��������ô������149�Ͳ�����*/
                }

                portContext_plc.read_status.plc_net = 1;
                portContext_plc.plc_net_cast_485_time_s = 1*60*60;    //�ɼ����ѱ���ʱ
                portContext_plc.plc_net_wait_report_node_time_s = 30*60; //��Сʱ
          }
        }
        else if ((datetime[HOUR] == 20) && (datetime[MINUTE] == 1) && (portContext_plc.cur_plc_task != PLC_TASK_PLC_NET))
        {
            fread_array(FILEID_RUN_PARAM,FLADDR_AUTO_MNG_METER_SWITCH,&open_flag,1);
            if ((open_flag != 1) && (open_flag != 2)) return;

            portContext_plc.read_status.plc_net = 1;
            portContext_plc.plc_net_cast_485_time_s = 1*60*60;    //�ɼ����ѱ���ʱ
            portContext_plc.plc_net_wait_report_node_time_s = 2*60*60; //2Сʱ
        }

        #endif
}

INT8U set_plc_net(INT16U last_time)
{
    #ifdef __PLC_NET_JIANGSU__
    SET_F111 set_F111;
    INT8U value;
	#endif

    if(portContext_plc.read_status.plc_net) return 1;
    if(portContext_plc.cur_plc_task == PLC_TASK_PLC_NET) return 1;
    portContext_plc.read_status.plc_net = 1;
    portContext_plc.plc_net_cast_485_time_s = 60;
    portContext_plc.plc_net_wait_report_node_time_s = 0;
    portContext_plc.params.task_plc_net.last_minute[0] = last_time;
    portContext_plc.params.task_plc_net.last_minute[1] = last_time >> 8;
    #ifdef __PLC_NET_JIANGSU__
    fread_ertu_params(EEADDR_SET_F111 + (COMMPORT_PLC - 1) * sizeof(SET_F111),set_F111.value,sizeof(SET_F111));
    if (set_F111.port != COMMPORT_PLC)
    {
        set_F111.port = COMMPORT_PLC;
        set_F111.flag = 1;
        set_F111.bcd_start_time[0] = 0x00;
        set_F111.bcd_start_time[1] = 0x20;
        set_F111.bcd_start_time[2] = 0x00;
        set_F111.exec_hour = 3;
        value = 2;
        fwrite_array(FILEID_RUN_PARAM,FLADDR_AUTO_MNG_METER_SWITCH,&value,1);
    }
    portContext_plc.plc_net_wait_report_node_time_s = set_F111.exec_hour * 60 * 60;
    #endif

    return 0;
}

INT8U set_plc_net_force_exit(void)
{
    if(portContext_plc.cur_plc_task == PLC_TASK_PLC_NET)
    {
        portContext_plc.params.task_plc_net.run_params.is_force_exit = 1;
        return 0;
    }
    return 1;
}

INT8U active_node_logon(PLCPortContext *portContext)
{
    INT32U count;
    INT16U plc_net_last_minute_tmp;
    INT8U cur_datetime[6];
    
    count = 0;
    fread_array(FILEID_RUN_PARAM,FLADDR_AUTO_MNG_METER_SWITCH,(INT8U*)&count,1);
    if(count == 1) portContext->params.task_plc_net.run_params.save_mode = 0;
    else portContext->params.task_plc_net.run_params.save_mode = 1;
    if(portContext->params.task_plc_net.run_params.save_mode)
    {
        count = 0;
        fread_array(FILEID_RUN_PARAM,FLADDR_PLC_NET_FILE_VAILD_INDEX,(INT8U*)&count,1);
        if(count > 1) portContext->params.task_plc_net.run_params.file_idx = 0;
        else portContext->params.task_plc_net.run_params.file_idx = (count == 0) ? 1 : 0;
        if(portContext->router_base_info.router_info3.plc_net_clear_file)
        {
            file_delete(FILEID_PLC_NET);
            file_delete(FILEID_PLC_NET+1);
        }
        else
        {
            file_delete(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx);
        }
        //�������ļ���
        fread_array(FILEID_RUN_PARAM,FLADDR_PLC_NET_STAMP,portContext->params.task_plc_net.stamp,4);
        count = bin2_int32u(portContext->params.task_plc_net.stamp);
        count++;
        int32u2_bin(count,portContext->params.task_plc_net.stamp);
        portContext->params.task_plc_net.start_time[0] = portContext->params.task_plc_net.run_params.file_idx;
        fwrite_array(FILEID_RUN_PARAM,FLADDR_PLC_NET_STAMP,portContext->params.task_plc_net.stamp,5);  //�������file_idx������������һ������5���ֽ�
    }
    mem_set(portContext->plc_net_buffer,511,0x00);

   //���ü���ӽڵ�����ע�����
    plc_net_last_minute_tmp = bin2_int16u(portContext->params.task_plc_net.last_minute); //��Ҫ�жϺͶ�ʱ�ѱ�Ĺ�ϵ����

    if(plc_net_last_minute_tmp > 200 ) //ʱ�䲻��̫��������200���ӣ�����Ĭ��8�㣬���������11:30���ң�
    plc_net_last_minute_tmp = 200 ;

    portContext->plc_net_wait_time_s = plc_net_last_minute_tmp*60; //·�ɵȴ�ʱ��ת������

    mem_set(portContext->params.task_plc_net.start_time,10,0x00);
    cur_datetime[0] = byte2BCD(datetime[SECOND]);
    cur_datetime[1] = byte2BCD(datetime[MINUTE]);
    cur_datetime[2] = byte2BCD(datetime[HOUR]);
    cur_datetime[3] = byte2BCD(datetime[DAY]);
    cur_datetime[4] = byte2BCD(datetime[MONTH]);
    cur_datetime[5] = byte2BCD(datetime[YEAR]);
    mem_cpy(portContext->params.task_plc_net.start_time,cur_datetime,6);
    portContext->params.task_plc_net.last_minute[0] = plc_net_last_minute_tmp;
    portContext->params.task_plc_net.last_minute[1] = plc_net_last_minute_tmp>>8;

    portContext->params.task_plc_net.run_params.is_force_exit = 0;
    portContext->plc_net_time_out_10ms = os_get_systick_10ms();

   // if(portContext->router_base_info.router_info1.comm_mode == 2)
    {
   //     readport_plc.OnPortReadData = router_send_afn_11_F225; /*�����ѱ����������,����ſ�����*/
   //     portContext->OnPortReadData = router_send_afn_11_F225;
    }
   // else
    {
        readport_plc.OnPortReadData = router_send_afn_11_F5; /*���ͼ����ϱ��󣬵ȴ�·���ϱ�����������ͣ��*/
        portContext->OnPortReadData = router_send_afn_11_F5;
    }

    portContext->cur_plc_task = PLC_TASK_PLC_NET;
    portContext->cur_plc_task_step = PLC_NET_ACTIVE_NODE_LOGON_11_F5;

	return 0;
}
//·�����ã�F225������ӽڵ�ע��
INT8U router_send_afn_11_F225(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U tmp_send_data[10];

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    tmp_send_data[0] = byte2BCD(datetime[SECOND]);
    tmp_send_data[1] = byte2BCD(datetime[MINUTE]);
    tmp_send_data[2] = byte2BCD(datetime[HOUR]);
    tmp_send_data[3] = byte2BCD(datetime[DAY]);
    tmp_send_data[4] = byte2BCD(datetime[MONTH]);
    tmp_send_data[5] = byte2BCD(datetime[YEAR]);

    tmp_send_data[6] = portContext->params.task_plc_net.last_minute[0];
	tmp_send_data[7] = portContext->params.task_plc_net.last_minute[1];

	tmp_send_data[8] = portContext->params.task_plc_net.node_repeat;
    tmp_send_data[9] = 0x30;
    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RSET,DT_F225,tmp_send_data,10,portContext);
    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}
INT8U check_plc_net(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    //INT32U ertu_addr;

    portContext = (PLCPortContext*)readportcontext;
  //  if(portContext-> router_interactive_status.plc_net_not_allow == 1)return FALSE;//���ſ��·�ɣ�������������еģ������ڱȶԵ����еģ��������ѱ�

    if (portContext->read_status.plc_net)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** START PLC NET ***");
        debug_println_ext(info);
        #endif

        portContext->read_status.plc_net = 0;

        if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_FC09) return FALSE;
        if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_RISECOMM) return FALSE; //��˹����·�ɶ��ѱ�֧�֣��ȹر�
     //   if (portContext->router_base_info.router_vendor == ROUTER_VENDOR_NTSR) return FALSE;    //�������·�ɶ��ѱ�֧�֣��ȹر�
        portContext->router_work_info.status.pause = 0;

        //�����ѱ�ǰ����ʱ��״̬����Ϊʱ���⣬�����ѱ���ɺ���½���ʱ�ο�ʼ��������
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** EXIT RECORDING TIME ***");
        debug_println_ext(info);
        #endif
        portContext->read_status.is_in_read_cycle = 0;

        if (portContext->cur_plc_task == PLC_TASK_READ_METER)  //��������У���Ҫ�ȷ���ͣ
        {
            readport_plc.OnPortReadData = router_send_afn_12_F2;
            portContext->OnPortReadData = router_send_afn_12_F2;
            portContext->cur_plc_task = PLC_TASK_PLC_NET;
            portContext->cur_plc_task_step = PLC_NET_PAUSE_ROUTER_12_F2;
        }
        else
        {
            get_router_main_node_addr(portContext->params.task_check_main_node.main_node);
            readport_plc.OnPortReadData = router_send_afn_05_F1;
            portContext->OnPortReadData = router_send_afn_05_F1;
            portContext->cur_plc_task = PLC_TASK_PLC_NET;
            portContext->cur_plc_task_step = PLC_NET_SET_MAIN_NODE_05_F1;
        }
        return TRUE;
    }
    return FALSE;
}
INT8U check_plc_distinguish(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    //INT32U ertu_addr;

    portContext = (PLCPortContext*)readportcontext;
    if (portContext->read_status.clear_area_distinguish)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** START PLC clear_area_distinguish ***");
        debug_println_ext(info);
        #endif

        portContext->read_status.clear_area_distinguish = 0;

        if (portContext->router_base_info.router_vendor != ROUTER_VENDOR_TOPSCOMM) return FALSE;

        portContext->router_work_info.status.pause = 0;


        if (portContext->cur_plc_task == PLC_TASK_READ_METER)  //��������У���Ҫ�ȷ���ͣ
        {
            readport_plc.OnPortReadData = router_send_afn_12_F2;
            portContext->OnPortReadData = router_send_afn_12_F2;
            portContext->cur_plc_task = PLC_TASK_CLERA_AREA_DIS;
            portContext->cur_plc_task_step = PLC_CLEAR_AREA_DIS_PAUSE_12HF2;
        }
        else
        {
            readport_plc.OnPortReadData = router_send_afn_F0_F14;
            portContext->OnPortReadData = router_send_afn_F0_F14;
            portContext->cur_plc_task = PLC_TASK_CLERA_AREA_DIS;
            portContext->cur_plc_task_step = PLC_CLEAR_AREA_DIS_FRAME_SEND;

        }
        return TRUE;
    }
    return FALSE;
}
BOOLEAN plc_router_reg_node_save_document(INT8U phase,INT8U relay,INT8U *meter_no,INT8U node_prop,INT8U *rtu_no,INT8U rtu_type,INT8U* erc35_buffer)
{
    BOOLEAN set_meter_doc(METER_DOCUMENT *meter_doc,BOOLEAN is_transctrl,BOOLEAN is_F210);
    INT16U meter_idx = 0;
	INT16U meter_idx_rtu = 0;
	INT16U save_pos,save_pos_rtu,spot_idx;
    INT8U router_protocol,router_protocol_rtu;
    INT8U node_no[6]={0};
    METER_DOCUMENT meter_doc;
    BOOLEAN isexsit,is_modify;
	#ifdef __PROVICE_ZHEJIANG__
	INT16U pos_event;
	INT16U f150;
    INT8U event_record[EVENT_RECORD_SIZE]={0};//,read_datetime[5];
    #endif
	
    #ifdef __PROVICE_JIANGSU__
    if(rtu_no != NULL)
    {
        rtu_no[5] = 0;      //���ղɼ�����ַΪ10λ����λ��0
    }
    #endif

    //������ַ�Ǵ���
    isexsit = memory_fast_index_find_node_no(READPORT_PLC,meter_no,&meter_idx,&save_pos,&router_protocol,NULL);
    if(isexsit)
    {
        if((meter_idx & FAST_IDX_RTU_FLAG) && (meter_idx & FAST_IDX_MASK) == 0) //�ɼ���
        {
            isexsit = FALSE;
        }
    }

    //�ɼ�����ַ��F10���Ѿ����ڣ��ٱ������ĿյĲɼ���������ˣ�Ҳ��������erc35,Ҳ����ӵ��̱��ļ���
    if ((rtu_no != NULL) && (FALSE == isexsit))
    {
        if ((compare_string(meter_no,rtu_no,5) == 0) && (meter_no[5] > 0x99))
        {
            if (memory_fast_index_find_node_no(READPORT_PLC,rtu_no,&meter_idx_rtu,&save_pos_rtu,&router_protocol_rtu,NULL))
            {
                if (meter_idx_rtu & FAST_IDX_RTU_FLAG) return TRUE;
            }
        }
    }

    //�ж��Ƿ�Ҫ����δ֪����¼���
    if (!isexsit)
    {
        if ((rtu_no != NULL) && (compare_string(meter_no,rtu_no,5) == 0) && (meter_no[5] > 0x99))
        {
            //����erc35   �ɼ�����ַ
            event_unconfirm_meter(phase,rtu_no,node_prop,erc35_buffer);
        }
        else
        {
            //����erc35  ����ַ
            event_unconfirm_meter(phase,meter_no,node_prop,erc35_buffer);
        }
    }

    ////F10���Ѿ����ڵĵ������ӣ�������Լ�Ͳɼ�����ַ�Ƿ�Ҫ����
    if (isexsit)
    {
        is_modify = FALSE;
        meter_idx &= FAST_IDX_MASK;
        fread_meter_params(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));

        //���һ�µ���Լ
        if((node_prop == 1) || (node_prop == 2))
        {
            if(meter_doc.protocol != ((node_prop==2) ? GB645_2007 : GB645_1997))
            {
                //�ǽ��չ�Լ��97����Ҫ�޸ĵ���Լ
                if (!(((meter_doc.protocol == GB645_1997_JINANGSU_4FL) || (meter_doc.protocol == GB645_1997_JINANGSU_2FL)) && (node_prop == 1)))
                {
                    meter_doc.protocol = (node_prop==2) ? GB645_2007 : GB645_1997;
                    meter_doc.baud_port.baud = (node_prop==2) ? 3 : 2;
                    is_modify = TRUE;
                }
            }
        }
        if(node_prop == 3)
        {
            meter_doc.protocol = GB_OOP;
            meter_doc.baud_port.baud = 3;
            is_modify = TRUE;
        }
        //���ɼ�����ַ
        if (rtu_no != NULL)
        {
            if(compare_string(meter_doc.rtu_no,rtu_no,6) != 0)
            {
                mem_cpy(meter_doc.rtu_no,rtu_no,6);
                is_modify = TRUE;
            }
        }

        if (is_modify)
        {
            set_meter_doc(&meter_doc,FALSE,FALSE);

           #ifdef __PROVICE_ZHEJIANG__
            //����F150
            //����erc3 ��F150�¼�
            spot_idx  = bin2_int16u(meter_doc.spot_idx);
             //���²������spot_idx->���meter_idxӳ��ͼ
            meter_spot_2_meter_seq_map[spot_idx] = meter_idx;

            //����F150��������״̬
            f150 = meter_spot_2_meter_seq_map[spot_idx];

            //�㽭����ʹ��
                    f150 |= 0x8000;
                    f150 |= 0x4000;
           //     f150 &= ~0x8000;
            //    f150 |= 0x4000;

            meter_spot_2_meter_seq_map[spot_idx] = f150;

            //����ERC3�¼�
             event_record[EVENT_POS_ERC_LEN]=14;
             event_record[EVENT_POS_ERC_CONTENT]= 1;  //����վ��ַ
             pos_event = EVENT_POS_ERC_CONTENT+1;

            //��ʾҪ����erc3��F112�¼��� ���������ʱ����
            event_record[pos_event++] = DA_P0;
            event_record[pos_event++] = DA_P0>>8;
            event_record[pos_event++] = DT_F150 & 0xFF;
            event_record[pos_event++] = DT_F150>>8;
            event_record[pos_event++] = DA_P0;
            event_record[pos_event++] = DA_P0>>8;
            event_record[pos_event++] = DT_F10 & 0xFF;
            event_record[pos_event++] = DT_F10>>8;

            event_params_set(event_record);
            #endif
        }
    }
    else //��ӵ���
    {
        //��������Ϣ
        mem_set(&meter_doc,sizeof(METER_DOCUMENT),0xFF);

        //��Ҫ�õ�һ��������źͲ������
        //����Ҫ���ǲɼ���ռ�õ����,�����Ҫһ�������ı���������ڵ�����
        //read_fmArray(FMADDR_PLC_NODE_COUNT,&save_pos,sizeof(INT16U));
        //�õ�һ���������:�Ӳ�����11��ʼ�ҿյĲ�����
        #ifdef __PROVICE_NEIMENG__
        for(meter_idx = 65;meter_idx<=MAX_METER_COUNT;meter_idx++)
        {
            if(!file_exist(meter_idx)) break;
        }
        #else
        #if defined (__PROVICE_JIANGXI__)
        for(meter_idx = 17;meter_idx<=MAX_METER_COUNT;meter_idx++)
        {
            if(!file_exist(meter_idx)) break;
        }
        #else
        for(meter_idx = 11;meter_idx<=MAX_METER_COUNT;meter_idx++)
        {
            if(!file_exist(meter_idx)) break;
        }
        #endif
        #endif
        if(meter_idx > MAX_METER_COUNT)
        {
            #if defined (__DELETE_30_DAY_NO_REC_SPOT__)
            meter_idx = update_invalid_meterdoc2(TRUE);
            if(meter_idx == 0)
            #endif
            //������ӵ��
            return isexsit;
        }

        //����Ƿ��пյĲɼ������еĻ������ǿղɼ���
        if(rtu_no != NULL)
        {
            mem_cpy(node_no,rtu_no,6);
            node_no[5] |= 0xA0;
            if (memory_fast_index_find_node_no(READPORT_PLC,node_no,&meter_idx_rtu,&save_pos_rtu,&router_protocol_rtu,NULL))
            {
                meter_idx = meter_idx_rtu & FAST_IDX_MASK;
            }
        }

        //��д�����ṹ��
        spot_idx = meter_idx;
        meter_doc.meter_idx[0] = meter_idx;
        meter_doc.meter_idx[1] = meter_idx>>8;
        meter_doc.spot_idx[0] = spot_idx;
        meter_doc.spot_idx[1] = spot_idx>>8;
        meter_doc.baud_port.port = COMMPORT_PLC;
        if(node_prop == 3)
        {
            meter_doc.baud_port.baud = 3;
            meter_doc.protocol = GB_OOP;
        }
        else
        {
            meter_doc.baud_port.baud = (node_prop==2) ? 3 : 2;
            meter_doc.protocol = (node_prop==2) ?  GB645_2007 : GB645_1997;
        }
        if(rtu_no == NULL)
        {
            mem_set(meter_doc.rtu_no,6,0x00);
        }
        else
        {
            mem_cpy(meter_doc.rtu_no,rtu_no,6);
        }
        mem_cpy(meter_doc.meter_no,meter_no,6);
        meter_doc.fl_count = (node_prop==2) ?  0 : 4;   //97��ķ�������дΪ4
        meter_doc.meter_class.value = 0;

        set_meter_doc(&meter_doc,FALSE,FALSE);
        #ifdef __PROVICE_ZHEJIANG__
            //����F150
            //����erc3 ��F150�¼�
            spot_idx  = bin2_int16u(meter_doc.spot_idx);
             //���²������spot_idx->���meter_idxӳ��ͼ
            meter_spot_2_meter_seq_map[spot_idx] = meter_idx;

            //����F150��������״̬
            f150 = meter_spot_2_meter_seq_map[spot_idx];

            //�㽭����ʹ��
            f150 |= 0x8000;
            f150 |= 0x4000;
           //     f150 &= ~0x8000;
           //     f150 |= 0x4000;

            meter_spot_2_meter_seq_map[spot_idx] = f150;

            //����ERC3�¼�
             event_record[EVENT_POS_ERC_LEN]=14;
             event_record[EVENT_POS_ERC_CONTENT]= 1;  //����վ��ַ
             pos_event = EVENT_POS_ERC_CONTENT+1;

            //��ʾҪ����erc3��F112�¼��� ���������ʱ����
            event_record[pos_event++] = DA_P0;
            event_record[pos_event++] = DA_P0>>8;
            event_record[pos_event++] = DT_F150 & 0xFF;
            event_record[pos_event++] = DT_F150>>8;
            event_record[pos_event++] = DA_P0;
            event_record[pos_event++] = DA_P0>>8;
            event_record[pos_event++] = DT_F10 & 0xFF;
            event_record[pos_event++] = DT_F10>>8;

            event_params_set(event_record);
            #endif

    }
    return isexsit;
}

void plc_router_reg_node_save_file(INT8U phase,INT8U *meter_no,INT8U node_prop,INT8U *rtu_no,INT8U file_idx,INT8U *stamp,INT8U* erc35_buffer)
{
    PLC_NET_METER plc_net_meter;
    INT16U count = 0;
    INT16U meter_idx = 0;
    #if (defined __PROVICE_JIANGSU__)
    INT16U meter_idx_tmp1=0;
    #endif
    INT16U save_pos;//meter_idx_rtu,,save_pos_rtu,spot_idx;
    INT8U router_protocol;//,router_protocol_rtu;
    BOOLEAN isexsit;
    #ifdef __PROVICE_JIANGSU__
    METER_DOCUMENT meter_doc;
    extern void save_meter_info(METER_DOCUMENT *meter_doc);
    INT8U need_insert_rtu_no_flag;
    #endif

    #ifdef __PROVICE_JIANGSU__
    if(rtu_no != NULL)
    {
        rtu_no[5] = 0;      //���ղɼ�����ַΪ10λ����λ��0
    }
    if(memory_fast_index_find_node_no(READPORT_PLC,rtu_no,&meter_idx,&save_pos,&router_protocol,NULL))
    {
        if(meter_idx & FAST_IDX_RTU_FLAG) /*����ǲɼ��������Ը��£�������ǣ��Ͳ�Ҫ������ */
        {
           need_insert_rtu_no_flag = TRUE;
        }
        else
        {
           need_insert_rtu_no_flag = FALSE;
        }
    }
    else
    {
       need_insert_rtu_no_flag = TRUE; /*������û�У�˵���������*/
    }
    #endif

    //������ַ�Ǵ���
    isexsit = memory_fast_index_find_node_no(READPORT_PLC,meter_no,&meter_idx,&save_pos,&router_protocol,NULL);
    #if (defined __PROVICE_JIANGSU__)
    meter_idx_tmp1 = meter_idx & FAST_IDX_MASK;
    fread_meter_params(meter_idx_tmp1,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
    #endif
    if(isexsit)
    {
        #if defined __PROVICE_JIANGSU__
        if(need_insert_rtu_no_flag)
        {
            if((compare_string(meter_doc.rtu_no,rtu_no,6) != 0) && (compare_string(meter_doc.meter_no,rtu_no,6) != 0)) /*����ɼ����͵���ַһ������Ҫ�ӵ���*/
            {
               mem_cpy(meter_doc.rtu_no,rtu_no,6);
               save_meter_info(&meter_doc);
            }
        }
        #endif
        if((meter_idx & FAST_IDX_RTU_FLAG) && (meter_idx & FAST_IDX_MASK) == 0) //�ɼ���
        {
            isexsit = FALSE;
        }
    }

    //�յĲɼ����������
    if ((rtu_no != NULL) && (FALSE == isexsit))
    {
        if ((compare_string(meter_no,rtu_no,5) == 0) && (meter_no[5] > 0x99)) return;
    }

    //�ж��Ƿ�Ҫ����δ֪����¼���
    if (!isexsit)
    {
        if ((rtu_no != NULL) && (compare_string(meter_no,rtu_no,5) == 0) && (meter_no[5] > 0x99))
        {
            //����erc35   �ɼ�����ַ
            event_unconfirm_meter(phase,rtu_no,node_prop,erc35_buffer);
        }
        else
        {
            //����erc35  ����ַ
            event_unconfirm_meter(phase,meter_no,node_prop,erc35_buffer);

            #if (defined __PROVICE_JIANGSU__)
         //   fread_meter_params(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
            if(meter_doc.baud_port.port != READPORT_PLC)
            {
              event_cjq_erc53(meter_idx_tmp1,meter_doc.baud_port.port, READPORT_PLC,rtu_no);
            }
            #endif
        }
    }

    mem_set(plc_net_meter.value,sizeof(PLC_NET_METER),0x00);
    mem_cpy(plc_net_meter.meter_no,meter_no,6);

    switch(node_prop)
    {
        case 2:
             plc_net_meter.protocol = GB645_2007;
             break;
        case 3:
             plc_net_meter.protocol = GB_OOP;
             break;
        default:
             plc_net_meter.protocol = GB645_1997;
             break;
    }
  //  plc_net_meter.protocol = (node_prop==2) ?  GB645_2007 : GB645_1997;
    plc_net_meter.phase = phase;
    if(rtu_no != NULL)
    {
        mem_cpy(plc_net_meter.rtu_no,rtu_no,6);
    }

  //  mem_cpy(plc_net_meter.stamp,stamp,4); /*�Ҳ����ô����ĳɴ�ʱ���*/
    mem_cpy(plc_net_meter.stamp,datetime+MINUTE,4);

    fread_array(FILEID_PLC_NET+file_idx,0,(INT8U*)&count,2);
    if(count > MAX_METER_COUNT)  count = 0;
    fwrite_array(FILEID_PLC_NET+file_idx,2+sizeof(PLC_NET_METER)*count,plc_net_meter.value,sizeof(PLC_NET_METER));
    count++;
    fwrite_array(FILEID_PLC_NET+file_idx,0,(INT8U*)&count,2);
}

//erc35_buffer�Ĳ���С��135���ֽڣ�����ʹ���ز�portContext->plc_net_buffer
void plc_router_reg_node(INT8U phase,INT8U relay,INT8U *meter_no,INT8U node_prop,INT8U *rtu_no,INT8U rtu_type,INT8U save_mode,INT8U file_idx,INT8U *stamp,INT8U* erc35_buffer)
{
    #ifdef __SOFT_SIMULATOR__
    snprintf(info,100,"plc_router_reg_node: %02X%02X%02X%02X%02X%02X %02X phase:%02d depth:%02d",
        meter_no[5],meter_no[4],meter_no[3],meter_no[2],meter_no[1],meter_no[0],node_prop,phase,relay);
    debug_println_ext(info);

    if(rtu_no != NULL)
    {
        snprintf(info,100,"rtu_no: %02X%02X%02X%02X%02X%02X",
            rtu_no[5],rtu_no[4],rtu_no[3],rtu_no[2],rtu_no[1],rtu_no[0]);
        debug_println_ext(info);
    }
    #endif


    if(portContext_plc.cur_plc_task == PLC_TASK_PLC_NET)  
    {
        if(save_mode)
        {
            plc_router_reg_node_save_file(phase,meter_no,node_prop,rtu_no,file_idx,stamp,erc35_buffer);
        }
        else
        {          
            plc_router_reg_node_save_document(phase,relay,meter_no,node_prop,rtu_no,rtu_type,erc35_buffer);
        }
    }
    else
    {
        /*����������������������ϱ����洢�������ļ��У����ӵ���*/
        plc_router_roam_reg_node_save_file(phase,meter_no,node_prop,rtu_no,erc35_buffer);
        plc_router_reg_node_save_file(phase,meter_no,node_prop,rtu_no,0,stamp,erc35_buffer);
    }

}

INT8U start_cast_cjq_485_search_meter(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;

    //���÷��ͻ��������ݱ���
    portContext->cast_content = portContext->frame_cast_buffer; // portContext->frame_send

    portContext->cast_content_len = make_gb645_cjq_485_frame(portContext->cast_content,60); //�㲥�ɼ�������485���

    portContext->plc_net_wait_time_s = portContext->plc_net_cast_485_time_s;

    readport_plc.OnPortReadData = router_send_afn_05_F3;
    portContext->OnPortReadData = router_send_afn_05_F3;
    portContext->cur_plc_task_step = PLC_CAST_OPEN_05_F3;
    return 0;
}

INT8U wait_cast_exec_end(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;

    if ((portContext->cast_wait_time_s & 0x8000) == 0)
    {
        if (time_elapsed_10ms(portContext->cast_timer_10ms) >= portContext->cast_wait_time_s*100)
        {
            //����·�����õĹ㲥�������ʱʱ�䣨��λS���ȴ���������100s�ȴ�
            if (bin2_int16u(portContext->router_base_info.max_cast_task_timeout_s) > portContext->cast_wait_time_s)
            {
                portContext->cast_wait_time_s = bin2_int16u(portContext->router_base_info.max_cast_task_timeout_s) - portContext->cast_wait_time_s;
                portContext->cast_wait_time_s |= 0x8000;
            }
            else
            {
                portContext->cast_wait_time_s = 100;
                portContext->cast_wait_time_s |= 0x8000;
            }

            readport_plc.OnPortReadData = router_send_afn_10_F4;
            //portContext->OnPortReadData = router_send_afn_10_F4;
            //portContext->cur_plc_task_step = PLC_CAST_WAIT_EXEC_END;
            portContext->cast_timer_10ms = os_get_systick_10ms();
            portContext->cast_time_out_10ms = os_get_systick_10ms();
        }
    }
    else
    {
        portContext->cast_wait_time_s &= ~0x8000;
        if (time_elapsed_10ms(portContext->cast_time_out_10ms) > portContext->cast_wait_time_s*100)
        {

            //�����㲥����
            if (portContext->cur_plc_task == PLC_TASK_PLC_NET)
            {
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** �ȴ���ʱ���㲥���������***");
                debug_println_ext(info);
                #endif
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext->OnPortReadData = router_check_urgent_timeout;
                portContext->cur_plc_task_step = PLC_NET_WAIT_CJQ_SEARCH_METER;
                portContext->plc_net_time_out_10ms = os_get_systick_10ms();
            }
            else if((portContext->urgent_task_id == RECMETER_TASK_TRANS_CAST  )
            ||  (portContext->urgent_task_id == RECMETER_TASK_TRANS_CAST_TIME  )
            || (portContext->urgent_task_id == RECMETER_TASK_TRANS_CAST_TIME_FROM_STATION )
            || (portContext->urgent_task_id == RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE )
            || (portContext->urgent_task_id == RECMETER_TASK_TRANS_CAST_PRECISE_TIME ))
            {
                    //�л��ؽ������񣬹㲥Уʱ�����˳�
                if((portContext->urgent_task_step == PLC_CAST_WAIT_EXEC_END) && (portContext->urgent_task_id == RECMETER_TASK_TRANS_CAST_TIME )) /*����˵���ն��Լ������645�㲥����ˣ�������Ҫ��OOP�Ķ�ʱ*/
                {
                    portContext->cast_content = portContext->frame_cast_buffer; // portContext->frame_send
                  //  #ifdef __READ_OOP_METER__
                    portContext->cast_content_len =  make_oop_adj_time_frame(portContext->cast_content, 0);
                  //  #endif
                    readport_plc.OnPortReadData = router_send_afn_03_F9;
                    portContext->urgent_task = PLC_TASK_URGENT_TASK;
                    portContext->urgent_task_step = PLC_CAST_OOP_DELAY_03_F9;
                }
                else
                {
                    urgent_task_in_wait_next_urgent_task(portContext);
                }
            }
            else
            {
                //����ǵ�ǰ�����ڷ��㲥�أ�
                urgent_task_in_wait_next_urgent_task(portContext);
            }
        }
        else if (time_elapsed_10ms(portContext->cast_timer_10ms) >= 10*100)
        {
            readport_plc.OnPortReadData = router_send_afn_10_F4;
            //portContext->OnPortReadData = router_send_afn_10_F4;
            //portContext->cur_plc_task_step = PLC_CAST_WAIT_EXEC_END;
            portContext->cast_timer_10ms = os_get_systick_10ms();
        }
        portContext->cast_wait_time_s |= 0x8000;
    }
    return 0;
}

BOOLEAN check_exsit_buffer(INT8U* buffer,INT8U* num)
{
    INT8U idx;

    for(idx=0;idx<buffer[0];idx++)
    {
        if (compare_string(buffer+1+idx*2,num,2) == 0) return TRUE;
    }
    return FALSE;
}
#ifdef __PROVICE_CHONGQING__
void plc_net_state_chongqing(objReadPortContext * readportcontext)
{
	INT32U 	offset_new;//���������Ϣ
	INT32U	offset_fail;//δ�������ĵ����Ϣ
	PLCPortContext *portContext;
    PLC_NET_METER *plc_net_meter; // ռ�ݻ�������18���ֽ� 
	INT8U 	*meter_status = NULL; //  ռ�ݻ����� 250���ֽ� ƫ���Ǵ�18��ʼ�ģ��������ܹ�298  ��������Ҫ��
    INT16U 	idx,count_z_new;
	INT16U 	meter_count,node_count;
	INT16U	fail_cnt;
	INT16U 	meter_idx = 0;
    INT16U 	save_pos = 0;
    INT8U 	router_protocol;
	INT8U 	pos,pos_bit;
	INT8U temp_buffer[READPORT_PLC_FRAME_SIZE] = {0};
    //INT8U is_find;//max_read_count;

    portContext = (PLCPortContext*)readportcontext;
    //count_z = 0;
    //is_find = FALSE;
    //max_read_count = READPORT_PLC_FRAME_SIZE/sizeof(PLC_NET_METER);

	// get total count
	fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,0,(INT8U*)&count_z_new,2);
    if( (count_z_new > MAX_METER_COUNT) || (count_z_new == 0) ) 
    {
		return;
    }

	plc_net_meter = (PLC_NET_METER*)(temp_buffer);
	meter_status =   (INT8U *) (temp_buffer + sizeof(PLC_NET_METER) );
	mem_set(meter_status,250,0x00);//
	meter_count = 0;
	offset_new = 2+count_z_new*sizeof(PLC_NET_METER);
	for(idx=0;idx<count_z_new;idx++)
	{		
        fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,2+idx*sizeof(PLC_NET_METER),plc_net_meter->value,sizeof(PLC_NET_METER));
		if( TRUE == memory_fast_index_find_node_no(READPORT_PLC,plc_net_meter->meter_no,&meter_idx,&save_pos,&router_protocol,NULL))
		{
			meter_idx &= FAST_IDX_MASK;
			if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
        	{
        		pos = (meter_idx-1) / 8;
            	pos_bit = (meter_idx-1) % 8;
				meter_status[pos] |= (0x01<<pos_bit);				
			}
		}
		else
		{
			//�������
			fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,offset_new+2+meter_count*6,plc_net_meter->meter_no,6);
			meter_count++;
		}
	}
	fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,offset_new,(INT8U*)&meter_count,2);

	offset_fail = offset_new+2+6*meter_count;//δ���ѵ��ĵ���ַ

	tpos_mutexPend(&SIGNAL_FAST_IDX);
    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
    fail_cnt = 0;
    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > READPORT_PLC) break;
        if (READPORT_PLC != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

		pos = (meter_idx-1) / 8;
        pos_bit = (meter_idx-1) % 8;		
		if( (meter_status[pos] & (0x01<<pos_bit)) == 0x00 )
		{
			fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,offset_fail+2+fail_cnt*6,fast_index_list.fast_index[idx].node,6);
        	fail_cnt ++;
		}
    }
	fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,offset_fail,(INT8U*)&fail_cnt,2);
    tpos_mutexFree(&SIGNAL_FAST_IDX);
}
#endif
#ifdef __PROVICE_JIANGSU__
 void plc_net_state_jiangsu(objReadPortContext * readportcontext)
{

	PLCPortContext *portContext;
    PLC_NET_METER *plc_net_meter_old; //
    PLC_NET_METER *plc_net_meter_new; //

        INT16U 	count_z_old,count_z_new;
	INT8U 	old_file_idx,new_file_idx;
        INT8U   temp_buffer_old[30];
        INT8U   idx,idx1,idx2,find_result,need_read_time,cjq_info_num;

    portContext = (PLCPortContext*)readportcontext;

    old_file_idx = ((~portContext->params.task_plc_net.run_params.file_idx) & 0x01);
    new_file_idx = portContext->params.task_plc_net.run_params.file_idx;

  //  file_copy(FILEID_CJQ_SEARCH_NEW,FILEID_PLC_NET+new_file_idx);
  //  file_copy(FILEID_CJQ_SEARCH_NEW,FILEID_PLC_NET+new_file_idx);

	fread_array(FILEID_PLC_NET+new_file_idx,0,(INT8U*)&count_z_new,2);
        fread_array(FILEID_PLC_NET+old_file_idx,0,(INT8U*)&count_z_old,2);

    if( (count_z_new > MAX_METER_COUNT) || (count_z_new == 0) )
    {
		return;
    }

    if( (count_z_old > MAX_METER_COUNT) || (count_z_old == 0) )
    {
      //����һ�����ļ���
      //   file_copy();
		return;
    }

        plc_net_meter_old  = (PLC_NET_METER*)(temp_buffer_old);

        tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
	for(idx=0;idx<count_z_old;idx++) //�����ȡ�������ļ��������Ȼ�������ļ��ıȶ�
	{		
          fread_array(FILEID_PLC_NET+old_file_idx,2+idx*sizeof(PLC_NET_METER),temp_buffer_old,sizeof(PLC_NET_METER));   //�ó���һ�����ļ��еı��

               find_result = 0;
               cjq_info_num = (MAX_PAGE_SIZE-2)/sizeof(PLC_NET_METER); //һ��g_temp_buffer�����������ٸ�

              need_read_time = count_z_new/cjq_info_num ;//��Ҫ���ٸ�ѭ������ȫ����ѯһ�飬
              need_read_time ++ ;


              for(idx1=0;idx1<need_read_time;idx1++)    //������ѯһ��
              {

                fread_array(FILEID_PLC_NET+old_file_idx,2+idx1*cjq_info_num*sizeof(PLC_NET_METER),g_temp_buffer,(cjq_info_num*sizeof(PLC_NET_METER)));
                plc_net_meter_new = (PLC_NET_METER*)(g_temp_buffer);
                for(idx2=0;idx2<cjq_info_num;idx2++)          //��ǰbuffer�������ѯһ��
                {
                  if(compare_string(plc_net_meter_old->rtu_no, plc_net_meter_new->rtu_no,6) == 0)
                  {
                    find_result = 1;
                    break ;
                  }
                  else
                  plc_net_meter_new ++;
                }

                if(find_result)break;
              }

              if(find_result == 0)//û�ҵ�
              {
                event_cjq_erc54(plc_net_meter_old->rtu_no);
              }


	}
        tpos_mutexFree(&SIGNAL_TEMP_BUFFER);

       //��ɺ���Ҫ����һ���ļ�
       
     //	fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,offset_new,(INT8U*)&meter_count,2);

     //	offset_fail = offset_new+2+6*meter_count;//δ���ѵ��ĵ���ַ

}
#endif
//�Ѿɵ��ļ��в��ظ������ݰ��ƹ���
INT8U plc_net_finish_tail_work_step_3(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    PLC_NET_METER *plc_net_meter;
    INT16U count_z,idx,idx_min,count_z_new;
    INT8U old_file_idx,max_read_count;//is_find,
    INT8U temp_buffer[READPORT_PLC_FRAME_SIZE] = {0};

    portContext = (PLCPortContext*)readportcontext;
    count_z = 0;
    //is_find = FALSE;
	count_z_new = 0;
    max_read_count = READPORT_PLC_FRAME_SIZE/sizeof(PLC_NET_METER);

    old_file_idx = ((~portContext->params.task_plc_net.run_params.file_idx) & 0x01);
    idx_min = bin2_int16u(portContext->params.task_plc_net.last_minute);
    
    fread_array(FILEID_PLC_NET+old_file_idx,0,(INT8U*)&count_z,2);
    if(count_z > MAX_METER_COUNT) count_z = 0;

    fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,0,(INT8U*)&count_z_new,2);
    if(count_z_new > MAX_METER_COUNT) count_z_new = 0;


    if ((idx_min < count_z) && (count_z_new > 0))
    {
        //��ȡ���
        fread_array(FILEID_PLC_NET+old_file_idx,2+idx_min*sizeof(PLC_NET_METER),portContext->params.task_plc_net.start_time,6);
        plc_net_meter = (PLC_NET_METER*)(temp_buffer);
        for(idx=0;idx<count_z_new;idx++)
        {
            if ((idx % max_read_count) == 0)
            {
                plc_net_meter = (PLC_NET_METER*)(temp_buffer);
                fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,2+idx*sizeof(PLC_NET_METER),plc_net_meter->value,sizeof(PLC_NET_METER)*max_read_count);
            }

            if (compare_string(portContext->params.task_plc_net.start_time,plc_net_meter->meter_no,6) == 0)
            {
                //�ҵ��ˣ�����һ��
                idx_min++;
                int16u2_bin(idx_min,portContext->params.task_plc_net.last_minute);
                
                return 0;
            }
            plc_net_meter++;
        }

        //û�ҵ�����ӵ����ļ�����
        plc_net_meter = (PLC_NET_METER*)(temp_buffer);
        fread_array(FILEID_PLC_NET+old_file_idx,2+idx_min*sizeof(PLC_NET_METER),plc_net_meter->value,sizeof(PLC_NET_METER));
        #ifdef __PLC_NET_JIANGSU__
        if (bin2_int32u(portContext->params.task_plc_net.stamp) - bin2_int32u(plc_net_meter->stamp) < 5)
        #endif
        {
            fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,2+count_z_new*sizeof(PLC_NET_METER),plc_net_meter->value,sizeof(PLC_NET_METER));
            count_z_new++;
            fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,0,(INT8U*)&count_z_new,2);
        }
        //����һ��
        idx_min++;
        int16u2_bin(idx_min,portContext->params.task_plc_net.last_minute);
    }
    else
    {

        #ifdef __PROVICE_JIANGSU__
        plc_net_state_jiangsu(readportcontext);
        #endif

        if (count_z_new == 0)
        {
            file_delete(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx);
        }
        else
        {
            file_delete(FILEID_PLC_NET+old_file_idx);
        }

		#ifdef __PROVICE_CHONGQING__//��������Ҫ������Ϣ�������������ʾ
		plc_net_state_chongqing(readportcontext);
		#endif

		#ifdef __PROVICE_JIANGXI__
        plc_net_state_jiangxi(readportcontext);
        #endif

        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** �ѱ�����ȫ��������***");
        debug_println_ext(info);
        #endif

        //�����ѱ�����
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = 0;
        portContext->OnPortReadData = router_check_urgent_timeout;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext_plc.read_status.plc_net = 0;
        #ifdef __PLC_BPLC_AGG__
        if( 1 == portContext->aggregate_auth )
        {
            portContext->aggregate_flg = 1;
        }
        #endif
        
    }
   
    return 0;
}

INT8U plc_net_finish_tail_work_step_2(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    PLC_NET_METER *plc_net_meter;
    INT16U count_z,idx_min;//idx,
    INT16S idx = 0;//�з���
    INT8U temp_buffer[READPORT_PLC_FRAME_SIZE] = {0};
    
    portContext = (PLCPortContext*)readportcontext;
    count_z = 0;

    if(portContext->params.task_plc_net.node_repeat < portContext->plc_net_buffer[0])
    {
        idx_min = bin2_int16u(portContext->plc_net_buffer+1+portContext->params.task_plc_net.node_repeat*2);
        fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,0,(INT8U*)&count_z,2);
        if(count_z > MAX_METER_COUNT) count_z = 0;
        if(idx_min < count_z)
        {
            plc_net_meter = (PLC_NET_METER*)(temp_buffer);

            for(idx=(count_z-1);idx>=0;idx--)
            {
                if (check_exsit_buffer(portContext->plc_net_buffer,(INT8U*)&idx) == FALSE)
                {
                    //�ҵ���
                    fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,2+idx*sizeof(PLC_NET_METER),plc_net_meter->value,sizeof(PLC_NET_METER));
                    fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,2+idx_min*sizeof(PLC_NET_METER),plc_net_meter->value,sizeof(PLC_NET_METER));
                    count_z--;

                    break;
                }
                else
                {
                    count_z--;
                    if(idx == idx_min) break;
                }
            }

            fwrite_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,0,(INT8U*)&count_z,2);

            portContext->params.task_plc_net.node_repeat++;

            return 0;
        }
    }

    if(portContext->router_base_info.router_info3.plc_net_clear_file)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** �ѱ�����ȫ��������***");
        debug_println_ext(info);
        #endif

        //�����ѱ�����
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = 0;
        portContext->OnPortReadData = router_check_urgent_timeout;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext_plc.read_status.plc_net = 0;
    }
    else
    {
        portContext->params.task_plc_net.last_minute[0] = 0;
        portContext->params.task_plc_net.last_minute[1] = 0;
        portContext->OnPortReadData = plc_net_finish_tail_work_step_3;
        readport_plc.OnPortReadData = plc_net_finish_tail_work_step_3;
    }
    return 0;
}

INT8U plc_net_finish_tail_work_step_1(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    PLC_NET_METER *plc_net_meter;
    INT16U meter_seq,count_z,idx,idx1,min_num,idx_min;
    INT8U max_read_count;
    INT8U temp_buffer[READPORT_PLC_FRAME_SIZE] = {0};

    portContext = (PLCPortContext*)readportcontext;
    count_z = 0;
    max_read_count = READPORT_PLC_FRAME_SIZE/sizeof(PLC_NET_METER);

    //��¼���صĵ�ǰidx 2�ֽ�
    meter_seq = bin2_int16u(portContext->params.task_plc_net.last_minute);
    fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,0,(INT8U*)&count_z,2);
    if(count_z > MAX_METER_COUNT) count_z = 0;
    if(meter_seq >= count_z)
    {
        //���ظ������ݣ���Ҫ����
        if(portContext->plc_net_buffer[0] > 0)
        {
            //��Ҫ����
            for(idx=0;idx<portContext->plc_net_buffer[0];idx++)
            {
                min_num = 0xFFFF;
                idx_min = 0xFFFF;
                for(idx1=idx;idx1<portContext->plc_net_buffer[0];idx1++)
                {
                    meter_seq = bin2_int16u(portContext->plc_net_buffer+1+idx1*2);
                    if(min_num > meter_seq)
                    {
                        min_num = meter_seq;
                        idx_min = idx1;
                    }
                }
                if ((idx_min < portContext->plc_net_buffer[0]) && (idx_min != idx))
                {
                    //����λ��
                    mem_cpy(portContext->plc_net_buffer+1+idx_min*2,portContext->plc_net_buffer+1+idx*2,2);
                    mem_cpy(portContext->plc_net_buffer+1+idx*2,(INT8U*)&min_num,2);
                }
            }

            portContext->params.task_plc_net.node_repeat = 0;
            portContext->OnPortReadData = plc_net_finish_tail_work_step_2;
            readport_plc.OnPortReadData = plc_net_finish_tail_work_step_2;
            return 0;
        }
        else
        {
            if(portContext->router_base_info.router_info3.plc_net_clear_file)
            {
                #ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** �ѱ�����ȫ��������***");
                debug_println_ext(info);
                #endif

                //�����ѱ�����
                portContext->cur_plc_task = PLC_TASK_IDLE;
                portContext->cur_plc_task_step = 0;
                portContext->OnPortReadData = router_check_urgent_timeout;
                readport_plc.OnPortReadData = router_check_urgent_timeout;
                portContext_plc.read_status.plc_net = 0;
            }
            else
            {
                portContext->params.task_plc_net.last_minute[0] = 0;
                portContext->params.task_plc_net.last_minute[1] = 0;
                portContext->OnPortReadData = plc_net_finish_tail_work_step_3;
                readport_plc.OnPortReadData = plc_net_finish_tail_work_step_3;
            }
            return 0;
        }
    }
    //��ȡ���
    fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,2+meter_seq*sizeof(PLC_NET_METER),portContext->params.task_plc_net.start_time,6);
    
    plc_net_meter = (PLC_NET_METER*)(temp_buffer);
    for(idx=0;idx<count_z;idx++)
    {
        if(idx >= meter_seq)
        {
            //�Լ���Ҫ�Ƚ��ظ����˳�����ʼ��һ��
            meter_seq++;
            int16u2_bin(meter_seq,portContext->params.task_plc_net.last_minute);
            return 0;
        }

        if ((idx % max_read_count) == 0)
        {
            plc_net_meter = (PLC_NET_METER*)(temp_buffer);
            fread_array(FILEID_PLC_NET+portContext->params.task_plc_net.run_params.file_idx,2+idx*sizeof(PLC_NET_METER),plc_net_meter->value,sizeof(PLC_NET_METER)*max_read_count);
        }

        if (compare_string(plc_net_meter->meter_no,portContext->params.task_plc_net.start_time,6) == 0)
        {
            //�ظ���Ҫ��¼����
            if (check_exsit_buffer(portContext->plc_net_buffer,(INT8U*)&idx) == FALSE)
            {
               // if ((portContext->plc_net_buffer[0]+1)*2+1 < sizeof(ROUTER_PHASE_WORK_INFO)*3) /*����ĸ�ֵ�����511���˴��ж�û������*/
                {
                    mem_cpy(portContext->plc_net_buffer+portContext->plc_net_buffer[0]*2+1,(INT8U*)&idx,2);
                    portContext->plc_net_buffer[0]++;
                }
            }
        }
        plc_net_meter++;
    }
    return 0;
}

INT8U start_plc_net_stat(PLCPortContext *portContext)
{
    //����δ֪����¼�����״̬
    event_unconfirm_meter(0,NULL,0,portContext->plc_net_buffer);

    if(portContext->params.task_plc_net.run_params.save_mode)
    {
        mem_set(portContext->params.task_plc_net.start_time,8,0x00);
        mem_set(portContext->plc_net_buffer,511,0x00);
        portContext->cur_plc_task = PLC_TASK_PLC_NET;
        portContext->cur_plc_task_step = PLC_NET_STAT_DATA;
        portContext->OnPortReadData = plc_net_finish_tail_work_step_1;
        readport_plc.OnPortReadData = plc_net_finish_tail_work_step_1;
    }
    else
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** �ѱ�����ȫ��������***");
        debug_println_ext(info);
        #endif

        //�����ѱ�����
        portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = 0;
        portContext->OnPortReadData = router_check_urgent_timeout;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext_plc.read_status.plc_net = 0;

        #ifdef __PLC_BPLC_AGG__
        if( 1 == portContext->aggregate_auth )
        {
            portContext->aggregate_flg = 1;
        }
        #endif
    }
    return 0;
}

BOOLEAN exec_ctrl_cmd(PLCPortContext *portContext)
{
    BOOLEAN re_read,result;

    re_read = FALSE;
    result = FALSE;
    if (portContext->ctrl_cmd.cmd_pause)
    {
        portContext->ctrl_cmd.cmd_pause = 0;
        portContext->router_work_info.status.pause = 1;
        if(portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER) //ֻ�б����ŷ���ͣ
        {
            readport_plc.OnPortReadData = router_send_afn_12_F2;
        }
        return TRUE;
    }
    else if (portContext->ctrl_cmd.cmd_resume)
    {
        portContext->ctrl_cmd.cmd_resume = 0;
        portContext->router_work_info.status.pause = 0;
        if(portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER) //ֻ�б����ŷ��ָ�
        {
            readport_plc.OnPortReadData = router_send_afn_12_F3;
        }
        return TRUE;
    }
    else if ((portContext->ctrl_cmd.cmd_init_router) || (portContext->ctrl_cmd.cmd_delete_all_meter))
    {
        portContext->ctrl_cmd.cmd_init_router = 0;
        portContext->ctrl_cmd.cmd_delete_all_meter = 0;
        //portContext->read_status.is_in_read_cycle = 0;
        //portContext->cur_plc_task = PLC_TASK_IDLE;
        portContext->cur_plc_task_step = PLC_INIT_ROUTER_01F2;
        portContext->OnPortReadData = router_send_afn_01_F2;
        readport_plc.OnPortReadData = router_send_afn_01_F2;
        return TRUE;
    }
    #if defined(__SHANXI_READ_BPLC_NETWORK_INFO__)
    else if(portContext->ctrl_cmd.allow_area_distinguish)
    {
        portContext->ctrl_cmd.allow_area_distinguish = 0;
        portContext->urgent_task_id = BPLC_AREA_DIS_F0_F111;
        return TRUE;

    }
    else if(portContext->ctrl_cmd.forbid_area_distinguish)
    {
        portContext->ctrl_cmd.forbid_area_distinguish = 0;
        portContext->urgent_task_id = BPLC_AREA_DIS_F0_F112;
        return TRUE;
    }
    #endif
    else
    {
        if (portContext->ctrl_cmd.cmd_redo)
        {
            portContext->ctrl_cmd.cmd_redo = 0;
            portContext->router_work_info.status.pause = 0;
            /*
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_PLC);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_month,COMMPORT_PLC);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
    
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CY);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_CAS);
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_cycle_day.flag,COMMPORT_485_REC);
            */
            if ((portContext->cur_plc_task == PLC_TASK_PLC_NET) && (portContext->cur_plc_task_step >= PLC_NET_WAIT_NODE_LOGON))
            {
                portContext->params.task_plc_net.run_params.is_force_exit = 1;
            }
            else
            {
                re_read = TRUE;
            }
            result = TRUE;
        }

        if ((portContext->ctrl_cmd.batch_meter_task_redo) && (time_elapsed_10ms(portContext->batch_meter_task_redo_time_10ms) > 30*100))   //30s
        {
            portContext->ctrl_cmd.batch_meter_task_redo = 0;
            portContext->router_work_info.status.pause = 0;
            if (portContext->cur_plc_task == PLC_TASK_READ_METER)
            {
                re_read = TRUE;
            }
            result = TRUE;
        }

        #ifdef __BATCH_TRANSPARENT_METER_TASK__
        if ((portContext->batch_meter_ctrl.is_restart_read_meter) && (time_elapsed_10ms(portContext->batch_meter_time_10ms) > 60*100))   //60s
        {
            portContext->batch_meter_ctrl.is_restart_read_meter = 0;
            portContext->router_work_info.status.pause = 0;
            if (portContext->cur_plc_task == PLC_TASK_READ_METER)
            {
                re_read = TRUE;
            }
            result = TRUE;
        }
        #endif

        if (re_read)
        {
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** EXIT RECORDING TIME ***");
            debug_println_ext(info);
            #endif
            portContext->read_status.is_in_read_cycle = 0;

            portContext->cur_plc_task = PLC_TASK_IDLE;
            portContext->cur_plc_task_step = 0;
            portContext->OnPortReadData = router_check_urgent_timeout;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
        }
        return result;
    }
    return FALSE;
}

INT16U get_meter_idx_from_fast_index(INT16U node_idx,INT8U port)
{
    INT16U node_count;//,idx;
    INT16U meter_idx;

    meter_idx = 0;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    if (node_idx < node_count)
    {
        if (fast_index_list.fast_index[node_idx].port == port)
        {
            meter_idx = bin2_int16u(fast_index_list.fast_index[node_idx].seq_spec);
            meter_idx &= FAST_IDX_MASK;
        }
    }
    else
    {
        meter_idx = 0xFFFF;
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    
    return meter_idx;
}
#ifdef __PLC_BPLC_AGG__
BOOLEAN get_node_addr_from_fast_index(INT8U port,AGGREGATE_CTL *agg_ctl)
{
    INT16U node_count;
//    INT16U idx;
    INT16U node_idx;
    BOOLEAN flg = FALSE;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    //node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
    node_count = bin2_int16u(agg_ctl->bplc.max_node_idx);
    node_idx   = bin2_int16u(agg_ctl->bplc.node_idx);
    while(node_idx < node_count)
    {
        if (fast_index_list.fast_index[node_idx].port == port)
        {
            mem_cpy(agg_ctl->bplc.node_addr,fast_index_list.fast_index[node_idx].node,6);
            /* mark idx position */
            agg_ctl->bplc.node_idx[0] = node_idx;
            agg_ctl->bplc.node_idx[1] = node_idx >> 8;
            flg = TRUE;
            break;
        }
        node_idx++;
    }
    tpos_mutexFree(&SIGNAL_FAST_IDX);
    
    return flg;
}
#endif

//����������ģʽ�£�ʹ�ü�س���
INT8U prepare_read_item_concentrator(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U gb_645_frame[260];
    INT8U frame_pos;
    BOOLEAN result;

    portContext = (PLCPortContext*)readportcontext;
    
    portContext->router_work_info.phase = 0;
    
    frame_pos = 0;
    if(portContext->router_base_info.router_info4.monitor_afn_type)  //02H-F1
    {
        gb_645_frame[frame_pos++] = meter_protocol_2_router_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol);      //ͨ��Э������
    }
    else  //13H-F1
    {
        gb_645_frame[frame_pos++] = meter_protocol_2_router_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol);      //ͨ��Э������

        if((portContext->router_base_info.router_13_or_09 == ROUTER_PROTOCOL_GB13762))
        {
        if (portContext->router_base_info.router_info3.rec_delay_flag) gb_645_frame[frame_pos++] = 0x00;      //ͨ����ʱ����Ա�־
        }
        gb_645_frame[frame_pos++] = 0; //�ӽڵ㸽���ڵ�����n
    }
    result = prepare_read_item(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),gb_645_frame+frame_pos+1,gb_645_frame+frame_pos);

    if (result)
    {
        if (portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.batch_ctrl.is_no_resp) //�ұ�
        {
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            portContext->OnPortReadData = prepare_read_item_concentrator;
            return 0;
        }
        
        if ((portContext->router_base_info.router_info3.rtu_no_mode)
        && (isvalid_meter_addr(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,FALSE))
        && (!check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol)))
        {
                    //����ɼ���ģʽ����
            if(portContext->router_base_info.router_info3.rtu_frame_format)
            {
                            //�޸ı���Ϊ�ɼ���ģʽ�� ����ַ�ӵ���������ײ���
            gb_645_frame[frame_pos] = trans_read_frame_to_cjq_mode(gb_645_frame+frame_pos+1,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no);
            }
               //�ӽڵ�Ϊ�ɼ�����645����Ϊ����ַ������ģʽ��С������Ӧ��. ����ֻ�а���һ�������ɼ���ģʽ��ע����ʾ�ͼ�أ�����֧��
            mem_cpy(portContext->router_work_info.ADDR_DST,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6); //�ӽڵ��ɲɼ���
        }
        else if((portContext->router_base_info.router_info4.dzc_cvt_no_mode) 
        && (isvalid_meter_addr(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,FALSE))
        && (check_is_sqr_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol)))
        {
            mem_cpy(portContext->router_work_info.ADDR_DST,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6); //�ӽڵ���ת����
        }
        else
        {
            mem_cpy(portContext->router_work_info.ADDR_DST,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,6);
        }

        router_376_2_set_aux_info(0,40,1,TRUE);

        if(portContext->router_base_info.router_info4.monitor_afn_type)
        {
            portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_TRANS,DT_F1,gb_645_frame,gb_645_frame[frame_pos]+frame_pos+1,portContext);
        }
        else
        {
            portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RTRANS,DT_F1,gb_645_frame,gb_645_frame[frame_pos]+frame_pos+1,portContext);
        }
        #ifdef __MESSAGE_SEND_RECEIVE_RECORD__
        message_send_and_receive_num_record(bin2_int16u(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.spot_idx),0x55);//��¼���ͱ��Ĵ���
        #endif
        portContext->cur_plc_task = PLC_TASK_READ_METER;
        portContext->cur_plc_task_step = PLC_READ_METER_WAIT_RESP_13F1;
        portContext->OnPortReadData = prepare_read_item_concentrator;
        readport_plc.OnPortReadData = router_wait_resp_frame;
        return portContext->frame_send_Len;
    }
    else
    {
        //����
        
        //portContext->params.task_read_data.has_fail_meter = TRUE;
        portContext->cur_plc_task = PLC_TASK_READ_METER;
        portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
        portContext->OnPortReadData = get_read_meter_info;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
    }
    return 0;
}

INT8U get_read_meter_info(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT16U meter_idx,node_idx;
    //INT8U gb_645_frame[260];
    //INT8U *meter_no;

    portContext = (PLCPortContext*)readportcontext;

    #ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
    if (check_is_all_ch(priority_node.map,256,0x00) == FALSE)
    {
        for(meter_idx=1;meter_idx<MAX_METER_COUNT;meter_idx++)
        {
            if (get_bit_value(priority_node.map,256,meter_idx))
            {
                clr_bit_value(priority_node.map,256,meter_idx);
                break;
            }
        }
    }
    else
    #endif
    {
    node_idx = bin2_int16u(portContext->params.task_read_data.node_idx);
    meter_idx = get_meter_idx_from_fast_index(node_idx,COMMPORT_PLC);

    if (meter_idx == 0xFFFF)
    {
        portContext->params.task_read_data.has_fail_meter = 1; //����һֱ��ѯ��
        if (portContext->params.task_read_data.has_fail_meter)
        {
            portContext->params.task_read_data.has_fail_meter = FALSE;
            node_idx = 0;

              portContext->concentrator_read_cycle_no ++ ;
              if( portContext->concentrator_read_cycle_no > 3)
              {
                 portContext->concentrator_read_cycle_no = 0xAA;
              }

        }
        else
        {

              portContext->concentrator_read_cycle_no = 0xAA;

            //Ъ��
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** �����ˣ�Ъ��������� ***");
            debug_println_ext(info);
            #endif
            portContext->cur_plc_task = PLC_TASK_READ_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_SLEEP;
            portContext->OnPortReadData = router_check_urgent_timeout;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            return 0;
        }
    }
        else
        {
            node_idx++;
        }
    int16u2_bin(node_idx,portContext->params.task_read_data.node_idx);
    }

    if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
    {
        portContext->router_work_info.phase = 0;

        if (prepare_read_meter_param(meter_idx,&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params)))
        {

            portContext->cur_plc_task = PLC_TASK_READ_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_PREPARE_ITEM;
            portContext->OnPortReadData = prepare_read_item_concentrator;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            return 0;
        }

    }

    portContext->cur_plc_task = PLC_TASK_READ_METER;
    portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
    portContext->OnPortReadData = get_read_meter_info;
    readport_plc.OnPortReadData = router_check_urgent_timeout;

    return 0;
}


void set_plc_meter_event_task_flag(INT8U rec_year,INT8U rec_month,INT8U rec_day)
{
    INT8U idx;
    INT8U cycle[MAX_METER_EVENT_LEVEL*2]={0};

    fread_ertu_params(EEADDR_SET_F107,cycle,MAX_METER_EVENT_LEVEL*2);
    for(idx=0;idx<MAX_METER_EVENT_LEVEL;idx++)
    {
        if(cycle[idx*2+1] == 2) //Сʱ
        {
            file_delete(FILEID_EVENT_GRADE_READ_FALG+idx);
            #ifdef __DEBUG_RECINFO__
            snprintf(info,100,"****���õ���¼���������!  idx = %d ",idx);
            println_info(info);
            #endif
        }
        else if(cycle[idx*2+1] == 1) //����
        {
            file_delete(FILEID_EVENT_GRADE_READ_FALG+idx);
            #ifdef __DEBUG_RECINFO__
            snprintf(info,100,"****���õ���¼���������!  idx = %d ",idx);
            println_info(info);
            #endif
        }
        else if(cycle[idx*2+1] == 3) //��
        {
            if(cycle[idx*2] == 0)
            {
                //����Ϊ0 ��ִ�к�������
                continue;
            }
            if(((rec_day-1) % cycle[idx*2]) == 0)
            {
                file_delete(FILEID_EVENT_GRADE_READ_FALG+idx);
                #ifdef __DEBUG_RECINFO__
                snprintf(info,100,"****���õ���¼���������!  idx = %d ",idx);
                println_info(info);
                #endif
            }
        }
        else if(cycle[idx*2+1] == 4) //��
        {
            if(cycle[idx*2] == 0)
            {
                //����Ϊ0 ��ִ�к�������
                continue;
            }
            if(((rec_month-1) % cycle[idx*2]) == 0)
            {
                file_delete(FILEID_EVENT_GRADE_READ_FALG+idx);
                #ifdef __DEBUG_RECINFO__
                snprintf(info,100,"****���õ���¼���������!  idx = %d ",idx);
                println_info(info);
                #endif
            }
        }
    }
}
//data��0��ʾ��С��һ��ĳ�������>0û��
void get_plan_min_cycle(INT8U data[MAX_METER_EVENT_PLAN_COUNT])
{
    INT32U offset;
    INT8U idx,plan_id;
    INT8U cycle[MAX_METER_EVENT_LEVEL*2]={0};
    PARAM_F106 f106;

    fread_ertu_params(EEADDR_SET_F107,cycle,MAX_METER_EVENT_LEVEL*2);
    for(idx=0;idx<MAX_METER_EVENT_LEVEL;idx++)
    {
        if((cycle[idx*2+1] == 2) && (cycle[idx*2]) < 24)
        {
            cycle[idx*2] = 0;   //���ڵ�λ��Сʱ
            continue;
        }
        if(cycle[idx*2+1] == 1)
        {
            cycle[idx*2] = 0;   //���ڵ�λ�Ƿ���
            continue;
        }
        cycle[idx*2] = 0xFF;
    }

    mem_set(data,MAX_METER_EVENT_PLAN_COUNT,0xFF);
    for(plan_id=1;plan_id<=MAX_METER_EVENT_PLAN_COUNT;plan_id++)
    {
        offset = sizeof(PARAM_F106) * MAX_METER_EVENT_ITEM_COUNT * (plan_id-1);
        offset += PIM_PARAM_F106;
        for(idx=0;idx<MAX_METER_EVENT_ITEM_COUNT;idx++)
        {
            fread_array(FILEID_METER_EVENT_PARAM,offset+idx*sizeof(PARAM_F106),f106.value,sizeof(PARAM_F106));
            if(check_is_all_FF(f106.value,5)) break;
            if((f106.level == 0) || (f106.level > MAX_METER_EVENT_LEVEL)) continue;
            if(cycle[(f106.level-1)*2] == 0)
            {
                data[plan_id-1] = 0;
                break;
            }
        }
    }
}

//�㲥Уʱ
BOOLEAN  meter_cast_timing(objReadPortContext * readportcontext,INT8U port)
{
      PLCPortContext *portContext;
      //RS485PortContext* pRs485Context;
      bool flag;
      SET_F33 f33;
      INT8U BS8 = 0;
      INT8U readport_idx;
	  
      flag = 0;
      #ifdef __PRECISE_TIME__
      PRECISE_TIME_CAST_CTRL cast_time_ctrl;
      #endif
      //���ʱ�Ӷ�ʧ�󣬲��ܶԵ����ж�ʱ
     if(get_system_flag(SYS_CLOCK_LOST,SYS_FLAG_BASE) || get_system_flag(SYS_CLOCK_DOUBT,SYS_FLAG_BASE))
     {
         return 0;
     }
     #ifdef __PRECISE_TIME__ //�˷�flag����ò�Ҫʹ�ã�
     if((system_flag & SYS_CAST_TIME_DONE) || (system_flag & SYS_POWER_ON_NEED_PRECISE_TIME))
     {
        readport_idx = get_readport_idx(port);
        if(readport_idx != 0xFF)
        {
             switch(port)
             {
             case 2:
                 if(system_flag & SYS_POWER_ON_NEED_PRECISE_TIME)
                 {
                 // pRs485Context = (RS485PortContext*)readportcontext;
                     readport_rs485[readport_idx].OnPortReadData = rs485_prepare_read_item;
                     portContext_rs485[readport_idx].cur_rs485_task = RS485_TASK_CAST_TIMING;
                     portContext_rs485[readport_idx].run_param.adjtime_flag = 1;
                     portContext_rs485[readport_idx].run_param.adjtime_step = 1;
                     flag = 1;
                     system_flag &= ~SYS_POWER_ON_NEED_PRECISE_TIME;
                 }
                 break;
             case 3:
                  if(system_flag & SYS_CAST_TIME_DONE)
                  {
                 // pRs485Context = (RS485PortContext*)readportcontext;
                     readport_rs485[readport_idx].OnPortReadData = rs485_prepare_read_item;
                     portContext_rs485[readport_idx].cur_rs485_task = RS485_TASK_CAST_TIMING;
                     portContext_rs485[readport_idx].run_param.adjtime_flag = 1;
                     portContext_rs485[readport_idx].run_param.adjtime_step = 1;
                     flag = 1;
                     system_flag &= ~SYS_CAST_TIME_DONE;
                  }
                  break;
             default:
                 flag =0;
                 break;
             }
            
        }
        else  flag =0;
            cast_time_ctrl.cast_485_ready = 0;
            cast_time_ctrl.after_cast_read_time_complete = 0;
            cast_time_ctrl.cast_485_complete = 1;
            fwrite_ertu_params(EEADDR_PRECISE_CAST_PARAM,cast_time_ctrl.value ,sizeof(PRECISE_TIME_CAST_CTRL));
         return flag;
     }
     #endif
      
      fread_array(FILEID_RUN_PARAM,FLADDR_METER_SET_CLOCK_FALG,&BS8,1);//�¹淶��չF30���ƹ㲥�����ر�
      if( BS8 != 0xAA)
     {
         fread_ertu_params(EEADDR_SET_F33 + (port-1) * sizeof(tagSET_F33),f33.value,sizeof(SET_F33));
         #if  defined(__DL698_41_0928__)
        f33.cast_time[2] = f33.rec_timeseg[10];
        f33.cast_time[1] = f33.rec_timeseg[9];
        f33.cast_time[0] = f33.rec_timeseg[8];
        #endif
         if(f33.run_ctrl[0] & 0x08)
         {    
           #ifdef __PROVICE_SHANGHAI__  //2015-11-30
		    INT8U frame[5]={0};
            fread_ertu_params(EEADDR_SET_F241,frame,5);
            if(frame[0] > 2) frame[0] = 0;    //��ʱƵ�ʣ�0-ÿ�ն�ʱһ�Σ�1-ÿ�ܶ�ʱһ�Σ�2-ÿ�¶�ʱһ�Ρ���Ĭ��0��     
             if(frame[0] == 0)
             {
             f33.cast_time[2] = 0;
             }
            else if(frame[0] == 1)
             {
            if(datetime[WEEKDAY] != (frame[1]-1)) return flag;
             f33.cast_time[2] = 0;
             }
            else if(frame[0] == 2)
             {
             f33.cast_time[2] = byte2BCD(frame[1]);
             }
              f33.cast_time[1] = byte2BCD((frame[2] > 23) ? 4 : frame[2]);
              f33.cast_time[0] = byte2BCD((frame[3] > 59) ? 5 : frame[3]);
           #endif
           
           if(f33.cast_time[2]==0)
           f33.cast_time[2] = byte2BCD(datetime[DAY]); 
              
           if((byte2BCD(datetime[DAY])==f33.cast_time[2])&&(byte2BCD(datetime[HOUR])==f33.cast_time[1])&&(byte2BCD(datetime[MINUTE])==f33.cast_time[0]))//�����ǰʱ��==�㲥Уʱʱ�䣬ִ��Уʱ
           {
               readport_idx = get_readport_idx(port); //�˿ں�4��ʾά��485�ڣ�����᲻����Ӱ�죿
         
                switch(port)
                {
                    case 1:
                    case 2:
                    case 3:
                        if(readport_idx != 0xFF)
                        {
                            readport_rs485[readport_idx].OnPortReadData = rs485_prepare_read_item;
                            portContext_rs485[readport_idx].cur_rs485_task = RS485_TASK_CAST_TIMING;
                            portContext_rs485[readport_idx].run_param.adjtime_flag = 1;
                            portContext_rs485[readport_idx].run_param.adjtime_step = 1;
                            flag = 1;
                        }
                        else
                        {
                            flag = 0;
                        }
                         break;

                    case 4:
                    #if defined(__NGRID_HARDWARE_II__)
                         if(readport_idx != 0xFF)
                        {
                            readport_rs485[readport_idx].OnPortReadData = rs485_prepare_read_item;
                            portContext_rs485[readport_idx].cur_rs485_task = RS485_TASK_CAST_TIMING;
                            portContext_rs485[readport_idx].run_param.adjtime_flag = 1;
                            portContext_rs485[readport_idx].run_param.adjtime_step = 1;
                            flag = 1;
                        }
                        else
                        {
                            flag = 0;
                        }
                         break;
                    #else 
                         portContext = (PLCPortContext*)readportcontext;
                         readport_plc.OnPortReadData = router_send_afn_12_F2;
                         portContext->OnPortReadData = router_send_afn_12_F2;
                         portContext->cur_plc_task = PLC_TASK_METER_TIMING_CAST;
                         portContext->cur_plc_task_step = PLC_TASK_TIMING_CAST_PAUSE_ROUTER_12_F2;
                         flag = 1;
                         break;
                    #endif
                    default:
                        flag =0;
                        break;
                }
           }
           else
           flag = 0;
         }
         else
         flag = 0;
     }
    return flag;

}
#ifdef __HUNAN_NEW_RECORDING__//__PROVICE_HUNAN__
void check_hunan_curve_recording(void)
{
	//INT8U patch_flag;
    BOOLEAN has_task;

    //patch_flag = 0;
    has_task = FALSE;

	//����ʱ��
	if(compare_string(read_meter_flag_curve_hunan.cycle_day,datetime+DAY,3) != 0)
    {
         mem_cpy(read_meter_flag_curve_hunan.cycle_day,datetime+DAY,3);
      //   clr_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);/*�������Ļ����ᵼ���¼�������*/
         clr_readport_read_meter_flag_from_fast_index(read_meter_flag_curve_hunan.flag,COMMPORT_PLC);
	}
	// ����
    #ifdef __PROVICE_SHAANXI__/*����Ҫ���5������ȥ��*/
    if( (datetime[HOUR] == 7 )&&(datetime[MINUTE] == 5 ))
    #else
	if( (datetime[HOUR] == 7 )&&(datetime[MINUTE] == 0 ))
    #endif
	{
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve_hunan.flag,COMMPORT_PLC);
		mem_cpy(read_meter_flag_curve_hunan.cycle_4_hour,datetime+MINUTE,5);
		#ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** ����7������ ***");
                debug_println(info);
                #endif
		has_task = TRUE;
	}
    #ifdef __PROVICE_SHAANXI__
    else if( (datetime[HOUR] == 11 )&&(datetime[MINUTE] == 5 ))
    #else
	else if( (datetime[HOUR] == 11 )&&(datetime[MINUTE] == 0 ) )
    #endif
	{
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve_hunan.flag,COMMPORT_PLC);
		mem_cpy(read_meter_flag_curve_hunan.cycle_4_hour,datetime+MINUTE,5);
		#ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** ����11������ ***");
                debug_println(info);
                #endif
		has_task = TRUE;
	}
    #ifdef __PROVICE_SHAANXI__
    else if( (datetime[HOUR] == 15 )&&(datetime[MINUTE] == 5 ))
    #else
	else if( (datetime[HOUR] == 15 )&&(datetime[MINUTE] == 0 ) )
    #endif
	{
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve_hunan.flag,COMMPORT_PLC);
		mem_cpy(read_meter_flag_curve_hunan.cycle_4_hour,datetime+MINUTE,5);
		#ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** ����15������ ***");
                debug_println(info);
                #endif
		has_task = TRUE;
	}
    #ifdef __PROVICE_SHAANXI__
    else if( (datetime[HOUR] == 19 )&&(datetime[MINUTE] == 5 ))
    #else
	else if( (datetime[HOUR] == 19 )&&(datetime[MINUTE] == 0 ) )
    #endif
	{
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_curve_hunan.flag,COMMPORT_PLC);
		mem_cpy(read_meter_flag_curve_hunan.cycle_4_hour,datetime+MINUTE,5);
		#ifdef __SOFT_SIMULATOR__
                snprintf(info,100,"*** ����19������ ***");
                debug_println(info);
                #endif
		has_task = TRUE;
	}
    #ifdef __PROVICE_HUNAN__
    else if( (datetime[HOUR] == 17 )&&(datetime[MINUTE] == 30 ))
    {
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
		#ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** �������ʱ�ӣ��ж�ERC12 ***");
        debug_println(info);
        #endif
		has_task = TRUE;
    }
    #endif

    if(has_task == TRUE) 
    {
        if(portContext_plc.router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER)
        {
            portContext_plc.need_reset_router = 1;      /*����ģʽ����·��*/
        }
        else
        {
            portContext_plc.need_reset_router = 3;    /*�����Ͳ���ģʽ�����³���*/
        }
    }
	/*
    if (has_task)
    {
        //����ģʽ�£���������Ϣ
        if ((portContext_plc.cur_plc_task == PLC_TASK_READ_METER) && (portContext_plc.cur_plc_task_step == PLC_READ_METER_SLEEP))
        {
            portContext_plc.cur_plc_task_step = PLC_READ_METER_FIND_METER;
            portContext_plc.OnPortReadData = get_read_meter_info;
            readport_plc.OnPortReadData = router_check_urgent_timeout;

            portContext_plc.params.task_read_data.node_idx[0] = 0;
            portContext_plc.params.task_read_data.node_idx[1] = 0;
            portContext_plc.params.task_read_data.has_fail_meter = FALSE;
        }
    }
    */
}

#endif

#ifdef __PROVICE_SHANGHAI__
/*
 �Ϻ�ÿ��00:10����һ��·�ɣ�ֻ���ڷ��Ӽ�����������ã�����Ƶ������
 */
void check_ShangHai_reset_router()
{
    if((datetime[HOUR] == 0 )&&(datetime[MINUTE] == 10 ))
    {
        if(portContext_plc.router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER)
        {
            portContext_plc.need_reset_router = 1;      /*����ģʽ����·��*/
        }
        else
        {
            portContext_plc.need_reset_router = 3;    /*�����Ͳ���ģʽ�����³���*/
        }
    }
}
#endif
void get_router_main_node_addr(INT8U* main_node)
{
    INT32U ertu_addr;

    #ifdef __PROVICE_CHONGQING__
    //����ʹ����̫����mac��Ϊ���ڵ��ַ
    //mem_cpy(main_node,gSystemInfo.eth_mac_addr,6);
	mem_cpy(main_node,gSystemInfo.eth_mac_addr+3,3);
    mem_set(main_node+3,3,0x00);
    ertu_addr = bin2_int32u(main_node);
    //�ѵ�ַת��Ϊ4�ֽڵ�BCD��
    ul2bcd(ertu_addr,main_node,4);
    #else

    {
        #if defined __PROVICE_SHAANXI__ ||  defined __PROVICE_GANSU__/*�����͸�������ڵ��й���Ҫ��*/
        ertu_addr = bin2_int16u(gSystemInfo.ertu_devid + 2);
        //�ѵ�ַת��Ϊ4�ֽڵ�BCD��
        ul2bcd(ertu_addr,main_node,3);

        mem_cpy(main_node+3,gSystemInfo.ertu_devid,2); //������
        main_node[5] = 0;
        #else
        ertu_addr = bin2_int16u(gSystemInfo.ertu_devid + 2);
        //�ѵ�ַת��Ϊ4�ֽڵ�BCD��
        ul2bcd(ertu_addr,main_node,4);

        mem_cpy(main_node+4,gSystemInfo.ertu_devid,2); //������
        #endif
        if(is_valid_bcd(gSystemInfo.ertu_devid,2) == FALSE) //����bcd��ת��0
        {
         main_node[4] = 0;
         main_node[5] = 0;
        }

    }

    #endif
}

#ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
void preap_priority_node(void)
{
    INT16U meter_idx,meter_seq,rec_pos;
    INT8U router_protocol;
    INT8U meter_no[6];

    if (priority_node.flag == 0x00) //����׼���ڵ�����
    {
        if (check_is_all_ch(priority_node.map,256,0x00) == FALSE)
        {
            priority_node.count = 0;
            for(meter_idx=1;meter_idx<=MAX_METER_COUNT;meter_idx++)
            {
                if (get_bit_value(priority_node.map,256,meter_idx))
                {
                    fread_array(meter_idx,PIM_METER_NO,meter_no,6);
                    if (memory_fast_index_find_node_no(COMMPORT_PLC,meter_no,&meter_seq,&rec_pos,&router_protocol,NULL))
                    {
                        mem_cpy(priority_node.node[priority_node.count],meter_no,6);
                        priority_node.count++;
                        if (priority_node.count >= 20) break;
                    }
                }
            }
            if (priority_node.count > 0)
            {
                priority_node.flag = 0x55;
            }
        }
    }
}

INT8U check_priority_node(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;
    
    if(portContext->router_base_info.router_vendor != ROUTER_VENDOR_TOPSCOMM) return 0; //����������֧�ֲ������
    #ifdef __PROVICE_JIBEI__
    if(portContext->need_reset_router)
    {
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"**** ����·���� �ݲ�ִ�����ȶ��� ****");
        debug_println_ext(info);
        #endif
        return 0;
    }
    #endif
    if(portContext->cur_plc_task == PLC_TASK_READ_METER) //�ڳ���״̬�Ų���
    {
      if (portContext->cur_plc_task_step == PLC_READ_METER_ROUTER_MODE)
      {
          if(portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_ROUTER)
          {
            if (portContext_plc.batch_meter_ctrl.is_restart_node_yugao == 1) return 0;
            
              preap_priority_node();

              if (priority_node.flag == 0x55) //����׼���ڵ�����
              {
                  if (priority_node.count > 0)
                  {
                      readport_plc.OnPortReadData = router_send_afn_11_F8;
                      portContext->cur_plc_task_step = PLC_READ_METER_PRIOR_11F8;
                      return 1;
                  }
              }
          }
      }
    }
    return 0;
}

void clear_priority_node(READPORT_METER_DOCUMENT* meter_doc)
{
    INT8U idx;

    tpos_mutexPend(&SIGNAL_BATCH_SET);

    if (priority_node.flag == 0xAA)
    {
        for(idx=0;idx<priority_node.count;idx++)
        {
            if (compare_string(meter_doc->meter_no,priority_node.node[idx],6) == 0)
            {
                mem_cpy(priority_node.node[idx],priority_node.node[priority_node.count-1],6);
                mem_set(priority_node.node[priority_node.count-1],6,0x00);
                priority_node.count--;
                clr_bit_value(priority_node.map,256,bin2_int16u(meter_doc->meter_idx));
            }
        }
        if (priority_node.count == 0) priority_node.flag = 0x00;
    }

    tpos_mutexFree(&SIGNAL_BATCH_SET);
}
#endif

#if (defined __PROVICE_CHONGQING__ )

/*
    ����ͳ���м̸��ڵ���Ϣ����Ҫ������ʾ��
    //save  FILEID_RELAY_CHONGQING_EXT
    �������         2�ֽ�
    0���м�����n      2�ֽ�
    0�����1��Ϣ     PLC_RELAY_INFO  17�ֽ�
    0�����n��Ϣ     PLC_RELAY_INFO  17�ֽ�
    1���м�����n      2�ֽ�
    1�����1��Ϣ     PLC_RELAY_INFO  17�ֽ�
    1�����n��Ϣ     PLC_RELAY_INFO  17�ֽ�

    9���м�����n      2�ֽ�
    9�����1��Ϣ     PLC_RELAY_INFO  17�ֽ�
    9�����n��Ϣ     PLC_RELAY_INFO  17�ֽ�

*/
void plc_router_relay_stat(void)       //save  FILEID_RELAY_CHONGQING_EXT
{
    INT32U  offset,pos;
    INT16U  meter_seq,meter_idx,meter_count,relay_count;
    INT16U  meter_cnt = 0;
    INT16U  parent_meter_idx = 0;
    INT16U  relay_meter_cnt = 0;//ÿһ���м̵ĵ������
    INT16U  rec_no = 0;
    INT16U  pos_fast_idx = 0;
    FAST_INDEX fast_index;
    INT8U  count[2];
    INT8U  flag[256];//
    struct{
        INT8U seq[4];
        INT8U read_date[5];
        METER_READ_INFO meter_read_info;
        //INT8U value[27];
    }var;
    C2_F56 c2f56;
    INT8U  td[3] = {0};
    METER_DOCUMENT meter_doc;
    METER_DOCUMENT parent_meter_doc;
    PLC_RELAY_INFO plc_relay_info;
    INT8U  idx;
    INT8U  meter_relay_info[12] = {0};
    INT8U  router_protocol = 0;
    INT8U  user_class = 0;
    get_yesterday(td);
    mem_set(c2f56.value,sizeof(C2_F56),0x00);

    mem_cpy(c2f56.td,td,3);
    //ɾ��ͳ���ļ�
    file_delete(FILEID_RELAY_CHONGQING_EXT);
    meter_count = 0;
    pos = 2;

    meter_cnt = fast_index_list.count;
    mem_set(flag,256,0xFF);//�������Ҫ���㡣
    for(idx = 0; idx < 10; idx++ )
    {
        relay_count = 0;
        for(meter_seq=0;meter_seq<meter_cnt;meter_seq++)
        {
            //ִ��·������
            //plc_router_process_urgent_task();

            

            mem_cpy(fast_index.value, fast_index_list.fast_index[meter_seq].value, sizeof(FAST_INDEX));
            meter_idx = bin2_int16u(fast_index.seq_spec) & FAST_IDX_MASK;

            if( (meter_idx == 0) || (meter_idx > MAX_METER_COUNT) || (COMMPORT_PLC != fast_index.port) ) 
            {
                continue;
            }
            
            if(get_bit_value(flag,256,meter_idx) == 0)
            {
                continue;
            }
            //fread_array(meter_idx,PIM_REC_STATE,C1_F170.value,sizeof(DC1_F170));
            fread_array(meter_idx,PIM_CUR_READ_INFO,var.seq,sizeof(var));
            if( (var.meter_read_info.depth <= 9) && (var.meter_read_info.port == COMMPORT_PLC) )
            {
                
                if(idx != var.meter_read_info.depth) continue; 

                //������ľ������������ߴ���Ч��
                clr_bit_value(flag,256,meter_idx);
                
                fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
                mem_set(plc_relay_info.value ,sizeof(PLC_RELAY_INFO),0xFF);
                plc_relay_info.depth = idx;
                mem_cpy(plc_relay_info.spot_idx ,meter_doc.spot_idx ,2);
                mem_cpy(plc_relay_info.meter_no ,meter_doc.meter_no ,6);
                if(idx != 0)
                {
                    //
                    fread_array(FILEID_ROUTER_INFO_TABLE,PIM_AREA_ROUTER_TABLE_DATA_START+(meter_idx-1)*PIM_AREA_ROUTER_TABLE_PER_METER_INFO_LEN,meter_relay_info,12);
                    if(0 == compare_string(plc_relay_info.meter_no,meter_relay_info,6))
                    {
                        // 
                        if(memory_fast_index_find_node_no(COMMPORT_PLC,meter_relay_info+6,&rec_no,&pos_fast_idx,&router_protocol,&user_class))
                        {
                            //
                            parent_meter_idx = bin2_int16u(fast_index_list.fast_index[pos_fast_idx].seq_spec);
                            parent_meter_idx &= FAST_IDX_MASK;
                            fread_array(parent_meter_idx,PIM_METER_DOC,(INT8U *)&parent_meter_doc,sizeof(METER_DOCUMENT));
                            mem_cpy(plc_relay_info.parent_spot_idx,parent_meter_doc.spot_idx,2);
                            mem_cpy(plc_relay_info.parent_meter_no,meter_relay_info+6,6);
                        }
                        
                    }
                }

                fwrite_array(FILEID_RELAY_CHONGQING_EXT,pos+2+relay_count*sizeof(PLC_RELAY_INFO),plc_relay_info.value,sizeof(PLC_RELAY_INFO));
                relay_count++;
                meter_count++;

                // ������ʾ 4-6  �м�·��ͳ����Ϣ 
                relay_meter_cnt = c2f56.meter_count[var.meter_read_info.depth][1] * 0x100 + c2f56.meter_count[var.meter_read_info.depth][0];
                relay_meter_cnt++;
                c2f56.meter_count[var.meter_read_info.depth][0] = relay_meter_cnt;
                c2f56.meter_count[var.meter_read_info.depth][1] = relay_meter_cnt >> 8;
                
            }                        
        }
        count[0] = relay_count;
        count[1] = relay_count>>8;
        fwrite_array(FILEID_RELAY_CHONGQING_EXT,pos,count,2);
        pos += 2 + relay_count*sizeof(PLC_RELAY_INFO);
    }
    count[0] = meter_count;
    count[1] = meter_count>>8;
    fwrite_array(FILEID_RELAY_CHONGQING_EXT,0,count,2);

    // 
    offset = getPassedDays(2000+td[2],td[1],td[0]);
    offset = offset % SAVE_POINT_NUMBER_DAY_HOLD;
    offset *=sizeof(C2_F56);
	offset += PIM_C2_F56;
	fwrite_array(FILEID_C2_CHONGQING_EXT,offset,c2f56.value,sizeof(C2_F56));
}

/*+++
  ���ܣ����³���ͳ����Ϣ
  ������
        
  ���أ�
         ��
  ������
       1��ͳ���ز���������
---*/
void  update_llvc_rec_state(void)
{
   
	INT16U	node_count;
	INT16U	read_count;
	INT16U	fail_count;
	INT16U	vip_count;
	INT16U	meter_idx;
	INT16U	idx;
	C2_F55 	c2f55;
	INT8U 	td[3];//
	INT8U	value;
	INT8U	seq_spec[2];
	INT8U	port;
	INT8U  	pos,pos_bit;
	BOOLEAN	is_F35 = FALSE;    


	vip_count = 0;
	fail_count = 0;
    //ͳ�ƶ�ȡ�ɹ��ĵ������
    ClrTaskWdt();

	//
	tpos_enterCriticalSection();
	mem_cpy(td,read_meter_flag_cycle_day.cycle_day,3);
	tpos_leaveCriticalSection();
	
    fread_array(FILEID_RUN_DATA,FLADDR_REC_STATUS,c2f55.value,sizeof(C2_F55));
    if( (c2f55.td[0] != td[0]) || (c2f55.td[1] != td[1]) || (c2f55.td[2] != td[2]) )
    {
        mem_set(c2f55.value,sizeof(C2_F55),0x00);
        c2f55.td[0] = td[0];
        c2f55.td[1] = td[1];
        c2f55.td[2] = td[2];
        fwrite_array(FILEID_RUN_DATA,FLADDR_REC_STATUS,c2f55.value,sizeof(C2_F55));
		return;//�������ͳ��һ��
    }

	read_count = get_readport_meter_count_from_fast_index(COMMPORT_PLC);
	c2f55.meter_count[0] = (read_count & 0x00FF);
	c2f55.meter_count[1] = ((read_count>>8) & 0x00FF);

	tpos_mutexPend(&SIGNAL_FAST_IDX);
	node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
	tpos_mutexFree(&SIGNAL_FAST_IDX);
	
	for(idx=0;idx<node_count;idx++)
    {
        tpos_enterCriticalSection();
        seq_spec[0]=fast_index_list.fast_index[idx].seq_spec[0];
        seq_spec[1]=fast_index_list.fast_index[idx].seq_spec[1];
		port = fast_index_list.fast_index[idx].port;
        tpos_leaveCriticalSection();

		if(port != READPORT_PLC) continue;
		
        meter_idx = bin2_int16u(seq_spec) & FAST_IDX_MASK;		
        if(!file_exist(meter_idx)) continue;

		if(check_F161_day_hold(meter_idx,1,FALSE))
		{
			pos = (meter_idx-1) / 8;
        	pos_bit = (meter_idx-1) % 8;
        	c2f55.rec_status[pos] |= (0x01<<pos_bit);
		}
		else
		{
			//
			fail_count++;
		}
        //�ж��Ƿ����ص��
        fread_ertu_params(EEADDR_SET_F35+(meter_idx/8),&value,1);
        is_F35 = ((~value) & (0x01<<(meter_idx%8))) ? TRUE : FALSE;

		if(is_F35 == TRUE)
		{
			vip_count ++;
			if(vip_count >= 255)
				vip_count = 255;
		}
        

    }

	c2f55.vip_count = (vip_count & 0x00FF);//�ص���û�
	   
    c2f55.fail_count[0] = fail_count;
    c2f55.fail_count[1] = fail_count>>8;

    fwrite_array(FILEID_RUN_DATA,FLADDR_REC_STATUS,c2f55.value,sizeof(C2_F55));
}

#endif

BOOLEAN  plc_router_vipmeter_recording(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    //INT16U idx,meter_idx;
    INT8U  vip_meter_num;
    //INT8U  datalen,pos,result;
    //INT8U need_read_item_num;
    static INT8U last_hour = 0xFF;

    portContext = (PLCPortContext*)readportcontext;

    if((portContext->router_base_info.router_vendor == ROUTER_VENDOR_TOPSCOMM)
      && (portContext->vip_meter_read.vip_cmd_11F8 == 1)) return 0;  //���ŷ�����֧�ֶ���Ҳ����

    if(  portContext->router_base_info.router_info1.cycle_rec_mode == CYCLE_REC_MODE_CONCENTRATOR) return 0;//����ģʽ����
    if(portContext->plc_other_read_mode == CYCLE_REC_MODE_PARALLEL) return 0;//����ģʽ����

    if(last_hour == 0xFF)last_hour = datetime[HOUR];

   if((last_hour != datetime[HOUR]) && (datetime[MINUTE] > 0) && (datetime[MINUTE] < 30)) //�ϵ��һ�β�����0���ӵ�ʱ�򲻳�,30�����Ժ󲻳�
    {
        last_hour = datetime[HOUR];
    }
    else
    {
        return 0;
    }

    //���ȼ���Ƿ����ص����Ҫ����

      vip_meter_num = get_vip_meter_count();

     if((vip_meter_num == 0) || (vip_meter_num > 20)) //������ص��ֱ�ӽ�����һ��
     {
       #if ( (defined __PROVICE_HUNAN__) || (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__) )
       if(check_special_meters_need_read()==0)//���û��������������أ�
       #endif
       {
        //��������������
        return 0;
       }

     }

    if(portContext->cur_plc_task != PLC_TASK_READ_VIP_METER) //�ص������Ҫ������
    {
        portContext->before_vip_task = portContext->cur_plc_task;  //���浱ǰ����
        portContext->before_vip_task_step = portContext->cur_plc_task_step;  //���浱ǰ����״̬
        portContext->Before_OnPortReadData = portContext->OnPortReadData;  //���浱ǰ�˿�ִ������
        portContext->PLC_Before_OnPortReadData = readport_plc.OnPortReadData;
    }
    //��ͣ
    if(portContext->vip_meter_read.vip_cmd_pause != 1)
    {
        readport_plc.OnPortReadData = router_check_urgent_timeout;
        portContext->OnPortReadData = router_send_afn_12_F2;
        portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
        portContext->cur_plc_task_step = PLC_VIP_METER_12HF2;
    }
    else
    {
        start_read_vip_meter(portContext);
    }

    return 0;
}

/******************************************************************
 * ���ܣ�
 *     ��ȡ�ص㻧�Ĳ����������Ϣ
 * ������
 *     INT8U *resp  �洢�ص㻧�Ĳ�������ŵ���Ϣ
 *     INT8U max_vip_count  ���֧�ֵ��ص㻧��������resp��buffer��С�����
 * ������
 *     1��ԭ��ֻ��resp�Ĵ��Σ�δ���������ᵼ�³��ָ�������Խ���bug�������ն˸�λ
 *     2��Ҫע�⣬resp�����ֽ������ڴ洢�����ģ����resp������СӦ���ǣ�max_vip_count*2+1��
 * ���ߣ�
 *     zyl by 2017.08.10
 *******************************************************************/
INT16U get_ertu_vip_meters(INT8U *resp,INT8U max_vip_count)
{
  INT16U vip_meters,idx,meter_idx;
  INT8U pos,count;
  INT8U buffer[256]={0};
  BOOLEAN result;

  count = 0;

  fread_ertu_params(EEADDR_SET_F35,buffer,256);
  bit_value_opt_inversion(buffer,256);
  //��ʱ�������˷�ת��1��ʾ��Ч��0��ʾ��Ч
  for(idx = 0;idx < 256;idx++)
  {
      if(buffer[idx] == 00)
          continue;
      for(pos = 0;pos < 8;pos++)
      {
          result = get_bit_value(buffer+idx,1,pos);
          if(result)
          {
              if((idx == 0) && (pos == 0))
                  continue;
              meter_idx = idx * 8 + pos;
              int16u2_bin(meter_idx,(INT8U *)&vip_meters);
              mem_cpy(resp+count*2+1,&vip_meters,2);
              count++;
              if(count >= max_vip_count)
              {
               count = max_vip_count;
               break; //����max_vip_count��������
              }
          }
      }
      if(count == max_vip_count)
      {
          break;
      }
  }

  resp[0] = count;
  #ifdef __PROVICE_JIANGSU__
  get_special_meters_info(resp);
  count = resp[0];
  #endif

  return (count * 2 + 1);
}

INT8U check_vip_meter_available(INT16U meter_idx)
{
    METER_DOCUMENT meter_doc;

       //�����ŵĺ�����
       if(meter_idx == 0  || meter_idx > MAX_METER_SEQ) return FALSE;


       //��ȡ�ص����
       fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));

       //������Ƿ�����ز��˿�
       if((meter_doc.baud_port.port != COMMPORT_PLC)  &&  (meter_doc.baud_port.port != COMMPORT_PLC_REC)) return FALSE;

   return TRUE;
}

void start_read_vip_meter(PLCPortContext* portContext) //Ҫ�ж�һ�½���ǰ�ĳ���״̬�������л�ȥ�����⡣
{

        //���볭������
        #ifdef __SOFT_SIMULATOR__
        snprintf(info,100,"*** PLC_TASK_READ_VIP_METER ***");
        debug_println_ext(info);
        #endif
        //���0��λ���ݣ����浱ǰ������״̬���ָ���ʱ��ʹ��
        mem_set((INT8U*)(&portContext->router_phase_work_info[0].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);

        {
            //��ʼ�����ص��

            readport_plc.OnPortReadData = router_check_urgent_timeout;
            portContext->OnPortReadData = get_read_vip_meter_info;
            portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;

            portContext->params.task_read_data.node_idx[0] = 0;
            portContext->params.task_read_data.node_idx[1] = 0;
            portContext->params.task_read_data.has_fail_meter = FALSE;
        }

}

INT8U prepare_read_vip_item(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U gb_645_frame[260];
    INT8U frame_pos;
    BOOLEAN result;

    portContext = (PLCPortContext*)readportcontext;

    frame_pos = 0;
    if(portContext->router_base_info.router_info4.monitor_afn_type)  //02H-F1
    {
        gb_645_frame[frame_pos++] = meter_protocol_2_router_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol);      //ͨ��Э������
    }
    else  //13H-F1
    {
        gb_645_frame[frame_pos++] = meter_protocol_2_router_protocol(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.protocol);      //ͨ��Э������
        if (portContext->router_base_info.router_info3.rec_delay_flag) gb_645_frame[frame_pos++] = 0x00;      //ͨ����ʱ����Ա�־
        gb_645_frame[frame_pos++] = 0; //�ӽڵ㸽���ڵ�����n
    }
    result = prepare_read_item_curve(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),gb_645_frame+frame_pos+1,gb_645_frame+frame_pos);
    #if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
    if(FALSE == result)
    {
        if(14 == portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_class.user_class)
        {
            result = prepare_read_item_cur_data(&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params),gb_645_frame+frame_pos+1,gb_645_frame+frame_pos);    
        }
    }
    #endif
    if (result)
    {
        if ((portContext->router_base_info.router_info3.rtu_no_mode)
        && (isvalid_meter_addr(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,FALSE)))
        {
                    //����ɼ���ģʽ����
            if(portContext->router_base_info.router_info3.rtu_frame_format)
            {
                        //�޸ı���Ϊ�ɼ���ģʽ�� ����ַ�ӵ���������ײ���
                gb_645_frame[frame_pos] = trans_read_frame_to_cjq_mode(gb_645_frame+frame_pos+1,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no);
            }
               //�ӽڵ�Ϊ�ɼ�����645����Ϊ����ַ������ģʽ��С������Ӧ��. ����ֻ�а���һ�������ɼ���ģʽ��ע����ʾ�ͼ�أ�����֧��
            mem_cpy(portContext->router_work_info.ADDR_DST,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6); //�ӽڵ��ɲɼ���
        }
        else if((portContext->router_base_info.router_info4.dzc_cvt_no_mode)
        && (isvalid_meter_addr(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,FALSE)))
        {
            //����ɼ���ģʽ����
            if(portContext->router_base_info.router_info3.rtu_frame_format)
            {
                //�޸ı���Ϊ�ɼ���ģʽ�� ˮ���ȱ��ַ�ӵ���������ײ���
                gb_645_frame[frame_pos] = trans_read_frame_to_cjq_mode(gb_645_frame+frame_pos+1,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no);
            }
            //�ӽڵ�Ϊ�ɼ�����645����Ϊˮ���ȱ��ַ������ģʽ��С������Ӧ��. ����ֻ�а���һ�������ɼ���ģʽ��ע����ʾ�ͼ�أ�����֧��
            mem_cpy(portContext->router_work_info.ADDR_DST,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.rtu_no,6); //�ӽڵ��ɲɼ���            
        }
        else
        {
         mem_cpy(portContext->router_work_info.ADDR_DST,portContext->router_phase_work_info[portContext->router_work_info.phase].read_params.meter_doc.meter_no,6);
        }

        router_376_2_set_aux_info(0,40,1,TRUE);

        if(portContext->router_base_info.router_info4.monitor_afn_type)
        {
            portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_TRANS,DT_F1,gb_645_frame,gb_645_frame[frame_pos]+frame_pos+1,portContext);
        }
        else
        {
            portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_RTRANS,DT_F1,gb_645_frame,gb_645_frame[frame_pos]+frame_pos+1,portContext);
        }

        portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
        portContext->cur_plc_task_step = PLC_READ_METER_WAIT_RESP_13F1;
        portContext->OnPortReadData = prepare_read_vip_item;
        readport_plc.OnPortReadData = router_wait_resp_frame;
        return portContext->frame_send_Len;
    }
    else
    {
        //����
        
        //portContext->params.task_read_data.has_fail_meter = TRUE;
        portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
        portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
        portContext->OnPortReadData = get_read_vip_meter_info;
        readport_plc.OnPortReadData = router_check_urgent_timeout;
    }
    return 0;
}

INT8U get_read_vip_meter_info(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT16U meter_idx,node_idx;
    INT8U vip_meters[61];//gb_645_frame[260],
    //INT8U *meter_no;

    portContext = (PLCPortContext*)readportcontext;

    if(datetime[MINUTE] > 30 ) goto VIP_METER_READ_END;

    //��ȡ�ص��������
    mem_set(vip_meters,61,0);
    //vip_meters�У���10���������λ�ã��Ǹ���������ʹ�õġ���get_ertu_vip_meters�����������⴦��
    //��ˣ��˴����δ��ݵ������ص㻧����������20��
    get_ertu_vip_meters(vip_meters,MAX_VIP_METERS);  //�ڿ�ʼ��ʱ���Ѿ������һ�Σ�

    node_idx = bin2_int16u(portContext->params.task_read_data.node_idx);

    meter_idx = bin2_int16u(vip_meters+node_idx*2+1);

    if ((meter_idx > MAX_METER_COUNT) )//|| (meter_idx < 0)) //��Χ�жϣ�����ֱ�Ӹ�ֵ�жϣ�1~2040��
    {
        if (portContext->params.task_read_data.has_fail_meter)
        {
            portContext->params.task_read_data.has_fail_meter = FALSE;
            node_idx = 0;
        }
        else
        {
VIP_METER_READ_END:
            //Ъ��
            #ifdef __SOFT_SIMULATOR__
            snprintf(info,100,"*** �ص�����ˣ����� ***");
            debug_println_ext(info);
            #endif

        //       portContext->cur_plc_task = 3;
                //portContext->cur_plc_task = portContext->before_vip_task;
      //          portContext->cur_plc_task_step = PLC_READ_VIP_METER_PREPARE_ITEM;
                portContext->cur_plc_task_step = PLC_VIP_METER_12HF3;
                portContext->OnPortReadData = router_send_afn_12_F3;
                readport_plc.OnPortReadData = router_wait_resp_frame;

                //A��������
                mem_set((INT8U*)(&portContext->router_phase_work_info[0].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);

                portContext->vip_meter_read.vip_cmd_pause = 0;
                portContext->vip_meter_read.vip_cmd_resume = 1;
                portContext->vip_meter_read.vip_cmd_finish = 1;
                return 0;


        }
    }
    else  
    {
        node_idx++;
    }
    int16u2_bin(node_idx,portContext->params.task_read_data.node_idx);

//    if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
    if(check_vip_meter_available(meter_idx))
    {
        portContext->router_work_info.phase = 0;

        if (prepare_read_meter_param(meter_idx,&(portContext->router_phase_work_info[portContext->router_work_info.phase].read_params)))
        {

            portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
            portContext->cur_plc_task_step = PLC_READ_METER_PREPARE_ITEM;
            portContext->OnPortReadData = prepare_read_vip_item;
            readport_plc.OnPortReadData = router_check_urgent_timeout;
            return 0;
        }

    }

    portContext->cur_plc_task = PLC_TASK_READ_VIP_METER;
    portContext->cur_plc_task_step = PLC_READ_METER_FIND_METER;
    portContext->OnPortReadData = get_read_vip_meter_info;
    readport_plc.OnPortReadData = router_check_urgent_timeout;

    return 0;
}

#ifdef __READ_MODULE_ID_PLAN__
/*
��ǰʱ��Ͷ�ȡID�Ļ�׼ʱ���Լ����ڱȽϣ��ó��Ƿ���Ҫ����ģ��ID

params:
    taskDataΪ�ն˵�ǰʱ�� BCD  �� ʱ  �� �� ��
    task_info    �� �� ʱ  �� �� �� 6B + 1B cycle 
*/
BOOLEAN check_task_read_id_datetime(INT8U *taskDate,READ_MODULE_ID_CTRL *task_info,INT8U *last_exec_date)
{
    INT32U pass_min;
    INT16U tmpint;
    INT8U tmp,idx;
    INT8U tmp_date1[6],tmp_date2[6];
    
    //����׼ʱ��
    //����ķ��ͻ�׼ʱ���Լ�����
    //[0]���ڵ�λ|��ʱ��������       BIN      1
    //[1]���ͻ�׼ʱ�䣺��[1] ��[2] ʱ[3] ��[4] ��[5] ��[6]  6  �Ƿ����������ݸ�ʽ1 ?  TODO
    
    
    //���δ��:��ǰ���С�������׼���
    if(task_info->base_time.year > taskDate[4]) return FALSE;
    
    //�·�δ��:����Ѿ�����,��������ͬ,��Ҫ����Ƿ񵽴��׼�·�
    //task_info->base_time.week_month &= 0x1F; //���ݸ�ʽ01 ����-��
    if((task_info->base_time.year == taskDate[4]) && (task_info->base_time.month > taskDate[3])) 
	{
	    return FALSE;
    }
    
    //��δ��:���,�·ݶ��Ѿ�����,��Ҫ������Ƿ��Ѿ����˻�׼��(������ͬ��������)
    if(    (task_info->base_time.year == taskDate[4])
    && (task_info->base_time.month == taskDate[3])
    && (task_info->base_time.day > taskDate[2])  )
    {
        return FALSE;
    }
    
    
    if(task_info->cycle.cycle_cnt == 0)/*Ϊ0��ʱ�򣬱�ʾִֻ��һ��*/
    {
        if((task_info->base_time.year == taskDate[4])
        && (task_info->base_time.month == taskDate[3])
        && (task_info->base_time.day == taskDate[2])
        && (task_info->base_time.hour == taskDate[1])
        && (task_info->base_time.minute == taskDate[0]))
        {
            return TRUE;
        }
        else
        {
           return FALSE;
        }
    }
    
    //�������ڵ�λ�����ж�:
    switch(task_info->cycle.cycle_unit)
    {
        case 0: //���ڵ�λ:����
        if((last_exec_date[0] == taskDate[0]) && (last_exec_date[1] == taskDate[1]) &&
        (last_exec_date[2] == taskDate[2]) && (last_exec_date[3] == taskDate[3]) &&
        (last_exec_date[4] == taskDate[4])) break;
        tmpint = BCD2byte(taskDate[0])+60;
        tmpint -= BCD2byte(task_info->base_time.minute);
        tmp = tmpint % task_info->cycle.cycle_cnt;
        if(tmp == 0)
        {
        //���������ϴ�ִ�е�ʱ��
        mem_cpy(last_exec_date,taskDate,5);
        return TRUE;
        }
        /*
        else
        {
        //���ѹ��ϱ����ڣ����жϺ��ϴ��ϱ���ʱ�������ϴ���δ�ϱ��ɽ����ر�
        tmp_date1[0] = 0;
        tmp_date2[0] = 0;
        mem_cpy(tmp_date1+1,taskDate,5);
        mem_cpy(tmp_date2+1,last_exec_date,5);
        for(idx = 0;idx < 6;idx++)
        {
        tmp_date1[idx] = BCD2byte(tmp_date1[idx]);
        tmp_date2[idx] = BCD2byte(tmp_date2[idx]);
        }
        pass_min = diff_min_between_dt(tmp_date1,tmp_date2);
        if(pass_min > task_info->cycle.cycle_cnt)
        {
        mem_cpy(last_exec_date,taskDate,5);
        return TRUE;
        }
        } */
        break;
        case 1: //���ڵ�λ:Сʱ
        if((last_exec_date[1] == taskDate[1]) && (last_exec_date[2] == taskDate[2]) &&
        (last_exec_date[3] == taskDate[3]) && (last_exec_date[4] == taskDate[4])) break;
        if(taskDate[0]== task_info->base_time.minute)
        {
        //�����׼����
        tmpint = BCD2byte(taskDate[1])+24;
        tmpint -= BCD2byte(task_info->base_time.hour);
        tmp = tmpint % task_info->cycle.cycle_cnt;
        if(tmp == 0)
        {
        //���������ϴ�ִ�е�ʱ��
        mem_cpy(last_exec_date,taskDate,5);
        return TRUE;
        }
        }
        break;
        case 2: //���ڵ�λ:��,ÿ�°�30�����,����ֻ�������ж�
            if((last_exec_date[2] == taskDate[2]) && (last_exec_date[3] == taskDate[3]) &&
               (last_exec_date[4] == taskDate[4])) 
            {
               break;
            }
            if(taskDate[1] < task_info->base_time.hour) 
    		{
    		    break;   //��׼Сʱû�е��
            }
            //�ﵽ��׼Сʱ
            if( taskDate[1] == task_info->base_time.hour)
            {
                //�ոյ���׼Сʱ������Ҫ�ȴ����ӵ���
                if(taskDate[0] < task_info->base_time.minute) break;
                if(taskDate[0] == task_info->base_time.minute)
                {
                    //�յ���׼���ӣ���Ҫ�û�׼��Ҳ��ȥ
                    if(datetime[SECOND]< BCD2byte(task_info->base_time.second)) break;
                }
            }
            //����򳬹���׼�룬����,Сʱ
            tmpint = BCD2byte(taskDate[2])+30;// BCD2HEX(taskDate[2])+30
            tmpint -= BCD2byte(task_info->base_time.day);
            tmp = tmpint % task_info->cycle.cycle_cnt;
            if(tmp == 0)
            {
                //���������ϴ�ִ�е�ʱ��
                mem_cpy(last_exec_date,taskDate,5);
                return TRUE;
            }
            break;
        case 3: //���ڵ�λ: ��,ֻ��Ҫ������1����
            if((last_exec_date[3] == taskDate[3]) && (last_exec_date[4] == taskDate[4])) break;
            if(taskDate[2] < task_info->base_time.day) break;   //��׼��û�е��
            if(taskDate[2] == task_info->base_time.day)
            {
                //�պ��ǻ�׼�գ�������Ҫ�ȴ���׼Сʱ
                if(taskDate[1] < task_info->base_time.hour) break;   //��׼Сʱû�е��
                //�ﵽ��׼Сʱ
                if( taskDate[1] == task_info->base_time.hour)
                {
                    //�պ��ǻ�׼Сʱ��������Ҫ�ȴ����ӵ���
                    if(taskDate[0] < task_info->base_time.minute) break;
                    if(taskDate[0] == task_info->base_time.minute)
                    {
                        //�պ��ǻ�׼���ӣ�����Ҫ�û�׼��Ҳ��ȥ
                        if(datetime[SECOND]< BCD2byte(task_info->base_time.second)) break;
                    }
                }
            }
            //�����׼����,Сʱ,��
            //���������ϴ�ִ�е�ʱ��
            mem_cpy(last_exec_date,taskDate,5);
            return TRUE;
    }
    return FALSE;

}
void check_module_id_task(void)
{
    static INT8U exec_flag = 0xAA;
    INT16U day1 = 0;
	INT16U day2 = 0;
	INT8U unit = 0;
	INT8U BCD_dt[5]= {0};// �� ʱ �� �� ��
	INT8U idx = 0;
	INT8U last_exec_date[5] = {0};// BCD  �� ʱ �� �� ��
    if(portContext_plc.read_status.plc_net) return;
    if(portContext_plc.cur_plc_task == PLC_TASK_PLC_NET) return; //�ѱ��в�Ҫ������Ϣ��

	fread_ertu_params(EEADDR_SET_F305,portContext_plc.read_id_ctrl.value,sizeof(READ_MODULE_ID_CTRL));
    if( (check_is_all_ch(portContext_plc.read_id_ctrl.base_time.bcd_datetime,6,0x00) == TRUE)
		||(check_is_all_ch(portContext_plc.read_id_ctrl.base_time.bcd_datetime,6,0xFF) == TRUE) ) 
   	{
   	    #if defined __AUTO_UPDATE_PHASE_EVERYDAY__ /* �Ĵ�Ҫ���ն���ɼ����ִκ��Զ�������λ��,����ȫ���ſ��� */
        if(portContext_plc.concentrator_read_cycle_no >=2)
        {
            if(portContext_plc.modul_id_read_flag != 0xAA)
            {

                
                if(tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))//���뵽�ź��� �ʹ��� ���򲻴���
                {
                    tpos_enterCriticalSection();
                    if( RECMETER_TASK_NONE == portContext_plc.urgent_task_id )
                    {
                        portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_READ_MODULE_ID; //��������
                    }
                    tpos_leaveCriticalSection();
                    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
                }
            }
        }
        return;
        #endif
   	}

    portContext_plc.read_id_ctrl.base_time.month &= 0x1F; /*ʱ���ʽҪ���ܴ����ڣ������и�����ִ�������*/
    
	for(idx=0;idx<5;idx++)
	{
		//
		BCD_dt[idx] = byte2BCD(datetime[idx+MINUTE]);
	}

	fread_array(FILEID_RUN_DATA,PIM_READ_ID_TASK_LAST_EXEC_DT,last_exec_date,5);
	if( check_task_read_id_datetime(BCD_dt,&(portContext_plc.read_id_ctrl),last_exec_date) == TRUE )
	{
	    fwrite_array(FILEID_RUN_DATA,PIM_READ_ID_TASK_LAST_EXEC_DT,last_exec_date,5);
    	exec_flag = 0x55;
	}
    
    if(exec_flag != 0x55) return;

    if(0 == tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))
    {
        return ;
    }

    tpos_enterCriticalSection();
    if (portContext_plc.urgent_task_id == RECMETER_TASK_NONE) 
    {
        portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_READ_MODULE_ID;
    }
    tpos_leaveCriticalSection();

    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
    if(portContext_plc.urgent_task_id != RECMETER_TASK_TRANS_READ_MODULE_ID)
    {        
        return;  //��������������ռ���ˣ� ����ִ���ˡ�
    }
       
    exec_flag = 0xAA;
}
#endif
/*+++
  ���ܣ����������λ���
  ������cjq_or_meter_check--�ǲɼ���ģʽ���ǵ����Ҫ�ȶԵ�no��һ��

  ���أ�
        TRUE--����ͬ��FALSE--δ�ҵ�
  ������
       1���յ����ĺ�����ϱ���λ��û�������
          ���������λ�Ƿ�����������Ϣ��������λ���¸�ֵ
---*/
BOOLEAN check_other_phase_same_meter_no_after_receive(PLCPortContext *portContext,INT8U *meter_no,INT8U cjq_or_meter_check)
{
  INT8U phase;

  for(phase=0;phase<4;phase++)//�����λ���������
  {
    if(cjq_or_meter_check == 0xAA)  //�ɼ���ģʽ
    {
      if(compare_string(portContext->router_phase_work_info[phase].cjq_info.rtu_no,meter_no,6) == 0)
      {
       portContext->router_work_info.phase = phase;
       return TRUE;
      }
    }
    else
    {
     if(compare_string(meter_no,portContext->router_phase_work_info[phase].read_params.meter_doc.meter_no,6) == 0)
     {
       portContext->router_work_info.phase = phase;
       return TRUE;

     }
    }
  }
  return FALSE;
}
/*+++
  ���ܣ�������λ����ͬ�����0
  ������cjq_or_meter_check--�ǲɼ���ģʽ���ǵ����Ҫ�ȶԵ�no��һ��

  ���أ�
        ��
  ������
       1������ǰ�����һ��������λ�Ƿ�����ͬ��ţ��еĻ���0
---*/
void clear_other_phase_same_meter_no_before_send(PLCPortContext* portContext,INT8U cjq_or_meter_check,INT8U *meter_no)
{

    INT8U phase,idx;

    phase = portContext->router_work_info.phase;

    if(phase > 3) return; //���3��
    for(idx=0;idx<4;idx++)
    {
        if(idx == phase) continue;
        if(cjq_or_meter_check == 0xAA) //�ɼ���ģʽ
        {
          if(compare_string(portContext->router_phase_work_info[idx].cjq_info.rtu_no,meter_no,6) == 0)
          {
              mem_set((INT8U*)(&portContext->router_phase_work_info[idx].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
          }
        }
        else
        {
          if(compare_string(portContext->router_phase_work_info[idx].read_params.meter_doc.meter_no,meter_no,6) == 0)
          {
              mem_set((INT8U*)(&portContext->router_phase_work_info[idx].meter_idx),sizeof(ROUTER_PHASE_WORK_INFO),0x00);
          }
        }
    }

}


void check_trans_task_priority_node(void)
{
    INT16U meter_idx = 0;
    INT8U buffer[256]={0};
    INT8U value[1]={0};

    if (check_is_all_ch(priority_node.map,256,0x00) == FALSE) return;
    fread_array(FILEID_RUN_DATA,FMDATA_F305,buffer,255);
    for(meter_idx=1;meter_idx<=MAX_METER_COUNT;meter_idx++)
    {
        if (get_bit_value(buffer,255,meter_idx-1) == 0)
        {
            fread_array(FILEID_RUN_DATA,FMDATA_TRANS_TASK_PRIORITY_NODE+(meter_idx/8),value,1);
            if ((value[0] & (0x01<<(meter_idx%8))) == 0)
            {
                set_bit_value(priority_node.map,256,meter_idx);
            }
        }
    }

    if ((portContext_plc.batch_meter_ctrl.ctrl_close_event_report == 1) && (check_is_all_ch(priority_node.map,256,0x00)))
    {
        portContext_plc.batch_meter_ctrl.ctrl_close_event_report = 0;
        portContext_plc.batch_meter_ctrl.ctrl_open_event_report = 1;
    }
}

INT8U check_open_event_report(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;

    if ((portContext->batch_meter_ctrl.ctrl_open_event_report) && (portContext->cur_plc_task == PLC_TASK_READ_METER))
    {
        portContext->batch_meter_ctrl.ctrl_open_event_report = 0;

        portContext->params.task_read_data.ctrl_event_report = 1;
        readport_plc.OnPortReadData = router_send_afn_05_F2;
        return 1;
    }
    return 0;
}
//#endif //#ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
#ifdef __INSTANT_FREEZE__
BOOLEAN check_plc_instant_freeze(void)
{
    INT8U cycle_unit,cycle_num;
    static INT8U hour = 0xFF;  //��ǰСʱ��ִ��
    INT8U once_time_flag;
    SET_F234 f234;
    INT8U bin_td[6],bcd_td[6],idx;
    INT8U comm_mode;
    once_time_flag = 0;
    if(hour == 0xFF)
    {
        hour = datetime[HOUR];
    }
  
 // instant_freeze.instant_time               //���û�е�ִ��ʱ�䣬����
    if(instant_freeze.cycle == 0xFF) return FALSE;

    if(instant_freeze.cycle == 0x55) //��F234��ȡֵ������Ƿ�ʱ��
    {
        if(hour == datetime[HOUR]) return FALSE;

        hour = datetime[HOUR];

        fread_ertu_params(EEADDR_SET_F234,f234.value,142);

        if(f234.seg.freeeze_time_count > 20) return FALSE;

        for(idx=0;idx<f234.seg.freeeze_time_count;idx++)
        {
            mem_cpy(bcd_td,f234.seg.rec_timeseg+idx,6);
            buffer_bcd_to_bin(bcd_td,bin_td,6);
           // if(bin_td[3] == 0)
            if(compare_string(bin_td+2,datetime+2,1) == 0)
            {
                /*TODO ��������һ��鿴 */
                if(tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))
                {
                    tpos_enterCriticalSection();
                    if(portContext_plc.urgent_task_id == RECMETER_TASK_NONE)
                    {
                        portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE; //ִ�н�������
                    }
                    tpos_leaveCriticalSection();
                    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
                }
                mem_cpy(instant_freeze_control.instant_start_time,datetime,6);
                set_readport_read_meter_flag_from_fast_index(read_meter_flag_instant_freeze.flag,COMMPORT_PLC);
  
                return TRUE;
            }
  
        }

    }

    cycle_unit = instant_freeze.cycle & 0xC0 ;
    cycle_num =  instant_freeze.cycle & 0x3F ;
    if(cycle_num == 0)
    {
        if((instant_freeze.instant_time[1] == byte2BCD(datetime[MINUTE]))
        && (instant_freeze.instant_time[2] == byte2BCD(datetime[HOUR]))
        && (instant_freeze.instant_time[3] == byte2BCD(datetime[DAY]))
        && (instant_freeze.instant_time[5] == byte2BCD(datetime[YEAR])))
        {
            once_time_flag = 1;
        }
        cycle_num = 0xAA;
    
    
    }

    #ifdef __READ_0001FF00_DAYHOLD_FREEZE__
    if((instant_freeze.instant_time[1] == byte2BCD(datetime[MINUTE]))  //���⴦�������ı�ʾÿ��ִ��һ�Σ�00��00��
    && (instant_freeze.instant_time[2] == byte2BCD(datetime[HOUR]))
    && (instant_freeze.instant_time[3] == 0x99)
    && (instant_freeze.instant_time[5] == 0x99))
    {
        once_time_flag = 1;
    }
    cycle_num = 0xAA;
    
    
    if(once_time_flag)
    {
        hour = datetime[HOUR];
        if(tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))
        {
            tpos_enterCriticalSection();
            if(portContext_plc.urgent_task_id == RECMETER_TASK_NONE)
            {
                portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE; //ִ�н�������
            }
            tpos_leaveCriticalSection();
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
        }
        mem_cpy(instant_freeze_control.instant_start_time,datetime,6);
        set_readport_read_meter_flag_from_fast_index(read_meter_flag_instant_freeze.flag,COMMPORT_PLC);
    }
    #else
    if(((datetime[MINUTE] % cycle_num == 0)  && (cycle_unit == 0))
    || ((cycle_unit == 0x40) && ((datetime[HOUR] % cycle_num == 0)) && (datetime[HOUR] != hour))
    || once_time_flag)
    //  || ((cycle_unit == 0x40) && ((datetime[HOUR] % cycle_num == 0)) && (datetime[HOUR] != hour)) ) //��д��ֻ���˷���
    {
        hour = datetime[HOUR];
        if(tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))
        {
            tpos_enterCriticalSection();
            if(portContext_plc.urgent_task_id == RECMETER_TASK_NONE)
            {
                portContext_plc.urgent_task_id = RECMETER_TASK_TRANS_CAST_INSTANT_FREEZE; //ִ�н�������
            }
            tpos_leaveCriticalSection();
            tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
        }
        mem_cpy(instant_freeze_control.instant_start_time,datetime,6);
        comm_mode = portContext_plc.router_base_info.router_info1.comm_mode;
        if(comm_mode != 2)/* topscomm's bplc need set flag 5 minutes later */
        {
            set_readport_read_meter_flag_from_fast_index(read_meter_flag_instant_freeze.flag,COMMPORT_PLC);
        }
    }
    #endif
    //����ǰ���δ���ڣ�Ҫ�ж�һ��ֻ��ִ��һ�Σ�ִ�й��˾Ͳ�Ҫ�ظ�ִ��

    return TRUE;
}
#endif
#ifdef __MESSAGE_SEND_RECEIVE_RECORD__
/*
 �洢��ʽ�� ʱ��3�ֽ�+���ڳ�������4+����4+����2,+�쳣2
 * ͸���ģ�
 
 */
void message_send_and_receive_num_record(INT16U meter_idx,INT8U send_or_receive)
{
    INT8U send_num[15],td[3],a[4];
    INT32U send_num_tmp,offset;//���ķ��ʹ���
    
     mem_set(a,4,0);
    if((send_or_receive == 0x55) || (send_or_receive == 0xAA))//���ͼ�¼
    {
      mem_cpy(td,datetime+3,3);
      offset = getPassedDays(2000+td[2],td[1],td[0]);
      offset = offset % SAVE_POINT_NUMBER_DAY_HOLD;
      offset *= 15;
      offset += PIM_DAY_HOLD_MESS_CYCLE_SEND_AND_RECEIVE_NUM;
      fread_array(meter_idx,offset, send_num,15);

      if(compare_string(td,send_num,3) == 0)
      {
          if(send_or_receive == 0x55)//���ڳ�������
          {
                send_num_tmp =  bin2_int32u(send_num+3);
                send_num_tmp ++;
                mem_cpy(send_num+3,&send_num_tmp,4);
          }
          else//����
          {
                send_num_tmp =  bin2_int32u(send_num+7);
                send_num_tmp ++;
                mem_cpy(send_num+7,&send_num_tmp,4);
          }
      }
      else
      {
        mem_cpy(send_num,td,3);
        mem_set(send_num+3,12,0);
        send_num[3] = 1;
      }
      fwrite_array(meter_idx,offset,send_num,15);
    }
    else if((send_or_receive == 0x66) || (send_or_receive == 0x77)) //͸���ķ���
    {
       mem_cpy(td,datetime+3,3);
      offset = getPassedDays(2000+td[2],td[1],td[0]);
      offset = offset % SAVE_POINT_NUMBER_DAY_HOLD;
      offset *= 15;
      offset += PIM_DAY_HOLD_MESS_TRAN_RECEIVE_NUM;
      fread_array(meter_idx,offset, send_num,15);

      if(compare_string(td,send_num,3) == 0)
      {
          if(send_or_receive == 0x66)//���ڳ�������
          {
                send_num_tmp =  bin2_int32u(send_num+3);
                send_num_tmp ++;
                mem_cpy(send_num+3,&send_num_tmp,4);
          }
          else//����
          {
            send_num_tmp =  bin2_int32u(send_num+7);
            send_num_tmp ++;
            mem_cpy(send_num+7,&send_num_tmp,4);
          }
      }
      else
      {
        mem_cpy(send_num,td,3);
        mem_set(send_num+3,12,0);
        send_num[3] = 1;
      }
      fwrite_array(meter_idx,offset,send_num,15);
    }


}
#endif
#ifdef __COUNTRY_ISRAEL__
void check_read_curve(void)
{
  static day = 0;
  if(day != datetime[DAY])/*���жϣ������º��ꡣ���յ�һ�����Ӳ�Ҫִ�С���ֹ����ʱ����������ִ�е��磬allow_read_curve����AA.*/
  {
      portContext_plc.success_rate = 0;
      day =  datetime[DAY];
      return;
  }

  if(israel_curve_read.hour_curve_dayhold_rate >100) /*�����FF*/
  {
      israel_curve_read.hour_curve_dayhold_rate = 95;
  }

  if((portContext_plc.success_rate >= israel_curve_read.hour_curve_dayhold_rate) || (portContext_plc.success_rate ==1))
  {
      if(israel_curve_read.allow_read_curve != 0xAA)   //����ϵ��1����������˳����������Ƿ�������
      {
         //��Ҫ����һ�£��������˲�Ҫ��������
         portContext_plc.need_reset_router = 1;
         israel_curve_read.allow_read_curve = 0xAA;
      }

  }
  else
  {
      israel_curve_read.allow_read_curve = 0x55;
  }

}
#endif
#ifdef __PLC_EVENT_READ__
void check_plc_event_task(void)
{
   INT8U bitpos;
   INT8U event_set[16];
   INT16U meter_num;

      meter_num = get_readport_meter_count_from_fast_index(COMMPORT_PLC);
      if(meter_num ==1)
      {
 	if((datetime[MINUTE] % 2 == 0 ))
	{
		set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
        }
      }

      if((datetime[HOUR] == 0x14) && (datetime[MINUTE] == 0))
      {
      set_readport_read_meter_flag_from_fast_index(read_meter_flag_cur_data,COMMPORT_PLC);
      portContext_plc.need_reset_router = 1;
      }

}
#endif
/*
BOOLEAN check_ertu_special_meters(void)
{
  METER_DOCUMENT meter_doc;
  INT16U idx,meter_idx;
  INT8U pos,count;
  INT8U buffer[256];
  BOOLEAN result,need_change;

  count = 0;
  need_change = 0;

  fread_ertu_params(EEADDR_SPECIAL_READ_IDX,buffer,256);
  bit_value_opt_inversion(buffer,256);
  //��ʱ�������˷�ת��1��ʾ��Ч��0��ʾ��Ч
  for(idx = 0;idx < 256;idx++)
  {
      if(buffer[idx] == 00)
          continue;
      for(pos = 0;pos < 8;pos++)
      {
          result = get_bit_value(buffer+idx,1,pos);
          if(result)
          {
              if((idx == 0) && (pos == 0))
                  continue;
              meter_idx = idx * 8 + pos;

                     //��ȡ������ȷ���Ƿ��ǿ��˱�
              fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_doc,sizeof(METER_DOCUMENT));
              if( meter_doc.meter_class.user_class != 6 )
              {
                clr_bit_value(buffer,256,idx*8 + pos);//������λ��
                need_change = TRUE;
                continue;
              }
              count++;
          }
      }
  }
  if(need_change)
  {
  bit_value_opt_inversion(buffer,256);
  fwrite_ertu_params(EEADDR_SPECIAL_READ_IDX,buffer,256);
  }

  return count;
}
 */
INT8U get_vip_meter_count(void)
{
	INT16U count,idx;
	INT8U  pos;
	INT8U  buffer[256] = {0};
	INT8U  result = 0;

	count = 0;

	fread_ertu_params(EEADDR_SET_F35,buffer,256);
	bit_value_opt_inversion(buffer,256);
	//��ʱ�������˷�ת��1��ʾ��Ч��0��ʾ��Ч
	for(idx = 0;idx < 256;idx++)
    {
    	if(buffer[idx] == 00)
    	{
    	    continue;
    	}
    	for(pos = 0;pos < 8;pos++)
    	{
        	result = get_bit_value(buffer+idx,1,pos);
        	if(result)
        	{
            	if((idx == 0) && (pos == 0))
            	{
            	    continue;
            	}
            
            	count++;
        	}
    	}
    }
	if(count >20) count = 0; //����20�����쳣

	return count;
}

INT8U check_special_meters_need_read(void)
{
    //FAST_INDEX fast_index;
    #if ( (defined __PROVICE_HUNAN__) || (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__) )
    METER_CLASS meter_class;
    #endif
    INT16U node_count;
    INT16U idx;
    //INT8U meter_port;
    //INT8U seq_spec[2];


    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
    for(idx=0;idx<node_count;idx++)
    {
        tpos_enterCriticalSection();
        //seq_spec[0]=fast_index_list.fast_index[idx].seq_spec[0];
        //seq_spec[1]=fast_index_list.fast_index[idx].seq_spec[1];
        //meter_port=fast_index_list.fast_index[idx].port;
        #if ( (defined __PROVICE_HUNAN__) || (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__) )
        meter_class.value = fast_index_list.fast_index[idx].user_class;
        #endif
        tpos_leaveCriticalSection();
        #if (defined __PROVICE_HUNAN__)
        if(meter_class.user_class == 6 ) return TRUE; //���޵Ĵ����6����Ҫ����
        #else
        #if (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
        if(meter_class.user_class == 14) return TRUE;// ���� ʣ�����������
      //  #else
      //  return FALSE;
        #endif //__JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__
        #endif //__PROVICE_HUNAN__
    }
    
    return FALSE;
}

INT16U get_special_meters_info(INT8U *data)
{
    //FAST_INDEX fast_index;
    #if ( (defined __PROVICE_HUNAN__) || (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__) )
    METER_CLASS meter_class;
    INT8U meter_port = 0;
    INT8U seq_spec[2] = {0};
    #endif
    INT16U node_count;
    INT16U idx;
    INT8U count;
    

    node_count = 0;
    count = data[0];

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;
    for(idx=0;idx<node_count;idx++)
    {
        tpos_enterCriticalSection();
        #if ( (defined __PROVICE_HUNAN__) || (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__) )
        seq_spec[0]=fast_index_list.fast_index[idx].seq_spec[0];
        seq_spec[1]=fast_index_list.fast_index[idx].seq_spec[1];
        meter_port=fast_index_list.fast_index[idx].port;
        meter_class.value = fast_index_list.fast_index[idx].user_class;
        #endif
        tpos_leaveCriticalSection();

        #if (defined __PROVICE_HUNAN__)
        if(meter_class.user_class == 6) //�������޴���6 ����
        {
            if((meter_port == COMMPORT_PLC)  ||  (meter_port == COMMPORT_PLC_REC))
            {
                data[count*2+1] = seq_spec[0];
                data[count*2+2] = seq_spec[1];
                count ++;
            }
        }
        #elif (defined __JIANGSU_RESIDUAL_CURRENT_TRANSFORMER__)
        if(meter_class.user_class == 14)// ����14  ���⴦��
        {
            if( (meter_port == COMMPORT_PLC) || (meter_port == 28) )
            {
                data[count*2+1] = seq_spec[0];
                data[count*2+2] = seq_spec[1];
                count ++;
                if(count >= 30)
                {
                    break;
                }
            }
        }
        #else

        #endif
    
    }
    if(count > 30) count =30;
    
    data[0] = count;
    
    return      count*2+1;
}
/*
void clear_curve_read_ok_flag(void)
{
    INT8U port;
    INT16U node_count,idx,meter_idx;
    port =  COMMPORT_PLC;
    
    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    for(idx=0;idx<node_count;idx++)
    {
        if (fast_index_list.fast_index[idx].port > port) break;
        if (port != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

                   //��������߲����㣬��Ҫ�ж�һ��flag_curve
           if(get_bit_value(jzq_need_read_curve_node.map,256,meter_idx))
           {

               {
                    set_bit_value(jzq_read_ok_node.map,256,meter_idx);  // ���߲����㣬��������ִ����ɲ������
               }

           }

    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);


}
*/
#ifdef __NGRID_HARDWARE_II__
void get_cy_voltage(void)
{
    extern void save_curve_midu(INT16U meter_idx,INT32U offset,INT8U midu,BOOLEAN is_min);
    extern INT32U get_curve_save_offset(READ_WRITE_DATA *phy,INT8U td[5],INT8U midu);
    INT16U tmp;
    INT8U td[5],midu,v_buffer[50];
//    INT8U port,record_cycle,block_count;
    READ_WRITE_DATA phy;
    INT32U offset;//seq
    METER_DOCUMENT meter_no;
    INT32U phy_v=0x00001E00;
//    INT16U datalen,tmp,block_begin_idx;
//    C1F25     F25;
    STAT_SET  stat_set_value;  //ͳ����ֵ�趨ֵ
    SPOT_STAT var;
    INT8U sys_datetime[10];
    INT16U meter_idx;

    os_get_datetime(sys_datetime);
    mem_set(v_buffer,50,REC_DATA_IS_DENY);

    //#if defined (__NGRID_HARDWARE_II__)
    //��������1Ϊ���ɣ���ÿ15������һ�δ洢
    meter_idx = get_ac_meter_idx();
    if(meter_idx != 0xFFFF)
    {
        fread_array(meter_idx,PIM_METER_DOC,(INT8U *)&meter_no,sizeof(METER_DOCUMENT));
        if((COMMPORT_485_CY == meter_no.baud_port.port) && ((sys_datetime[MINUTE]%15)== 0))
        {
            mem_cpy(td,sys_datetime+MINUTE,5);
            midu=15;
            get_phy_form_list_cruve(phy_v,&phy);

            offset = get_curve_save_offset(&phy,td,midu);
            tmp = ger_ertu_cur_voltage();
            if(tmp <= 0x270F)//���֧��9999�����������������壬�����洢
            {
                mem_cpy(v_buffer,td,5);
                ul2bcd(tmp,v_buffer+5,2);
                fwrite_array(meter_idx,offset,v_buffer,phy.data_len);
            }
            offset = PIM_CURVE_V_I;
            save_curve_midu(meter_idx,offset,midu,FALSE);
        }
    }       
}
#endif
void plc_router_roam_reg_node_save_file(INT8U phase,INT8U *meter_no,INT8U node_prop,INT8U *rtu_no,INT8U* erc35_buffer)
{
    PLC_ROAM_METER plc_roam_meter;
    INT16U count = 0;
    INT16U meter_idx = 0;
    INT16U save_pos = 0;
    INT8U router_protocol = 0;
    BOOLEAN isexsit;

    #ifdef __PROVICE_JIANGSU__
    if(rtu_no != NULL)
    {
        rtu_no[5] = 0;      /*���ղɼ�����ַΪ10λ����λ��0*/
    }
    #endif

    /*������ַ�Ǵ���*/
    isexsit = memory_fast_index_find_node_no(READPORT_PLC,meter_no,&meter_idx,&save_pos,&router_protocol,NULL);

    if(isexsit)
    {

        if((meter_idx & FAST_IDX_RTU_FLAG) && (meter_idx & FAST_IDX_MASK) == 0) 
        {
            isexsit = FALSE;
        }
    }

    /*�յĲɼ����������*/
    if ((rtu_no != NULL) && (FALSE == isexsit))
    {

        if ((compare_string(meter_no,rtu_no,5) == 0) && (meter_no[5] > 0x99)) return;/*�㽭���⴦����λ����0��ֻ�ȶ�5���ֽ�*/
    }

    /*�浽����ļ���*/
    mem_set(plc_roam_meter.value,sizeof(PLC_ROAM_METER),0x00);
    mem_cpy(plc_roam_meter.meter_no,meter_no,6);
    if(node_prop==3)
    {
        plc_roam_meter.protocol = GB_OOP;
    }
    else
    {
        plc_roam_meter.protocol = (node_prop==2) ?  GB645_2007 : GB645_1997;
    }
    plc_roam_meter.phase = phase;
    if(rtu_no != NULL)
    {
        mem_cpy(plc_roam_meter.rtu_no,rtu_no,6);
    }
    mem_cpy(plc_roam_meter.stamp,datetime+SECOND,6); /*��,��¼�����ϱ���ʱ��*/

    fread_array(FILEID_ROUTER_ROAM_REPORT_INFO,0,(INT8U*)&count,2);
    if(count > MAX_METER_COUNT)
    {
        count = 0;
    }
    fwrite_array(FILEID_ROUTER_ROAM_REPORT_INFO,2+sizeof(PLC_ROAM_METER)*count,plc_roam_meter.value,sizeof(PLC_ROAM_METER));
    count++;
    fwrite_array(FILEID_ROUTER_ROAM_REPORT_INFO,0,(INT8U*)&count,2);
}
INT8U active_node_distinguish(PLCPortContext *portContext)
{

    INT16U plc_net_last_minute_tmp;
    INT8U cur_datetime[6];

   //���ü���ӽڵ�����ע�����
    plc_net_last_minute_tmp = bin2_int16u(portContext->params.task_plc_net.last_minute); //��Ҫ�жϺͶ�ʱ�ѱ�Ĺ�ϵ����

    if(plc_net_last_minute_tmp > 200 ) //ʱ�䲻��̫��������200���ӣ�����Ĭ��8�㣬���������11:30���ң�
    plc_net_last_minute_tmp = 200 ;

    file_delete(FILEID_AREA_DIFFERENT_DATA);
    
    portContext->plc_net_wait_time_s = plc_net_last_minute_tmp*60; //·�ɵȴ�ʱ��ת������

    mem_set(portContext->params.task_plc_net.start_time,10,0x00);
    cur_datetime[0] = byte2BCD(datetime[SECOND]);
    cur_datetime[1] = byte2BCD(datetime[MINUTE]);
    cur_datetime[2] = byte2BCD(datetime[HOUR]);
    cur_datetime[3] = byte2BCD(datetime[DAY]);
    cur_datetime[4] = byte2BCD(datetime[MONTH]);
    cur_datetime[5] = byte2BCD(datetime[YEAR]);
    mem_cpy(portContext->params.task_plc_net.start_time,cur_datetime,6);
    portContext->params.task_plc_net.last_minute[0] = plc_net_last_minute_tmp;
    portContext->params.task_plc_net.last_minute[1] = plc_net_last_minute_tmp>>8;

    portContext->params.task_plc_net.run_params.is_force_exit = 0;
    portContext->plc_net_time_out_10ms = os_get_systick_10ms();
    readport_plc.OnPortReadData = router_send_afn_05_F6;
    portContext->OnPortReadData = router_send_afn_05_F6;
    portContext->cur_plc_task = PLC_TASK_PLC_NET;
    portContext->cur_plc_task_step = PLC_NET_ACTIVE_NODE_LOGON_11_F5;

	return 0;
}

INT8U router_send_afn_14_F2_no_change_status(objReadPortContext * readportcontext)
{
    INT8U cur_datetime[6];

    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    cur_datetime[0] = byte2BCD(datetime[SECOND]);
    cur_datetime[1] = byte2BCD(datetime[MINUTE]);
    cur_datetime[2] = byte2BCD(datetime[HOUR]);
    cur_datetime[3] = byte2BCD(datetime[DAY]);
    cur_datetime[4] = byte2BCD(datetime[MONTH]);
    cur_datetime[5] = byte2BCD(datetime[YEAR]);

    router_send_3762_frame_no_change_status(portContext,DL69842_AFN_REQUEST,DT_F2,cur_datetime,6);

    return 0;
}
/*+++
  ���ܣ������л�״̬��ֱ�ӷ���3762����
  ������
        AFN��FN��frame,frame_len
  ���أ�
       ���ݳ��ȳ���10���ֽڣ�ֱ�ӷ���0��
       �����������1
  ������
       1�����л�֮ǰ��״̬����ֱ�Ӵӵײ㷢�͡�
       2��������͹����У�·��һֱæµ������300ms�˳���
---*/
INT8U router_send_3762_frame_no_change_status(PLCPortContext * portContext,INT8U afn,INT16U fn,INT8U *data_frame,INT8U data_len)
{
    INT32U time_tick;
    INT8U cs = 0;
    INT8U idx;

    time_tick = os_get_systick_10ms();
    if(data_len > 35)return 0; /*ȫ�ֱ�������50���ֽڣ������ݲ�֧�֣��������������ܳ���35*/

    while(1)
    {
        #ifdef __SOFT_SIMULATOR__
        if(time_elapsed_10ms(portContext->router_resp_time_out) > 30)   //300ms
        #else
        if(drv_plc_is_idle() == TRUE)
        #endif
        {
             // 68 0F 00 41 00 00 28 00 00 0B F0 80 0D F1 16

             mem_set(portContext->frame_00HF1_send,21,0);
             portContext->frame_00HF1_send[0] = 0x68;

             //������:   ����ʽ�ز�·��
            if (portContext->router_base_info.router_info1.comm_mode == 2)
            {
                portContext->frame_00HF1_send[POS_DL69842_CTRL] = DL69842_CTRL_ROUTER_BPLC;
            }
            else if (portContext->router_base_info.router_info1.comm_mode == 3)
            {
                portContext->frame_00HF1_send[POS_DL69842_CTRL] = DL69842_CTRL_ROUTER_WIFI;
            }
            else
            {
                portContext->frame_00HF1_send[POS_DL69842_CTRL] = DL69842_CTRL_ROUTER;
            }

              //��Ϣ��
             router_376_2_set_aux_info(portContext->router_work_info.channel_id,40,0,FALSE);

             mem_cpy(portContext->frame_00HF1_send + POS_DL69842_INFO,portContext->router_work_info.AUX_INFO,6);

             portContext->frame_00HF1_send[POS_DL69842_AFN] = afn;
             portContext->frame_00HF1_send[POS_DL69842_FN] = fn;
             portContext->frame_00HF1_send[POS_DL69842_FN+1] = fn >>8;

             if(data_frame != NULL)
             {
                  mem_cpy(portContext->frame_00HF1_send+POS_DL69842_DATA,data_frame,data_len);
             }
             else
             {
                  data_len = 0;
             }

             for(idx=POS_DL69842_CTRL;idx<(POS_DL69842_DATA+data_len);idx++)
             {
                 cs  += portContext->frame_00HF1_send[idx];
             }
             portContext->frame_00HF1_send[POS_DL69842_DATA+data_len] = cs;

             portContext->frame_00HF1_send[POS_DL69842_DATA+1+data_len] = 0x16;

             portContext->frame_00HF1_send[POS_DL69842_LEN] = (POS_DL69842_DATA+ 2+ data_len);

             PlcChannel.channel_send(portContext->frame_00HF1_send,portContext->frame_00HF1_send[POS_DL69842_LEN],NULL);

             #ifdef __SOFT_SIMULATOR__
             snprintf(info,100,"%02d-%02d-%02d %02d:%02d:%02d:%03d [send] : ",datetime[YEAR],datetime[MONTH],datetime[DAY],datetime[HOUR],datetime[MINUTE],datetime[SECOND],(datetime[MSECOND_H]<<8) | datetime[MSECOND_L]);
             GetHexMessage(portContext->frame_00HF1_send,portContext->frame_00HF1_send[POS_DL69842_LEN],info+31);
             frameInfo(portContext->frame_00HF1_send,info+31+portContext->frame_00HF1_send[POS_DL69842_LEN]*3);
             debug_println_ext(info);
             #endif
             if(is_debug_enabled() && is_monitor_frame(MONTIOR_3762))
             {
                 record_log_frame(portContext->frame_00HF1_send,portContext->frame_00HF1_send[POS_DL69842_LEN],LOG_DEBUG);
             }

             while(1)
             {
                if(drv_plc_is_idle() == TRUE) /*�ֱ���˿��У�˵�����������*/
                {
                   break;
                }
                else
                {
                   if(time_elapsed_10ms(time_tick) > 30) /*300ms��û�����꣬�Ͳ�Ҫ���ˣ��˳�*/
                   {
                       break;
                   }
                }
             }
             
             break;
        }
        else
        {
            if(time_elapsed_10ms(time_tick) < 30)
            {
                DelayNmSec(10);
            }
            else
            {
                break;
            }

        }
    }

    return 1;
}

/*-----------------------------------------
���ܣ���������
       �������һ���ѱ���ɺ󣬺͵����еĵ��ȶԡ�
       û���ѵ��ĵ���¼�ERC64

-------------------------------*/
#ifdef __PROVICE_JIANGXI__
 void plc_net_state_jiangxi(objReadPortContext * readportcontext)
{

	PLCPortContext *portContext;
    PLC_NET_METER *plc_net_meter_new;

    INT16U 	count_z_old,count_z_new,idx,idx1,idx2,need_read_time,meter_info_once_get_max,meter_idx,current_read_meter_info_num,pos_idx;
	INT8U 	new_file_idx;
    INT8U   document_no[6]={0};
    INT8U   find_result;
    INT8U   no_find_num = 0,protocol;
    INT8U   data[110]={0};
    INT8U   map_temp[256] = {0};

    portContext = (PLCPortContext*)readportcontext;

    count_z_old = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    if( (count_z_old > MAX_METER_COUNT) || (count_z_old == 0) )
    {
		return;
    }

    new_file_idx = portContext->params.task_plc_net.run_params.file_idx;
	fread_array(FILEID_PLC_NET+new_file_idx,0,(INT8U*)&count_z_new,2);
    if( (count_z_new > MAX_METER_COUNT))
    {
        count_z_new = 0;
    }

    meter_info_once_get_max = (4096/sizeof(PLC_NET_METER)); /*����g_temp_buffer����һ�������������ٸ������Ϣ*/
    if(meter_info_once_get_max == 0)
    {
        meter_info_once_get_max = 1;
    }

    need_read_time = count_z_new/meter_info_once_get_max ;/*���������Ҫ�����ٴ��ļ������ܽ����е����ѯһ�飬*/
    if(count_z_new % meter_info_once_get_max != 0) /*��ֹ�պö�������������+1*/
    {
        need_read_time ++ ;
    }

    tpos_mutexPend(&SIGNAL_TEMP_BUFFER);

    for(idx1=0;idx1<need_read_time;idx1++)
    {

        if(meter_info_once_get_max >= (count_z_new - (idx1*meter_info_once_get_max)) )  /*���������Ҫ���������������  */
        {
            current_read_meter_info_num = (count_z_new - (idx1*meter_info_once_get_max));  /*���ʣ�µĲ�����������ˣ��Ͷ�ʣ�µļ���*/
        }
        else
        {

            current_read_meter_info_num =  meter_info_once_get_max;
        }

        fread_array(FILEID_PLC_NET+new_file_idx,2+idx1*meter_info_once_get_max*sizeof(PLC_NET_METER),g_temp_buffer,(current_read_meter_info_num*sizeof(PLC_NET_METER)));
        plc_net_meter_new = (PLC_NET_METER*)(g_temp_buffer);

        for(idx2=0;idx2<current_read_meter_info_num;idx2++)          /*��ǰg_temp_buffer����ı�Ŷ�����һ��*/
        {
            if(memory_fast_index_find_node_no(COMMPORT_PLC,plc_net_meter_new->meter_no,&meter_idx,&pos_idx,&protocol,NULL))
            {
                meter_idx &= FAST_IDX_MASK;
                set_bit_value(map_temp,256,meter_idx);
            }

            plc_net_meter_new ++;   /*��һ���ѱ�ṹ��*/

        }

    }
    tpos_mutexFree(&SIGNAL_TEMP_BUFFER);

    tpos_mutexPend(&SIGNAL_FAST_IDX);
  	for(idx=0;idx<count_z_old;idx++) /*�����ȡ���ӿ��������������Ȼ����map_temp��ǵĽ���ȶ�*/
	{

        if (fast_index_list.fast_index[idx].port > READPORT_PLC) break;
        if (READPORT_PLC != fast_index_list.fast_index[idx].port) continue;
        meter_idx = bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_MASK;
        if ((meter_idx == 0) || (meter_idx > MAX_METER_COUNT)) continue;

        if(get_bit_value(map_temp,256,meter_idx))/*��������㱾�α���������*/
        {
            continue;
        }
        else
        {
            mem_cpy(document_no,fast_index_list.fast_index[idx].node,6);
			protocol = (fast_index_list.fast_index[idx].router_protocol - 1); /*Э��*/
			if(protocol > 3)
			{
		    	protocol = 2;
			}

            mem_cpy(data+no_find_num*7,document_no,6);
			data[no_find_num*7+6] = protocol;
			no_find_num ++;

        }

		if(no_find_num >= 15) /*����15��Ҫ�ϱ�һ�Σ�����erc64��û���õ����������ź���*/
		{
		    event_erc64(data,no_find_num);
		    no_find_num = 0;
		}
	}
    tpos_mutexFree(&SIGNAL_FAST_IDX);

    if(no_find_num >0)//����15������ȫ������һ����
    {
        event_erc64(data,no_find_num);
    }

}
#endif

#ifdef __PLC_BPLC_AGG__
BOOLEAN check_exec_agg_read(tagAggInfo *aggPar,INT8U dt[5])
{
    BOOLEAN flg = FALSE;
    INT8U  val = aggPar->cycle.val;
    INT8U idx= 0;
    if( (FALSE == is_valid_bcd(aggPar->base_time,5)) || (check_is_all_ch(aggPar->base_time,5,0x00)) )
    {
        return 0;
    }

    for(idx=0;idx<5;idx++)
    {
        aggPar->base_time[idx] = BCD2byte(aggPar->base_time[idx]);
    }
    switch(aggPar->cycle.unit)
    {
        case 0:/* minute ��֧�� */
            break;
        case 1: /* hour  */
            if( (val>=1) && (val<= 23) )
            {
                if (0 == (dt[1]%val) )
                {
                    if(dt[0] == aggPar->base_time[0])/* ������ͬ */
                    {
                        flg = TRUE;
                    }
                }
            }
            break;
        case 2:/* day */
            if( (val>=1) && (val<= 31) )
            {
                if (0 == (dt[2]%val) )
                {
                    if(0 == compare_string(dt,aggPar->base_time,2))/* ������ͬ */
                    {
                        flg = TRUE;
                    }
                }
            }
            break;
        case 3: /* month  ��֧�� */
            break;
    }

    return flg;
}
BOOLEAN check_plc_aggregation_task(objReadPortContext * readportcontext)
{
	PLCPortContext *portContext;
	tagAggInfo aggPar;
    static INT8U exec_flag = 0xAA;
    static INT8U last_minute[5] = {0x0};

    if(compare_string(last_minute,datetime+MINUTE,5) != 0)
    {
        mem_cpy(last_minute,datetime+MINUTE,5);
    }
    else
    {
        return FALSE;
    }
    portContext = (PLCPortContext*)readportcontext;
    if (portContext->router_base_info.router_vendor != ROUTER_VENDOR_TOPSCOMM)
    {
        return FALSE;  /* ֻ����Զ���·�� */
    }
    if(portContext->read_status.plc_net)
    {
        return FALSE;
    }
    if(portContext->cur_plc_task == PLC_TASK_PLC_NET)
    {
        return FALSE; /* �ѱ��в���ѯ?? */
    }

    if( (1 == portContext->router_base_info.router_info1.comm_mode) )/* խ�� */
    {
        return FALSE;
    }
    #if 0
    /* 1��ִ��һ��  */
    
    if ( (last_minute[0] == 0) && (last_minute[1] == 1) )
    {
        /*  */
        exec_flag = 0x55;
    }
    #endif
    /* �����׼ʱ�� */
    tpos_enterCriticalSection();
    mem_cpy(aggPar.value,agg_par.value,6);
    tpos_leaveCriticalSection();
    if(check_exec_agg_read(&aggPar,last_minute))
    {
        exec_flag = 0x55;
    }
    /* խ���ѱ��ִ��һ�� TODO:: */
    if (1 == portContext->aggregate_flg )
    {
        /*  */
        portContext->aggregate_flg = 0;
        exec_flag = 0x55;
    }
    
    if(exec_flag != 0x55)
    {
        return FALSE;
    }
    if(0 == tpos_mutexRequest(&SIGNAL_PLC_METER_MONITOR))
    {
        return FALSE;
    }

    tpos_enterCriticalSection();
    if (portContext->urgent_task_id == RECMETER_TASK_NONE)
    {
        portContext->urgent_task_id = RECMETER_TASK_GET_AGGREGATION;
    }
    tpos_leaveCriticalSection();

    tpos_mutexFree(&SIGNAL_PLC_METER_MONITOR);
    if(portContext->urgent_task_id != RECMETER_TASK_GET_AGGREGATION)
    {        
        return FALSE;  //��������������ռ���ˣ� ����ִ���ˡ�
    }
    exec_flag = 0xAA;
    return TRUE;
}
#endif

#if defined(__SHANXI_READ_BPLC_NETWORK_INFO__)
INT8U router_send_afn_F0_F100(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_DEBUG,DT_F100,NULL,0,portContext);

    readport_plc.OnPortReadData = router_urgent_task_send_idle;   //�ڽ��������г���

    return portContext->frame_send_Len;
}

INT8U router_recv_afn_F0_F100(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT32U offset;
    C2_F231 c2_f231;
    INT8U pos;

    portContext = (PLCPortContext*)readportcontext;

    //��������
    c2_f231.read_date[0] = ((datetime[MINUTE]/15)*15);
    c2_f231.read_date[1] = (datetime[HOUR]);
    c2_f231.read_date[2] = (datetime[DAY]);
    c2_f231.read_date[3] = (datetime[MONTH]);
    c2_f231.read_date[4] = (datetime[YEAR]);

    pos = portContext->recv_data_pos;

    mem_cpy(c2_f231.network_info,portContext->frame_recv+pos,19);


    offset = getPassedDays(2000+c2_f231.read_date[4],c2_f231.read_date[3],c2_f231.read_date[2]);
    offset = offset % 10;
    offset *= sizeof(C2_F231)*96;
    offset += c2_f231.read_date[1]*4*sizeof(C2_F231);
    offset += (c2_f231.read_date[0]/15)*sizeof(C2_F231);
    fwrite_array(FILEID_BPLC_NETWORK_INFO,offset,c2_f231.value,sizeof(C2_F231));

    portContext->urgent_task = PLC_TASK_URGENT_TASK;
    readport_plc.OnPortReadData = router_send_afn_F0_F102;
    portContext->urgent_task_step = PLC_URGENT_BPLC_CCO_INFO_READ;
}
//·�ɲ�ѯ��F102���ӽڵ���Ϣ,����Ų�ѯ
INT8U router_send_afn_F0_F102(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;
    
    INT8U temp[3] = {0};
    temp[0] = 0x01;
    temp[1] = 0x00;
    temp[2] = 0x01;

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_DEBUG,DT_F102,temp,3,portContext);

   // readport_plc.OnPortReadData = router_wait_resp_frame;  //5s�Ļ���
   readport_plc.OnPortReadData = router_urgent_task_send_idle ;

    return portContext->frame_send_Len;

}
//·�ɲ�ѯ��F102���ӽڵ���Ϣ,����Ų�ѯ
INT8U router_recv_afn_F0_F102(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT32U offset;
    C2_F232 c2_f233;
    INT8U pos,index;

    portContext = (PLCPortContext*)readportcontext;

    //��������
    c2_f233.read_date[0] = ((datetime[MINUTE]/15)*15);
    c2_f233.read_date[1] = (datetime[HOUR]);
    c2_f233.read_date[2] = (datetime[DAY]);
    c2_f233.read_date[3] = (datetime[MONTH]);
    c2_f233.read_date[4] = (datetime[YEAR]);

    pos = portContext->recv_data_pos+6;  //2+2+2,

    mem_cpy(c2_f233.bplc_cco_info,portContext->frame_recv+pos,53);


    offset = getPassedDays(2000+c2_f233.read_date[4],c2_f233.read_date[3],c2_f233.read_date[2]);
    offset = offset % 10;
    offset *= sizeof(C2_F232)*96;
    offset += c2_f233.read_date[1]*4*sizeof(C2_F232);
    offset += (c2_f233.read_date[0]/15)*sizeof(C2_F232);
    fwrite_array(FILEID_BPLC_CCO_INFO,offset,c2_f233.value,sizeof(C2_F232));

    portContext->urgent_task = PLC_TASK_URGENT_TASK;
    readport_plc.OnPortReadData = router_send_afn_F0_F103;
    portContext->urgent_task_step = PLC_URGENT_BPLC_STA_INFO_READ;

}
//·�ɲ�ѯ��F103���ӽڵ���Ϣ
INT8U router_send_afn_F0_F103(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT16U node_count;
    INT16U seq_idx = 0;
    INT8U temp[7] = {0};

    portContext = (PLCPortContext*)readportcontext;

    tpos_mutexPend(&SIGNAL_FAST_IDX);

    node_count = (fast_index_list.count > MAX_METER_COUNT) ? 0 : fast_index_list.count;

    seq_idx = bin2_int16u(portContext->bplc_network_info_start_idx);

    if(seq_idx >= node_count)
    {
        seq_idx = 0;
        portContext->bplc_network_info_start_idx[0] = 0;
        portContext->bplc_network_info_start_idx[1] = 0;
            //����״̬������ȴ���һ�����������״̬
        urgent_task_in_wait_next_urgent_task(portContext);
        tpos_mutexFree(&SIGNAL_FAST_IDX);
        return 0;
    }

    while(seq_idx < node_count)
    {

        //��ȡ�ز��ӽڵ��ַ,���
        if (fast_index_list.fast_index[seq_idx].port == READPORT_PLC)
        {
            break;
        }
        else
        {
            seq_idx ++ ;
            continue ;
        }

    }
    router_376_2_set_aux_info(0,40,0,TRUE);
    
    temp[0] = 1;
    mem_cpy(temp+1,fast_index_list.fast_index[seq_idx].node,6);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_DEBUG,DT_F103,temp,7,portContext);
    portContext->bplc_network_info_start_idx[0] = (INT8U)(seq_idx & 0xFFFF);
    portContext->bplc_network_info_start_idx[1] = (INT8U)(seq_idx >> 8);

    
    tpos_mutexFree(&SIGNAL_FAST_IDX);

   // readport_plc.OnPortReadData = router_wait_resp_frame;
   readport_plc.OnPortReadData = router_urgent_task_send_idle ;

    return portContext->frame_send_Len;
}
//·�ɲ�ѯ��F103���ӽڵ���Ϣ
INT8U router_recv_afn_F0_F103(objReadPortContext * readportcontext)
{
    extern INT32U get_curve_save_offset(READ_WRITE_DATA *phy,INT8U td[5],INT8U midu);
    PLCPortContext* portContext;
    READ_WRITE_DATA phy;
    INT32U offset = 0;
    INT16U idx = 0;
    INT16U meter_idx,pos_idx;
    INT8U router_protocol,user_class;
    BOOLEAN is_exist;
    INT8U meter_no[6];
    INT8U buffer[60]= {0};
    INT8U td[7]={0};
    INT8U td_former[7]={0};
    INT8U ver_info[3] ={0};
    INT8U i=0;
    INT8U phase;
    
    is_exist = FALSE;

    portContext = (PLCPortContext*)readportcontext;

    idx = bin2_int16u(portContext->bplc_network_info_start_idx);
    tpos_enterCriticalSection();
    mem_cpy(meter_no,fast_index_list.fast_index[idx].node,6);
    tpos_leaveCriticalSection();


    if (memory_fast_index_find_node_no(COMMPORT_PLC,meter_no,&meter_idx,&pos_idx,&router_protocol,&user_class))
    {
        meter_idx &= FAST_IDX_MASK;
        if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
        {
            is_exist = TRUE;
        }
    }
    else//�����Ǽ������������в�ѯ�����ֲ����ڵ����ʱ��·�ɿ��ܳ������⵼�·��ر���쳣
    {

        readport_plc.OnPortReadData = router_send_afn_F0_F103;
        portContext->urgent_task_step = PLC_URGENT_BPLC_STA_INFO_READ;
        return 0;
    }

    if(is_exist)
    {

        //�������������
        phy.offset  = PIM_CURVE_BPLC_STA_INFO;
        phy.phy =  BPLC_STA_INFO;
        phy.data_len = 58; //5+53

        mem_cpy(td,datetime+MINUTE,5);

        //���td�������µ�15���ӵ㣬��ֹ�г����ģ�
        if( (td[0] % 15) != 0)
        {
           i = td[0] /15;

           td[0] = i*15;
        }
        INT8U midu = 15;
        fwrite_array(meter_idx,phy.offset,&midu,1);
        offset = get_curve_save_offset(&phy,td,midu);
        fread_array(meter_idx,offset,buffer,phy.data_len);

        //����ʱ��
        if(compare_string(buffer,td,5) != 0)
        {
            mem_set(buffer,phy.data_len,0xFF);
            mem_cpy(buffer,td,5);

            mem_cpy(buffer+5,portContext->frame_recv+portContext->recv_data_pos+2,53);
        }
        else
        {

        }
        //û���жϷ��ر��ĵĸ�ʽ��
       fwrite_array(meter_idx,offset,buffer,phy.data_len);

        phase = buffer[19];
        mem_cpy(ver_info,buffer+40,3);
        buffer_bin_to_bcd(td,td_former,5);
        td_former[5] = 1;
        get_former_curve_td(td_former,1);
        buffer_bcd_to_bin(td_former,td,5);

        offset = get_curve_save_offset(&phy,td,midu);
        fread_array(meter_idx,offset,buffer,phy.data_len);
        if(buffer[0] != 0xFF)//�������ٱȽ�
        {
            if(phase != buffer[19])
            {
               if((phase == 0) || (buffer[19] == 0))
               {
                 //��λ0�������¼�
               }
               else
               {
                  bplc_event_erc_62(meter_idx,phase,buffer[19]);
               }
            }

            if(compare_string(ver_info,buffer+40,3) != 0) //��һ��
            {
               if((check_is_all_ch(ver_info,3,0)) || (check_is_all_ch(buffer+40,3,0)))
               {
                     //�汾��0���������¼�
               }
               else
               {
                  bplc_event_erc_63(meter_idx,ver_info,buffer+40);
               }
            }
        }

    }

    idx++;
    portContext->bplc_network_info_start_idx[0] = idx;
    portContext->bplc_network_info_start_idx[1] = (idx>>8);
    
    readport_plc.OnPortReadData = router_send_afn_F0_F103;
    portContext->urgent_task_step = PLC_URGENT_BPLC_STA_INFO_READ;

    return 0;

}
/*�ϱ��ӽڵ�������Ϣ  */
INT8U router_recv_afn_06_F10(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U idx,count;
    INT8U node[6] = {0};
    INT8U report_buffer[200] = {0};/*�����¼�ʱʹ�ã��¼����һ��д��15������Ϣ */
    INT8U  pos = 0;
    INT8U line_on_or_off = 0;

    portContext = (PLCPortContext*)readportcontext;

    portContext->plc_net_timer_10ms = os_get_systick_10ms();
    pos = portContext->recv_data_pos;

    pos += 2; /*������ */

    count = portContext->frame_recv[pos++];
    if(count > 16) count = 16;/*̨�����ֵı�һ�ν���16���������ٿ��� */
    pos += 2;        /*�����ϱ������  */


       for(idx=0;idx<count;idx++)
       {
         mem_cpy(node, portContext->frame_recv + pos,6);
         line_on_or_off = portContext->frame_recv[pos + 6];
         pos += 12;    /* 6���ڵ��ַ+6����Ϣ */

         bplc_sta_offline_report(1,node,report_buffer,line_on_or_off);
       }

       bplc_sta_offline_report(0,NULL,report_buffer,line_on_or_off);

       router_send_afn_00_F1_no_change_status(readportcontext);  /*��ȷ�ϣ���������  */

    return 0;
}

/*+++
  ���ܣ�ERC60 �ڵ����߼�¼
  ����:
        INT8U status      ��ʾ��ǰ֡�Ƿ������
       INT8U node_no[6]  �ڵ��ַ, ����

  ������

---*/
void bplc_sta_offline_report(INT8U status,INT8U node_no[6],INT8U* buffer,INT8U line_on_or_off)
{
    INT16U meter_idx,pos_idx,spot_idx;
    INT8U event_flag,router_protocol;//
    INT8U meter_count;
    void save_event_record(INT8U *event,INT8U event_flag);

    event_flag = check_event_prop(ERC60);
    if (!event_flag ) return;

    #ifdef __SOFT_SIMULATOR__
    INT8U count=0,flag=0;;
    snprintf(info,100,"event_unconfirm_meter: status=%d  flag=%02X  count=%d",status,flag,count);
    debug_print(info);
    #endif

    //��ʼ���¼�ͷ
    if (buffer[EVENT_POS_ERC_NO] != ERC60)
    {
        buffer[EVENT_POS_ERC_NO] = ERC60;
        buffer[9] = 0;   //count
    }

    meter_count =  buffer[EVENT_POS_ERC_METER_COUNT]; /*buffer�����Ѿ�����ķ����¼��ı�����*/

    if(status == 1)
    {
        if (memory_fast_index_find_node_no(COMMPORT_PLC,node_no,&meter_idx,&pos_idx,&router_protocol,NULL))
        {
            meter_idx &= FAST_IDX_MASK;
            if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
            {
                fread_array(meter_idx,PIM_METER_DOC+2,(INT8U*)&spot_idx,2);
                if ((spot_idx == 0) || (spot_idx > MAX_METER_COUNT)) spot_idx = 0;
            }
        }
        else
        return;
    }

    //�¼��������
    if (status == 0)
    {
        if (buffer[EVENT_POS_ERC_NO] != ERC60) return;
        if (meter_count == 0) return;
        if (meter_count > 15) return;

        //�����¼���¼ ERC60
        buffer[EVENT_POS_ERC_LEN] = meter_count*12+1+1;
        save_event_record(buffer,event_flag);
        mem_set(buffer+EVENT_POS_ERC_NO,9,0x00);
    }
    else if (status == 1)
    {
        set_event_datetime(buffer+EVENT_POS_ERC_METER_CONTENT+12*meter_count);  //ÿ����������5�ֽڷ���ʱ��+2�ֽڲ�����+5�ֽ����һ�ε���ʱ��
        buffer[EVENT_POS_ERC_METER_CONTENT+12*meter_count+5] = spot_idx;
        buffer[EVENT_POS_ERC_METER_CONTENT+12*meter_count+6] = spot_idx >> 8;
        if(line_on_or_off == 1)
        {
          buffer[EVENT_POS_ERC_METER_CONTENT+12*meter_count+6] |= (0x10); /*bit4λ��1����ʾ����*/
        }
        else
        {
           buffer[EVENT_POS_ERC_METER_CONTENT+12*meter_count+6] &= (0xEF);  /*bit4λ���㣬��ֹ֮ǰ��λ���в���*/
        }
        set_event_datetime(buffer+EVENT_POS_ERC_METER_CONTENT+12*meter_count+7);

        buffer[EVENT_POS_ERC_METER_COUNT]++;
        meter_count = buffer[EVENT_POS_ERC_METER_COUNT];

        if(meter_count >= 15)
        {
            //�����¼���¼ ERC60
            buffer[EVENT_POS_ERC_LEN] = 15*12+1+1;  //��һ���ֽ���Ϊ�˴洢��
            buffer[EVENT_POS_ERC_LEN+1] = 0;   //���������ֽ�
            save_event_record(buffer,event_flag);
            mem_set(buffer+EVENT_POS_ERC_NO,9,0x00);
        }
    }

}
void save_area_sta_change_info(INT8U *frame)
{
    INT16U meter_idx,spot_idx,pos_idx;
    INT8U area_info[7]={0};
    INT8U idx_33,router_protocol;

    spot_idx = 0;

    mem_cpy(area_info,frame+POS_GB645_METERNO,6);

    if (memory_fast_index_find_node_no(COMMPORT_PLC,area_info,&meter_idx,&pos_idx,&router_protocol,NULL))
    {
        meter_idx &= FAST_IDX_MASK;
        if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
        {
            fread_array(meter_idx,PIM_METER_DOC+2,(INT8U*)&spot_idx,2);
            if ((spot_idx == 0) || (spot_idx > MAX_METER_COUNT)) spot_idx = 0;
        }

        mem_cpy(area_info,frame+POS_GB645_ITEM+1,7); //1���ֽ�AREA+6���ֽ�̨����ַ ��
        for (idx_33=0;idx_33<7;idx_33++)
        {
            area_info[idx_33] = area_info[idx_33] - 0x33;
        }

        //�¼�
        bplc_event_erc_61((INT8U*)&spot_idx,area_info);
    }
}

INT8U router_send_afn_F0_F111(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_DEBUG,DT_F111,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

INT8U router_send_afn_F0_F112(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_DEBUG,DT_F112,NULL,0,portContext);

    readport_plc.OnPortReadData = router_wait_resp_frame;

    return portContext->frame_send_Len;
}

#else
#ifdef __PROVICE_SHAANXI__
/*�ϱ��ӽڵ�������Ϣ  */
INT8U router_recv_afn_06_F10(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;
    INT8U idx,count;
    INT8U node[40] = {0};
    INT8U report_buffer[300] = {0};/*�����¼�ʱʹ�ã��¼����һ��д��6������Ϣ */
    INT8U  pos = 0;
    INT8U line_on_or_off = 0;

    portContext = (PLCPortContext*)readportcontext;

    portContext->plc_net_timer_10ms = os_get_systick_10ms();
    pos = portContext->recv_data_pos;

    pos += 2; /*������ */

    count = portContext->frame_recv[pos++];
    if(count > 6) count = 6;/*̨�����ֵı�һ�ν���6���������ٿ��� */
    pos += 2;        /*�����ϱ������  */


       for(idx=0;idx<count;idx++)
       {
         mem_cpy(node, portContext->frame_recv + pos,6);
         line_on_or_off = portContext->frame_recv[pos + 6];
         set_event_datetime(node+6);
         mem_cpy(node+11,portContext->frame_recv+pos+7, 29);
         pos += 36;    /* 6���ڵ��ַ+1���仯+4����ʱ�䳤+1+24оƬ��Ϣ */

         bplc_sta_offline_report(1,node,report_buffer,line_on_or_off);
       }

       bplc_sta_offline_report(0,NULL,report_buffer,line_on_or_off);

       router_send_afn_00_F1_no_change_status(readportcontext);  /*��ȷ�ϣ���������  */

    return 0;
}

/*+++
  ���ܣ�ERC58 �ڵ����߼�¼
  ����:
        INT8U status      ��ʾ��ǰ֡�Ƿ������
       INT8U node_no[6]  �ڵ��ַ, ����

  ������

---*/
void bplc_sta_offline_report(INT8U status,INT8U *node_no,INT8U* buffer,INT8U line_on_or_off)
{
    INT16U meter_idx,pos_idx,spot_idx;
    INT8U event_flag,router_protocol;//
    INT8U meter_count;
    void save_event_record(INT8U *event,INT8U event_flag);

    event_flag = check_event_prop(ERC58);
    if (!event_flag ) return;

    #ifdef __SOFT_SIMULATOR__
    INT8U count=0,flag=0;;
    snprintf(info,100,"event_unconfirm_meter: status=%d  flag=%02X  count=%d",status,flag,count);
    debug_print(info);
    #endif

    //��ʼ���¼�ͷ
    if (buffer[EVENT_POS_ERC_NO] != ERC58)
    {
        buffer[EVENT_POS_ERC_NO] = ERC58;
        buffer[9] = 0;   //count
    }

    meter_count =  buffer[EVENT_POS_ERC_METER_COUNT]; /*buffer�����Ѿ�����ķ����¼��ı�����*/

    if(status == 1)
    {
        if (memory_fast_index_find_node_no(COMMPORT_PLC,node_no,&meter_idx,&pos_idx,&router_protocol,NULL))
        {
            meter_idx &= FAST_IDX_MASK;
            if ((meter_idx > 0) && (meter_idx <= MAX_METER_COUNT))
            {
                fread_array(meter_idx,PIM_METER_DOC+2,(INT8U*)&spot_idx,2);
                if ((spot_idx == 0) || (spot_idx > MAX_METER_COUNT)) spot_idx = 0;
            }
        }
        else
        return;
    }

    //�¼��������
    if (status == 0)
    {
        if (buffer[EVENT_POS_ERC_NO] != ERC58) return;
        if (meter_count == 0) return;
        if (meter_count > 3) return;

        //�����¼���¼ ERC58
        buffer[EVENT_POS_ERC_LEN] = meter_count*47+1+1;
        save_event_record(buffer,event_flag);
        mem_set(buffer+EVENT_POS_ERC_NO,9,0x00);
    }
    else if (status == 1)
    {
        set_event_datetime(buffer+EVENT_POS_ERC_METER_CONTENT+47*meter_count);  //ÿ����������5�ֽڷ���ʱ��+2�ֽڲ�����+6�ֽڵ�ַ+5�ֽ����һ�ε���ʱ��+4�ֽ�����ʱ��+1�ֽ�����ԭ��+24�ֽ�оƬID
        buffer[EVENT_POS_ERC_METER_CONTENT+47*meter_count+5] = spot_idx;
        buffer[EVENT_POS_ERC_METER_CONTENT+47*meter_count+6] = spot_idx >> 8;
        if(line_on_or_off == 1)
        {
          buffer[EVENT_POS_ERC_METER_CONTENT+47*meter_count+6] |= (0x10); /*bit4λ��1����ʾ����*/
        }
        else
        {
           buffer[EVENT_POS_ERC_METER_CONTENT+47*meter_count+6] &= (0xEF);  /*bit4λ���㣬��ֹ֮ǰ��λ���в���*/
        }

        mem_cpy(buffer+EVENT_POS_ERC_METER_CONTENT+47*meter_count+7,node_no,40);

        buffer[EVENT_POS_ERC_METER_COUNT]++;
        meter_count = buffer[EVENT_POS_ERC_METER_COUNT];

        if(meter_count >= 3)
        {
            //�����¼���¼ ERC58
            buffer[EVENT_POS_ERC_LEN] = 3*47+1+1;  //��һ���ֽ���Ϊ�˴洢��
            buffer[EVENT_POS_ERC_LEN+1] = 0;   //���������ֽ�
            save_event_record(buffer,event_flag);
            mem_set(buffer+EVENT_POS_ERC_NO,9,0x00);
        }
    }

}
#else
/*�ϱ��ӽڵ�������Ϣ  */
INT8U router_recv_afn_06_F10(objReadPortContext * readportcontext)
{

    router_send_afn_00_F1_no_change_status(readportcontext);  /*��ȷ�ϣ���������  */

    return 0;
}
#endif
#endif
/*+++
  ���ܣ��ڿ��������в��ҵ����ŵĿ�������
  ������

  ����:  �û���С���

---*/
INT8U memory_fast_index_find_by_meter_seq(INT16U meter_seq)
{
    INT16U idx,meter_count;
    INT8U user_class=0xFF;
    INT8U seq_spec[2]={0};

    tpos_mutexPend(&SIGNAL_FAST_IDX);
    meter_count = fast_index_list.count;
    if(meter_count > MAX_METER_COUNT)
    {
        meter_count = 0;
    }

    for(idx=0;idx<meter_count;idx++)
    {
        tpos_enterCriticalSection();
        seq_spec[0]=fast_index_list.fast_index[idx].seq_spec[0];
        seq_spec[1]=fast_index_list.fast_index[idx].seq_spec[1];
        tpos_leaveCriticalSection();

        if(!file_exist(meter_seq)) continue;
        if(meter_seq == (bin2_int16u(seq_spec) & FAST_IDX_MASK))
        {
            user_class = fast_index_list.fast_index[idx].user_class;
            break;
        }
    }

    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return user_class;
}

/*+++
  ���ܣ��������������ȶ��нڵ�
  ������

  ����:

---*/
#ifdef __BATCH_TRANSPARENT_METER_TASK_USE_PRIORITY_NODE__
void creat_priority_map_jibei(void)
{
    INT16U idx,meter_count;
    INT16U meter_idx;
    tpos_mutexPend(&SIGNAL_FAST_IDX);

    meter_count = fast_index_list.count;
    if(meter_count > MAX_METER_COUNT)
    {
        meter_count = 0; 
    }
    for(idx=0;idx<meter_count;idx++)
    {
        if(COMMPORT_PLC == fast_index_list.fast_index[idx].port)
        {
            /* С���2�������ʹ����F�Ĺ���� */
            if( ( (fast_index_list.fast_index[idx].user_class & 0x0F) == 0x02)
              ||( (fast_index_list.fast_index[idx].user_class & 0xF0) == 0xF0) )
            {
                meter_idx  = bin2_int16u(fast_index_list.fast_index[idx].seq_spec);
                meter_idx &= FAST_IDX_MASK;
                if( (meter_idx >=1) && (meter_idx <= MAX_METER_COUNT) )
                {
                    set_bit_value(priority_node.map,256,meter_idx);
                }
            }
        }
    }
    tpos_mutexFree(&SIGNAL_FAST_IDX);
    return ;
}
#endif
#if (defined __MEXICO_VALVA_OPERATED_SWITCH__)
BOOLEAN check_ctrl_switch_trip(objReadPortContext * readportcontext)
{
    PLCPortContext *portContext;

    portContext = (PLCPortContext*)readportcontext;

    if(0 == portContext->router_interactive_status.meter_doc_synchro_done)
    {
        return FALSE;
    }
    if(0xAA == gSystemInfo.terminal_manage)
    {
        return FALSE;
    }
    if(0xAA == portContext->switch_trip_flag)
    {
        tpos_enterCriticalSection();
        if (portContext_plc.urgent_task_id == RECMETER_TASK_NONE) 
        {
            portContext_plc.urgent_task_id = RECMETER_TASK_SWITCH_TRIP_CTRL;
        }
        tpos_leaveCriticalSection();
        return TRUE;
    }
    return FALSE;
}

/*
9010��ȡ����״̬
*/
INT16U make_read_switch_stat_frame(INT8U *frame,INT8U *meter_no)
{
    //
    INT16U fcs = 0;
    INT8U idx,pos;
    INT8U cs;
    INT8U tmp_meter_no[6] = {0};
    pos=0;
    frame[pos++] = 0x68;
    frame[pos++] = meter_no[0];
    frame[pos++] = meter_no[1];
    frame[pos++] = meter_no[2];
    frame[pos++] = meter_no[3];
    frame[pos++] = meter_no[4];
    frame[pos++] = meter_no[5];
    frame[pos++] = 0x68;
    frame[pos++] = 0x01;
    frame[pos++] = 0x02; //len
    frame[pos++] = 0x10; 
    frame[pos++] = 0x90; // 0xEA 0xCC ��բ
    
    for(idx=10;idx<pos;idx++) frame[idx]+=0x33;
    cs = 0;
    for(idx=0;idx<pos;idx++) cs+=frame[idx];
    frame[pos++]=cs;
    frame[pos++]=0x16;
    return pos;
}

INT16U make_switch_ctrl_frame(INT8U *frame,INT8U *meter_no)
{
    //
    INT16U fcs = 0;
    INT8U idx,pos;
    INT8U cs;
    INT8U tmp_meter_no[6] = {0};
    pos=0;
    frame[pos++] = 0x68;
    frame[pos++] = meter_no[0];
    frame[pos++] = meter_no[1];
    frame[pos++] = meter_no[2];
    frame[pos++] = meter_no[3];
    frame[pos++] = meter_no[4];
    frame[pos++] = meter_no[5];
    frame[pos++] = 0x68;
    frame[pos++] = 0x04;
    frame[pos++] = 0x04; //len
    frame[pos++] = 0xCC; 
    frame[pos++] = 0xEA; // 0xEA 0xCC ��բ

    //
    mem_cpy_reverse(tmp_meter_no,meter_no,6);
    for(idx=0;idx<6;idx++)
    {
        if(tmp_meter_no[idx] == 0xA4)
        {
            tmp_meter_no[idx] = 0x68;
        }
        else
        {
            tmp_meter_no[idx] = BCD2byte(tmp_meter_no[idx]);
        }
    }
    fcs = tcik081_ctrl_keyword(tmp_meter_no);
    frame[pos++] = (INT8U)fcs;
    frame[pos++] = (INT8U)(fcs >> 8);
   
    for(idx=10;idx<pos;idx++) frame[idx]+=0x33;
    cs = 0;
    for(idx=0;idx<pos;idx++) cs+=frame[idx];
    frame[pos++]=cs;
    frame[pos++]=0x16;
    return pos;
}
BOOLEAN prepare_plc_switch_trip_ctrl(objReadPortContext * readportcontext,INT8U *frame,INT8U *framelen)
{
    PLCPortContext *portContext;
    READPORT_METER_DOCUMENT *meter_doc = NULL;
    INT16U meter_idx = 0;
    INT16U idx = 0;
    BOOLEAN find_flag = FALSE;
    portContext = (PLCPortContext*)readportcontext;

    //tpos_mutexPend(&SIGNAL_FAST_IDX);
    //portContext->router_phase_work_info[0].meter_idx = bin2_int16u(fast_index_list.fast_index[portContext->switch_idx].seq_spec);
    //portContext->router_phase_work_info[0].meter_idx &= FAST_IDX_MASK;
    //tpos_mutexFree(&SIGNAL_FAST_IDX);

    tpos_mutexPend(&SIGNAL_FAST_IDX);
    for(idx= portContext->switch_idx;idx<fast_index_list.count;idx++)
    {
        // �����12  97Э�� �˿��ز�
        if( (0xC0 == (fast_index_list.fast_index[idx].user_class & 0xC0)) 
          && (GB645_1997 == fast_index_list.fast_index[idx].router_protocol)
          && (READPORT_PLC == fast_index_list.fast_index[idx].port) )
        {
            // û����ӱ�־��ʱ�򣬲�ȥ����բ������ӱ�־�� �������� TODO ?????
            if( 0 ==(bin2_int16u(fast_index_list.fast_index[idx].seq_spec) & FAST_IDX_ADD_ROUTER_FLAG) )
            {
                find_flag = TRUE;
                portContext->switch_idx = idx;
                portContext->router_phase_work_info[0].meter_idx = bin2_int16u(fast_index_list.fast_index[portContext->switch_idx].seq_spec);
                portContext->router_phase_work_info[0].meter_idx &= FAST_IDX_MASK;
                break;
            }
        }
    }
    tpos_mutexFree(&SIGNAL_FAST_IDX);

    if(FALSE == find_flag)
    {
        return FALSE;
    }
    
    meter_idx = portContext->router_phase_work_info[0].meter_idx;
    meter_doc = &(portContext->router_phase_work_info[0].read_params.meter_doc);
    if(FALSE == prepare_read_meter_doc(meter_idx,meter_doc))
    {
        //
        return FALSE;
    }

    #if (defined __CTRL_READ_SWITCH__)
    
    #else
    // ��ʱ��ô�����  
    *framelen = make_switch_ctrl_frame(frame,meter_doc->meter_no);
    #endif
    return TRUE;    
}
#endif
#ifdef __PLC_BPLC_AGG__
INT8U router_send_afn_F0_F16(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;

    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_DEBUG,DT_F16,portContext->agg_ctl.bplc.node_addr,6,portContext);

    readport_plc.OnPortReadData = router_urgent_task_send_idle;   //�ڽ��������г���

    return portContext->frame_send_Len;
}
INT8U router_recv_afn_F0_F16(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT32U offset;
    INT8U  *ptr;
    INT16U fileid;
    INT16U grp_idx;
    INT16U node_idx;
    INT16U pos = 0;
    INT16U grp_index = 0;/* ID ����������λ�� */
    INT16U grp_cnt = 0;
    CLUSTER_INFO      clust_info;
    AGGREGATION_INFO  agg_info;
    INT8U sub_idx;
    BOOLEAN node_flg = FALSE;
    BOOLEAN flg = FALSE;

    portContext = (PLCPortContext*)readportcontext;
    pos = portContext->recv_data_pos;
    mem_cpy(clust_info.value,portContext->frame_recv+pos,sizeof(CLUSTER_INFO));

    if(portContext->aggregate_id > 1)
	{
		/* log error and exit */
		urgent_task_in_wait_next_urgent_task(portContext);
	}
	
    fileid  = FILEID_ROUTER_AGGREGATION_INFO + portContext->aggregate_id;
    offset = PIM_AGGREGATION_INFO_START;
    tpos_mutexPend(&SIGNAL_TEMP_BUFFER);
    ptr = g_temp_buffer;
    fread_array(fileid,offset,ptr,PIM_AGGREGATION_GRP_INFO_START);
    pos = 2;/* �ҵ�ID��Ϣ */

    grp_cnt = bin2_int16u(ptr);
    if(grp_cnt == 0xFFFF)
    {
        grp_cnt = 0;
    }
    for(grp_idx =0;grp_idx< MAX_METER_COUNT;grp_idx++)
    {
        /*  */
        if(0 == compare_string(ptr + pos + grp_idx * 2 ,clust_info.id,2))
        {
            grp_index = grp_idx; 
            flg = TRUE;
            break;
        }
    }

    /* δ�ҵ�����ID����Ҫ�����Ƿ��п���ռ� */
    if(FALSE == flg)
    {
        flg = FALSE;
        for(grp_idx =0;grp_idx< MAX_METER_COUNT;grp_idx++)
        {
            /*  */
            if(check_is_all_FF(ptr+pos+grp_idx * 2 ,2))
            {
                grp_index = grp_idx;
                mem_cpy(ptr+pos+grp_idx * 2,clust_info.id,2);
                flg = TRUE;
                break;
            }
        }
        
        if(FALSE == flg)/* �޿���ռ� return  */
        {
            /* �洢����?? TODO log_info */
            return 0;
        }

        /* ���±���ID����Ϣ */
        grp_cnt++;
        ptr[0] = grp_cnt;
        ptr[1] = grp_cnt >> 8;
        fwrite_array(fileid,offset,ptr,PIM_AGGREGATION_GRP_INFO_START);

        /* ���±�������Ĵӽڵ��ַ��Ϣ  ���³�1��  */
        offset  = grp_index * PIM_AGGREGATION_GRP_INFO_SIZE;
        offset +=  PIM_AGGREGATION_GRP_INFO_START;
        mem_set(agg_info.value,sizeof(AGGREGATION_INFO),0xFF);
        agg_info.seq[0] = grp_index;
        agg_info.seq[1] = grp_index << 8;
        agg_info.cnt    = 1;
        mem_cpy(agg_info.node_addr+0,clust_info.addr,6);
        fwrite_array(fileid,offset,(INT8U *)&agg_info,sizeof(AGGREGATION_INFO));        
    }
    else
    {
        offset  = grp_index * PIM_AGGREGATION_GRP_INFO_SIZE;
        offset +=  PIM_AGGREGATION_GRP_INFO_START;
        fread_array(fileid,offset,(INT8U *)&agg_info,sizeof(AGGREGATION_INFO));
        if(bin2_int16u(agg_info.seq) == grp_index)
        {
            /*  */
            node_flg = FALSE;
            for(sub_idx = 0 ;sub_idx < 36;sub_idx++)
            {
                if(0 == compare_string(agg_info.node_addr[sub_idx],clust_info.addr,6))
                {
                    node_flg = TRUE;
                    break;
                }
            }
            if(FALSE == node_flg)
            {
                /*  */
                #if 0
                for(sub_idx = 0 ;sub_idx < 36;sub_idx++)
                {
                    if(TRUE == check_is_all_FF(agg_info.node_addr+sub_idx,6))
                    {
                        mem_cpy();
                        node_flg = TRUE;
                        break;
                    }
                }
                #endif
                if(agg_info.cnt < 36)
                {
                    mem_cpy(agg_info.node_addr+agg_info.cnt,clust_info.addr,6);
                    agg_info.cnt++ ;
                    fwrite_array(fileid,offset,(INT8U *)&agg_info,sizeof(AGGREGATION_INFO));
                }
                else
                {
                    /* log info  ???TODO: */
                }
            }
        }
        else /* ����seq  cnt  and addr ��Ϣ */
        {
            agg_info.seq[0] = grp_index;
            agg_info.seq[1] = grp_index << 8;
            agg_info.cnt    = 1;
            mem_cpy(agg_info.node_addr+0,clust_info.addr,6);
            fwrite_array(fileid,offset,(INT8U *)&agg_info,sizeof(AGGREGATION_INFO));
        }
    }
    tpos_mutexFree(&SIGNAL_TEMP_BUFFER);

    node_idx = bin2_int16u(portContext->agg_ctl.bplc.node_idx);
    node_idx++;
    portContext->agg_ctl.bplc.node_idx[0] = node_idx;
    portContext->agg_ctl.bplc.node_idx[1] = node_idx >> 8;
    if(TRUE == get_node_addr_from_fast_index(COMMPORT_PLC,&(portContext->agg_ctl)) )
    {
        readport_plc.OnPortReadData   = router_send_afn_F0_F16;
		portContext->urgent_task      = PLC_TASK_URGENT_TASK;
		portContext->urgent_task_step = PLC_URGENT_TASK_AGGREGATION;
    }
    else
    {
        fwrite_array(FILEID_RUN_PARAM,FLADDR_AGGREGATION_FILE_IDX,(INT8U*)&portContext->aggregate_id,1);
        urgent_task_in_wait_next_urgent_task(portContext);
    }
    return 0;
}
INT8U router_send_afn_F0_F11(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT8U data[5] = { 0 };
    portContext = (PLCPortContext*)readportcontext;

    router_376_2_set_aux_info(0,40,0,TRUE);

    data[0] = 0x64;
    data[1] = portContext->agg_ctl.plc.start_seq[0];
    data[2] = portContext->agg_ctl.plc.start_seq[1];
    portContext->frame_send_Len = router_376_2_cmd_frame(DL69842_AFN_DEBUG,DT_F11,data,3,portContext);

    readport_plc.OnPortReadData = router_urgent_task_send_idle;   //�ڽ��������г���

    return portContext->frame_send_Len;
}
INT8U router_recv_afn_F0_F11(objReadPortContext * readportcontext)
{
    PLCPortContext* portContext;
    INT32U offset;
    INT16U seq;
    INT16U grp_cnt;
    INT16U fileid;
    INT16U cp_len;
//    AGGREGATION_INFO agg_info;
    INT8U pos;

    portContext = (PLCPortContext*)readportcontext;

    pos = portContext->recv_data_pos;

    /* �������� */
    pos++;
    /* �ۺ��������� */
    grp_cnt = bin2_int16u(portContext->frame_recv+pos);
    pos    += 2;
    /* �ۺ������ */
    seq     = bin2_int16u(portContext->frame_recv+pos);

	if(portContext->aggregate_id > 1)
	{
		/* log error and exit */
		urgent_task_in_wait_next_urgent_task(portContext);
	}
    fileid  = FILEID_ROUTER_AGGREGATION_INFO + portContext->aggregate_id;

    if(seq == 0)
    {
        offset = PIM_AGGREGATION_INFO_START;
        fwrite_array(fileid,offset,(INT8U *)&grp_cnt,2);
        portContext->agg_ctl.plc.grp_cnt[0] = grp_cnt;
        portContext->agg_ctl.plc.grp_cnt[1] = grp_cnt >> 8;
    }

    cp_len = bin2_int16u(portContext->frame_recv+pos+2)*6;
    cp_len += 3;

    offset = PIM_AGGREGATION_GRP_INFO_START + seq * PIM_AGGREGATION_GRP_INFO_SIZE;
    fwrite_array(fileid,offset,portContext->frame_recv+pos,cp_len);

    seq ++;
    if(seq >= bin2_int16u(portContext->agg_ctl.plc.grp_cnt))
    {
        /* ����״̬������ȴ���һ�����������״̬ */
        fwrite_array(FILEID_RUN_PARAM,FLADDR_AGGREGATION_FILE_IDX,(INT8U*)&portContext->aggregate_id,1);
        urgent_task_in_wait_next_urgent_task(portContext);
    }
    else
    {
        portContext->agg_ctl.plc.start_seq[0] = seq;
        portContext->agg_ctl.plc.start_seq[1] = seq >> 8;
        portContext->urgent_task = PLC_TASK_URGENT_TASK;
        readport_plc.OnPortReadData = router_send_afn_F0_F11;
        portContext->urgent_task_step = PLC_URGENT_TASK_AGGREGATION;
    }
    return 0;
}
#endif

void cco_report_erc12(INT16U meter_idx)
{
    INT8U event[20];
    INT8U event_flag;
    INT8U pos;


    if((meter_idx == 0 ) || (meter_idx > MAX_METER_COUNT) )
    {
        return;
    }
    event_flag = VIPEVENT_FLAG;//check_event_prop(ERC12);

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

    //4. ��¼�¼�
    save_event_record(event,event_flag);

}
void main_node_modul_id_deny_process(void)
{
    INT8U value[5] = {0};
    INT8U tmp_val[64] = {0};
    INT8U flag,read_info_len;

 //·�ɳ��̴���value
    fread_array(FILEID_RUN_DATA,FLADDR_ROUTER_ID_INFO,tmp_val,4);
    fread_array(FILEID_RUN_DATA,FLADDR_MODULE_ID_FILE_ID_FLAG,&flag,1);
                /* ������ 0x55 ����ûд�� */
    mem_cpy(value,gSystemInfo.plc_ver_info+6,2); //���̴���
    value[2] = 0x01;
    value[3] = 0x02;
    value[4] = 0;
    if(flag != 0x55)
    {
                    //��һ�Σ�ֱ��д��

        fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_ID_INFO,value,5);
        flag = 0x55;
        fwrite_array(FILEID_RUN_DATA,FLADDR_MODULE_ID_FILE_ID_FLAG,&flag,1);
    }
    else
    {
                    //
        tmp_val[2] = (tmp_val[2]>50)?50: tmp_val[2];
        read_info_len = tmp_val[2]+4;
        fread_array(FILEID_RUN_DATA,FLADDR_ROUTER_ID_INFO+4,tmp_val+4,tmp_val[2]);

        if(compare_string(value,tmp_val,5) != 0)
        {
                        //����ERC43
            fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_ID_INFO,value,5);
            #ifdef __PROVICE_HUNAN__
            #else
            event_erc_43(tmp_val,read_info_len,value,5,02); // ����ģ����
            #endif
        }
    }

}
void main_node_chip_id_deny_process(void)
{
    INT8U flag,read_info_len;

    SLAVE_NODE_ID_INFO node_id_info;
	SLAVE_NODE_ID_INFO last_node_id_info;
    INT8U last_id[35] = {0};/*��֮ǰ��10F7 �ظ���ʽ���� ������ 11+24(24 ���µ�ģ��ID�� )*/
    INT8U cur_id[35]  = {0};

    mem_set(node_id_info.value,33,0xFF);
    get_router_main_node_addr(node_id_info.addr);
    node_id_info.node_dev_type = 0x02;
    fread_array(FILEID_RUN_DATA,FLADDR_CHIP_ID_FILE_ID_FLAG,&flag,1);
    if(flag != 0xAA)
    {
            /*����AA ��˵��֮ǰû��д�� ֱ��д�� ������ERC�¼� */
            fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,node_id_info.value,33);
            flag = 0xAA;
            fwrite_array(FILEID_RUN_DATA,FLADDR_CHIP_ID_FILE_ID_FLAG,&flag,1);
    }
    else
    {
            /* ֻ�ж�ID ��Ϣ ��ַ����Ϣ���ܻ����ڷ����仯 ��ʱ������ TODO ???? */
            fread_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,last_node_id_info.value,33);
            if(0 != compare_string(node_id_info.node_ic_id_info.value,last_node_id_info.node_ic_id_info.value,24) )
            {
                fwrite_array(FILEID_RUN_DATA,FLADDR_ROUTER_CHIP_ID_INFO,node_id_info.value,33);
                /* TODO: */
                /* ����10F7�ظ���ʽ�������ݣ������¼� */

                mem_cpy(last_id,last_node_id_info.addr,6);
                last_id[6] = 0;
                mem_cpy(last_id+7,last_node_id_info.node_ic_id_info.head_1+15,2);/* ���̴��� */
                last_id[9]= 24;/*ID ���� */
                last_id[10] = 0x00;/*��ʽ ��� */
                mem_cpy(last_id+11,last_node_id_info.node_ic_id_info.value,24);

                mem_cpy(cur_id,node_id_info.addr,6);
                cur_id[6] = 0;
                mem_cpy(cur_id+7,node_id_info.node_ic_id_info.head_1+15,2);/* ���̴��� */
                cur_id[9]= 24;/*ID ���� */
                cur_id[10] = 0x00;/*��ʽ ��� */
                mem_cpy(cur_id+11,node_id_info.node_ic_id_info.value,24);
                #ifdef __PROVICE_GANSU__
                 //���಻Ҫģ�����¼��ˣ�ֻ����оƬ
                #else
                event_erc_43(last_id,35,cur_id,35,0x04);
                #endif
            }
    }
}
