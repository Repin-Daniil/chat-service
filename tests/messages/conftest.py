import pytest


@pytest.fixture(autouse=True)
async def garbage_collect(service_client, testpoint, request):
    """Сброс сервиса после теста"""
    yield

    @testpoint('reset-task/action')
    def task_action(data):
        pass

    await service_client.run_task('reset-task')


@pytest.fixture()
def gc_config(dynamic_config, request):
    """Динамический конфиг таски удаления старых очередей"""
    param = getattr(request, "param", ())

    if isinstance(param, bool):
        param = (param,)

    is_enabled = param[0] if len(param) > 0 else True
    period_seconds = param[1] if len(param) > 1 else 5
    inter_shard_pause_ms = param[2] if len(param) > 2 else 100

    dynamic_config.set_values({
        "GC_TASK_CONFIG": {
            "is_enabled": is_enabled,
            "period_seconds": period_seconds,
            "inter_shard_pause_ms": inter_shard_pause_ms
        }
    })


@pytest.fixture()
def registry_config(dynamic_config, request):
    """Динамический конфиг mailbox registry"""
    param = getattr(request, "param", ())

    if isinstance(param, int):
        param = (param,)

    max_users_amount = param[0] if len(param) > 0 else 10000

    dynamic_config.set_values({
        "REGISTRY_CONFIG": {
            "max_users_amount": max_users_amount,
        }
    })


@pytest.fixture()
def sessions_config(dynamic_config, request):
    """Динамический конфиг sessions registry"""
    param = getattr(request, "param", ())

    if isinstance(param, int):
        param = (param,)

    idle_timeout_sec = param[0] if len(param) > 0 else 1
    max_sessions_amount = param[1] if len(param) > 1 else 5

    dynamic_config.set_values({
        "SESSIONS_CONFIG": {
            "idle_timeout_sec": idle_timeout_sec,
            "max_sessions_amount": max_sessions_amount
        }
    })


@pytest.fixture()
def queue_config(dynamic_config, request):
    """Динамический конфиг queue factory"""
    param = getattr(request, "param", ())

    if isinstance(param, int):
        param = (param,)

    max_queue_size = param[0] if len(param) > 0 else 1000

    dynamic_config.set_values({
        "QUEUE_CONFIG": {
            "max_queue_size": max_queue_size,
        }
    })


@pytest.fixture()
def short_polling(dynamic_config, request):
    """Делаем поллинг коротким — всего секунда"""
    dynamic_config.set_values({
        "POLLING_CONFIG": {
            "max_size": 100,
            "polling_time_sec": 1,
        }
    })


@pytest.fixture()
def custom_polling(dynamic_config, request):
    """Конфигурация поллинга"""
    param = getattr(request, "param", ())

    if isinstance(param, int):
        param = (param,)

    polling_time_sec = param[0] if len(param) > 0 else 100
    max_size = param[1] if len(param) > 1 else 5

    dynamic_config.set_values({
        "POLLING_CONFIG": {
            "polling_time_sec": polling_time_sec,
            "max_size": max_size,
        }
    })
