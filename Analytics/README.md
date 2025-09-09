# Analytics

Collects analytics events and forwards them to configured backends.

JSON-RPC
- version() -> { "major": number, "minor": number, "patch": number }
- identify(userId?: string, traits?: object) -> { "success": bool }
- track(event: string, properties?: object, ts?: number) -> { "success": bool }
- screen(name: string, properties?: object, ts?: number) -> { "success": bool }
- flush() -> { "success": bool }
- setBackend(name: string) -> { "success": bool } // dynamically switch backend if available

Events file mapping:
- Optional mapping file path can be supplied via PLUGIN_ANALYTICS_EVENTS_MAP CMake/cache.
- Backends discovered from ${MODULE_NAME}Backends and loaded via LIBLOADER_DFL_DIR.

Notifications:
- onEventQueued { "count": number }
- onFlush { "sent": number, "failed": number }
