# Trabajo Final PEV 2 - Arms Race Arena
**Alumno:** Jose Antonio Lopez Zaccaro | **Legajo:** 42637455 | **Modo:** Arena Shooter / Escalada de Armas | **Versión de Unreal:** UE 5.6

## Reglas y Mecánica Principal
**Mecánica:** Arena shooter todos contra todos donde cada baja cambia y mejora automáticamente el arma del jugador.
**Cómo se gana:** El primer jugador en conseguir 10 bajas completando la rotación de armas gana la partida.

## Ficha Técnica de Red
**RepNotify:**
CurrentWeaponIndex en PEV2_Character -> Actualiza el mesh del arma en clientes y el HUD.

**Server RPC:**
Server_FireWeapon en PEV2_Character -> Validación de disparo y daño en el servidor con autoridad.

**Client RPC:**
Client_NotifyKillScored en PEV2_PlayerController -> Aviso directo al atacante con feedback de baja y sonido local.

**NetMulticast RPC:**
Multicast_PlayFireEffects en PEV2_Character -> VFX de disparo, trazador y audio global a todos.

## Cómo Probar el Proyecto
1. Abrir el mapa: `Content/TFPEV2/Maps/L_GameArena`.
2. En Play (PIE): 
    `Number of Players = 2` (o 4+) y `Net Mode Play as Listen Server`.