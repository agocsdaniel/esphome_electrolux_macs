# Findings

I had to reverse engineer most of the infos below, because very little information is available.

### Controller

The dryers are very simple creatures, that don't need much control. 
Basically you need a motor and a heat source and you're done. Of course there are different variants like the ones with heat exchanger, but they don't need very different controlling.
Also Electrolux and its brands (AEG, Zanussi, ...) use very much the same controller boards.

What I have is from an older series that use the EDR1x type controllers. They can be connected to many kinds of user interfaces. What I have is called "Blue Fire Inspiration TC3".

The main controller and the front panel is separated. They are connected with one 4-pin cable that I think is called MACS.

### L1. MACS

Actually this is similar to UART with some quirks.

* One bidirectional data line
* Inverted logic level
* 5V level
* It uses *even* parity, which is unusual
* Usually there are two or more MACS connectors on the boards that are all connected

The MACS port is the interconnect between the boards and also there are the NIU and NIUX Wi-Fi modules that also connects here.
The pinout from the [NIU User Manual](https://fccid.io/2ABHC-5430042/User-Manual/User-manual-2359011.pdf) shows different pinout than what I found. 
Also the Electrolux's debugger tool's [docs](https://sidekick.electrolux.com/sidekickportal/Media/ContentImages/AMITechnicalGuide/AMI%20Technical%20Guide%20v1.1.pdf) have this pinout on page 15.

The suitable connectors are the Stocko MFMP 7238-004-061-960-000-00-G and Lumberg 3517-04-K01 (RAST 2.5, 4-pin). Actually the keying matches with the connector I found and the models are from the NIU docs.

Pinout from the NIU and Sidekick docs:
1 - +12V
  --key--
2 - +5V
3 - Data
4 - GND

Currently I only want to listen on the bus so connect the line to the Rx only.

### L2. Framing

Two types of frames:

1. Message frame
2. ACK frame

Message frame format: `9C <target> <source> <length> <data> <checksum>` 
where target, source, length, checksum are 1 bytes.

In the MACS / DAAS protocols, every panel has its own 1-byte address. In my situation, the main board is `0x22` and the front panel is `0x2A`. Also one can send broadcast message with the target address `0x00`.

ACK frame format: `98 <received data checksum> <checksum>` 
where the received data checksum (I think) nost be calculated from the received data and this frame has its own checksum too. Broadcasted frames don't get ACK replies.

### Messages

I found some meanings of the values by trial and error.

Below all the numbers are purely the data field inside the message frame. All hex values.

```
10 - ???
   00 - ??? (broadcasted at power on)
      13 - ??? (when sent by the front panel)
      17 - ??? (when sent by the controller)
   01 - ??? (sent directly from front panel to controller)
```

```
11 - ???
   00 - (sent by the front panel at power up)
      55 46 43 31 4D 31 30 34 - sw version "UFC1M104" as shown on the board too
                              58 D9 83 4C
   01 - (sent by the controller at power up)
```

```
50 - program change (usually from panel to controller, but can be heard from controller)
   03 - ???
      0x - wool level 0
      1x - wool level 1
      2x - wool level 2
      x0 - dryness level not required; programs: time drying, refresh
      x3 - dryness level 3 - low;      programs: iron dry, easy iron
      x5 - dryness level 5 - medium;   programs: wool, cupboard dry, rapid, delicates, duvet
      x6 - dryness level 6 - high;     programs: cupboard dry +
      x7 - dryness level 7 - extra;    programs: extra dry
         00
            0x - *when any other program
            1x - when rapid
            8x - when cotton cupboard dry
            x2 - delicate mode - wool, delicates, duvet, easy iron programs use it by default; cotton cupboard dry sets it always
               00 - ??? when duvet
               01 - ??? when not duvet
                  00 - time as specified in program
                  xx - custom program time in minutes
                  0A - 10 minutes
                  14 - 20 minutes
                     00 - no delay
                     xx - delay n * 30 min (max 20 hr?)
                     01 - delay 30 min
                     02 - delay 60 min
                     04 - delay 2 hr
                        00 - *when any other programs
                        80 - ??? when time drying, refresh program
                           01 - program  1 cotton extra dry
                           02 - program  2 cotton cupboard dry+
                           03 - program  3 cotton cupboard dry
                           04 - program  4 cotton iron dry
                           05 - program  5 rapid
                           06 - program  6 time drying
                           07 - program  7 refresh
                           08 - program  8 wool
                           09 - program  9 delicates
                           0A - program 10 duvet
                           0B - program 11 easy iron
                           0C - program 12 synthetics iron dry
                           0D - program 13 synthetics cupboard dry
                           0E - program 14 synthetics extra dry
                              00
```

```
51 - change state
   00 
      60 00 80 1B 03 - start drying
      62 - pause drying
      7D 
         04 - sent after power up
         08 - sent when pressing power button
         00 - * sent twice with this but with the same other parameters
            00 - power off
            01 - power on
```


```                                       
52 - appliance state
   03 
      01 00 00 00 00 00 03 00 00 - power on, initializing ???
      01 00 00 00 00 00 01 00 00 - power on, ready, no program running
      02 01 00 00 00 00 01 00 00 - drying
      04 01 00 00 00 00 01 00 00 - drying paused
      0B 00 00 00 00 00 03 00 00 - power off
```                                                    

```
55 - program change response ?
   03 
      00 
         00 
            00 - ??? when time drying 10 min, wool
            02 - ??? when time drying 20 min or more
            03 - ??? when iron dry
            23 - ???
               02 - time drying
               22 - ???
               41 - wool
                  00
                  78
                  DC
                     83 - in wool program
                     8A - no heat maybe? when time drying 10 min
                     8B - ??? when time drying 20 min or more
                        02
                           00
                              00 - program time as specified in program
                              0A - program time 10 minutes
                              xx - program time in minutes
                                 06 - program 6
```

```                               
56 - displayed program time
   03
      00 
         00 
            0A - time 10 minutes
         00 00 - display turn on, empty
         FF FF - display turn off
```

```
57 - ??? (sent from controller to front panel on power up)
   03 00 71 63 63
```

```
5F - heartbeat
   03
```

