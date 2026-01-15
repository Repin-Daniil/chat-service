"""Тесты для получения информации о пользователе."""
from http import HTTPStatus

import pytest

from endpoints import register_user, get_user_by_name
from models import User
from utils import get_user_token
from validators import validate_profile


async def test_get_user(service_client, registered_user):
    """Проверяет успешное получение профиля пользователя."""
    response = await get_user_by_name(service_client, registered_user.username, registered_user.token)

    assert response.status == HTTPStatus.OK
    assert validate_profile(registered_user, response)


async def test_get_user_multiple_times(service_client, registered_user):
    """Проверяет успешное получение профиля пользователя несколько раз."""
    for i in range(0, 3):
        response = await get_user_by_name(service_client, registered_user.username, registered_user.token)

        assert response.status == HTTPStatus.OK
        assert validate_profile(registered_user, response)


@pytest.mark.parametrize('multiple_users', [(3, False)], indirect=True)
async def test_get_multiple_users(service_client, multiple_users):
    """Проверяет получение профилей нескольких пользователей."""
    for user in multiple_users:
        response = await get_user_by_name(service_client, user.username, user.token)

        assert response.status == HTTPStatus.OK
        assert validate_profile(user, response)


@pytest.mark.parametrize('token', [
    None,
    'wrong_token',
])
async def test_get_user_unauthorized(service_client, registered_user, token):
    """Проверяет отказ в доступе при невалидном токене."""
    response = await get_user_by_name(service_client, registered_user.username, token)

    assert response.status == HTTPStatus.UNAUTHORIZED


@pytest.mark.parametrize('token_prefix,jwt_token', [
    ('Bearer', 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6ImlkIn0.d9NfuI8JzGnYtjPa5rHRh4Jr104WKp-yls9POZJbe9U'),
    ('Token', 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6ImlkIn0.d9NfuI8JzGnYtjPa5rHRh4Jr104WKp-yls9POZJbe9U'),
])
async def test_get_user_wrong_token_format(service_client, registered_user, token_prefix, jwt_token):
    """Проверяет отказ в доступе при неправильном формате токена."""
    invalid_token = f'{token_prefix} {jwt_token}'
    response = await get_user_by_name(service_client, registered_user.username, invalid_token)

    assert response.status == HTTPStatus.UNAUTHORIZED


async def test_get_user_empty_username(service_client, registered_user):
    """Проверяет ошибку при пустом имени пользователя."""

    response = await get_user_by_name(service_client, None, registered_user.token)

    assert response.status == HTTPStatus.NOT_FOUND
    assert 'errors' in response.json().get('details', {})


async def test_get_user(service_client, registered_user):
    """Проверяет ошибку при неизвестном пользователе."""
    response = await get_user_by_name(service_client, 'unknown_user', registered_user.token)

    assert response.status == HTTPStatus.NOT_FOUND
    assert 'errors' in response.json().get('details', {})
