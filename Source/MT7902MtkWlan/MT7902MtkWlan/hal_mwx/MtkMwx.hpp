/*
 * Reworked by Yusufcan Yavic, 2026, for MT7902WiFi (native MT7922 Wi-Fi on macOS).
 * Starting point: the SimpleMtkWlan version of this file by laobamac.
 * Changed by Yusufcan Yavic for MT7902WiFi (see CHANGELOG.md).
 * See CHANGELOG.md. The original notices below remain in force; these
 * modifications are distributed under the same license as this file.
 */
//
//  MtkMwx.hpp
//  MT7902WiFi
//
//  Originally written by laobamac (王孝慈) on 2026/5/17 for SimpleMtkWlan.
//  Copyright © 2026 laobamac (王孝慈). All rights reserved.
//

/*
 * MediaTek MT7922 HAL for MT7902MtkWlan.
 *
 * The driver body is ported from OpenBSD if_mwx.  This header only exposes
 * the MT7902MtkWlan HAL boundary; OpenBSD driver internals stay in MtkMwx.cpp.
 */

#ifndef MtkMwx_hpp
#define MtkMwx_hpp

#include <compat.h>
#include <HAL/MtkHalService.hpp>
#include <HAL/MtkDriverInfo.hpp>
#include <HAL/MtkDriverController.hpp>

struct mwx_softc;
struct pci_attach_args;

class MtkMwx : public MtkHalService, MtkDriverInfo, MtkDriverController {
    OSDeclareDefaultStructors(MtkMwx)

public:
    bool attach(IOPCIDevice *device) override;
    void detach(IOPCIDevice *device) override;
    IOReturn enable(IONetworkInterface *interface) override;
    IOReturn disable(IONetworkInterface *interface) override;
    struct ieee80211com *get80211Controller() override;
    MtkDriverInfo *getDriverInfo() override;
    MtkDriverController *getDriverController() override;
    void free() override;

    const char *getFirmwareVersion() override;
    int16_t getBSSNoise() override;
    bool is5GBandSupport() override;
    int getTxNSS() override;
    const char *getFirmwareName() override;
    UInt32 supportedFeatures() override;
    const char *getFirmwareCountryCode() override;
    uint32_t getTxQueueSize() override;

    void clearScanningFlags() override;
    IOReturn setMulticastList(IOEthernetAddress *addr, int count) override;

    static bool mwx_match(IOPCIDevice *device);

    IOEthernetController *mwxController();
    int mwxSleepNsec(void *ident, int priority, const char *wmesg, uint64_t timo);
    void mwxWakeup(void *ident);
    bool mwxOnWorkLoop();
    void mwxStartPoll(struct mwx_softc *sc);
    IOReturn setOwnAddress(const uint8_t *mac);   /* MT7902 radio bridge */
    IOReturn setMonitor(bool on, uint8_t channel, bool logAwdl);   /* ABI 7: monitor mode + channel */
    IOReturn setMonitorPassive(bool on, bool logAwdl);             /* ABI 7: sniff while associated (home channel) */
    bool isMonitor();                                              /* ABI 7: radio is in monitor mode */
    void setMonHook(void *fn, void *ctx);                          /* ABI 7: yy_radio_mon_fn (NULL = detach) */
    void getMonStats(uint32_t *out6);                              /* ABI 7: frames, mgmt, action, awdl, psf, mif */

private:
    void releaseAll();

    bool taskQueueOwned;
    struct mwx_softc *com;
    struct pci_attach_args *pci;
};

#endif /* MtkMwx_hpp */
