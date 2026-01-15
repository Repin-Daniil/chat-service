from http import HTTPStatus

import pytest

from endpoints import poll_messages, send_message, start_session
from utils import username_generator, get_session_id
from models import Message, User
from collections import Counter
from validators import validate_messages
import asyncio


async def test_poll_messages_with_pending_messages(service_client, communication, short_polling):
    """Polling сообщений когда есть непрочитанные сообщения"""
    sender, recipient, message = communication

    await send_message(service_client, message, sender.token)

    response = await poll_messages(service_client, recipient)
    assert response.status == HTTPStatus.OK
    validate_messages(response, [message])


async def test_poll_messages_with_multiple_messages(service_client, communication, short_polling):
    sender, recipient, _ = communication
    messages = []

    for i in range(3):
        messages.append(
            Message(recipient=recipient.username, sender=sender.username))
        await send_message(service_client, messages[-1], sender.token)

    response = await poll_messages(service_client, recipient)
    assert response.status == HTTPStatus.OK
    validate_messages(response, messages)


async def test_multiple_polling(service_client, communication, short_polling):
    """Двойной поллинг"""
    sender, recipient, dummy_message = communication
    await send_message(service_client, dummy_message, sender.token)

    response = await poll_messages(service_client, recipient)
    assert response.status == HTTPStatus.OK
    validate_messages(response, [dummy_message])

    # Second part
    messages = []

    for i in range(3):
        messages.append(
            Message(recipient=recipient.username, sender=sender.username))
        await send_message(service_client, messages[-1], sender.token)

    response = await poll_messages(service_client, recipient)
    assert response.status == HTTPStatus.OK
    validate_messages(response, messages)


async def test_messaging_dialog_flow(service_client, communication):
    """
    Тест сценария диалога:
    1. A -> B (B получает)
    2. B -> A (A получает)
    3. A -> B (B получает второй раз)
    """

    user_a, user_b, _ = communication

    # --- Step 1 ---
    msg_hello = Message(
        sender=user_a.username,
        recipient=user_b.username,
        text="Hello, world!"
    )
    await send_message(service_client, msg_hello, user_a.token)

    # --- Step 2 ---
    response_b1 = await poll_messages(service_client, user_b)

    assert response_b1.status == HTTPStatus.OK
    validate_messages(response_b1, [msg_hello])

    # --- Step 3 ---
    msg_reply = Message(
        sender=user_b.username,
        recipient=user_a.username,
        text="World is here"
    )
    await send_message(service_client, msg_reply, user_b.token)

    response_a = await poll_messages(service_client, user_a)

    assert response_a.status == HTTPStatus.OK
    validate_messages(response_a, [msg_reply])


async def test_poll_messages_empty_queue(service_client, single_consumer, short_polling):
    """Polling когда нет новых сообщений"""
    response = await poll_messages(service_client, single_consumer)
    assert response.status == HTTPStatus.OK

    data = response.json()
    assert 'resync_required' in data
    assert 'messages' in data
    assert isinstance(data['messages'], list)
    assert len(data['messages']) == 0


async def test_poll_messages_without_session(service_client, registered_user, short_polling):
    """Polling без сессии"""
    response = await poll_messages(service_client, registered_user)
    assert response.status == HTTPStatus.GONE


@pytest.mark.parametrize('token', [
    None,
    'wrong_token',
    'invalid.jwt.token',
    '',
])
async def test_poll_messages_wrong_token(service_client, token):
    """Polling с неправильным токеном должен возвращать 401"""
    fake_user = User(token="token", session_id="123")
    response = await poll_messages(service_client, fake_user)
    assert response.status == HTTPStatus.UNAUTHORIZED


async def test_poll_messages_multiple_users(service_client, communication, short_polling):
    """Проверка, что каждый пользователь получает только свои сообщения"""
    sender, recipient, _ = communication

    message = Message(recipient=recipient.username, payload="Test message")
    await send_message(service_client, message, sender.token)

    sender_response = await poll_messages(service_client, sender)
    sender_data = sender_response.json()

    recipient_response = await poll_messages(service_client, recipient)
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
async def test_poll_messages_long_polling(service_client, single_consumer, custom_polling):
    """Проверка работы long polling (должен вернуться в течение poll_time)"""
    import time

    start_time = time.time()
    response = await poll_messages(service_client, single_consumer)
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

    response = await poll_messages(service_client, recipient)
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

    response = await poll_messages(service_client, recipient)
    data = response.json()

    assert response.status == HTTPStatus.OK
    assert 'messages' in data

    messages = data['messages']
    if len(messages) >= 5:
        received_payloads = [msg['text'] for msg in messages[-5:]]
        assert received_payloads == messages_sent


@pytest.mark.usefixtures("short_polling")
async def test_poll_messages_resync_required_false(service_client, single_consumer):
    """Проверка, что resync_required = false в нормальных условиях"""
    response = await poll_messages(service_client, single_consumer)
    assert response.status == HTTPStatus.OK

    data = response.json()
    assert data['resync_required'] is False


@pytest.mark.parametrize('queue_config', [1], indirect=True)
@pytest.mark.usefixtures("short_polling")
async def test_poll_messages_resync_required_true(service_client, queue_config, communication):
    """Проверка, что resync_required = true при переполнении очереди"""
    sender, recipient, message = communication

    response_1 = await send_message(service_client, message, sender.token)
    assert response_1.status == HTTPStatus.ACCEPTED
    response_2 = await send_message(service_client, message, sender.token)
    assert response_2.status == HTTPStatus.SERVICE_UNAVAILABLE

    response = await poll_messages(service_client, recipient)
    assert response.status == HTTPStatus.OK

    data = response.json()
    assert data['resync_required'] is True


@pytest.mark.usefixtures("short_polling")
async def test_poll_messages_structure(service_client, communication):
    """Проверка структуры возвращаемых сообщений"""
    sender, recipient, message = communication

    await send_message(service_client, message, sender.token)

    response = await poll_messages(service_client, recipient)
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

    response = await poll_messages(service_client, recipient)
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
        poll_messages(service_client, recipient)
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
        poll_messages(service_client, recipient1),
        poll_messages(service_client, recipient2)
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
    ("gc_config", "sessions_config"),
    [
        ([True, 1800, 0], 1),
    ],
    indirect=True,
)
async def test_garbage_collection(
    service_client, communication, gc_config, sessions_config, mocked_time, short_polling
):
    sender, recipient, message = communication

    await send_message(service_client, message, sender.token)

    mocked_time.sleep(2)

    await service_client.run_periodic_task('gc-background-job')

    response = await poll_messages(service_client, recipient)
    assert response.status == HTTPStatus.GONE


async def test_list_tasks(service_client):
    tasks = await service_client.list_tasks()
    assert 'reset-task' in tasks


async def test_reset(service_client, single_consumer, short_polling, testpoint):
    response = await poll_messages(service_client, single_consumer)
    assert response.status == HTTPStatus.OK

    @testpoint('reset-task/action')
    def task_action(data):
        pass

    await service_client.run_task('reset-task')
    response = await poll_messages(service_client, single_consumer)
    assert response.status == HTTPStatus.GONE


async def test_polling_anothers_session(service_client, communication):
    sender, recipient, _ = communication
    sender.session_id = recipient.session_id

    response = await poll_messages(service_client, sender)
    assert response.status == HTTPStatus.GONE


async def test_polling_rubbish_session_id(service_client, single_consumer):
    single_consumer.session_id = "random text"
    response = await poll_messages(service_client, single_consumer)
    assert response.status == HTTPStatus.GONE


async def test_polling_empty_session_id(service_client, single_consumer):
    single_consumer.session_id = ""
    response = await poll_messages(service_client, single_consumer)
    assert response.status == HTTPStatus.GONE
