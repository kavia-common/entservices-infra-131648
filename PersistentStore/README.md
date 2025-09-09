# PersistentStore

Key-value storage for plugins requiring persistence.

JSON-RPC
- get(namespace: string, key: string) -> { "value": string|null }
- set(namespace: string, key: string, value: string) -> { "success": bool }
- delete(namespace: string, key: string) -> { "success": bool }
- list(namespace: string, prefix?: string) -> { "keys": string[] }
- clear(namespace: string) -> { "success": bool }

Notes
- Backed by SQLite and stored at PLUGIN_PERSISTENTSTORE_PATH, with legacy migration.
- Max sizes enforced via PLUGIN_PERSISTENTSTORE_MAX* cache variables.
