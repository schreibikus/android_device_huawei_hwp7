/*
 ******************************************************************************
 *                     HEADER FILE
 *
 *   Document no: @(#) $Name: VXWORKS_ITER18A_FRZ10 $ $RCSfile: ipnet.h,v $ $Revision: 1.108.12.1.4.2 $
 *   $Source: /home/interpeak/CVSRoot/ipnet2/include/ipnet.h,v $
 *   $State: Exp $ $Locker:  $
 *
 *   INTERPEAK_COPYRIGHT_STRING
 *     Design and implementation by Kenneth Jonsson <kenneth@interpeak.se>
 ******************************************************************************
 */


#ifndef IPNET_H
#define IPNET_H

/*
 ****************************************************************************
 * 1                    DESCRIPTION
 ****************************************************************************
 * IPNET2 API header file.
 *
 * NOTE: Also see ipcom/include/ipcom_sock.h and ipcom_sock6.h!
 */

/*
 ****************************************************************************
 * 2                    CONFIGURATION
 ****************************************************************************
 */

#include "ipnet_config.h"


/*
 ****************************************************************************
 * 3                    INCLUDE FILES
 ****************************************************************************
 */

#include <ipcom_type.h>
#include <ipcom_errno.h>
#include <ipcom_cstyle.h>
#include <ipcom_time.h>
#include <ipcom_sock.h>
#include <ipcom_pkt.h>
#include <ipcom_sock6.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 ****************************************************************************
 * 4                    DEFINES
 ****************************************************************************
 */

/* IPNET release XYyZz,
   X  = major version
   Yy = minor version (two different versions might be incompatible)
   Zz = patch version (two different versions are always compatible)  */
#define IPNET_RELEASE 60704

/*
 *===========================================================================
 *                         IPNET Routing
 *===========================================================================
 */

/* Route flag values */
#define IPNET_RTF_UP            0x1     /* Route usable */
#define IPNET_RTF_GATEWAY       0x2     /* Destination is reached through a gateway */
#define IPNET_RTF_HOST          0x4     /* Host entry (net otherwise) */
#define IPNET_RTF_REJECT        0x8     /* Host or net unreachable */
#define IPNET_RTF_DYNAMIC       0x10    /* Created dynamically */
#define IPNET_RTF_MODIFIED      0x20    /* Modified dynamically */
#define IPNET_RTF_DONE          0x40    /* Message confirmed */
#define IPNET_RTF_MASK          0x80    /* Subnet mask present */
#define IPNET_RTF_CLONING       0x100   /* Generate new routes on use */
#define IPNET_RTF_XRESOLVE      0x200   /* External daemon resolves name */
#define IPNET_RTF_LLINFO        0x400   /* Generated by IPv4 ARP or IPv6 NDP */
#define IPNET_RTF_STATIC        0x800   /* Manually added */
#define IPNET_RTF_BLACKHOLE     0x1000  /* Just discard pkts (during updates) */
#define IPNET_RTF_LOCAL         0x2000  /* route represents a local address */
#define IPNET_RTF_PROTO1        0x4000  /* protocol specific routing flag #1 */
#define IPNET_RTF_PROTO2        0x8000  /* protocol specific routing flag #2 */
#define IPNET_RTF_PREF          0x10000 /* Add route first, no ECMP */
#define IPNET_RTF_TUNNELEND     0x20000 /* The gateway field specifies the tunnel endpoint address*/
#define IPNET_RTF_MBINDING      0x40000 /* This route is part of multiple binding */
#define IPNET_RTF_SKIP          0x80000 /* Treat this route as a routing lookup failure */

/* Specifiers for metric values in rmx_locks and rtm_inits are: */
#define IPNET_RTV_EXPIRE   0x4     /* init or lock _hopcount */
#define IPNET_RTV_HOPCOUNT 0x8     /* init or lock _hopcount */
#define IPNET_RTV_RTT      0x10    /* init or lock _rtt */
#define IPNET_RTV_RTTVAR   0x20    /* init or lock _rttvar */
#define IPNET_RTV_MTU      0x40    /* init or lock _mtu */
#define IPNET_RTV_RPIPE    0x100   /* Unsupported! */
#define IPNET_RTV_SPIPE    0x200   /* Unsupported! */
#define IPNET_RTV_SSTHRESH 0x400   /* Unsupported! */
#ifdef IP_PORT_OSE5
#define IPNET_RTV_PROTO    0x800   /* OSE5 only */
#define IPNET_RTV_USR1     0x1000  /* OSE5 only */
#define IPNET_RTV_USR2     0x2000  /* OSE5 only */
#define IPNET_RTV_USR3     0x4000  /* OSE5 only */
#endif /* IP_PORT_OSE5 */


/*
 *===========================================================================
 *                         ipnet_ping_cmd
 *===========================================================================
 */
#define IPNET_PING_CMD_NO_PING_IN_PROGRESS 0
#define IPNET_PING_CMD_PINGING 1
#define IPNET_PING_CMD_REQUEST_STOP 2
#define IPNET_PING_CMD_STOPPED 3


/*
 *===========================================================================
 *                    Ipnet_rx_cache_fast_input_f
 *===========================================================================
 * Description: Hook that can process an incoming packet much faster if
 *              the correct conditions are met.
 * Parameters:  sock - The socket the packet will be delivered to.
 *              pkt - The packet to deliver.
 *              nat_t   - [OUT] NAT Traversal attribute.
 * Returns:     IP_TRUE - The fast path processing was successful.
 *              IP_FALSE - Need to use the normal receive path.
 *
 */
struct Ipnet_socket_struct;
typedef Ip_bool (*Ipnet_rx_cache_fast_input_f)(struct Ipnet_socket_struct *sock,
                                               Ipcom_pkt *pkt,
                                               int    *flags);


/*
 ****************************************************************************
 * 5                    TYPES
 ****************************************************************************
 */

/*
 *===========================================================================
 *                         Ipnet_rt_metrics
 *===========================================================================
 */
struct Ipnet_rt_metrics
{
#if defined(IPNGC) || defined(IP_PORT_OSE5)
    Ip_u32      rmx_locks;      /* Unsupported! */
#endif
    Ip_u32      rmx_mtu;        /* MTU for this path */
    Ip_u32      rmx_hopcount;   /* Max hops expected */
    Ip_u32      rmx_expire;     /* Lifetime for route, e.g. redirect */
#if defined(IPNGC) || defined(IP_PORT_OSE5)
    Ip_u32      rmx_recvpipe;   /* Unsupported! */
    Ip_u32      rmx_sendpipe;   /* Unsupported! */
    Ip_u32      rmx_ssthresh;   /* Unsupported! */
#endif
    Ip_u32      rmx_rtt;        /* Estimated round trip time */
    Ip_u32      rmx_rttvar;     /* Estimated rtt variance */
#ifdef IP_PORT_OSE5
    Ip_u8       rmx_proto;      /* User defined protocol */
    Ip_u8       rmx_usr1;       /* User defined value */
    Ip_u16      rmx_usr2;       /* User defined value */
    Ip_u32      rmx_usr3;       /* User defined value */
#endif
};


/*
 *===========================================================================
 *                         address types
 *===========================================================================
 */
enum Ipnet_addr_type {
    IPNET_ADDR_TYPE_NOT_LOCAL =    -1, /* Not any of the host local addresses */
    IPNET_ADDR_TYPE_ANY =           0, /* The ANY address */
    IPNET_ADDR_TYPE_UNICAST =       1, /* One of the localhost unicast addresses */
    IPNET_ADDR_TYPE_MULTICAST =     2, /* One of the localhost multicast addresses */
    IPNET_ADDR_TYPE_BROADCAST =    10, /* IPv4 only, Broadcast address */
    IPNET_ADDR_TYPE_NETBROADCAST = 11, /* IPv4 only, (sub)network broadcast */
    IPNET_ADDR_TYPE_ANYCAST =      20, /* IPv6 only, anycast address (RFC3512 chapter 2.6) */
    IPNET_ADDR_TYPE_TENTATIVE =    30  /* Address in tentative state */
};

/*
 ****************************************************************************
 * 6                    FUNCTIONS
 ****************************************************************************
 */

/*
 *===========================================================================
 *                    ipnet_resume_stack
 *===========================================================================
 * Description: Mimics the semantic of the old IPNET_CODE_LOCK() macro.
 * Parameters:
 * Returns:     A handle that must be passed to ipnet_resume_stack()
 *                    when the stack should resume again. IP_NULL is
 *                    return if the stack refused or could not resume.
 *
 */
IP_PUBLIC void * ipnet_suspend_stack(void);

/*
 *===========================================================================
 *                    ipnet_resume_stack
 *===========================================================================
 * Description: Mimics the semantic of the old IPNET_CODE_UNLOCK() macro.
 * Parameters:  suspend_handle - handle return from ipnet_suspend_stack()
 * Returns:
 *
 */
IP_PUBLIC void ipnet_resume_stack(void *suspend_handle);

/*
 *===========================================================================
 *                    ipnet_configure
 *===========================================================================
 * Description: Sets the default configuration for IPNET.
 * Parameters:
 * Returns:     IPCOM_SUCCESS if the operation was successful.
 *
 */
IP_PUBLIC Ip_err ipnet_configure(void);


/*
 *===========================================================================
 *                    ipnet_version
 *===========================================================================
 * Description: Returns a version string for IPNET.
 * Parameters:
 * Returns:
 *
 */
IP_PUBLIC const char * ipnet_version(void);

#ifdef IP_PORT_LKM
/*
 *===========================================================================
 *                    ipcom_pkt_recv_raw_pkt
 *===========================================================================
 * Description: Receives a raw packet from a socket.
 * Parameters:  fd - A socket descriptor.
 *              flags -
 * Returns:     A reference to a packet
 *              IP_NULL - failure
 *
 */
IP_PUBLIC Ipcom_pkt * ipcom_pkt_recv_raw_pkt(Ip_fd fd, int flags);

#endif /* IP_PORT_LKM */

/*
 *===========================================================================
 *                    Shell command prototypes
 *===========================================================================
 */

IP_PUBLIC int ipnet_cmd_route(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_ctrl(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_ifconfig(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_sysctl(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_netstat(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_qc(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_qos(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_cga(int argc, char **argv);

#ifdef IPCOM_USE_INET
IP_PUBLIC int ipnet_cmd_arp(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_ping(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_ping_stoppable(int argc, char **argv, int *stop);
#endif /* IPCOM_USE_INET */

#ifdef IPCOM_USE_INET6
IP_PUBLIC int ipnet_cmd_ndp(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_ping6(int argc, char **argv);
IP_PUBLIC int ipnet_cmd_radvd(int argc, char **argv);

#ifdef IPMIP6
IP_PUBLIC int ipnet_cmd_mip6(int argc, char **argv);
#endif

#endif /* IPCOM_USE_INET6 */

#ifdef IPNET_USE_TUNNEL
IP_PUBLIC int ipnet_cmd_tunnel(int argc, char **argv);
#endif

#if defined(IPNET_INCLUDE_CMD_SCALETEST) && !defined(IP_PORT_LKM)
IP_PUBLIC int ipnet_cmd_scaletest(int argc, char **argv);
#endif

#ifdef IPNET_DEBUG
IP_PUBLIC int ipnet_cmd_timeout_dump(int argc, char **argv);
#endif


#ifdef __cplusplus
}
#endif


#endif

/*
 ****************************************************************************
 *                      END OF FILE
 ****************************************************************************
 */

