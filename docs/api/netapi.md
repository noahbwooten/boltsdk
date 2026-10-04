# netapi.h — connections

`#include <netapi.h>` · exported by **netapi.dll** · link **`netapi.lib`**

TCP connections this machine opens, and telnet over them. Nothing here ever
waits: an open returns at once and is watched until it connects, and a send or
a receive moves what it can and answers how much. Everything tolerates a
machine with no network adapter and fails rather than waiting for one.

A connection is a `NETAPISOCKET`, which is zero when none could be opened. It
carries a generation, so one kept past a disconnection does not reach whatever
took its slot next.

```c
#define NETAPI_IDLE        0   /* not a connection, or a stale one */
#define NETAPI_CONNECTING  1   /* the handshake is under way */
#define NETAPI_CONNECTED   2   /* open */
#define NETAPI_CLOSED      3   /* closed by either end */
#define NETAPI_REFUSED     4   /* the open never completed */
```

```c
BYTE Address[4];
NETAPISOCKET Socket = 0;

if (NetApi_ParseAddress("10.0.2.2", Address))
    Socket = NetApi_Open(Address, 7);

/* ... then, from a timer: */
switch (NetApi_State(Socket)) {
case NETAPI_CONNECTED:
    NetApi_Send(Socket, "hello\r\n", 7);
    Got = NetApi_Receive(Socket, Buffer, sizeof(Buffer));
    break;
case NETAPI_CLOSED:
case NETAPI_REFUSED:
    NetApi_Close(Socket);
    Socket = 0;
    break;
}
```

Under the SDK's emulator the host machine is `10.0.2.2`.

---

### NetApi_Present

```c
BOOL NetApi_Present(VOID);
```

netapi.dll · export `NetApi_Present1` · SDK 10

Whether the machine has a working adapter.

### NetApi_ParseAddress

```c
BOOL NetApi_ParseAddress(PSTR Text, PWORD8 OutAddress);
```

netapi.dll · export `NetApi_ParseAddress1` · SDK 10

A dotted quad, such as `"192.168.1.20"`, into four bytes. `FALSE` for anything
else, so a mistyped address is reported rather than half read. There are no
host names: SDK 10 has no name lookup.

### NetApi_LocalAddress

```c
VOID NetApi_LocalAddress(PWORD8 OutAddress);
```

netapi.dll · export `NetApi_LocalAddress1` · SDK 10

This machine's own address, into four bytes.

### NetApi_Open

```c
NETAPISOCKET NetApi_Open(PWORD8 Address, WORD16 Port);
```

netapi.dll · export `NetApi_Open1` · SDK 10

Opens a connection to a port at an address. Returns before the handshake
finishes, so the connection starts in `NETAPI_CONNECTING` and nothing may be
sent until [`NetApi_State`](#netapi_state) says `NETAPI_CONNECTED`. An open
that fails ends in `NETAPI_REFUSED`.

- **Returns** the connection, or 0 when there is no adapter, no free slot or
  no local port left.

### NetApi_OpenTelnet

```c
NETAPISOCKET NetApi_OpenTelnet(PWORD8 Address, WORD16 Port);
```

netapi.dll · export `NetApi_OpenTelnet1` · SDK 10

The same, speaking telnet: option negotiation is answered as it arrives and
taken out of the stream, so what [`NetApi_Receive`](#netapi_receive) gives
back is the text alone.

### NetApi_State

```c
WORD32 NetApi_State(NETAPISOCKET Socket);
```

netapi.dll · export `NetApi_State1` · SDK 10

What a connection is doing: a `NETAPI_*` value.

### NetApi_Send

```c
WORD32 NetApi_Send(NETAPISOCKET Socket, PVOID Data, WORD32 Length);
```

netapi.dll · export `NetApi_Send1` · SDK 10

Queues data to go out. **Returns** how much was taken, which may be less than
`Length` and may be zero; send the rest later.

### NetApi_Receive

```c
WORD32 NetApi_Receive(NETAPISOCKET Socket, PVOID Buffer, WORD32 Capacity);
```

netapi.dll · export `NetApi_Receive1` · SDK 10

Takes what has arrived, up to `Capacity` bytes. **Returns** how much, which may
be zero.

### NetApi_Pending

```c
WORD32 NetApi_Pending(NETAPISOCKET Socket);
```

netapi.dll · export `NetApi_Pending1` · SDK 10

How much has arrived and not been read. It survives the connection closing, so
drain a closed connection before treating it as finished: the last thing a
peer said before hanging up is still there.

### NetApi_ServerEchoes

```c
BOOL NetApi_ServerEchoes(NETAPISOCKET Socket);
```

netapi.dll · export `NetApi_ServerEchoes1` · SDK 10

For a telnet connection, whether the far end agreed to echo what is typed. A
terminal draws what is typed only when this is `FALSE`, or every character
appears twice.

### NetApi_Close

```c
VOID NetApi_Close(NETAPISOCKET Socket);
```

netapi.dll · export `NetApi_Close1` · SDK 10

Finishes with a connection. Anything already sent goes out first. The handle
is not valid afterwards, though the exchange may take a moment longer to end.
