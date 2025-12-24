from http import HTTPStatus

import pytest

from endpoints import poll_messages, send_message
from utils import username_generator
from models import Message
from collections import Counter
import asyncio


async def test_poll_messages_with_pending_messages(service_client, communication, short_polling):
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


async def test_poll_messages_empty_queue(service_client, registered_user, short_polling):
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


async def test_poll_messages_multiple_users(service_client, multiple_users, short_polling):
    """Проверка, что каждый пользователь получает только свои сообщения"""
    sender, recipient = multiple_users

    message = Message(recipient=recipient.username, payload="Test message")
    await send_message(service_client, message, sender.token)

    sender_response = await poll_messages(service_client, sender.token)
    sender_data = sender_response.json()

    recipient_response = await poll_messages(service_client, recipient.token)
    recipient_data = recipient_response.json()

    assert sender_response.status == HTTPStatus.OK
    assert 'messages' in sender_data
    assert len(sender_data['messages']) == 0
    assert recipient_response.status == HTTPStatus.OK
    assert 'messages' in recipient_data
    assert len(recipient_data['messages']) == 1
    assert 'text' in recipient_data['messages'][0]
    assert recipient_data['messages'][0]['text'] == "Test message"


@pytest.mark.parametrize("custom_polling", [(3)], indirect=True)
async def test_poll_messages_long_polling(service_client, registered_user, custom_polling):
    """Проверка работы long polling (должен вернуться в течение poll_time)"""
    import time

    start_time = time.time()
    response = await poll_messages(service_client, registered_user.token)
    elapsed_time = time.time() - start_time

    assert response.status == HTTPStatus.OK
    assert elapsed_time < 5

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


@pytest.mark.usefixtures("short_polling")
async def test_poll_messages_resync_required_false(service_client, registered_user):
    """Проверка, что resync_required = false в нормальных условиях"""
    response = await poll_messages(service_client, registered_user.token)
    assert response.status == HTTPStatus.OK

    data = response.json()
    assert data['resync_required'] is False


@pytest.mark.usefixtures("short_polling")
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


@pytest.mark.parametrize('multiple_users', [4], indirect=True)
async def test_poll_messages_multiple_senders_concurrent(service_client, multiple_users):
    """Проверка получения сообщений от нескольких отправителей одновременно"""
    recipient = multiple_users[0]
    senders = multiple_users[1::]

    async def send_messages_from_sender(sender, count=3):
        tasks = []
        for i in range(count):
            message = Message(
                recipient=recipient.username,
                payload=f"Message {i} from {sender.username}"
            )
            tasks.append(send_message(service_client, message, sender.token))
        await asyncio.gather(*tasks)

    await asyncio.gather(
        send_messages_from_sender(senders[0]),
        send_messages_from_sender(senders[1]),
        send_messages_from_sender(senders[2])
    )

    response = await poll_messages(service_client, recipient.token)
    data = response.json()

    assert response.status == HTTPStatus.OK
    assert 'messages' in data
    assert len(data['messages']) == 9

    senders_in_messages = set(msg['sender'] for msg in data['messages'])
    expected_senders = {sender.username for sender in senders}
    assert senders_in_messages == expected_senders

    sender_counts = Counter(msg['sender'] for msg in data['messages'])
    for sender in senders:
        assert sender_counts[sender.username] == 3


@pytest.mark.parametrize("custom_polling", [20], indirect=True)
async def test_poll_messages_immediate_delivery(
    service_client, communication, custom_polling
):
    sender, recipient, message = communication

    async def delayed_send():
        await asyncio.sleep(0.2)  # даём poll реально уйти
        resp = await send_message(
            service_client, message, sender.token
        )
        assert resp.status == HTTPStatus.ACCEPTED

    poll_task = asyncio.create_task(
        poll_messages(service_client, recipient.token)
    )

    send_task = asyncio.create_task(delayed_send())

    poll_response = await asyncio.wait_for(poll_task, timeout=2.0)

    await send_task

    assert poll_response.status == HTTPStatus.OK
    data = poll_response.json()
    assert len(data["messages"]) == 1
    assert data["messages"][0]["text"] == message.payload


@pytest.mark.parametrize('multiple_users', [3], indirect=True)
async def test_poll_messages_multiple_recipients_one_sender(service_client, multiple_users):
    """Проверка поллинга сообщений несколькими получателями от одного отправителя"""
    sender = multiple_users[0]
    recipient1 = multiple_users[1]
    recipient2 = multiple_users[2]

    messages_to_recipient1 = [
        Message(recipient=recipient1.username,
                payload=f"Hello recipient1, message 0"),
        Message(recipient=recipient1.username,
                payload=f"Sending message 1 to recipient1")
    ]

    messages_to_recipient2 = [
        Message(recipient=recipient2.username,
                payload=f"Hello recipient2, message 0"),
        Message(recipient=recipient2.username,
                payload=f"Sending message 1 to recipient2")
    ]

    send_tasks = []
    for msg in messages_to_recipient1:
        send_tasks.append(send_message(service_client, msg, sender.token))
    for msg in messages_to_recipient2:
        send_tasks.append(send_message(service_client, msg, sender.token))

    await asyncio.gather(*send_tasks)

    response1, response2 = await asyncio.gather(
        poll_messages(service_client, recipient1.token),
        poll_messages(service_client, recipient2.token)
    )

    data1 = response1.json()
    data2 = response2.json()

    assert response1.status == HTTPStatus.OK
    assert 'messages' in data1
    assert len(data1['messages']) == 2

    for msg in data1['messages']:
        assert msg['sender'] == sender.username
        assert 'recipient1' in msg['text']

    payloads1 = [msg['text'] for msg in data1['messages']]
    assert "Hello recipient1, message 0" in payloads1
    assert "Sending message 1 to recipient1" in payloads1

    assert response2.status == HTTPStatus.OK
    assert 'messages' in data2
    assert len(data2['messages']) == 2

    for msg in data2['messages']:
        assert msg['sender'] == sender.username
        assert 'recipient2' in msg['text']

    payloads2 = [msg['text'] for msg in data2['messages']]
    assert "Hello recipient2, message 0" in payloads2
    assert "Sending message 1 to recipient2" in payloads2

    # Проверяем, что сообщения не пересекаются
    assert set(payloads1).isdisjoint(set(payloads2))


@pytest.mark.enable_gc
@pytest.mark.parametrize(
    ("overseer_config", "registry_config"),
    [
        ([True, 1800, 0], 1),
    ],
    indirect=True,
)
async def test_garbage_collection(
    service_client, communication, overseer_config, registry_config, mocked_time, short_polling
):
    sender, recipient, message = communication

    await send_message(service_client, message, sender.token)

    mocked_time.sleep(2)

    await service_client.run_periodic_task('overseer-background-job')

    response = await poll_messages(service_client, recipient.token)
    data = response.json()
    assert 'resync_required' in data
    assert 'messages' in data
    assert isinstance(data['messages'], list)
    assert len(data['messages']) == 0
