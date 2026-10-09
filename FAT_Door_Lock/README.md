# DoorLock — verrouillage de portes par liste d'ID

Les joueurs autorisés ont une action **Verrouiller / Déverrouiller la porte** sur n'importe quelle porte de bâtiment. Porte verrouillée = l'action vanilla Ouvrir/Fermer affiche « Verrouillée » pour tout le monde.

## Fichiers
- `DL_DoorLockManagerComponent.c` — état des verrous (serveur), réplication + JIP, liste d'ID, persistance JSON
- `DL_DoorLockUserAction.c` — l'action Verrouiller/Déverrouiller
- `DL_DoorUserAction.c` — bloque l'ouverture vanilla si verrouillée
- `DL_PlayerController.c` — indique au client s'il est autorisé (affichage de l'action)

## Mise en place Workbench
1. **GameMode** : ajouter `DL_DoorLockManagerComponent` sur l'entité GameMode du monde (ou sur ton prefab de game mode).
2. **Prefab de porte** : dans le Resource Browser, ouvrir une porte de bâtiment et remonter son héritage jusqu'au prefab de porte de base → **Override in DoorLock**.
   Dans `ActionsManagerComponent` → *Additional Actions* → ajouter `DL_DoorLockUserAction`, *Parent Context List* = le même contexte que l'action Ouvrir/Fermer existante, UI Info Name = « Verrou ».
   S'il y a plusieurs prefabs de base (portes doubles, portails…), faire l'override sur chacun.

## Liste des ID (fichier serveur, géré par les admins)
Le fichier vit **uniquement sur le serveur**, dans le dossier profil (`-profile` de la ligne de lancement) :
`<dossier profil>/DoorLock/config.json` — créé automatiquement au 1er lancement avec un modèle.

```json
{
  "authorizedIds": [
    "a1b2c3d4-xxxx-xxxx-xxxx-xxxxxxxxxxxx # Dark",
    "e5f6a7b8-xxxx-xxxx-xxxx-xxxxxxxxxxxx # Bravo 1"
  ]
}
```
- ID = Identity ID Bohemia. Le mod l'écrit dans le log serveur à chaque connexion :
  `[DoorLock] Connexion <Nom> — Identity ID : <ID> — autorisé : 0/1` → copier-coller dans le fichier.
- `# Nom` après l'ID est facultatif (ignoré, juste pour s'y retrouver).
- **Mise à jour à chaud** : le mod relit le fichier toutes les 30 s (attribut *Reload Interval Sec*) et met à jour les joueurs connectés. Pas de redémarrage, pas de republication du mod.
- JSON invalide → l'erreur est loguée et la liste précédente reste active.
- `"*"` = tout le monde autorisé → **nécessaire pour tester en Workbench** (pas d'ID backend en local).
- Le chemin du fichier est modifiable via l'attribut *Config Path* du composant.

Portes verrouillées sauvegardées dans `$profile/DoorLock/locked_doors.json` (désactivable via l'attribut *Persist*).

## Limites connues
- Une porte est identifiée par sa position : si un bâtiment est déplacé dans le monde, son verrou est perdu.
- L'IA peut encore ouvrir les portes verrouillées (non traité).
- Signatures à vérifier à la compilation : `SCR_DoorUserAction.CanBePerformedScript`, `DoorComponent.SetControlValue(valeur, instigateur)`, `SCR_BaseGameMode.GetOnPlayerAuditSuccess`.
