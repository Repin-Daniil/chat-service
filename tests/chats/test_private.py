from http import HTTPStatus

import pytest

from models import PrivateChat
from endpoints import get_private_chat
from validators import validate_chat


async def test_new_private_chat(service_client, multiple_users):
    user_a, user_b = multiple_users
    private_chat = PrivateChat(target_username=user_b.username)
    
    response = await get_private_chat(service_client, private_chat, user_a.token)

    assert response.status == HTTPStatus.CREATED
    assert validate_chat(response)

async def test_new_private_chat_to_self(service_client, registered_user):
    private_chat = PrivateChat(target_username=registered_user.username)
    response = await get_private_chat(service_client, private_chat, registered_user.token)

    assert response.status == HTTPStatus.CREATED
    assert validate_chat(response)

async def test_get_existing_private(service_client, multiple_users):
    user_a, user_b = multiple_users
    private_chat = PrivateChat(target_username=user_b.username)
    
    response_1 = await get_private_chat(service_client, private_chat, user_a.token)
    
    assert response_1.status == HTTPStatus.CREATED
    assert validate_chat(response_1)

    response_2 = await get_private_chat(service_client, private_chat, user_a.token)

    assert response_2.status == HTTPStatus.OK
    assert validate_chat(response_2)
    assert response_1.json()["chat_id"] == response_2.json()["chat_id"]

async def test_private_chat_with_unexisting_user(service_client, registered_user):
    private_chat = PrivateChat(target_username="random_unexisting_user")
    response = await get_private_chat(service_client, private_chat, registered_user.token)

    assert response.status == HTTPStatus.NOT_FOUND


async def test_new_private_chat(service_client, multiple_users):
    user_a, user_b = multiple_users
    private_chat = PrivateChat(target_username=user_b.username)
    
    response = await get_private_chat(service_client, private_chat, "wrong token")

    assert response.status == HTTPStatus.UNAUTHORIZED