from helpers.utils import RequiredFields, Routes, model_dump


async def register_user(service_client, user):
    return await service_client.post(
        Routes.REGISTRATION,
        json=model_dump(user),
    )


async def get_user_by_name(service_client, username, token):
    return await service_client.get(
        Routes.GET_USER_BY_NAME.format(username=(username or "")),
        headers={'Authorization': token or ""},
    )


async def login_user(service_client, user):
    return await service_client.post(
        Routes.LOGIN,
        json=model_dump(user, include=RequiredFields.LOGIN.value),
    )
