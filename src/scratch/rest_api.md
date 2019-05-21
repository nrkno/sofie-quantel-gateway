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

## Topology of a Quantel system

Paths are all of the form ...

    /:zoneID/server-:serverID/:portID

In most cases, `:zoneID` is the `default` local zone at the ISA communicated with.

Types are:

* `:zoneID` - string name of zone;
* `:serverID` - integer number of the server;
* `:portID` - string name of the port.

A GET requests to the root path `/` lists all available zones.

```JSON
[ {
  "type": "ZonePortal",
  "zoneNumber": 1000,
  "zoneName": "Dummy Zone 1000" } ]
```

A GET request to the zones name (`/:zoneID/`) retrieves details of all the available servers. This includes the assigned port names and number of channels.

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

    /:zoneID/server-:serverID/:portID/channel-:channelID

The `:channelID` is a zero-based channel index that must be less than the number of channels available on the server. If the port name (`:portID`) is already assigned, this is a conflict. Another kind of error response will result if the channel is already assigned to a port (tbc - I hope - otherwise this gets messy).

Note that this approach does not allow more than one channel to be assigned to a port that is supported for key and fill playback on separate channels.

To release the port, send a DELETE request to the complete path for the port.

    /:zoneID/server-:serverID/:portID

A GET request to this path returns the port status.

***TODO: example***

## Clip references

_Fragments_ of _clips_ are loaded onto _ports_. This means that the above concept is not that useful to an automation systems that wants a specific video clip to appear on an output _channel_ (SDI port). So we need a way to reference clips by their integer identifier...

    /:zoneID/clip-:clipID

A GET request to this path should return a selection of metadata known about the clip.

***TODO: example***

If it is necessary to search for the clip by, say, title, a query interface can be provided ...

    /:zoneID?title=Game%20of%20Thrones%20Disappoints

A GET request to this path should return a JSON array listing documents matching the query and each element must contain a _clipID_.

## Loading clips

Clips consist of _fragments_. To play a _clip_, or a sub-clip of a clip, it is necessary to load fragments onto a port. To query all fragments:

    /:zoneID/clip-:clipID/framgments

***TODO: example***

To query fragments for a specific in and out range:

    /:zoneID/clip-:clipID/fragments/:in-:out

The `:in` and `:out` range parameters are measured in frame offset from the start of the clip. To translate to time, the framerate of the clip must be known. For interlaced material, a frame is a pair of fields.

To load the fragments onto a port, POST the fragments to the port reference, adding a frame offset to load onto the port at a position other than zero (unlikely for Sofie), e.g.:

    /:zoneID/server-:serverID/:portID(/offset-:offset?)

If fragments need to be cloned from one server to another, an asynchronous clone process is initiated. Although it is possible for servers to play material while it is cloning, clients should allow a couple of seconds between starting to load fragments and setting the jump (see below) before triggering start to allow the clone to progress around about the play head.

The port status is returned.

## Controlling the port

To control the PORT, POST trigger messages:

    /:zoneID/server-:serverID/:portID/trigger-:trigger(/:offset?)

The `:trigger` is one of `START`, `STOP` or `JUMP`. Note that `STOP` is equivalent to CasparCG _pause_. To _resume_, use `START`. The optional `:offset` is a frame at which to trigger the action, for example `.../trigger-STOP/345` signals that the playing of the clip should stop at frame 345 on the ports timeline.

### Hard jump

To force a hard jump to specific point, ideally when stopped and not on air, POST to:

    /:zoneID/server-:serverID/:portID/jump-:offset

This will cause an immediate jump and - if the video is playing - it will pause on the jumped-to frame. If the video at the jump-to point is not currently loaded on the port, a short period of black may be played out.

### Triggered jump

For smoother jumping, each port can have a controlled jump point set and can cause a triggered jump. As long as the jump point is set with a second or more to spare, the material to jump to will be loaded onto the port. This enables smooth playing across the jump, e.g. to achieve a smooth loop. To set the triggered jump point:

1. Prepare the jump by PUTing an empty document to `/:zoneID/server-:serverID/:portID/jump-:offset`
2. Wait a second or so and trigger the jump by POSTing an empty document to `/:zoneID/server-:serverID/:portID/trigger-JUMP`
