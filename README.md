# MT7902-WiFi-Kext (AMD Ryzen 7 Hackintosh)

Native macOS Wi-Fi 6 kernel extension and Bluetooth setup guide for the **MediaTek MT7902** wireless card on **AMD Ryzen 7** Hackintosh builds, supporting **macOS Tahoe (26)** and **macOS Sequoia (15)** via OpenCore.

> **Maintainer:** Shahin Sharkar (`jacksproject`)  
> **Repository:** [jacksproject/MT7902-WiFi-Kext](https://github.com/jacksproject/MT7902-WiFi-Kext)

---

## Project Status

| Component | Status | Notes |
| :--- | :--- | :--- |
| **Wi-Fi (MT7902)** | **Fixed** | Native `IO80211` / `Skywalk` integration for macOS Tahoe (26) & Sequoia (15) on AMD Ryzen 7 |
| **Bluetooth (MT7902)** | **Supported** | Supported via `SimpleMtkBtFw` (MT7902 patched fork) + `SimpleMtkBtPatcher` + `BlueToolFixup` |

---

## Architecture Overview

`MT7902WiFi.kext` combines two components into a single native macOS kext (no external menu-bar helper app required):

1. **Radio Driver (`MT7902MtkWlan`)**: Handles MediaTek firmware loading, DMA rings, 802.11 state machine, and Wi-Fi 6 (`802.11ax`) operations.
2. **Native Wi-Fi Layer (`MT7902AirWiFiLayer`)**: Implements `IO80211Controller`, `IO80211InfraInterface`, and the `IOSkywalkFamily` data path so macOS's native Wi-Fi stack (`airportd`, System Settings, Keychain, auto-join) drives the card directly.

*(Note: On MediaTek MT7902 combo cards, Wi-Fi operates over PCIe while Bluetooth operates over an internal USB interface and uses a separate USB firmware loader.)*

---

## Requirements & Compatibility

* **CPU Platform:** AMD Ryzen 7 (Desktop / Laptop with OpenCore AMD Vanilla patches)
* **Wireless Chipset:** MediaTek MT7902 (`14c3:7902`) — also matches `14c3:7922`, `14c3:0616`, `14c3:7961`, and `14c3:0608`
* **Supported macOS Versions:**
  * **macOS Tahoe (26)**
  * **macOS Sequoia (15)**
* **Bootloader Settings (OpenCore):**
  * `SecureBootModel`: `Disabled`
  * SIP partially disabled (`csr-active-config` = `0x00000A43`)
  * Remove or disable any conflicting wireless drivers (`SimpleMtkWlan`, `YYavicWiFi`, `itlwm`, `AirportItlwm`, etc.)

---

## Boot Arguments (`boot-args`)

Add your regulatory country code in `config.plist -> NVRAM -> Add -> 7C436110-AB2A-4BBB-A880-FE41995C9F82 -> boot-args` (for example, `mt7902-cc=US`, `mt7902-cc=DE`, `mt7902-cc=BD`, `mt7902-cc=SA`):

| Boot-arg | Effect |
| :--- | :--- |
| `mt7902-cc=XX` | Sets your ISO 3166 two-letter country/regulatory code (unlocks full 2.4 GHz & 5 GHz channels; without it, only common channels 1–11 and 36–64 are enabled). |
| `-mt7902noverify` | Skips the strict macOS build-number check (for testing newer macOS Tahoe/Sequoia point releases; may kernel panic if `IO80211`/`Skywalk` layouts changed). |
| `-mt7902nohe` | Disables Wi-Fi 6 (`802.11ax`) and connects in `802.11ac` mode. |
| `-mt7902off` | Loads the radio driver only without starting the native macOS Wi-Fi layer. |
| `-mt7902nobt` | Prevents the kext from writing Bluetooth NVRAM keys at boot. |
| `-mt7902nodrvavail` | Suppresses posting the boot-time driver-available event. |

> **Automatic Crash Guard:** If macOS encounters an issue while initializing the native Wi-Fi layer, the NVRAM guard (`mt7902-arm-guard`) automatically skips starting the native layer on the next boot so you can recover safely.

---

## How to Use: Wi-Fi (OpenCore Installation)

### 1. Prepare `EFI/OC/Kexts/`
1. Back up your working OpenCore EFI folder before making changes.
2. Download or compile `MT7902WiFi.kext` for your target macOS version (**macOS 26 Tahoe** or **macOS 15 Sequoia**).
3. Copy `MT7902WiFi.kext` into `EFI/OC/Kexts/`.
4. Ensure no older MediaTek Wi-Fi kexts remain in `EFI/OC/Kexts/`.

### 2. Configure `config.plist`
1. Open `EFI/OC/config.plist` in **ProperTree** or **OCAT**.
2. Add `MT7902WiFi.kext` under **`Kernel -> Add`**:
   ```xml
   <dict>
       <key>Arch</key>
       <string>x86_64</string>
       <key>BundlePath</key>
       <string>MT7902WiFi.kext</string>
       <key>Comment</key>
       <string>MT7902WiFi (MediaTek MT7902 Native Wi-Fi)</string>
       <key>Enabled</key>
       <true/>
       <key>ExecutablePath</key>
       <string>Contents/MacOS/MT7902WiFi</string>
       <key>MaxKernel</key>
       <string></string>
       <key>MinKernel</key>
       <string></string>
       <key>PlistPath</key>
       <string>Contents/Info.plist</string>
   </dict>
   ```
3. Set your country code in **`NVRAM -> Add -> 7C436110-AB2A-4BBB-A880-FE41995C9F82 -> boot-args`**:
   ```text
   mt7902-cc=US
   ```
   *(Replace `US` with your two-letter country code; add `-mt7902noverify` only if testing on an unverified macOS Tahoe/Sequoia build).*
4. Set **`Misc -> Security -> SecureBootModel`** to `Disabled`.

### 3. Important Wi-Fi Settings on macOS
* **Keep Private Wi-Fi Address Enabled:** In **System Settings -> Wi-Fi -> Details**, leave **Private Wi-Fi Address** set to **Fixed** or **Rotating** (setting it to **Off** prevents joining networks).
* **Supported Security:** WPA2-Personal (`CCMP`) is supported. WPA3-only and 802.1X Enterprise networks are not yet supported.

### 4. Validate, Reboot & Verify
1. Validate `config.plist` using `ocvalidate`, save, and reboot.
2. Allow ~20–40 seconds after kernel boot for the native Wi-Fi layer to initialize.
3. Verify the kext is active in Terminal:
   ```bash
   kextstat | grep -i mt7902
   ```

---

## How to Enable Bluetooth (MT7902 Firmware Setup)

Because the MT7902's Bluetooth module runs over an internal USB bus (commonly `13d3:3579`, `0489:e0e2`, or `0e8d:7902`), it requires a USB firmware loader alongside `BlueToolFixup.kext`.

### Step 1: Verify Your Bluetooth Device ID & USB Mapping
Before adding kexts, confirm macOS detects the internal Bluetooth USB endpoint:
1. Open **Hackintool** (under the **USB** tab), **System Information -> USB**, or run in Terminal:
   ```bash
   ioreg -p IOUSB -l -w0 | grep -i -E "13d3|0489|0e8d|MediaTek|AzureWave"
   ```
2. **Check your USB Port Map:** If the device does not appear at all, make sure the internal USB header/port (often `HS10` or `HS14`) is mapped and set to **Internal (`255` / `0xFF`)** in your `USBMap.kext` or `UTBMap.kext`.

### Step 2: Download the Required Kexts
1. **`Lilu.kext`** (latest release from Acidanthera)
2. **`BlueToolFixup.kext`** (from Acidanthera's **BrcmPatchRAM** release—required on macOS 12+ so `bluetoothd` initializes third-party controllers)
3. **`SimpleMtkBtFw.kext`** and **`SimpleMtkBtPatcher.kext`** from the MT7902-patched fork ([jigyanshu-codes/SimpleMtkBtFw](https://github.com/jigyanshu-codes/SimpleMtkBtFw), release `v1.1.1` or newer), which adds `0x7902` (`BT_RAM_CODE_MT7902_1_1_hdr.bin`) firmware support.
   * *(Tip: If your OEM variant uses a different USB VID/PID than `13d3:3579`, inspect `SimpleMtkBtFw.kext/Contents/Info.plist` under `IOKitPersonalities` and ensure your decimal `idVendor` and `idProduct` match your hardware).*

### Step 3: Configure OpenCore (`config.plist`)

#### 1. `Kernel -> Add` (Exact Load Order Matters)
Copy the kexts into `EFI/OC/Kexts/` and ensure they load in this exact order (after `Lilu.kext`):
1. `Lilu.kext`
2. `BlueToolFixup.kext`
3. `SimpleMtkBtFw.kext`
4. `SimpleMtkBtPatcher.kext`

*(Ensure no conflicting Bluetooth firmware kexts such as `IntelBluetoothFirmware` or `BrcmFirmwareData` are enabled).*

#### 2. `NVRAM -> Add -> 7C436110-AB2A-4BBB-A880-FE41995C9F82`
Add these two keys so macOS treats the controller as an internal Bluetooth module:
* `bluetoothInternalControllerInfo` | Type: `Data` | Value: `00000000 00000000 00000000 0000` (14 zero bytes)
* `bluetoothExternalDongleFailed` | Type: `Data` | Value: `00`

*(Also add `bluetoothInternalControllerInfo` and `bluetoothExternalDongleFailed` under `NVRAM -> Delete -> 7C436110-AB2A-4BBB-A880-FE41995C9F82` so stale values clear on reboot).*

#### 3. `NVRAM -> Add -> 7C436110-AB2A-4BBB-A880-FE41995C9F82 -> boot-args`
Append the following flags to your existing `boot-args`:
```text
-btlfxboardid -btlfxallowanyaddr
```

### Step 4: Cold Boot & Verify
1. **Perform a Cold Boot:** Save `config.plist`, shut down the system completely, disconnect power (or hold the power button for 10 seconds on a laptop) to reset the MediaTek chip's state, and power back on.
2. **Reset NVRAM** at the OpenCore boot picker if you added the `NVRAM -> Delete` entries.
3. Verify the Bluetooth firmware loaded in Terminal:
   ```bash
   log show --last boot --predicate 'eventMessage contains "SimpleMtkBt"'
   ioreg -r -c SimpleMtkBtFw -l | grep -i -E "FirmwareLoaded|fw_name"
   ```

---

## How to Compile from Source

### Prerequisites
* A Mac or Hackintosh running macOS with **full Xcode** installed (Command Line Tools alone are not sufficient).
* `MacKernelSDK` headers (included under `Source/MT7902MtkWlan/MacKernelSDK` and `Source/sdk26/MacKernelSDK`).

### Build Commands
1. Clone this repository and enter the `Source` directory:
   ```bash
   git clone https://github.com/jacksproject/MT7902-WiFi-Kext.git
   cd MT7902-WiFi-Kext/Source
   chmod +x build-mt7902.sh MT7902AirWiFiLayer/build.sh
   ```
2. **Build for macOS Tahoe (26):**
   ```bash
   DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer MT7902_MACOS=26 MT7902_VERSION=0.8.0 ./build-mt7902.sh release
   ```
3. **Build for macOS Sequoia (15):**
   ```bash
   DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer MT7902_MACOS=15 MT7902_VERSION=0.8.0 ./build-mt7902.sh release
   ```
4. The compiled kext bundle will be generated at:
   ```text
   Source/build-release/MT7902WiFi.kext
   ```

---

## Roadmap

* [x] Bring up MediaTek MT7902 Wi-Fi on AMD Ryzen 7 Hackintosh
* [x] Support macOS Tahoe (26) & macOS Sequoia (15) native Wi-Fi stack
* [x] Full `MT7902WiFi` rebrand (`MT7902WiFi.kext`, `mt7902-*` boot-args, build scripts, and GitHub Actions workflow)
* [x] Document MT7902 Bluetooth setup (`SimpleMtkBtFw` / `SimpleMtkBtPatcher` + `BlueToolFixup`)

---

## Credits & Acknowledgments

* **Maintainer:** **Shahin Sharkar** (`jacksproject`)
* **Base Bringup:** Special thanks to **[yusufcanyavic/YYavicWiFi](https://github.com/yusufcanyavic/YYavicWiFi)** by Yusufcan Yaviç for the base MT7922 bringup and native `IO80211` / `Skywalk` layer architecture.
* **Bluetooth Support (`SimpleMtkBt*` & `BlueToolFixup`):**
  * **[laobamac/SimpleMtkBtFw](https://github.com/laobamac/SimpleMtkBtFw)** (`SimpleMtkBtFw.kext` & `SimpleMtkBtPatcher.kext`) by `laobamac` for the MediaTek USB Bluetooth firmware loader and patcher.
  * **[jigyanshu-codes/SimpleMtkBtFw](https://github.com/jigyanshu-codes/SimpleMtkBtFw)** by `jigyanshu-codes` for patching `SimpleMtkBtFw` with MediaTek MT7902 (`0x7902`) firmware support.
  * **Acidanthera** (`BrcmPatchRAM` / `BlueToolFixup.kext` & `Lilu.kext`) for enabling third-party Bluetooth controllers on modern macOS.
* **Upstream Foundations:** Built upon `SimpleMtkWlan` (laobamac), `zxystd`'s macOS Wi-Fi framework, OpenBSD's `mwx` / `net80211`, and Acidanthera's `MacKernelSDK`.

---

## License

This project is licensed under the **GNU General Public License v3.0 or later (`GPL-3.0-or-later`)** — see the [LICENSE](LICENSE) file for full details.

* **Native Wi-Fi Layer (`Source/MT7902AirWiFiLayer`), Build Scripts & Rebrand Changes:** Licensed under `GPL-3.0-or-later`.
* **Radio Driver Base (`Source/MT7902MtkWlan`):** Retains its upstream `GPL-2.0-or-later` and OpenBSD `BSD` / `ISC` license notices as authoritative for their respective files.
* **Firmware Blobs (`FwBinary.cpp`):** Embedded MediaTek firmware is redistributed under MediaTek's firmware redistribution terms (not covered by GPL).
* **SDK Headers (`MacKernelSDK`):** Licensed under the **Apple Public Source License (APSL)** — see `MacKernelSDK/LICENSE.txt`.
