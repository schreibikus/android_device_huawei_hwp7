/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : rfilesystem_transfer.h
  版 本 号   : 初稿
  生成日期   : 2013年1月26日
  最近修改   :
  功能描述   : rfilesystem_transfer.h 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2013年1月26日
    修改内容   : 创建文件

******************************************************************************/

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#ifndef __RFILESYSTEM_TRANSFER___
#define __RFILESYSTEM_TRANSFER___

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif



/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define TRANS_MAX_SIZE   (1024)
#define UDI_ACM_HSIC_ID  (27)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 消息头定义
*****************************************************************************/


/*****************************************************************************
  5 消息定义
*****************************************************************************/


/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/
typedef struct RFILECONTEXT_A_TAG
{
    int               iccOpened;
	int               hsicOpened;
	UDI_HANDLE        rFileIccFd;
	UDI_HANDLE        rFileHsic9Fd;
	ICC_CHAN_ATTR_S   attr;
	UDI_OPEN_PARAM    rFileIccParam;
	UDI_OPEN_PARAM    rFileHsic9Param;
	void *            sendBuf;
	struct semaphore  ap2mpSem;
	struct semaphore  mp2apSem;
}RFILECONTEXT_A;


typedef struct
{
    unsigned int ulModuleId;
    unsigned int ulFuncId;
    unsigned int ulFlag;
}RFILE_IFC_MSG;


/*****************************************************************************
  7 UNION定义
*****************************************************************************/


/*****************************************************************************
  8 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  9 全局变量声明
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

int rfile_a_give_c_sem(void);











#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of rfilesystem_transfer_new.h */
