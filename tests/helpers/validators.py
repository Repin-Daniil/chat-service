from testsuite.utils import matching

from helpers.models import User


def validate_user(user: User, response):
    return response.json() == {
        'user': {
            'token': matching.RegexString(r'^[\w-]+\.[\w-]+\.[\w-]+$'),
            'username': user.username,
        },
    }
