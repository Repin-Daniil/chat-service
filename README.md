# Чатик на userver
[![CI](https://github.com/Repin-Daniil/chat-service/actions/workflows/ci.yml/badge.svg)](
https://github.com/Repin-Daniil/chat-service/actions/workflows/ci.yml/
)

Сервер для многопользовательского онлайн-чатика на [userver framework](https://github.com/userver-framework/userver). 

- Есть три готовых дашборда в grafana. Метрики, скрины дашбордов [тут](docs/metrics.md)

- Проводился [нагрузочный тест](docs/load-testing.md). Тянуло 30 000 RPS (уперлись в сеть ВМ), на 10k RPS 98 квантиль времени отправки был 3мс.

- Работает через long polling, API [вот](docs/api.yaml), аутентификация по JWT

- Поддержан динамический конфиг, вот [пример](configs/dynamic_config_fallback.json). Можно крутить максимальное время поллинга, количество открытых сессий, времени жизни очередей, rate limiter и так далее. 

### Принцип работы
Для получения сообщений клиент создает на сервере сессию, которую будет в дальнейшем поллить. Сессия — по факту `lock-free` очередь (MPSC), которую наполняют другие пользователи. Сессии добавляются в `Registry`, откуда их можно достать, чтобы получить или отправить сообщения. Старые сессии удаляются фоновым процессом, который периодически делает обход и заодно снимает метрики. 

В качестве структуры данных для `Registry` была реализована ShardedMap — шардированная мапа, где шард — это `std::unordered_map` с `SharedMutex`, что позволяет блокироваться не всех пользователей, а только один шард, читатели же в пределах одного шарда не конкурируют. `ShardedMap` хранит отображение `user_id` в "почтовый ящик" пользователя, который в себе инкапсулирует набор пользователських сессий. Их может быть максимум 5 (крутится через дин конфиг), поэтому используется `FlatMap`, защищенная через `rcu::Variable`. Один пользователь может иметь несколько сессий, допустим, с разных страниц браузере или устройств.

<img width="642" height="805" alt="image" src="https://github.com/user-attachments/assets/293d3725-a550-4e39-b7eb-4714d5873bfd" />


### Тесты 
- 104 функциональных тестов на Python (`testsuite`)
- 382 модульных и интеграционных тестов на `gtest`
- 3 бенчмарка на шардированную мапу, MPSC-очередь и `rcu<flat_map>` (хранилище сессий)


- Registry делает сервис statefull, поэтому была сделана отдельная таска доступная в тестах для сброса состояния сервиса
- Для удобной правки в тесте динамических конфигов были реализованы фикстурки `*_config`


### Пример работы
Небольшой ~~навайбкоденный~~ фронтендик есть

https://github.com/user-attachments/assets/14d27043-c88d-4a8a-b7b0-f8b2bc0e4faa


### Система в целом
В качестве сервиса динконфигов можно поднять [uservice-dynconf](https://github.com/userver-framework/uservice-dynconf). Остальное материализуется само через
```bash
docker compose up
```

<img width="664" height="579" alt="image" src="https://github.com/user-attachments/assets/1fbd59c7-4589-45a8-b713-0ef757f40fbb" />

### База данных
<img width="2092" height="1242" alt="image" src="https://github.com/user-attachments/assets/9c9b58d5-0cff-4ef6-bab8-7d2c6300d30a" />



## Makefile

`PRESET` is either `debug`, `release`

* `make cmake-PRESET` - run cmake configure, update cmake options and source file lists
* `make build-PRESET` - build the service
* `make test-PRESET` - build the service and run all tests
* `make start-PRESET` - build the service, start it in testsuite environment and leave it running
* `make install-PRESET` - build the service and install it in directory set in environment `PREFIX`
* `make` or `make all` - build and run all tests in `debug` and `release` modes
* `make format` - reformat all C++ and Python sources
* `make dist-clean` - clean build files and cmake cache
* `make docker-COMMAND` - run `make COMMAND` in docker environment
* `make docker-clean-data` - stop docker containers

## Debug
Add in CMakePresets.json "USERVER_DEBUG_INFO_COMPRESSION": "none"
