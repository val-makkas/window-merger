# absolute-cinema-window-merger

A simple utility for merging or managing windows on Windows OS.

## Features

- Written in C++
- Includes a pre-built executable (`window-merger.exe`)
- Useful for window management automation

## Usage

1. Place `window-merger.exe` on your system.
2. Run from the command line or integrate into scripts as needed.

## Files

- `window-merger.cpp` — Source code
- `window-merger.exe` — Compiled executable
- `window-merger.obj` — Object file

### BUILD
```sh
    cl /EHsc window-merger.cpp user32.lib
```

## License

MIT License