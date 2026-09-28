# Electrolux MACS component for ESPHome

This is a small project to integrate my dryer to Home Assistant.

**Be very very very careful, because the board is not very much isolated from the mains, so don't even try to open your appliance if you don't have RCD or you are not a specialist!**

### Compatible units

Actually most Electrolux / AEG / Zanussi appliances with EDR and EWX controllers should be compatible, but I have only tested this on mine.

Tested compatible models:

| Brand           | Model             | Type                 | Motor to drum ratio |
|-----------------|-------------------|----------------------|---------------------|
| Electrolux      | EDC2086GDW        | Dryer                |                     |
| Electrolux      | EW6SN347SHI       | Washing machine      |                12.2 |
| Electrolux      | EW8W261B          | Washer-dryer         |                     |


If you have access and motivation to add new devices, feel free to open an issue with your findings. Please don't make requests without at least serial dumps, I won't buy a dryer just to help you.

To check what infos I've found, look at FINDINGS.md

For the EW8W261B washer-dryer, set `model: EW8W261B` and `motor_drum_ratio: 10.5` (see "EW8W261B washer-dryer" in `EWX FINDINGS.md`) and see `example_electrolux_ew8w261b_xiao_esp32c5.yaml`.
The pure decoding helpers have a host test: `g++ -std=c++17 -I components tests/decode_test.cpp -o /tmp/decode_test && /tmp/decode_test`, and so does the frame parser: `g++ -std=c++17 -I components tests/framer_test.cpp -o /tmp/framer_test && /tmp/framer_test`.

### Connection to the dryer

You should find the MACS port. I used the one on the front panel as shown below. But I think there should also be one on the main board.

The MACS port is a 4-pin connector on the side of the board.
You should look for a pad that's connected to the ground and one that looks like an inverted (so 0V on idle) UART TX (on a scope). The machine periodically sends a heartbeat, so you'll see it.
Also the logic high level should be at 5V, otherwise that might be a different interface.

To connect an ESP8266 or ESP32 board to the appliance, you need to do level shifting like this: Without it the higher voltage might damage the board and can cause data errors.
```
        MACS (5V)
            |
           [4k7]
            |
            +-----------> ESP_RX (3.3V)
            |
           [10k]
            |
           GND
```

**Saying this again. Be very very very careful, because the board is not very much isolated from the mains! Use only isolated instruments for measurements!!!**
