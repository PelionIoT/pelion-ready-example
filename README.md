# Simple Mbed Cloud Client template application

## Overview

This is a template application for platform vendors. It demonstrates how to create a simple Mbed Cloud Client application that can connect to Mbed Cloud, register resources and get ready to receive a firmware update.

It's intended to be customized to add platform-specific features (sensors, actuators, etc) and configure the connectivity and storage to work **out-of-the-box**.
The Simple Mbed Cloud Client template application works in **developer mode** by default.

## Setup process

This is a summary of the process for developers to get started and get a device connected to Mbed Cloud.

### Mbed Online IDE

* Import application into the Online IDE
* Add API key to establish connection to Mbed Cloud
* Install developer certificate
* Compile & program

### Mbed CLI tools

* Import application in developer's desktop:

    ```
    mbed import https://os.mbed.com/teams/mbed-os-examples/code/mbed-cloud-example
    cd mbed-cloud-example
    ```

* Download developer certificate from Mbed Cloud.
* Compile & program:

    ```
    mbed compile -t <toolchain> -m <target> -f
    ```

## Porting to a new platform

### Requirements
The hardware requirements for Mbed OS platforms to support Mbed Cloud Client are shown [here](https://cloud.mbed.com/docs/current/cloud-requirements/index.html).

In general, to start creating a secure connected product, you need a microcontroller that has the following features.
*	RAM: 96K or more
*	Flash: 512K or more
*	True Random Number Generator (TRNG)
*	Real Time Clock (RTC)

Additionally, to use Mbed Cloud Client, the Microcontroller needs support for the following in Mbed-OS (latest version preferred) or in a compatible driver library.
*	Storage Device (SDcard, SPI Flash, Data Flash)
*	IP connectivity (Ethernet, WiFi, Cellular, 6lowpan, Thread)

For Firmware update over the air (FOTA), the following is also needed.
*	[FlashIAP](https://github.com/ARMmbed/mbed-os/blob/master/drivers/FlashIAP.h) - Flash In-Application Programming (IAP)
*	[Mbed Bootloader](https://github.com/ARMmbed/mbed-bootloader) or compatible bootloader with Mbed Cloud Client

### References
* Check which Mbed OS platforms are supported in the [Mbed Cloud quick-start guide](https://cloud.test.mbed.com/quick-start)
* Check which storage options are available [here](https://os.mbed.com/docs/v5.8/reference/storage.html)
* Check which network options are available [here](https://os.mbed.com/docs/v5.8/reference/network-socket.html)


### Porting Steps

Supporting a new derivative platform requires the following steps:

* Fork the template and create an example application for your platform in https://os.mbed.com
* [Optional] Change connectivity interface. By default uses Ethernet - see `main.cpp`.
* [Optional] Change the filesystem and/or the block device for storage. By default uses FAT filesystem over SD card. See `main.cpp`.
* [Optional] Make minor changes in `mbed_app.json` to support multiple platforms with same connectivity and storage.

 **Note:** Make sure to the application works Out-of-the-Box and no changes are required in `main.cpp`. The goal is to deliver a great UX to our developers.

### Porting Example

In this example, we’re going take an app that uses SD Card and on-chip Ethernet, to a custom board that has an MCU + Wi-Fi module.

#### If required, change the storage option.

##### For SD Card:

	Add the SD Card driver (sd-driver.lib) if it is not already added.

On the command line

```
mbed add https://github.com/armmbed/sd-driver
```

In the online compiler, click Import, then Click here to import from URL. Then enter https://github.com/armmbed/sd-driver for Source URL and Import As: Library.

Next include the header files for the SD Driver and FAT File system.

```cpp
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
```

Declare global objects for the SD Card and File System.

```cpp
SDBlockDevice bd(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS);
FATFileSystem fs("sd", &sd);
```

Note that the `SPI_*` macros represent pin names.  These pin names can be defined in a variety of places including the sd-driver, your project’s configuration file (`mbed_app.json`) or the pinnames.h file for the target that defines default pin names.  You can use other pin names depending on the platform and the connections.

For example, if the SPI signals for the SD Card interface are connected on an Arduino compatible shield, you may define them like this:

```cpp
SDBlockDevice sd(D11, D12, D13, D10);
```

##### For SPI Flash (devices that support SFDP):

<Please note that this section of the document is under construction.  More information is needed.>

Add the SPI Flash driver (spif-driver) if it is not already added.

  ```
  mbed add https://github.com/ARMmbed/spif-driver
  ```

Next include the header files for the SPI Flash Driver and LitteFS file system.  For SPI Flash, we recommend LittleFS file system which has wear leveling support.

```cpp
#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"
```

Declare global objects for the SD Card and File System.

```cpp
SPIFBlockDevice spif(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS);
LittleFileSystem fs("fs", &spif);
```

Also update the construction of the `SimpleMbedCloudClient` object to pass in the file system and block device:

```cpp
SimpleMbedCloudClient client(&net, &spif, &fs);
```

#### If required, change the Network interface.

##### For Ethernet:

The Ethernet interface is included within Mbed OS, so no need to add a library.
Include the header file for the interface.

```
#include "EthernetInterface.h"
```

Declare the network interface object.

```
EthernetInterface net;
```

Connect the interface.

```
status = net.connect();
```

When the Mbed Cloud Client is started, pass in the network interface.

```
SimpleMbedCloudClient client(&net, &sd, &fs);
```

##### For WiFi:

This example references the ESP8266 WiFi module, but the instructions are applicable to others.

Add the ESP8266 Wi-Fi Interface driver (esp8266-driver) if it is not already added.

```
Mbed add https://github.com/ARMmbed/esp8266-driver
```

Note that you may have to update the firmware inside the ESP8266 module.

Next include the header file for interface.

```cpp
#include "ESP8266Interface.h"
```

  Add driver configuration information in `mbed_app.json` (located at the top level of the Mbed Cloud Connect example project)

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

Declare the network interface object.

```cpp
ESP8266Interface net(D1, D0);
```

Connect the interface.

```cpp
nsapi_error_t status = net.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
```

When the Mbed Cloud Client is started, pass in the network interface.

```cpp
SimpleMbedCloudClient client(&net, &sd, &fs);
```

#### Change the target MCU
To change the target board to another board that is supported by Mbed OS, simply change the target name.

##### Using the command line

Use the -m option.

```
mbed compile -m Hexiwear -t GCC_ARM
```

##### Using the online compiler.
Click the platform name on the top right corner, then select another platform.

##### Using an IDE
First re-export (create project files) for the target with the command line.

```
mbed export -m Hexiwear -i uvision
```

#### To create a custom target board

Read the Mbed OS [Contributing](https://os.mbed.com/docs/latest/reference/porting-targets.html) documentation on how to add a new target.

#### Update the application logic

The template example uses a ticker object to periodically fire a software interrupt to simulate button presses.  Let’s say you want to make an actual button press.

By default, there is a Ticker object, which fires every 5 seconds and invokes a callback function.

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

If you want to change this to an actual button, here is what you do:

1. Remove:

    ```cpp
    Ticker timer;
    timer.attach(eventQueue.event(&fake_button_press), 5.0);
    ```

1. Declare an `InterruptIn` object on the button, and attach the callback function to the `fall` handler:

    ```cpp
    InterruptIn btn(BUTTON1);
    btn.fall(eventQueue.event(&fake_button_press), 5.0);
    ```

1. Rename `fake_button_press` to `real_button_press`.


#### Update the LWM2M objects.

See guide at [TODO]

## Enabling firmware updates

To enable firmware updates a compatible bootloader needs to be added in the `tools/` folder.
The process to merge the application with the bootloader currently only works when building with Mbed CLI. In the future, this combine process will be done automatically by Mbed tools.

1. Compile [mbed-bootloader](https://github.com/armmbed/mbed-bootloader) for the platform and storage configuration used in this application; and place the binary in the tools folder.
1. Add a section to `mbed_app.json` under `target_overrides` with the bootloader configuration. An example is:

    ```json
        "K64F": {
            "target.mbed_app_start"            : "0x0000a400",
            "update-client.bootloader-details" : "0x00007188",
            "sotp-section-1-address"           : "(32*1024)",
            "sotp-section-1-size"              : "( 4*1024)",
            "sotp-section-2-address"           : "(36*1024)",
            "sotp-section-2-size"              : "( 4*1024)",
            "update-client.application-details": "(40*1024)"
        }
    ```

Next, instruct your users to do the following:

1. Install the [manifest tool](https://github.com/armmbed/manifest-tool).
1. Generate an update certificate:

    ```
    $ manifest-tool init -a YOUR_MBED_CLOUD_API_KEY -d yourdomain.com -m device-model-id -q --force
    ```

    **Note:** Make sure to replace `YOUR_MBED_CLOUD_API_KEY` with an Mbed Cloud API key.

1. Build the application and combine it with the bootloader:

    ```
    $ mbed compile -m YOUR_TARGET -t GCC_ARM
    $ tools/combine_bootloader_with_app.py -m YOUR_TARGET -a BUILD/YOUR_TARGET/GCC_ARM/simple-mbed-cloud-client-example_application.bin -o combined.bin
    ```

1. Flash `combined.bin` to the development board.
1. Write down the endpoint ID of the board. It's needed to start the update.

Now an Firmware Update can be scheduled as explained in the [Mbed Cloud documentation](https://cloud.mbed.com/docs/current/updating-firmware/index.html). This can be done with the manifest tool itself or with help from the Mbed Cloud portal. Here we explain how to do it with the manifest tool.

1. Change the application, for example by changing some strings in `main.cpp`.
1. Compile the application:

    ```
    $ mbed compile -m YOUR_TARGET -t GCC_ARM
    ```

1. The manifest tool can both sign the update - using the private key generated earlier - and upload it to Mbed Cloud in a single command. Run:

    ```
    $ manifest-tool update device -p BUILD/YOUR_BOARD_NAME/GCC_ARM/simple-mbed-cloud-client-example_application.bin -D YOUR_ENDPOINT_NAME
    ```

    Replace `YOUR_BOARD_NAME` with the name of your development board, and replace `YOUR_ENDPOINT_NAME` with the endpoint name in Mbed Cloud.

1. Inspect the logs on the device to see the update progress. It looks similar to:

    ```
    Firmware download requested
    Authorization granted
    Downloading: [+++-                                              ] 6 %
    ```

1. When the download completes, the firmware is verified. If everything is OK, the firmware update is applied.

## Known issues

None.
