#############################################################################
#			INTERPEAK MAKEFILE
#
#     Document no: @(#) $Name: VXWORKS_ITER18A_FRZ10 $ $RCSfile: ipcom_defs.mk,v $ $Revision: 1.31.24.1 $
#     $Source: /home/interpeak/CVSRoot/ipcom/gmake/ipcom_defs.mk,v $
#     $State: Exp $ $Locker:  $
#
#     INTERPEAK_COPYRIGHT_STRING
#
#############################################################################

include $(IPCOM_ROOT)/port/$(IPPORT)/gmake/ipcom_$(IPPORT)_config.mk

# Port
IPPRODS += $(IPCOM_ROOT)
IPPRODS += $(IPCOM_ROOT)/port/$(IPPORT)

# Stack
IPPRODS += $(IPNET_ROOT)
IPPRODS += $(IPLITE_ROOT)
IPPRODS += $(IPTCP_ROOT)
IPPRODS += $(IPSCTP_ROOT)
IPPRODS += $(IPPPP_ROOT)
IPPRODS += $(IPIPSEC_ROOT)
IPPRODS += $(IPFIREWALL_ROOT)
IPPRODS += $(IPCCI_ROOT)
IPPRODS += $(IPMCRYPTO_ROOT)

# Applications
IPPRODS += $(IPIFPROXY_ROOT)
IPPRODS += $(IP8021X_ROOT)
IPPRODS += $(IPANVL_ROOT)
IPPRODS += $(IPAPPL_ROOT)
IPPRODS += $(IPCCI_ROOT)
IPPRODS += $(IPCRYPTO_ROOT)
IPPRODS += $(IPDHCPC6_ROOT)
IPPRODS += $(IPDHCPR_ROOT)
IPPRODS += $(IPDHCPS6_ROOT)
IPPRODS += $(IPDHCPS_ROOT)
IPPRODS += $(IPDIAMETER_ROOT)
IPPRODS += $(IPEAP_ROOT)
IPPRODS += $(IPEMANATE_ROOT)
IPPRODS += $(IPCAVIUM_ROOT)
IPPRODS += $(IPFREESCALE_ROOT)
IPPRODS += $(IPHWCRYPTO_ROOT)
IPPRODS += $(IPIKE_ROOT)
IPPRODS += $(IPL2TP_ROOT)
IPPRODS += $(IPLDAPC_ROOT)
IPPRODS += $(IPMCP_ROOT)
IPPRODS += $(IPMIP4_ROOT)
IPPRODS += $(IPMIP6CN_ROOT)
IPPRODS += $(IPMIP6HA_ROOT)
IPPRODS += $(IPMIP6MN_ROOT)
IPPRODS += $(IPMIP6PM_ROOT)
IPPRODS += $(IPMIP6_ROOT)
IPPRODS += $(IPMIPFA_ROOT)
IPPRODS += $(IPMIPHA_ROOT)
IPPRODS += $(IPMIPMN_ROOT)
IPPRODS += $(IPMLDS_ROOT)
IPPRODS += $(IPMPLS_ROOT)
IPPRODS += $(IPNETPERF_ROOT)
IPPRODS += $(IPNETSNMP_ROOT)
IPPRODS += $(IPNGC_ROOT)
IPPRODS += $(IPPRISM_ROOT)
IPPRODS += $(IPQUAGGA_ROOT)
IPPRODS += $(IPRADIUS_ROOT)
IPPRODS += $(IPRIPNG_ROOT)
IPPRODS += $(IPRIP_ROOT)
IPPRODS += $(IPSAFENET_ROOT)
IPPRODS += $(IPSNMP_ROOT)
IPPRODS += $(IPSNTP_ROOT)
IPPRODS += $(IPSSH_ROOT)
IPPRODS += $(IPSSLPROXY_ROOT)
IPPRODS += $(IPSSL_ROOT)
IPPRODS += $(IPWEBS_ROOT)
IPPRODS += $(IPXINC_ROOT)
IPPRODS += $(WRSNMP_ROOT)
IPPRODS += $(IPRIPNG_ROOT)
IPPRODS += $(IPDIAMETER_ROOT)
IPPRODS += $(IPWLAN_ROOT)
IPPRODS += $(IPROUTE2_ROOT)
IPPRODS += $(IPMIPPM_ROOT)
IPPRODS += $(IPMIP_ROOT)
IPPRODS += $(VXCOMPAT_ROOT)
IPPRODS += $(VXMUX_ROOT)
IPPRODS += $(IPROHC_ROOT)
IPPRODS += $(VXBRIDGE_ROOT)
IPPRODS += $(VXCOREIP_ROOT)
IPPRODS += $(IPWPS_ROOT)
IPPRODS += $(VXDOT1X_ROOT)
IPPRODS += $(WRNAD_ROOT)
IPPRODS += $(IFPROXY_ROOT)

ifeq ($(IPTARGETS),)
include $(IPCOM_ROOT)/gmake/ipcom_make.mk
endif


#############################################################################
# END OF IPCOM_DEFS.MK
#############################################################################
