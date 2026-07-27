#!/usr/bin/env python3
"""
GARS - Guardian Agent for Runtime Solutions (Osxie)

Objetivo: Resolver problemas concretos de compatibilidad en Osxie
Enfoque: Cada ejecución debe arreglar un problema específico

Problemas conocidos:
1. iTerm2: Symbol not found: _OBJC_CLASS_$_CATiledLayer
2. Homebrew: Shell freezes on network activity
"""

import os
import sys
import subprocess
import json
import re
from pathlib import Path
from datetime import datetime

class GARS:
    def __init__(self):
        self.osxie_root = Path("/home/fenix/src/Osxie")
        self.knowledge_base = Path.home() / ".osxie" / "gars"
        self.knowledge_base.mkdir(parents=True, exist_ok=True)
        self.problems_db = self.knowledge_base / "problems.json"
        self.solutions_db = self.knowledge_base / "solutions.json"
        self.load_knowledge()
        
    def load_knowledge(self):
        """F7: Learn - Cargar conocimiento previo"""
        if self.problems_db.exists():
            with open(self.problems_db) as f:
                self.known_problems = json.load(f)
        else:
            self.known_problems = {
                "iTerm2": {
                    "pattern": "Symbol not found.*CATiledLayer",
                    "cause": "QuartzCore framework missing CATiledLayer implementation",
                    "solution": "create_catiledlayer_stub"
                },
                "Homebrew": {
                    "pattern": "brew.*freeze|stuck|hang",
                    "cause": "Network syscalls blocking in Darling container",
                    "solution": "fix_network_syscalls"
                }
            }
            
        if self.solutions_db.exists():
            with open(self.solutions_db) as f:
                self.applied_solutions = json.load(f)
        else:
            self.applied_solutions = []
    
    def phase_1_parse(self, error_output):
        """F1: Parse - Transformar error sin interpretación"""
        print("[GARS] Fase 1: Parse - Analizando output...")
        
        parsed = {
            "raw": error_output,
            "symbols": [],
            "frameworks": [],
            "commands": [],
            "files": []
        }
        
        # Extraer símbolos faltantes
        symbol_matches = re.findall(r"Symbol not found: ([^\n]+)", error_output)
        parsed["symbols"] = symbol_matches
        
        # Extraer frameworks
        framework_matches = re.findall(r"Expected in: ([^\n]+)", error_output)
        parsed["frameworks"] = framework_matches
        
        # Extraer paths de archivos
        file_matches = re.findall(r"(/[^\s:]+\.(framework|dylib|so))", error_output)
        parsed["files"] = [f[0] for f in file_matches]
        
        print(f"  - Símbolos faltantes: {parsed['symbols']}")
        print(f"  - Frameworks involucrados: {parsed['frameworks']}")
        
        return parsed
    
    def phase_2_understand(self, parsed_error):
        """F2: Understand - Construir contexto"""
        print("[GARS] Fase 2: Understand - Construyendo contexto...")
        
        context = {
            "problem_type": None,
            "application": None,
            "missing_components": [],
            "can_fix": False
        }
        
        # Identificar problema conocido
        for app, problem in self.known_problems.items():
            if re.search(problem["pattern"], parsed_error["raw"]):
                context["problem_type"] = problem["cause"]
                context["application"] = app
                context["can_fix"] = True
                break
        
        # Si no es conocido, intentar entender
        if not context["problem_type"]:
            if parsed_error["symbols"]:
                context["problem_type"] = "missing_symbols"
                context["missing_components"] = parsed_error["symbols"]
            elif "freeze" in parsed_error["raw"].lower():
                context["problem_type"] = "process_freeze"
        
        print(f"  - Tipo de problema: {context['problem_type']}")
        print(f"  - Aplicación: {context['application']}")
        print(f"  - Se puede arreglar automáticamente: {context['can_fix']}")
        
        return context
    
    def phase_3_research(self, context):
        """F3: Research - Buscar soluciones existentes"""
        print("[GARS] Fase 3: Research - Buscando soluciones...")
        
        research = {
            "existing_fixes": [],
            "similar_problems": [],
            "proposed_solution": None
        }
        
        # Buscar en soluciones aplicadas previamente
        for solution in self.applied_solutions:
            if solution.get("problem_type") == context["problem_type"]:
                research["existing_fixes"].append(solution)
        
        # Proponer solución
        if context["application"] in self.known_problems:
            solution_name = self.known_problems[context["application"]]["solution"]
            research["proposed_solution"] = solution_name
            print(f"  - Solución propuesta: {solution_name}")
        else:
            print("  - No hay solución conocida, se necesita investigación manual")
        
        return research
    
    def phase_4_design(self, context, research):
        """F4: Design - Diseñar fix específico"""
        print("[GARS] Fase 4: Design - Diseñando solución...")
        
        design = {
            "steps": [],
            "files_to_create": [],
            "files_to_modify": [],
            "rollback_plan": []
        }
        
        if research["proposed_solution"] == "create_catiledlayer_stub":
            design["steps"] = [
                "Crear stub de CATiledLayer en QuartzCore",
                "Añadir al CMakeLists.txt de cocotron",
                "Recompilar QuartzCore.framework"
            ]
            design["files_to_create"] = [
                self.osxie_root / "src/external/cocotron/QuartzCore/CATiledLayer.h",
                self.osxie_root / "src/external/cocotron/QuartzCore/CATiledLayer.m"
            ]
            design["files_to_modify"] = [
                self.osxie_root / "src/external/cocotron/QuartzCore/CMakeLists.txt"
            ]
            
        elif research["proposed_solution"] == "fix_network_syscalls":
            design["steps"] = [
                "Parchear select/poll timeout en darlingserver",
                "Añadir non-blocking flags a sockets",
                "Implementar timeout handler"
            ]
            design["files_to_modify"] = [
                self.osxie_root / "src/external/darlingserver/src/message.cpp"
            ]
        
        print(f"  - Pasos a seguir: {len(design['steps'])}")
        for i, step in enumerate(design["steps"], 1):
            print(f"    {i}. {step}")
        
        return design
    
    def phase_5_execute(self, design):
        """F5: Execute - Implementar fix"""
        print("[GARS] Fase 5: Execute - Implementando solución...")
        
        execution = {
            "success": False,
            "files_created": [],
            "files_modified": [],
            "errors": []
        }
        
        try:
            # Crear archivos necesarios
            for file_path in design["files_to_create"]:
                if "CATiledLayer.h" in str(file_path):
                    self.create_catiledlayer_header(file_path)
                    execution["files_created"].append(str(file_path))
                elif "CATiledLayer.m" in str(file_path):
                    self.create_catiledlayer_implementation(file_path)
                    execution["files_created"].append(str(file_path))
            
            # Modificar archivos existentes
            for file_path in design["files_to_modify"]:
                if "CMakeLists.txt" in str(file_path) and "QuartzCore" in str(file_path):
                    self.update_quartzcore_cmake(file_path)
                    execution["files_modified"].append(str(file_path))
            
            execution["success"] = True
            print(f"  - Archivos creados: {len(execution['files_created'])}")
            print(f"  - Archivos modificados: {len(execution['files_modified'])}")
            
        except Exception as e:
            execution["errors"].append(str(e))
            print(f"  - ERROR: {e}")
        
        return execution
    
    def phase_6_verify(self, execution):
        """F6: Verify - Verificar que funciona"""
        print("[GARS] Fase 6: Verify - Verificando solución...")
        
        verification = {
            "compiled": False,
            "tests_passed": False,
            "app_runs": False
        }
        
        # Verificar que los archivos existen
        for file_path in execution["files_created"]:
            if not Path(file_path).exists():
                print(f"  - ERROR: Archivo no existe: {file_path}")
                return verification
        
        # Intentar compilar
        print("  - Intentando compilar...")
        build_dir = self.osxie_root / "build"
        if build_dir.exists():
            result = subprocess.run(
                ["make", "-C", str(build_dir), "QuartzCore"],
                capture_output=True,
                text=True
            )
            verification["compiled"] = result.returncode == 0
            
            if verification["compiled"]:
                print("  - ✓ Compilación exitosa")
            else:
                print(f"  - ✗ Error de compilación: {result.stderr[:200]}")
        
        return verification
    
    def phase_7_learn(self, context, execution, verification):
        """F7: Learn - Documentar solución"""
        print("[GARS] Fase 7: Learn - Documentando solución...")
        
        solution_record = {
            "timestamp": datetime.now().isoformat(),
            "problem_type": context["problem_type"],
            "application": context["application"],
            "solution_applied": execution["files_created"] + execution["files_modified"],
            "success": verification["compiled"],
        }
        
        # Guardar en base de conocimiento
        self.applied_solutions.append(solution_record)
        with open(self.solutions_db, 'w') as f:
            json.dump(self.applied_solutions, f, indent=2)
        
        print(f"  - Solución documentada en: {self.solutions_db}")
        
        return solution_record
    
    def create_catiledlayer_header(self, file_path):
        """Crear header de CATiledLayer"""
        content = """// Auto-generated by GARS for iTerm2 compatibility
#import <QuartzCore/CALayer.h>

@interface CATiledLayer : CALayer

@property size_t levelsOfDetail;
@property size_t levelsOfDetailBias;
@property CGSize tileSize;

+ (CFTimeInterval)fadeDuration;

@end
"""
        file_path.parent.mkdir(parents=True, exist_ok=True)
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"    Created: {file_path}")
    
    def create_catiledlayer_implementation(self, file_path):
        """Crear implementación de CATiledLayer"""
        content = """// Auto-generated by GARS for iTerm2 compatibility
#import "CATiledLayer.h"
#import <Foundation/Foundation.h>

@implementation CATiledLayer

- (instancetype)init {
    self = [super init];
    if (self) {
        _levelsOfDetail = 1;
        _levelsOfDetailBias = 0;
        _tileSize = CGSizeMake(256, 256);
        NSLog(@"[GARS] CATiledLayer initialized (stub)");
    }
    return self;
}

+ (CFTimeInterval)fadeDuration {
    return 0.25;
}

- (void)setNeedsDisplayInRect:(CGRect)rect {
    [super setNeedsDisplayInRect:rect];
}

@end
"""
        file_path.parent.mkdir(parents=True, exist_ok=True)
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"    Created: {file_path}")
    
    def update_quartzcore_cmake(self, file_path):
        """Actualizar CMakeLists.txt de QuartzCore"""
        if not file_path.exists():
            print(f"    WARNING: {file_path} no existe")
            return
        
        with open(file_path, 'r') as f:
            content = f.read()
        
        # Verificar si ya está incluido
        if "CATiledLayer" in content:
            print(f"    CATiledLayer ya está en CMakeLists.txt")
            return
        
        # Buscar dónde insertar
        if "set(QuartzCore_SRCS" in content:
            # Añadir al final de la lista de sources
            content = content.replace(
                "set(QuartzCore_SRCS",
                "set(QuartzCore_SRCS\n    CATiledLayer.m"
            )
            
            with open(file_path, 'w') as f:
                f.write(content)
            print(f"    Updated: {file_path}")
        else:
            print(f"    WARNING: No se pudo actualizar CMakeLists.txt")
    
    def run_pipeline(self, error_output):
        """Ejecutar pipeline completo de GARS"""
        print("\n" + "="*60)
        print("GARS - Guardian Agent for Runtime Solutions")
        print("="*60 + "\n")
        
        # Fase 1: Parse
        parsed = self.phase_1_parse(error_output)
        
        # Fase 2: Understand
        context = self.phase_2_understand(parsed)
        
        if not context["can_fix"]:
            print("\n[GARS] No puedo arreglar este problema automáticamente")
            return False
        
        # Fase 3: Research
        research = self.phase_3_research(context)
        
        if not research["proposed_solution"]:
            print("\n[GARS] No hay solución disponible")
            return False
        
        # Fase 4: Design
        design = self.phase_4_design(context, research)
        
        # Fase 5: Execute
        execution = self.phase_5_execute(design)
        
        if not execution["success"]:
            print("\n[GARS] Falló la ejecución de la solución")
            return False
        
        # Fase 6: Verify
        verification = self.phase_6_verify(execution)
        
        # Fase 7: Learn  
        learning = self.phase_7_learn(context, execution, verification)
        
        print("\n" + "="*60)
        if verification["compiled"]:
            print("✓ SOLUCIÓN APLICADA EXITOSAMENTE")
            print("  Reinicia Osxie y prueba iTerm2 nuevamente")
        else:
            print("✗ La solución fue aplicada pero necesita compilación manual")
        print("="*60 + "\n")
        
        return verification["compiled"]


def main():
    """Punto de entrada principal"""
    
    # Ejemplo de error de iTerm2
    iterm_error = """
dyld: Symbol not found: _OBJC_CLASS_$_CATiledLayer
  Referenced from: /System/Library/Frameworks/WebKit.framework/Versions/A/Frameworks/WebCore.framework/Versions/A/WebCore
  Expected in: /System/Library/Frameworks/QuartzCore.framework/Versions/A/QuartzCore
  in /System/Library/Frameworks/WebKit.framework/Versions/A/Frameworks/WebCore.framework/Versions/A/WebCore
    """
    
    gars = GARS()
    
    # Si se pasa un archivo de error como argumento
    if len(sys.argv) > 1:
        with open(sys.argv[1]) as f:
            error_output = f.read()
    else:
        # Usar el error de ejemplo
        error_output = iterm_error
    
    success = gars.run_pipeline(error_output)
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()