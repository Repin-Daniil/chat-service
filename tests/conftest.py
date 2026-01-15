from helpers.utils import get_user_token, get_session_id
from helpers.endpoints import register_user, start_session
from helpers.models import User, Message
from http import HTTPStatus
import os
import sys
import pytest

from testsuite.databases.pgsql import discover

sys.path.append(os.path.join(os.path.dirname(__file__), 'helpers'))


pytest_plugins = [
    'pytest_userver.plugins.core',
    'pytest_userver.plugins.postgresql',
]


@pytest.fixture
async def registered_user(service_client):
    """Registered user with token"""
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK, "Регистрация пользователя не удалась"

    user.token = get_user_token(response)
    return user


@pytest.fixture
async def multiple_users(service_client, request):
    """Multiple registered users, by default 2 with opened sessions"""
    users_amount = 2
    start_session_enabled = True

    if hasattr(request, "param"):
        param = request.param
        if isinstance(param, int):
            users_amount = param
        elif isinstance(param, (tuple, list)):
            users_amount = param[0] if len(param) > 0 else 2
            start_session_enabled = param[1] if len(param) > 1 else True

    users_data = []

    for _ in range(users_amount):
        user = User()
        response = await register_user(service_client, user)
        assert response.status == HTTPStatus.OK

        user.token = get_user_token(response)
        if start_session_enabled:
            response = await start_session(service_client, user.token)
            user.session_id = get_session_id(response)

        users_data.append(user)

    return users_data


@pytest.fixture
async def single_consumer(service_client, registered_user):
    """Registered user with opened session"""
    registered_user.session_id = get_session_id(await start_session(service_client, registered_user.token))
    return registered_user


@pytest.fixture
async def communication(service_client, multiple_users):
    """Two users with opened sessions and dummy message"""
    sender, recipient = multiple_users
    message = Message(recipient=recipient.username, sender=sender.username)
    return sender, recipient, message


@pytest.fixture(scope='session')
def initial_data_path(service_source_dir):
    """Path for find files with data"""
    return [
        service_source_dir / 'postgresql/data',
    ]


@pytest.fixture(scope='session')
def pgsql_local(service_source_dir, pgsql_local_create):
    """Create schemas databases for tests"""
    databases = discover.find_schemas(
        'chat_service',
        [service_source_dir.joinpath('postgresql/schemas')],
    )
    return pgsql_local_create(list(databases.values()))
