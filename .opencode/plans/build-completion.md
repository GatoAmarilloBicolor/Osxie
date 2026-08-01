# Build completion: OOM analysis & single-command automation

## Estado (31 jul 2026)

- Build de Osxie (generador **Unix Makefiles**, dir `build-all`, `COMPONENTS=all`,
  `-DBUILD_Foundation=ON`, Release, clang `/usr/bin/clang`) detenido al **~15%**.
- Bloqueos encadenados ya resueltos: SDK mach headers pisados, OpenLDAP `open.c:47`
  (guard DARLING→OSXIE), python `python.c` (`err.h`), SDK `GSS.framework` relinked
  de `darling/include/GSS` → `osxie/include/GSS`, `SecurityTokend.framework`
  regenerado, `framework-include`/`framework-private-include` regenerados (0 rotos).
- **No hay ccache** instalado (`CCACHE_PROGRAM-NOTFOUND`): cada reintento recompila.
  Instalar ccache (`pacman -S ccache`) aceleraría mucho los reintentos tras OOM.

## Causa raíz de los fallos de build

La máquina (17Gi RAM) está compartida con otros agentes/concurrentes que usan ~12Gi
(baloo ~1.1Gi, firefox, varios `opencode`). Con `-j2`, dos TUs pesadas
(`LowLevelInterpreter.cpp.o` de JavaScriptCore, `NSPredicateParser.tab.c`/`Operator`,
`cryptkitcsp.cpp.o`, `libunwind`, `CFBasicHash.c.o`) disparan OOM del sistema.

### Fallo del retry-loop original

El loop `/tmp/opencode/retry-build.sh` detectaba OOM con `grep Terminado` y reintentaba,
pero el OOM killer se llevaba **el propio shell del loop** (el log de salida quedaba
vacío y el proceso moría en silencio). Detectado en el log: cada ejecución solo
alcanzaba `=== ATTEMPT 1 ===`, con `Terminado` en `_nfsclnt.S.o`, `security_apple_csp`
y `LowLevelInterpreter.cpp.o`.

## Solución (scripts/build_complete.sh)

- **Un solo comando** para todo el ciclo: resume build → reintenta OOM → install
  (sudo/pkexec) → opcionalmente fork+push de los submódulos osxificados.
- **`ulimit -v` por proceso** (6Gi, baja a 5Gi tras un OOM): el propio build nunca
  dispara OOM de sistema, así el shell del retry-loop ya no es víctima del killer.
  TUs observados ≤ ~2.5Gi, así que el techo es seguro.
- **Paralelismo adaptativo**: `-j2` si `MemAvailable ≥ 8Gi`, si no `-j1`; tras el
  primer OOM se fija `-j1` (TU pico ~2.5Gi + 12Gi de terceros = 14.5Gi < 17Gi → estable).
- **Detección de error real vs OOM**: rc≠0 con `Terminado|Killed|bad_alloc` → OOM,
  reintentar; cualquier otra cosa → parar con el último `error:`/`fatal error`
  para fixear (p. ej. guards `DARLING`→`defined(DARLING)||defined(OSXIE)`).

### Comandos mínimos

```bash
./scripts/build_complete.sh                 # build + install (1 comando)
./scripts/build_complete.sh --no-install    # solo build
./scripts/build_complete.sh --osxify-push   # build + install + fork/push submódulos
./scripts/build_complete.sh --jobs=4 --max-attempts=60
# logs: /tmp/opencode/build-retry.log (marcas "=== ATTEMPT N ... ===")
```

## Osxificación / forks (fase post-build)

Submódulos **genuinamente** modificados (todo lo demás en `git submodule status`
es basura de build y se ignora):

| Submódulo | Cambios |
|---|---|
| `src/external/OpenLDAP` | `open.c:47` guard DARLING→OSXIE; `CMakeLists.txt`; `darling/` removido |
| `src/external/python` | `python.c` guard OSXIE; `CMakeLists.txt`; `darling/` removido |
| `src/external/JavaScriptCore` | osxificación Fase 1 (`OSXIE_NONUNIFIED_BUILD`, muchos archivos) |
| `src/external/Heimdal` | `CMakeLists.txt`; `darling/include/*` removido |
| `src/external/security` | guards + `CMakeLists.txt` |
| Padre (Developer no es submódulo) | relinks del SDK `darling/`→`osxie/`, frameworks regenerados |

Procedimiento (`--osxify-push` lo automatiza por submódulo):
1. `git add -u` (solo tracked: no sube basura untracked) + commit `osxify: replace DARLING guards/refs with OSXIE`.
2. `git checkout -B osxie`.
3. `gh repo fork <upstream> --remote=false` (crea fork bajo `GatoAmarilloBicolor`).
4. `git remote add fork` + `git push -u fork osxie`.
5. Padre: commit `Developer cmake CMakeLists.txt scripts` → `git push` a `GatoAmarilloBicolor/Osxie`.

La cuenta `gh` es `GatoAmarilloBicolor` (scopes `repo`, `workflow`) y el padre ya
apunta a `github.com/GatoAmarilloBicolor/Osxie.git`.
