modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		CS_Core.Get();
	}

	override void OnMissionFinish()
	{
		CS_Core.Get().Tracker.FlushAll();
		super.OnMissionFinish();
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (identity)
			CS_Core.Get().Tracker.OnConnect(identity.GetId());
	}

	override void PlayerDisconnected(PlayerBase player, PlayerIdentity identity, string uid)
	{
		CS_Core.Get().Tracker.OnDisconnect(uid);
		super.PlayerDisconnected(player, identity, uid);
	}

	override void OnEvent(EventType eventTypeId, Param params)
	{
		super.OnEvent(eventTypeId, params);

		if (eventTypeId != ChatMessageEventTypeID)
			return;

		ChatMessageEventParams chat = ChatMessageEventParams.Cast(params);
		if (!chat)
			return;

		// The chat event only gives the sender's name, so match it to a player.
		PlayerBase player = CS_FindPlayerByName(chat.param2);
		if (player)
			CS_Core.Get().Commands.TryDispatch(player, chat.param3);
	}

	protected PlayerBase CS_FindPlayerByName(string name)
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		PlayerBase match;
		foreach (Man man : players)
		{
			PlayerBase candidate = PlayerBase.Cast(man);
			if (!candidate || !candidate.GetIdentity() || candidate.GetIdentity().GetName() != name)
				continue;

			// Two players with the same name: refuse rather than charge the wrong one.
			if (match)
			{
				Print("[ChillSpawn] Ignoring command: more than one player is named " + name);
				return null;
			}

			match = candidate;
		}

		return match;
	}
}
