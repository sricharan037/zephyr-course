# Assignment 4 - Heartbeat LED (Kconfig + Devicetree Overlay)

**Lecture 04 homework - tag `l4-task1`**
Board used here: `esp32_devkitc/esp32/procpu` (ESP32-DevKitC, ESP32 PROCPU)

---

## 1. What the assignment asks for

Straight from the lecture (Lecture 04, slide 21):

> Add a configurable heartbeat LED to the blinky app from the demo. Works on any board.
> - Create `app.overlay` - add alias `app-led` pointing to your board's `led0`
> - Add `Kconfig` file with `int APP_HEARTBEAT_PERIOD_MS` (default `500`, range `100`-`2000`)
> - In C: use `DT_ALIAS(app_led)` for the GPIO and `CONFIG_APP_HEARTBEAT_PERIOD_MS`
>   for the sleep duration
> - Verify: open menuconfig, change the period, rebuild - LED blink speed must change
> - Push tag: `l4-task1`

So there are four deliverables:

1. `app/app.overlay` - the hardware half, described in Devicetree.
2. `APP_HEARTBEAT_PERIOD_MS` in `app/Kconfig` - the software half, described in Kconfig.
3. `app/src/main.cpp` - uses both, without hardcoding a GPIO or a delay.
4. A build that works, plus the `l4-task1` tag.

## 2. The idea: Kconfig vs Devicetree

Assignment 3 was pure Kconfig, assignment 2 was pure Devicetree. Assignment 4 is the
first one that uses **one of each on purpose**, so the same compiled application can be
re-targeted two different ways without editing any C code.

|             | Kconfig                    | Devicetree                      |
| ----------- | -------------------------- | ------------------------------- |
| Purpose     | Software features          | Hardware description            |
| Question    | "Do I want this feature?"  | "Where is this hardware?"       |
| Example     | `CONFIG_LOG=y`             | `zephyr,console = &uart0`       |
| Files       | `.conf`, `Kconfig`         | `.dts`, `.overlay`              |
| In C        | `CONFIG_*` macros          | `DT_*` macros                   |

Concretely, in this assignment:

- **Which pin the LED is on** is a hardware fact -> Devicetree (an overlay + an alias).
- **How fast it blinks** is an application preference -> Kconfig.

## 3. Files touched (and why)

| File                                             | What it does here |
| ------------------------------------------------ | ----------------- |
| `app/app.overlay`                              | **NEW.** Declares the LED node (`compatible = "gpio-leds"`), the node label `led0`, and the aliases `led0` and `app-led`. This is the hardware half. |
| `app/Kconfig`                                  | **MODIFIED.** Already existed from assignment 3 (`LED_SUBSYSTEM`, blink-sleep choice). `config APP_HEARTBEAT_PERIOD_MS` was appended at top level. |
| `app/src/main.cpp`                             | **MODIFIED.** Now takes its GPIO from `DT_ALIAS(app_led)` and its delay from `CONFIG_APP_HEARTBEAT_PERIOD_MS`. |
| `app/prj.conf`                                 | Unchanged. It *enables drivers* (`CONFIG_GPIO=y`, `CONFIG_LOG=y`); a `Kconfig` file only *declares* symbols, it does not enable them. |
| `app/CMakeLists.txt`                           | Unchanged. Still compiles `src/main.cpp`. |
| `app/boards/esp32_devkitc_esp32_procpu.overlay` | **DELETED.** Its contents moved into `app.overlay` - see the next section. |

### The trap: which overlay does Zephyr actually load?

When `DTC_OVERLAY_FILE` is not set on the command line, Zephyr 4.2 looks for overlays in
this order (see `zephyr/cmake/modules/configuration_files.cmake` and the docs page
"Devicetree HOWTOs -> Set devicetree overlays"):

1. `app/socs/<SOC>_<BOARD_QUALIFIERS>.overlay`
2. `app/boards/<BOARD>.overlay`
3. `app/boards/<BOARD>_<revision>.overlay`
4. if any of 1-3 was found -> **stop looking**
5. otherwise `app/<BOARD>.overlay`
6. otherwise `app/app.overlay`

The important part is step 4: it **stops at the first match**. So while the assignment-2
file `app/boards/esp32_devkitc_esp32_procpu.overlay` existed, a newly created
`app/app.overlay` would have been *silently ignored* - the alias `app-led` would never
reach the build, and `DT_ALIAS(app_led)` would fail to compile.

That is why the LED description was **moved** from the board overlay into `app.overlay`
instead of being duplicated: one file, one source of truth, and the plain
`west build` command loads it.

## 4. How the pieces connect

Everything below is generated at build time - nothing is hardcoded in C.

**Devicetree chain (which LED):**

```
app/app.overlay
    app-led = &led0;                              <- alias used by the app
    led0: led_0 { gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>; }
        |
        v   (build/zephyr/include/generated/zephyr/devicetree_generated.h)
    #define DT_N_ALIAS_app_led  DT_N_S_leds_S_led_0
        |
        v   (app/src/main.cpp)
    #define LED_NODE DT_ALIAS(app_led)
    static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
        |            -> port &gpio0, pin 2, GPIO_ACTIVE_HIGH
        v
    gpio_pin_toggle_dt(&led)
```

Note the naming: a devicetree alias written as `app-led` becomes the C macro
`DT_ALIAS(app_led)` - dashes turn into underscores.

**Kconfig chain (how fast):**

```
app/Kconfig
    config APP_HEARTBEAT_PERIOD_MS   (int, default 500, range 100 2000)
        |
        v   (build/zephyr/.config)
    CONFIG_APP_HEARTBEAT_PERIOD_MS=500
        |
        v   (build/zephyr/include/generated/zephyr/autoconf.h)
    #define CONFIG_APP_HEARTBEAT_PERIOD_MS 500
        |
        v   (app/src/main.cpp)
    k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
```

The three files that matter:

```dts
/* app/app.overlay */
/ {
    aliases {
        led0 = &led0;
        app-led = &led0;
    };
    leds {
        compatible = "gpio-leds";
        led0: led_0 {
            gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>;
        };
    };
};
```

```kconfig
# app/Kconfig (appended at top level, assignment 3 symbols left untouched)
config APP_HEARTBEAT_PERIOD_MS
	int "Heartbeat LED period (ms)"
	default 500
	range 100 2000
```

```c
/* app/src/main.cpp */
#define LED_NODE DT_ALIAS(app_led)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

while (1) {
    if (gpio_pin_toggle_dt(&led) < 0) return 0;
    LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
    k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
}
```

## 5. How to build

```bash
cd C:\zephyr-workspace
west build -p always -b esp32_devkitc/esp32/procpu zephyr-course/app
```

- `-p always` = pristine: wipes `build/` and re-runs CMake. Use it after changing
  overlays or Kconfig; afterwards a plain `west build` is enough for code edits.
- A fresh shell also needs the toolchain variables, otherwise CMake stops in
  `FindZephyr-sdk.cmake` with `Unknown arguments specified`:

```bat
set ZEPHYR_TOOLCHAIN_VARIANT=zephyr
set ZEPHYR_SDK_INSTALL_DIR=C:\zephyr-workspace\zephyr-sdk-0.17.2
```

Flash and watch the console (the DevKitC's USB-serial is a CP2102, 115200 baud):

```bash
west flash
west espressif monitor          # or any 115200 baud serial terminal
```

## 6. Expected output

### 6.1 Build log (the line that proves the overlay was used)

```
-- Application: C:/zephyr-workspace/zephyr-course/app
-- Board: esp32_devkitc, qualifiers: esp32/procpu
-- Found BOARD.dts: .../boards/espressif/esp32_devkitc/esp32_devkitc_procpu.dts
-- Found devicetree overlay: C:/zephyr-workspace/zephyr-course/app/app.overlay
-- Generated zephyr.dts: C:/zephyr-workspace/build/zephyr/zephyr.dts
...
[216/216] Linking CXX executable zephyr\zephyr.elf

Memory region         Used Size  Region Size  %age Used
           FLASH:      148452 B    4194048 B      3.54%
     iram0_0_seg:       38400 B       224 KB     16.74%
     dram0_0_seg:       20320 B       192 KB     10.34%
     irom0_0_seg:       17380 B     11456 KB      0.15%
     drom0_0_seg:         64 KB         4 MB      1.56%

esptool.py v4.8.1
Creating esp32 image...
Successfully created esp32 image.
```

Two things to look for:

1. `Found devicetree overlay: .../app/app.overlay` - if this line is missing,
   `app.overlay` is being ignored (see section 3).
2. `Successfully created esp32 image.` and `zephyr.elf`/`zephyr.bin` in
   `build/zephyr/`. The exact FLASH/RAM numbers move slightly with the log level.

### 6.2 Merged devicetree - `build/zephyr/zephyr.dts`

This is the "verify your overlay was correctly merged" tip from slide 14. Search for
`app-led`:

```dts
    aliases {
        led0 = &led0;      /* in zephyr-course\app\app.overlay:3 */
        app-led = &led0;   /* in zephyr-course\app\app.overlay:4 */
        ...
    };

    leds {
        compatible = "gpio-leds";
        led0: led_0 {
            gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>;
        };
    };
```

### 6.3 Generated header - `build/zephyr/include/generated/zephyr/devicetree_generated.h`

```c
#define DT_N_ALIAS_led0     DT_N_S_leds_S_led_0
#define DT_N_ALIAS_app_led  DT_N_S_leds_S_led_0
```

Both aliases resolve to the same node, which is what makes `DT_ALIAS(app_led)` in
`main.cpp` compile.

### 6.4 Kconfig results

```
build/zephyr/.config                                ->  CONFIG_APP_HEARTBEAT_PERIOD_MS=500
build/zephyr/include/generated/zephyr/autoconf.h    ->  #define CONFIG_APP_HEARTBEAT_PERIOD_MS 500
```

`500` is there without anybody setting it, because it is the symbol's `default`.

### 6.5 What you see on the board

Serial console (`LOG_INF` fires once per toggle):

```
*** Booting Zephyr OS build v4.2.0 ***
[00:00:00.000,000] <inf> main: LED state: ON
[00:00:00.500,000] <inf> main: LED state: OFF
[00:00:01.000,000] <inf> main: LED state: ON
[00:00:01.500,000] <inf> main: LED state: OFF
```

Physical LED: toggles every 500 ms -> on for 0.5 s, off for 0.5 s, i.e. one full
on/off cycle per second (1 Hz). The "expected output" of this assignment is exactly
this rhythm, and - more importantly - that the rhythm follows the Kconfig value
instead of a hardcoded constant.

## 7. Proving it: change the period, rebuild, blink changes

This is the assignment's own acceptance test. Two ways to do it.

**A. Interactively (what the slides describe)**

```bash
cd C:\zephyr-workspace
west build -t menuconfig      # runs on the build dir produced by section 5
```

`app/Kconfig` is the Kconfig root for this app (Zephyr uses
`${APPLICATION_SOURCE_DIR}/Kconfig` when it exists, which is why the file ends with
`source "Kconfig.zephyr"`), so the symbol appears in menuconfig's **main menu** as
`Heartbeat LED period (ms)`. Type a new value (for example `1200`), save, exit, then:

```bash
west build -b esp32_devkitc/esp32/procpu zephyr-course/app
west flash
```

**B. Non-interactively (same result, scriptable)**

Put `CONFIG_APP_HEARTBEAT_PERIOD_MS=1200` in a small conf fragment *outside* the
repository (for example `C:\temp\hb.conf`) and pass it as an extra config file:

```bash
west build -p always -b esp32_devkitc/esp32/procpu zephyr-course/app \
    -- -DEXTRA_CONF_FILE=C:/temp/hb.conf
```

**Expected result of either route**

```
build/zephyr/.config                              -> CONFIG_APP_HEARTBEAT_PERIOD_MS=1200
build/zephyr/include/generated/zephyr/autoconf.h  -> #define CONFIG_APP_HEARTBEAT_PERIOD_MS 1200
```

and on the board the LED now toggles every 1.2 s instead of 0.5 s, with the serial
lines 1.2 s apart. Verified on this setup: with `1200` the build returns `0`, produces
`zephyr.elf` + `zephyr.bin`, and `.config` carries `CONFIG_APP_HEARTBEAT_PERIOD_MS=1200`.

The `range 100 2000` in `Kconfig` is what rejects silly values - try `5000` and Kconfig
will clamp it back into range instead of accepting it.

> Note: this implementation is a *periodic toggle* (on 500 ms / off 500 ms at the
> default). That is what the assignment checks. If you want a classic "heartbeat"
> double-blink pattern (blink-blink ... pause), it is an extra loop inside the
> `while (1)`, but the period still has to come from
> `CONFIG_APP_HEARTBEAT_PERIOD_MS`.

## 8. Troubleshooting

| Symptom | Cause | Fix |
| ------- | ----- | --- |
| Compile error around `DT_ALIAS(app_led)` / "alias not defined" | `app.overlay` is being ignored because `app/boards/<BOARD>.overlay` also exists (auto-discovery stops at the first match) | keep exactly one of them - this repo keeps `app.overlay` |
| CMake Error in `FindZephyr-sdk.cmake:57`: `Unknown arguments specified` | `ZEPHYR_TOOLCHAIN_VARIANT` / `ZEPHYR_SDK_INSTALL_DIR` are not set in that shell | set both (section 5) |
| Symbol not visible in menuconfig | `app/Kconfig` is missing, or it does not end with `source "Kconfig.zephyr"` | restore the `source` line; it is the Kconfig root, not an include |
| `CONFIG_APP_HEARTBEAT_PERIOD_MS` undefined in C even though Kconfig shows it | stale `.config`/`autoconf.h` | rebuild pristine (`-p always`) |
| Build log shows `Found BOARD.dts` but no `Found devicetree overlay` | overlay file not found, misnamed, or shadowed | check the name and the two rules in section 3 |
| LED never changes state | wrong pin or polarity for your wiring | edit `gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>` (pin `2`, or `GPIO_ACTIVE_LOW` for active-low LEDs) |
| `west flash` cannot find the board | CP2102 driver / COM port | install the CP210x driver (already downloaded in the workspace as `cp210x.zip`) and check `west flash --context` |

## 9. Submission checklist

- [ ] `app/app.overlay` exists and defines the `app-led` alias (plus the LED node).
- [ ] No `app/boards/<BOARD>.overlay` fighting it for auto-discovery.
- [ ] `app/Kconfig` declares `int APP_HEARTBEAT_PERIOD_MS`, `default 500`, `range 100 2000`.
- [ ] `app/src/main.cpp` uses `DT_ALIAS(app_led)` and `CONFIG_APP_HEARTBEAT_PERIOD_MS`
      - no hardcoded pin numbers, no hardcoded delays.
- [ ] `west build -p always -b esp32_devkitc/esp32/procpu zephyr-course/app` succeeds
      and prints `Found devicetree overlay: .../app/app.overlay`.
- [ ] Changing the period in menuconfig and rebuilding visibly changes the blink speed.
- [ ] Commit the three files and `git tag l4-task1`, then push the tag.
- [ ] Delete this README before pushing - it is a study aid, not part of the deliverable.

### Files to commit

```
app/app.overlay        (new)
app/Kconfig            (modified: APP_HEARTBEAT_PERIOD_MS)
app/src/main.cpp       (modified: DT_ALIAS(app_led) + CONFIG_APP_HEARTBEAT_PERIOD_MS)
app/boards/esp32_devkitc_esp32_procpu.overlay   (deleted - merged into app.overlay)
```


