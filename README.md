# DTA

![gif](images/output.gif)

**DTA** is a pluggable framework for OP-TEE Trusted Applications (TAs) that lets them run **outside** the secure world. This enables rapid TA fuzzing with state-of-the-art fuzzers during development.

## Targets

DTA is designed to run trusted user-land code outside the secure world. Primary targets:

* [Trusted Applications (TAs)](https://optee.readthedocs.io/en/latest/architecture/trusted_applications.html)
* [Trusted Application Libraries](https://optee.readthedocs.io/en/latest/architecture/libraries.html)

## Getting Started

DTA currently builds on **OP-TEE**, the open-source TrustZone reference implementation. To run the demo TAs:

1. **Build OP-TEE for QEMU v8**
   (See OP-TEE’s docs for details: the [QEMU v8 guide](https://optee.readthedocs.io/en/latest/building/devices/qemu.html#qemu-v8))

   ```
   mkdir optee && cd optee
   repo init -u https://github.com/OP-TEE/manifest.git -m qemu_v8.xml
   repo sync
   cd build
   make toolchains && make run
   ```

2. **Copy demo TA sources**
   Move the demo TA sources from `optee_examples/` into the same path within your OP-TEE project tree.

3. **Install the AFL++ Buildroot package**
   Move the AFL++ Buildroot package from `build/br-ext/package/` into the same path within your OP-TEE project tree.

4. **Enable the package in Buildroot**
   Append the following to `build/common.mk`:

   ```
   # add below BR2_PACKAGE_KEYUTILS ?= y (near line 314)
   BR2_PACKAGE_AFLPLUSPLUS_EXT ?= y
   ```

5. **Register the package in Config.in**
   Append to the bottom of `build/br-ext/Config.in`:

   ```
   source "$BR2_EXTERNAL_OPTEE_PATH/package/aflplusplus_ext/Config.in"
   ```

6. **Build and boot**
   (Disable ASLR in the secure world for consistency.)

   ```
   make run CFG_CORE_ASLR=n CFG_TA_ASLR=n
   ```

### Testing TAs

Three demo TAs are installed in `/usr/bin/`:

* `dta_test`, `dta_test2`: exercise secure-world syscalls
* `test1`: intended for fuzzing

  * Panics when the input is exactly `abcd`

All demo TAs accept `-d` to run **outside** the secure world. Without `-d`, they behave as normal TAs.

### Testing Fuzzers

We use [AFL++ Frida mode](https://github.com/AFLplusplus/AFLplusplus/blob/stable/frida_mode/README.md) for the demo. The `afl-fuzz` binary is available in `/root`.

Example fuzzing of `test1`:

```
mkdir in && echo aaaa > in/aaaa
./afl-fuzz -O -i in -o out -D -- test1 -d
```

## Integration with Your TAs

To run your own TAs outside the secure world, integrate the DTA framework into your project.

1. **Add framework files**

| Files                                                 | Destination |
| ----------------------------------------------------- | ----------- |
| `ditto.*`, `memory.*`, `setup.*`, `syscall.*`         | `host/`     |
| `ditto_ta.c`, `include/ditto_ta.h`, `func_extended.*` | `ta/`       |

2. **Apply code changes**

* **Host**: call `setup()` once at init; use `ditto_invoke_command()` instead of `InvokeCommand()` (see `host/main.c` in the demo).
* **TA**: rename your entry point to `__TA_InvokeCommandEntryPoint()` and use the provided implementation (see `ta/*_ta.c`).

3. **Update build files**

* `host/Makefile`:

  ```
  OBJS = main.o setup.o ditto.o syscall.o memory.o
  ```

* `ta/sub.mk`:

  ```
  srcs-y += ditto_ta.c
  srcs-y += func_extended.c
  ```

* `CMakeLists.txt`:

  ```
  set (SRC host/main.c host/setup.c host/ditto.c host/syscall.c host/memory.c)
  ```

> There are a few additional, straightforward changes (e.g., adding `#include` directives for DTA headers). Refer to the demo TA sources, which are adapted from OP-TEE’s official examples.

## Troubleshooting

* **TA initialization fails**
  We use `CFG_CORE_ASLR=n` and `CFG_TA_ASLR=n` to disable secure-world ASLR. Some environments may still place TAs at different base addresses. Check the secure-world console for the loaded TA base and update `ta_code_addr` in `host/setup.c` accordingly.

## Publication

```
@article{song2024dta,
  title     = {DTA: Run TrustZone TAs Outside the Secure World for Security Testing},
  author    = {Song, Juhyun and Jo, Eunji and Kim, Jaehyu},
  journal   = {IEEE Access},
  volume    = {12},
  pages     = {16715--16727},
  year      = {2024},
  publisher = {IEEE}
}
```
