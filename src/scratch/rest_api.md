# Sketch of REST API

## Use cases

* I want to be able to find the clip number (*X*) with title *T*.
* I want to play clip with clipID *X* on  channel *Y* of server *Z*, loading from at in point *IN*, seeking to start frame _SEEK_ and playing to out point *Z*.
* I want to start clip playback.
* I want to stop clip playback and not restart. (release resources)
* I want to pause clip playback and subsequently resume. (retain resources)
* I want to seek (jump) to a specific frame within the clip.
* (optional) I want to be able to set a clip to loop playback. (Can be done in Sofie automation.)

TBD:

* I want to be able to include timecode in the output, starting from a given value or based on wallclock.
* I want to include captions/subtitles (or another form of AUX/ANC/VANC) data in the output.

## Design goals and assumptions

* The quantel gateway should be stateless. Details of server, port and channel in use and the clip ID will be stored upstream in a quantel state component. This is similar to the channel / layer / clip name being managed in the CasparCG state library.

 Thought: This does limit potential abstractions as control is via server and port identifiers, not clip identifier. Otherwise the quantel gateway needs to store and maintain clip-to-server+port mappings.

* The output of the Quantel is via a vision mixer under automation control and so capabilities of the sQ server to do transitions and other effects are not used.

## Connecting to an ISA

By default, the gateway tries to connect to an ISA at on the localhost.

To connect to an ISA system on a different host, POST to:

    /connect/:address

The `:address` should consist of a DNS name or IP address and, optionally, a port number (defaults to 2096), where the CORBA Interoperable Object Reference (IOR) of the Quantel ISA is advertised. For example:

    /connect/isa.national.ztv.com:3737

A successful request produces the IOR reference as a plain text response.

## Topology of a Quantel system

Paths are all of the form ...

    /:zoneID/server/:serverID/port/:portID

In most cases and in the current implementation, `:zoneID` is the `default` local zone at the ISA communicated with.

Types are:

* `:zoneID` - zone number or `default`;
* `:serverID` - integer number of the server;
* `:portID` - string name of the port.

A GET requests to the root path `/` lists all available zones.

```JSON
[ {
  "type": "ZonePortal",
  "zoneNumber": 1000,
  "zoneName": "Dummy Zone 1000",
  "isRemote": false } ]
```

This first element will always be the local / `default` zone.

A GET request to the zones name (`/:zoneID/server/`) retrieves details of all the available servers. This includes the assigned port names and number of channels.

```JSON
[ { "type": "Server",
    "ident": 1100,
    "down": false,
    "name": "Dummy 1100",
    "numChannels": 4,
    "pools": [ 11 ],
    "portNames": []
  }, {
    "type": "Server",
    "ident": 1200,
    "down": false,
    "name": "Dummy 1200",
    "numChannels": 2,
    "pools": [ 12 ],
    "portNames": []
  }, {
    "type": "Server",
    "ident": 1300,
    "down": false,
    "name": "Dummy 1300",
    "numChannels": 3,
    "pools": [ 13 ],
    "portNames": []
  } ]
```

## Create a port

To assign a channel to a port, choose a suitable `:portID` and PUT an empty document to ...

    /:zoneID/server/:serverID/port/:portID/channel/:channelID

The `:channelID` is a zero-based channel index that must be less than the number of channels available on the server. If the port name (`:portID`) is already assigned, this is a conflict. Another kind of error response will result if the channel is already assigned to a port (TODO - I hope - otherwise this gets messy).

On success, the response is:

```JSON
{
  "type": "PortInfo",
  "serverID": 1100,
  "channelNo": 1,
  "audioOnly": false,
  "portName": "fred",
  "portID": 2,
  "assigned": true
}
```

Note that this approach does not allow more than one channel to be assigned to a port that is supported for key and fill playback on separate channels.

To release the port, send a DELETE request to the complete path for the port.

    /:zoneID/server/:serverID/port/:portID

A GET request to this path returns the port status.

```JSON
{
  "type": "PortStatus",
  "serverID": 1100,
  "portName": "fred",
  "refTime": "14:47:31:00",
  "portTime": "10:00:15:03",
  "portID": 2,
  "speed": 1,
  "offset": 0,
  "status": "unknown",
  "endOfData": 0,
  "framesUnused": 0,
  "channels": [ 1 ]
}
```

## Clip references

_Fragments_ of _clips_ are loaded onto _ports_. This means that the above concept is not that useful to an automation systems that wants a specific video clip to appear on an output _channel_ (SDI port). So we need a way to reference clips by their integer identifier...

    /:zoneID/clip/:clipID

A GET request to this path should return a selection of metadata known about the clip.

```JSON
{
  "type": "ClipData",
  "ClipID": 3,
  "Completed": "2019-05-13T17:46:59.000Z",
  "Created": "2019-05-13T17:46:59.000Z",
  "Description": "Example clip on a Quantel ISA.",
  "Frames": "1000",
  "NumAudTracks": 1,
  "NumVidTracks": 1,
  "PoolID": 11,
  "PublishedBy": "",
  "Register": "0",
  "Tape": "",
  "Template": 0,
  "Title": "Example 3",
  "AudioFormats": "73",
  "VideoFormats": "90",
  "ClipGUID": "0e2be6f2478649df9fcf56c0d0ecc040",
  "PublishCompleted": "2019-05-13T17:46:59.000Z"
}
```

If it is necessary to search for the clip by, say, title, a query interface can be provided ...

    /:zoneID/clip?Title=Game%20of%20Thrones%20Disappoints

A GET request to this path should return a JSON array listing documents matching the query and each element  contains a _clipID_. A wildcard character `*` can be used to match zero of more characters.

## Loading clips

Clips consist of _fragments_. To play a _clip_, or a sub-clip of a clip, it is necessary to load fragments onto a port. To query all fragments:

    /:zoneID/clip/:clipID/fragments

```JSON
{
  "type": "ServerFramgments",
  "clipID": 2,
  "fragments": [ {
    "type": "VideoFragment",
    "trackNum": 0,
    "start": 0,
    "finish": 1000,
    "rushID": "344aed5ed1204908a54302de951eecb7",
    "format": 90,
    "poolID": 11,
    "poolFrame": 5,
    "skew": 0,
    "rushFrame": 0
  }, {
    "type": "AudioFragment",
    "trackNum": 0,
    "start": 0,
    "finish": 1000,
    "rushID": "520c2157fc66443b9e2fc580cb2cf789",
    "format": 73,
    "poolID": 11,
    "poolFrame": 8960,
    "skew": 0,
    "rushFrame": 0 } ] }
```

To query fragments for a specific in and out range:

    /:zoneID/clip/:clipID/fragments/:in-:out

The `:in` and `:out` range parameters are measured in frame offsets from the start of the clip. To translate to time, the framerate of the clip must be known. For interlaced material, a frame is a pair of fields.

To load the fragments onto a port, POST the fragments to the port reference, adding a frame offset to load onto the port at a position other than zero (unlikely for Sofie), e.g.:

    /:zoneID/server/:serverID/port/:portID/fragments(?offset=:offset)

If fragments need to be cloned from one server to another, an asynchronous clone process is initiated. Although it is possible for servers to play material while it is cloning, clients should allow a couple of seconds between starting to load fragments and setting the jump (see below) before triggering start to allow the clone to progress around about the play head.

Information about the status of the port is returned.

## Controlling the port

To control the PORT, POST trigger messages:

    /:zoneID/server/:serverID/port/:portID/trigger/:trigger(?offset=:offset)

The `:trigger` is one of `START`, `STOP` or `JUMP`. Note that `STOP` is equivalent to CasparCG _pause_. To _resume_, use `START`. The optional `:offset` is a frame at which to trigger the action, for example `.../trigger/STOP/345` signals that the playing of the clip should stop at frame 345 on the ports timeline.

### Hard jump

To force a hard jump to specific point, ideally when stopped and not on air, POST to:

    /:zoneID/server/:serverID/port/:portID/jump?offset=:offset

This will cause an immediate jump and - if the video is playing - it will pause on the jumped-to frame. If the video at the jump-to point is not currently loaded on the port, a short period of black may be played out.

### Triggered jump

For smoother jumping, each port can have a controlled jump point set and can cause a triggered jump. As long as the jump point is set with a second or more to spare, the material to jump to will be loaded onto the port. This enables smooth playing across the jump, e.g. to achieve a smooth loop. To set the triggered jump point:

1. Prepare the jump by PUTing an empty document to `/:zoneID/server/:serverID/port/:portID/jump?offset=:offset`
2. Wait a second or so and trigger the jump by POSTing an empty document to `/:zoneID/server/:serverID/port/:portID/trigger/JUMP`

It is not possible to query the currently set jump point.
