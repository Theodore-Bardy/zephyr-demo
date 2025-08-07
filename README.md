# Zephyr demo for Witekio

This Witekio's project is a demo based on Zephyr.

It stands to show the capabilities of Zephyr and how easy it is to develop secure and connected devices with Zephyr.

This project highlight the modularity of Zephyr and run the same code on different targets:
- [ESP32-S3-DevKitC-1](hhttps://docs.zephyrproject.org/latest/boards/espressif/esp32s3_devkitc/doc/index.html)
- More to come...

## Setup the build envrionement

**💡 Note:** Before running any `west` command you must activate the virtual environment:

```
source ~/zephyrproject/.venv/bin/activate
```

The virtual environment can be deactivated at any time by running `deactivate`.

### Using docker container

TODO

### On your host

**Zephyr**

To build the project on your host, the Zephyr project needs to be installed. For more information follow the offical Zephyr guide:

https://docs.zephyrproject.org/latest/develop/getting_started/index.html

**mender-mcu**

The project use the [mender-mcu](https://github.com/mendersoftware/mender-mcu) as a Zephyr module to support the OTA updates and the cloud telemetry.
Because `mender-mcu` is not part of the Zephyr's native module, we need to add it by hand.

1. Add `mender-mcu` to your project's west manifest (zephyr/west.yml):

```
manifest:
  projects:
    - name: mender-mcu
      url: https://github.com/mendersoftware/mender-mcu
      revision: main
      path: modules/mender-mcu
      import: true
```

2. Update the modules with:

```
west update
```

**Mender Artifacts**

In order to deploy software, the project will create [Mender Artifacts](https://docs.mender.io/downloads#mender-artifact) at the build time, which are files with the `.mender` suffix. To do so, the `mender-artifact` utility is used on the host.

```
sudo apt-get update
sudo apt-get install mender-artifact
```

**Espressif specific (ESP32)**

If you intend to build the project for ESP32, this ESP32-specific steps is required.
Espressif HAL requires WiFi and Bluetooth binary blobs in order work. Run the command below to retrieve those files.

```
west blobs fetch hal_espressif
```

## Build & Flash the project

### ESP32-S3-DevKitC-1

**Prerequisites**

- ESP32-S3-DevKitC board and a USB cable to connect it to your computer.
- Tenant token in your hosted Mender account. Go to your user menu → My organization → Organization token. Copy this token and paste it in the `prj.conf` at the option `CONFIG_MENDER_SERVER_TENANT_TOKEN`.
- WiFi network credentials.

**Build the first application with mcuboot**

First ***configure network credentials*** using the interactive Kconfig interface (`menuconfig`).

```
west build -t menuconfig
```

Then, ***build the firmware*** using `sysbuild`. [Sysbuild](https://docs.zephyrproject.org/latest/build/sysbuild/index.html) is higher-level build system that wraps `west`. Sysbuild will build the Zephyr application together with the MCUboot bootloader.

```
west build --sysbuild .
```

***Flash the device*** with the following command:

```
west flash
```

Optionaly you can watch the device's logs by ***monitoring the serial output***. For ESP32, west provides a monitor integration. Type the following command in a separate terminal:

```
west espressif monitor
```

WitekioPSK
v.w4A7btQS(#mH)ZZy>y

**Authorize the device in Mender**

If all the previous steps went well you should see the following logs in you terminal:

```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcb5400,len:0x1ec0
load:0x403ba400,len:0x79ac
load:0x403c6400,len:0x9dc
entry 0x403bced0
I (36) soc_init: MCUboot 2nd stage bootloader
[...]
*** Booting Zephyr OS build v4.1.0-1727-gc53fb67f568b ***
<inf> main: Hello world! esp32s3_devkitc/esp32s3/procpu
<inf> wifi_agent: Wi-Fi agent initialized
<inf> ota_agent: OTA agent initialized
<inf> ota_agent: OTA agent thread started
<inf> wifi_agent: Wi-Fi agent thread started
<inf> wifi_agent: Attempting to connect to Wi-Fi...
<inf> wifi_agent: Wi-Fi connected to AP-TEST
[...]
<inf> ota_agent: Mender client initialized
<inf> ota_agent: Update Module 'zephyr-image' initialized
<inf> ota_agent: Persistent inventory callback added
<inf> ota_agent: Mender client started
<inf> mender: Initialization done
<inf> mender: Checking for deployment...
<err> mender: [401] Unauthorized: dev auth: unauthorized
<err> mender: Authentication failed
<err> mender: Unable to perform HTTP request
<err> mender: Unknown error occurred, status=0
<err> mender: Unable to check for deployment
<err> mender: Work mender_client_main failed, retrying in 5 seconds
<err> mender: Unable to allocate memory
<err> mender: Unable to sign payload
<err> mender: Authentication failed
<err> mender: Unable to perform HTTP request
<err> mender: Unable to publish inventory data
<err> mender: Work mender_inventory failed, retrying in 60 seconds
```

This is the expected behavior, because the device is not accepted by the platform yet. Check on you Mender accound on the dashboard, you should see one device pending authorization.

Select the pending device and accept it. You should now see the following logs:

```
<inf> mender: Checking for deployment...
<inf> mender: No deployment available
```

**Deploy the firmware update**

After having modified the firmware, for example change the first log message in the main function, the artifact name needs to be changed, so the cloud like trigger that the current runing version on the device is different to the deployed one.

To change the mender artifact name change the `CONFIG_MENDER_SERVER_TENANT_TOKEN` option in the `prj.conf` file of the project.

Then recompile the project with `west build`. This time `sysbuild` is not needed since we're only building the application and not the bootloader.

The artifact can be found in the build directory (e.g., `build/zephyr-demo/zephyr/zephyr.mender`). Upload the artifact on your Mender account on the **Releases** tab.

Finally create the deployement, to do so in the **Devices** tab, find you device and select **Create a deployment for this device** from the **Device actions**.

On the terminal printing the device logs you should be able to see:

```
<inf> mender: Checking for deployment...
<inf> mender: Downloading artifact with id '60f737f...', name 'release.1.0.1', uri '...'
<inf> mender: Start flashing artifact 'zephyr.signed.bin' with size 755208
<inf> mender: Downloading 'zephyr-image' 10%... [75776/755208]
<inf> mender: Downloading 'zephyr-image' 20%... [151552/755208]
<inf> mender: Downloading 'zephyr-image' 30%... [226816/755208]
<inf> mender: Downloading 'zephyr-image' 40%... [302592/755208]
<inf> mender: Downloading 'zephyr-image' 50%... [377856/755208]
<inf> mender: Downloading 'zephyr-image' 60%... [453632/755208]
<inf> mender: Downloading 'zephyr-image' 70%... [528896/755208]
<inf> mender: Downloading 'zephyr-image' 80%... [604672/755208]
<inf> mender: Downloading 'zephyr-image' 90%... [679936/755208]
<inf> mender: Download done, installing artifact
<inf> mender: Artifact installation done, rebooting
```

And the device will reboot with the new firwmare installed and running:

```
[...]
<inf> main: Hello world VERSION 2! esp32s3_devkitc/esp32s3/procpu
[...]
```
