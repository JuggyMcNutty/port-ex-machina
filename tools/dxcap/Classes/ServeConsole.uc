//=============================================================================
// ServeConsole: a listen server for the other engine to join -- a
// deathmatch on DXMP_Cathedral, started from the menu map as the game's Host
// screen starts one, never on the master servers' lists (the run's ini has
// no uplink). While serving, once another player is in, the host's own
// player stands 300 units in front of it, turned across its view, and walks
// 2 s, stands 2 s and turns about, over and over, for the client to watch;
// where each player stands is logged every 2 s. Exits after 290 s.
//=============================================================================
class ServeConsole extends Console;

var float CapTime, LogTime, WalkTime;
var int Phase;
var bool bPlaced;

event Tick(float Delta)
{
	local PlayerPawn P, Other;

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
	else if (Phase == 2)
	{
		if (!bPlaced)
		{
			foreach P.AllActors(class'PlayerPawn', Other)
			{
				if (Other != P && P.SetLocation(Other.Location + 300 * vector(Other.Rotation)))
				{
					P.ViewRotation.Yaw = Other.Rotation.Yaw + 16384;
					P.SetRotation(P.ViewRotation);
					Log("DXCAP: host placed at " $ P.Location $ " before " $ Other.Location);
					bPlaced = true;
					WalkTime = 0.0;
					break;
				}
			}
		}
		WalkTime += Delta;
		if (WalkTime < 2.0)
			P.aBaseY = 300.0;
		else
			P.aBaseY = 0.0;
		if (WalkTime >= 4.0)
		{
			WalkTime = 0.0;
			P.ViewRotation.Yaw += 32768;
		}
		if (CapTime - LogTime >= 2.0)
		{
			LogTime = CapTime;
			foreach P.AllActors(class'PlayerPawn', Other)
				Log("DXCAP: " $ Other.PlayerReplicationInfo.PlayerName $ " at " $ Other.Location);
		}
	}

	if (Phase >= 1 && Phase < 9 && CapTime > 290.0)
	{
		Log("DXCAP: done, exiting");
		P.ConsoleCommand("exit");
		Phase = 9;
	}
}
