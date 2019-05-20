# Sketch of REST API

## Use cases

* I want to be able to find the clip number (_X_) with title _T_.
* I want to play clip with clipID _X_ on  channel _Y_ of server _Z_, loading from at in point _IN_, seeking to start frame _SEEK_ and playing to out point _Z_.
* I want to start clip playback.
* I want to stop clip playback and not restart.
* I want to pause clip playback and subsequently resume.
* I want to seek (jump) to a specific frame within the clip.
* I want to be able to set a clip to loop playback.

TBD:

* I want to be able to include timecode in the output, starting from X or based on wallclock.
* I want to include captions/subtitles (or another form of AUX/ANC/VANC) data in the output.

## Topology of a Quantel system

Paths are all of the form ...

    /:zoneID/:serverID/:portID

In most cases, `:zoneID` is the default local zone at the ISA communicated with.

## Clip references

_Fragments_ of _clips_ are loaded onto _ports_. This means that the above concept is not that useful to an automation systems that wants a specific video clip to appear on an output _channel_ (SDI port). So we need a way to talk about clips ...

    /:zoneID/:clipID

If it is necessary to search for the clip by, say, title, a query interface can be provided ...

    /:zoneID?title=Game%20of%20Thrones%20Disappoints
