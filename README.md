# Чатик на userver
Сервер для многопользовательского онлайн-чатика на [userver framework](https://github.com/userver-framework/userver) без сохранения сообщений


### Принцип работы
Для получения сообщений клиент создает на сервере сессию, которую будет в дальнейшем поллить. Сессия — по факту lock-free очередь (MPSC), которую наполняют другие пользователи. Сессии добавляются в `Registry`, откуда их можно достать, чтобы получить или отправить сообщения. Старые сессии удаляются фоновым процессом, который периодически делает обход и заодно снимает метрики. 

В качестве структуры данных внутри Registry была выбрана и реализована ShardedMap — шардированная мапа, где каждый шард — это unordered_map с SharedMutex, таким образом блокируется не вся мапа, а только один шард, читатели друг с другом не конкурируют. ShardedMap хранит отображение `user_id` в "почтовый ящик" пользователя, который в себе инкапсулирует набор пользователських сессий. Их может быть максимум 5 (крутится через дин конфиг), поэтому используется FlatMap, защищенная через rcu::Variable. Один пользователь может иметь несколько сессий, допустим, с разных страниц браузере или устройств.

Для всех этих действий требуется авторизация, используется JWT, реализованы CRUD операции для сущности user.

### Конфигурация
Есть динамическая конфигурация сервиса на лету через [uservice-dynconf](https://github.com/userver-framework/uservice-dynconf), вот тут лежит пример [конфига](configs/dynamic_config_fallback.json). Настройка максимального времени поллинга, количества открытых сессий, времени жизни очередей, rate limiter и так далее.

### API
Пока поддержано получение сообщений через long-polling по http, спецификация к ручкам лежит [тут](docs/api.yaml)

### Пример работы
Небольшой ~~навайбкоденный~~ фронтендик есть

https://github.com/user-attachments/assets/14d27043-c88d-4a8a-b7b0-f8b2bc0e4faa


### Архитектура
Реализована слоистая архитектура: выделены отдельно ядро, use cases, сервисы и репозитории для работы с PostgreSQL/YDB. Вышестоящие слои не зависят от инфраструктуры (userver, СУБД и т.д)


### Система в целом
В качестве сервиса динконфигов можно использовать . Остальное доступно через
```bash
docker compose up
```

<img width="664" height="579" alt="image" src="https://github.com/user-attachments/assets/1fbd59c7-4589-45a8-b713-0ef757f40fbb" />


### Тесты
- 100 функциональных тестов на Python (testsuite)
- 374 модульных и интеграционных тестов на gtest
- 3 бенчмарка на шардированную мапу, MPSC-очередь и rcu<flat_map> (хранилище сессий)

Coverage столько то todo
- Так как сервис statefull была сделана отдельная таска для сброса сервиса между тестами
- Для удобной правки в тесте динамических конфигов были реализованы фикстурки `*_config`

### Метрики
Метрики сервиса начинаются с `chat_`, там есть статистика по сбалансированности шардов, размеру очередей, работе GC, количество активных сессий, отправленных сообщений, гистограммы latency.

Забрать можно так
```
curl '127.0.0.1:8081/service/monitor?format=prometheus&labels={}' | grep chat_
```
todo картинка после load_test

### Нагрузочный тест
todo 
todo флеймграф

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
