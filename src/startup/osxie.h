/*
This file is part of Osxie.

Copyright (C) 2016 Lubos Dolezel

Osxie is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Osxie is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Osxie.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sys/types.h>

#ifndef _OSXIE_H_
#define _OSXIE_H_

void showHelp(const char* argv0);
void showVersion(const char* argv0);

// Reports that this executable is not setuid root
void missingSetuidRoot(void);

// Returns ~/.osxie with ~ expanded
char* defaultPrefixPath(void);

void setupWorkdir(void);

void setupPrefix(void);

int checkPrefixDir(void);

// Creates the given directory, exit()ing if not possible
void createDir(const char* path);

int connectToShellspawn(void);
void setupShellspawnEnv(int shellspawnFD);
void setupWorkingDir(int shellspawnFD);
void setupIDs(int shellspawnFD);
void setupFDs(int fds[3], int* master);
void spawnGo(int shellspawnFD, int fds[3], int master);
void spawnShell(const char** argv);
void spawnBinary(const char* binary, const char** argv);

// Set up some environment variables
// As well as the working directory
// Called in the child process
void setupChild(const char *curPath);

// Returns the PID of the init process in prefix (in our namespace)
// Returns 0 if no init is running
pid_t getInitProcess(void);

pid_t spawnInitProcess(void);

void putInitPid(pid_t pidInit);
void mapUids(pid_t pid);

void spawnLaunchd(void);
void osxiePreInit(void);

void checkPrefixOwner(void);

int isModuleLoaded(void);

void loadKernelModule(void);
void joinNamespace(pid_t pid, int type, const char* typeName);

void setupCoredumpPattern(void);
void setupUserHome(void);

#endif
