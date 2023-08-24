# dta 

![gif](images/output.gif)

DTA is a pluggable framework for OP-TEE TAs (Trusted Applications) that enables them to run outside the secure world. This allows for the rapid launch of TA fuzzing with state-of-the-art fuzzers during the development phase.

## Targets

DTA is designed to run trusted user-land code outside the secure world. Its main targets are:

- [Trusted Apps](https://optee.readthedocs.io/en/latest/architecture/trusted_applications.html#pseudo-trusted-applications)
- [Trusted App Libraries](https://optee.readthedocs.io/en/latest/architecture/libraries.html)

## Getting Started

DTA is currently based on OP-TEE, an open-source reference implementation of TrustZone technology. To run our demo TAs, follow these instructions:

1. Build the complete OP-TEE project deployed for QEMU v8. (For detailed instructions, check the OP-TEE [documentation](https://optee.readthedocs.io/en/latest/building/devices/qemu.html#qemu-v8))

```
mkdir optee && cd optee
repo init -u https://github.com/OP-TEE/manifest.git -m qemu_v8.xml
repo sync
cd build
make toolchains && make run
```

2. Move the demo TA source codes from `optee_examples/` to the same path in the OP-TEE project directory.

3. Move the AFL++ buildroot package from `build/br-ext/package/` to the same path in the OP-TEE project directory.

4. Add the following line to `build/common.mk`.

```
# add this below BR2_PACKAGE_KEYUTILS ?= y (near line 314)
BR2_PACKAGE_AFLPLUSPLUS_EXT ?= y
```

5. Append the following line to the bottom of `build/br-ext/Config.in`

```
source "$BR2_EXTERNAL_OPTEE_PATH/package/aflplusplus_ext/Config.in"
```

6. Make and boot the system.

```
make run CFG_CORE_ASLR=n CFG_TA_ASLR=n
```

### Testing TAs

Three demo TAs will be installed in `/usr/bin/`.

- `dta_test`, `dta_test2` for testing secure world system calls.
- `test1` for testing fuzzers.
    - This TA panics when "abcd" is given as input.

All demo TAs require the `"-d"` argument to run outside the secure world. Without `"-d"`, they will act as normal TAs.

### Testing fuzzers

We use [AFL++ Frida mode](https://github.com/AFLplusplus/AFLplusplus/blob/stable/frida_mode/README.md) for test fuzzer. The `afl-fuzz` binary is already in the `/root` directory, and we can fuzz the `test1` demo TA as follows.

```
mkdir in && echo aaaa > in/aaaa
./afl-fuzz -O -i in -o out -D -- test1 -d
```

## Integration with TAs

If you want your TAs to run outside, you need to add and modify your project with our framework.

1. Add files from our demo TA source code to your project.

|files|destination|
|---|---|
|`ditto.*`, `memory.*`, `setup.*`, `syscall.*`|`host/`|
|`ditto_ta.c`, `include/ditto_ta.h`, `func_extended.*`|`ta/`|

2. Apply necessary changes to your project codes.

- **Host:** Call `setup()` once on init, use `ditto_invoke_command()` instead of `InvokeCommand()`. (check our `host/main.c`)
- **TA:** Replace your entrypoint function name to `__TA_InvokeCommandEntryPoint()`, and use ours as an alternative. (check our `ta/*_ta.c`)

3. Add our framework codes to makefiles.

- `host/Makefile:9`
```
OBJS = main.o setup.o ditto.o syscall.o memory.o
```
- `ta/sub.mk:3`
```
srcs-y += ditto_ta.c
srcs-y += func_extended.c
```
- `CMakeLists:3`
```
set (SRC host/main.c host/setup.c host/ditto.c host/syscall.c host/memory.c)
```

There are a few more subtle changes required for integration, such as adding `#include`s for our header files. Please also check our demo TA project codes, since they all originated from official examples of OP-TEE.

## Troubleshooting

- **TA initialization does not work.**
    - We use `CFG_CORE_ASLR=n`, `CFG_TA_ASLR=n` to disable ASLR in the secure world. However, some environments may load TAs at a different address than we use. In that case, check the base address in the secure world console and manually change the value of `ta_code_addr` in `host/setup.c`.