# USBDevice

Monitors USB device connect/disconnect and exposes device information.

JSON-RPC
- devices() -> { "items": [ { "path": string, "vendorId": string, "productId": string, "class": string } ] }
- info(path: string) -> { "details": object|null }
- subscribe() -> { "success": bool }
- unsubscribe() -> { "success": bool }

Notifications
- onDeviceAdded { "device": object }
- onDeviceRemoved { "device": object }

Notes
- Uses libusb when available; falls back to test stubs under test flags.
