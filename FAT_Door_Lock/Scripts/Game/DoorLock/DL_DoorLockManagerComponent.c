//------------------------------------------------------------------------------------------------
// FAT_mod scipt
// Licence APL_ND
//------------------------------------------------------------------------------------------------


class DL_DoorLockConfig : JsonApiStruct
{
	ref array<string> authorizedIds = {};

	void DL_DoorLockConfig()
	{
		RegV("authorizedIds");
	}
}

//------------------------------------------------------------------------------------------------
class DL_DoorLockState : JsonApiStruct
{
	ref array<string> lockedDoors = {};

	void DL_DoorLockState()
	{
		RegV("lockedDoors");
	}
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "DoorLock", description: "Gestion du verrouillage des portes. A placer sur l'entité GameMode.")]
class DL_DoorLockManagerComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class DL_DoorLockManagerComponent : ScriptComponent
{
	[Attribute("$profile:DoorLock/config.json", desc: "Fichier JSON des ID autorisés (côté serveur)")]
	protected string m_sConfigPath;

	[Attribute("$profile:DoorLock/locked_doors.json", desc: "Fichier JSON des portes verrouillées (côté serveur)")]
	protected string m_sStatePath;

	[Attribute("1", desc: "Conserver les portes verrouillées après redémarrage du serveur")]
	protected bool m_bPersist;

	[Attribute("30", desc: "Relecture du fichier des ID toutes les X secondes (0 = jamais, seulement au démarrage/connexion)", params: "0 600 1")]
	protected int m_iReloadIntervalSec;

	protected static DL_DoorLockManagerComponent s_Instance;

	protected ref array<string> m_aLockedDoors = {};

	protected ref array<string> m_aAuthorizedIds = {};

	//------------------------------------------------------------------------------------------------
	static DL_DoorLockManagerComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	static string GetDoorKey(IEntity door)
	{
		vector p = door.GetOrigin();
		int x = Math.Round(p[0] * 10);
		int y = Math.Round(p[1] * 10);
		int z = Math.Round(p[2] * 10);
		return string.Format("%1|%2|%3", x, y, z);
	}

	//------------------------------------------------------------------------------------------------
	bool IsLocked(IEntity door)
	{
		if (!door)
			return false;

		return m_aLockedDoors.Contains(GetDoorKey(door));
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		s_Instance = this;
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;

		FileIO.MakeDirectory("$profile:DoorLock");
		LoadConfig();

		if (m_bPersist)
			LoadState();

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(owner);
		if (!gameMode)
		{
			Print("[DoorLock] Le composant doit être placé sur une entité SCR_BaseGameMode", LogLevel.ERROR);
			return;
		}

		gameMode.GetOnPlayerAuditSuccess().Insert(OnPlayerAudited);
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);

		if (m_iReloadIntervalSec > 0)
			GetGame().GetCallqueue().CallLater(ReloadConfig, m_iReloadIntervalSec * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (s_Instance == this)
			s_Instance = null;

		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(ReloadConfig);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(owner);
		if (gameMode)
		{
			gameMode.GetOnPlayerAuditSuccess().Remove(OnPlayerAudited);
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerAudited(int playerId)
	{
		LoadConfig();
		RefreshPlayer(playerId);

		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		Print(string.Format("[DoorLock] Connexion %1 — Identity ID : %2 — autorisé : %3", GetGame().GetPlayerManager().GetPlayerName(playerId), uid, IsAuthorized(playerId)), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		RefreshPlayer(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReloadConfig()
	{
		if (!LoadConfig())
			return;

		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		foreach (int playerId : players)
		{
			RefreshPlayer(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshPlayer(int playerId)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (pc)
			pc.DL_SetDoorKeyHolder(IsAuthorized(playerId));
	}

	//------------------------------------------------------------------------------------------------
	bool IsAuthorized(int playerId)
	{
		if (m_aAuthorizedIds.Contains("*"))
			return true;

		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (uid.IsEmpty())
			return false;

		return m_aAuthorizedIds.Contains(uid);
	}

	//------------------------------------------------------------------------------------------------
	void RequestToggle(IEntity door, IEntity user)
	{
		if (!Replication.IsServer() || !door || !user)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId <= 0 || !IsAuthorized(playerId))
		{
			Print(string.Format("[DoorLock] Tentative refusée (playerId %1)", playerId), LogLevel.WARNING);
			return;
		}

		string key = GetDoorKey(door);
		bool lock = !m_aLockedDoors.Contains(key);

		ApplyLocal(key, lock);
		Rpc(RpcDo_SetLocked, key, lock);

		if (m_bPersist)
			SaveState();

		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		string verb = "Déverrouillage";
		if (lock)
			verb = "Verrouillage";

		Print(string.Format("[DoorLock] %1 porte %2 par %3 (%4)", verb, key, GetGame().GetPlayerManager().GetPlayerName(playerId), uid), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyLocal(string key, bool lock)
	{
		if (lock)
		{
			if (!m_aLockedDoors.Contains(key))
				m_aLockedDoors.Insert(key);
		}
		else
		{
			m_aLockedDoors.RemoveItem(key);
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetLocked(string key, bool lock)
	{
		ApplyLocal(key, lock);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		int count = m_aLockedDoors.Count();
		writer.WriteInt(count);
		foreach (string key : m_aLockedDoors)
		{
			writer.WriteString(key);
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int count;
		if (!reader.ReadInt(count))
			return false;

		m_aLockedDoors.Clear();
		for (int i = 0; i < count; i++)
		{
			string key;
			if (!reader.ReadString(key))
				return false;

			m_aLockedDoors.Insert(key);
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoadConfig()
	{
		DL_DoorLockConfig config = new DL_DoorLockConfig();

		if (!FileIO.FileExists(m_sConfigPath))
		{
			// Crée un modèle au premier lancement, à compléter par les admins
			config.authorizedIds.Insert("REMPLACER-PAR-IDENTITY-ID # Nom du joueur");
			config.Pack();
			config.SaveToFile(m_sConfigPath);
			Print("[DoorLock] Fichier créé : " + m_sConfigPath + " — ajoutez les Identity ID dans authorizedIds", LogLevel.WARNING);
		}
		else if (!config.LoadFromFile(m_sConfigPath))
		{
			// JSON invalide (virgule oubliée...) : on garde la liste précédente
			Print("[DoorLock] JSON invalide, liste précédente conservée : " + m_sConfigPath, LogLevel.ERROR);
			return false;
		}

		array<string> newIds = {};
		foreach (string entry : config.authorizedIds)
		{
			int hash = entry.IndexOf("#");
			if (hash >= 0)
				entry = entry.Substring(0, hash);

			entry.TrimInPlace();
			if (!entry.IsEmpty() && !newIds.Contains(entry))
				newIds.Insert(entry);
		}

		bool changed = newIds.Count() != m_aAuthorizedIds.Count();
		if (!changed)
		{
			foreach (string id : newIds)
			{
				if (!m_aAuthorizedIds.Contains(id))
				{
					changed = true;
					break;
				}
			}
		}

		if (changed)
		{
			m_aAuthorizedIds.Clear();
			m_aAuthorizedIds.Copy(newIds);
			Print(string.Format("[DoorLock] Liste des ID chargée : %1 entrée(s)", m_aAuthorizedIds.Count()), LogLevel.NORMAL);
		}

		return changed;
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadState()
	{
		if (!FileIO.FileExists(m_sStatePath))
			return;

		DL_DoorLockState state = new DL_DoorLockState();
		if (!state.LoadFromFile(m_sStatePath))
		{
			Print("[DoorLock] Lecture impossible : " + m_sStatePath, LogLevel.ERROR);
			return;
		}

		m_aLockedDoors.Clear();
		foreach (string key : state.lockedDoors)
		{
			m_aLockedDoors.Insert(key);
		}

		Print(string.Format("[DoorLock] %1 porte(s) verrouillée(s) restaurée(s)", m_aLockedDoors.Count()), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveState()
	{
		DL_DoorLockState state = new DL_DoorLockState();
		foreach (string key : m_aLockedDoors)
		{
			state.lockedDoors.Insert(key);
		}

		state.Pack();
		state.SaveToFile(m_sStatePath);
	}
}
