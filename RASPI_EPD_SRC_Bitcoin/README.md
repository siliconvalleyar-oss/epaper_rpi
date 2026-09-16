# RASPI_EPD_SRC_Bitcoin

Bitcoin price ticker for Raspberry Pi + E-Paper display (2.66″ 296×152).

## Features

- Live BTC/USD price from CoinGecko API
- Argentina timezone (ART)
- Date and time display
- Auto-refresh every 60 seconds
- Fast e-paper updates (minimal flash)
- Full-screen refresh on startup, gentle diff-based updates thereafter

## Requirements

- Raspberry Pi (Zero 2 W, 3, 4, or 5)
- 2.66″ e-paper display (296×152, UC8151/SSD1619)
- bcm2835 library
- curl (for API fetching)

## Build & Run

```bash
cd RASPI_EPD_SRC_Bitcoin
make
sudo TZ=America/Argentina/Buenos_Aires ./bin/bitcoin_app
```

Or simply:

```bash
make run
```

(Press Ctrl+C to exit)

## Project Structure

```
RASPI_EPD_SRC_Bitcoin/
├── Makefile
├── src/main.cpp              # Application entry point
├── libs/
│   ├── epaper/               # E-Paper display driver + framebuffer
│   ├── fonts/                # Bitmap fonts (5x8, 7x8, 16x32 numbers, etc.)
│   ├── gpio/                 # GPIO abstraction (bcm2835)
│   └── tyme/                 # Delay utilities
```

## How it works

1. Initializes e-paper display with global update (full refresh)
2. Every 60 seconds: fetches BTC price from CoinGecko via curl
3. Renders price, date, and time using bitmap fonts
4. Updates display with fast (diff-based) update — only changed pixels refresh
5. Sleeps 60 seconds, repeats
