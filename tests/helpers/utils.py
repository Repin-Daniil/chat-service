from enum import Enum

from faker import Faker

fake = Faker()  # todo Faker("ru_RU")
fake.seed_instance(4321)


class Routes(str, Enum):
    # users
    LOGIN = '/v1/users/login'
    REGISTRATION = '/v1/users/register'
    USER_BY_NAME = '/v1/users/{username}'

    # messages
    SEND_MESSAGE = '/v1/messages/send'

    def __str__(self) -> str:
        return self.value


class RequiredFields(tuple, Enum):
    LOGIN = 'username', 'password'
    REGISTRATION = 'username', 'password', 'display_name'


def model_dump(model, **kwargs):
    return {model.__class__.__name__.lower(): model.model_dump(exclude_none=True, **kwargs)}


def get_user_token(response):
    return 'Bearer {token}'.format(token=response.json()['user']['token'])


def generate_string(min_length: int, max_length: int, generator: callable, fill_char: str = "x") -> str:
    value = generator()
    if len(value) > max_length:
        value = value[:max_length]
    if len(value) < min_length:
        value = value.ljust(min_length, fill_char)
    return value


def username_generator():
    return generate_string(3, 32, fake.user_name)


def password_generator():
    return generate_string(8, 32, lambda: fake.password(length=12), fill_char="A")


def display_name_generator():
    return generate_string(3, 50, fake.name)


def biography_generator():
    return generate_string(0, 180, fake.paragraph)
