# Built by .github/workflows/deploy.yml (context ., file Dockerfile) and pushed
# to Artifact Registry.
#
# A job image, not a server: the default command runs the program, which
# checks its own results and exits 0 only when every check passes. It will
# never satisfy a $PORT health check. Eigen 3.4 from Debian trixie. Eigen is
# header-only, so the runtime stage is a slim Debian image with just the
# binary, run as a non-root user.
FROM debian:trixie AS build
RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential cmake ninja-build libeigen3-dev \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt ./
COPY src ./src
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build \
 && install -D build/app /out/app

FROM debian:trixie-slim AS runtime
RUN useradd -r -u 10001 app
WORKDIR /app
ARG BUILD_ID=""
ENV BUILD_ID=$BUILD_ID
COPY --from=build /out/app /app/app
USER app
CMD ["/app/app"]
