# Multi-Container Runtime

## Overview
This project implements a lightweight container runtime in C. It allows users to create, manage, and monitor multiple containers using basic Linux primitives such as `fork()`, `chroot()`, and process control.

The system simulates container behavior by isolating filesystem environments and managing processes independently.

---

## Features
- Start containers using custom root filesystems
- Run multiple containers simultaneously
- List active containers
- Stop running containers
- Maintain container metadata
- Logging of container lifecycle events

---

## System Architecture
The runtime consists of:

1. **User-space Engine (`engine.c`)**
   - Handles CLI commands (`start`, `ps`, `stop`)
   - Creates and manages container processes
   - Maintains container metadata

2. **Container Execution**
   - Uses `fork()` to create a child process
   - Uses `chroot()` to isolate filesystem
   - Uses `exec()` to run container command

3. **Tracking System**
   - Container details stored in `/tmp/containers.txt`

4. **Logging System**
   - Events stored in `runtime.log`

---

## Commands

### Start a container
```bash
sudo ./engine start <id> <rootfs> <command>
