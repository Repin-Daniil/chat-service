from http import HTTPStatus

from endpoints import register_user
from models import User
from utils import Routes
from validators import validate_user


async def test_get_user(service_client):
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK
    
    response = await get_user_by_name(service_client, get_user_token(user)) #fixme Выбрать Bearer или Token?
    assert response.status == HTTPStatus.OK
    assert validate_profile(user, response)


async def test_get_user_unauthorized(service_client):
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK

    response = await get_user_by_name(service_client, get_user_token(response))
    assert response.status == HTTPStatus.UNAUTHORIZED

async def test_get_user_wrong_token(service_client):
    user = User()
    response = await register_user(service_client, user)
    assert response.status == HTTPStatus.OK

    jwt_token = (
        'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.'
        'eyJpZCI6ImlkIn0.'
        'd9NfuI8JzGnYtjPa5rHRh4Jr104WKp-yls9POZJbe9U'
    )

    response = await get_user_by_name(
        service_client, 'Bearer {token}'.format(token=jwt_token),
    )
    assert response.status == HTTPStatus.UNAUTHORIZED

    response = await get_user_by_name(
        service_client, 'Token {token}'.format(token=jwt_token),
    )
    assert response.status == HTTPStatus.UNAUTHORIZED