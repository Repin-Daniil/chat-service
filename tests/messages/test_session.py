from http import HTTPStatus

import pytest

from endpoints import start_session
from collections import Counter
from validators import validate_session
import asyncio


@pytest.mark.parametrize('registered_user', [(False)], indirect=True)
async def test_start_session_basic(service_client, registered_user):
    response = await start_session(service_client, registered_user.token)

    assert response.status == HTTPStatus.OK
    assert validate_session(response)


async def test_multiple_sessions(service_client, registered_user, monitor_client):
    for i in range(5):
        response = await start_session(service_client, registered_user.token)
        assert response.status == HTTPStatus.OK
        assert validate_session(response)
        metrics = await monitor_client.metrics(prefix='chat_sessions.')
        assert metrics.value_at('chat_sessions.opened.total') == (i + 1)


@pytest.mark.parametrize('sessions_config', [(1, 1)], indirect=True)
async def test_session_limit(service_client, registered_user, sessions_config):
    response_1 = await start_session(service_client, registered_user.token)
    response_2 = await start_session(service_client, registered_user.token)

    assert response_1.status == HTTPStatus.OK
    assert response_2.status == HTTPStatus.TOO_MANY_REQUESTS


@pytest.mark.parametrize('registry_config', [(0)], indirect=True)
async def test_users_limit(service_client, registered_user, registry_config):
    response = await start_session(service_client, registered_user.token)

    assert response.status == HTTPStatus.SERVICE_UNAVAILABLE
