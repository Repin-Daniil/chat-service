from testsuite.utils import matching
from typing import List
from helpers.models import User, Message


def validate_user_reg(user: User, response):
    return response.json() == {
        'user': {
            'username': user.username,
            'token': matching.RegexString(r'^[\w-]+\.[\w-]+\.[\w-]+$'),
        },
    }


def validate_profile(user: User, response):
    return response.json() == {
        'user': {
            'username': user.username,
            'display_name': user.display_name,
            'biography': user.biography,
        },
    }


def validate_user_login(user: User, response):
    return response.json() == {
        'token': matching.RegexString(r'^[\w-]+\.[\w-]+\.[\w-]+$'),
    }


def validate_session(response):
    return response.json() == {
        'session_id': matching.RegexString(
            r'^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$'),
    }


def validate_messages(response, expected_messages: List[Message]):
    data = response.json()

    assert 'resync_required' in data, "'Resync required' not found"
    assert 'messages' in data, "Key 'messages' not found"
    assert isinstance(data['messages'], list), "'messages' must be a list"

    actual_messages = data['messages']

    assert len(actual_messages) == len(expected_messages), \
        f"Expected {len(expected_messages)} messages, got {len(actual_messages)}"

    for i, (actual, expected) in enumerate(zip(actual_messages, expected_messages)):

        assert actual['text'] == expected.payload, \
            f"Message #{i}: Text not equal. Expected: '{expected.payload}', got: '{actual.get('text')}'"

        if expected.sender:
            assert actual['sender'] == expected.sender, \
                f"Message #{i}: Wrong sender. Expected: '{expected.sender}', got: '{actual.get('sender')}'"

    return data


def validate_chat(response):
    return 'chat_id' in response.json()
