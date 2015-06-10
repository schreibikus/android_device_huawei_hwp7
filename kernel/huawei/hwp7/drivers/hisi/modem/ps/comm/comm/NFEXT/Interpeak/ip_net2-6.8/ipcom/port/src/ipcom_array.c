/*
 ******************************************************************************
 *                     SOURCE FILE
 *
 *     Document no: @(#) $Name: VXWORKS_ITER18A_FRZ10 $ $RCSfile: ipcom_array.c,v $ $Revision: 1.7 $
 *     $Source: /home/interpeak/CVSRoot/ipcom/port/src/ipcom_array.c,v $
 *     $State: Exp $ $Locker:  $
 *
 *     INTERPEAK_COPYRIGHT_STRING
 *     Design and implementation by Lennart Bang <lob@interpeak.se>
 ******************************************************************************
 */

/*
 ****************************************************************************
 * 1                    DESCRIPTION
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 2                    CONFIGURATION
 ****************************************************************************
 */

#include "ipcom_config.h"


/*
 ****************************************************************************
 * 3                    INCLUDE FILES
 ****************************************************************************
 */

#include "ipcom_clib.h"

#if !defined(IP_PORT_LKM)
#include <stdlib.h>
#endif

/*
 ****************************************************************************
 * 4                    EXTERN PROTOTYPES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 5                    DEFINES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 6                    TYPES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 7                    LOCAL PROTOTYPES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 8                    DATA
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 9                    LOCAL FUNCTIONS
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 10                   PUBLIC FUNCTIONS
 ****************************************************************************
 */

/*
 *===========================================================================
 *                    ipcom_qsort
 *===========================================================================
 * Description:
 * Parameters:
 * Returns:
 *
 */
#if defined(IPCOM_QSORT) && IPCOM_QSORT == 1
IP_PUBLIC void
ipcom_qsort(void *base, Ip_size_t nel, Ip_size_t width, int (*compar)(const void *, const void *))
{
#ifdef IP_PORT_RTCORE
    ip_assert("ipcom_qsort() is not implemented" == 0);
#else
    qsort(base, (size_t) nel, (size_t) width, compar);
#endif
}
#endif


/*
 *===========================================================================
 *                    ipcom_bsearch
 *===========================================================================
 * Description:
 * Parameters:
 * Returns:
 *
 */
#if defined(IPCOM_BSEARCH) && IPCOM_BSEARCH == 1
IP_PUBLIC void *
ipcom_bsearch(const void *key, const void *base, Ip_size_t nel, Ip_size_t size,
              int (*compar)(const void *,const void *))
{
#ifdef IP_PORT_RTCORE
    ip_assert("ipcom_bsearch() is not implemented" == 0);
    return IP_NULL;
#else
    return (void *)bsearch(key, base, (size_t)nel, (size_t)size, compar);
#endif
}
#endif


/*
 ****************************************************************************
 *                      END OF FILE
 ****************************************************************************
 */
