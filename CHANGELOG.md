# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

### [1.0.2](https://github.com/nrkno/tv-automation-quantel-gateway/compare/1.0.2-0...1.0.2) (2019-09-18)

### [1.0.2-0](https://github.com/nrkno/tv-automation-quantel-gateway/compare/0.0.1-0...1.0.2-0) (2019-09-18)

### [0.0.1-0](https://github.com/nrkno/tv-automation-quantel-gateway/compare/1.0.0-1...0.0.1-0) (2019-09-18)


### Bug Fixes

* Hot fix to re-add triggers after port reset ([2bc6c41](https://github.com/nrkno/tv-automation-quantel-gateway/commit/2bc6c41))
* hotfix to force port into play mode after reset ([206f623](https://github.com/nrkno/tv-automation-quantel-gateway/commit/206f623))
* memory leak caused by allocating a zone portal per call ([6829a03](https://github.com/nrkno/tv-automation-quantel-gateway/commit/6829a03))

## 1.0.0-1 (2019-08-20)

### Bug Fixes

* add EffectFragment to typings ([c0f90f1](https://github.com/nrkno/tv-automation-quantel-gateway/commit/c0f90f1))
* asynchronous operations from HTTP to CORBA - no queue required ([6c456d3](https://github.com/nrkno/tv-automation-quantel-gateway/commit/6c456d3))
* correcting README for REST path ([44f4b4a](https://github.com/nrkno/tv-automation-quantel-gateway/commit/44f4b4a))
* correctly parse timecode fragments ([ca41490](https://github.com/nrkno/tv-automation-quantel-gateway/commit/ca41490))
* fix fragment typings ([916abf4](https://github.com/nrkno/tv-automation-quantel-gateway/commit/916abf4))
* get play port status BadIdent bug ([a696955](https://github.com/nrkno/tv-automation-quantel-gateway/commit/a696955))
* linting errors and connection promise handling ([f0d2efc](https://github.com/nrkno/tv-automation-quantel-gateway/commit/f0d2efc))
* protect against empty dates returned by clip searches ([4d64d4b](https://github.com/nrkno/tv-automation-quantel-gateway/commit/4d64d4b))
* refactor to async methods completed, with new delete clip feature for testing. ([0d70ad1](https://github.com/nrkno/tv-automation-quantel-gateway/commit/0d70ad1))
* regeression in getPlayPortStatus and check win32 libs are present ([d317e40](https://github.com/nrkno/tv-automation-quantel-gateway/commit/d317e40))
* remove compiler warnings and checked still builds for win32 ([d879e70](https://github.com/nrkno/tv-automation-quantel-gateway/commit/d879e70))
* report an error when trying to release unknown ports ([d1c4171](https://github.com/nrkno/tv-automation-quantel-gateway/commit/d1c4171))
* report playing port status correctly ([369f3d1](https://github.com/nrkno/tv-automation-quantel-gateway/commit/369f3d1))
* reset ORB after connection failure ([5438050](https://github.com/nrkno/tv-automation-quantel-gateway/commit/5438050))
* searching for clips with empty poolIDs ([5f7a7a6](https://github.com/nrkno/tv-automation-quantel-gateway/commit/5f7a7a6))
* typo in link to rest api doc ([5cde13f](https://github.com/nrkno/tv-automation-quantel-gateway/commit/5cde13f))
* wipe fragments defaults to deleting all frmes up to current play head offset on the port ([b55f517](https://github.com/nrkno/tv-automation-quantel-gateway/commit/b55f517))


### Features

* ability to ask for details of a format by format number ([02fee8c](https://github.com/nrkno/tv-automation-quantel-gateway/commit/02fee8c))
* ability to query the properties of a port ([1312909](https://github.com/nrkno/tv-automation-quantel-gateway/commit/1312909))
* automated tests of HTTP API ([f659c46](https://github.com/nrkno/tv-automation-quantel-gateway/commit/f659c46))
* build and push docker images ([25888c5](https://github.com/nrkno/tv-automation-quantel-gateway/commit/25888c5))
* building and deploying with Jenkins ([b89752b](https://github.com/nrkno/tv-automation-quantel-gateway/commit/b89752b))
* building and running on Linux ([8f2e32c](https://github.com/nrkno/tv-automation-quantel-gateway/commit/8f2e32c))
* building and running on Windows x64 platfprm with 64-bit Node.js ([0d2ccc7](https://github.com/nrkno/tv-automation-quantel-gateway/commit/0d2ccc7))
* Clearing and querying the fragments loaded onto a port ([a57ce6d](https://github.com/nrkno/tv-automation-quantel-gateway/commit/a57ce6d))
* cloning, deleting and copy status on clips ([30e3b51](https://github.com/nrkno/tv-automation-quantel-gateway/commit/30e3b51))
* just enough to test a quantel connection ([d25ae6f](https://github.com/nrkno/tv-automation-quantel-gateway/commit/d25ae6f))
* log all http requests ([8e2812e](https://github.com/nrkno/tv-automation-quantel-gateway/commit/8e2812e))
* minimal viable playback control for quantel servers ([75f438b](https://github.com/nrkno/tv-automation-quantel-gateway/commit/75f438b))
* minimum viable playout exposed via HTTP ([de6ace8](https://github.com/nrkno/tv-automation-quantel-gateway/commit/de6ace8))
* Prototype Quantel gateway HTTP interface prototype with error support ([f2ea830](https://github.com/nrkno/tv-automation-quantel-gateway/commit/f2ea830))
* searching for clips using query parameters ([3efd56c](https://github.com/nrkno/tv-automation-quantel-gateway/commit/3efd56c))
* support for specifying more than one ISA system to connect to, round-robin on failure ([0353c37](https://github.com/nrkno/tv-automation-quantel-gateway/commit/0353c37))
* throw connect error (502) before first connect message POSTed ([6817385](https://github.com/nrkno/tv-automation-quantel-gateway/commit/6817385))
