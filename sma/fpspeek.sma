#include <amxmodx>
#include <amxmisc>
#include <frameometer>

#define PLUGIN "fpspeek"
#define VERSION "1.1"
#define AUTHOR "arc"

public plugin_init()
{
	register_plugin(PLUGIN, VERSION, AUTHOR)
	register_concmd("amx_fps", "cmd_fps", ADMIN_KICK, "[name|#userid] - show measured fps")
}

public cmd_fps(id, level, cid)
{
	if (!cmd_access(id, level, cid, 1))
		return PLUGIN_HANDLED

	new filter[32]
	read_argv(1, filter, charsmax(filter))

	new userid = (filter[0] == '#') ? str_to_num(filter[1]) : 0

	console_print(id, "%-4s %-7s %-8s %-8s %-8s %-8s %s", "id", "userid", "fps", "msec", "5s", "30s", "name")

	new players[MAX_PLAYERS], num, shown
	get_players(players, num, "ch")

	for (new i = 0; i < num; i++)
	{
		new pid = players[i]
		new puserid = get_user_userid(pid)
		new name[MAX_NAME_LENGTH]
		get_user_name(pid, name, charsmax(name))

		if (userid > 0)
		{
			if (puserid != userid)
				continue
		}
		else if (filter[0] && containi(name, filter) == -1)
		{
			continue
		}

		new idstr[8], fps[16], msec[16], avg[16], avgLong[16]
		formatex(idstr, charsmax(idstr), "#%d", pid)
		format_fps(fps, charsmax(fps), fps_get(pid, 1))
		format_fps(msec, charsmax(msec), fps_msec(pid, 1))
		format_fps(avg, charsmax(avg), fps_get(pid, 5))
		format_fps(avgLong, charsmax(avgLong), fps_get(pid, 30))

		console_print(id, "%-4s %-7d %-8s %-8s %-8s %-8s %s", idstr, puserid, fps, msec, avg, avgLong, name)
		shown++
	}

	console_print(id, "%d player(s)", shown)
	return PLUGIN_HANDLED
}

format_fps(out[], len, Float:fps)
{
	if (fps == 0.0)
		copy(out, len, "?")
	else
		formatex(out, len, "%.1f", fps)
}
