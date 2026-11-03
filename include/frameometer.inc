#if defined _frameometer_included
	#endinput
#endif
#define _frameometer_included

#pragma reqlib frameometer
#if !defined AMXMODX_NOAUTOLOAD
	#pragma loadlib frameometer
#endif

/**
 * Measured client fps, averaged over the most recent completed 1 second windows.
 *
 * @param player     Player index
 * @param windows    Number of windows to average, 1-60; clamped to what is available
 *
 * @return           Frames per second, or 0.0 if no window has completed
 */
native Float:fps_get(const player, const windows = 1);

/**
 * Client-reported fps: usercmds divided by the sum of their msec, over the same windows as fps_get.
 *
 * @param player     Player index
 * @param windows    Number of windows to average, 1-60; clamped to what is available
 *
 * @return           Frames per second, or 0.0 if no window has completed or msec summed to 0
 */
native Float:fps_msec(const player, const windows = 1);

/**
 * Number of completed 1 second windows held for a player, up to 60. Windows that lost commands are discarded.
 *
 * @param player     Player index
 *
 * @return           Window count
 */
native fps_windows(const player);

/**
 * Clears a player's measurement history.
 *
 * @param player     Player index
 *
 * @noreturn
 */
native fps_reset(const player);
