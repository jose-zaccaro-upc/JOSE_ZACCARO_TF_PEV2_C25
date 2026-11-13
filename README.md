# Trabajo Final PEV 2 - Arms Race Arena
**Alumno:** Jose Antonio Lopez Zaccaro | **Legajo:** 42637455 | **Modo:** Arena Shooter / Escalada de Armas | **Versión de Unreal:** UE 5.6

## Reglas y Mecánica Principal
**Mecánica:** Arena shooter todos contra todos donde cada baja cambia y mejora automáticamente el arma del jugador.
**Cómo se gana:** El primer jugador en conseguir 10 bajas completando la rotación de armas gana la partida.

## Ficha Técnica de Red
**RepNotify:**
- `CurrentWeaponIndex` en `PEV2_Character` -> Actualiza el nivel de arma en clientes, equipa la nueva malla y actualiza el HUD.
- `CurrentHealth` y `CurrentShield` en `PEV2_HealthComponent` -> Sincroniza barras de vida y escudo en el HUD de los clientes.
- `Eliminations` en `PEV2_PlayerState` -> Sincroniza el marcador de bajas de cada jugador.

**Server RPC:**
- `Server_StartFire`, `Server_StopFire` y `Server_Reload` en `PEV2_WeaponBase` -> Validación autoritativa de disparo, trazado de rayos (Hitscan) o generación de proyectiles, y recarga de munición en servidor.
- `Server_UseAbility` en `PEV2_AbilityComponent` -> Validación y activación autoritativa de granadas arrojables y power-ups (Dash, Escudo, Velocidad).

**Client RPC:**
- `Client_NotifyKillScored` en `PEV2_PlayerController` -> Notificación directa al atacante tras una baja confirmada con feedback en pantalla y conteo de progreso.
- `Client_ReceiveHitFeedback` en `PEV2_PlayerController` -> Indicador direccional de impacto y viñeta de daño en la pantalla de la víctima.
- `Client_NotifyHitConfirmed` en `PEV2_PlayerController` -> Hitmarker y sonido de impacto confirmado (y headshot) para el tirador.

**NetMulticast RPC:**
- `Multicast_PlayFireFX` en `PEV2_WeaponBase` -> Fogonazo en Muzzle, trazador Niagara (beam) y efectos de impacto/audio a todos los clientes.
- `Multicast_PlayExplosionFX` en `PEV2_ProjectileBase` -> Partículas Niagara de explosión y audio de detonación a todos los clientes.
- `Multicast_PlayEliminationFX` y `Multicast_ResetRespawnFX` en `PEV2_Character` -> Física de Ragdoll al morir y reincorporación al reaparecer tras 3 segundos.

## Cómo Probar el Proyecto
1. Abrir el mapa: `Content/PEV2/Maps/L_GameArena`.
2. En Play (PIE): 
    `Number of Players = 2` (o 4+) y `Net Mode = Play as Listen Server`.