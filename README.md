# Electrolux MACS component for ESPHome

This is a small project to integrate my dryer to Home Assistant.

**Be very very very careful, because the board is not very much isolated from the mains, so don't even try to open your appliance if you don't have RCD or you are not a specialist!**

### Compatible units

Actually every Electrolux / AEG / Zanussi dryer with EDR10/12/14/16 controller should be compatible, but I have only tested this on mine.

Tested compatible models:

| Brand           | Model             | 
|-----------------|-------------------|
| Electrolux      | EDC2086GDW        |

If you have access and motivation to add new devices, feel free to open an issue with your findings. Please don't make requests without at least serial dumps, I won't buy a dryer just to help you.

To check what infos I've found, look at FINDINGS.md

### Connection to the dryer

You should find the MACS port. I used the one on the front panel as shown below. But I think there should also be one on the main board.

The MACS port is a 4-pin connector on the side of the board.
You should look for a pad that's connected to the ground and one that looks like an inverted (so 0V on idle) UART TX (on a scope). The machine periodically sends a heartbeat, so you'll see it.
Also the logic high level should be at 5V, otherwise that might be a different interface.

**Saying this again. Be very very very careful, because the board is not very much isolated from the mains! Use only isolated instruments for measurements!!!**
