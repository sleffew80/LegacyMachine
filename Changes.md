# LegacyMachine Change Log

## [0.0.7] - 3/31/2026
- Input: Added preliminary core support for keyboard, mouse, and analog joypads.
- Input: Improved input port mapping.
- Code: Heavily refactored, overhauled, and cleaned up a lot of code.
- Initial commit.

## [0.0.6] - 1/16/2026
- Cores: Implemented shutdown request handling.
- Input: Added support for bit masking in joypad input state queries.
- Audio: Improved audio latency handling.

## [0.0.5] - 1/15/2026
- Cores: Implemented disk control API for cores that support it.
- Cores: Added support for saving/loading SRAM/RTC data.
- Input: Added keyboard configuration support via the settings configuration file.

## [0.0.4] - 11/3/2025
- States: Implemented saving/loading of states (for cores that support it).
- Tasks: Implemented libretro SDK's task handler.
- Input: Abstracted keys from SDL keycodes into LMC_KeyCode (for future non-SDL drivers).
- Input: Added Joypad Auto Configuration example for using LMC_SetAutoConfigureJoypadCallback().

## [0.0.3] - 2/4/2025
- Compression: Added support for zip and 7z archived content.

## [0.0.2] - 1/27/2025
- Options: Implemented core options API v1.
- Cores: Support for contentless cores added.

## [0.0.1] - 12/1/2024
- Basic core/content loading.