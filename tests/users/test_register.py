from http import HTTPStatus

from endpoints import register_user
from models import User
from utils import Routes
from validators import validate_user_reg


async def test_register(service_client):
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK
    assert validate_user_reg(user, response)


async def test_register_same_username(service_client):
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK

    another_user = User(username=user.username)
    response = await register_user(service_client, another_user)
    assert response.status == HTTPStatus.CONFLICT
    assert 'errors' in response.json()["details"]


async def test_register_same_password(service_client):
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK

    another_user = User(password=user.password)
    response = await register_user(service_client, another_user)
    assert response.status == HTTPStatus.OK


async def test_register_invalid_username_empty(service_client):
    user = User(username='')
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.BAD_REQUEST
    assert 'errors' in response.json()["details"]
    errors = response.json()["details"]["errors"]
    assert any(err == 'username' for err in errors)


async def test_register_invalid_username_too_short(service_client):
    user = User(username='ab')
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.BAD_REQUEST
    errors = response.json()["details"]["errors"]
    assert any(err == 'username' for err in errors)


async def test_register_easy_password(service_client):
    user = User(password='123456789')
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.BAD_REQUEST
    errors = response.json()["details"]["errors"]
    assert any(err == 'password' for err in errors)


async def test_register_invalid_password_empty(service_client):
    user = User(password='')
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.BAD_REQUEST
    assert 'errors' in response.json()["details"]


async def test_register_missing_required_field(service_client):
    response = await service_client.post(
        Routes.REGISTRATION,
        json={'user': {'username': 'testuser', 'display_name': 'test'}},
    )

    assert response.status == HTTPStatus.BAD_REQUEST


async def test_register_missing_biography(service_client):
    response = await service_client.post(
        Routes.REGISTRATION,
        json={'user': {'username': 'testuser',
                       'display_name': 'test user!', 'password': "VerYStrong#1234"}}
    )
    # todo можно просто None в user передавать
    assert response.status == HTTPStatus.BAD_REQUEST


async def test_register_invalid_json(service_client):
    response = await service_client.post(
        Routes.REGISTRATION,
        data='invalid json'
    )
    assert response.status == HTTPStatus.BAD_REQUEST


async def test_register_empty_body(service_client):
    response = await service_client.post(
        Routes.REGISTRATION,
        json={}
    )
    assert response.status == HTTPStatus.BAD_REQUEST


async def test_register_multiple_users(service_client):
    users = [User() for _ in range(5)]

    for user in users:
        response = await register_user(service_client, user)
        assert response.status == HTTPStatus.OK
        assert validate_user_reg(user, response)


async def test_register_case_sensitive_username(service_client):
    user1 = User(username='TestUser')
    response = await register_user(service_client, user1)
    assert response.status == HTTPStatus.OK

    user2 = User(username='testuser')
    response = await register_user(service_client, user2)
    assert response.status == HTTPStatus.CONFLICT
