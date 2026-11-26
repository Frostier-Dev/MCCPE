# Security Policy

## Supported Versions

Only the latest release of **MCCPE** is actively supported with bug fixes and stability updates. Older versions are archived and will not receive patches.

| Version | Supported          |
| ------- | ------------------ |
| c0.11.x | :white_check_mark: |
| c0.10.x | :x:                |

## Reporting a Vulnerability

Because **MCCPE** runs directly on portable hardware (such as the Nintendo DS/DSi) and reads files from SD cards (`nitrofs` / FAT), we take stability, memory corruption, and file-handling bugs seriously.

If you discover a critical vulnerability, memory exploit, or bug that causes hardware crashes or SD card corruption:

1. **Do not open a public Issue** with exploit details.
2. Report it privately using GitHub's **Private Vulnerability Reporting** feature (under the **Security** tab -> **Advisories** -> **Report a vulnerability**) or contact **@Frostier-Dev** directly.
3. Include your **game version**, **hardware/emulator setup**, and **steps to reproduce** the issue.

### What to Expect

- **Initial Response:** You can expect an acknowledgment within **7 days**.
- **Updates & Resolution:** If the report is accepted, we will work on a fix and keep you updated on the progress before releasing a patched version. If declined, we will explain the reasoning clearly.
