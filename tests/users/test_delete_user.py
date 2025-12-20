"""Тесты для удаления пользователя."""
from http import HTTPStatus

import pytest

from endpoints import delete_user_by_name
from models import User


async def test_delete_user(service_client, registered_user):
    """Проверяет успешное удаление пользователя."""
    response = await delete_user_by_name(service_client, registered_user.username, registered_user.token)

    assert response.status == HTTPStatus.OK


async def test_delete_user_multiple_times(service_client, registered_user):
    """Проверяет ошибку при повторном удалении пользователя."""
    response = await delete_user_by_name(service_client, registered_user.username, registered_user.token)
    assert response.status == HTTPStatus.OK

    response = await delete_user_by_name(service_client, registered_user.username, registered_user.token)
    assert response.status == HTTPStatus.UNAUTHORIZED
    assert response.json().get('message', {}) == "Invalid user auth"


@pytest.mark.parametrize('multiple_users', [3], indirect=True)
async def test_delete_multiple_users(service_client, multiple_users):
    """Проверяет удаление нескольких пользователей."""
    for user in multiple_users:
        response = await delete_user_by_name(service_client, user.username, user.token)

        assert response.status == HTTPStatus.OK


@pytest.mark.parametrize('token', [
    None,
    'wrong_token',
])
async def test_delete_user_unauthorized(service_client, registered_user, token):
    """Проверяет отказ в доступе при невалидном токене."""
    response = await delete_user_by_name(service_client, registered_user.username, token)

    assert response.status == HTTPStatus.UNAUTHORIZED


async def test_delete_user_empty_username(service_client, registered_user):
    """Проверяет ошибку при пустом имени пользователя."""
    response = await delete_user_by_name(service_client, None, registered_user.token)

    assert response.status == HTTPStatus.NOT_FOUND
    assert 'errors' in response.json().get('details', {})


@pytest.mark.parametrize('multiple_users', [2], indirect=True)
async def test_delete_user_forbidden(service_client, multiple_users):
    """Проверяет ошибку при попытке удалить чужого пользователя."""
    user_a, user_b = multiple_users

    response = await delete_user_by_name(service_client, user_b.username, user_a.token)

    assert response.status == HTTPStatus.FORBIDDEN
    assert 'errors' in response.json().get('details', {})
