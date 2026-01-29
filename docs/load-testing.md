## Нагрузочное тестирование
- Нагрузочное тестирование проводилось в Yandex Load Testing
- Генератор нагрузки - Pandora
- Для стрельбы использовалась самописная кастомная [пушка](https://github.com/Ilya-Repin/chat-load-testing) на Go.
- Нагрузка подавалась с 8 агентов (4 vCPU, 4 GB RAM, 30 GB Disk каждый)
- Удалось достичь нагрузки в 30k RPS (сервис не завалили, уперлись в сеть)
- Характеристики машинки-стенда: 28 vCPU, 110 GB RAM, 256 GB Disk
- Отчет о нагрузочном тестировании
- [Флеймграф](https://github.com/user-attachments/assets/3c932757-b4f1-4d54-b918-7c304e283f99)

### Пушка
- Не используются патроны и ammo файл
- На подготовительном этапе (150 секунд) пушка регистрирует в сервисе N пользователей и среди них для M пользователей начинает поллинг
- В секции `rps` регулируется количество `send` запросов
- Фоновые процессы поллинга также создают RPS
- Бинарь с пушкой загружен в Object Storage в Yandex Cloud и подгружается перед тестами
- Настроен автостоп

**Пример конфига:**
```yaml
pandora:
  enabled: true
  package: yandextank.plugins.Pandora
  pandora_cmd: ./top-gun
  config_content:
    pools:
      - id: Chat pool
        gun:
          type: top-gun
          target: http://host:port
          prefix: load_test_session
          pollers: M
          delay: 10
        ammo:
          type: dummy
        result:
          type: phout
          destination: ./phout.log
        rps:
          - type: const
            ops: 0
            duration: 150s
          - type: step
            from: 1000
            to: 6000
            step: 200
            duration: 10s
        startup:
          type: once
          times: N
        discard_overflow: true
    log:
      level: error
    monitoring:
      expvar:
        enabled: true
        port: 1234
telegraf:
  config:
    hosts:
      localhost: null
    metrics:
      cpu: null
      mem: null
      diskio: null
      net: null
      netstat: null
      system: null
      kernel: null
  enabled: true
  package: yandextank.plugins.Telegraf
autostop:
  enabled: true
  package: yandextank.plugins.Autostop
  autostop:
    - instances(90%,5s)
    - negative_http(2xx,100%,5s,)
    - negative_net(0,100%,5s,)
    - limit(20m)
core: {}

```

### Ход теста
**1) Подготовительный этап (только поллинг)**
<img width="1187" height="431" alt="image" src="https://github.com/user-attachments/assets/ba69347d-97db-4231-b3ae-ea94a274bdfe" />

**2.1) Основной этап (poll)**
<img width="1179" height="435" alt="image" src="https://github.com/user-attachments/assets/2de1add2-01d1-4ecf-b39f-affb841984c7" />

**2.2) Основной этап (send)**
<img width="1185" height="433" alt="image" src="https://github.com/user-attachments/assets/bc171a43-345e-4e0d-8630-190da07d0439" />

**3.1) Деградация (poll)**
<img width="1174" height="442" alt="image" src="https://github.com/user-attachments/assets/d0887a47-c051-4f06-aec4-c1204bb7ff95" />

**3.2) Деградация (send)**
<img width="1183" height="432" alt="image" src="https://github.com/user-attachments/assets/661061be-623f-497a-a171-b0846edda563" />

**Общий вид (poll)**
<img width="1184" height="441" alt="image" src="https://github.com/user-attachments/assets/406b364e-6e0f-4159-adb2-188bcb889cc0" />

**Общий вид (send)**
<img width="1181" height="438" alt="image" src="https://github.com/user-attachments/assets/0755e79b-9ee8-4116-b231-fab013a2a03d" />


**Деградация на сетевых метриках одного из агентов**
<img width="1189" height="458" alt="image" src="https://github.com/user-attachments/assets/d40809ad-9261-4d15-9552-47c8d3995935" />


### Метрики сервиса
<img width="1401" height="772" alt="Screenshot 2026-01-29 at 17 13 46" src="https://github.com/user-attachments/assets/184d5827-4064-4ba3-979d-f2368a096ad3" />
<img width="1397" height="816" alt="Screenshot 2026-01-29 at 17 14 06" src="https://github.com/user-attachments/assets/5c87be37-f333-40d5-9177-1388ac092b33" />

### Статистика по ручкам
<img width="1397" height="816" alt="Screenshot 2026-01-29 at 17 14 06" src="https://github.com/user-attachments/assets/7aabfaa6-f7da-4328-b28b-8db33bcdcece" />

### Дашборд Node Exporter
<img width="1398" height="761" alt="image" src="https://github.com/user-attachments/assets/ea69cc51-a42f-4ed0-8139-5d483edd3ecb" />



<img width="488" height="260" alt="Screenshot 2026-01-29 at 17 15 10" src="https://github.com/user-attachments/assets/dbc0b105-6945-4f73-9b2b-9c2827ceb1ff" />
