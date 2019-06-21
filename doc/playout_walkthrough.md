## Playout walkthrough - Node API

Follow the steps in this document to run a Quantel playout using the Node.js API directly, rather than via the HTTP API. This document assumes that the Quantel gateway has been imported as module `Quantel`, e.g.:

      const { Quantel } = require('.')
			import { Quantel } from 'tv-automation-quantel-gateway'

### Server Connection

If the ISA is running on the localhost on the default port, each of the calls will establish connection automatically. Otherwise, it is necessary to estabinsh the ISA IOR reference with a call to:

```Javascript
await Quantel.getISAReference('http://isa.adresss.or.name:port/')
```

This requests the CORBA reference to the ISA over HTTP.

Test the CORBA connection with:

```Javascript
Quantel.testConnection().then(console.log)
// 'PONG!' on success
```

### Playout walkthrough

The following example assumes you are using the Node.js REPL. As all methods return promises, in a project it is recommended that `async`/`await` are used.

#### Record a clip

This following steps assume that you have already stored clips onto servers. With the dummy servers, the _Controller UI_ can be used to do this. Select the _System_ tab, click on a server (e.g. _1100_), double click on a channel (e.g. _S1100C2_), put the port into record mode, select number of frames to record (default _1000_), click _Initial Frames_ then click _Start_. Once recording is complete, click _Save_. Once finished, click _Release_ to release the port.

#### Create a playout port

An single ISA software system controls a _zone_. To find out details of the current zone:

```Javascript
Quantel.getZoneInfo().then(console.log)
```

```Javascript
{ type: 'ZonePortal',
zoneNumber: 1000,
zoneName: 'Dummy Zone 1000' }
```

Each zone has some hardware _servers_. Each _server_ has a number of _channels_ which, in practice, map to the SDI ports on the device. Servers also have _pools_ of disk, a sort of mount point for a RAID of hard disks. To find out about the servers connected to an ISA, call `getServers`.

```Javascript
Quantel.getServers().then(console.log)
```

```Javascript
[ { type: 'Server',
ident: 1100,
down: false,
name: 'Dummy 1100',
numChannels: 4,
pools: [ 11 ],
portNames: [ 'solitary' ] },
{ type: 'Server',
ident: 1200,
down: false,
name: 'Dummy 1200',
numChannels: 2,
pools: [ 12 ],
portNames: [] },
{ type: 'Server',
ident: 1300,
down: false,
name: 'Dummy 1300',
numChannels: 3,
pools: [ 13 ],
portNames: [] } ]
```

To be able to play out material on a channel of a specific server, it is necessary to create a logical playout port attached to that channel and configured for playout.

```Javascript
Quantel.createPlayPort({ serverID: 1100, portName: 'nrk8', channelNo: 1}).then(console.log)
```

```Javascript
{ type: 'PortInfo',
serverID: 1100,
channelNo: 1,
audioOnly: false,
portName: 'nrk8',
portID: 1,
assigned: true }
```

Note that the `assigned` flag is `true` only if the port is newly created, otherwise it is set to `false`. Also note that it is very easy to steal someone else's port!

#### Find fragments

To be able to play a clip, the fragments that make up that clip must be loaded onto a port. The details of what fragments are required and where they are stored are available in the database attached to ISA. To query the fragments for a clip with ID `2`:

```Javascript
let frags
Quantel.getAllFragments({ clipID: 2 }).then(f => { frags = f; console.log(f); })
```

```Javascript
{ type: 'ServerFramgments',
clipID: 2,
fragments:
[ { type: 'VideoFragment',
	 trackNum: 0,
	 start: 0,
	 finish: 1000,
	 rushID: '344aed5ed1204908a54302de951eecb7',
	 format: 90,
	 poolID: 11,
	 poolFrame: 5,
	 skew: 0,
	 rushFrame: 0 },
 { type: 'EffectFragment', /* ... */ },
 { type: 'EffectFragment', /* ... */ },
 { type: 'AudioFragment',
	 trackNum: 0,
	 start: 0,
	 finish: 1000,
	 rushID: '520c2157fc66443b9e2fc580cb2cf789',
	 format: 73,
	 poolID: 11,
	 poolFrame: 8960,
	 skew: 0,
	 rushFrame: 0 } ] }
```

#### Load fragments onto port

To load fragments onto a port, the server with the port must also have the disk storage `pool` where the fragments are stored attached. Otherwise a clone will need to be initiated (to follow). Loading the clips at offset 0 in the timeline of the port:

```Javascript
Quantel.loadPlayPort({ serverID: 1100, portName: 'nrk8', offset: 0, fragments: frags })
```

#### Check port status

At any time after the creation of a port and before its release, it is possible to check its status:

```Javascript
Quantel.getPlayPortStatus({ serverID: 1100, portName: 'nrk8'}).then(console.log)
```

```Javascript
{ type: 'PortStatus',
serverID: 1100,
portName: 'nrk8',
portID: 1,
speed: 1,
offset: 0,
status: 'readyToPlay',
endOfData: 1000,
framesUnused: 0 }
```

The status message includes the current position of the play head (`offset`) measured in frames and the speed that the media is play at (float value, `0.5` for half speed, `-1.0` for reverse etc..). (To follow: setting speed?)

#### Control playout

To start and stop playback, use triggers. For example:

```Javascript
Quantel.trigger({ serverID: 1100, portName: 'nrk8', trigger: Quantel.Trigger.START })
Quantel.trigger({ serverID: 1100, portName: 'nrk8', trigger: Quantel.Trigger.STOP })
```

It is also possible to set an offset at which play should stop, e.g.:

```Javascript
Quantel.trigger({ serverID: 1100, portName: 'nrk8',
trigger: Quantel.Trigger.STOP, offset: 300 })
```

To jump to a specific frame, ideally when stopped and not when playing out live, use `jump`:

```Javascript
Quantel.jump({ serverID: 1100, portName: 'nrk8', offset: 123 })
```

Note that if the port is playing when asked to jump, it will jump and stop.

To follow: get `setJump` to work.

#### Release the port

Once playout is finished, release the port and its resources with:

```Javascript
Quantel.releasePort({ serverID: 1100, portName: 'nrk8'})
```
