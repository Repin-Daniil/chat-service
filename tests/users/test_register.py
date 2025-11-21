from http import HTTPStatus

from helpers.endpoints import register_user
from helpers.models import User
from helpers.validators import validate_user


async def test_register(service_client):
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK
    assert validate_user(user, response)


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
