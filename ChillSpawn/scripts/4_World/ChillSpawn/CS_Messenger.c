class CS_Messenger
{
	// Private chat line to one player. Uses a vanilla RPC, so clients need no mod.
	static void Send(PlayerBase player, string message)
	{
		if (!player || !player.GetIdentity())
			return;

		Param1<string> param = new Param1<string>(message);
		GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, param, true, player.GetIdentity());
	}
}
