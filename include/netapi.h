#pragma once
/*
netapi.h
BoltOS Usermode Include Declarations
Connections, and the telnet protocol on top of them

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"

/*
 The Bolt API names this header declares, for an application built against
 the SDK: the name an application writes, and the export it calls. See
 boltsdk.h. The system's own modules leave BOLTSDK_APIV undefined and call
 the plain names.
 */
#ifdef BOLTSDK_APIV
#define NetApi_Present      NetApi_Present1
#define NetApi_ParseAddress NetApi_ParseAddress1
#define NetApi_LocalAddress NetApi_LocalAddress1
#define NetApi_Open         NetApi_Open1
#define NetApi_OpenTelnet   NetApi_OpenTelnet1
#define NetApi_State        NetApi_State1
#define NetApi_Send         NetApi_Send1
#define NetApi_Receive      NetApi_Receive1
#define NetApi_Pending      NetApi_Pending1
#define NetApi_ServerEchoes NetApi_ServerEchoes1
#define NetApi_Close        NetApi_Close1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 A connection this machine opened. Zero means none was: no slot free, no
 adapter, or no local port left. The handle carries a generation, so one kept
 across a disconnection does not address whatever took the slot next.
 */
typedef WORDPTR NETAPISOCKET;

/* -- What a connection is doing -- */

/* Not a connection, or one whose handle has gone stale. */
#define NETAPI_IDLE         0

/* The handshake is in progress. Nothing may be sent yet. */
#define NETAPI_CONNECTING   1

/* Open. Anything received is still waiting to be read after this changes, so a
   reader should drain before treating a closed connection as finished. */
#define NETAPI_CONNECTED    2

/* Closed by either end, or by the peer refusing to talk further. */
#define NETAPI_CLOSED       3

/* The open never completed: refused outright, or never answered. */
#define NETAPI_REFUSED      4

/* -- */

/* Whether the machine has a working adapter. Everything below tolerates the
   absence of one and fails rather than waiting. */
UMI_FUNCTION BOOL NetApi_Present(VOID);

/* A dotted quad and nothing else, written into four bytes. False for anything
   that is not one, so a mistyped address is reported rather than half read. */
UMI_FUNCTION BOOL NetApi_ParseAddress(PSTR Text, PWORD8 OutAddress);

/* This machine's own address, for showing what a connection would come from. */
UMI_FUNCTION VOID NetApi_LocalAddress(PWORD8 OutAddress);

/*
 Open a connection. Returns before the handshake finishes, so what comes back
 is in NETAPI_CONNECTING and nothing may be sent until NetApi_State says
 otherwise. Watch for NETAPI_REFUSED, which is where an open that fails ends.
 */
UMI_FUNCTION NETAPISOCKET NetApi_Open(PWORD8 Address, WORD16 Port);

/*
 The same, speaking telnet. Option negotiation is answered as it arrives and
 the commands are taken out of the stream, so what NetApi_Receive gives back is
 the text alone. Nothing else about the connection differs.
 */
UMI_FUNCTION NETAPISOCKET NetApi_OpenTelnet(PWORD8 Address, WORD16 Port);

UMI_FUNCTION WORD32 NetApi_State(NETAPISOCKET Socket);

/* How much was taken or given, which may be less than asked for and may be
   zero. Neither ever waits. */
UMI_FUNCTION WORD32 NetApi_Send(NETAPISOCKET Socket, PVOID Data, WORD32 Length);
UMI_FUNCTION WORD32 NetApi_Receive(NETAPISOCKET Socket, PVOID Buffer, WORD32 Capacity);

/* How much has arrived and not been read. Survives the connection closing, so
   the last thing a peer said before hanging up is not lost. */
UMI_FUNCTION WORD32 NetApi_Pending(NETAPISOCKET Socket);

/*
 Whether the far end agreed to do the echoing, which only a telnet connection
 answers. A terminal draws what is typed only when this is false, or every
 character appears twice.
 */
UMI_FUNCTION BOOL NetApi_ServerEchoes(NETAPISOCKET Socket);

/*
 Finish with a connection. Anything already sent goes out first. The slot is
 released here, so the handle is not valid afterwards even though the exchange
 may take a moment longer to finish.
 */
UMI_FUNCTION VOID NetApi_Close(NETAPISOCKET Socket);

#endif /* BOLTSDK_APIV_10 */
