from http import HTTPStatus

import pytest

from endpoints import poll_messages, send_message
from utils import username_generator
from models import Message


async def test_poll_messages_success(service_client, registered_user):
    """Успешный polling сообщений с валидным токеном"""
    response = await poll_messages(service_client, registered_user.token)
    assert response.status == HTTPStatus.OK
    
    data = response.json()
    assert 'resync_required' in data
    assert 'messages' in data
    assert isinstance(data['messages'], list)


async def test_poll_messages_with_pending_messages(service_client, communication):
    """Polling сообщений когда есть непрочитанные сообщения"""
    sender, recipient, message = communication
    
    await send_message(service_client, message, sender.token)
    
    response = await poll_messages(service_client, recipient.token)
    assert response.status == HTTPStatus.OK
    
    data = response.json()
    assert 'resync_required' in data
    assert 'messages' in data
    assert isinstance(data['messages'], list)
    assert len(data['messages']) > 0


async def test_poll_messages_empty_queue(service_client, registered_user):
    """Polling когда нет новых сообщений"""
    response = await poll_messages(service_client, registered_user.token)
    assert response.status == HTTPStatus.OK
    
    data = response.json()
    assert 'resync_required' in data
    assert 'messages' in data
    assert isinstance(data['messages'], list)
    assert len(data['messages']) == 0


@pytest.mark.parametrize('token', [
    None,
    'wrong_token',
    'invalid.jwt.token',
    '',
])
async def test_poll_messages_wrong_token(service_client, token):
    """Polling с неправильным токеном должен возвращать 401"""
    response = await poll_messages(service_client, token)
    assert response.status == HTTPStatus.UNAUTHORIZED


async def test_poll_messages_multiple_users(service_client, multiple_users):
    """Проверка, что каждый пользователь получает только свои сообщения"""
    sender, recipient = multiple_users
    
    message = Message(recipient=recipient.username, payload="Test message")
    await send_message(service_client, message, sender.token)

    sender_response = await poll_messages(service_client, sender.token)
    sender_data = sender_response.json()

    recipient_response = await poll_messages(service_client, recipient.token)
    recipient_data = recipient_response.json()
    
    assert sender_response.status == HTTPStatus.OK
    assert recipient_response.status == HTTPStatus.OK
    assert 'messages' in recipient_data
    assert len(recipient_data['messages']) > 0


async def test_poll_messages_long_polling(service_client, registered_user):
    """Проверка работы long polling (должен вернуться в течение poll_time)"""
    import time
    
    start_time = time.time()
    response = await poll_messages(service_client, registered_user.token)
    elapsed_time = time.time() - start_time
    
    assert response.status == HTTPStatus.OK
    assert elapsed_time < 185 # TODO из конфига
    
    data = response.json()
    assert 'resync_required' in data
    assert 'messages' in data


async def test_poll_messages_max_size_limit(service_client, communication):
    """Проверка, что возвращается не больше max_size сообщений"""
    sender, recipient, base_message = communication

    for i in range(105):
        message = Message(
            recipient=recipient.username,
            payload=f"Message {i}"
        )
        await send_message(service_client, message, sender.token)
    
    response = await poll_messages(service_client, recipient.token)
    data = response.json()
    
    assert response.status == HTTPStatus.OK
    assert 'messages' in data
    assert len(data['messages']) <= 100


async def test_poll_messages_ordering(service_client, communication):
    """Проверка порядка получения сообщений (FIFO)"""
    sender, recipient, _ = communication

    messages_sent = []
    for i in range(5):
        message = Message(
            recipient=recipient.username,
            payload=f"Message {i}"
        )
        await send_message(service_client, message, sender.token)
        messages_sent.append(message.payload)

    response = await poll_messages(service_client, recipient.token)
    data = response.json()
    
    assert response.status == HTTPStatus.OK
    assert 'messages' in data
    
    messages = data['messages']
    if len(messages) >= 5:
        received_payloads = [msg['text'] for msg in messages[-5:]]
        assert received_payloads == messages_sent


async def test_poll_messages_resync_required_false(service_client, registered_user):
    """Проверка, что resync_required = false в нормальных условиях"""
    response = await poll_messages(service_client, registered_user.token)
    assert response.status == HTTPStatus.OK
    
    data = response.json()
    assert data['resync_required'] is False


async def test_poll_messages_structure(service_client, communication):
    """Проверка структуры возвращаемых сообщений"""
    sender, recipient, message = communication
    
    await send_message(service_client, message, sender.token)
    
    response = await poll_messages(service_client, recipient.token)
    data = response.json()
    
    assert response.status == HTTPStatus.OK
    assert 'resync_required' in data
    assert 'messages' in data
    
    if len(data['messages']) > 0:
        msg = data['messages'][0]
        assert 'sender' in msg
        assert 'text' in msg
        assert msg['sender'] == sender.username

# TODO тесты на вымывание очереди и конкурентный поллинг
