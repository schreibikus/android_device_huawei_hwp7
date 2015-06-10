/******************************************************************************

                  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : test_iomux_drv.c
  �� �� ��   : ����
  ��������   : 2012��10��15��
  ����޸�   :
  ��������   : test_iomux_drv
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2012��10��15��
    �޸�����   : �����ļ�

******************************************************************************/

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/

#include <linux/mux.h>
#include  "test_acore.h"
#include  "test_iomux_drv.h"




#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif



/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/
int test_iomux_get_config(void)
{
    if (0 == iomux_get_block("block_uart0"))
        return ERROR;
    if (0 == iomux_get_blockconfig("block_uart0"))
        return ERROR;
    return OK;
}












#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
