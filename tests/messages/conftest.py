import pytest

# todo Надо отдельный механизм сброс сервиса, а не через GC


@pytest.fixture(autouse=True)
async def garbage_collect(service_client, dynamic_config, mocked_time, request):
    yield

    dynamic_config.set_values({
        "OVERSEER_TASK_CONFIG": {
            "is_enabled": True,
            "period_seconds": 1800,
            "inter_shard_pause_ms": 0
        },
        "REGISTRY_CONFIG": {
            "idle_timeout_sec": 1,
            "max_queue_size": 1000,
        }
    })
    mocked_time.sleep(2)
    await service_client.run_periodic_task('overseer-background-job')


@pytest.fixture()
def overseer_config(dynamic_config, request):
    param = getattr(request, "param", ())

    if isinstance(param, bool):
        param = (param,)

    is_enabled = param[0] if len(param) > 0 else True
    period_seconds = param[1] if len(param) > 1 else 5
    inter_shard_pause_ms = param[2] if len(param) > 2 else 100

    dynamic_config.set_values({
        "OVERSEER_TASK_CONFIG": {
            "is_enabled": is_enabled,
            "period_seconds": period_seconds,
            "inter_shard_pause_ms": inter_shard_pause_ms
        }
    })


@pytest.fixture()
def registry_config(dynamic_config, request):
    param = getattr(request, "param", ())

    if isinstance(param, int):
        param = (param,)

    idle_timeout_sec = param[0] if len(param) > 0 else 60
    max_queue_size = param[1] if len(param) > 1 else 1000

    dynamic_config.set_values({
        "REGISTRY_CONFIG": {
            "idle_timeout_sec": idle_timeout_sec,
            "max_queue_size": max_queue_size,
        }
    })


@pytest.fixture()
def short_polling(dynamic_config, request):
    dynamic_config.set_values({
        "POLLING_CONFIG": {
            "max_size": 100,
            "polling_time_sec": 1,
        }
    })


@pytest.fixture()
def custom_polling(dynamic_config, request):
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
