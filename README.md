# Simple Mbed Cloud Client template application

## Overview

This is a template application for platform vendors. It demonstrates how to create a simple Mbed Cloud Client application that can connect to Mbed Cloud, register resources and get ready to receive a firmware update.

It's intended to be customized to add platform-specific features (sensors, actuators, etc) and configure the connectivity and storage to work **out-of-the-box**.
The Simple Mbed Cloud Client template application works to work in **developer mode** by default.

## Setup process

This is a summary of the process for developers to get started and get a device connected to Mbed Cloud.

### Mbed Online IDE

* Import application into the Online IDE
* Add API key to establish connection to Mbed Cloud
* Install developer certificate
* Compile & program

### Mbed CLI tools

* Import application in developer's desktop

```
mbed import https://os.mbed.com/teams/mbed-os-examples/code/mbed-cloud-example
cd mbed-cloud-example
```

* Download developer certificate from Mbed Cloud
* Compile & program

```
mbed compile -t <toolchain> -m <target> -c -f
```

## Porting to a new platform

### Requirements 
The hardware requirements for Mbed OS platforms to support Mbed Cloud Client as shown [here].

In general, to start creating a secure connected product, you need a microcontroller that has the following features. 
*	RAM: 96K or more
*	Flash: 512K or more
*	True Random Number Generator (TRNG)
*	Real Time Clock (RTC)

Additionally, to use Mbed Cloud Client, the Microcontroller needs support for the following in Mbed-OS (latest version preferred) or in a compatible driver library. 
*	Storage Device (SDcard, SPI Flash, Data Flash)
*	IP connectivity (Eth, Wifi, Cellular, 6lowpan, Thread)

For Firmware update over the air (FOTA), the following is also needed.  
*	Flash In-Application Programming (IAP)
*	Bootloader  (https://github.com/ARMmbed/mbed-bootloader)

### References 
* Check which Microcontroller platforms are supported here [?]
* Check which storage options are available here [?]
* Check which network options are available here [?]


### Porting Steps

Supporting a new derivative platform requires the following steps:

* Fork the template and create an example application for your platform in https://os.mbed.com
* [Optional] Change connectivity interface. By default uses Ethernet - see main.cpp.
* [Optional] Change the filesystem and/or the block device for storage. By default uses FAT filesystem over SD card. See main.cpp.

### Porting Example
In this example, we’re going take an app that uses SD Card and on-chip Ethernet, to a custom board that has an MCU + Wi-Fi module.  

#### If required, change the storage option.

##### For SD Card:

	Add the SD Card driver (sd-driver) if it is not already added.
	
  On the command line 

  ```
  mbed add https://github.com/armmbed/sd-driver 
  ```

  In the online compiler, click Import, then Click here to import from URL. Then enter https:\\github.com/armmbed/sd-driver for Source URL and Import As: Library.  

  Next include the header files for the SD Driver and FAT File system.

  ```
  #include "SDBlockDevice.h"
  #include "FATFileSystem.h"
  ```
    
  Declare global objects for the SD Card and File System.

  ```
  SDBlockDevice sd(SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
  FATFileSystem fs("sd");
  ```

  Note that the SPI_MOSI, SPI_MISO, etc macros represent pin names.  These pin names can be defined in a variety of places including the sd-driver, your project’s configuration file (mbed_app.json) or the pinnames.h file for the target that defines default pin names.  You can use other pin names depending on the platform and the connections.

  For example, if the SPI signals for the SD Card interface are connected on an Arduino compatible shield, you may define them like this:

  ```
  SDBlockDevice sd(D11, D12, D13, D10);
  ```
  
##### For SPI Flash (devices that support SFDP): 

<Please note that this section of the document is under construction.  More information is needed.>

Add the SPI Flash driver (spif-driver) if it is not already added.

  ```
  mbed add https://github.com/ARMmbed/spif-driver
  ```
Next include the header files for the SPI Flash Driver and LitteFS file system.  For SPI Flash, we recommend LittleFS file system which has wear leveling support.

``` 
#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"
```
Declare global objects for the SD Card and File System.

```
SPIFBlockDevice spif(SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS); 
LittleFileSystem fs("fs");
```

Update a few function calls to reference the SPI flash object you created.

```
status = fs.mount(&spif);
status = fs.reformat(&spif);
```
  
#### If required, change the Network interface.

For Ethernet:
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
SimpleMbedCloudClient mbedClient(&net);
```

##### For WiFi: 

This example references the ESP8266 WiFi module, but the instructions are applicable to others.

Add the ESP8266 Wi-Fi Interface driver (esp8266-driver) if it is not already added.

```
Mbed add https://github.com/ARMmbed/esp8266-driver
```

Note that you may have to update the firmware inside the ESP8266 module.

Next include the header file for interface.

```
#include "ESP8266Interface.h"
```
 
  Add driver configuration information in mbed_app.json (located at the top level of the Mbed Cloud Connect example project)
  
```
    "config": {
        "developer-mode": {
            "help": "Enable Developer mode to skip Factory enrollment",
            "value": 1
        },
        "network-interface":{
            "help": "Options are ETHERNET, WIFI_ESP8266, WIFI_ODIN",
            "value": "WIFI_ESP8266"
        },
        "wifi-ssid": {
            "help": "WiFi SSID",
            "value": "\"SSID\""
        },
        "wifi-password": {
            "help": "WiFi Password",
            "value": "\"PASSWORD\""
        },
        "wifi-tx": {
            "help": "TX pin for serial connection to external device",
            "value": "PTD3"
        },
        "wifi-rx": {
            "help": "RX pin for serial connection to external device",
            "value": "PTD2"
        }

    }
```

Declare the network interface object.

```
ESP8266Interface net(MBED_CONF_APP_WIFI_TX, MBED_CONF_APP_WIFI_RX);  //pins defined in mbed_app.json
```
  
Connect the interface.

```
status = net.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
```
  
When the Mbed Cloud Client is started, pass in the network interface.  

```
SimpleMbedCloudClient mbedClient(&net);
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
    
  	References to porting guides to come…
    
#### Update the application logic

The template example uses a ticker object to periodically fire a software interrupt to simulate button presses.  Let’s say you want to make an actual button press. 

By default, there is a Ticker object.

```
Ticker timer;
```

A callback function is called when the timer value of 5 seconds expires.

```
timer.attach(&button_press, 5.0);
```

The callback function just indicates that a button was pressed.

```
void button_press() {
    		button_pressed = true;
}
```

There is a button resource defined for Mbed Cloud Client.

```
    // Mbed Cloud Client resource setup
    MbedCloudClientResource *button = mbedClient.create_resource("3200/0/5501", "button_resource");
    button->set_value("0");
    button->methods(M2MMethod::GET);
    button->observable(true);
    button->attach_notification_callback(button_callback);
```

The button count (number of times the button is pressed) gets incremented when the button_press global variable is set by the ticker callback.  This value is passed to the button resource.  

```
    while (mbedClient.is_register_called()) {
        static int button_count = 0;
        wait_ms(100);
        if (button_pressed) {
            button_pressed = false;
            printf("Simulated button clicked %d times\r\n", ++button_count);
            button->set_value(button_count);
        }
    }
```

If you want to change this to an actual button, here is what you do.

Declare a global variable for the button count.

```
static int button_count = 0;
```

Create a global pointer for use with the button resource.

```
static MbedCloudClientResource* button_ptr;
```

Now instead of a Ticker, use an InterruptIn object to trigger an interrupt when a button is pressed.  In this case, we are using SW2, which is defined for the platform.  

```
InterruptIn sw2(SW2);
```

Set internal pullup resistor and callback function.

```
sw2.mode(PullUp);
sw2.fall(button_press);
```

Now in the callback function, update the button count and pass to button resource. 

```
void button_press() {
    button_pressed = true;
    ++button_count;
    button_ptr->set_value(button_count);
}
```

Make sure the new pointer is assigned to the button resource

```
    //Mbed Cloud Client resource setup
    MbedCloudClientResource *button = mbedClient.create_resource("3200/0/5501", "button_resource");
    button->set_value("0");
    button->methods(M2MMethod::GET);
    button->observable(true);
    button->attach_notification_callback(button_callback);
    button_ptr = button;
```


In the main function, clear the button_count upon starting up, then print the value when it is updated.  

```
    button_count = 0;
    while (mbedClient.is_register_called()) {            
        wait_ms(100);
        if (button_pressed) {
            button_pressed = false;
            printf("button clicked %d times\r\n", button_count);            
        }  
    }
```

#### Update the LWM2M objects.
	See guide at []


## Enabling firmware updates

To enable firmware updates a compatible bootloader needs to be added in the `tools/` folder. This currently only works when building with Mbed CLI.

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

Now an update can be scheduled.

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
