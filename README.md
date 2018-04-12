== Simple Mbed Cloud Client template application

==== Overview

This is a template application for platform vendors. It demonstrates how to create a simple Mbed Cloud Client application that can connect to Mbed Cloud, register resources and get ready to receive a firmware update.

It's intended to be customized to add platform-specific features (sensors, actuators, etc) and configure the connectivity and storage to work **out-of-the-box**.
The Simple Mbed Cloud Client template application works to work in **developer mode** by default.

==== Setup process

This is a summary of the process for developers to get started and get a device connected to Mbed Cloud.

===== Mbed Online IDE

* Import application into the Online IDE
* Add API key to establish connection to Mbed Cloud
* Install developer certificate
* Compile & program

===== Mbed CLI tools

* Import application in developer's desktop

<<code>>
mbed import https://os.mbed.com/teams/mbed-os-examples/code/mbed-cloud-example
cd mbed-cloud-example
<</code>>

* Download developer certificate from Mbed Cloud
* Compile & program

<<code>>
mbed compile -t <toolchain> -m <target> -c -f
<</code>>

==== Porting to a new platform

The hardware requirements for Mbed OS platforms to support Mbed Cloud Client as shown [here].

However, adding a new platform requires the following:

* Fork the template and create an example application for your platform in https://os.mbed.com
* [Optional] Change connectivity interface. By default uses Ethernet - see main.cpp.
* [Optional] Change the filesystem and/or the block device for storage. By default uses FAT filesystem over SD card. See main.cpp.

==== Known issues

None.

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
