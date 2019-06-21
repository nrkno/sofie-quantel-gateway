
# Sofie: The Modern TV News Studio Automation System (Quantel gateway)

This is a part of the [**Sofie** TV News Studio Automation System](https://github.com/nrkno/Sofie-TV-automation/).

## Abstract
This library uses native bindings to bridge the Quantel ISA System CORBA API and a Sofie-specific HTTP REST API, allowing discovery of clips and playback control of Quantel servers.

## Supported devices
* [Grass Valley sQ series media servers](https://www.grassvalley.com/products/sq_1000_servers/) via ISA System.

## Install instructions

This software can be built for Windows and Linux platforms:

* On Linux, an OmniORB4 development package should be available ([Ubuntu example](https://packages.ubuntu.com/search?keywords=omniorb)) for your distribution.

* On Windows, the required DLL and LIB files are included with this package. The included OmniORB was built with Visual Studio 2017 for x64 architecture. The native extension runs with the 64-bit version of Node.js.

This addon has not been built for or tested on MacOS.

### Prerequisites

* This addon has been developed with Node.js v8.1.15 LTS and makes use of the N-API which is not available in earlier versions than 8.
* Install the `node-gyp` build tool globally with `npm install -g node-gyp`.
* Ensure the build system has the [node-gyp prerequisites](https://github.com/nodejs/node-gyp#installation).
* Either install the Quantel ISA dummy server installation or ensure you have access to an installed ISA and sQ servers. (Not ones that are on-air ... yet!)
* Install the [yarn package manager](https://yarnpkg.com/en/docs/install).

On Linux systems, install the OmniORB development package. On Ubuntu, this can be achieved with:

    sudo apt-get install libomniorb4-dev

### Building

Install packages and build the native extension:

    yarn install

Build the typescript interface module:

    yarn build

This package has automated tests that run with [jest](). Test with:

		yarn test

### Running

For development purposes, the HTTP server included with the quantel gateway can be run as follows:

    yarn watch-server

This will start a [nodemon](https://nodemon.io/) watch on the source files and restart the server on any changes.

For production use, a simple server can be run with:

		yarn server

These servers listen on port `3000` by default.

### Experimenting and importing

Experiment from the REPL with:

    const { Quantel } = require('.')

Import into an external project with:

    import { Quantel } from 'tv-automation-quantel-gateway'
    const { Quantel } = require('tv-autonation-quantel-gateway')

See the [walkthrough for how to do playback](./doc/plyout_walkthrough.md) with this module as a Node.js API.

## HTTP API

### Connecting to an ISA

By default, the gateway tries to connect to an ISA at on the localhost.

To connect to an ISA system on a different host, POST to:

    /connect/:address

The `:address` should consist of a DNS name or IP address and, optionally, a port number (defaults to 2096), where the CORBA Interoperable Object Reference (IOR) of the Quantel ISA is advertised. For example:

    /connect/isa.national.ztv.com:3737

A successful request produces a JSON response with the discovered IOR (`isaIOR`) and ISA endpoint address (`href`). Subsequently, the currently configured connection can be queried with a GET request to `/connect`.

### Topology of a Quantel system

Paths are all of the form ...

    /:zoneID/server/:serverID/port/:portID

In most cases and in the current implementation, `:zoneID` is the `default` local zone at the ISA that the gateway connects to.

Types are:

* `:zoneID` - zone number or `default`;
* `:serverID` - integer number or the string name of the server;
* `:portID` - string name of the port.

A GET request to the root path `/` lists all available zones.

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
    "portNames": [],
    "chanPorts": [ "", "", "", "" ]
  }, {
    "type": "Server",
    "ident": 1200,
    "down": false,
    "name": "Dummy 1200",
    "numChannels": 2,
    "pools": [ 12 ],
    "portNames": [],
    "chanPorts": [ "", "" ]
  }, {
    "type": "Server",
    "ident": 1300,
    "down": false,
    "name": "Dummy 1300",
    "numChannels": 3,
    "pools": [ 13 ],
    "portNames": [],
    "chanPorts": [ "", "", "" ]
  } ]
```

### Create a port

To assign a channel to a port, choose a suitable `:portID` and PUT an empty document to ...

    /:zoneID/server/:serverID/port/:portID/channel/:channelID

The `:channelID` is a zero-based channel index that must be less than the number of channels available on the server. If the port name (`:portID`) is already assigned, this is a conflict (`409`). Another kind of error response will result if the channel is already assigned to a port, to prevent you from stealing a port in use by another automation system.

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

Note that this approach does not allow more than one channel to be assigned to a port. As a result, key and fill playback on separate channels is not yet supported by this gateway.

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
  "channels": [ 1 ],
	"outputTime": "00:00:00:00"
}
```

### Clip references

_Fragments_ of _clips_ are loaded onto _ports_. This means that an automation system is responsible of associating a specific video clip to appear on an output _channel_ (SDI ports). Clips are referenced by their integer identifier (`:clipID`) ...

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

(Many fields have been omitted from the example above.)

If it is necessary to search for the clip by, say, title, a query interface is available ...

    /:zoneID/clip?Title=Game%20of%20Thrones%20Disappoints

A GET request to this path should return a JSON array listing documents matching the query and each element  contains a clip identifier. A wildcard character `*` can be used to match zero of more characters.

To refine a search, multiple query parameters can be used. For example, to limit the above search to clips only stored on pool `11`:

    /:zoneID/clip?Title=Game%20of%20Thrones%20Disappoints&PoolID=11

### Formats

To query information about format of a clip, including framerate, compression and dimensions, use the format resource.

    /:zoneID/format/:formatID

The format ID for a clip can be found in the `AudioFormats` and `VideoFormats` properties. Omit the `:formatID` to list all of the formats available in a zone (may take a couple of seconds). For example, to find out the details of video format `90`:

    /default/format/90

This produces (a few fields omitted):

```JSON
{
	"type": "FormatInfo",
	"formatNumber": 90,
	"essenceType": "VideoFragment",
	"frameRate": 25,
	"height": 576,
	"width": 720,
	"samples": 0,
	"formatName": "Legacy 9E Mpeg 40 576i25",
	"layoutName": "720x576i25",
	"compressionName": "Mpeg-2"
}
```

### Loading clips

Clips consist of _fragments_. To play a _clip_, or a sub-clip of a clip, it is necessary to load fragments onto a port. To query all fragments for a clip:

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

Information about the status of the port is returned.

### Port fragment operations

The fragments that are loaded onto a port can be queried with a GET request to:

    /:zoneID/server/:serverID/port/:portID/fragments(?start=:start&finish=:finish)

If the optional `:start` and/or `:finish` query parameters is/are provided, the search will be bounded to include only fragments loaded onto the port between those bounds. The `:start` parameter is inclusive and the `:finish` parameter is one frame beyond the end of the range.

Note that once loaded onto a port/server, the fragments are no longer directly linked with a clip and so the `clipID` property is set to `-1`.

All fragments can be cleared from the port by sending a DELETE request to the port's fragments resource. By default, this will clear (_wipe_ is the Quantel CORBA API term) all fragments from the port. To provide a time range for this operation, use the `start` and `frames` query parameters.

    /:zoneID/server/:serverID/port/:portID/fragments(?start=:start&finish=:finish)

The `start` and `finish` frames, measured relative to the timeline of the port, are optional. When not present, all fragments will be returned. The result is similar to the example above, except that instead of a `clipID` property you have `serverID` and `portName`.

The `:start` parameter is the first frame in the port's timeline to wipe from and `:frames` is the count of frames to wipe. If frames is omitted, all frames from the start offset to the end of the port timeline will be wiped.

### Controlling the port

To control the PORT, POST trigger messages:

    /:zoneID/server/:serverID/port/:portID/trigger/:trigger(?offset=:offset)

The `:trigger` is one of `START`, `STOP` or `JUMP`. Note that `STOP` is equivalent to CasparCG _pause_. To _resume_, use `START`. The optional `:offset` is a frame at which to trigger the action, for example `.../trigger/STOP?offset=345` signals that the playing of the clip should stop at frame 345 on the ports timeline.

#### Hard jump

To force a hard jump to specific point, ideally when stopped and not on air, POST to:

    /:zoneID/server/:serverID/port/:portID/jump?offset=:offset

This will cause an immediate jump and - if the video is playing - it will pause on the jumped-to frame. If the video at the jump-to point is not currently loaded on the port, a short period of black may be played out.

#### Triggered jump

For smoother jumping, each port can have a controlled jump point set and can cause a triggered jump. As long as the jump point is set with a second or more to spare, the material to jump to will be loaded onto the port. This enables smooth playing across the jump, e.g. to achieve a smooth loop. To set the triggered jump point:

1. Prepare the jump by PUTing an empty document to `/:zoneID/server/:serverID/port/:portID/jump?offset=:offset`
2. Wait a second or so and trigger the jump by POSTing an empty document to `/:zoneID/server/:serverID/port/:portID/trigger/JUMP`

It is not possible to query the currently set jump point.

### Errors

All error responses are in JSON format and includes properties for a numerical `status`, an error `message` and, where available, a `stack` trace to help pinpoint where the error occurs. For example:

```JSON
{
	"status": 502,
	"message": "Bad gateway. CORBA subsystem supports a transient connection problem. Check network and ISA server.",
	"stack": "Error: Bad gateway. CORBA subsystem supports a transient connection problem. ..."
}
```

## License

Unless otherwise called out in the header of a file, the files of this project are licensed under the GPL V2 or later, as detailed in the [`LICENSE`](./LICENSE) file. This project links to [omniORB](http://omniorb.sourceforge.net/) that is similarly GPL v2.

Please also note the specific terms of the [`Quentin.idl`](./include/quantel/Quentin.idl) file.
