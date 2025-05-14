# window-merger

A lightweight C++ utility for merging or embedding one window inside another on Windows OS. Useful for advanced window management, automation, or integrating applications with custom workflows.

## Features

- Written in C++
- Embeds any window as a child of another window
- Blocks mouse and focus events to the embedded window (optional)
- Dynamically resizes the embedded window to match the parent

## Usage

1. Place `window-merger.exe` in a convenient location on your system.
2. Run from the command line or integrate into scripts as needed. Example usage:

   ```sh
   window-merger.exe "<child_window_title>" "<parent_window_title>"
   ```
   - Replace `<child_window_title>` with the exact title of the window you want to embed.
   - Replace `<parent_window_title>` with the exact title of the window you want to use as the parent.

3. The child window will be embedded into the parent window and will resize automatically.

## Build Instructions

To build from source, use the following command (requires g++ and Windows development libraries):

```sh
 g++ window-merger.cpp -o window-merger.exe -lcomctl32 -static -static-libgcc -static-libstdc++
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
