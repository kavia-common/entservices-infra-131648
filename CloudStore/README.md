# CloudStore

Provides secure cloud storage via HTTP or gRPC backend.

JSON-RPC
- put(namespace: string, key: string, value_b64: string) -> { "success": bool }
- get(namespace: string, key: string) -> { "value_b64": string|null }
- delete(namespace: string, key: string) -> { "success": bool }

Configuration
- URI configured via PLUGIN_CLOUDSTORE_URI (e.g., grpc://host:port or https://host/path)
- When grpc is selected, secure_storage.proto stubs will be compiled and linked.

Notes
- May optionally integrate with IARM sysMgr if available.
