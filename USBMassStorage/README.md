# USBMassStorage

Manages USB mass storage devices: mounting, unmounting and enumeration.

JSON-RPC
- volumes() -> { "items": [ { "mount": string, "fs": string, "label": string, "size": int, "used": int } ] }
- mount(devicePath: string, mountPoint?: string) -> { "success": bool, "mount": string }
- unmount(mountPoint: string, force?: bool) -> { "success": bool }
- eject(devicePathOrMount: string) -> { "success": bool }

Notifications
- onVolumeMounted { "mount": string }
- onVolumeUnmounted { "mount": string }
