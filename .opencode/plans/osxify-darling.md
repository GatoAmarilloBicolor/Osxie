# Plan: completar osxificación (DARLING → OSXIE) y desbloquear el build

## Estado
- Build completo (`make -C build-all`) bloqueado solo por `API/JSScript.mm` en i386.
- Causa raíz diagnosticada y reproducida (preprocesado del TU i386 de JSScript.mm):
  - `JSBase.h` nunca se incluye en el TU de JSScript.mm → `JSC_OBJC_API_ENABLED` indefinido → todo `JSScript.h` se salta (errores: `JSScriptType` unknown, `'JSScript' cannot use 'super' because it is a root class`, `kJSScriptTypeProgram/Module` undeclared, etc.).
  - Motivo: `API/JSScriptInternal.h:26`, `API/JSValue.h:31`, `API/JSScriptSourceProvider.h` y `osxie/include/JavaScriptCore/JSValue.h` incluyen `<JavaScriptCore/JSBase.h>` bajo `#ifdef OSXIE_NONUNIFIED_BUILD`, pero el build solo define `DARLING_NONUNIFIED_BUILD` (`src/external/JavaScriptCore/CMakeLists.txt:2153`).
  - `JSContext.mm` compila solo porque incluye `JSBaseInternal.h:30` → `<JavaScriptCore/JSBase.h>` sin guard.
- Es la inconsistencia del rename: el código se renombró a `OSXIE_*` pero los defines de build no se actualizaron (único macro `OSXIE_*` sin definir en JSC).

## Fase 1 — Desbloquear JSC (APROBADA por el usuario)
1. `src/external/JavaScriptCore/CMakeLists.txt:2153`:
   `add_compile_definitions(DARLING_NONUNIFIED_BUILD OSXIE_NONUNIFIED_BUILD)` (definir ambos; 7 archivos aún usan el viejo).
2. Reconfigurar: `cmake -DCMAKE_BUILD_TYPE=Release -DCOMPONENTS=all -S . -B build-all`
3. Rebuild: `make -C build-all -j$(nproc) JavaScriptCore`
4. Si quedan errores en el branch i386 (imports `CachedBytecode.h`/`WeakObjCPtr.h`/`FileSystem.h` ya activos), iterar.

## Fase 2 — Estrategia del rename (directiva del usuario)
- "Todo submódulo que haga referencia a darling debe ser forkeado y subido a mi GitHub y terminar de osxificar lo que falta."
- Implica: submodulos → fork a `github.com/GatoAmarilloBicolor/...`, y completar el rename DARLING→OSXIE + definir los macros `OSXIE*` que faltan (p. ej. `-DOSXIE_NONUNIFIED_BUILD`, decidir `-DDARLING` global vs por-subproyecto).
- Pendientes concretos ya identificados:
  - 503 guards `defined(DARLING)`/`#ifdef DARLING` en `src`; 0 símbolos `__darling_*`; el build define `-DOSXIE` (src/CMakeLists.txt:78), nunca `-DDARLING`.
  - ~30 archivos con ambos guards (dyld, libkqueue, foundation, objc4, libpthread, libplatform, ...) — auditar ramas divergentes.
  - Verificar remotes de submodulos (apuntan al fork del usuario o a Darling original).

## Fase 3 — Build completo + instalar
1. `make -C build-all -j$(nproc)`
2. `sudo -n true`; si falla, `pkexec make -C build-all install`
3. Commit + push (superproject + submodulos `src/external/security`, `src/external/JavaScriptCore`)
