/*
 ******************************************************************************
 *                     INTERPEAK SOURCE FILE
 *
 *   Document no: @(#) $Name: VXWORKS_ITER18A_FRZ10 $ $RCSfile: ipcom_pkt.c,v $ $Revision: 1.5 $
 *   $Source: /home/interpeak/CVSRoot/ipcom/port/vxworks/src/ipcom_pkt.c,v $
 *   $State: Exp $ $Locker:  $
 *
 *   INTERPEAK_COPYRIGHT_STRING
 *     Design and implementation by FirstName LastName <email@interpeak.se>
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
#include "ipcom_pkt.h"

#include <stdlib.h>

/*
 ****************************************************************************
 * 4                    DEFINES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 5                    TYPES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 6                    EXTERN PROTOTYPES
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
 * 9                    STATIC FUNCTIONS
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 10                   GLOBAL FUNCTIONS
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 11                   PUBLIC FUNCTIONS
 ****************************************************************************
 */

/*
 *===========================================================================
 *                    ipcom_pkt_buffer_malloc
 *===========================================================================
 * Description:
 * Parameters:
 * Returns:
 *
 */
IP_PUBLIC void *
ipcom_pkt_buffer_malloc(Ip_size_t size)
{
    return ipcom_malloc(size);
}


/*
 *===========================================================================
 *                    ipcom_pkt_buffer_free
 *===========================================================================
 * Description:
 * Parameters:
 * Returns:
 *
 */
IP_PUBLIC void
ipcom_pkt_buffer_free(void *buffer)
{
    ipcom_free(buffer);
}


/*
 ****************************************************************************
 *                      END OF FILE
 ****************************************************************************
 */

