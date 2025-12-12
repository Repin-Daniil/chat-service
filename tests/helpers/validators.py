from testsuite.utils import matching

from helpers.models import User


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
