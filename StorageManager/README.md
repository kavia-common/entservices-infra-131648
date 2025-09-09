-----------------
# StorageManager

This plugin provides storage management for:
- Local (device filesystem paths)
- Persistent store access (delegates to PersistentStore)
- Shared storage (delegates to SharedStorage)
- Cloud storage proxying (delegates to CloudStore)
- gRPC-based secure storage (delegates to CloudStore gRPC when available)

JSON-RPC summary:
- version() -> { "major": int, "minor": int, "patch": int }
- enumerate(path: string) -> { "entries": [ { "name": string, "type": "file"|"dir", "size": int } ] }
- info(path: string) -> { "exists": bool, "type": "file"|"dir"|"unknown", "size": int }
- mkdir(path: string, recursive?: bool) -> { "success": bool }
- remove(path: string, recursive?: bool) -> { "success": bool }
- move(src: string, dst: string) -> { "success": bool }
- copy(src: string, dst: string) -> { "success": bool }
- quota(namespace?: string) -> { "limit": int, "used": int }
- setnamespace(ns: string) -> { "success": bool }  // for Persistent/Shared namespace operations
- cloud.put(key: string, data_b64: string) -> { "success": bool }
- cloud.get(key: string) -> { "data_b64": string|null }
- cloud.delete(key: string) -> { "success": bool }

Notes:
- Actual authorization and sandboxing depend on WPEFramework security context.
- Large object transfers should use chunking or file descriptors; the minimal interface here is illustrative.
- The implementation composes helpers and underlying plugins via helpers/ and RequestHandler.

## Versions
`org.rdk.StorageManager.1`

## Methods:
```
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method": "org.rdk.StorageManager.clear", "params": {"appId": "com.sky.testapp"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json" --request POST --data '{"jsonrpc":"2.0", "id":3, "method": "org.rdk.StorageManager.clearAll", "params": {"exemptionAppIds": "com.sky.testapp"}}' http://127.0.0.1:9998/jsonrpc
```

## Responses
```
clear:
{"jsonrpc":"2.0","id":3,"result":""}

clearAll:
{"jsonrpc":"2.0","id":3,"result":""}

```

## Events
```
none
```
