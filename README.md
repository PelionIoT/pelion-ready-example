# Simple Pelion Device Management Client - template application

(aka Simple Mbed Cloud Client template)

## Overview

This is a template application for platform vendors. It demonstrates how to create a simple application that can connect to the Pelion IoT Platform service, register resources and get ready to receive a firmware update.

It's intended to be forked and customized to add platform-specific features (such as sensors and actuators) and configure the connectivity and storage to work **out-of-the-box**. The template application works in **developer mode** by default.

There is a mirror version of the stable (master) template application on [this location](https://os.mbed.com/teams/mbed-os-examples/code/mbed-cloud-example) to facilitate the fork and publish on os.mbed.com.

## Table of Contents

1. [Getting started with the application](#getting-started-with-the-application)
2. [Porting to a new platform](#porting-to-a-new-platform)
3. [Enabling firmware updates](#enabling-firmware-updates)
4. [Automated testing](#automated-testing)
5. [Known issues](#known-issues)

## Getting started with the application

This is a summary of the process for developers to get started and get a device connected to Pelion IoT Device Management.

### Mbed Online IDE

1. Import the application into the Online IDE.
2. Add the API key to establish connection to Pelion Device Management.
3. Install the developer certificate.
4. Compile and program.

### Mbed CLI tools

1. Import the application into your desktop:

    ```
    mbed import https://os.mbed.com/teams/mbed-os-examples/code/mbed-cloud-example
    cd mbed-cloud-example
    ```

2. Download the developer certificate from the Pelion Device Management [portal](https://portal.mbedcloud.com).
3. Compile and program:

    ```
    mbed compile -t <toolchain> -m <target> -f
    ```

## Porting to a new platform

### Requirements

The hardware requirements for Mbed OS platforms to support Pelion Client are [here](https://cloud.mbed.com/docs/current/cloud-requirements/index.html).

In general, to start creating a secure connected product, you need a microcontroller that has the following features:

* RAM: 96K or more
* Flash: 512K or more
* True Random Number Generator (TRNG)
* Real Time Clock (RTC)

Additionally, to use the Pelion Client, the microcontroller needs to support the following in Mbed OS (latest version preferred) or in a compatible driver library:

* A storage device (SDcard, SPI Flash, Data Flash)
* IP connectivity (Ethernet, WiFi, Cellular, 6LoWPAN, Thread)

For the Firmware update over the air (FOTA), you need the following:

* [FlashIAP](https://github.com/ARMmbed/mbed-os/blob/master/drivers/FlashIAP.h) - Flash In-Application Programming (IAP).
* [Mbed Bootloader](https://github.com/ARMmbed/mbed-bootloader) or a bootloader compatible with Pelion Client.
* TCP connection - the current Firmware Download client only supports HTTP download over TCP (this will be resolved in a future release, so that CoAP blockwise transfer will be used for UDP connections).

### References

* Check which Mbed OS platforms are supported in the [Pelion Device Management quick-start guide](https://cloud.mbed.com/quick-start).
* Check which storage options are available [here](https://os.mbed.com/docs/latest/reference/storage.html).
* Check which network options are available [here](https://os.mbed.com/docs/latest/reference/network-socket.html).


### Porting steps

Supporting a new derivative platform requires the following steps:

* Fork the template and create an example application for your platform in https://os.mbed.com/teams/your-team
* Change the connectivity interface. Ethernet is the default - see `main.cpp`.
* Change the filesystem and/or the block device for storage. FAT filesystem over SD card is the default. See `main.cpp`.
* (Optional) Make minor changes in `mbed_app.json` to support multiple platforms with same connectivity and storage.
* (Recommended) Remove information and files not related to the platform you're porting.

<span class="notes">**Note:** Make sure that the application works out-of-the-box and no changes are required in the `main.cpp` file nor `mbed_app.json`. The goal is to deliver a great UX to our developers.</span>

### Porting example

In this example, an app with an SD card and on-chip Ethernet is taken to a custom board that has an MCU + Wi-Fi module.

#### Changing the storage option

<span class="notes">**Note:** From Mbed OS 5.10+, block device drivers have been moved to `mbed-os/components/storage/blockdevice` and many platforms have a default block device interface. </span>

##### Non-default storage configuration

If you wish to override the default storage configuration or add support for storage, you can add the configuration into the `mbed_app.json` file. For example:

```json
    "NUCLEO_F429ZI": {
        "target.features_add"  : ["STORAGE"],
        "target.components_add": ["SD"],
        "sd.SPI_MOSI"  : "PE_6",
        "sd.SPI_MISO"  : "PE_5",
        "sd.SPI_CLK"   : "PE_2",
        "sd.SPI_CS"    : "PE_4"
    }
```

##### Example of default storage configuration using Mbed OS 5.10+

1. Include the header files for the FAT file system:

    ```cpp
    #include "FATFileSystem.h"
    ```

2. Declare the global object for the default block device driver 

    ```cpp
    BlockDevice* bd = BlockDevice::get_default_instance();
    ```

3. Declare the global objects for the file system.

    ```cpp
    FATFileSystem fs("sd", bd);
    ```

##### Example of SD card configuration using Mbed OS 5.9 and older

1. Add the SD card driver (`sd-driver.lib`) if it is not already added.

    On the command line:

    ```
    mbed add https://github.com/armmbed/sd-driver
    ```

2. Include the header files for the SD driver and FAT file system:

    ```cpp
    #include "SDBlockDevice.h"
    #include "FATFileSystem.h"
    ```

3. Declare the global objects for the SD card and file system.

    ```cpp
    SDBlockDevice bd(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS);
    FATFileSystem fs("sd", &sd);
    ```

<span class="notes">**Note:** The `SPI_*` macros represent the pin names. The names can be defined in a variety of places including the sd-driver, your project’s configuration file (`mbed_app.json`) or the `pinnames.h` file for the target that defines the default pin names. You can use other pin names depending on the platform and the connections.</span>

For example, if the SPI signals for the SD card interface are connected on an Arduino compatible shield, you may define them like this:

```cpp
SDBlockDevice sd(D11, D12, D13, D10);
```

<span class="notes">**Note:** The default mounting point is `sd`. This can be overridden in the `mbed_app.json` file using `PAL_FS_MOUNT_POINT_PRIMARY` and `PAL_FS_MOUNT_POINT_SECONDARY`. See [documentation](https://cloud.mbed.com/docs/current/porting/port-filesystem.html).</span>

##### For SPI Flash (devices that support SFDP)

<Please note that this section of the document is under construction.  More information is needed.>

1. Add the SPI Flash driver (`spif-driver`) if it is not already added.

    ```
    mbed add https://github.com/armmbed/spif-driver
    ```

2. Include the header files for the SPI Flash driver and LitteFS file system. For SPI Flash, we recommend LittleFS file system which supports wear leveling.

    ```cpp
    #include "SPIFBlockDevice.h"
    #include "LittleFileSystem.h"
    ```

3. Declare the global objects for the SD card and file system:

    ```cpp
    SPIFBlockDevice spif(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS);
    LittleFileSystem fs("fs", &spif);
    ```

4. Update the construction of the `SimpleMbedCloudClient` object to pass in the file system and block device:

    ```cpp
    SimpleMbedCloudClient client(&net, &spif, &fs);
    ```

#### Changing the network interface

<span class="notes">**Note:** From Mbed OS 5.10, platforms have a default network interface defined in `mbed-os/targets/targets.json`. If you wish to override the default configuration, you can add the configuration into the `mbed_app.json` file.</span>

##### Non-default network configuration

If you wish to override the default network configuration, you can add the configuration into the `mbed_app.json` file. For example:

```json
    "NUCLEO_F429ZI": {
        "target.network-default-interface-type" : "WIFI",
        "esp8266.rx"                            : "D0",
        "esp8266.tx"                            : "D1",
        "esp8266.provide-default"               : true,
        "nsapi.default-wifi-security"           : "WPA_WPA2",
        "nsapi.default-wifi-ssid"               : "\"SSID\"",
        "nsapi.default-wifi-password"          : "\"Password\""
    }
```

##### Example of network initialization for Ethernet using Mbed OS 5.10+

1. Declare the network interface object.
   
    ```
    EthernetInterface * net = NetworkInterface::get_default_instance();
    ```

2. Connect the interface.
   
    ```
    status = net->connect();
    ```

3. When Pelion Client is started, pass the network interface.
    ```
    SimpleMbedCloudClient client(net, &sd, &fs);
    ```

##### Example of network initialization for WiFi using Mbed OS 5.10+

1. Declare the network interface object.
   
    ```
    WiFiInterface *net = WiFiInterface::get_default_instance();
    ```

2. Connect the interface.
   
    ```
    status  = net->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    ```

3. When Pelion Client is started, pass the network interface.
    ```
    SimpleMbedCloudClient client(net, &sd, &fs);
    ```


##### Example of network initialization for Ethernet using Mbed OS 5.9 and older versions

The Ethernet interface is included within Mbed OS, so you do not need to add a library.

1. Include the header file for the interface.
    ```
    #include "EthernetInterface.h"
    ```

2. Declare the network interface object.
    ```
    EthernetInterface net;
    ```

3. Connect the interface.
    ```
    status = net.connect();
    ```

4. When Pelion Client is started, pass the network interface.
    ```
    SimpleMbedCloudClient client(&net, &sd, &fs);
    ```

##### Example of network initialization for WiFi using Mbed OS 5.9 and older versions

This example references the ESP8266 WiFi module, but the instructions are applicable to other modules.

1. Add the ESP8266 WiFi interface driver (esp8266-driver) if it is not already added.
   
    ```
    mbed add https://github.com/ARMmbed/esp8266-driver
    ```

    <span class="notes">**Note:** You may have to update the firmware inside the ESP8266 module.</span>

2. Include the header file for the interface.
   
    ```cpp
    #include "ESP8266Interface.h"
    ```

3. Declare the network interface object.
   
    ```cpp
    ESP8266Interface net(D1, D0);
    ```

4. Connect the interface.
   
    ```cpp
    nsapi_error_t status = net.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    ```

5. When Pelion Client is started, pass the network interface.
   
    ```cpp
    SimpleMbedCloudClient client(&net, &sd, &fs);
    ```

6. Add the WiFi credentials information in `mbed_app.json` (located at the top level of the Simple Pelion Client example project).
   
    ```json
        "config": {
            "wifi-ssid": {
                "help": "WiFi SSID",
                "value": "\"SSID\""
            },
            "wifi-password": {
                "help": "WiFi Password",
                "value": "\"PASSWORD\""
            }
        }
    ```

#### Changing the target MCU

To change the target board to another board that is supported by Mbed OS, simply change the target name.

##### Using the command line

Use the -m option. For example:

```
mbed compile -m Hexiwear -t GCC_ARM
```

##### Using the online compiler

Click the platform name on the top right corner, then select another platform.

##### Using an IDE

First re-export (create project files) for the target with the command line. For example:

```
mbed export -m Hexiwear -i uvision
```

#### Creating a custom target board

Read the Mbed OS [Contributing](https://os.mbed.com/docs/latest/reference/porting-targets.html) documentation on how to add a new target.

#### Update the application logic

The template example uses a ticker object to periodically fire a software interrupt to simulate button presses. Let’s say you want to make an actual button press.

By default, there is a Ticker object, which fires every five seconds and invokes a callback function.

```cpp
Ticker timer;
timer.attach(eventQueue.event(&fake_button_press), 5.0);
```

This callback function changes the `button_res` resource:

```cpp
void fake_button_press() {
    int v = button_res->get_value_int() + 1;

    button_res->set_value(v);

    printf("Simulated button clicked %d times\n", v);
}
```

If you want to change this to an actual button, here is how to do it:

1. Remove:

    ```cpp
    Ticker timer;
    timer.attach(eventQueue.event(&fake_button_press), 5.0);
    ```

2. Declare an `InterruptIn` object on the button, and attach the callback function to the `fall` handler:

    ```cpp
    InterruptIn btn(BUTTON1);
    btn.fall(eventQueue.event(&fake_button_press), 5.0);
    ```

3. Rename `fake_button_press` to `real_button_press`.


#### Pelion Client v1.3.x SOTP-specific changes

Pelion Client v1.3.x introduces a new feature called Software One-Time Programming (SOTP) that makes use of the internal flash of the MCU as an One-Time-Programmable section. It stores the keys required to decrypt the credentials stored in the persistent storage. Read more on this in the [porting documentation](https://cloud.mbed.com/docs/current/porting/changing-a-customized-porting-layer.html#rtos-module) under the RTOS module section.

The flash must be divided into two sections (default 2, maximum 2) for your target. You need to modify the `mbed_app.json` file as follows:

1. Add a section to the `target_overrides` with SOTP addresses and sizes.

    Here is an example for the NUCLEO_L476RG board. Note that with these flash sectors, the SOTP region is placed at the last two sectors of the flash. You can find the memory map information in the reference manual of your MCU.

    ```json
        "NUCLEO_L476RG": {
            "sotp-section-1-address"           : "(0x08000000+((1024-32)*1024))",
            "sotp-section-1-size"              : "(16*1024)",
            "sotp-section-2-address"           : "(0x08000000+((1024-16)*1024))",
            "sotp-section-2-size"              : "(16*1024)",
            "sotp-num-sections"                : 2
        }
    ```

2. Add the macro definition to the "config" section. Note that the address and size macros are already provided. You only need to add the macro for the number of sections:

    ```json
        "sotp-num-sections": {
            "help": "Number of SOTP sections",
            "macro_name": "PAL_INT_FLASH_NUM_SECTIONS",
            "value": null
        }
    ```

## Enabling firmware updates

Mbed OS 5.10 and Mbed CLI 1.8 simplifies the process to enable and perform Firmware Updates. Here is a summary on how to configure the device and verify its correct behaviour.

For full documentation about bootloaders and firmware update, read the following documents:

- [Introduccion to bootloaders](https://os.mbed.com/docs/latest/porting/bootloader.html)
- [Creating and using a bootloader](https://os.mbed.com/docs/latest/tutorials/bootloader.html)
- [Bootloader configuration in Mbed OS](https://os.mbed.com/docs/latest/tools/configuring-tools.html)
- [Mbed Bootloader for Pelion IoT Device Management](https://github.com/ARMmbed/mbed-bootloader)
- [Updating devices with Arm Mbed CLI](https://os.mbed.com/docs/latest/tools/cli-update.html)
  
This is a summary to use Arm Mbed OS managed bootloaders.

#### Preparing a bootloader

If Mbed OS contains a default pre-built bootloader in `mbed-os/feature/FEATURE_BOOTLOADER`, then you can skip this section.

Otherwise, you'll need to compile the [mbed-bootloader](https://github.com/armmbed/mbed-bootloader) and add it to your application. Once it's done, we recommend to send a Pull-Requests to [Mbed OS](https://github.com/ARMmbed/mbed-os) to contribute with a default bootloader for your Mbed Enabled platform.

<span class="notes">**Note:** Make sure the configuration in the `mbed-bootloader/mbed_app.json` corresponds with the configuration in your application's `mbed_app.json`, otherwise the bootloader will not be able to apply the new firmware.</span>

#### Enabling the application to use a bootloader 

- Option 1: default & prebuilt bootloader

    If Mbed OS contains a prebuilt bootloader for the target, then you can indicate to use it in the `mbed_app.json`. For example:

    ```
    {
        "target_overrides": {
            "K64F": {
                "target.features_add": ["BOOTLOADER"]
            }
        }
    }
    ```

- Option 2: custom bootloader

    If you'd like to overide a default bootloader or use a custom one available in the application, then indicate the path to the booloader, `app_offset` and `header_offset` parameters in `mbed_app.json`. For example:

    ```
    "target_overrides": {
            "K64F": {
                "target.app_offset": "0xa400",
                "target.header_offset": "0xa000",
                "target.bootloader_img": "bootloader/my_bootloader.bin"
            }
        }
    ```

    You may need to specify `header_format` as well. You could include the default header format from [Mbed OS](https://github.com/ARMmbed/mbed-os/blob/master/features/FEATURE_BOOTLOADER/mbed_lib.json) by adding `"target.features_add": ["BOOTLOADER"]`.

#### Verifying that firmware update works

Follow these steps to generate a manifest, compile and perform a firmware update of your device:

1. Configure the API key for your Pelion account.

     If you don't have an API key available, then login in [Pelion IoT Platform portal](https://portal.mbedcloud.com/), navigate to 'Access Management', 'API keys' and create a new one. Then specify the API key as global `mbed` configuration:

    ```
    mbed config -G CLOUD_SDK_API_KEY <your-api-key>
    ```

2. Initialize the device management feature 

    ```
    mbed dm init -d "company.com" --model-name "product-model" -q --force
    ```

3. Compile the application, include the firware update credentials generated before, merge with the bootloader and program the device

    ```
    mbed compile -t <target> -m <toolchain> -c -f
    ```

4. Open a serial terminal, verify the application boots and is able to register to the Pelion Device Management service. Write down the `<endpoint ID>`, as it's required to identify the device to perform a firmware update.

5. Update the firmware of the device through Mbed CLI
   
    ```
    mbed dm update device -D <device ID>
    ```
    
    Inspect the logs on the device to see the update progress. It should look similar to:

    ```
    Firmware download requested
    Authorization granted
    Downloading: [+++- ] 6 %
    ```

    When the download completes, the firmware is verified. If everything is OK, the firmware update is applied, the device reboots and attemps to connect to Pelion Device Management service again. The `<endpoint ID>` should be preserved.

## Automated testing

The Simple Pelion Client provides Greentea tests to confirm your platform works as expected. The network and storage configuration is already defined in Mbed OS 5.10, but you may want to override the configuration in `mbed_app.json`.

For details on Simple Pelion Client testing, refer to the documentation [here](https://github.com/ARMmbed/simple-mbed-cloud-client/tree/def_network#tests).

This template application contains a working application and tests passing for the `K64F` and `K66F` platforms.

## Known issues

Please check the issues reported on github.
