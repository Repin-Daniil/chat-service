"""Тесты для регистрации пользователей."""
from http import HTTPStatus

import pytest

from endpoints import register_user
from models import User
from utils import Routes
from validators import validate_user_reg


async def test_register(service_client):
    """Проверяет успешную регистрацию нового пользователя."""
    user = User()
    response = await register_user(service_client, user)

    assert response.status == HTTPStatus.OK
    assert validate_user_reg(user, response)


async def test_register_same_username(service_client, registered_user):
    """Проверяет, что нельзя зарегистрировать пользователя с существующим username."""
    user, _ = registered_user

    duplicate_user = User(username=user.username)
    response = await register_user(service_client, duplicate_user)

    assert response.status == HTTPStatus.CONFLICT
    assert 'errors' in response.json()['details']


async def test_register_same_password(service_client, registered_user):
    """Проверяет, что можно зарегистрировать разных пользователей с одинаковым паролем."""
    user, _ = registered_user

    another_user = User(password=user.password)
    response = await register_user(service_client, another_user)

    assert response.status == HTTPStatus.OK


async def test_register_multiple_users(service_client):
    """Проверяет последовательную регистрацию нескольких пользователей."""
    users = [User() for _ in range(5)]

    for user in users:
        response = await register_user(service_client, user)

        assert response.status == HTTPStatus.OK
        assert validate_user_reg(user, response)


async def test_register_case_sensitive_username(service_client):
    """Проверяет, что username чувствителен к регистру."""
    user1 = User(username='TestUser')
    response = await register_user(service_client, user1)
    assert response.status == HTTPStatus.OK

    user2 = User(username='testuser')
    response = await register_user(service_client, user2)
    assert response.status == HTTPStatus.CONFLICT


# === Валидация полей ===

@pytest.mark.parametrize('username,expected_error_field', [
    ('', 'username'),      # Пустой username
    ('ab', 'username'),    # Слишком короткий username
])
async def test_register_invalid_username(service_client, username, expected_error_field):
    """Проверяет отклонение невалидных username."""
    user = User(username=username)
    response = await register_user(service_client, user)

    assert response.status == HTTPStatus.BAD_REQUEST
    errors = response.json()['details']['errors']
    assert any(err == expected_error_field for err in errors)


@pytest.mark.parametrize('password,expected_error_field', [
    ('', 'password'),          # Пустой пароль
    ('123456789', 'password'),  # Слабый пароль
])
async def test_register_invalid_password(service_client, password, expected_error_field):
    """Проверяет отклонение невалидных паролей."""
    user = User(password=password)
    response = await register_user(service_client, user)

    assert response.status == HTTPStatus.BAD_REQUEST
    errors = response.json()['details']['errors']
    assert any(err == expected_error_field for err in errors)


# === Невалидные запросы ===

@pytest.mark.parametrize('user,status', [
    (User.model_construct(password=None), HTTPStatus.BAD_REQUEST),
    (User(biography=None), HTTPStatus.OK),
    (User.model_construct(
        **{field: None for field in User.model_fields}), HTTPStatus.BAD_REQUEST),  # empty user
])
async def test_register_missing_fields(service_client, user, status):
    """Проверяет запросы с отсутствующими полями."""
    response = await register_user(service_client, user)

    assert response.status == status


async def test_register_invalid_json(service_client):
    """Проверяет отклонение запроса с невалидным JSON."""
    response = await service_client.post(Routes.REGISTRATION, data='invalid json')

    assert response.status == HTTPStatus.BAD_REQUEST
