from enum import Enum

from faker import Faker

fake = Faker()
fake.seed_instance(4321)


class Routes(str, Enum):
    LOGIN = '/v1/users/login'
    REGISTRATION = '/v1/users/register'
    GET_USER_BY_NAME = '/v1/users/{username}'

    def __str__(self) -> str:
        return self.value


class RequiredFields(tuple, Enum):
    LOGIN = 'username', 'password'
    REGISTRATION = 'username', 'password', 'display_name'


def model_dump(model, **kwargs):
    return {model.__class__.__name__.lower(): model.model_dump(**kwargs)}


def get_user_token(response):
    return 'Bearer {token}'.format(token=response.json()['user']['token'])
