"""Тесты для логина пользователей."""
from http import HTTPStatus

import pytest

from endpoints import login_user
from models import User
from utils import Routes
from validators import validate_user_login


async def test_login_success(service_client, registered_user):
    """Проверяет успешный логин зарегистрированного пользователя."""
    response = await login_user(service_client, registered_user)

    assert response.status == HTTPStatus.OK
    assert validate_user_login(registered_user, response)
    assert 'token' in response.json()


async def test_login_wrong_password(service_client, registered_user):
    """Проверяет, что логин с неверным паролем отклоняется."""
    user = User(username=registered_user.username,
                password='WrongPassword!123')
    response = await login_user(service_client, user)

    assert response.status == HTTPStatus.UNAUTHORIZED
    assert 'errors' in response.json()['details']


async def test_login_nonexistent_user(service_client):
    """Проверяет, что логин несуществующего пользователя отклоняется."""
    user = User(username='nonexistent_user')
    response = await login_user(service_client, user)

    assert response.status == HTTPStatus.UNAUTHORIZED
    assert 'errors' in response.json()['details']
    assert 'credentials' in response.json()['details']['errors']
    assert response.json()['details']['errors']['credentials'] == [
        'Wrong credentials']


async def test_login_multiple_times(service_client, registered_user):
    """Проверяет, что пользователь может логиниться несколько раз."""
    for _ in range(3):
        response = await login_user(service_client, registered_user)

        assert response.status == HTTPStatus.OK
        assert 'token' in response.json()


async def test_login_different_users(service_client):
    """Проверяет логин нескольких разных пользователей."""
    users = [User() for _ in range(3)]

    # Регистрируем пользователей
    for user in users:
        from endpoints import register_user
        await register_user(service_client, user)

    # Логинимся каждым
    for user in users:
        response = await login_user(service_client, user)

        assert response.status == HTTPStatus.OK
        assert 'token' in response.json()


# === Невалидные запросы ===

@pytest.mark.parametrize('user,status', [
    (User.model_construct(username=None), HTTPStatus.BAD_REQUEST),
    (User.model_construct(password=None), HTTPStatus.BAD_REQUEST),
    (User.model_construct(username=None, password=None), HTTPStatus.BAD_REQUEST),
])
async def test_login_missing_fields(service_client, user, status):
    """Проверяет запросы с отсутствующими обязательными полями."""
    response = await login_user(service_client, user)

    assert response.status == status


async def test_login_invalid_json(service_client):
    """Проверяет отклонение запроса с невалидным JSON."""
    response = await service_client.post(Routes.LOGIN, data='invalid json')

    assert response.status == HTTPStatus.BAD_REQUEST


async def test_login_invalid_login(service_client, registered_user):
    """Проверяет, что логин с невалидным логином отклоняется."""
    user = User(username='123', password=registered_user.username)
    response = await login_user(service_client, user)

    assert response.status == HTTPStatus.UNAUTHORIZED
    assert 'errors' in response.json()['details']
    assert 'credentials' in response.json()['details']['errors']
    assert response.json()['details']['errors']['credentials'] == [
        'Invalid credentials']


async def test_login_invalid_password(service_client, registered_user):
    """Проверяет, что логин с невалидным паролем отклоняется."""
    user = User(username=registered_user.username, password='invalidpassword')
    response = await login_user(service_client, user)

    assert response.status == HTTPStatus.UNAUTHORIZED
    assert 'errors' in response.json()['details']
    assert 'credentials' in response.json()['details']['errors']
    assert response.json()['details']['errors']['credentials'] == [
        'Invalid credentials']
