# NEB-API

A HTTP JSON API for Naemon.

# Build as a developer

Set the `includePath` in the file `.vscode/c_cpp_properties.json` to the directory that contains your `naemon/naemon.h`

`tasks.json` und `launch.json` müssen auch angepasst werden

VS Code Erweiterungen:

- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [autoconf](https://marketplace.visualstudio.com/items?itemName=maelvalais.autoconf) (optional)



# HTTP API

## WebSocket subscription

Naemon is an event-driven system. This means that whenever an internal event, such as sending out a notification, occurs,
Naemon will actively send this event to all subscribed WebSocket clients.

Let's assume you'd like to consume executed checks with `Node-RED`. The WebSocket will push new events to your client and you can process the event data.

Supported events:
- Hostchecks
- TDB

A WebSocket client can be as simple as
```javascript
let sockets = new WebSocket("ws://localhost:8000/websocket");

socket.onopen = function (e) {
    console.log("Connected");
};

socket.onmessage = function (event) {
    // We got data from Naemon 🎉
    console.log(event);
};

socket.onclose = function (event) {
    console.log("Connection closed");
};

socket.onerror = function (error) {
    console.error(error);
};
```

or check out the [websocket.html](https://github.com/nook24/neb-api/blob/main/websocket.html) example client.

## Polling data

Status data records contain detailed information but are most of the time only interesting after a given event has occurred.
For this reason, status data needs to be pulled from the API using a `GET` request whenever needed.

Supported end points:
- Hoststatus
- Servicestatus

<details>
 <summary><code>GET</code> <code><b>/hoststatus</b></code> <code>(gets all hoststatus records)</code></summary>

##### Parameters

> None

##### Responses

> | http code     | content-type                      | response                                                            |
> |---------------|-----------------------------------|---------------------------------------------------------------------|
> | `200`         | `application/json`                | JSON string                                                         |

##### Example cURL

> ```javascript
>  curl -X GET -H http://localhost:8000/hoststatus
> ```

</details>


<details>
  <summary><code>GET</code> <code><b>/hoststatus?hostname={name}</b></code> <code>(gets hoststatus record for one host)</code></summary>

##### Parameters

> | name       |  type      | data type      | description                                          |
> |------------|------------|----------------|------------------------------------------------------|
> | `hostname` |  required  | string         | The hostname you want to query                       |

##### Responses

> | http code     | content-type                      | response                                                            |
> |---------------|-----------------------------------|---------------------------------------------------------------------|
> | `200`         | `application/json`                | JSON string                                                         |
> | `404`         | `application/json`                | `{"error":"Host not found"}`                                        |

##### Example cURL

> ```javascript
>  curl -X GET http://localhost:8000/hoststatus?hostname=hplj2605dn
> ```

</details>
