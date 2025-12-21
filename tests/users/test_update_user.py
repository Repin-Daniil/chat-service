"""Тесты для обновления пользователя."""
from http import HTTPStatus

import pytest

from endpoints import update_user_by_name
from models import User


async def test_update_user_username(service_client, registered_user):
    """Проверяет успешное обновление имени пользователя."""
    new_username = f"{registered_user.username}_updated"
    user_fields = {"user": {"username": new_username}}
    response = await update_user_by_name(
        service_client, 
        registered_user.username, 
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.OK
    assert response.json().get('username') == new_username


async def test_update_user_password(service_client, registered_user):
    """Проверяет успешное обновление пароля пользователя."""
    new_password = "new_secure_Password123"
    user_fields = {"user": {"password": new_password}}
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.OK
    assert response.json().get('username') == registered_user.username


async def test_update_user_display_name(service_client, registered_user):
    """Проверяет успешное обновление отображаемого имени пользователя."""
    new_display_name = "New Display Name"
    user_fields = {"user": {"display_name": new_display_name}}
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.OK
    assert response.json().get('username') == registered_user.username


async def test_update_user_biography(service_client, registered_user):
    """Проверяет успешное обновление биографии пользователя."""
    new_biography = "This is my new biography"
    user_fields = {"user": {"biography": new_biography}}
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.OK
    assert response.json().get('username') == registered_user.username


async def test_update_user_multiple_fields(service_client, registered_user):
    """Проверяет успешное обновление нескольких полей одновременно."""
    new_username = f"{registered_user.username}_new"
    new_display_name = "Updated Name"
    new_biography = "Updated bio"
    
    user_fields = {
        "user": {
            "username": new_username,
            "display_name": new_display_name,
            "biography": new_biography
        }
    }
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.OK
    assert response.json().get('username') == new_username


@pytest.mark.parametrize('multiple_users', [3], indirect=True)
async def test_update_multiple_users(service_client, multiple_users):
    """Проверяет обновление нескольких пользователей."""
    for user in multiple_users:
        new_display_name = f"{user.username}_display"
        user_fields = {"user": {"display_name": new_display_name}}
        response = await update_user_by_name(
            service_client,
            user.username,
            user.token,
            user_fields
        )

        assert response.status == HTTPStatus.OK
        assert response.json().get('username') == user.username


@pytest.mark.parametrize('token', [
    None,
    'wrong_token',
])
async def test_update_user_unauthorized(service_client, registered_user, token):
    """Проверяет отказ в доступе при невалидном токене."""
    user_fields = {"user": {"display_name": "New Name"}}
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        token,
        user_fields
    )

    assert response.status == HTTPStatus.UNAUTHORIZED


async def test_update_nontexistent_user(service_client, registered_user):
    """Проверяет ошибку при попытке обновить несуществующего пользователя."""
    user_fields = {"user": {"display_name": "New Name"}}
    response = await update_user_by_name(
        service_client,
        "nonexistent_user",
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.FORBIDDEN


@pytest.mark.parametrize('multiple_users', [2], indirect=True)
async def test_update_user_forbidden(service_client, multiple_users):
    """Проверяет ошибку при попытке обновить чужого пользователя."""
    user_a, user_b = multiple_users

    user_fields = {"user": {"display_name": "Hacked Name"}}
    response = await update_user_by_name(
        service_client,
        user_b.username,
        user_a.token,
        user_fields
    )

    assert response.status == HTTPStatus.FORBIDDEN
    assert 'errors' in response.json().get('details', {})


@pytest.mark.parametrize('username,expected_error_field', [
    ('', 'username'),
    ('ab', 'username'),
])
async def test_update_user_invalid_username(service_client, registered_user, username, expected_error_field):
    """Проверяет отклонение невалидных username."""
    user_fields = {"user": {"username": username}}
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.BAD_REQUEST
    errors = response.json()['details']['errors']
    assert any(err == expected_error_field for err in errors)


@pytest.mark.parametrize('password,expected_error_field', [
    ('', 'password'),
    ('123', 'password'),
])
async def test_update_user_invalid_password(service_client, registered_user, password, expected_error_field):
    """Проверяет отклонение невалидных password."""
    user_fields = {"user": {"password": password}}
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.BAD_REQUEST
    errors = response.json()['details']['errors']
    assert any(err == expected_error_field for err in errors)


@pytest.mark.parametrize('display_name,expected_error_field', [
    ('', 'display_name'),
])
async def test_update_user_invalid_display_name(service_client, registered_user, display_name, expected_error_field):
    """Проверяет отклонение невалидных display_name."""
    user_fields = {"user": {"display_name": display_name}}
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.BAD_REQUEST
    errors = response.json()['details']['errors']
    assert any(err == expected_error_field for err in errors)


@pytest.mark.parametrize('multiple_users', [2], indirect=True)
async def test_update_user_duplicate_username(service_client, multiple_users):
    """Проверяет ошибку при попытке обновить username на уже существующий."""
    user_a, user_b = multiple_users
    
    user_fields = {"user": {"username": user_b.username}}
    response = await update_user_by_name(
        service_client,
        user_a.username,
        user_a.token,
        user_fields
    )

    assert response.status == HTTPStatus.CONFLICT
    errors = response.json()['details']['errors']
    assert 'user' in errors


async def test_update_user_biography_can_be_empty(service_client, registered_user):
    """Проверяет что biography может быть пустой."""
    user_fields = {"user": {"biography": ""}}
    response = await update_user_by_name(
        service_client,
        registered_user.username,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.OK
    assert response.json().get('username') == registered_user.username


async def test_update_user_empty_username_in_path(service_client, registered_user):
    """Проверяет ошибку при пустом имени пользователя в пути."""
    user_fields = {"user": {"display_name": "New Name"}}
    response = await update_user_by_name(
        service_client,
        None,
        registered_user.token,
        user_fields
    )

    assert response.status == HTTPStatus.BAD_REQUEST
