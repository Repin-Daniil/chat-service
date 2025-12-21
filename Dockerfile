# --- Stage 1: Builder ---
FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg:v2.13 AS builder

WORKDIR /app

COPY . .

RUN make build-release

RUN make deps

# --- Stage 2: Runner ---
FROM ubuntu:22.04

WORKDIR /app

COPY --from=builder /app/build-release/chat_service .
COPY --from=builder /app/deps ./deps

COPY --from=builder /app/configs/static_config.yaml ./configs/static_config.yaml
COPY ./entrypoint.sh /app/entrypoint.sh

EXPOSE 8080
EXPOSE 8081

RUN chmod +x /app/entrypoint.sh

ENV LD_LIBRARY_PATH="/app/deps"

ENTRYPOINT ["./entrypoint.sh"]
CMD ["./chat_service", "--config", "./configs/static_config.yaml", "--config_vars", "./configs/config_vars.yaml"]
