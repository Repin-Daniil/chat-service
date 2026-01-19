from http import HTTPStatus

import pytest

from endpoints import send_message, start_session, poll_messages
from models import Message
from utils import Routes, username_generator, get_session_id
from validators import validate_user_reg, validate_messages


async def test_send_message(service_client, communication, monitor_client):
    sender, recipient, message = communication
    response = await send_message(service_client, message, sender.token)
    assert response.status == HTTPStatus.ACCEPTED


async def test_send_yourself(service_client, registered_user):
    message = Message(recipient=registered_user.username)
    session_id = get_session_id(await start_session(service_client, registered_user.token))
    response = await send_message(service_client, message, registered_user.token)
    assert response.status == HTTPStatus.ACCEPTED


async def test_session_fanout(service_client, communication):
    sender, recipient, message = communication
    recipient_sessions = [recipient.session_id]

    for i in range(3):
        session = await start_session(service_client, recipient.token)
        recipient_sessions.append(get_session_id(session))

    response = await send_message(service_client, message, sender.token)
    assert response.status == HTTPStatus.ACCEPTED

    for session in recipient_sessions:
        recipient.session_id = session
        response = await poll_messages(service_client, recipient)

        assert response.status == HTTPStatus.OK
        validate_messages(response, [message])


@pytest.mark.parametrize('queue_config', [1], indirect=True)
async def test_queue_overloaded(service_client, registered_user, queue_config):
    registered_user.session_id = get_session_id(await start_session(service_client, registered_user.token))
    message = Message(recipient=registered_user.username,
                      sender=registered_user.username)

    response_1 = await send_message(service_client, message, registered_user.token)
    assert response_1.status == HTTPStatus.ACCEPTED
    response_2 = await send_message(service_client, message, registered_user.token)
    assert response_2.status == HTTPStatus.SERVICE_UNAVAILABLE


async def test_send_offline(service_client, registered_user):
    message = Message(recipient=registered_user.username)
    response = await send_message(service_client, message, registered_user.token)
    assert response.status == HTTPStatus.CONFLICT


async def test_wrong_recipient(service_client, registered_user):
    message = Message(recipient=username_generator())
    response = await send_message(service_client, message, registered_user.token)

    assert response.status == HTTPStatus.NOT_FOUND


@pytest.mark.parametrize('token', [
    None,
    'wrong_token',
])
async def test_wrong_token(service_client, communication, token):
    sender, recipient, message = communication
    response = await send_message(service_client, message, token)

    assert response.status == HTTPStatus.UNAUTHORIZED


async def test_empty_recipient(service_client, registered_user):
    message = Message()
    response = await send_message(service_client, message, registered_user.token)

    assert response.status == HTTPStatus.BAD_REQUEST


async def test_empty_payload(service_client, multiple_users):
    sender, recipient = multiple_users
    message = Message.model_construct(
        payload=None, recipient=recipient.username)
    response = await send_message(service_client, message, sender.token)

    assert response.status == HTTPStatus.BAD_REQUEST


async def test_too_large_payload(service_client, registered_user):
    message = Message(payload="A" * 4097, recipient=registered_user.username)
    response = await send_message(service_client, message, registered_user.token)

    assert response.status == HTTPStatus.BAD_REQUEST
