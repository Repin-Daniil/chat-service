# from http import HTTPStatus

# from endpoints import register_user, get_user_by_name
# from models import User
# from utils import Routes, get_user_token
# from validators import validate_user_reg


# # todo наверное в фикстуру надо вынести регистрацию пользователя?
# async def test_get_user(service_client):
#     user = User()
#     response = await register_user(service_client, user)
#     assert response.status == HTTPStatus.OK

#     # fixme Выбрать Bearer или Token?
#     response = await get_user_by_name(service_client, user.username, get_user_token(response))
#     assert response.status == HTTPStatus.OK
#     assert validate_profile(user, response)


# async def test_get_multiple_users(service_client):
#     user_amount = 5
#     users = [User() for _ in range(user_amount)]
#     tokens = []

#     for user in users:
#         response = await register_user(service_client, user)
#         assert response.status == HTTPStatus.OK
#         tokens.append(get_user_token(response))

#     for i in range(user_amount):  # fixme Может чище можно?
#         response = await get_user_by_name(service_client, user[i].username, tokens[i])
#         assert validate_profile(user[i], response)


# async def test_get_user_unauthorized(service_client):
#     user = User()
#     response = await register_user(service_client, user)
#     assert response.status == HTTPStatus.OK

#     response = await get_user_by_name(service_client, user.username, None)
#     assert response.status == HTTPStatus.UNAUTHORIZED

#     response = await get_user_by_name(service_client, user.username,  "wrong_token")
#     assert response.status == HTTPStatus.UNAUTHORIZED


# async def test_get_user_wrong_token(service_client):
#     user = User()
#     response = await register_user(service_client, user)
#     assert response.status == HTTPStatus.OK

#     jwt_token = (
#         'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.'
#         'eyJpZCI6ImlkIn0.'
#         'd9NfuI8JzGnYtjPa5rHRh4Jr104WKp-yls9POZJbe9U'
#     )

#     response = await get_user_by_name(
#         service_client, user.username, 'Bearer {token}'.format(
#             token=jwt_token),
#     )
#     assert response.status == HTTPStatus.UNAUTHORIZED

#     response = await get_user_by_name(
#         service_client, user.username, 'Token {token}'.format(token=jwt_token),
#     )
#     assert response.status == HTTPStatus.UNAUTHORIZED


# async def test_get_user_empty_input(service_client):
#     user = User()
#     response = await register_user(service_client, user)
#     assert response.status == HTTPStatus.OK

#     response = get_user_by_name(service_client, None, get_user_token(response))
#     assert response.status == HTTPStatus.BAD_REQUEST
#     assert 'errors' in response.json()["details"]
