# Agent Context: Distributed Chat Service Architecture

Контекст для AI-агентов. Содержит ключевые архитектурные решения, идеи и ограничения, согласованные в процессе проектирования распределённой системы доставки сообщений.

> При работе с этим проектом учитывать: целевая архитектура — распределённая (Gateway, Hub, API), текущий код — standalone. Рефакторинг направлен на подготовку к распилу без переписывания.

---

## 1. Текущее состояние и целевая архитектура

### Текущий standalone
- Один процесс, userver
- Polling / в будущем WebSockets
- Lock-free очереди, шардированная мапа (mailbox registry)
- Фоновый GC удаляет старые очереди
- PostgreSQL (миграция на YDB планируется)

### Целевая распределённая система (3 сервиса)

| Сервис | Роль |
|--------|------|
| **Gateway** | Подключения пользователей (WebSocket, polling), lock-free очереди, локальное состояние подписок (channel→users) |
| **Hub** | Кластер, к нему подключаются все Gateways. Маршрутизация по channel_id → gateways через Redis. Stateless (пока) |
| **API Service** | CRUD, единственный писатель в БД (YDB) |

---

## 2. Проблема больших чатов

- Участники чата хранятся в YDB (много)
- Онлайн пользователи — на шлюзах
- Intersection «все участники ∩ онлайн» в runtime — нереализуемо

### Решение: инкрементальное построение

1. При подключении пользователя Gateway выгружает все его чаты и строит локальную мапу: `channel_id → user_id(s)`
2. Состояние обновляется только по **пунктуации** (служебным сообщениям в канале). Отдельных RPC-вызовов от API к gateways **нет**.
3. При отключении пользователь удаляется из мапы
4. Фоновая userver task обходит локальные мапы, батчами пишет в Redis: `chat_id → [gateway_id]`
5. Hub не знает участников — только доставка до нужных gateways
6. Жертва консистентностью ради производительности

---

## 3. Доменная модель: TGroupChat и DDD

### Проблема
Группа может содержать огромное число участников. Грузить всех из БД при каждом обращении — неприемлемо.

### Принцип
Группа держит **только бизнес-логику**, не состояние участников.

### Дизайн

- **Команды** (AddMember, DeleteMember, GrantUser, ChangeOwner): группа принимает контекст с ролями requester/target, возвращает **дельту** (TGroupMemberDelta)
- **Репозиторий** загружает только нужных участников: `GetMembersForCommand(chat_id, {requester_id, target_id})`, применяет `ApplyDelta(chat_id, delta)`
- **CanPost** — проверка роли отправителя (1 запрос в БД)
- **GetRecipients** — не возвращать вектор всех id для routing; для групп — отдельная модель доставки (pub/sub, fan-out)

### Инварианты
- В группе всегда ровно один Owner
- Owner не может покинуть группу без передачи владения



## 4. SubscriptionRegistry (локальное состояние подписок)

### Роль
Центральный компонент для «кто из локальных пользователей в каком чате». В standalone симулирует Gateway.

### Интерфейс
```
AddUserChats(user_id, chat_ids)           // при connect
RemoveUser(user_id)                       // при disconnect (через DisconnectSet)
ApplyMembershipChange(chat_id, user_id, action)  // из пунктуации
GetLocalSubscribers(chat_id) → [user_id]  // для доставки
```

### Структура данных (без inverse map)

**Основная мапа:** `channel_id → Set(user_id)` — одна, не бидирекциональная.

**Зачем inverse (user_id → [channel_id])?** Только для RemoveUser при disconnect. Но:
- При disconnect добавляем `user_id` в **DisconnectSet** (MPSC-очередь или concurrent set)
- Фоновая task при обходе мапы для Redis sync: для каждого `(channel_id, users)` делает `users -= DisconnectSet`, удаляет пустые каналы
- Inverse map **не нужна** — cleanup во время итерации

**Шардирование:**
- По тем же шардам, что и воркеры (consistent hashing по `channel_id`)
- `N` шардов: `vector<pair<map<channel_id, set<user_id>>, SharedMutex>>`
- Паттерн как в `TShardedMap`: шард = обычная мапа + мьютекс, меньше contention
- userver BidiMap не подходит — не concurrent, и не multimap (one-to-many)

**Значение:** `Set<user_id>` — один канал, много пользователей. Тип: `map<channel_id, unordered_set<user_id>>` внутри шарда.

**DisconnectSet:** MPSC-очередь или concurrent set для `user_id`, которые отключились. Фоновая task дренирует в локальный set и при обходе мапы применяет `users -= DisconnectSet`.

### Обновление
- **Пунктуация (service message)** → воркер применяет `ApplyMembershipChange` к мапе
- **Connect** → AddUserChats (из use case при старте сессии)
- **Disconnect** → user_id в DisconnectSet, фоновая task при обходе удалит

---

## 5. Синхронизация с Redis

- **При connect** — сразу писать в Redis по чатам пользователя (низкая задержка)
- **Фоновая task** — обход мапы, применение DisconnectSet (users -= DisconnectSet), обновление Redis, cleanup пустых каналов
- Redis: `chat:{chat_id} → Set(gateway_id)`

---

## 6. Punctuation (служебные сообщения)

- **Нет RPC-вызовов от API к gateways** — только пунктуация через транспорт
- Обновления идут как **служебные сообщения** в канале
- Первичное состояние — при connect (список чатов с API)
- Punctuation для gateways, уже имеющих подписчиков в чате
- Один idempotency key для дедупликации

---

## 7. Hash / эпоха состояния чата

- Состояние чата: `state = f(ordered_events)` (детерминированно)
- `hash = H(state)` или эпоха — хранить в БД
- **Hash/эпоху менять только при служебном сообщении** (membership change). Обычное сообщение — не меняет состояние чата.
- К служебным сообщениям прикладывать новый hash; к обычным — текущий (для проверки)
- Gateway при получении сравнивает hash с локальным:
  - Совпадает → применить (если service) и доставить
  - Не совпадает → загрузить актуальное состояние из API/БД, применить, затем доставить
- Восстановление при рассогласованности без отдельного канала синхронизации

---

## 8. Calvin-style ordering, Hub как sequencer

- Порядок событий: Kafka partition по `channel_id`
- Hub (partition owner) потребляет в порядке partition, рассылает gateways
- Без 2PC с gateways — fire-and-forget
- Coordinator: partition_map (какой Hub обслуживает какие каналы)

### Primary / Standby Hub
- Primary: применяет (передаёт) события, может держать историю
- Standby: держит историю, для failover и catch-up

---

## 9. Per-channel serialization (критично)

- В рамках одного `channel_id` обрабатывает **один воркер** → FIFO
- Обновление мапы (service message) должно происходить **синхронно** в том же обработчике, до следующего сообщения
- Нельзя «положить в очередь и обработать позже» — следующие сообщения должны видеть уже обновлённое состояние

---

## 10. Userver: N очередей, N воркеров, consistent hashing

### Схема
- **N очередей** (количество динамически конфигурируется)
- **N worker tasks** — читают из своих очередей, пушат в очереди пользователей или применяют события
- **Streaming thread** — только читает из gRPC и пушит в очереди воркеров (минимальная логика)
- Распределение: **consistent hashing** по `channel_id` → выбор очереди/воркера
- Один channel_id всегда обрабатывается одним воркером → FIFO внутри канала без отдельного per-channel handler

```
gRPC Stream → [read] → consistent_hash(channel_id) → Queue[worker_id] → Worker → user queues / apply events
```

---

## 11. Чек-листы

### Перед распилом
- [ ] SubscriptionRegistry изолирован
- [x] GetRecipients убран из IChat; доставка через GetRecipientsForDelivery (Private) / GetLocalSubscribers (Group)
- [x] TGroupChat stateless (валидация + delta)
- [ ] Интерфейсы разделены (API validation vs Gateway delivery)

### Redis
- [ ] HA (Cluster / Sentinel)
- [ ] TTL / heartbeat для gateways
- [ ] Graceful shutdown — снятие gateway с Redis

### Надёжность
- [ ] Idempotency key для сообщений и membership events
- [ ] Мониторинг: Redis latency, sync lag, delivery latency

---

## 12. Файлы и модули (релевантные)

| Модуль | Описание |
|--------|----------|
| `src/core/chats/group/group_chat.hpp/cpp` | TGroupChat stateless — валидация + delta (ValidateAddMember и т.д.) |
| `src/core/chats/group/group_delta.hpp` | TGroupMemberDelta, TGroupInfoDelta — дельты для ApplyDelta в репо |
| `src/core/chats/delivery_recipients.hpp/cpp` | GetRecipientsForDelivery — для Private; Group → GetLocalSubscribers |
| `src/core/chats/chat.hpp` | IChat — без GetRecipients |
| `src/core/chats/chat_repo.hpp` | Репозиторий чатов |
| `src/app/use-cases/messages/send_message/` | Использует GetRecipientsForDelivery |
| `src/core/messaging/router/` | Сейчас итерирует всех recipients; для групп — другая схема |
| `postgresql/schemas/chat_db.sql` | channel_members, channels |

---

*Документ создан на основе обсуждений архитектуры. Обновлять при изменении решений.*
