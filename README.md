# Eigen template

Provisioned from [`Qode-Fleet-Control/fleet-template-v1`](https://github.com/Qode-Fleet-Control/fleet-template-v1) — the fleet
lifecycle contract (`bin/`, `fleet.conf`, `compose.yaml`, deploy workflows) with a Eigen (linear algebra) starter laid on top.

A small linear-algebra program on Eigen 3.4 (Debian trixie, header-only), built with CMake, that checks its own answers: it solves a dense linear system (LU), fits a line by least squares (QR), eigen-decomposes a symmetric matrix and inverts a matrix, printing each result with the residual it checked. The exit code is the number of failed checks — 0 means every check passed — so the container's exit code is the job's result, and `ctest` runs the same program as a test.

## Origin

    hand-written (Eigen ships no project generator) — CMakeLists.txt as Eigen's "Using Eigen in CMake Projects" page teaches: find_package(Eigen3 3.3 REQUIRED NO_MODULE) + target_link_libraries(... Eigen3::Eigen)


## Run it

### On the fleet

The fleet runs it as containers (the docker runtime): `bin/run` builds the image with
`docker compose build` and then stops — this is a job, so `DOCKER_START_CMD` is empty and nothing listens on `$PORT`.

### With docker

```sh
docker compose build
docker compose run --rm app            # runs the job; exit code = result
```

### Without docker

```sh
# Debian/Ubuntu: sudo apt install build-essential cmake libeigen3-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
./build/app                     # or: ctest --test-dir build --output-on-failure
```

`fleet.conf` drives every script in `bin/`:

| step | docker runtime (fleet) | `FLEET_RUNTIME=process` |
|---|---|---|
| install | — | `(none)` |
| build | `docker compose build` | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j` |
| start | `(none — a job)` | `(none — a job)` |

## Layout

- `CMakeLists.txt` — one executable target `app` linked to `Eigen3::Eigen`; `add_test()` registers it with CTest.
- `src/main.cpp` — the four computations and the `check()` helper that counts failures.
- `Dockerfile` — `debian:trixie` build stage with `libeigen3-dev`; `debian:trixie-slim` runtime with just the binary (Eigen is header-only); non-root user `app`; `CMD ["/app/app"]`.
- `compose.yaml` — service `app`, no ports (a job), fleet variables passed through by name.

## Deviations from stock, and why

- Not generated: there is no generator to deviate from. The layout is the one Eigen's CMake page shows, plus an `add_test()` so `ctest` works.
- `PORT`, `HEALTH_PATH`, `START_CMD` and `DOCKER_START_CMD` are empty by design: nothing listens on a port, and `bin/run` builds the image and stops there.

## Verified

2026-10-05, Docker 29.8 on linux/amd64, from the scaffold directory:

- `docker compose build` → built.
- `docker compose run --rm app` → exit 0: `Eigen 3.4.0`, 7 × `PASS`, `All checks passed.`
- `docker compose down --rmi local -v` → clean.

The no-docker path (`FLEET_RUNTIME=process`) was not run on a host toolchain; it is the same CMake build the image runs.

See `docs/fleet-lifecycle.md` for the lifecycle contract.
