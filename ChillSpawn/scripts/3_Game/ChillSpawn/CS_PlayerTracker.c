class CS_SpawnEntry
{
	string Preset;
	float Playtime;		// player's playtime in seconds when they spawned it
	int RealTime;		// CS_Clock.Now() when they spawned it
}

// Real-world time as seconds since 2000-01-01 UTC, so limits survive restarts.
class CS_Clock
{
	static int Now()
	{
		int year, month, day, hour, minute, second;
		GetYearMonthDayUTC(year, month, day);
		GetHourMinuteSecondUTC(hour, minute, second);
		return DaysSince2000(year, month, day) * 86400 + hour * 3600 + minute * 60 + second;
	}

	protected static int DaysSince2000(int year, int month, int day)
	{
		array<int> monthDays = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		int days = 0;

		for (int y = 2000; y < year; y++)
			days += 365 + IsLeap(y);

		for (int m = 1; m < month; m++)
		{
			days += monthDays[m - 1];
			if (m == 2)
				days += IsLeap(year);
		}

		return days + day - 1;
	}

	protected static int IsLeap(int year)
	{
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
			return 1;

		return 0;
	}
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

// Tracks each player's time in game and their spawn history, so limits count
// playtime rather than wall-clock time. Saved per player under
// $profile:ChillSpawn/players/.
class CS_PlayerTracker
{
	static const string DIR = "$profile:ChillSpawn/players";
	// History older than this is dropped, so limits can be up to a day of playtime.
	static const float HISTORY_SECONDS = 86400;

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

	// Spawns of this preset within the last windowSeconds of playtime, or of
	// real-world time when realTime is set. waitSeconds is how long until the
	// oldest slot frees up.
	int CountRecentSpawns(string id, string preset, float windowSeconds, bool realTime, out float waitSeconds)
	{
		CS_PlayerRecord rec = GetRecord(id);
		float now = GetPlaytime(id);
		if (realTime)
			now = CS_Clock.Now();

		float oldest = -1;
		int count = 0;

		foreach (CS_SpawnEntry entry : rec.Spawns)
		{
			float when = entry.Playtime;
			if (realTime)
				when = entry.RealTime;

			if (entry.Preset != preset || when <= now - windowSeconds)
				continue;

			count++;
			if (oldest < 0 || when < oldest)
				oldest = when;
		}

		waitSeconds = 0;
		if (oldest >= 0)
			waitSeconds = oldest + windowSeconds - now;

		return count;
	}

	void RecordSpawn(string id, string preset)
	{
		CS_SpawnEntry entry = new CS_SpawnEntry();
		entry.Preset = preset;
		entry.Playtime = GetPlaytime(id);
		entry.RealTime = CS_Clock.Now();
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

	// Folds the current session into the stored playtime, drops old history and
	// writes the file.
	protected void Flush(string id)
	{
		CS_PlayerRecord rec = GetRecord(id);
		rec.PlaytimeSeconds = GetPlaytime(id);
		m_SessionStartMs.Set(id, GetGame().GetTime());

		int now = CS_Clock.Now();
		for (int i = rec.Spawns.Count() - 1; i >= 0; i--)
		{
			CS_SpawnEntry entry = rec.Spawns[i];
			if (entry.Playtime <= rec.PlaytimeSeconds - HISTORY_SECONDS && entry.RealTime <= now - HISTORY_SECONDS)
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
