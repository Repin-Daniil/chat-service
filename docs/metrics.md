# Метрики сервиса

Забрать можно так
```
curl --globoff '127.0.0.1:8081/service/monitor?format=prometheus&labels={}' | grep chat_
```

### Метрики доставки сообщений
- Gauge chat_messages_polling_active_current — число активных сеансов поллинга, сколько пользователей прямо сейчас ждут сообщений
- Гистограмма chat_messages_polling_duration_sec_hist — распределение времени поллинга в секундах {1, 2, 5, 10, 90, 180}
- Гистограмма chat_messages_queue_wait_latency_sec_hist — распределение времени ожидания вычитки сообщения в секундах {1, 3, 5, 10, 15, 50, 100, 150, 200}
- Гистограмма chat_messages_send_overhead_us_hist — распределение времени оверхеда на отправку сообщения в микросекундах (до пуша в очередь) {1, 100, 500, 1000, 5'000
- Гистограмма chat_messages_polling_overhead_us_hist — распределениие времени оверхеда на получения сообщения в микросекундах (от момента вычитки из очереди) {1, 500, 700, 1000, 5'000, 10'000, 100'000}
- Гистограмма chat_messages_batch_size_hist — распределение размера получаемого батча сообщений {1, 5, 10, 20, 50, 70, 100}

### Метрики по сессиям
- Gauge chat_sessions_opened_current — число активных сессий (очередей на сервере)
- Counter chat_sessions_removed_total — число удаленных сессий сборщиком мусора
- Counter chat_sessions_sent_messages_total — число отправленных сообщений за время работы
- Гистограмма chat_sessions_per_user_hist — распределение количества сессий на пользователя {1, 2, 3, 4, 5}
- Гистограмма chat_sessions_queue_size_hist — распределение размера очередей (невычитанные сообщения) {1, 5, 10, 25, 50, 100, 500, 1000}
- Гистограмма chat_sessions_lifetime_sec_hist — распределение возраста очередей/сессий в секундах {1, 10, 50, 100, 500, 1000, 10000}

### Метрики по онлайн-пользователям (Mailbox registry)
- Gauge chat_mailbox_opened_current — число онлайн пользователей (без учета сессий)
- Counter chat_mailbox_removed_total — число удаленных сборщиком мусора «почтовых ящиков» пользователей
- Гистограмма chat_mailbox_shards_size_hist — распределение размера шардов в мапе {1, 10, 100, 500, 1000, 10000}

### Метрики лимитера
- Gauge chat_limiter_opened_current — число активных лимитеров
- Counter chat_limiter_removed_total — счетчик удаленных лимитеров сборщиком мусора (старых лимитеров)
- Counter chat_limiter_rejected_total — счетчик отклоненных запросов
- Гистограмма chat_limiter_shards_size_hist — распределение размера шардов в мапе {1, 10, 100, 500, 1000, 10000}

# Дашборды
### Метрики сервиса
<img width="1401" height="772" alt="Screenshot 2026-01-29 at 17 13 46" src="https://github.com/user-attachments/assets/184d5827-4064-4ba3-979d-f2368a096ad3" />
<img width="1397" height="816" alt="Screenshot 2026-01-29 at 17 14 06" src="https://github.com/user-attachments/assets/5c87be37-f333-40d5-9177-1388ac092b33" />

### Статистика по ручкам
<img width="1397" height="816" alt="Screenshot 2026-01-29 at 17 14 06" src="https://github.com/user-attachments/assets/7aabfaa6-f7da-4328-b28b-8db33bcdcece" />

### Дашборд Node Exporter
<img width="1398" height="761" alt="image" src="https://github.com/user-attachments/assets/ea69cc51-a42f-4ed0-8139-5d483edd3ecb" />
