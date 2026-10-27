# Contribution regression probes

Fork-only validation for three independent RetroPie/EmulationStation PRs. These files and CI workflow are outside their production diffs.

The probes link the real CMake-built objects, replacing main with GNU ld --wrap=main. The input probe observes Window state through a test-only private/public include; no production test hook is added. The splash probe counts actual NanoSVG parser calls via --wrap=nsvgParse. Temporary homes isolate configuration writes. Files named .nes are inert text fixtures; no ROM, emulator, BIOS or save is used.

Build from source/ into build/ with CMAKE_EXPORT_COMPILE_COMMANDS=ON. Copy these files into validation/ alongside those directories. Example baseline validation:

```sh
python3 validation/build-probes.py "$PWD" baseline axis discovery splash
python3 validation/run-headless.py "$PWD" baseline axis
python3 validation/run-headless.py "$PWD" baseline discovery
mkdir -p fixture-home/.emulationstation
xvfb-run -a results/baseline/splash "$PWD/fixture-home" baseline
```

Use policy axis for fix/screensaver-axis and its axis probe; policy empty for fix/empty-system-themes and its discovery probe; policy cache for fix/loading-splash-cache and its splash probe (pass cached to the executable).

Discovery runs both ThreadedLoading values; more than two hardware threads are needed to exercise the parallel path. These tests do not establish thread-pool race freedom.

Rendering uses stock splash.svg, three 12-frame loading phases with a regular frame between them, followed by real context teardown/recreation at 1280x720. VSync is disabled. Reported timings are loading-redraw microbenchmarks, not boot times.
