/*
 * Copyright 2020 u-blox Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _U_LOCATION_H_
#define _U_LOCATION_H_

/* No #includes allowed here */

/** @file
 * @brief This header file defines the location API, which is designed
 * to determine location using any u-blox module and potentially a
 * cloud service.  These functions are thread-safe.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

#ifndef U_LOCATION_TIMEOUT_SECONDS
/** The timeout for location establishment in seconds.
 */
# define U_LOCATION_TIMEOUT_SECONDS 240
#endif

#ifndef U_LOCATION_ASSIST_DEFAULTS
/** Default values for uLocationAssist_t.
 */
# define U_LOCATION_ASSIST_DEFAULTS {-1, -1, -1}
#endif

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

/** The possible types of location fix.  Note that not all modules
 * support all types.
 */
typedef enum {
    U_LOCATION_TYPE_NONE,
    U_LOCATION_TYPE_GNSS, /**< supported on GNSS network instances and
                               also on cellular network instances if a
                               GNSS module is attached to the cellular
                               module. */
    U_LOCATION_TYPE_CLOUD_CELL_LOCATE, /**< supported on cellular network
                                            instances only. */
    U_LOCATION_TYPE_CLOUD_GOOGLE, /**< supported on short-range network instances only. */
    U_LOCATION_TYPE_CLOUD_SKYHOOK, /**< supported on short-range network instances only. */
    U_LOCATION_TYPE_CLOUD_HERE,  /**< supported on short-range network instances only. */
    U_LOCATION_TYPE_MAX_NUM
} uLocationType_t;

/** Definition of additional information where a variety of location
 * establishment mechanisms can be employed.
 * Note: if this is updated then U_LOCATION_ASSIST_DEFAULTS should be
 * updated also.
 */
typedef struct {
    int32_t desiredAccuracyMillimetres; /**< the desired location accuracy
                                             in millimetres; may be ignored,
                                             set to -1 for none-specified. */
    int32_t desiredTimeoutSeconds; /**< the desired location establishment
                                        time in seconds; may be ignored,
                                        set to -1 for none-specified. Note
                                        that this is NOT a hard timeout,
                                        simply an indication to the underlying
                                        system as to how urgently location
                                        establishment is required. */
    int32_t networkHandleAssist; /**< the network handle to use for
                                      assistance information, if
                                      available; when using Cell Locate
                                      this may be set to the network handle
                                      of an attached short-range chip (currently
                                      a short-range Wifi chip only) from
                                      which information can obtained to give
                                      a more accurate location fix.  It
                                      is not [currently] otherwise used and
                                      may be left at the default of -1. */
} uLocationAssist_t;

/** Definition of a location.
 */
typedef struct {
    uLocationType_t type; /**< the location mechanism that was used. */
    int32_t latitudeX1e7; /**< latitude in ten millionths of a degree. */
    int32_t longitudeX1e7; /**< longitude in ten millionths of a degree. */
    int32_t altitudeMillimetres; /**< altitude in millimetres; if the
                                      altitude is unknown -1 will be
                                      returned. */
    int32_t radiusMillimetres; /**< radius of location in millimetres;
                                    if the radius is unknown -1 will be
                                    returned. */
    int32_t speedMillimetresPerSecond; /**< the speed (in millimetres
                                            per second); if the speed
                                            is unknown -1 will be
                                            returned. */
    int32_t svs;                       /**< the number of space vehicles
                                            used in establishing the
                                            location. If the number of
                                            space vehicles is unknown or
                                            irrelevant -1 will be
                                            returned. */
    int32_t tickTimeMs; /**< the tick time at which the location fix
                             was made. */
} uLocation_t;

/** The possible states a location establishment
 * attempt can be in.
 */
typedef enum {
    U_LOCATION_STATUS_UNKNOWN,
    U_LOCATION_STATUS_CELLULAR_SCAN_START,
    U_LOCATION_STATUS_CELLULAR_SCAN_END,
    U_LOCATION_STATUS_REQUESTING_DATA_FROM_SERVER,
    U_LOCATION_STATUS_RECEIVING_DATA_FROM_SERVER,
    U_LOCATION_STATUS_SENDING_FEEDBACK_TO_SERVER,
    U_LOCATION_STATUS_WRONG_URL,
    U_LOCATION_STATUS_HTTP_ERROR,
    U_LOCATION_STATUS_CREATE_SOCKET_ERROR,
    U_LOCATION_STATUS_WRITE_TO_SOCKET_ERROR,
    U_LOCATION_STATUS_READ_FROM_SOCKET_ERROR,
    U_LOCATION_STATUS_CONNECTION_OR_DNS_ERROR,
    U_LOCATION_STATUS_BAD_AUTHENTICATION_TOKEN,
    U_LOCATION_STATUS_GENERIC_ERROR,
    U_LOCATION_STATUS_USER_TERMINATED,
    U_LOCATION_STATUS_NO_DATA_FROM_SERVER,
    U_LOCATION_STATUS_UNKNOWN_COMMS_ERROR,
    U_LOCATION_STATUS_MAX_NUM
} uLocationStatus_t;

/* ----------------------------------------------------------------
 * FUNCTIONS
 * -------------------------------------------------------------- */

/** Get the current location, returning on success or when
 * pKeepGoingCallback returns false.
 *
 * @param networkHandle           the handle of the nework instance
 *                                to use.
 * @param type                    the type of location fix to perform;
 *                                how this can be used depends upon the
 *                                type of networkHandle:
 *                                - GNSS:     ignored, U_LOCATION_TYPE_GNSS
 *                                            will always be used,
 *                                - cellular: U_LOCATION_TYPE_GNSS may be
 *                                            used if a GNSS chip is
 *                                            attached to the cellular
 *                                            module, else
 *                                            U_LOCATION_TYPE_CLOUD_CELL_LOCATE
 *                                            may be used and
 *                                            pAuthenticationTokenStr must
 *                                            be populated with a valid
 *                                            Cell Locate authentication
 *                                            token. In the latter case, if
 *                                            pLocationAssist includes the
 *                                            handle of a Wifi instance, it
 *                                            will be used to provide improved
 *                                            location accuracy.
 *                                - Wifi:     only U_LOCATION_TYPE_CLOUD_GOOGLE,
 *                                            U_LOCATION_TYPE_CLOUD_SKYHOOK and
 *                                            U_LOCATION_TYPE_CLOUD_HERE are
 *                                            supported; pAuthenticationTokenStr
 *                                            must be populated with a valid
 *                                            authentication token for the
 *                                            chosen service.
 *                                - BLE:      no form of BLE location is currently
 *                                            supported.
 * @param pLocationAssist         additional information for the location
 *                                establishment process, useful where several
 *                                different location establishment strategies
 *                                are possible; currently only used with Cell
 *                                Locate. May be NULL, in which case the values of
 *                                U_LOCATION_ASSIST_DEFAULTS will be assumed.
 * @param pAuthenticationTokenStr the null-terminated authentication token,
 *                                must be non-NULL if a cloud service is being
 *                                used to establish location.
 * @param pLocation               a place to put the location; may be NULL.
 * @param pKeepGoingCallback      a callback function that governs how long
 *                                location establishment is allowed to take.
 *                                This function is called while waiting for
 *                                location establishment to complete; location
 *                                establishment will only continue while
 *                                it returns true.  This allows the caller
 *                                to terminate the locating process at
 *                                their convenience.  This function may
 *                                also be used to feed any watchdog
 *                                timer that might be running.  May be NULL,
 *                                in which case location establishment will
 *                                stop when U_LOCATION_TIMEOUT_SECONDS have
 *                                elapsed.  The single int32_t parameter is
 *                                the network handle.
 * @return                        zero on success or negative error code
 *                                on failure.
 */
int32_t uLocationGet(int32_t networkHandle, uLocationType_t type,
                     uLocationAssist_t *pLocationAssist,
                     const char *pAuthenticationTokenStr,
                     uLocation_t *pLocation,
                     bool (*pKeepGoingCallback) (int32_t));

/** Get the current location, non-blocking version.
 *
 * @param networkHandle           the handle of the network instance to use.
 * @param type                    the type of location fix to perform; the
 *                                comments concerning which types can be
 *                                used for the uLocationGet() API apply.
 * @param pLocationAssist         additional information for the location
 *                                establishment process, useful where several
 *                                different location establishment strategies
 *                                are possible (e.g. when using Cell Locate);
 *                                may be NULL in which case
 *                                U_LOCATION_ASSIST_DEFAULTS will be assumed.
 * @param pAuthenticationTokenStr the null-terminated authentication token,
 *                                must be non-NULL if a cloud service is being
 *                                used to establish location.
 * @param pCallback               a callback that will be called when
 *                                location has been determined.  The
 *                                first parameter to the callback is the
 *                                network handle, the second parameter is a
 *                                uLocation_t structure.
 * @return                        zero on success or negative error code on
 *                                failure.
 */
int32_t uLocationGetStart(int32_t networkHandle, uLocationType_t type,
                          uLocationAssist_t *pLocationAssist,
                          const char *pAuthenticationTokenStr,
                          void (*pCallback) (int32_t, uLocation_t));

/** Get the current status of a location establishment attempt.
 *
 * @param networkHandle  the handle of the network instance.
 * @return               the status.
 */
uLocationStatus_t uLocationGetStatus(int32_t networkHandle);

/** Cancel a uLocationGetStart(); after calling this function the
 * callback passed to uLocationGetStart() will not be called until
 * another uLocationGetStart() is begun.
 *
 * @param networkHandle  the handle of the network instance.
 */
void uLocationGetStop(int32_t networkHandle);

#ifdef __cplusplus
}
#endif

#endif // _U_LOCATION_H_

// End of file
