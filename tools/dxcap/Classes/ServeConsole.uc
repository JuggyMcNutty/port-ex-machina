//=============================================================================
// ServeConsole: a listen server for the other engine to join -- a
// deathmatch on DXMP_Cathedral, started from the menu map as the game's Host
// screen starts one, never on the master servers' lists (the run's ini has
// no uplink). Exits after 290 s.
//=============================================================================
class ServeConsole extends Console;

var float CapTime;
var int Phase;

event Tick(float Delta)
{
	local PlayerPawn P;

	Super.Tick(Delta);
	if (Viewport == None || Viewport.Actor == None)
		return;
	P = Viewport.Actor;
	CapTime += Delta;

	if (Phase == 0 && CapTime > 3.0)
	{
		Log("DXCAP: starting the listen server");
		P.ConsoleCommand("start DXMP_Cathedral?game=DeusEx.DeathMatchGame?listen");
		Phase = 1;
	}
	else if (Phase == 1 && P.Level.NetMode != NM_Standalone)
	{
		Log("DXCAP: serving " $ P.Level.GetLocalURL() $ ", net mode " $ P.Level.NetMode);
		Phase = 2;
	}
	else if (Phase >= 1 && CapTime > 290.0)
	{
		Log("DXCAP: done, exiting");
		P.ConsoleCommand("exit");
		Phase = 9;
	}
}
