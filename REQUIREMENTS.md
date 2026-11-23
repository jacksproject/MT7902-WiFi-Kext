# Requirements — MT7902-WiFi-Kext (AWDL / AirDrop Port)

## Target Hardware & OS
- **OS**: macOS 26 Tahoe (Darwin 25, e.g., build `25G332`)
- **Wi-Fi Controller**: MediaTek MT7902 (`14c3:7902`)
- **Architecture**: `MT7902AirWiFiLayer` (Skywalk/IO80211), `MT7902MtkWlan` (radio bridge), and `MtkMwx` (`MWX_IS_7902` / `KeepFullPwr 1`)

## Boot Arguments & Kernel Flags
- **Enable AWDL Staging**: Pass `mt7902_awdl=1` (or `-mt7902awdl`) in NVRAM / OpenCore `boot-args`. AWDL remains disabled by default when omitted.
- **AWDL IPv6 / NDP Helper (`AwdlNeighbors`)**: Active by default when AWDL is enabled; set `amtk_awdl_ndp=0` to disable kernel `PF_ROUTE` link-local prefix route (`fe80::%awdl0/64`) and static peer neighbor updates.
- **Power & Channel Safety**: Keep `KeepFullPwr 1` enforced on MT7902 and verify `mwx_dbg` `op 42` / `op 38` (`TIMEOUT` / `TRAP` counters) during off-channel AWDL activity.

## Staged Sources From `MtkWireless`
- Interface & protocol: `AirportMtkAwdlInterface.{hpp,cpp}`, `IO80211AWDLProtocol.h`, `AWDL/*.hpp`
- Peer & IPv6 neighbor management: `AwdlNeighbors.{hpp,cpp}`
