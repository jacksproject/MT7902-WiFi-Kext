//
//  mt7902_radio.h - MT7902 radio bridge ABI (MT7902MtkWlan <-> MT7902AirWiFiLayer)
//  Copyright (C) 2026 Yusufcan Yavic
//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  The native Wi-Fi layer drives the MT7922 radio only through these calls.
//  While the radio lives in MT7902MtkWlan they are reached with
//  IOService::callPlatformFunction(name, false, p1, p2, 0, 0); once both are
//  one kext the same functions are called directly.
//

#ifndef mt7902_radio_h
#define mt7902_radio_h

#include <stdint.h>

#define YY_RADIO_ABI            7   /* 2: + join/status/leave, 3: + mac/data hook/tx, 4: + power, 5: + link rate/width, 6: + full IEs, HE, 7: + monitor mode / raw RX tap */

#define YY_RADIO_FN_ABI         "yyRadioAbi"        /* p1: uint32_t *abi */
#define YY_RADIO_FN_SCAN_START  "yyRadioScanStart"  /* p1: uint32_t *state (out, optional) */
#define YY_RADIO_FN_SCAN_COPY   "yyRadioScanCopy"   /* p1: yy_radio_bss[], p2: uint32_t *count (in: capacity, out: filled) */
#define YY_RADIO_FN_CHANNELS    "yyRadioChannels"   /* p1: yy_radio_chan[], p2: uint32_t *count (in/out) */
#define YY_RADIO_FN_JOIN        "yyRadioJoin"       /* p1: const yy_radio_join * */
#define YY_RADIO_FN_STATUS      "yyRadioStatus"     /* p1: yy_radio_status * (out) */
#define YY_RADIO_FN_LEAVE       "yyRadioLeave"      /* no args */
#define YY_RADIO_FN_MAC         "yyRadioMac"        /* p1: uint8_t[6] (out), the radio's own address */
/* New own address (firmware device re-added); refused (busy) while associated. */
#define YY_RADIO_FN_SET_MAC     "yyRadioSetMac"     /* p1: const uint8_t[6] */
/*
 * Data path (ABI 3). While a hook is attached every received data frame
 * (Ethernet framing, as the radio's own interface would get it) goes to the
 * hook instead, which then owns the mbuf; the radio's Ethernet interface
 * reports its link inactive meanwhile. The hook runs on the radio's command
 * gate: it must not block or call back into the radio.
 */
#define YY_RADIO_FN_DATA_HOOK   "yyRadioDataHook"   /* p1: yy_radio_rx_fn (NULL = detach), p2: ctx */
/* Ethernet frame out through the radio (consumed in all cases). Not run on
 * the command gate, like the radio's own output path. */
#define YY_RADIO_FN_TX          "yyRadioTx"         /* p1: mbuf_t */
/* Radio power (ABI 4): off = deauth + firmware device down (mwx_stop),
 * on = up again (mwx_init, scanning). */
#define YY_RADIO_FN_POWER       "yyRadioPower"      /* p1: uint32_t *on */
/* 0.7.3: the card's factory address (not the current, possibly private one). */
#define YY_RADIO_FN_HW_MAC      "yyRadioHwMac"      /* p1: uint8_t[6] (out) */

typedef void (*yy_radio_rx_fn)(void *ctx, void *mbuf);

/* net80211 states as reported in yy_radio_status.state */
#define YY_RADIO_S_INIT         0
#define YY_RADIO_S_SCAN         1
#define YY_RADIO_S_AUTH         2
#define YY_RADIO_S_ASSOC        3
#define YY_RADIO_S_RUN          4

#define YY_RADIO_BAND_2G        0
#define YY_RADIO_BAND_5G        1

struct yy_radio_bss {
    uint8_t  bssid[6];
    uint8_t  ssid_len;
    uint8_t  ssid[32];
    uint8_t  channel;           /* IEEE channel number (primary) */
    uint8_t  band;              /* YY_RADIO_BAND_* */
    uint8_t  nrates;
    uint8_t  rates[16];         /* 802.11 rate octets */
    int16_t  rssi_dbm;
    uint16_t capinfo;
    uint16_t intval;            /* beacon interval (TU) */
    uint16_t ies_len;
    uint8_t  ies[1024];         /* ABI 6: the beacon/probe response elements as
                                 * received (all of them, from the SSID on; the
                                 * last bytes may be a partial element) */
};

struct yy_radio_chan {
    uint8_t  channel;
    uint8_t  band;              /* YY_RADIO_BAND_* */
    uint8_t  passive;           /* passive scan only */
    uint8_t  reserved;
};

struct yy_radio_join {
    uint8_t  ssid_len;
    uint8_t  ssid[32];
    uint8_t  has_pmk;           /* 0 = open network */
    uint8_t  pmk[32];           /* WPA/WPA2 PSK PMK */
    uint8_t  bssid[6];          /* preferred BSS, all zero = any */
};

struct yy_radio_status {
    uint32_t state;             /* YY_RADIO_S_* */
    uint8_t  port_valid;        /* 802.1X/4-way done (or open network) */
    uint8_t  rsn;               /* RSN/WPA in use */
    uint8_t  bssid[6];
    uint8_t  ssid_len;
    uint8_t  ssid[32];
    uint8_t  channel;
    uint8_t  band;              /* YY_RADIO_BAND_* */
    int16_t  rssi_dbm;
    uint32_t tx_rate_mbps;      /* legacy rate of the BSS node, 0 if unknown */
    /* ABI 5 */
    uint8_t  bss_bw;            /* operating width: 0 = 20, 1 = 40, 2 = 80, 3 = 160 MHz */
    uint8_t  center;            /* center channel of that width */
    uint8_t  rx_mode;           /* last HT/VHT/HE data frame: 2/3 = HT, 4 = VHT, 8-11 = HE, 0 = none yet */
    uint8_t  rx_mcs;            /* MCS (HT: 0-31, VHT: 0-9, HE: 0-11) */
    uint8_t  rx_nss;            /* spatial streams (1-2) */
    uint8_t  rx_bw;             /* 0 = 20, 1 = 40, 2 = 80, 3 = 160 MHz */
    uint8_t  rx_sgi;            /* HT/VHT: short guard interval; HE: GI 0/1/2 = 0.8/1.6/3.2 us */
    uint8_t  reserved5;
    /* ABI 6 */
    uint8_t  he_enabled;        /* radio advertises 802.11ax (no -mt7902nohe) */
    uint8_t  he_link;           /* the current association is HE */
    char     country[2];        /* 0.7.1 (was reserved): the radio's country
                                 * code (mt7902-cc), "00" = world */
};

/*
 * Monitor mode / raw 802.11 RX tap (ABI 7, AWDL milestone 1).
 *
 * yyRadioMonitor switches the radio between station and monitor mode and
 * sets the monitor channel (while in monitor mode a second call only changes
 * the channel). Entering monitor mode drops any association: the station
 * link is down until it is called again with enable = 0.
 *
 * yyRadioMonHook attaches a receiver for every frame the radio sniffs in
 * monitor mode: the raw 802.11 frame starting at the MAC header (no radiotap,
 * no FCS). The hook runs on the radio's RX path: it must not block, must not
 * call back into the radio and must not keep the pointer after returning.
 */
#define YY_RADIO_FN_MONITOR     "yyRadioMonitor"    /* p1: const yy_radio_monitor *, p2: yy_radio_mon_stats * (out, optional) */
#define YY_RADIO_FN_MON_HOOK    "yyRadioMonHook"    /* p1: yy_radio_mon_fn (NULL = detach), p2: ctx */

struct yy_radio_rxmeta {
    uint8_t  channel;           /* IEEE channel number the frame was received on */
    uint8_t  reserved[3];
    int32_t  rssi_raw;          /* rxi_rssi at the tap point: dBm (negative), before net80211's rssi conversion */
    uint64_t tsf;               /* rxi_tstamp (low 32 bits, us), 0 if the RXD carries none */
};

typedef void (*yy_radio_mon_fn)(void *ctx, const uint8_t *frame, uint32_t len,
                                const struct yy_radio_rxmeta *meta);

struct yy_radio_monitor {
    uint8_t  enable;            /* 1 = monitor mode on, 0 = back to station mode */
    uint8_t  channel;           /* IEEE channel (AWDL social channels: 6, 44, 149) */
    uint8_t  log_awdl;          /* 1 = log AWDL action frames in the kernel log */
    uint8_t  reserved;
};

struct yy_radio_mon_stats {
    uint32_t frames;            /* all frames seen in monitor mode */
    uint32_t mgmt;              /* management frames */
    uint32_t action;            /* action frames */
    uint32_t awdl;              /* action frames of Apple's AWDL (OUI 00:17:F2, type 8) */
    uint32_t awdl_psf;          /* AWDL periodic synchronization frames */
    uint32_t awdl_mif;          /* AWDL master indication frames */
};

#endif /* mt7902_radio_h */
