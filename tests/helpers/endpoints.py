from helpers.utils import RequiredFields, Routes, model_dump


async def register_user(service_client, user):
    return await service_client.post(
        Routes.REGISTRATION,
        json=model_dump(user),
    )


async def get_user_by_name(service_client, username, token):
    return await service_client.get(
        Routes.USER_BY_NAME.format(username=(username or "")),
        headers={'Authorization': token or ""},
    )


async def delete_user_by_name(service_client, username, token):
    return await service_client.delete(
        Routes.USER_BY_NAME.format(username=(username or "")),
        headers={'Authorization': token or ""},
    )


async def update_user_by_name(service_client, username, token, user_fields):
    return await service_client.patch(
        Routes.USER_BY_NAME.format(username=(username or "")),
        headers={'Authorization': token or ""},
        json=user_fields,
    )


async def login_user(service_client, user):
    return await service_client.post(
        Routes.LOGIN,
        json=model_dump(user, include=RequiredFields.LOGIN.value),
    )


async def send_message(service_client, message, token):
    return await service_client.post(
        Routes.SEND_MESSAGE,
        json=model_dump(message),
        headers={'Authorization': token or ""},
    )


async def poll_messages(service_client, token):
    # todo
    pass
