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

