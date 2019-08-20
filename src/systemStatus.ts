/*
	 Copyright (c) 2019 Norsk rikskringkasting AS (NRK)

	 This file is part of Sofie: The Modern TV News Studio Automation
	 System (Quantel gateway)

	 This program is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License along
	 with this program; if not, write to the Free Software Foundation, Inc.,
	 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// /** Enum for the different status codes in the system  */
// export enum StatusCode {
// 	/** Status unknown */
// 	UNKNOWN = 0,
// 	/** All good and green */
// 	GOOD = 1,
// 	/** Everything is not OK, operation is not affected */
// 	WARNING_MINOR = 2,
// 	/** Everything is not OK, operation might be affected */
// 	WARNING_MAJOR = 3,
// 	/** Operation affected, possible to recover */
// 	BAD = 4,
// 	/** Operation affected, not possible to recover without manual interference */
// 	FATAL = 5
// }

export type ExternalStatus = 'OK' | 'FAIL' | 'WARNING' | 'UNDEFINED'
export interface CheckObj {
	description: string
	status: ExternalStatus
	updated: string // Timestamp, on the form "2017-05-11T18:00:10+02:00" (new Date().toISOString())
	statusMessage?: string
	errors?: Array<CheckError>

	// internal fields (not according to spec):
	// _status: StatusCode
}
export interface CheckError {
	type: string
	time: string // Timestamp, on the form "2017-05-11T18:00:10+02:00" (new Date().toISOString())
	message: string
}
export interface StatusResponseBase {
	status: ExternalStatus
	name: string
	updated: string

	statusMessage?: string // Tekstlig beskrivelse av status. (Eks: OK, Running, Standby, Completed successfully, 2/3 nodes running, Slow response time).
	instanceId?: string
	utilises?: Array<string>
	consumers?: Array<string>
	version?: '3' // version of healthcheck
	appVersion?: string

	checks?: Array<CheckObj>
	components?: Array<Component>

	// internal fields (not according to spec):
	// _internal: {
	// 	// statusCode: StatusCode,
	// 	statusCodeString: string
	// 	messages: Array<string>
	// 	versions: {[component: string]: string}
	// }
	// _status: StatusCode
}
export interface StatusResponse extends StatusResponseBase {
	documentation: string
}
export interface Component extends StatusResponseBase {
	documentation?: string
}
export enum SystemStatusAPI {
	getSystemStatus = 'systemStatus.getSystemStatus'
}
