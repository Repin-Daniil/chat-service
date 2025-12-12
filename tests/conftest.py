import os
import sys
import pytest

from testsuite.databases.pgsql import discover

sys.path.append(os.path.join(os.path.dirname(__file__), 'helpers'))

from http import HTTPStatus
from models import User
from endpoints import register_user
from utils import get_user_token

pytest_plugins = [
    'pytest_userver.plugins.core',
    'pytest_userver.plugins.postgresql',
]


@pytest.fixture
async def registered_user(service_client):
    """Создаёт и регистрирует пользователя, возвращает (user, token)."""
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK, "Регистрация пользователя не удалась"

    token = get_user_token(response)
    return user, token


@pytest.fixture
async def multiple_users(service_client):
    """Создаёт и регистрирует несколько пользователей."""
    user_amount = 5
    users_data = []

    for _ in range(user_amount):
        user = User()
        response = await register_user(service_client, user)
        assert response.status == HTTPStatus.OK

        token = get_user_token(response)
        users_data.append((user, token))

    return users_data


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
