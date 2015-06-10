/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : test_gpio_drv.c
  版 本 号   : 初稿
  生成日期   : 2012年10月15日
  最近修改   :
  功能描述   : test_gpio.c
  函数列表   :
  修改历史   :
  1.日    期   : 2012年10月15日
    修改内容   : 创建文件

******************************************************************************/

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include <linux/gpio.h>
#include "test_acore.h"
#include "test_gpio_drv.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif



/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/
int test_gpio_to_irq(void)
{
    int i,j;
    i=0;
    j=0;
    i=__gpio_to_irq(0);
    if (0 == i){
        return ERROR;
    }
    j=__gpio_to_irq(1);
    if (0 == i){
        return ERROR;
    }
    if (1 == (j-i)){
        return OK;
    }
    else{
        return ERROR;
    }

}












#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

