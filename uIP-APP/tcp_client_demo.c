#include "tcp_demo.h"
#include "sys.h"
#include "uip.h"
#include <string.h>
#include <stdio.h>	
#include "eth0.h"


#define Perrec_len 1500				//�յ��İ�����󳤶�Ϊ1500
#define maxrec_len 30*1024

u8 tcp_client_databuf[Perrec_len];   	//�������ݻ���	  
u8 tcp_client_sta;				         //�ͻ���״̬
u16 IAP_num = 0;                      //���յ������ݵĸ���
extern u8 IAP_pro[maxrec_len]; 
extern u8 ready_flag;

//[7]:0,������;1,�Ѿ�����;
//[6]:0,������;1,�յ��ͻ�������
//[5]:0,������;1,��������Ҫ����

//����һ��TCP �ͻ���Ӧ�ûص�������
//�ú���ͨ��UIP_APPCALL(tcp_demo_appcall)����,ʵ��Web Client�Ĺ���.
//��uip�¼�����ʱ��UIP_APPCALL�����ᱻ����,���������˿�(1400),ȷ���Ƿ�ִ�иú�����
//���� : ��һ��TCP���ӱ�����ʱ�����µ����ݵ�������Ѿ���Ӧ��������Ҫ�ط����¼�
void tcp_client_demo_appcall(void)
{	
  u16 i=0;	
 	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
	if(uip_aborted())tcp_client_aborted();		//������ֹ	   
	if(uip_timedout())tcp_client_timedout();	//���ӳ�ʱ   
	if(uip_closed())tcp_client_closed();		//���ӹر�	   
 	if(uip_connected())tcp_client_connected();	//���ӳɹ�	    
	if(uip_acked())tcp_client_acked();			//���͵����ݳɹ��ʹ� 
 	//���յ�һ���µ�TCP���ݰ� 
	if (uip_newdata()) 
	{
		if((tcp_client_sta&(1<<6))==0)//��δ�յ�����
		{
			if(uip_len>Perrec_len-1)
			{		   
				((u8*)uip_appdata)[Perrec_len-1]=0;
			}

	   	/********************************************************************************************************/
	    /*   ������ֵĳ���������ר�Ž���.bin�ļ��ģ���ready_flagΪ1��Ҳ����stm32���յ�����������"ready"*/	
	     /*	�ַ����󣬿�ʼ��uip_appdata�е�����һ�������ƽ�IAP_pro��ȥ                                     */	
		/********************************************************************************************************/	
			if( ready_flag ) 
			{	   				
			  for(i=0; i<= uip_len-1; i++)
				{
				   IAP_pro[IAP_num] =  (((u8*)uip_appdata)[i]);
				   IAP_num++;
				}
				//����Ѿ�PC�������Ѿ�������׼����stm32���յ������׼������
            }
	   /***************************************************************************************************/
	    
		  strcpy(tcp_client_databuf,uip_appdata); 	          //�� uip_appdata���Ƶ�tcp_client_databuf��ȥ����eth0_receive()�����л��õõ�
			
		  tcp_client_sta|=1<<6;//��ʾ�յ��ͻ�������
		}			
	}else if(tcp_client_sta&(1<<5))//��������Ҫ����
	{
		s->textptr=tcp_client_databuf;
		s->textlen=strlen((const char*)tcp_client_databuf);
		tcp_client_sta&=~(1<<5);//������
	}  
	//����Ҫ�ط��������ݵ�����ݰ��ʹ���ӽ���ʱ��֪ͨuip�������� 
	if(uip_rexmit()||uip_newdata()||uip_acked()||uip_connected()||uip_poll())
	{
		tcp_client_senddata();
	}
   i = 0;	
}
//�������Ǽٶ�Server�˵�IP��ַΪ:192.168.0.103
//���IP�������Server�˵�IP�޸�.
//������������
void tcp_client_reconnect()
{
	uip_ipaddr_t ipaddr;
	uip_ipaddr(&ipaddr,192,168,0,103);	//����IPΪ192.168.0.103
	uip_connect(&ipaddr,htons(1400)); 	//�˿�Ϊ1400
}
//��ֹ����				    
void tcp_client_aborted(void)
{
	tcp_client_sta&=~(1<<7);	//��־û������
	tcp_client_reconnect();		//������������
	uip_log("tcp_client aborted!\r\n");//��ӡlog
}
//���ӳ�ʱ
void tcp_client_timedout(void)
{
	tcp_client_sta&=~(1<<7);	//��־û������	   
	uip_log("tcp_client timeout!\r\n");//��ӡlog
}
//���ӹر�
void tcp_client_closed(void)
{
	tcp_client_sta&=~(1<<7);	//��־û������
	tcp_client_reconnect();		//������������
	uip_log("tcp_client closed!\r\n");//��ӡlog
}	 
//���ӽ���
void tcp_client_connected(void)
{ 
	struct tcp_demo_appstate *s=(struct tcp_demo_appstate *)&uip_conn->appstate;
 	tcp_client_sta|=1<<7;		//��־���ӳɹ�
  	uip_log("tcp_client connected!\r\n");//��ӡlog
	s->state=STATE_CMD; 		//ָ��״̬
	s->textlen=0;
	s->textptr="TCP_client Connected Successfully!\r\n";//��Ӧ��Ϣ
	s->textlen=strlen((char *)s->textptr);	  
}
//���͵����ݳɹ��ʹ�
void tcp_client_acked(void)
{											    
	struct tcp_demo_appstate *s=(struct tcp_demo_appstate *)&uip_conn->appstate;
	s->textlen=0;//��������
	uip_log("tcp_client acked!\r\n");//��ʾ�ɹ�����		 
}
//�������ݸ������
void tcp_client_senddata(void)
{
	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
	//s->textptr:���͵����ݰ�������ָ��
	//s->textlen:���ݰ��Ĵ�С����λ�ֽڣ�		   
	if(s->textlen>0)uip_send(s->textptr, s->textlen);//����TCP���ݰ�	 
}

















