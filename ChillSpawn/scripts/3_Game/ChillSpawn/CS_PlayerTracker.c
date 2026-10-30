class CS_SpawnEntry
{
	string Preset;
	float Playtime;		// player's playtime in seconds when they spawned it
}

class CS_PlayerRecord
{
	float PlaytimeSeconds;
	ref array<ref CS_SpawnEntry> Spawns;

	void CS_PlayerRecord()
	{
		Spawns = new array<ref CS_SpawnEntry>;
	}
}

// Tracks each player's time in game and their spawn history, so limits are
// "per hour of playtime" rather than per wall-clock hour. Saved per player under
// $profile:ChillSpawn/players/.
class CS_PlayerTracker
{
	static const string DIR = "$profile:ChillSpawn/players";
	static const float WINDOW_SECONDS = 3600;

	protected ref map<string, ref CS_PlayerRecord> m_Records;
	protected ref map<string, int> m_SessionStartMs;

	void CS_PlayerTracker()
	{
		m_Records = new map<string, ref CS_PlayerRecord>;
		m_SessionStartMs = new map<string, int>;

		if (!FileExist(DIR))
			MakeDirectory(DIR);
	}

	void OnConnect(string id)
	{
		GetRecord(id);
	}

	void OnDisconnect(string id)
	{
		if (!m_Records.Contains(id))
			return;

		Flush(id);
		m_Records.Remove(id);
		m_SessionStartMs.Remove(id);
	}

	void FlushAll()
	{
		foreach (string id, CS_PlayerRecord rec : m_Records)
			Flush(id);
	}

	float GetPlaytime(string id)
	{
		CS_PlayerRecord rec = GetRecord(id);
		int elapsedMs = GetGame().GetTime() - m_SessionStartMs.Get(id);
		return rec.PlaytimeSeconds + elapsedMs / 1000.0;
	}

	// Spawns of this preset within the last hour of playtime. waitSeconds is how
	// much more playtime frees the oldest slot.
	int CountRecentSpawns(string id, string preset, out float waitSeconds)
	{
		CS_PlayerRecord rec = GetRecord(id);
		float now = GetPlaytime(id);
		float oldest = -1;
		int count = 0;

		foreach (CS_SpawnEntry entry : rec.Spawns)
		{
			if (entry.Preset != preset || entry.Playtime <= now - WINDOW_SECONDS)
				continue;

			count++;
			if (oldest < 0 || entry.Playtime < oldest)
				oldest = entry.Playtime;
		}

		waitSeconds = 0;
		if (oldest >= 0)
			waitSeconds = oldest + WINDOW_SECONDS - now;

		return count;
	}

	void RecordSpawn(string id, string preset)
	{
		CS_SpawnEntry entry = new CS_SpawnEntry();
		entry.Preset = preset;
		entry.Playtime = GetPlaytime(id);
		GetRecord(id).Spawns.Insert(entry);
		Flush(id);
	}

	protected CS_PlayerRecord GetRecord(string id)
	{
		CS_PlayerRecord rec = m_Records.Get(id);
		if (rec)
			return rec;

		string error;
		string path = GetPath(id);
		if (!FileExist(path) || !JsonFileLoader<CS_PlayerRecord>.LoadFile(path, rec, error))
			rec = new CS_PlayerRecord();

		m_Records.Set(id, rec);
		m_SessionStartMs.Set(id, GetGame().GetTime());
		return rec;
	}

	// Folds the current session into the stored playtime, drops history older
	// than the window and writes the file.
	protected void Flush(string id)
	{
		CS_PlayerRecord rec = GetRecord(id);
		rec.PlaytimeSeconds = GetPlaytime(id);
		m_SessionStartMs.Set(id, GetGame().GetTime());

		for (int i = rec.Spawns.Count() - 1; i >= 0; i--)
		{
			if (rec.Spawns[i].Playtime <= rec.PlaytimeSeconds - WINDOW_SECONDS)
				rec.Spawns.Remove(i);
		}

		string error;
		if (!JsonFileLoader<CS_PlayerRecord>.SaveFile(GetPath(id), rec, error))
			Print("[ChillSpawn] Could not save player record: " + error);
	}

	protected string GetPath(string id)
	{
		// Player IDs are base64 and may contain characters that are unsafe in file names.
		string safe = id;
		safe.Replace("/", "_");
		safe.Replace("+", "-");
		safe.Replace("=", "");
		return DIR + "/" + safe + ".json";
	}
}
