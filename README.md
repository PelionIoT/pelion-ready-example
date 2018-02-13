# Simple Mbed Cloud Client application

### Overview

This is a reference application for platform vendors. It demonstrates how to create a simple Mbed Cloud Client application that can connect to Mbed Cloud, register resources and get ready to receive a firmware update.

It's intenteded to be forked and customized to add platform specific features (sensors, actuators) and configure the connectivity and storage to work **out-of-the-box**.
The Simple Mbed Cloud Client application is intended to work in **developer mode** only. For production considerations, please read the following [document].

### Setup process

This is a summary of the process for developers to get started and get a device connected to Mbed Cloud.

#### Mbed Online IDE

- Import application into the Online IDE
- Add API key to stablish connection to Mbed Cloud
- Install developer certificate
- Compile & program

#### Mbed CLI tools

- Import application in developer's desktop:

```
mbed import https://github.com/ARMmbed/simple-mbed-cloud-client-example
cd simple-mbed-cloud-client-example
```
- Download developer certificate from Mbed Cloud

- Compile & program

```
mbed compile -t <toolchain> -m <target> -c -f
```

### Porting to a new platform

The hardware requirements for Mbed OS platforms to support Mbed Cloud Client as shown [here].
This template application works with the [FRDM-K64F](https://os.mbed.com/platforms/FRDM-K64F/) platform by default.

However, adding a new platform requires the following:

- Fork the template and create an example application for your platform in https://os.mbed.com
- Modify `mbed_app.json` with corresponding addresses to match your platform memory map.

```
        "K64F": {
            "target.mbed_app_start": "0x00020400",
            "update-client.application-details": "0x00020000",
            "update-client.bootloader-details": "0x172e4"
        }
```
- [Optional] Change connectivity interface. By default uses Ethernet. See lines [xxx].
- [Optional] Change the filesystem and/or the block device for storage. By default uses FAT filesystem over SD card. See lines [xxx].
- Create a [bootloader binary](TBD) according to addresses specified in `mbed_app.json`. The booloader has to use the same storage configuration specified in the application.

### Known issues

None.


