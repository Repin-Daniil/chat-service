# from http import HTTPStatus

# import pytest

# from endpoints import send_message
# from models import Message
# from utils import Routes, username_generator
# from validators import validate_user_reg


# async def test_send_message(service_client, communication):
#     sender, recipient, message = communication
#     response = await send_message(service_client, message, sender.token)
#     assert response.status == HTTPStatus.ACCEPTED


# async def test_send_yourself(service_client, registered_user):
#     message = Message(recipient=registered_user.username)
#     response = await send_message(service_client, message, registered_user.token)
#     assert response.status == HTTPStatus.ACCEPTED


# async def test_wrong_recipient(service_client, registered_user):
#     message = Message(recipient=username_generator())
#     response = await send_message(service_client, message, registered_user.token)

#     assert response.status == HTTPStatus.NOT_FOUND


# @pytest.mark.parametrize('token', [
#     None,
#     'wrong_token',
# ])
# async def test_wrong_token(service_client, communication, token):
#     sender, recipient, message = communication
#     response = await send_message(service_client, message, token)

#     assert response.status == HTTPStatus.UNAUTHORIZED


# async def test_empty_recipient(service_client, registered_user):
#     message = Message()
#     response = await send_message(service_client, message, registered_user.token)

#     assert response.status == HTTPStatus.BAD_REQUEST


# async def test_empty_text(service_client, multiple_users):
#     sender, recipient = multiple_users
#     message = Message.model_construct(text=None, recipient=recipient.username)
#     response = await send_message(service_client, message, sender.token)

#     assert response.status == HTTPStatus.BAD_REQUEST
