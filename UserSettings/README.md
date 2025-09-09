# UserSettings

Manages user preferences and accessibility settings.

JSON-RPC
- get(key: string) -> { "value": any|null }
- set(key: string, value: any) -> { "success": bool }
- remove(key: string) -> { "success": bool }
- list(prefix?: string) -> { "keys": string[] }
- export() -> { "document": object }  // export full user settings
- import(document: object, mode?: "merge"|"replace" = "merge") -> { "success": bool }

Accessibility-related keys (non-exhaustive):
- accessibility.voiceGuideEnabled
- accessibility.voiceGuideSpeed
- accessibility.highContrast
- captions.enabled
- captions.language
- captions.style

Notes:
- Backed by PersistentStore for durability.
- Emits notifications on change: onSettingsChanged { "keys": string[] }
