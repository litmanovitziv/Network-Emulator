Network Emulator
================

Network Emulator emulates tracking of packets over TCP/IP stack. The emulator emulates network conditions using [NetFilter](http://www.netfilter.org/), a packet filtering framework.

As following the main features :

- The emulator application runs on a dedicated machine, which is connected to the network.

- Capturing the IP traffic using IPTables, and enforce the rules as defined by destinied XML file.

- Supporting on following conditions (the policy) :

  | Parameter       | Description                                                  | Valid range/type | Default Value |
  | :-------------- | :----------------------------------------------------------- | ---------------- | ------------- |
  | Packet Loss     | The amount of packets discarded                              | Rational Num (%) | 0             |
  | Packet ordering | How are the packet released out ?A proportion of the packets which change their place on the queue | Rational Num (%) | 0             |
  | Delay on Queue  | The time the packet wait on the Queue before it’s released   | Rational Num     | 0             |
  | Throughput      | The data that pass per second                                | Num (Bits/sec)   | Infinite      |

# Configuration File

The file defines the Link Properties (5-tuple) and its Metrics for each rule.

The emulator supports only XML file as following :

- First line defines XML Specifications.

- Each file defines single policy, which is unique by numeric Identifier (ID attribute).

- Policy configuration may includes several rules. Means, Policy element may includes several Rule elements.

- LinkProperties and Metrics elements validates as follows :

  - Order doesn’t matter attributes and subelements inside 
  - Flow attribute is unique by numeric Identifier.
  - Port attribute is assigned by integer number.
  - Metrics attributes is assigned by numeric values.
  - IPAdress is combined by 4 Integer numbers separated by dot.
  - Chain attribute is assigned either INPUT, OUTPUT or FORWARD.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Policy ID=”1”>
    <Rule>
        <LinkProperties Protocol="tcp" Chain="INPUT" Flow="2">
            <Source Port="" IPAddress=""/>
            <Destination Port="8999" IPAddress="127.0.0.1"/>
        </LinkProperties>
            <Metrics Delay="34.5" Reordering="25"
	            PacketLoss="50" Throughput="1.5"/>
    </Rule>
</Policy>
```

Installation
============

Although NetFilter is embedded inside the linux kernel,
development versions of packages IPTable and libnetfilter_queue are necessary.
Therefore, ensure their installation.

- sudo apt-get install iptables-dev
- sudo apt-get install libnetfilter-queue-dev

These libraries handle the incoming packets.

Additional necessary library is Poco, which builds the lifecycle of Emulator.

Visit in [the website](https://pocoproject.org/download.html) to download and install of Poco.

# Running

The emulator runs under root privileges (*sudo su*)

Firstly, inform the dynamic link loader on Poco library.
Use one of these options of LD_LIBRARY_PATH :  

- sudo export LD_LIBRARY_PATH="<full path of library>:$LD_LIBRARY_PATH"  
- sudo env LD_LIBRARY_PATH="<path of library>:$LD_LIBRARY_PATH"

Finally, run the provided executable file (under ***bin*** directory) with a full path of configuration file. The enclosed file described the hendeled rules and is formatted by XML.

```bash
make POCO_LIBRARY=<full path of POCO library>
LD_LIBRARY_PATH=/usr/local/lib:<full path of POCO library>/lib/Linux/x86_64
export LD_LIBRARY_PATH
./bin/sim <full path of configuration file>
```

